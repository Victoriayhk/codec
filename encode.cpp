#include "encode.h"
#include <stdint.h>
#include <queue>

auto *decode_frame_buffer=new queue<Frame>();
int block_decode_save(block* blk,queue<block>* que)
{ 

}

int encode(Frame* frame,av_encode_para* para,pkt* pkt)
{

	auto *decode_buffer_Y=new queue<block>();
	auto *decode_buffer_U=new queue<block>();
	auto *decode_buffer_V=new queue<block>();

	block_num=frame->block_num;
	block* block_intra=new block;
	block* block_inter=new block;
	int min_block_intra;
	int min_block_inter;
	for(int i=0;i<block_num;++i){
// encode Y
	block input_block=frame->Y_que[i];	
	//predic_block_inter();
	//predic_block_intra();
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
	delete decode_buffer_Y;
	delete decode_buffer_U;
	delete decode_buffer_V;


}