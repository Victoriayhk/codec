#pragma once


#include <list>
#include <vector>
#include <stdint.h>
#include "def.h"
#include "util.h"


 /**
 *  解码数据缓存，用于下一块或者下一帧的预测
 *  李春尧
 **/
class BlockBufferPool{
	/**
	*  一帧数据
	*/
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
	void setValue(int h,int w,uint8_t value);
	uint8_t getValue(int h,int w);
	int clear();
};

 /**
 *     使用循环队列结构存储帧池
 **/
typedef CyclePool<BlockBufferPool> FrameBufferPool;

