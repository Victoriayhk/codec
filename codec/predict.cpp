#pragma once


#include "decode_buffer_pool.h"
#include "predict.h"
#include "Pattern.h"
#include "quantization.h"
#include "dctInterface.h"


int predict(Block &block,ResidualBlock  &residual_block,Tree &tree,BlockBufferPool & block_buffer_pool,FrameBufferPool &frame_pool,Block & buffer_block,AVFormat & para,double & min_score)
{
	static InterMV tmp_inter_mv;
	int tph = tree.left_top_h,tpw = tree.left_top_w,brh = tree.right_bottom_h, brw = tree.right_bottom_w;
	if(tree.data->pre_type == Node::INTRA_PREDICTION){
		Pattern::predict(block,residual_block,tph,tpw,brh,brw,block_buffer_pool,tree.data->prediction,para);
	}else{
		tmp_inter_mv.fi = tree.data->prediction;
		tmp_inter_mv.mv.first = tree.data->mv[0];
		tmp_inter_mv.mv.second = tree.data->mv[1];
		Pattern::inter_predict_setvalue(block,residual_block, tph, tpw, brh, brw, frame_pool, tmp_inter_mv, para);
	}
	return 0;
}

double calc_coef(int,int,int,int,Block & block,Block & block_another){
	return 0;
}

double intra_predict(Block &block,ResidualBlock  &residual_block,Tree &tree,BlockBufferPool & block_buffer_pool,Block & buffer_block,AVFormat & para,double & min_score){
	

	int best_pattern = 0;
	int tph = tree.left_top_h,tpw = tree.left_top_w,brh = tree.right_bottom_h, brw = tree.right_bottom_w;
	double score = min_score;
	for(int i = 0; i < para.pattern_num; ++i){
		score = Pattern::predict(block,residual_block,tph,tpw,brh,brw,block_buffer_pool,i,para);
		score += calc_coef(tph,tpw,brh,brw,buffer_block,block);
		if(score < min_score){
			min_score = score;
			tree.data->pre_type = Node::INTRA_PREDICTION;
			tree.data->prediction = i;
		}

	}


	return min_score;
}

double inter_predict(Block &block,ResidualBlock  &residual_block,Tree &tree,FrameBufferPool &frame_pool,Block & buffer_block,AVFormat & para,double & min_score){
	double score = min_score;
	int tph = tree.left_top_h,tpw = tree.left_top_w,brh = tree.right_bottom_h, brw = tree.right_bottom_w;
	static InterMV tmp_inter_mv;
	score = Pattern::inter_predict(block,residual_block, tph, tpw, brh, brw, frame_pool, tmp_inter_mv, para, min_score);
	if(score < min_score){
		min_score = score;
		tree.data->pre_type = Node::INTER_PREDICTION;
		tree.data->prediction = tmp_inter_mv.fi;
		tree.data->mv[0] = tmp_inter_mv.mv.first;
		tree.data->mv[1] = tmp_inter_mv.mv.second;
	}

	return min_score;
}

double search_predict_pattern(Block &block,ResidualBlock  &residual_block,Tree &tree,BlockBufferPool & block_buffer_pool,FrameBufferPool &frame_pool,Block & buffer_block,AVFormat & para){
	double score = DBL_MAX;
	intra_predict(block,residual_block,tree,block_buffer_pool,buffer_block,para,score);
	/*if(frame_pool.size() > 1)
		inter_predict(block,residual_block,tree,frame_pool,buffer_block,para,score);	*/
	//else
	//	intra_predict(block,residual_block,tree,block_buffer_pool,buffer_block,para,score);
	return score;
}

int reverse_predict(Block &block,ResidualBlock  &residual_block,Tree &tree,BlockBufferPool & block_buffer_pool,FrameBufferPool &frame_pool,AVFormat & para){
	int tph = tree.left_top_h,tpw = tree.left_top_w,brh = tree.right_bottom_h, brw = tree.right_bottom_w;
	if(tree.data->pre_type == Node::INTRA_PREDICTION){
		Pattern::de_predict(block,residual_block,tph,tpw,brh,brw,block_buffer_pool,tree.data->prediction,para);
	}else{
		static int cnt = 0;
		cnt++;
		if(cnt % 10000 == 0)
			printf("Ö¡¼ä  %d\n",cnt);
		static InterMV tmp_inter_mv;
		tmp_inter_mv.fi = tree.data->prediction;
		tmp_inter_mv.mv.first = tree.data->mv[0];
		tmp_inter_mv.mv.second = tree.data->mv[1];
		Pattern::inter_predict_reverse(block, residual_block, tph, tpw, brh, brw, frame_pool,block_buffer_pool, tmp_inter_mv, para);
	}
	return 0;
}
