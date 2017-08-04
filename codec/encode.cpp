

#include "predict.h"
#include "encode.h"
#include <queue>
#include "quantization.h"

using namespace std;



int block_decode_save(Block &blk,BlockBufferPool &blockbuffer,FrameBufferPool& framebuffer)
{ 
	/*
	if(blk.pre_type==blk.INTRA_PREDICTION)
		blockbuffer.add_block_to_pool(blk);
	else if(blk.pre_type==blk.INTER_PREDICTION)
		blockbuffer.add_block_to_pool(blk);
	else
	{}
	*/
	return 0;
}
inline int encode_one_block(Block & block,ResidualBlock & residual_block,AVFormat &para,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool){

	residual_block.block_id = block.block_id;
	residual_block.block_type = block.block_type;
	for(int i = 0; i < block.data.size(); ++i){
		residual_block.data[i] = block.data[i];
	}

	int h,w;
	para.getBlockSize(block,h,w);
	quantization(0 ,0 ,h-1 ,w-1 , residual_block , para);

	return 0;
}
inline int encode_one_component(vector<Block> & blocks, std::vector<ResidualBlock> & residual_blocks,AVFormat &para,FrameBufferPool & frame_pool){
	BlockBufferPool  decode_buffer(para.height,para.width);

	for(int i=0;i<blocks.size();++i){
		int block_height = para.block_height;
		int block_width = para.block_width;
		if(blocks[i].block_type != Block::Y)
		{
			block_height /= 2;
			block_width /= 2;
		}
		ResidualBlock residual_block(block_height,block_width);
		encode_one_block(blocks[i],residual_block,para,decode_buffer,frame_pool);

		residual_blocks.push_back(residual_block);
	}
	frame_pool.add_frame_to_pool(decode_buffer);
	return 0;

}
int encode(Frame &frame,AVFormat &para,PKT &pkt,vector<FrameBufferPool>  &frame_pool){
	encode_one_component(frame.Yblock,pkt.Ylist,para,frame_pool[0]);
	encode_one_component(frame.Ublock,pkt.Ulist,para,frame_pool[0]);
	encode_one_component(frame.Vblock,pkt.Vlist,para,frame_pool[0]);
	return 0;
}

