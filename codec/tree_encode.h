#pragma once



#include "ResidualBlock.h"
#include "decode_buffer_pool.h"


//double dp_encode_one_block_get_tree(Block & block, ResidualBlock & residual_block,Tree ** tree,int tph,int tpw,int brh,int brw,Block & block_buffer,ResidualBlock & residual_block_buffer,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool,AVFormat &para);
//int encode_and_decode_with_tree(Block & block, ResidualBlock & residual_block,Tree & tree, Block & block_buffer,ResidualBlock & residual_block_buffer,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool,AVFormat &para);
//double dp_encode_one_block(Block & block, ResidualBlock & residual_block,Tree & tree, Block & block_buffer,ResidualBlock & residual_block_buffer,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool,AVFormat &para);
int tree_encode(Frame &frame,AVFormat &para,PKT &pkt,vector<FrameBufferPool*>  &frame_pool);