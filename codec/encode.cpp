

#include "predict.h"
#include "encode.h"
#include <queue>
#include "quantization.h"
#include <time.h>
#include "Pattern.h"
using namespace std;



inline int encode_one_block(Block & block,ResidualBlock & residual_block,AVFormat &para,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool){

	residual_block.block_id = block.block_id;
	residual_block.block_type = block.block_type;
	
	int h,w;
	block.getBlockSize(para,h,w);

	for(int i = 0; i < block.data.size(); ++i){
		residual_block.data[i] = block.data[i];
	}


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

