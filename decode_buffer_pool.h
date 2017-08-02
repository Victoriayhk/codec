#pragma once

#include "stdafx.h"
#include <list>
#include "virtural.h"



struct FrameBufferPool{
	int max_size;
	std::list<Frame *> decoded_frame;
	FrameBufferPool(int size):max_size(size){}
};

struct BlockBufferPool{
	Frame * blocks;
};

int add_frame_to_pool(FrameBufferPool * pool,BlockBufferPool * frame);

int add_block_to_pool(BlockBufferPool * block_pool,int h,int w,Block * block);
