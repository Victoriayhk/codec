#pragma once


#include <list>
#include <vector>
#include <stdint.h>
#include "def.h"
#include "util.h"


 /**
 *  �������ݻ��棬������һ�������һ֡��Ԥ��
 *  �Ң
 **/
class BlockBufferPool{
	/**
	*  һ֡����
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
 *     ʹ��ѭ�����нṹ�洢֡��
 **/
typedef CyclePool<BlockBufferPool> FrameBufferPool;

