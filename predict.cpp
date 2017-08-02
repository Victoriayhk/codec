#pragma once


#include "decode_buffer_pool.h"
#include "predict.h"



int predict_block_inter(Block &block, ModeAndDiff  &mode_result, BlockBufferPool &pool,double &score){
	mode_result.data = block.data;
	score = 0;
	return 0;
}

int predict_block_intra(Block &block, ModeAndDiff  &mode_result, FrameBufferPool &pool,double &score){
	mode_result.data = block.data;
	score = 0;
	return 0;
}