#pragma once


#include <list>
#include <vector>
#include <stdint.h>
#include "def.h"
#include "util.h"


class BlockBufferPool{
	std::vector<uint8_t> frame;
	int height;
	int width;
public:
	BlockBufferPool(){}
	BlockBufferPool(int height,int width):frame(height*width),height(height),width(width){}
	BlockBufferPool(const BlockBufferPool & BlockBufferPool);
	int get_height();
	int get_width();
	//void set_block(Block & block,int tlh, int tlw, int brh, int brw,AVFormat &para);
	void setValue(int h,int w,int8_t value);
	int8_t getValue(int h,int w);
	int clear();
};


typedef CyclePool<BlockBufferPool> FrameBufferPool;
//class FrameBufferPool{
//	int max_size;
//	std::list<BlockBufferPool * > decoded_frame;
//
//public:
//	FrameBufferPool(int size):max_size(size){}
//	int add_frame_to_pool(BlockBufferPool * frame);
//};

