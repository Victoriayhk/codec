

#include "predict.h"
#include "encode.h"
#include <queue>
#include "quantization.h"
#include <time.h>
#include "Pattern.h"
using namespace std;




double dp_encode_one_block(Block & block, ResidualBlock & residual_block,Tree & tree, Block & block_buffer,ResidualBlock & residual_block_buffer,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool) {
	
	static map<int,Node> dp;
	
	double score = DBL_MAX,score0 = DBL_MAX, score1 = DBL_MAX, score2 = DBL_MAX;
	
	Node * node = new Node;
	tree.node = node;

	double score0 = search_predict_pattern(block,residual_block,tree,block_buffer_pool,frame_pool,block_buffer,para);
	
	if (w >= 8) {
		score1 = dp_encode_one_block(block, residual_block, tree, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool) + 
						dp_encode_one_block(block, residual_block, tree, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool);

	}

	if (h >= 8) {
		if(score1 < score0){
			//copy_to_buffer();
			
		}
		score2 = dp_encode_one_block(block, residual_block, tree, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool) + 
						dp_encode_one_block(block, residual_block, tree, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool);

	}
	if(score0 <= score1 && score0 <= score2){
		score = score0;

		predict(block,residual_block,tree,block_buffer_pool,frame_pool,para);

		quantization(tph,tpw, rbh, rbw , residual_block , para);

		residual_block_buffer.data = residual_block.data;
		//copy_to_buffer()

		Reverse_quantization(tph,tpw, rbh, rbw , residual_block_buffer , para);

		re_predict(buffer_block,residual_block_buffer,tree,block_buffer_pool,frame_pool,para);

		//copy
	}
	else if(score2 > score1){
		//copy_back();
		score = score1;
	}else{
		score = score2;
	}
	

	//double score = compute_coef(block,buffer_block,tph,tpw, rbh, rbw);
	
	return score;

}

inline int tree_encode_one_block(Block & block,ResidualBlock & residual_block,Block & block_buffer,ResidualBlock & residual_block_buffer,AVFormat &para,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool){

	residual_block.block_id = block.block_id;
	residual_block.block_type = block.block_type;
	
	int h,w;
	block.getBlockSize(para,h,w);

	for(int i = 0; i < block.data.size(); ++i){
		residual_block.data[i] = block.data[i];
	}

	dp_encode_one_block(block, residual_block, residual_block.tree,block_buffer,residual_block_buffer, block_buffer_pool, frame_pool);

	// dp_inter_predict_prepare();
	//dp_encode(block, residual_block, 0, 0, h, w, block_buffer_pool, frame_pool);

	/*for(int i = 0; i < block.data.size(); ++i){
		residual_block.data[i] = block.data[i];
	}*/

	quantization(0 ,0 ,h-1 ,w-1 , residual_block , para);

	return 0;
}
inline int tree_encode_one_component(vector<Block> & blocks, std::vector<ResidualBlock> & residual_blocks,Block & block_buffer,ResidualBlock & residual_block_buffer,AVFormat &para,FrameBufferPool & frame_pool){

	BlockBufferPool   * decode_buffer = new BlockBufferPool(para.height,para.width);
	int block_height,block_width;
	block_buffer.getBlockSize(para,block_height,block_width);

	for(int i=0;i<blocks.size();++i){
		
		encode_one_block(blocks[i],residual_blocks[i],block_buffer,residual_block_buffer,para,*decode_buffer,frame_pool);
		
	}
	frame_pool.add_frame_to_pool(decode_buffer);
	return 0;

}
int tree_encode(Frame &frame,AVFormat &para,PKT &pkt,vector<FrameBufferPool>  &frame_pool){
	Block block_buffer[3] = {Block(frame.Yblock[0]),Block(frame.Ublock[0]),Block(frame.Vblock[0])};
	ResidualBlock residual_block_buffer[3] = {ResidualBlock(frame.Yblock[0]),ResidualBlock(frame.Ublock[0]),ResidualBlock(frame.Vblock[0])};

	encode_one_component(frame.Yblock,pkt.Ylist,block_buffer[0],residual_block_buffer[0],para,frame_pool[0]);
	encode_one_component(frame.Ublock,pkt.Ulist,block_buffer[1],residual_block_buffer[1],para,frame_pool[1]);
	encode_one_component(frame.Vblock,pkt.Vlist,block_buffer[2],residual_block_buffer[2],para,frame_pool[2]);
	return 0;
}

