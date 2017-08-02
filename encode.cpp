#include "encode.h"
#include <stdint.h>
#include <queue>

using namespace std;

//auto *decode_frame_buffer=new queue<Frame>();


int block_decode_save(Block &blk,queue<Block> &que)
{ 

}

int encode(Frame &frame,AVFormat &para,pkt &pkt,vector<FrameBufferPool>  &frame_pool)
{
	BlockBufferPool  decode_buffer_Y(frame.Yblock).size());
	BlockBufferPool  decode_buffer_U(frame.Ublock).size());
	BlockBufferPool  decode_buffer_V(frame.Vblock).size());
	
	BlockBufferPool block_buffer_pool;

	
	Block block_intra(para.block_height,para.block_width);
	Block block_inter(para.block_height,para.block_width);
	double min_block_intra;
	double min_block_inter;

	for(int i=0;i<block_num;++i){
		// encode Y
		Block& input_block = frame.Y_que[i];	
		predic_block_inter(input_block,block_inter,decode_buffer_Y,min_block_inter);
		predic_block_intra(input_block,block_intra,frame_pool[0],min_block_intra);

		if(min_block_intra>min_block_inter)
		{
			pkt.Ylist.push_back(block_inter);
			block_decode_save(block_inter, decode_buffer_Y);		
		}
		else
			{
			pkt.Ylist.push_back(block_intra);
			block_decode_save(block_intra, decode_buffer_Y);	
		}
	//encode U
		Block& input_block = frame.U_que[i];	
		predic_block_inter(input_block,block_inter,decode_buffer_U,min_block_inter);
		predic_block_intra(input_block,block_intra,frame_pool[0],min_block_intra);

		if(min_block_intra>min_block_inter)
		{
			pkt.Ulist.push_back(block_inter);
			block_decode_save(block_inter, decode_buffer_U);		
		}
		else
			{
			pkt.Ulist.push_back(block_intra);
			block_decode_save(block_intra, decode_buffer_U);	
		}
	//encode V
		Block& input_block = frame.V_que[i];	
		predic_block_inter(input_block,block_inter,decode_buffer_V,min_block_inter);
		predic_block_intra(input_block,block_intra,frame_pool[0],min_block_intra);

		if(min_block_intra>min_block_inter)
		{
			pkt.Vlist.push_back(block_inter);
			block_decode_save(block_inter, decode_buffer_V);		
		}
		else
			{
			pkt.Vlist.push_back(block_intra);
			block_decode_save(block_intra, decode_buffer_V);	
		}
	}
	delete block_inter;
	delete block_intra;
}