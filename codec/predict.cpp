#pragma once


#include "decode_buffer_pool.h"
#include "predict.h"
#include "Pattern.h"
#include "quantization.h"



int predict_block_inter(Block &block, ResidualBlock  &mode_result, BlockBufferPool &pool,double &score){
	//mode_result.data = block.data;
	score = 0;
	return 0;
}

int predict_block_intra(Block &block, ResidualBlock  &mode_result, FrameBufferPool &pool,double &score){
	//mode_result = block;

	score = 0;
	return 0;
}
int predict(Block &block,ResidualBlock  &residual_block,Tree &tree,BlockBufferPool & block_buffer_pool,FrameBufferPool &frame_pool,Block & buffer_block,AVFormat & para,double & min_score)
{

	int tph = tree.left_top_h,tpw = tree.left_top_w,brh = tree.right_bottom_h, brw = tree.right_bottom_w;
	if(tree.data->pre_type == Node::INTRA_PREDICTION){
		Pattern::predict(block,residual_block,tph,tpw,brh,brw,block_buffer_pool,tree.data->prediction,para);
	}else{
	}
	return 0;
}

double calc_coef(int,int,int,int,Block & block,Block & block_another){
	return 0;
}

double intra_predict(Block &block,ResidualBlock  &residual_block,Tree &tree,BlockBufferPool & block_buffer_pool,Block & buffer_block,AVFormat & para,double & min_score){
	int pattern_num = 2;

	int best_pattern = 0;
	int tph = tree.left_top_h,tpw = tree.left_top_w,brh = tree.right_bottom_h, brw = tree.right_bottom_w;
	double score = min_score;
	for(int i = 0; i < pattern_num; ++i){
		Pattern::predict(block,residual_block,tph,tpw,brh,brw,block_buffer_pool,i,para);

		quantization(tph,tpw,brh,brw, residual_block , para);

		Reverse_quantization(tph,tpw,brh,brw , residual_block , para);

		Pattern::de_predict(buffer_block,residual_block,tph,tpw,brh,brw ,block_buffer_pool,i,para);
		
		score = calc_coef(tph,tpw,brh,brw,buffer_block,block);
		if(score < min_score){
			min_score = score;
			tree.data->pre_type = Node::INTRA_PREDICTION;
			tree.data->prediction = i;
		}

	}
	return min_score;
}
double inter_predict(Block &block,ResidualBlock  &residual_block,Tree &tree,FrameBufferPool &frame_pool,Block & buffer_block,AVFormat & para,double & min_score){

	return min_score;
}
double search_predict_pattern(Block &block,ResidualBlock  &residual_block,Tree &tree,BlockBufferPool & block_buffer_pool,FrameBufferPool &frame_pool,Block & buffer_block,AVFormat & para){
	double score = DBL_MAX;
	intra_predict(block,residual_block,tree,block_buffer_pool,buffer_block,para,score);
	inter_predict(block,residual_block,tree,frame_pool,buffer_block,para,score);
	return score;
}

int reverse_predict(Block &block,ResidualBlock  &residual_block,Tree &tree,BlockBufferPool & block_buffer_pool,FrameBufferPool &frame_pool,Block & buffer_block,AVFormat & para){
	int tph = tree.left_top_h,tpw = tree.left_top_w,brh = tree.right_bottom_h, brw = tree.right_bottom_w;
	if(tree.data->pre_type == Node::INTRA_PREDICTION){
		Pattern::de_predict(block,residual_block,tph,tpw,brh,brw,block_buffer_pool,tree.data->prediction,para);
	}else{
	}
	return 0;
}