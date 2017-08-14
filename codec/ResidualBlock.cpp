
#include "ResidualBlock.h"
#include "huffman.h"
#include <time.h>
#include <iostream>
#include <queue>
#include "cache.h"

using namespace std;

#define BASE_MALLOC_SIZE 360000

int head_len_test[3600 * 3];
int head_len_test_i = 0;
int head_len_test_out[3600 * 3];
int head_len_test_out_i = 0;

uint8_t* PKT::stream_buff = nullptr;
uint8_t* PKT::head_buff = nullptr;

ResidualBlock::ResidualBlock(Block::BlockType type,int height , int width):tree(0,0,height-1,width-1),block_type(type),left_zero_num(0){
	data.clear();
	data.resize(width*height);
}



ResidualBlock::ResidualBlock():tree(0,0,0,0),curr_node(0),left_zero_num(0)
{
}

ResidualBlock::ResidualBlock(int blocktype):
	tree(0,0,0,0),curr_node(0),left_zero_num(0)
{
	block_type = (Block::BlockType)blocktype;
}
ResidualBlock::ResidualBlock(const Block & block):tree(0,0,0,0),curr_node(0),data(block.data.size()),left_zero_num(0){
	block_id = block.block_id;
	block_type = block.block_type;
	//tree = tree(0,0,height,width);
}

Node & ResidualBlock::get_node(int &id){
	id = curr_node;
	return node_list[curr_node++];
}
void ResidualBlock::getBlockSize(AVFormat &para, int& height, int& width)
{
	if(block_type == Block::Y)
	{
		height = para.block_height;
		width = para.block_width;
	}
	else 
	{
		height = para.block_height / 2;
		width = para.block_width / 2;
	}
}



/**
*    节点的流化
*    李春尧
**/
int Node::to_stream(unsigned char * stream){
	*(stream) = (0x7F & prediction);
	int byte = 1;
	if(pre_type == INTRA_PREDICTION){
		*(stream) |= (unsigned char)0x00;
	}else{
		*(stream) |= (unsigned char)0x80;

		memcpy(stream + 1,&inter_value,1);
		++byte;
		//memcpy(stream+1,mv,2);
		//*(stream +1 ) = mv[0];
		
		//*(stream + 2) = mv[1];

		//byte += 2;

	}

	return byte;
}

/**
*    节点的反流化
*    李春尧
**/
int Node::from_stream(unsigned char * stream){
	int need_byte = 0;
	prediction = (*(stream + need_byte)) & ((unsigned char)0x7F);
	unsigned char type = *(stream + need_byte) & (unsigned char)0x80;
	++need_byte;
	if(type == (unsigned char)0x00){
		pre_type = INTRA_PREDICTION;
	}else{
		pre_type = INTER_PREDICTION;

		memcpy(&inter_value,stream + 1,1);
		++need_byte;
		//memcpy(mv,stream+1,2);
		//need_byte += 2;
		//mv[0] = *(stream + need_byte);
		//++need_byte;
		//mv[1] = *(stream + need_byte);
		//++need_byte;
	}
	return need_byte;

}

Tree::Tree():node_id(-1),
				split_direction(NONE),left(nullptr),right(nullptr),
				left_top_h(0),left_top_w(0),right_bottom_h(0),right_bottom_w(0),score(-1)
{
};
Tree::Tree(int left_top_h,int left_top_w,int right_bottom_h,int right_bottom_w):node_id(-1),
				split_direction(NONE),left(nullptr),right(nullptr),
				left_top_h(left_top_h),left_top_w(left_top_w),right_bottom_h(right_bottom_h),right_bottom_w(right_bottom_w),score(-1){
}



/**
*    树的流化
*    李春尧
**/
void Tree::serialize(unsigned char * stream,int &byte,int &bit,int * used_node_ids,int &idx){
	unsigned char * tmp = stream + byte;

	if(bit == 0) *tmp = 0x00;
	if(this->split_direction == NONE){
		used_node_ids[idx++] = node_id;
		*tmp |= ((unsigned char)0x00 << bit);
		bit += 2;
		if(bit == 8){
			bit = 0;
			++byte;
		}
	}else{
		if(this->split_direction == HORIZONTAL){
			*tmp |= ((unsigned char)0x01 << bit);

		}else{
			*tmp |= ((unsigned char)0x02 << bit);
		}
		bit += 2;
		if(bit == 8){
			bit = 0;
			++byte;
		}
		this->left->serialize(stream,byte,bit,used_node_ids,idx);
		this->right->serialize(stream,byte,bit,used_node_ids,idx);
	}
	
}

