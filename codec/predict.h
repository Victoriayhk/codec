#pragma once

#include "def.h"
#include "decode_buffer_pool.h"
#include "ResidualBlock.h"


int predict_block_inter(Block &block, ResidualBlock  &residual_block, BlockBufferPool &pool,double &score);

int predict_block_intra(Block &block, ResidualBlock  &residual_block, FrameBufferPool &pool,double &score);
