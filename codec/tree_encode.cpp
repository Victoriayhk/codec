

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

inline int dp_encode_one_block(Block & block, ResidualBlock & residual_block,Tree & tree, Block & block_buffer,ResidualBlock & residual_block_buffer,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool,AVFormat &para,int,int,int thread_hold);


/**
*  构建一棵分块树，如果在缓存中存在，直接使用
*  李春尧
*/
int dp_encode_one_block_get_tree(Block & block, ResidualBlock & residual_block,Tree ** tree,int tph,int tpw,int brh,int brw,Block & block_buffer,ResidualBlock & residual_block_buffer,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool,AVFormat &para,int i_offset,int j_offset,int thread_hold){

	uint64_t tmp = ((uint64_t)block.block_id << 32) | ((uint64_t)tph << 24)| (tpw << 16) | (brh << 8)| brw;
	int type = (int)block.block_type;
	*tree = cache::getTree(type,tmp);
	if((*tree)->score < 0){
		(*tree)->left_top_h = tph;
		(*tree)->left_top_w = tpw;
		(*tree)->right_bottom_h = brh;
		(*tree)->right_bottom_w = brw;
		int score = dp_encode_one_block(block, residual_block, **tree, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool,para,i_offset,j_offset,thread_hold);
		(*tree) -> score = score;
	}

	return (*tree)->score;

}

/**
*  编码并解码一个小块，输入原始数据，输出残差数据以及报解码数据保存在池里
*  李春尧
*/
int encode_and_decode_with_tree(Block & block, ResidualBlock & residual_block,Tree & tree, Block & block_buffer,ResidualBlock & residual_block_buffer,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool,AVFormat &para,int i_offset,int j_offset){
	
	int h,w;
	residual_block.getBlockSize(para,h,w);
	if(tree.split_direction == Tree::NONE){
		int score0;

		predict(block,residual_block,tree,block_buffer_pool,frame_pool,block_buffer,para,score0,i_offset,j_offset);
		
		
		int tph = tree.left_top_h,tpw = tree.left_top_w,brh = tree.right_bottom_h, brw = tree.right_bottom_w;
		dct_trans(residual_block,tph,tpw,brh,brw,h,w);
		quantization(tph,tpw, brh, brw , residual_block , para);
		
		
		for(int i = tph; i <=  brh; ++i){
			for(int j = tpw; j <= brw; ++j){
				residual_block_buffer.data[TABLE[i][w]+ j] = residual_block.data[TABLE[i][w]+ j];
			}
		}

		Reverse_quantization(tph,tpw, brh, brw , residual_block_buffer , para);
		
		reverse_dct_trans(residual_block_buffer,tph,tpw,brh,brw,h,w);
		
		reverse_predict(block_buffer,residual_block_buffer,tree,block_buffer_pool,frame_pool,para);

	}else{

			encode_and_decode_with_tree(block, residual_block, *tree.left, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool,para,i_offset,j_offset);
		


			encode_and_decode_with_tree(block, residual_block, *tree.right, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool,para,i_offset,j_offset);
	}
	return 0;
}

/**
*  递归搜索分块和模式
*  李春尧
*/
inline int dp_encode_one_block(Block & block, ResidualBlock & residual_block,Tree & tree, Block & block_buffer,ResidualBlock & residual_block_buffer,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool,AVFormat &para,int i_offset,int j_offset,int thread_hold) {
	

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

	score0 = search_predict_pattern(block,residual_block,tree,block_buffer_pool,frame_pool,block_buffer,para,i_offset,j_offset);



	Tree * left, * right;
	Tree * left2, * right2;

	if (score0 < thread_hold && w >= para.tree_mini_block_width ) {
		score1 = dp_encode_one_block_get_tree(block,residual_block,&left,tph,tpw,brh,brw - (w/2), block_buffer,residual_block_buffer,block_buffer_pool,frame_pool,para,i_offset,j_offset,score0 * para.split_gamma);
		score1 += dp_encode_one_block_get_tree(block,residual_block,&right,tph,tpw+(w/2),brh,brw, block_buffer,residual_block_buffer,block_buffer_pool,frame_pool,para,i_offset,j_offset,score0 * para.split_gamma);
	}

	if (score0 < thread_hold && h >= para.tree_mini_block_height) {
		score2 = dp_encode_one_block_get_tree(block,residual_block,&left2,tph,tpw,brh - (h/2),brw, block_buffer,residual_block_buffer,block_buffer_pool,frame_pool,para,i_offset,j_offset,score0 * para.split_gamma);
		score2 += dp_encode_one_block_get_tree(block,residual_block,&right2,tph + (h/2),tpw,brh,brw, block_buffer,residual_block_buffer,block_buffer_pool,frame_pool,para,i_offset,j_offset,score0 * para.split_gamma);
	}

	if(score0 <= score1 && score0 <= score2){
		tree.left = nullptr;
		tree.right = nullptr;
		tree.split_direction = Tree::NONE;
		tree.score = score0;
	}else{

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

	encode_and_decode_with_tree(block, residual_block, tree, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool,para,i_offset,j_offset);

	return tree.score;

}
/**
*  编码一个宏块
*  李春尧
*/
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
	

	int i_offset = TABLE[block.block_id / para.block_num_per_row][h];// 当前block的起始像素所在Frame的行
	int j_offset = TABLE[block.block_id % para.block_num_per_row][w];// 当前block的起始像素所在Frame的列
	dp_encode_one_block(block, residual_block, residual_block.tree, block_buffer,residual_block_buffer, block_buffer_pool, frame_pool,para,i_offset,j_offset,INT_MAX);

	

	return 0;
}


/**
*  编码一个分量,即编码每一个块
*  李春尧
*/
inline int tree_encode_one_component(vector<Block> & blocks, std::vector<ResidualBlock> & residual_blocks,Block & block_buffer,ResidualBlock & residual_block_buffer,AVFormat &para,FrameBufferPool & frame_pool){

	BlockBufferPool & decode_buffer = frame_pool.new_back();
	int block_height,block_width;
	block_buffer.getBlockSize(para,block_height,block_width);
	
	for(int i=0;i<blocks.size();++i){
		
		tree_encode_one_block(blocks[i],residual_blocks[i],block_buffer,residual_block_buffer,para,decode_buffer,frame_pool);
		
	}

	return 0;

}


/**
*  并行编码一帧
*  李春尧
*/
int tree_encode(Frame &frame,AVFormat &para,PKT &pkt,vector<FrameBufferPool*>  &frame_pool){

	
	static Block block_buffer[3] = {Block(frame.Yblock[0]),Block(frame.Ublock[0]),Block(frame.Vblock[0])};
	static ResidualBlock residual_block_buffer[3] = {ResidualBlock(frame.Yblock[0]),ResidualBlock(frame.Ublock[0]),ResidualBlock(frame.Vblock[0])};

	#pragma omp parallel sections num_threads(3)
	{	
		#pragma omp section
		{

			cache::reset(0);
			tree_encode_one_component(frame.Yblock,pkt.Ylist,block_buffer[0],residual_block_buffer[0],para,*frame_pool[0]);
		}	
		#pragma omp section
		{

			cache::reset(1);
			tree_encode_one_component(frame.Ublock,pkt.Ulist,block_buffer[1],residual_block_buffer[1],para,*frame_pool[1]);
		}
		#pragma omp section
		{

			cache::reset(2);
			tree_encode_one_component(frame.Vblock,pkt.Vlist,block_buffer[2],residual_block_buffer[2],para,*frame_pool[2]);
		}
	}


	return 0;
}

