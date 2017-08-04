
#include "ResidualBlock.h"



ResidualBlock::ResidualBlock(Block::BlockType type,int height , int width):tree(0,0,height,width),block_type(type),data(width * height){}

ResidualBlock::ResidualBlock():tree(0,0,0,0)
{
}

ResidualBlock::ResidualBlock(int blocktype):
	tree(0,0,0,0)
{
	block_type = (Block::BlockType)blocktype;
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
	//p += save_to_buffer(block_id, p);
	//p += save_to_buffer(order, p);
	//p += save_to_buffer(block_type, p);

	short *pdata = data.data(); 


	p += save_to_buffer(pdata, p,data.size() * sizeof(data[0]));
	return p - stream;
}


/*
* 从流中还原出ResidualBlock
*/
int ResidualBlock::from_stream(unsigned char *stream, int block_size) {
	unsigned char *p = stream;
	//p += get_from_buffer(block_id, p);
	//p += get_from_buffer(order, p);
	//p += get_from_buffer(block_type, p);

	data.resize(block_size);
	short *pdata = data.data();
	p += get_from_buffer(pdata, p, block_size * sizeof(data[0]));
	return p - stream;
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
