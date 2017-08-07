
#include "decode.h"
#include "quantization.h"



inline int decode_one_block(Block & block,ResidualBlock & residual_block,AVFormat &para,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool){
	int h,w;
	residual_block.getBlockSize(para,h,w);
	Reverse_quantization(0 ,0 ,h-1 ,w-1 , residual_block , para);


	block.block_id = residual_block.block_id;
	block.block_type = residual_block.block_type;
	for(int i = 0; i < residual_block.data.size(); ++i){
		block.data[i] = residual_block.data[i];
	}


	return 0;
}
inline int decode_one_component(vector<Block> & blocks, std::vector<ResidualBlock> & residual_blocks,AVFormat &para,FrameBufferPool & frame_pool){

	BlockBufferPool   * decode_buffer = new BlockBufferPool(para.height,para.width);
	int h,w;
	residual_blocks[0].getBlockSize(para,h,w);
	for(int i=0;i< residual_blocks.size();++i){
		decode_one_block(blocks[i],residual_blocks[i],para,*decode_buffer,frame_pool);
	}
	frame_pool.add_frame_to_pool(decode_buffer);
	return 0;

}
int decode(Frame &frame,AVFormat &para,PKT &pkt,vector<FrameBufferPool>  &frame_pool){
	decode_one_component(frame.Yblock,pkt.Ylist,para,frame_pool[0]);
	decode_one_component(frame.Ublock,pkt.Ulist,para,frame_pool[1]);
	decode_one_component(frame.Vblock,pkt.Vlist,para,frame_pool[2]);
	return 0;
}
