#ifndef ENCODE_H
#define ENCODE_H


#include "ResidualBlock.h"
#include "decode_buffer_pool.h"


int encode(Frame &frame,AVFormat &para,PKT &pkt,vector<FrameBufferPool*>  &frame_pool);

int block_decode_save(Block &blk,BlockBufferPool &blockbuffer,FrameBufferPool& framebuffer);


int dct_trans(ResidualBlock & residual_block,int left_row,int left_col,int right_row,int right_col,int h,int w);

int reverse_dct_trans(ResidualBlock & residual_block,int left_row,int left_col,int right_row,int right_col,int h,int w);

#endif