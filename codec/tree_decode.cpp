
#include "tree_decode.h"

#include "predict.h"
#include "encode.h"
#include <queue>
#include "quantization.h"
#include "ResidualBlock.h"
#include "Pattern.h"
#include <omp.h>
#include "dctInterface.h"



inline int decode_with_tree(Block & block, ResidualBlock & residual_block,Tree & tree,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool,AVFormat &para){
	
		
	int h,w;
	residual_block.getBlockSize(para,h,w);
	if(tree.split_direction == Tree::NONE){
		int tph = tree.left_top_h,tpw = tree.left_top_w,brh = tree.right_bottom_h, brw = tree.right_bottom_w;
		tree.data = &residual_block.node_list[tree.node_id];

		Reverse_quantization(tph,tpw, brh, brw , residual_block , para);
		reverse_dct_trans(residual_block,tph,tpw,brh,brw,h,w);
		
		reverse_predict(block,residual_block,tree,block_buffer_pool,frame_pool,para);

	}else{

			decode_with_tree(block, residual_block, *tree.left,  block_buffer_pool, frame_pool,para);

			decode_with_tree(block, residual_block, *tree.right, block_buffer_pool, frame_pool,para);
	}
	return 0;
}


inline int tree_decode_one_block(Block & block,ResidualBlock & residual_block,AVFormat &para,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool){

	int h,w;
	residual_block.getBlockSize(para,h,w);
	block.block_id = residual_block.block_id;
	block.block_type = residual_block.block_type;


	decode_with_tree(block,residual_block,residual_block.tree,block_buffer_pool,frame_pool,para);

	return 0;
}


inline int tree_decode_one_component(vector<Block> & blocks, std::vector<ResidualBlock> & residual_blocks,AVFormat &para,FrameBufferPool & frame_pool){

	BlockBufferPool & decode_buffer = frame_pool.new_back();
	int block_height,block_width;
	residual_blocks[0].getBlockSize(para,block_height,block_width);
	
	for(int i=0;i<blocks.size();++i){
		
		tree_decode_one_block(blocks[i],residual_blocks[i],para,decode_buffer,frame_pool);
		
	}
	
	return 0;

}


int tree_decode(Frame &frame,AVFormat &para,PKT &pkt,vector<FrameBufferPool*>  &frame_pool){

	#pragma omp parallel sections num_threads(3)
	{	
		#pragma omp section
		{
			tree_decode_one_component(frame.Yblock,pkt.Ylist,para,*frame_pool[0]);
		}	
		#pragma omp section
		{
			tree_decode_one_component(frame.Ublock,pkt.Ulist,para,*frame_pool[1]);
		}
		#pragma omp section
		{
			tree_decode_one_component(frame.Vblock,pkt.Vlist,para,*frame_pool[2]);
		}
	}
	
	return 0;

}