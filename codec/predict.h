#pragma once

#include "def.h"
#include "decode_buffer_pool.h"
#include "ResidualBlock.h"


int predict_block_inter(Block &block, ResidualBlock  &residual_block, BlockBufferPool &pool,double &score);

//int predict_block_intra(Block &block, ResidualBlock  &residual_block, FrameBufferPool &pool,double &score);

int predict(Block &block,ResidualBlock  &residual_block,int tph,int tpw, int rbh, int rbw, BlockBufferPool &block_buffer_pool, FrameBufferPool &pool,BufferBlock & buffer_block, AVFormat &para,double & score,int & pattern);

int re_predict(Block &block,ResidualBlock  &residual_block,int tph,int tpw, int rbh, int rbw, BlockBufferPool &block_buffer_pool, FrameBufferPool &pool,BufferBlock & buffer_block, AVFormat &para,double & score,int & pattern);

int intra_predict(Block &block,ResidualBlock  &residual_block,int tph,int tpw, int rbh, int rbw, BlockBufferPool &block_buffer_pool,BufferBlock & buffer_block, AVFormat &para,double & score,int & pattern);