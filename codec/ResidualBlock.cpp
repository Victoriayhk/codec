
#include "ResidualBlock.h"
#include "huffman.h"
#include <time.h>
#include <iostream>

using namespace std;

#define BASE_MALLOC_SIZE 360000

uint8_t* PKT::stream_buff = nullptr;
uint8_t* PKT::head_buff = nullptr;

ResidualBlock::ResidualBlock(Block::BlockType type,int height , int width):tree(0,0,height-1,width-1),block_type(type){
	data.clear();
	data.resize(width*height);
}


ResidualBlock::ResidualBlock():tree(0,0,0,0)
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
Node * ResidualBlock::get_node(){
	return &node_list[curr_node++];
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

Tree::Tree(int left_top_h,int left_top_w,int right_bottom_h,int right_bottom_w):data(nullptr),
				split_direction(NONE),left(nullptr),right(nullptr),
				left_top_h(left_top_h),left_top_w(left_top_w),right_bottom_h(right_bottom_h),right_bottom_w(right_bottom_w){
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
* 将ResidualBlock写入流
* 流stream需要预先开辟空间
*/
int ResidualBlock::to_stream(unsigned char *stream) {
	unsigned char *p = stream;
	p += save_to_buffer(block_id, p);
	p += save_to_buffer(order, p);
	p += save_to_buffer(block_type, p);
	if(!is_tree){
		p += save_to_buffer(type_slice, p);
		if(type_slice==0)
			p += save_to_buffer(node[0], p);
		if(type_slice==1){
			for(int i=0;i<4;++i)
				p += save_to_buffer(node[i], p);
		}
	}else{
	
	}


	short *pdata = data.data(); 

	p += save_to_buffer(pdata, p,data.size() * sizeof(data[0]));
	return p - stream;
}


/*
* 从流中还原出ResidualBlock
*/
int ResidualBlock::from_stream(unsigned char *stream, int block_size) {
	unsigned char *p = stream;
	p += get_from_buffer(block_id, p);
	p += get_from_buffer(order, p);
	p += get_from_buffer(block_type, p);
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
	data.resize(block_size);
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
	node.clear();
	//tree.clear();
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
int PKT::to_stream(unsigned char *stream) {
	unsigned char *p = stream;
	for (int i = 0; i < (int) Ylist.size(); i++) {
		p += Ylist[i].to_stream(p);
	}
	for (int i = 0; i < (int) Ulist.size(); i++) {
		p += Ulist[i].to_stream(p);
	}
	for (int i = 0; i < (int) Vlist.size(); i++) {
		p += Vlist[i].to_stream(p);
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
		p += Ylist[i].from_stream(p, yblock_size);
	}

	for (int i = 0; i < (int) Ulist.size(); i++) {
		p += Ulist[i].from_stream(p, yblock_size / 4);
	}
	for (int i = 0; i < (int) Vlist.size(); i++) {
		p += Vlist[i].from_stream(p, yblock_size / 4);
	}
	return p - stream;
}

/*
* 将PKT中的数据写成流使用熵编码后进行文件写入
*/
int PKT::stream_write(AVFormat& para)
{
	//对一帧的Y数据进行熵编码
	//uint8_t *stream;
	uint8_t *tmp_stream;
	uint8_t *tmp_head_t = (uint8_t*)malloc(sizeof(uint8_t) * BASE_MALLOC_SIZE);
	uint8_t *tmp_head;
	uint8_t *head_point = tmp_head_t;
	uint8_t *head_out = nullptr;
	unsigned int head_out_len = 0;
	uint8_t head_len_ch[4];
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

	//if(PKT::stream_buff == nullptr)
	//{
	//	PKT::stream_buff = (uint8_t*)malloc(10000);
	//}

	huffman_encode_memory(tmp_head_t,head_len,&head_out,&head_out_len);	//2017/8/9 gaowk
	toch4(head_out_len,head_len_ch);
	//cout<<endl;

	//cout<<len<<" ";

	//stream = (uint8_t *)malloc(len);
	//point = stream;
	//memcpy(point,tmp_stream,len);

	if(para.stream_writer == nullptr)
	{
		para.stream_writer = fopen(para.stream_file_name,"wb");
	}


	//for(int i=0;i<40;++i)
	//{
	//	cout<<(int)tmp_head_t[i]<<" ";
	//}
	//cout<<endl;
	fwrite(head_len_ch,sizeof(uint8_t),4,para.stream_writer);
	fwrite(head_out,sizeof(uint8_t),head_out_len,para.stream_writer);
	//fwrite(tmp_head_t,sizeof(uint8_t),head_len,para.stream_writer);
	fwrite(tmp_stream,sizeof(uint8_t),len,para.stream_writer);

	free(tmp_head_t);
	free(tmp_stream);
	stream_len = len + head_out_len + 2;
	
	//对一帧的U数据进行熵编码
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

	huffman_encode_memory(tmp_head_t,head_len,&head_out,&head_out_len);	//2017/8/9 gaowk
	toch4(head_out_len,head_len_ch);

	//cout<<endl;
	//cout<<len<<" ";
	//stream = (uint8_t *)realloc(stream,stream_len + len);
	//point = stream + stream_len;
	//memcpy(point,tmp_stream,len);

	fwrite(head_len_ch,sizeof(uint8_t),4,para.stream_writer);
	fwrite(head_out,sizeof(uint8_t),head_out_len,para.stream_writer);
	//fwrite(tmp_head_t,sizeof(uint8_t),head_len,para.stream_writer);
	fwrite(tmp_stream,sizeof(uint8_t),len,para.stream_writer);

	free(tmp_head_t);
	free(tmp_stream);
	stream_len = len + head_out_len + 2;
	
	//对一帧的V数据进行熵编码
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
	huffman_encode_memory(tmp_head_t,head_len,&head_out,&head_out_len);	//2017/8/9 gaowk
	toch4(head_out_len,head_len_ch);
	//cout<<endl;
	//cout<<len<<" "<<endl;
	//stream = (uint8_t *)realloc(stream,stream_len + len);
	//point = stream + stream_len;
	//memcpy(point,tmp_stream,len);

	fwrite(head_len_ch,sizeof(uint8_t),4,para.stream_writer);
	fwrite(head_out,sizeof(uint8_t),head_out_len,para.stream_writer);
	//fwrite(tmp_head_t,sizeof(uint8_t),head_len,para.stream_writer);
	fwrite(tmp_stream,sizeof(uint8_t),len,para.stream_writer);

	free(tmp_head_t);
	free(tmp_stream);
	//stream_len += len + head_len;
	stream_len = len + head_out_len + 2;

	return 0;
}

/*
* 将流文件进行反熵编码后读入PKT
*/
int PKT::stream_read(AVFormat& para)
{
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

	return 0;
}


/*
* 将残差宏块的头文件按照一定顺序存储到流中
* 残差宏块头文件流存储顺序为：
* 头文件其他部分长度,block_id,type_slice,order,node数组的长度,node数组中的数据
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
		int k;
		fromch4(k,point);
		rBlock.node[i] = k;
		point += sizeof(rBlock.node[i]);
	}

	*head_length = head_len;

	return 0;
}
