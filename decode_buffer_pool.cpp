#include "stdafx.h"

#include "decode_buffer_pool.h"

int add_frame_to_pool(FrameBufferPool * pool,BlockBufferPool * frame)
{
	if(pool->decoded_frame.size() == pool->max_size){
		pool->decoded_frame.pop_front();
	}
	pool->decoded_frame.push_back(frame->blocks);

	return 0;
}



int add_block_to_pool(BlockBufferPool * block_pool,int h,int w,Block * block)
{
	//(block_pool->blocks)->data[h][w] = block;
	return 0;
}