#include "def.h"

AVFormat::AVFormat():
	video(nullptr),
	stream_writer(nullptr),
	stream_reader(nullptr)
{
}

AVFormat::~AVFormat()
{
}

void AVFormat::getBlockSize(Block::BlockType  block_type, int& height, int& width)
{
	if(block_type == Block::Y)
	{
		height = block_height;
		width = block_width;
	}
	else
	{
		height = block_height/2;
		width = block_width/2;
	}
}

Block::Block()
{
}

Block::Block(int h , int w):
		data(h*w)
{
	//	data = new uint8_t[h*w];
}

Block::~Block()
{
}

void Block::print(AVFormat &para){
	int h;
	int w;
	getBlockSize(para,h,w);
	for(int i = 0; i < h; ++i){
		for(int j = 0; j < w; ++j){
			printf("%d ",data[i * w + j]);
		}
		printf("\n");
	}
};

void Block::getBlockSize(AVFormat &para, int& height, int& width)
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

Block::Block(Block::BlockType type,int height , int width):block_type(type),data(width * height){}

Frame::Frame()
{
}

Frame::~Frame()
{
}
int Frame::init(AVFormat& para){
	int h,w;
	para.getBlockSize(Block::Y,h,w);
	Yblock.resize(para.block_num,Block(Block::Y,h,w));
	para.getBlockSize(Block::U,h,w);
	Ublock.resize(para.block_num,Block(Block::U,h,w));
	para.getBlockSize(Block::V,h,w);
	Vblock.resize(para.block_num,Block(Block::V,h,w));

	return 0;
}
