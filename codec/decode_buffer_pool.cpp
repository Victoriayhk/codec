

#include "decode_buffer_pool.h"

int FrameBufferPool::add_frame_to_pool(BlockBufferPool * frame)
{
	if(decoded_frame.size() == max_size){
		auto * tmp = decoded_frame.front();
		decoded_frame.pop_front();
		delete tmp;
	}
	decoded_frame.push_back(frame);

	return 0;
}

void BlockBufferPool::setValue(int h,int w,int8_t value){
	frame[h * width + w] = value;
}
int8_t BlockBufferPool::getValue(int h,int w){
	return frame[h * width + w];
}


int BlockBufferPool::clear(){
	frame.clear();
	return 0;
}