/**
*    树的反流化
*    李春尧
**/
void Tree::deserialize(unsigned char * stream,int &byte,int &bit,Node * node_list,int &idx,int block_id,int block_type){
	unsigned char * tmp = stream + byte;
	unsigned char type = ((*tmp) >> bit) & (unsigned char)0x03;
	int w = right_bottom_w - left_top_w + 1;
	int h = right_bottom_h - left_top_h + 1;
	if(type == 0x00){
		bit += 2;
		if(bit == 8){
			bit = 0;
			++byte;
		}
		this->left = nullptr;
		this->right = nullptr;
		this->node_id = idx;
		this->split_direction = NONE;
		this->data = node_list + idx;
		idx += 1;
	}else{
		if(type == 0x01){
			this->split_direction = HORIZONTAL;
			
			uint64_t tmp = ((uint64_t)block_id << 32) | ((uint64_t)left_top_h << 24) | ((uint64_t)left_top_w << 16) | ((uint64_t)right_bottom_h << 8) | (right_bottom_w- (w/2));
			this->left = cache::getTree(block_type,tmp);
			this->left->left_top_h = left_top_h;
			this->left->left_top_w = left_top_w;
			this->left->right_bottom_h = right_bottom_h;
			this->left->right_bottom_w = (right_bottom_w- (w/2));

			tmp = ((uint64_t)block_id << 32) | ((uint64_t)left_top_h << 24) | ((uint64_t)(left_top_w+ (w/2)) << 16) | ((uint64_t)right_bottom_h << 8) | right_bottom_w;
			this->right = cache::getTree(block_type,tmp);
			//this->left = cache::getTree(type,tmp);
			this->right->left_top_h = left_top_h;
			this->right->left_top_w = left_top_w+ (w/2);
			this->right->right_bottom_h = right_bottom_h;
			this->right->right_bottom_w = right_bottom_w;
			
			//this->left = new Tree(left_top_h,left_top_w,right_bottom_h,right_bottom_w - (w/2));
			//this->right = new Tree(left_top_h,left_top_w + (w/2),right_bottom_h,right_bottom_w);
		}else{
			this->split_direction = VERTICAL;
			
			uint64_t tmp = ((uint64_t)block_id << 32) | ((uint64_t)left_top_h << 24) | ((uint64_t)left_top_w << 16) | ((uint64_t)(right_bottom_h- (h/2)) << 8) | (right_bottom_w);
			this->left = cache::getTree(block_type,tmp);
			this->left->left_top_h = left_top_h;
			this->left->left_top_w = left_top_w;
			this->left->right_bottom_h = right_bottom_h- (h/2);
			this->left->right_bottom_w = (right_bottom_w);

			tmp = ((uint64_t)block_id << 32) | ((uint64_t)(left_top_h+ (h/2)) << 24) | ((uint64_t)(left_top_w) << 16) | ((uint64_t)right_bottom_h << 8) | right_bottom_w;
			this->right = cache::getTree(block_type,tmp);
			//this->left = cache::getTree(type,tmp);
			this->right->left_top_h = left_top_h+ (h/2);
			this->right->left_top_w = left_top_w;
			this->right->right_bottom_h = right_bottom_h;
			this->right->right_bottom_w = right_bottom_w;
			
			//this->left = new Tree(left_top_h,left_top_w,right_bottom_h - (h/2),right_bottom_w);
			//this->right = new Tree(left_top_h + (h/2),left_top_w,right_bottom_h,right_bottom_w);
		}
		bit += 2;
		if(bit == 8){
			bit = 0;
			++byte;
		}
		this->node_id = -1;
		this->left->deserialize(stream,byte,bit,node_list,idx,block_id,block_type);
		this->right->deserialize(stream,byte,bit,node_list,idx,block_id,block_type);
	}
	
}

/**
*    树的流化api
*    李春尧
**/
int Tree::to_stream(unsigned char * stream,int * used_node_ids,int &num){
	int byte = 0,bit = 0,idx=0;
	this->serialize(stream,byte,bit,used_node_ids,idx);
	if(bit != 0){
		byte += 1;
	}
	num = idx;

	return byte;
}

/**
*    树的反流化api
*    李春尧
**/
int Tree::from_stream(unsigned char * stream,Node * node_list,int &num,int block_id,int type){
	int byte = 0,bit = 0,idx=0;
	this->deserialize(stream,byte,bit,node_list,idx,block_id,type);
	if(bit != 0)
		byte += 1;
	num = idx;

	return byte;
}


