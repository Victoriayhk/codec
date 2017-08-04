#include "def.h"

AVFormat::AVFormat():
	video(nullptr)
{
}

AVFormat::~AVFormat()
{
}

void AVFormat::getBlockSize(Block & block, int& height, int& width)
{
	if(block.block_type == block.Y)
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


Frame::Frame()
{
}

Frame::~Frame()
{
}

