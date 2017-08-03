#include "def.h"

AVFormat::AVFormat():
	video(nullptr)
{
}

AVFormat::~AVFormat()
{
}

void AVFormat::getBlockSize(Block block, int& height, int& width)
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

Frame::Frame()
{
}

Frame::~Frame()
{
}