/* 基础类型写入流
* 易惠康
*/
template<typename T>
inline int save_to_buffer(const T &val, unsigned char *buffer) {
	memcpy(buffer, &val, sizeof(val));
	return sizeof(val);
}

template<typename T>
inline int save_to_buffer(const T &val, unsigned char *buffer,int length) {
	memcpy(buffer, val, length);
	return length;
}

template<typename T>
inline int get_from_buffer(T &val, unsigned char *buffer) {
	memcpy(&val, buffer, sizeof(val));
	return sizeof(val);
}

template<typename T>
inline int get_from_buffer(T &val, unsigned char *buffer, int length) {
	memcpy(val, buffer, length);
	return length;
}


int ResidualBlock::tree_to_stream(){
	tree_byte = tree.to_stream(tree_buff,used_node_ids,curr_node);
	return tree_byte;
}

/*
* 将ResidualBlock写入流
* 流stream需要预先开辟空间
*  已废弃
*/

int ResidualBlock::to_stream(unsigned char *stream,AVFormat &para) {
	unsigned char *p = stream;
	p += save_to_buffer(block_id, p);
	p += save_to_buffer(order, p);
	p += save_to_buffer(block_type, p);

	if(!para.is_tree){
		p += save_to_buffer(type_slice, p);
		if(type_slice==0)
			p += save_to_buffer(node[0], p);
		if(type_slice==1){
			for(int i=0;i<4;++i)
				p += save_to_buffer(node[i], p);
		}
	}else{

		memset(used_node_ids,unsigned char(128),128);

		
		if(tree_byte > 0){
			p += save_to_buffer(tree_buff,p,tree_byte);
		}else{
			p += tree.to_stream(p,used_node_ids,curr_node);
		}
		int start_byte = 0;
		for(int i = 0; i < curr_node; ++i){
			if(used_node_ids[i] == (unsigned char)128) break;
			p += node_list[used_node_ids[i]].to_stream(p); 
		}
	}


	short *pdata = data.data(); 

	p += save_to_buffer(pdata, p,data.size() * sizeof(data[0]));
	return p - stream;
}


/*
* 从流中还原出ResidualBlock
* *  已废弃
*/

int ResidualBlock::from_stream(unsigned char *stream, int block_size,AVFormat &para) {
	unsigned char *p = stream;
	p += get_from_buffer(block_id, p);
	p += get_from_buffer(order, p);
	p += get_from_buffer(block_type, p);

	if(!para.is_tree){
		p += get_from_buffer(type_slice, p);
		if(type_slice==0){
			node.resize(1);
			p += get_from_buffer(node[0], p);
		}
		if(type_slice==1){
			node.resize(4);
			for(int i=0;i<4;++i)
				p += get_from_buffer(node[i], p);
		}
	}else{
		int num = 0;
		tree.left_top_h = 0;
		tree.left_top_w = 0;
		
		getBlockSize(para,tree.right_bottom_h,tree.right_bottom_w);
		tree.right_bottom_h -=  1;
		tree.right_bottom_w -=  1;
		p += tree.from_stream(p,node_list,num,block_id,(int)block_type);

		for(int i = 0; i < num; ++i){
			p += node_list[curr_node ++].from_stream(p);
		}
	}

	data.resize(block_size);

	short *pdata = data.data();
	p += get_from_buffer(pdata, p, block_size * sizeof(data[0]));
	return p - stream;
}

