
#include "predict.h"
#include "encode.h"
#include <queue>

using namespace std;



int block_decode_save(Block &blk,BlockBufferPool &blockbuffer,FrameBufferPool& framebuffer)
{ 
	if(blk.pre_type==blk.INTRA_PREDICTION)
		blockbuffer.add_block_to_pool(blk);
	else if(blk.pre_type==blk.INTER_PREDICTION)
		blockbuffer.add_block_to_pool(blk);
	else
	{}
	return 0;
}

int encode(Frame &frame,AVFormat &para,PKT &pkt,vector<FrameBufferPool>  &frame_pool)
{
	BlockBufferPool  decode_buffer_Y;
	BlockBufferPool  decode_buffer_U;
	BlockBufferPool  decode_buffer_V;


	Block block_intra(para.block_height,para.block_width);
	Block block_inter(para.block_height,para.block_width);
	double min_block_intra;
	double min_block_inter;
	int block_num = frame.Yblock.size();
	int uvblock_num = frame.Ublock.size();
	for(int i=0;i<block_num;++i){
		// encode Y

		Block& input_block = frame.Yblock[i];	
		predict_block_inter(input_block,block_inter,decode_buffer_Y,min_block_inter);
		predict_block_intra(input_block,block_intra,frame_pool[0],min_block_intra);

		if(min_block_intra>min_block_inter)
		{
			pkt.Ylist.push_back(block_inter);
			block_decode_save(block_inter, decode_buffer_Y,frame_pool[0]);		
		}
		else
			{
			pkt.Ylist.push_back(block_intra);
				
			block_decode_save(block_intra, decode_buffer_Y,frame_pool[0]);	
		
		}
	
	}
	

	//encode U
	for(int i=0;i<uvblock_num;++i){
		Block& input_block = frame.Ublock[i];	
		predict_block_inter(input_block,block_inter,decode_buffer_U,min_block_inter);
		predict_block_intra(input_block,block_intra,frame_pool[1],min_block_intra);

		if(min_block_intra>min_block_inter)
		{
			pkt.Ulist.push_back(block_inter);
			block_decode_save(block_inter, decode_buffer_U,frame_pool[1]);		
		}
		else
			{
			pkt.Ulist.push_back(block_intra);
			block_decode_save(block_intra, decode_buffer_U,frame_pool[1]);	
		}
		//encode V
		Block& input_block1 = frame.Vblock[i];	
		predict_block_inter(input_block1,block_inter,decode_buffer_V,min_block_inter);
		predict_block_intra(input_block1,block_intra,frame_pool[2],min_block_intra);

		if(min_block_intra>min_block_inter)
		{
			pkt.Vlist.push_back(block_inter);
			block_decode_save(block_inter, decode_buffer_V,frame_pool[2]);		
		}
		else
			{
			pkt.Vlist.push_back(block_intra);
			block_decode_save(block_intra, decode_buffer_V,frame_pool[2]);	
		}
	}
	
	frame_pool[0].add_frame_to_pool(decode_buffer_Y);
	frame_pool[1].add_frame_to_pool(decode_buffer_U);
	frame_pool[2].add_frame_to_pool(decode_buffer_V);
	decode_buffer_Y.clear();
	decode_buffer_U.clear();
	decode_buffer_V.clear();
	return 0;
}