#ifndef ENCODE_H
#define ENCODE_H


#include "ResidualBlock.h"
#include "decode_buffer_pool.h"


int encode(Frame &frame,AVFormat &para,PKT &pkt,vector<FrameBufferPool>  &frame_pool);

int block_decode_save(Block &blk,BlockBufferPool &blockbuffer,FrameBufferPool& framebuffer);

#endif