/*
* 将ResidualBlock的分块和模式信息写入流
* 流stream需要预先开辟空间
*/
int ResidualBlock::head_to_stream(unsigned char *stream,AVFormat &para){
	unsigned char *p = stream;

	if(!para.is_tree){
		p += 1;
		p += save_to_buffer(type_slice, p);
		if(type_slice==0)
			p += save_to_buffer(node[0], p);
		if(type_slice==1){
			for(int i=0;i<4;++i)
				p += save_to_buffer(node[i], p);
		}
		uint8_t len = p - stream;
		memcpy(stream,&len,sizeof(len));

	}else{
		memset(used_node_ids,-1,1024 * sizeof(int));

		p += tree.to_stream(p,used_node_ids,curr_node);
		int start_byte = 0;
		for(int i = 0; i < curr_node; ++i){
			if(used_node_ids[i] == -1) break;
			p += node_list[used_node_ids[i]].to_stream(p); 
		}
		//if(left_zero_num > 8)
		*p = left_zero_num;
		++p;
	}

	return p - stream;
}
/*
* 将ResidualBlock的分块和模式信息从流读入
* 流stream需要预先开辟空间
*/
int ResidualBlock::head_from_stream(unsigned char *stream, AVFormat &para){
	unsigned char *p = stream;
	if(!para.is_tree){
		uint8_t len;
		p += get_from_buffer(len,stream);
		p += get_from_buffer(type_slice, p);
		if(type_slice==0){
			node.resize(1);
			p += get_from_buffer(node[0], p);
		}
		if(type_slice==1){
			node.resize(4);
			for(int i=0;i<4;++i)
				p += get_from_buffer(node[i], p);
		}
	}else{
		int num = 0;
		curr_node = 0;
		tree.left_top_h = 0;
		tree.left_top_w = 0;
		
		getBlockSize(para,tree.right_bottom_h,tree.right_bottom_w);
		tree.right_bottom_h -=  1;
		tree.right_bottom_w -=  1;
		p += tree.from_stream(p,node_list,num,block_id,block_type);

		for(int i = 0; i < num; ++i){
			p += node_list[curr_node ++].from_stream(p);
		}

		left_zero_num = *p;
		++p;
	}

	return p - stream;

}

/*
* 将ResidualBlock的数据信息写入流
* 流stream需要预先开辟空间
*/
int ResidualBlock::data_to_stream(unsigned char *stream,AVFormat &para){
	unsigned char *p = stream;
	short *pdata = data.data(); 
	p += save_to_buffer(pdata, p,data.size() * sizeof(data[0]));
	return p - stream;
}

/*
* 将ResidualBlock的数据信息从流读入
* 流stream需要预先开辟空间
*/
int ResidualBlock::data_from_stream(unsigned char *stream, int block_size, AVFormat &para){
	unsigned char *p = stream;
	data.resize(block_size);
	//}
	short *pdata = data.data();
	p += get_from_buffer(pdata, p, block_size * sizeof(data[0]));
	return p - stream;
}




/*
* 清空ResidualBlock,方便复用
*/
int ResidualBlock::clear()
{
	//data.clear();
	//node.clear();  //清楚非树形划分中的保存的node信息，方便复用 董辰辰
	//tree.clear();
	memset(data.data(),0,data.size() * sizeof(data[0]));
	return 0;
}

 
/**
* 为PKT预分配空间
*/
int PKT::reserve(int size){
	Ylist.reserve(size);
	Ulist.reserve(size);
	Vlist.reserve(size);
	return 0;
}
int PKT::init(AVFormat& para){
	int h,w;
	para.getBlockSize(Block::Y,h,w);
	Ylist.resize(para.block_num,ResidualBlock(Block::Y,h,w));
	para.getBlockSize(Block::U,h,w);
	Ulist.resize(para.block_num,ResidualBlock(Block::U,h,w));
	para.getBlockSize(Block::V,h,w);
	Vlist.resize(para.block_num,ResidualBlock(Block::V,h,w));

	return 0;
}
/*
* 将PKT写入流
* 流stream需要预先开辟空间
*/

int PKT::to_stream(unsigned char *stream, AVFormat &para) {
	unsigned char *p = stream;
	for (int i = 0; i < (int) Ylist.size(); i++) {

		p += Ylist[i].to_stream(p,para);
	}
	for (int i = 0; i < (int) Ulist.size(); i++) {

		p += Ulist[i].to_stream(p,para);
	}
	for (int i = 0; i < (int) Vlist.size(); i++) {

		p += Vlist[i].to_stream(p,para);
	}
	return p - stream;
}

/*
* 从流中还原出PKT
*/
int PKT::from_stream(unsigned char *stream, AVFormat &para) {
	unsigned char *p = stream;
	int n_row = (para.height + para.block_height - 1) / para.block_height;
	int n_col = (para.width + para.block_width - 1) / para.block_width;
	Ylist.resize(n_row * n_col, ResidualBlock(0));
	Ulist.resize(n_row * n_col, ResidualBlock(1));
	Vlist.resize(n_row * n_col, ResidualBlock(2));

	int yblock_size = para.block_height * para.block_width;
	for (int i = 0; i < (int) Ylist.size(); i++) {

		p += Ylist[i].from_stream(p, yblock_size,para);
	}

	for (int i = 0; i < (int) Ulist.size(); i++) {

		p += Ulist[i].from_stream(p, yblock_size / 4, para);
	}
	for (int i = 0; i < (int) Vlist.size(); i++) {

		p += Vlist[i].from_stream(p, yblock_size / 4, para);
	}
	return p - stream;
}

