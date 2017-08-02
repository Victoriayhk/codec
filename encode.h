#ifndef ENCODE_H
#define ENCODE_H


#include "stdafx.h"
#include "decode_buffer_pool.h"

typedef Block ModeAndDiff;

int predict_block_inter(Block * block, ModeAndDiff * mode_result, BlockBufferPool * pool,double * score);

int predict_block_intra(Block * block, ModeAndDiff * mode_result, FrameBufferPool * pool,double * score);


int encode(Frame* frame,av_encode_para* para,pkt* pkt);

int block_decode_save(block* blk,queue<block>* que);

#endif