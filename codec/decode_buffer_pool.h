#pragma once


#include <list>
#include <vector>
#include <stdint.h>
#include "def.h"

class BlockBufferPool{
	std::vector<std::vector<uint8_t>> frame;
public:
	BlockBufferPool(int height,int width):frame(height,std::vector<uint8_t>(width,128)){}
	void setValue(int h,int w,int8_t value);
	int8_t getValue(int h,int w);
	int clear();
};

class FrameBufferPool{
	int max_size;
	std::list<BlockBufferPool> decoded_frame;

public:
	FrameBufferPool(int size):max_size(size){}
	int add_frame_to_pool(BlockBufferPool & frame);
};