/**
*  将一个分量写入流
*/
int PKT::stream_write_one_component(AVFormat& para,std::vector<ResidualBlock> & list){

	uint8_t *head_out = nullptr;
	unsigned int head_out_len = 0;
	//uint8_t *point;
	unsigned int len;
	uint8_t *tmp_stream = nullptr;
	int block_num = para.block_num;
	
	static const int head_buffer_size = 2048;
	static uint8_t *tmp_head_t = (uint8_t*)malloc(sizeof(uint8_t) * BASE_MALLOC_SIZE);
	static uint8_t * tmp_head = (uint8_t*)malloc(sizeof(uint8_t) * head_buffer_size);
	static int head_N = 1;
	int one_head_len;
	int head_len = 0;
	uint8_t *head_point = tmp_head_t;
	

	entropy_encode_by_frame(list.data(),block_num,para,&tmp_stream,&len);


	for(int i = 0;i<list.size();++i)
	{
		one_head_len = list[i].head_to_stream(tmp_head,para);

		while(one_head_len + head_len > head_N * BASE_MALLOC_SIZE)
		{
			++head_N;
			tmp_head_t = (uint8_t *)realloc(tmp_head_t, sizeof(uint8_t) * head_N * BASE_MALLOC_SIZE);
			head_point = tmp_head_t + head_len;
		}

		memcpy(head_point,tmp_head,one_head_len);
	
		head_point += one_head_len;
		head_len += one_head_len;
	}


	huffman_encode_memory(tmp_head_t,head_len,&head_out,&head_out_len);	//2017/8/9 gaowk
	//toch4(head_out_len,head_len_ch);

	fwrite(&head_out_len,sizeof(unsigned int),1,para.stream_writer);
	head_len_test[++head_len_test_i] = head_out_len;
	fwrite(head_out,sizeof(uint8_t),head_out_len,para.stream_writer);
	//fwrite(&len,sizeof(unsigned int),1,para.stream_writer);
	fwrite(tmp_stream,sizeof(uint8_t),len,para.stream_writer);

	free(tmp_stream);
	free(head_out);
	return 0;
}
/*
* 将PKT中的数据写成流使用熵编码后进行文件写入
*/
int PKT::stream_write(AVFormat& para)
{
	//对一帧的Y数据进行熵编码
	if(para.stream_writer == nullptr)
	{
		para.stream_writer = fopen(para.stream_file_name,"wb");
	}
	stream_write_one_component(para,Ylist);
	stream_write_one_component(para,Ulist);
	stream_write_one_component(para,Vlist);
	/*
	uint8_t *tmp_stream;
	
	
	uint8_t *head_out = nullptr;
	unsigned int head_out_len = 0;
	uint8_t head_len_ch[4];

	//uint8_t *point;
	unsigned int len;
	unsigned int stream_len;
	int block_num = para.width*para.height/para.block_height/para.block_width;

	//int start_time=clock();
	entropy_encode_slice(Ylist.data(),block_num,para,&tmp_stream,&len);
	//int end_time=clock();

	//std::cout<< "Stream wirte Running time is: "<<static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;	

	static uint8_t *tmp_head_t = (uint8_t*)malloc(sizeof(uint8_t) * BASE_MALLOC_SIZE);
	static uint8_t * tmp_head = (uint8_t*)malloc(sizeof(uint8_t) * 2048);
	static int head_N = 1;
	int one_head_len;
	int head_len = 0;
	uint8_t *head_point = tmp_head_t;
	
	for(int i = 0;i<Ylist.size();++i)
	{
		block_head2stream(para,&tmp_head,Ylist[i],&one_head_len);
		
		if(one_head_len + head_len > head_N * BASE_MALLOC_SIZE)
		{
			++head_N;
			tmp_head_t = (uint8_t *)realloc(tmp_head_t, sizeof(uint8_t) * head_N * BASE_MALLOC_SIZE);
			head_point = tmp_head + head_len;
		}

		memcpy(head_point,tmp_head,one_head_len);
	
		head_point += one_head_len;
		head_len += one_head_len;
	}


	huffman_encode_memory(tmp_head_t,head_len,&head_out,&head_out_len);	//2017/8/9 gaowk
	toch4(head_out_len,head_len_ch);

	fwrite(head_len_ch,sizeof(uint8_t),4,para.stream_writer);

	fwrite(head_out,sizeof(uint8_t),head_out_len,para.stream_writer);;
	fwrite(tmp_stream,sizeof(uint8_t),len,para.stream_writer);

	free(tmp_stream);
	//stream_len = len + head_out_len + 2;
	
	//对一帧的U数据进行熵编码
	entropy_encode_slice(Ulist.data(),block_num,para,&tmp_stream,&len);

	head_point = tmp_head_t;
	head_len = 0;

	for(int i = 0;i<Ulist.size();++i)
	{
		block_head2stream(para,&tmp_head,Ulist[i],&one_head_len);

		if(one_head_len + head_len > head_N * BASE_MALLOC_SIZE)
		{
			++head_N;
			tmp_head_t = (uint8_t *)realloc(tmp_head_t, sizeof(uint8_t) * head_N * BASE_MALLOC_SIZE);
			head_point = tmp_head + head_len;
		}

		memcpy(head_point,tmp_head,one_head_len);
	
		head_point += one_head_len;
		head_len += one_head_len;
	}

	huffman_encode_memory(tmp_head_t,head_len,&head_out,&head_out_len);	//2017/8/9 gaowk
	toch4(head_out_len,head_len_ch);


	fwrite(head_len_ch,sizeof(uint8_t),4,para.stream_writer);
	fwrite(head_out,sizeof(uint8_t),head_out_len,para.stream_writer);

	fwrite(tmp_stream,sizeof(uint8_t),len,para.stream_writer);

	free(tmp_stream);
	//stream_len = len + head_out_len + 2;
	
	//对一帧的V数据进行熵编码
	entropy_encode_slice(Vlist.data(),block_num,para,&tmp_stream,&len);

	head_point = tmp_head_t;
	head_len = 0;

	for(int i = 0;i<Vlist.size();++i)
	{
		block_head2stream(para,&tmp_head,Vlist[i],&one_head_len);

		if(one_head_len + head_len > head_N * BASE_MALLOC_SIZE)
		{
			++head_N;
			tmp_head_t = (uint8_t *)realloc(tmp_head_t, sizeof(uint8_t) * head_N * BASE_MALLOC_SIZE);
			head_point = tmp_head + head_len;
		}

		memcpy(head_point,tmp_head,one_head_len);
	
		head_point += one_head_len;
		head_len += one_head_len;
	}
	huffman_encode_memory(tmp_head_t,head_len,&head_out,&head_out_len);	//2017/8/9 gaowk
	toch4(head_out_len,head_len_ch);


	fwrite(head_len_ch,sizeof(uint8_t),4,para.stream_writer);
	fwrite(head_out,sizeof(uint8_t),head_out_len,para.stream_writer);

	fwrite(tmp_stream,sizeof(uint8_t),len,para.stream_writer);

	free(tmp_stream);

	*/
	return 0;
}

