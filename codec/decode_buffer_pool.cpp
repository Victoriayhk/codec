

#include "decode_buffer_pool.h"

int FrameBufferPool::add_frame_to_pool(BlockBufferPool &frame)
{
	if(decoded_frame.size() == max_size){
		decoded_frame.pop_front();
	}
	decoded_frame.push_back(std::move(frame));

	return 0;
}

void BlockBufferPool::setValue(int h,int w,int8_t value){
	frame[h][w] = value;
}
int8_t BlockBufferPool::getValue(int h,int w){
	return frame[h][w];
}


int BlockBufferPool::clear(){
	frame.clear();
	return 0;
}