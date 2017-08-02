#pragma once

#include "stdafx.h"
#include <list>
#include "virtural.h"
#include <vector>

class BlockBufferPool{
	std::vector<Block> frame;
public:
	BlockBufferPool(int size):frame(size){}
	int add_block_to_pool(int index,Block & block);
};

class FrameBufferPool{
	int max_size;
	std::list<BlockBufferPool> decoded_frame;

public:
	FrameBufferPool(int size):max_size(size){}
	int add_frame_to_pool(BlockBufferPool & frame);
};

