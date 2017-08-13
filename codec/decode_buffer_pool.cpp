
extern int TABLE[1500][1500];
#include "decode_buffer_pool.h"


 /**
 *   解码数据缓存的简单操作
 **/
void BlockBufferPool::setValue(int h,int w,uint8_t value){
	frame[TABLE[h][width] + w] = value;
}
int BlockBufferPool::get_height(){
	return height;
}
int BlockBufferPool::get_width(){
	return width;
}
BlockBufferPool::BlockBufferPool(const BlockBufferPool & pool){
	frame = pool.frame;
	height = pool.height;
	width = pool.width;
}
uint8_t BlockBufferPool::getValue(int h,int w){
	return frame[TABLE[h][width] + w];
}

int BlockBufferPool::clear(){
	frame.clear();
	return 0;
}