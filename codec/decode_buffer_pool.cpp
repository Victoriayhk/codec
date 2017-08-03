

#include "decode_buffer_pool.h"

int FrameBufferPool::add_frame_to_pool(BlockBufferPool &frame)
{
	if(decoded_frame.size() == max_size){
		decoded_frame.pop_front();
	}
	decoded_frame.push_back(std::move(frame));

	return 0;
}


int BlockBufferPool::add_block_to_pool(Block & block)
{
	frame.push_back(block);
	return 0;
}

int BlockBufferPool::clear(){
	frame.clear();
	return 0;
}