
extern int TABLE[1500][1500];
#include "decode_buffer_pool.h"

//int FrameBufferPool::add_frame_to_pool(BlockBufferPool * frame)
//{
//	if(decoded_frame.size() == max_size){
//		auto * tmp = decoded_frame.front();
//		decoded_frame.pop_front();
//		delete tmp;
//	}
//	decoded_frame.push_back(frame);
//
//	return 0;
//}

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

//void BlockBufferPool::set_block(Block & block,int tlh, int tlw, int brh, int brw,AVFormat &para){
//	//TO DO
//	int block_w, block_h;
//	block.getBlockSize(para, block_w, block_h);
//	int i_offset = (block.block_id / para.block_num_per_row) * block_h;	// 当前block的起始像素所在Frame的行
//	int j_offset = (block.block_id % para.block_num_per_row) * block_w;
//}


int BlockBufferPool::clear(){
	frame.clear();
	return 0;
}