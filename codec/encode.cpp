
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
	//predict_block_inter(input_block,block_inter,decode_buffer_Y,min_block_inter);
	//predict_block_intra(input_block,block_intra,frame_pool[0],min_block_intra);
//	printf("%d\n",residual_block.data[0]);
	quantization(0 ,0 ,7 ,7 , residual_block , para);
//	printf("%d\n",residual_block.data[0]);
	return 0;
}
inline int encode_one_component(vector<Block> & blocks, std::vector<ResidualBlock> & residual_blocks,AVFormat &para,FrameBufferPool & frame_pool){
	BlockBufferPool  decode_buffer(para.height,para.width);
	//ResidualBlock residual_block(para.block_height,para.block_width);
	for(int i=0;i<blocks.size();++i){
		ResidualBlock residual_block(para.block_height,para.block_width);
		encode_one_block(blocks[i],residual_block,para,decode_buffer,frame_pool);
		//decode_one_block(blocks[i],residual_block,para,decode_buffer,frame_pool);

		residual_blocks.push_back(residual_block);
		//residual_block.data.clear();
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