/**
*  将一个分量从流读入
*/
int PKT::stream_read_one_component(AVFormat& para,std::vector<ResidualBlock> & list,Block::BlockType type){
	cache::reset(type);
	uint8_t *tmp_head_t = nullptr;
	unsigned int out_len;
	int head_len;
	//unsigned int len = 1;
	unsigned int stream_len = 0;
	uint8_t stream_len_tmp[4];

	static int head_N = 1;
	static uint8_t * tmp_head = (uint8_t*)malloc(sizeof(uint8_t) * BASE_MALLOC_SIZE);
	
	int head_total_len = 0;
	fread(&head_total_len,sizeof(unsigned int),1,para.stream_reader);
	head_len_test_out[head_len_test_out_i] = head_total_len;
	if(head_len_test_out[head_len_test_out_i] != head_len_test[head_len_test_out_i])
	{
		int o = 0;
	}

	head_len_test_out_i++;

	while(head_total_len > BASE_MALLOC_SIZE * head_N){
		++head_N;
		tmp_head = (uint8_t *)realloc(tmp_head, sizeof(uint8_t) * head_N * BASE_MALLOC_SIZE);
	}
	
	fread(tmp_head,1,head_total_len,para.stream_reader);

	huffman_decode_memory(tmp_head,head_total_len,&tmp_head_t,&out_len);

	uint8_t *point = tmp_head_t;
	int block_num = para.block_num;

	for(int i = 0;i<para.block_num;++i)
	{
		list[i].block_id = i;
		list[i].block_type = type;
		head_len = list[i].head_from_stream(point,para);

		point += head_len;
	}

	//fread(&stream_len,sizeof(int),1,para.stream_reader);
	fread(stream_len_tmp,1,4,para.stream_reader);
	fromch4(stream_len,stream_len_tmp);

	static int stream_N = 1;
	
	static uint8_t * stream_buff = (uint8_t*)malloc(BASE_MALLOC_SIZE);

	while(stream_len > BASE_MALLOC_SIZE * stream_N){
		++stream_N;
		stream_buff = (uint8_t *)realloc(stream_buff, sizeof(uint8_t) * stream_N * BASE_MALLOC_SIZE);
	}

	//int stream_read_len;
	//unsigned char stream_read_len_ch[4];
	//fread(stream_read_len_ch,sizeof(int),stream_len,para.stream_reader);

	//fromch4(stream_read_len,stream_read_len_ch);

	fread(stream_buff,sizeof(uint8_t),stream_len,para.stream_reader);

	//entropy_decode_slice(list.data(),block_num,para,stream_buff,BASE_MALLOC_SIZE * stream_N);
	entropy_decode_by_frame(list.data(),block_num,para,stream_buff,stream_len);
	free(tmp_head_t);
	return 0;
}
/*
* 将流文件进行反熵编码后读入PKT
*/
int PKT::stream_read(AVFormat& para)
{

	if(para.stream_reader == nullptr)
	{
		para.stream_reader = fopen(para.stream_file_name,"rb");
	}
	stream_read_one_component(para,Ylist,Block::Y);
	stream_read_one_component(para,Ulist,Block::U);
	stream_read_one_component(para,Vlist,Block::V);
	/*
	if(stream_buff == nullptr)
		stream_buff = (uint8_t*)malloc(1000000);
	uint8_t *tmp_head;
	uint8_t stream_len[4];
	uint8_t *point = nullptr;
	uint8_t *tmp_head_t = nullptr;

	unsigned int out_len;
	int head_len;
	int head_total_len = 0;
	
	//uint8_t *point;
	uint8_t temp_len[4];
	unsigned int len = 1;
	//unsigned int stream_len;
	int block_num = para.width*para.height/para.block_height/para.block_width;

	if(para.stream_reader == nullptr)
	{
		para.stream_reader = fopen(para.stream_file_name,"rb");


		//uint8_t* out = (uint8_t*)malloc(40);
		//fread(out,1,40,para.stream_reader);
		//cout<<endl;
		//for(int i=0;i<40;++i)
		//{
		//	cout<<(int)out[i]<<" ";
		//}
		//cout<<endl;
		
		//fclose(para.stream_reader);

		//para.stream_reader = fopen(para.stream_file_name,"rb");
	}


	fread(temp_len,1,4,para.stream_reader);
	fromch4(head_total_len,temp_len);
	tmp_head = (uint8_t *)malloc(head_total_len);
	fread(tmp_head,1,head_total_len,para.stream_reader);

	huffman_decode_memory(tmp_head,head_total_len,&tmp_head_t,&out_len);

	point = tmp_head_t;

	for(int i = 0;i<para.block_num;++i)
	{
		//fread(temp_len,1,4,para.stream_reader);
		fromch4(len,point);
		//tmp_head = (uint8_t *)malloc(len);
		//fread(tmp_head,1,len,para.stream_reader);

		//fread(&temp_len,1,4,para.stream_reader);
		point += 4;
		block_stream2head(para, point, Ylist[i], len,&head_len);
		point += head_len;

		//free(tmp_head);
	}

	fread(stream_len,1,4,para.stream_reader);
	fromch4(len,stream_len);
	//tmp_stream = (uint8_t *)malloc(len);
	fread(stream_buff,1,len,para.stream_reader);


	entropy_decode_slice(Ylist.data(),block_num,para,stream_buff,len);
	//free(tmp_stream);

	fread(temp_len,1,4,para.stream_reader);
	fromch4(head_total_len,temp_len);
	tmp_head = (uint8_t *)malloc(head_total_len);
	fread(tmp_head,1,head_total_len,para.stream_reader);

	huffman_decode_memory(tmp_head,head_total_len,&tmp_head_t,&out_len);

	point = tmp_head_t;

	for(int i = 0;i<para.block_num;++i)
	{
		fromch4(len,point);

		point += 4;
		block_stream2head(para, point, Ulist[i], len,&head_len);
		point += head_len;
	}

	fread(stream_len,1,4,para.stream_reader);
	fromch4(len,stream_len);
	//tmp_stream = (uint8_t *)malloc(len);
	fread(stream_buff,1,len,para.stream_reader);

	entropy_decode_slice(Ulist.data(),block_num,para,stream_buff,len);
	//free(tmp_stream);

	fread(temp_len,1,4,para.stream_reader);
	fromch4(head_total_len,temp_len);
	tmp_head = (uint8_t *)malloc(head_total_len);
	fread(tmp_head,1,head_total_len,para.stream_reader);

	huffman_decode_memory(tmp_head,head_total_len,&tmp_head_t,&out_len);

	point = tmp_head_t;

	for(int i = 0;i<para.block_num;++i)
	{
		fromch4(len,point);

		point += 4;
		block_stream2head(para, point, Vlist[i], len,&head_len);
		point += head_len;
	}

	fread(stream_len,1,4,para.stream_reader);
	fromch4(len,stream_len);
	//tmp_stream = (uint8_t *)malloc(len);
	fread(stream_buff,1,len,para.stream_reader);

	entropy_decode_slice(Vlist.data(),block_num,para,stream_buff,len);
	//free(tmp_stream);
	*/
	return 0;
}


