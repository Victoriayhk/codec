#pragma once

#include "def.h"
#include "decode_buffer_pool.h"
#include "ResidualBlock.h"


int predict_block_inter(Block &block, ResidualBlock  &residual_block, BlockBufferPool &pool,double &score);

int predict_block_intra(Block &block, ResidualBlock  &residual_block, FrameBufferPool &pool,double &score);



int predict(Block &block,ResidualBlock  &residual_block,Tree &tree,BlockBufferPool & block_buffer_pool,FrameBufferPool &frame_pool,Block & buffer_block,AVFormat & para,double & min_score);
double intra_predict(Block &block,ResidualBlock  &residual_block,Tree &tree,BlockBufferPool & block_buffer_pool,Block & buffer_block,AVFormat & para,double & min_score);
double inter_predict(Block &block,ResidualBlock  &residual_block,Tree &tree,FrameBufferPool &frame_pool,Block & buffer_block,AVFormat & para,double & min_score);
double search_predict_pattern(Block &block,ResidualBlock  &residual_block,Tree &tree,BlockBufferPool & block_buffer_pool,FrameBufferPool &frame_pool,Block & buffer_block,AVFormat & para);
int re_predict(Block &block,ResidualBlock  &residual_block,Tree &tree,BlockBufferPool & block_buffer_pool,FrameBufferPool &frame_pool,Block & buffer_block,AVFormat & para);