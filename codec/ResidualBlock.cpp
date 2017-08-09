
#include "ResidualBlock.h"
#include "huffman.h"
#include <time.h>
#include <iostream>
#include <queue>

using namespace std;

#define BASE_MALLOC_SIZE 360000

uint8_t* PKT::stream_buff = nullptr;

ResidualBlock::ResidualBlock(Block::BlockType type,int height , int width):tree(0,0,height-1,width-1),block_type(type),curr_node(0){
	data.clear();
	data.resize(width*height);
}



ResidualBlock::ResidualBlock():tree(0,0,0,0),curr_node(0)
{
}

ResidualBlock::ResidualBlock(int blocktype):
	tree(0,0,0,0),curr_node(0)
{
	block_type = (Block::BlockType)blocktype;
}
ResidualBlock::ResidualBlock(const Block & block):tree(0,0,0,0),curr_node(0),data(block.data.size()){
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




	
int Node::to_stream(unsigned char * stream){

	*(stream) = (0x7F & prediction);
	int byte = 1;
	if(pre_type == INTRA_PREDICTION){
		*(stream) |= (unsigned char)0x00;
	}else{
		*(stream) |= (unsigned char)0x80;

		*(stream +1 ) = mv[0];
		
		*(stream + 2) = mv[1];

		byte += 2;

	}

	return byte;
}

int Node::from_stream(unsigned char * stream){
	int need_byte = 0;
	prediction = *(stream + need_byte) & (unsigned char)0x7F;
	unsigned char type = *(stream + need_byte) & (unsigned char)0x80;

	if(type == (unsigned char)0x00){
		pre_type = INTRA_PREDICTION;
	}else{
		pre_type = INTER_PREDICTION;
		++need_byte;

		mv[0] = *(stream + need_byte);
		++need_byte;
		mv[1] = *(stream + need_byte);
		++need_byte;
	}
	return need_byte;

}
Tree::Tree(int left_top_h,int left_top_w,int right_bottom_h,int right_bottom_w):node_id(-1),
				split_direction(NONE),left(nullptr),right(nullptr),
				left_top_h(left_top_h),left_top_w(left_top_w),right_bottom_h(right_bottom_h),right_bottom_w(right_bottom_w){
}
void Tree::serialize(unsigned char * stream,int &byte,int &bit,uint8_t * used_node_ids,int &idx){
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

void Tree::deserialize(unsigned char * stream,int &byte,int &bit,int &idx){
	unsigned char * tmp = stream + byte;
	unsigned char type = (*tmp >> bit) & (unsigned char)0x03;
	int w = right_bottom_w - left_top_w + 1;
	int h = left_top_h - right_bottom_h + 1;
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
		idx += 1;
	}else{
		if(type == 0x01){
			this->split_direction = HORIZONTAL;
			this->left = new Tree(left_top_h,left_top_w,right_bottom_h,right_bottom_w - (w/2));
			this->right = new Tree(left_top_h,left_top_w + (w/2),right_bottom_h,right_bottom_w);
		}else{
			this->split_direction = VERTICAL;
			this->left = new Tree(left_top_h,left_top_w,right_bottom_h - (h/2),right_bottom_w);
			this->right = new Tree(left_top_h + (h/2),left_top_w,right_bottom_h,right_bottom_w);
		}
		bit += 2;
		if(bit == 8){
			bit = 0;
			++byte;
		}
		this->node_id = -1;
		this->left->deserialize(stream,byte,bit,idx);
		this->right->deserialize(stream,byte,bit,idx);
	}
	
}


int Tree::to_stream(unsigned char * stream,uint8_t * used_node_ids,int &num){
	int byte = 0,bit = 0,idx=0;
	this->serialize(stream,byte,bit,used_node_ids,idx);
	num = idx;

	return byte;
}


int Tree::from_stream(unsigned char * stream,int &num){
	int byte = 0,bit = 0,idx=0;
	this->deserialize(stream,byte,bit,idx);
	num = idx;

	return byte;
}

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


/*
* ��ResidualBlockд����
* ��stream��ҪԤ�ȿ��ٿռ�
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

		p += tree.to_stream(p,used_node_ids,curr_node);
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
* �����л�ԭ��ResidualBlock
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
		tree.right_bottom_h = para.block_height - 1;
		tree.right_bottom_w = para.block_width - 1;
		p += tree.from_stream(p,num);

		for(int i = 0; i < num; ++i){
			p += node_list[curr_node ++].from_stream(p);
		}
	}

	//if(data.size() != block_size){
	data.resize(block_size);
	//}
	short *pdata = data.data();
	p += get_from_buffer(pdata, p, block_size * sizeof(data[0]));
	return p - stream;
}

/*
* ���ResidualBlock,���㸴��
*/
int ResidualBlock::clear()
{
	//data.clear();
	node.clear();
	//tree.clear();
	return 0;
}

/**
* ΪPKTԤ����ռ�
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
* ��PKTд����
* ��stream��ҪԤ�ȿ��ٿռ�
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
* �����л�ԭ��PKT
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

/*
* ��PKT�е�����д����ʹ���ر��������ļ�д��
*/
int PKT::stream_write(AVFormat& para)
{
	//��һ֡��Y���ݽ����ر���
	//uint8_t *stream;
	uint8_t *tmp_stream;
	uint8_t *tmp_head_t = (uint8_t*)malloc(sizeof(uint8_t) * BASE_MALLOC_SIZE);
	uint8_t *tmp_head;
	uint8_t *head_point = tmp_head_t;
	int one_head_len;
	int head_N = 1;
	int head_len = 0;
	//uint8_t *point;
	unsigned int len;
	unsigned int stream_len;
	int block_num = para.width*para.height/para.block_height/para.block_width;

	//int start_time=clock();
	entropy_encode_slice(Ylist.data(),block_num,para,&tmp_stream,&len);
	//int end_time=clock();

	//std::cout<< "Stream wirte Running time is: "<<static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;	

	for(int i = 0;i<Ylist.size();++i)
	{
		block_head2stream(para,&tmp_head,Ylist[i],&one_head_len);

		//cout<<one_head_len<<" ";
		
		if(one_head_len + head_len > head_N * BASE_MALLOC_SIZE)
		{
			tmp_head_t = (uint8_t *)realloc(tmp_head_t, ++head_N);
			head_point = tmp_head + head_len;
		}

		memcpy(head_point,tmp_head,one_head_len);
	
		head_point += one_head_len;
		head_len += one_head_len;
	}
	//cout<<endl;

	//cout<<len<<" ";

	//stream = (uint8_t *)malloc(len);
	//point = stream;
	//memcpy(point,tmp_stream,len);
	stream_len = len + head_len;

	if(para.stream_writer == nullptr)
	{
		para.stream_writer = fopen(para.stream_file_name,"wb");
	}


	//for(int i=0;i<40;++i)
	//{
	//	cout<<(int)tmp_head_t[i]<<" ";
	//}
	//cout<<endl;

	fwrite(tmp_head_t,sizeof(uint8_t),head_len,para.stream_writer);
	fwrite(tmp_stream,sizeof(uint8_t),len,para.stream_writer);

	free(tmp_head_t);
	free(tmp_stream);
	
	//��һ֡��U���ݽ����ر���
	entropy_encode_slice(Ulist.data(),block_num,para,&tmp_stream,&len);

	//for(int i = 0 ; i < 4 ; ++i)
	//{
	//	cout<<(int)tmp_stream[i]<<" ";
	//}
	//cout<<endl;
	//int x;
	//fromch4(x,tmp_stream);

	tmp_head_t = (uint8_t*)malloc(sizeof(uint8_t) * BASE_MALLOC_SIZE);
	head_point = tmp_head_t;
	head_len = 0;
	head_N = 1;

	for(int i = 0;i<Ulist.size();++i)
	{
		block_head2stream(para,&tmp_head,Ulist[i],&one_head_len);
		//cout<<one_head_len<<" ";
		if(one_head_len + head_len > head_N * BASE_MALLOC_SIZE)
		{
			tmp_head_t = (uint8_t *)realloc(tmp_head_t, ++head_N);
			head_point = tmp_head + head_len;
		}

		memcpy(head_point,tmp_head,one_head_len);
	
		head_point += one_head_len;
		head_len += one_head_len;
	}
	//cout<<endl;
	//cout<<len<<" ";
	//stream = (uint8_t *)realloc(stream,stream_len + len);
	//point = stream + stream_len;
	//memcpy(point,tmp_stream,len);

	fwrite(tmp_head_t,sizeof(uint8_t),head_len,para.stream_writer);
	fwrite(tmp_stream,sizeof(uint8_t),len,para.stream_writer);

	free(tmp_head_t);
	free(tmp_stream);
	stream_len += len + head_len;
	
	//��һ֡��V���ݽ����ر���
	entropy_encode_slice(Vlist.data(),block_num,para,&tmp_stream,&len);

	tmp_head_t = (uint8_t*)malloc(sizeof(uint8_t) * BASE_MALLOC_SIZE);
	head_point = tmp_head_t;
	head_len = 0;
	head_N = 1;

	for(int i = 0;i<Vlist.size();++i)
	{
		block_head2stream(para,&tmp_head,Vlist[i],&one_head_len);
		//cout<<one_head_len<<" ";
		if(one_head_len + head_len > head_N * BASE_MALLOC_SIZE)
		{
			tmp_head_t = (uint8_t *)realloc(tmp_head_t, ++head_N);
			head_point = tmp_head + head_len;
		}

		memcpy(head_point,tmp_head,one_head_len);
	
		head_point += one_head_len;
		head_len += one_head_len;
	}
	//cout<<endl;
	//cout<<len<<" "<<endl;
	//stream = (uint8_t *)realloc(stream,stream_len + len);
	//point = stream + stream_len;
	//memcpy(point,tmp_stream,len);

	fwrite(tmp_head_t,sizeof(uint8_t),head_len,para.stream_writer);
	fwrite(tmp_stream,sizeof(uint8_t),len,para.stream_writer);

	free(tmp_head_t);
	free(tmp_stream);
	stream_len += len + head_len;

	return 0;
}

/*
* �����ļ����з��ر�������PKT
*/
int PKT::stream_read(AVFormat& para)
{
	if(stream_buff == nullptr)
		stream_buff = (uint8_t*)malloc(1000000);
	uint8_t *tmp_head;
	uint8_t stream_len[4];

	int head_len;
	
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

	for(int i = 0;i<para.block_num;++i)
	{
		fread(temp_len,1,4,para.stream_reader);
		fromch4(len,temp_len);
		tmp_head = (uint8_t *)malloc(len);
		fread(tmp_head,1,len,para.stream_reader);

		//fread(&temp_len,1,4,para.stream_reader);

		block_stream2head(para, tmp_head, Ylist[i], len,&head_len);

		free(tmp_head);
	}

	fread(stream_len,1,4,para.stream_reader);
	fromch4(len,stream_len);
	//tmp_stream = (uint8_t *)malloc(len);
	fread(stream_buff,1,len,para.stream_reader);


	entropy_decode_slice(Ylist.data(),block_num,para,stream_buff,len);
	//free(tmp_stream);

	for(int i = 0;i<para.block_num;++i)
	{
		fread(temp_len,1,4,para.stream_reader);
		fromch4(len,temp_len);
		tmp_head = (uint8_t *)malloc(len);
		fread(tmp_head,1,len,para.stream_reader);

		//fread(&temp_len,1,4,para.stream_reader);

		block_stream2head(para, tmp_head, Ulist[i], len,&head_len);

		free(tmp_head);
	}

	fread(stream_len,1,4,para.stream_reader);
	fromch4(len,stream_len);
	//tmp_stream = (uint8_t *)malloc(len);
	fread(stream_buff,1,len,para.stream_reader);

	entropy_decode_slice(Ulist.data(),block_num,para,stream_buff,len);
	//free(tmp_stream);

	for(int i = 0;i<para.block_num;++i)
	{
		fread(&temp_len,1,4,para.stream_reader);
		fromch4(len,temp_len);
		tmp_head = (uint8_t *)malloc(len);
		fread(tmp_head,1,len,para.stream_reader);

		//fread(&temp_len,1,4,para.stream_reader);

		block_stream2head(para, tmp_head, Vlist[i], len,&head_len);

		free(tmp_head);
	}

	fread(stream_len,1,4,para.stream_reader);
	fromch4(len,stream_len);
	//tmp_stream = (uint8_t *)malloc(len);
	fread(stream_buff,1,len,para.stream_reader);

	entropy_decode_slice(Vlist.data(),block_num,para,stream_buff,len);
	//free(tmp_stream);

	return 0;
}


/*
* ���в����ͷ�ļ�����һ��˳��洢������
* �в���ͷ�ļ����洢˳��Ϊ��
* ͷ�ļ��������ֳ���,block_id,type_slice,order,node����ĳ���,node�����е�����
*/
int PKT::block_head2stream(AVFormat& para,uint8_t** stream, ResidualBlock& rBlock, int* buff_len)
{
	unsigned int head_len = 0;
	head_len = sizeof(rBlock.block_id) + sizeof(rBlock.type_slice) + sizeof(rBlock.order); 
	head_len += sizeof(rBlock.node.size());
	if(rBlock.node.size() != 0)
		head_len += sizeof(rBlock.node[0]) * rBlock.node.size();

	*stream = (uint8_t *)malloc(sizeof(head_len) + head_len);

	uint8_t *point = *stream + sizeof(head_len);

	uint8_t temp[4];

	//д��block_id
	toch4(rBlock.block_id, temp);
	memcpy(point,temp,sizeof(rBlock.block_id));
	point += sizeof(rBlock.block_id);

	//д��type_slice
	toch4(rBlock.type_slice, temp);
	memcpy(point,temp,sizeof(rBlock.type_slice));
	point += sizeof(rBlock.type_slice);

	//д��order
	toch4(rBlock.order, temp);
	memcpy(point,temp,sizeof(rBlock.order));
	point += sizeof(rBlock.order);

	//д��node���ȼ�node����
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

	return 0;
}

int PKT::block_stream2head(AVFormat& para, uint8_t* stream, ResidualBlock& rBlock, int buff_len, int *head_length)
{
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
		fromch4(rBlock.node[i],point);
		point += sizeof(rBlock.node[i]);
	}

	*head_length = head_len;

	return 0;
}