/*
* 将残差宏块的头文件按照一定顺序存储到流中
* 残差宏块头文件流存储顺序为：
* 头文件其他部分长度,block_id,type_slice,order,node数组的长度,node数组中的数据
*/
int PKT::block_head2stream(AVFormat& para,uint8_t** stream, ResidualBlock& rBlock, int* buff_len)
{


	*buff_len = rBlock.head_to_stream(* stream,para);

	/*
	unsigned int head_len = 0;
	head_len = sizeof(rBlock.block_id) + sizeof(rBlock.type_slice) + sizeof(rBlock.order); 
	head_len += sizeof(rBlock.node.size());
	if(rBlock.node.size() != 0)
		head_len += sizeof(rBlock.node[0]) * rBlock.node.size();

	*stream = (uint8_t *)malloc(sizeof(head_len) + head_len);

	uint8_t *point = *stream + sizeof(head_len);

	uint8_t temp[4];


	//写入block_id
	toch4(rBlock.block_id, temp);
	memcpy(point,temp,sizeof(rBlock.block_id));
	point += sizeof(rBlock.block_id);

	//写入type_slice
	toch4(rBlock.type_slice, temp);
	memcpy(point,temp,sizeof(rBlock.type_slice));
	point += sizeof(rBlock.type_slice);

	//写入order
	toch4(rBlock.order, temp);
	memcpy(point,temp,sizeof(rBlock.order));
	point += sizeof(rBlock.order);

	//写入node长度及node数据
	toch4(rBlock.node.size(), temp);
	memcpy(point,temp,sizeof(rBlock.node.size()));
	point += sizeof(rBlock.node.size());



	for(int i=0;i<rBlock.node.size();++i)
	{
		toch4(rBlock.node[i], temp);
		memcpy(point,temp,sizeof(rBlock.node[i]));
		point += sizeof(rBlock.node[i]);
	}


	*buff_len = head_len + sizeof(head_len);

	point = *stream;
	toch4(head_len, temp);
	memcpy(point,temp,sizeof(head_len));
	*/

	return 0;
}

