

#include "predict.h"
#include "encode.h"
#include <queue>
#include "quantization.h"
#include "ResidualBlock.h"
#include <time.h>
#include "Pattern.h"
#include "tree_encode.h"
#include <omp.h>
#include "dctInterface.h"
#include "cache.h"
using namespace std;

extern int TABLE[1500][1500];

static map<int,Tree *> dp[3];
inline int dp_encode_one_block(Block & block, ResidualBlock & residual_block,Tree & tree, Block & block_buffer,ResidualBlock & residual_block_buffer,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool,AVFormat &para);


void clear_map( map<int,Tree *> &dp){
	dp.clear();
	/*
	for (map<int,Tree *>::iterator p=dp.begin(); p!=dp.end(); ++p) 
	{
		p->second->score = -1;
		p->second->left = nullptr;
		p->second->right = nullptr;
		p->second->data = nullptr;
		p->second->node_id = 1024;
	}
	*/
}
int dp_encode_one_block_get_tree(Block & block, ResidualBlock & residual_block,Tree ** tree,int tph,int tpw,int brh,int brw,Block & block_buffer,ResidualBlock & residual_block_buffer,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool,AVFormat &para){

	uint64_t tmp = ((uint64_t)block.block_id << 32) | ((uint64_t)tph << 24)| (tpw << 16) | (brh << 8)| brw;
	int type = (int)block.block_type;
	*tree = cache::getTree(type,tmp);
	if((*tree)->score < 0){
		(*tree)->left_top_h = tph;
		(*tree)->left_top_w = tpw;
		(*tree)->right_bottom_h = brh;
		(*tree)->right_bottom_w = brw;
		int score = dp_encode_one_block(block, residual_block, **tree, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool,para);
		(*tree) -> score = score;
	}
	/*
	unsigned int tmp = 0 | (tph << 24) | (tpw << 16) | (brh << 8) | (brw);
	auto& dp_ = dp[(int)block.block_type];

	if(dp_.find(tmp) != dp_.end()){
		*tree = dp_[tmp];
		if((*tree)->score >= 0){
			return (*tree)->score;
		}
	}else{
		*tree = new Tree(tph,tpw,brh,brw);
		dp_[tmp] = *tree;
	}
	double score = dp_encode_one_block(block, residual_block, **tree, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool,para);
	(*tree) -> score = score;
	*/
	return (*tree)->score;

}
int encode_and_decode_with_tree(Block & block, ResidualBlock & residual_block,Tree & tree, Block & block_buffer,ResidualBlock & residual_block_buffer,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool,AVFormat &para){
	
	int h,w;
	residual_block.getBlockSize(para,h,w);
	if(tree.split_direction == Tree::NONE){
		int score0;
		predict(block,residual_block,tree,block_buffer_pool,frame_pool,block_buffer,para,score0);
		
		
		int tph = tree.left_top_h,tpw = tree.left_top_w,brh = tree.right_bottom_h, brw = tree.right_bottom_w;
		dct_trans(residual_block,tph,tpw,brh,brw,h,w);
		quantization(tph,tpw, brh, brw , residual_block , para);
		
		
		for(int i = tph; i <=  brh; ++i){
			for(int j = tpw; j <= brw; ++j){
				residual_block_buffer.data[TABLE[i][w]+ j] = residual_block.data[TABLE[i][w]+ j];
			}
		}
		//residual_block_buffer = residual_block;
		Reverse_quantization(tph,tpw, brh, brw , residual_block_buffer , para);
		
		reverse_dct_trans(residual_block_buffer,tph,tpw,brh,brw,h,w);
		
		reverse_predict(block_buffer,residual_block_buffer,tree,block_buffer_pool,frame_pool,para);

	}else{

			encode_and_decode_with_tree(block, residual_block, *tree.left, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool,para);
		

			encode_and_decode_with_tree(block, residual_block, *tree.right, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool,para);
	}
	return 0;
}
inline int dp_encode_one_block(Block & block, ResidualBlock & residual_block,Tree & tree, Block & block_buffer,ResidualBlock & residual_block_buffer,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool,AVFormat &para) {
	

	int tph = tree.left_top_h,tpw = tree.left_top_w,brh = tree.right_bottom_h, brw = tree.right_bottom_w;
	
	int score0 = INT_MAX, score1 = INT_MAX, score2 = INT_MAX;
	tree.score = INT_MAX;
	
	//Node * node = new Node;
	int node_id = 0;
	Node &node = residual_block.get_node(node_id);
	tree.node_id = node_id;
	tree.data = & node;

	int w = tree.right_bottom_w - tree.left_top_w + 1;
	int h = tree.right_bottom_h - tree.left_top_h  + 1;

	score0 = search_predict_pattern(block,residual_block,tree,block_buffer_pool,frame_pool,block_buffer,para);


	Tree * left, * right;
	Tree * left2, * right2;
	if (w >= para.tree_mini_block_width ) {
		score1 = dp_encode_one_block_get_tree(block,residual_block,&left,tph,tpw,brh,brw - (w/2), block_buffer,residual_block_buffer,block_buffer_pool,frame_pool,para);
		score1 += dp_encode_one_block_get_tree(block,residual_block,&right,tph,tpw+(w/2),brh,brw, block_buffer,residual_block_buffer,block_buffer_pool,frame_pool,para);
	}

	if (h >= para.tree_mini_block_height) {
		score2 = dp_encode_one_block_get_tree(block,residual_block,&left2,tph,tpw,brh - (h/2),brw, block_buffer,residual_block_buffer,block_buffer_pool,frame_pool,para);
		score2 += dp_encode_one_block_get_tree(block,residual_block,&right2,tph + (h/2),tpw,brh,brw, block_buffer,residual_block_buffer,block_buffer_pool,frame_pool,para);
	}
	if(score0 <= score1 && score0 <= score2){
		tree.left = nullptr;
		tree.right = nullptr;
		tree.split_direction = Tree::NONE;
		tree.score = score0;
	}else{
		//residual_block
		//delete tree.data;
		tree.node_id = -1;
		tree.data = nullptr;
		if(score1 < score2){
			tree.score = score1;
			tree.left = left;
			tree.right = right;
			tree.split_direction = Tree::HORIZONTAL;
		}else{
			tree.score = score2;
			tree.left = left2;
			tree.right = right2;
			tree.split_direction = Tree::VERTICAL;
		}
	}
	encode_and_decode_with_tree(block, residual_block, tree, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool,para);

	return tree.score;

}

