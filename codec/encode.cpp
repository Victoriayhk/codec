

#include "predict.h"
#include "encode.h"
#include <queue>
#include "quantization.h"
#include <time.h>
#include "Pattern.h"
using namespace std;




double dp_encode_one_block(Block & block, ResidualBlock & residual_block,Tree & tree, BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool) {
	
	static map<int,Node> dp;
	
	double score0 = DBL_MAX, score1 = DBL_MAX, score2 = DBL_MAX;


	//int predict_inter(Block &block, ResidualBlock  &residual_block, BlockBufferPool &pool,double &score);
	
	
	//Node node;
	//int pattern;
	//intra_predict(block,residual_block,start_r,start_l, h, w,block_buffer_pool,para,score0,pattern);
	//node.pre_type = Node::INTRA_PREDICTION;
	//node.prediction = pattern;


	//inter_predict
	predict(block,residual_block,start_r,start_l, h, w,block_buffer_pool,para,score0,pattern);

	quantization(tph,tpw, rbh, rbw , residual_block , para);



	Reverse_quantization(tph,tpw, rbh, rbw , residual_block , para);
	re_predict(block,residual_block,start_r,start_l, h, w,block_buffer_pool,para,score0,pattern);
	//Pattern::re_pattern(buffer_block,residual_block,tph,tpw,rbh,rbw,block_buffer_pool,i,para);

	double score = compute_coef(block,buffer_block,tph,tpw, rbh, rbw);
	
	return score;




	/*
	if (w > 8) {
		double score1 = dp_encode(block, residual_block, start_r, start_l, h, w/2, block_buffer_pool, frame_pool) + 
						dp_encode(block, residual_block, start_r, start_l + w/2, h, w/2, block_buffer_pool, frame_pool);
	}

	if (h >= 8) {
		double score2 = dp_encode(block, residual_block, start_r, start_l, h/2, w, block_buffer_pool, frame_pool) + 
						dp_encode(block, residual_block, start_r + h/2, start_l, h/2, w, block_buffer_pool, frame_pool);
	}

	*/
	/*if (score0 <= score1 && score0 <= score2) {
		decode(node,residual_block,block_buffer_pool)
		leaf_vector.push_back(Node());
		method_vector.push_back(0);

	} else if (score1 <= score2) {
		method_vector.push_back(1);
	} else {
		method_vector.push_back(2);
	}*/
	

}

inline int encode_one_block(Block & block,ResidualBlock & residual_block,AVFormat &para,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool){

	residual_block.block_id = block.block_id;
	residual_block.block_type = block.block_type;
	
	int h,w;
	block.getBlockSize(para,h,w);

	for(int i = 0; i < block.data.size(); ++i){
		residual_block.data[i] = block.data[i];
	}

	dp_encode_one_block(block, residual_block, residual_block.tree, block_buffer_pool, frame_pool);

	// dp_inter_predict_prepare();
	//dp_encode(block, residual_block, 0, 0, h, w, block_buffer_pool, frame_pool);

	/*for(int i = 0; i < block.data.size(); ++i){
		residual_block.data[i] = block.data[i];
	}*/

	quantization(0 ,0 ,h-1 ,w-1 , residual_block , para);

	return 0;
}
inline int encode_one_component(vector<Block> & blocks, std::vector<ResidualBlock> & residual_blocks,AVFormat &para,FrameBufferPool & frame_pool){

	BlockBufferPool   * decode_buffer = new BlockBufferPool(para.height,para.width);
	int block_height,block_width;
	blocks[0].getBlockSize(para,block_height,block_width);

	for(int i=0;i<blocks.size();++i){
		
		encode_one_block(blocks[i],residual_blocks[i],para,*decode_buffer,frame_pool);
		
	}
	frame_pool.add_frame_to_pool(decode_buffer);
	return 0;

}
int encode(Frame &frame,AVFormat &para,PKT &pkt,vector<FrameBufferPool>  &frame_pool){
	Block block_buffer[3] = {Block(frame.Yblock[0]),Block(frame.Ublock[0]),Block(frame.Vblock[0])};
	ResidualBlock residual_block_buffer[3] = {ResidualBlock(frame.Yblock[0]),ResidualBlock(frame.Ublock[0]),ResidualBlock(frame.Vblock[0])};
	encode_one_component(frame.Yblock,pkt.Ylist,para,frame_pool[0]);
	encode_one_component(frame.Ublock,pkt.Ulist,para,frame_pool[1]);
	encode_one_component(frame.Vblock,pkt.Vlist,para,frame_pool[2]);
	return 0;
}

