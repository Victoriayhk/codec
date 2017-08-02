#include "encode.h"
#include <stdint.h>
#include <queue>

using namespace std;

//auto *decode_frame_buffer=new queue<Frame>();


int block_decode_save(Block* blk,queue<Block>* que)
{ 

}

int encode(Frame* frame,av_encode_para* para,pkt* pkt,FrameBufferPool * frame_pool)
{
	/*
	auto *decode_buffer_Y=new queue<block>();
	auto *decode_buffer_U=new queue<block>();
	auto *decode_buffer_V=new queue<block>();
	*/
	BlockBufferPool block_buffer_pool;


	int block_num = frame->block_num;
	Block* block_intra = new Block;
	Block* block_inter = new Block;
	double min_block_intra;
	double min_block_inter;

	for(int i=0;i<block_num;++i){

		// encode Y
		Block * input_block = frame->Y_que[i];	
		predic_block_inter(input_block,block_inter,block_buffer_pool,min_block_inter);
		predic_block_intra(input_block,block_intra,frame_pool,min_block_intra);

		if(min_block_intra>min_block_inter)
		{
			pkt->Y_que->push_back(*block_intra);
			block_decode_save(block_intra, *decode_buffer_Y);		
		}
		else
			{
			pkt->Y_que->push_back(*block_inter);
			block_decode_save(block_inter, *decode_buffer_Y);	
		}
	//encode U
		block input_block=frame->U_que[i];	
		//predic_block_inter();
		//predic_block_intra();
		if(min_block_intra>min_block_inter)
		{
			pkt->U_que->push_back(*block_intra);
			block_decode_save(block_intra, *decode_buffer_U);		
		}
		else
			{
			pkt->U_que->push_back(*block_inter);
			block_decode_save(block_inter, *decode_buffer_U);	
		}
	//encode V
		block input_block=frame->V_que[i];	
		//predic_block_inter();
		//predic_block_intra();
		if(min_block_intra>min_block_inter)
		{
			pkt->V_que->push_back(*block_intra);
			block_decode_save(block_intra, *decode_buffer_V);		
		}
		else
			{
			pkt->V_que->push_back(*block_inter);
			block_decode_save(block_inter, *decode_buffer_V);	
		}
	}
	delete block_inter;
	delete block_intra;
}