inline int tree_encode_one_block(Block & block,ResidualBlock & residual_block,Block & block_buffer,ResidualBlock & residual_block_buffer,AVFormat &para,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool){

	residual_block.block_id = block.block_id;
	residual_block.block_type = block.block_type;
	int h,w;
	block.getBlockSize(para,h,w);
	residual_block.curr_node = 0;
	residual_block_buffer.block_id = block.block_id;
	residual_block_buffer.curr_node = 0;
	residual_block_buffer.block_type = block.block_type;
	residual_block.tree_byte=0;
	auto& dp_ = dp[(int)block.block_type];
	
	//clear_map(dp_);
	dp_encode_one_block(block, residual_block, residual_block.tree, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool,para);
	//residual_block.tree_byte=0;
	//residual_block.tree_to_stream();
	

	return 0;
}
inline int tree_encode_one_component(vector<Block> & blocks, std::vector<ResidualBlock> & residual_blocks,Block & block_buffer,ResidualBlock & residual_block_buffer,AVFormat &para,FrameBufferPool & frame_pool){

	BlockBufferPool & decode_buffer = frame_pool.new_back();
	int block_height,block_width;
	block_buffer.getBlockSize(para,block_height,block_width);
	
	for(int i=0;i<blocks.size();++i){
		
		tree_encode_one_block(blocks[i],residual_blocks[i],block_buffer,residual_block_buffer,para,decode_buffer,frame_pool);
		
	}

	return 0;

}
int tree_encode(Frame &frame,AVFormat &para,PKT &pkt,vector<FrameBufferPool*>  &frame_pool){

	
	static Block block_buffer[3] = {Block(frame.Yblock[0]),Block(frame.Ublock[0]),Block(frame.Vblock[0])};
	static ResidualBlock residual_block_buffer[3] = {ResidualBlock(frame.Yblock[0]),ResidualBlock(frame.Ublock[0]),ResidualBlock(frame.Vblock[0])};

	#pragma omp parallel sections num_threads(3)
	{	
		#pragma omp section
		{
			cout<<omp_get_thread_num()<<endl;
			cache::reset(0);
			tree_encode_one_component(frame.Yblock,pkt.Ylist,block_buffer[0],residual_block_buffer[0],para,*frame_pool[0]);
		}	
		#pragma omp section
		{
			cout<<omp_get_thread_num()<<endl;
			cache::reset(1);
			tree_encode_one_component(frame.Ublock,pkt.Ulist,block_buffer[1],residual_block_buffer[1],para,*frame_pool[1]);
		}
		#pragma omp section
		{
			cout<<omp_get_thread_num()<<endl;
			cache::reset(2);
			tree_encode_one_component(frame.Vblock,pkt.Vlist,block_buffer[2],residual_block_buffer[2],para,*frame_pool[2]);
		}
	}


	return 0;
}

