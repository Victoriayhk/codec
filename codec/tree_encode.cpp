

#include "predict.h"
#include "encode.h"
#include <queue>
#include "quantization.h"
#include "ResidualBlock.h"
#include <time.h>
#include "Pattern.h"
using namespace std;



double dp_encode_one_block_get_tree(Block & block, ResidualBlock & residual_block,Tree & tree, Block & block_buffer,ResidualBlock & residual_block_buffer,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool,AVFormat &para){
	static map<int,Tree *> dp;
}

double dp_encode_one_block(Block & block, ResidualBlock & residual_block,Tree & tree, Block & block_buffer,ResidualBlock & residual_block_buffer,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool,AVFormat &para) {
	
	static map<int,Tree *> dp;
	int tmp = 0;
	tmp = tmp | (tree.left_top_h << 24) | (tree.left_top_w << 16) | (tree.right_bottom_h << 8) | (tree.right_bottom_w);
	if(dp.fint(tmp) != dp.end()){
		tree.left = dp[tmp]->left;
		tree.right = dp[tmp]->right;
		tree.data = dp[tmp]->data;
		tree.score = dp[tmp]->score;
	}else{
		double score0 = DBL_MAX, score1 = DBL_MAX, score2 = DBL_MAX;
		tree.score = DBL_MAX,
		Node * node = new Node;
		tree.data = node;
		score0 = search_predict_pattern(block,residual_block,tree,block_buffer_pool,frame_pool,block_buffer,para);
		int w = tree.right_bottom_w - tree.left_top_w + 1;
		int h = tree.left_top_h - tree.right_bottom_h;

		Tree * left, * right;

		if (w >= 8) {
			left = new Tree(tree.left_top_h,tree.left_top_w,tree.right_bottom_h,tree.right_bottom_w /2 );
			right = new Tree(tree.left_top_h,tree.left_top_w/2 + 1,tree.right_bottom_h,tree.right_bottom_w);

			score1 = dp_encode_one_block(block, residual_block, *left, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool,para) + 
							dp_encode_one_block(block, residual_block, *right, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool,para);

		}

		if (h >= 8) {
			if(score1 < score0){
				//block_buffer_pool.set_block(block_buffer,tree.left_top_h,tree.left_top_w,tree.right_bottom_h,tree.right_bottom_w);
				//residual_block_buffer.data = residual_block.data;
				tree.left = left;
				tree.right = right;
			}else if(w >= 8) {
				left = nullptr;
				right = nullptr;
			}
			left = new Tree(tree.left_top_h,tree.left_top_w,tree.right_bottom_h/2,tree.right_bottom_w );
			right = new Tree(tree.left_top_h/2 + 1,tree.left_top_w,tree.right_bottom_h,tree.right_bottom_w);

			score2 = dp_encode_one_block(block, residual_block, *left, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool,para) + 
							dp_encode_one_block(block, residual_block, *right, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool,para);

		}
		if(score0 <= score1 && score0 <= score2){
			if(h >= 8){
				delete left;
				delete right;
				left = nullptr;
				right = nullptr;
			}
			delete tree.data;
			tree.data = nullptr;
			tree.split_direction = Tree::NONE;
			score = score0;
			predict(block,residual_block,tree,block_buffer_pool,frame_pool,block_buffer,para,score0);

			int tph = tree.left_top_h,tpw = tree.left_top_w,brh = tree.right_bottom_h, brw = tree.right_bottom_w;
			quantization(tph,tpw, brh, brw , residual_block , para);

			residual_block_buffer.data = residual_block.data;

			Reverse_quantization(tph,tpw, brh, brw , residual_block_buffer , para);

			re_predict(block_buffer,residual_block_buffer,tree,block_buffer_pool,frame_pool,block_buffer,para);

			block_buffer_pool.set_block(block_buffer,tree.left_top_h,tree.left_top_w,tree.right_bottom_h,tree.right_bottom_w);
		}
		else if(score2 > score1){
			tree.split_direction = Tree::VERTICAL;
			residual_block.data = residual_block_buffer.data;
			score = score1;
		}else{
			tree.split_direction = Tree::HORIZONTAL;
			tree.left = left;
			tree.right = right;
			score = score2;
			block_buffer_pool.set_block(block_buffer,tree.left_top_h,tree.left_top_w,tree.right_bottom_h,tree.right_bottom_w);
		}

	}
	return score;

}

inline int tree_encode_one_block(Block & block,ResidualBlock & residual_block,Block & block_buffer,ResidualBlock & residual_block_buffer,AVFormat &para,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool){

	residual_block.block_id = block.block_id;
	residual_block.block_type = block.block_type;
	
	int h,w;
	block.getBlockSize(para,h,w);

	dp_encode_one_block(block, residual_block, residual_block.tree, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool,para);


	return 0;
}
inline int tree_encode_one_component(vector<Block> & blocks, std::vector<ResidualBlock> & residual_blocks,Block & block_buffer,ResidualBlock & residual_block_buffer,AVFormat &para,FrameBufferPool & frame_pool){

	BlockBufferPool   * decode_buffer = new BlockBufferPool(para.height,para.width);
	int block_height,block_width;
	block_buffer.getBlockSize(para,block_height,block_width);

	for(int i=0;i<blocks.size();++i){
		
		tree_encode_one_block(blocks[i],residual_blocks[i],block_buffer,residual_block_buffer,para,*decode_buffer,frame_pool);
		
	}
	frame_pool.add_frame_to_pool(decode_buffer);
	return 0;

}
int tree_encode(Frame &frame,AVFormat &para,PKT &pkt,vector<FrameBufferPool>  &frame_pool){
	Block block_buffer[3] = {Block(frame.Yblock[0]),Block(frame.Ublock[0]),Block(frame.Vblock[0])};
	ResidualBlock residual_block_buffer[3] = {ResidualBlock(frame.Yblock[0]),ResidualBlock(frame.Ublock[0]),ResidualBlock(frame.Vblock[0])};

	tree_encode_one_component(frame.Yblock,pkt.Ylist,block_buffer[0],residual_block_buffer[0],para,frame_pool[0]);
	tree_encode_one_component(frame.Ublock,pkt.Ulist,block_buffer[1],residual_block_buffer[1],para,frame_pool[1]);
	tree_encode_one_component(frame.Vblock,pkt.Vlist,block_buffer[2],residual_block_buffer[2],para,frame_pool[2]);
	return 0;
}

