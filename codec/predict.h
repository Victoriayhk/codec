#pragma once

#include "def.h"
#include "decode_buffer_pool.h"
#include "ResidualBlock.h"


/**
*  预测逻辑 接口，具体注释看cpp
*  李春尧
*/
int predict_block_inter(Block &block, ResidualBlock  &residual_block, BlockBufferPool &pool,int &score);

int predict_block_intra(Block &block, ResidualBlock  &residual_block, FrameBufferPool &pool,int &score);

int predict(Block &block,ResidualBlock  &residual_block,Tree &tree,BlockBufferPool & block_buffer_pool,FrameBufferPool &frame_pool,Block & buffer_block,AVFormat & para,int & min_score,int,int);
int intra_predict(Block &block,ResidualBlock  &residual_block,Tree &tree,BlockBufferPool & block_buffer_pool,Block & buffer_block,AVFormat & para,int & min_score,int,int);
int inter_predict(Block &block,ResidualBlock  &residual_block,Tree &tree,FrameBufferPool &frame_pool,Block & buffer_block,AVFormat & para,int & min_score,int,int);
int search_predict_pattern(Block &block,ResidualBlock  &residual_block,Tree &tree,BlockBufferPool & block_buffer_pool,FrameBufferPool &frame_pool,Block & buffer_block,AVFormat & para,int,int);
int reverse_predict(Block &block,ResidualBlock  &residual_block,Tree &tree,BlockBufferPool & block_buffer_pool,FrameBufferPool &frame_pool,AVFormat & para);