int PKT::block_stream2head(AVFormat& para, uint8_t* stream, ResidualBlock& rBlock, int buff_len, int *head_length)
{
	*head_length = rBlock.head_from_stream(stream,para);
	/*
	unsigned int head_len = buff_len;
	uint8_t* point = stream;

	//fromch4(head_len,point);

	//point += sizeof(head_len);

	fromch4(rBlock.block_id, point);
	point += sizeof(rBlock.block_id);

	fromch4(rBlock.type_slice, point);
	point += sizeof(rBlock.type_slice);

	fromch4(rBlock.order, point);
	point += sizeof(rBlock.order);

	int node_len = 0;
	fromch4(node_len, point);
	point += sizeof(rBlock.node.size());

	rBlock.node.resize(node_len);

	for(int i = 0 ;i<node_len;++i)
	{
		int k;
		fromch4(k,point);
		rBlock.node[i] = k;
		point += sizeof(rBlock.node[i]);
	}

	*head_length = head_len;
	*/

	return 0;
}

void PKT::free_buff()	//gaowk 2017/8/10
{
	if(stream_buff != nullptr)
	{
		free(stream_buff);
		stream_buff = nullptr;
	}

	if(head_buff != nullptr)
	{
		free(head_buff);
		head_buff = nullptr;
	}
}