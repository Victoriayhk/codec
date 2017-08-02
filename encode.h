#ifndef ENCODE_H
#define ENCODE_H


#include "stdafx.h"
#include "decode_buffer_pool.h"

typedef Block ModeAndDiff;


int write_frames_to_file(Frame *pframe, FILE *fout);
int yuv_read(FILE *fin, Frame &frame);;

int predict_block_inter(Block * block, ModeAndDiff * mode_result, BlockBufferPool * pool,double * score);

int predict_block_intra(Block * block, ModeAndDiff * mode_result, FrameBufferPool * pool,double * score);


int encode(Frame* frame,AVFormat* para,pkt* pkt);

int block_decode_save(Block* blk,queue<Block>* que);

#endif