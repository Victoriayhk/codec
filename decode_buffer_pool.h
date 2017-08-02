#pragma once


#include <list>
#include "virtural.h"
#include <vector>
#include "def.h"

class BlockBufferPool{
	std::vector<Block> frame;
public:
	BlockBufferPool(){}
	int add_block_to_pool(Block & block);
};

class FrameBufferPool{
	int max_size;
	std::list<BlockBufferPool> decoded_frame;

public:
	FrameBufferPool(int size):max_size(size){}
	int add_frame_to_pool(BlockBufferPool & frame);
};

