
#include "decode.h"
#include "quantization.h"
#include "Pattern.h"
#include "encode.h"

inline int decode_one_block(Block & block,ResidualBlock & residual_block,AVFormat &para,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool){
	int h,w;
	residual_block.getBlockSize(para,h,w);
	block.block_id = residual_block.block_id;
	block.block_type = residual_block.block_type;
	if(residual_block.type_slice==0)  //采用方式0分割
	{
		Reverse_quantization(0 ,0 ,h-1 ,w-1 , residual_block , para);
		reverse_dct_trans(residual_block,0 ,0 ,h-1 ,w-1 ,h,w);
		Pattern::de_predict(block,residual_block,0,0,h-1,w-1,block_buffer_pool,residual_block.node[0],para);
	}
	else if(residual_block.type_slice==1)  //采用方式1分割
	{
		vector<int> _8_8_type=residual_block.node;
		vector<pos> position;
		position.push_back(pos(0,0,h/2-1,w/2-1));
		position.push_back(pos(0,w/2,h/2-1,w-1));
		position.push_back(pos(h/2,0,h-1,w/2-1));		
		position.push_back(pos(h/2,w/2,h-1,w-1));
		for(int i=0;i<4;++i){
		Reverse_quantization(position[i].left_top_x,position[i].left_top_y,
							position[i].right_bottom_x,position[i].right_bottom_y,residual_block,para);
		reverse_dct_trans(residual_block,position[i].left_top_x,position[i].left_top_y,position[i].right_bottom_x,position[i].right_bottom_y,h,w);
		Pattern::de_predict(block,residual_block,position[i].left_top_x,position[i].left_top_y,
											position[i].right_bottom_x,position[i].right_bottom_y,block_buffer_pool,_8_8_type[i],para);
		}
	}

	
//	Pattern::de_predict(block,residual_block,0,0,h-1,w-1,block_buffer_pool,0,para);
	/*for(int i = 0; i < residual_block.data.size(); ++i){
		block.data[i] = residual_block.data[i];
	}*/


	return 0;
}
inline int decode_one_component(vector<Block> & blocks, std::vector<ResidualBlock> & residual_blocks,AVFormat &para,FrameBufferPool & frame_pool){

	BlockBufferPool  & decode_buffer = frame_pool.new_back();
	int h,w;
	residual_blocks[0].getBlockSize(para,h,w);
	for(int i=0;i< residual_blocks.size();++i){
		decode_one_block(blocks[i],residual_blocks[i],para,decode_buffer,frame_pool);
	}
	//frame_pool.add_frame_to_pool( decode_buffer );
	return 0;

}
int decode(Frame &frame,AVFormat &para,PKT &pkt,vector<FrameBufferPool*>  &frame_pool){
	decode_one_component(frame.Yblock,pkt.Ylist,para,*frame_pool[0]);
	decode_one_component(frame.Ublock,pkt.Ulist,para,*frame_pool[1]);
	decode_one_component(frame.Vblock,pkt.Vlist,para,*frame_pool[2]);
	return 0;
}
