

#include "predict.h"
#include "encode.h"
#include <queue>
#include "quantization.h"
#include <time.h>
#include "Pattern.h"
#include "ResidualBlock.h"
#include "dct4x4.h"
#include <vector>
using namespace std;


const int N=3;  //预测方法的数量


int block_min(double *p,int N,int &pos,double &value)
{
	
	if(N<1)
		return -1;
	value=p[0];
	pos=0;
	for(int i=1;i<N;++i)
	{
		if(p[i]<value)
		{pos=i;value=p[i];}
	}
	return 0;
}
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

double dp_encode(Block & block, ResidualBlock & residual_block, int start_r, int start_l, int h, int w, BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool) {
	static map<int,Node> dp;
	double score0 = DBL_MAX, score1 = DBL_MAX, score2 = DBL_MAX;
	int pattern_num = 2;//Pattern::get_pattern_number();
	for(int i = 0; i < pattern_num; ++i){

	/*	intra_predict(block,residual_block,start_r,start_l, h, w,block_buffer_pool,Pattern::getPattern(i))

		quantization(start_r,start_l, h, w , residual_block , para);

		Reversed_quantization(start_r,start_l, h, w , residual_block , para);

		reversed_intra_predict(block,residual_block,start_r,start_l, h, w,block_buffer_pool,Pattern::getPattern(i));

		int score = compute_coef(residual_block,start_r,start_l, h, w);

		if(score < score0){
			update_node();
		}*/

	}
	//inter_predict
	

	/*
	if (w > 8) {
		double score1 = dp_encode(block, residual_block, start_r, start_l, h, w/2, block_buffer_pool, frame_pool) + 
						dp_encode(block, residual_block, start_r, start_l + w/2, h, w/2, block_buffer_pool, frame_pool);
	}

	if (h >= 8) {
		double score2 = dp_encode(block, residual_block, start_r, start_l, h/2, w, block_buffer_pool, frame_pool) + 
						dp_encode(block, residual_block, start_r + h/2, start_l, h/2, w, block_buffer_pool, frame_pool);
	}

	*/
	/*if (score0 <= score1 && score0 <= score2) {
		decode(node,residual_block,block_buffer_pool)
		leaf_vector.push_back(Node());
		method_vector.push_back(0);

	} else if (score1 <= score2) {
		method_vector.push_back(1);
	} else {
		method_vector.push_back(2);
	}*/
	return 0;
}


inline int encode_one_block(Block & block,ResidualBlock & residual_block,AVFormat &para,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool){

	DCT4x4Solver dct_trans; //dct 变换
	//dct_trans.set_()  

	Block save_block=block;  //保存block
	residual_block.block_id = block.block_id;
	residual_block.block_type = block.block_type;
	
	int h,w;
	block.getBlockSize(para,h,w);
//--------------------------------------------------------------------------
	//vector<uint8_t> data= block.data;
	
	double _16_16_pattern[N]; //N是模式数
	double _8_8_pattern[4][N];
	
	for(int i=0;i<N;++i){
	 _16_16_pattern[i]=Pattern::predict(block,residual_block,0,0,h-1,w-1,block_buffer_pool,i,para); //需要返回一个类型
	}
	int pos16;
	double min_value16;
	block_min(_16_16_pattern,N,pos16,min_value16);  //重载max函数计算16*16模式中的最小值

	//8*8
	int pos8;
	double value8;
	int _8_8_type[4];
	double min_value8=0;
	vector<pos> position;
	position.push_back(pos(0,0,h/2-1,w/2-1));
	position.push_back(pos(0,w/2,h/2-1,w-1));
	position.push_back(pos(h/2,0,h-1,w/2-1));		
	position.push_back(pos(h/2,w/2,h-1,w-1));

	for(int i=0;i<4;++i){
		for(int j=0;j<N;++j){
			_8_8_pattern[i][j]=Pattern::predict(block,residual_block,position[i].left_top_x,position[i].left_top_y,
											position[i].right_bottom_x,position[i].right_bottom_y,block_buffer_pool,j,para);
		}
		block_min(_8_8_pattern[i],N,pos8,value8);
		Pattern::predict(block,residual_block,position[i].left_top_x,position[i].left_top_y,
											position[i].right_bottom_x,position[i].right_bottom_y,block_buffer_pool,pos8,para);

		quantization(position[i].left_top_x,position[i].left_top_y,
					position[i].right_bottom_x,position[i].right_bottom_y,residual_block,para);
		Reverse_quantization(position[i].left_top_x,position[i].left_top_y,
							position[i].right_bottom_x,position[i].right_bottom_y,residual_block,para);
		Pattern::de_predict(block,residual_block,position[i].left_top_x,position[i].left_top_y,
											position[i].right_bottom_x,position[i].right_bottom_y,block_buffer_pool,pos8,para);
		min_value8+=value8;
		_8_8_type[i]=pos8;
	}

	ResidualBlock temp;//临时的返回值，作为量化后的残差。
	if(min_value8>=min_value16)  //采用16*16进行预测
	{
		residual_block.type_slice=0;
		residual_block.node.push_back(pos16);
		Pattern::predict(block,residual_block,0,0,h-1,w-1,block_buffer_pool,pos16,para);
		quantization(0 ,0 ,h-1 ,w-1 , residual_block , para);
		//dct_trans.dct(int16_t *data, h, w);
		temp=residual_block;
		//dct_trans.dct(0, 0, h, w, data);
		//dct_trans.dct(int16_t *data,int left_row,int left_col,int right_row,int right_col,int h,int w);
		Reverse_quantization(0 ,0 ,h-1 ,w-1 , residual_block , para);	
		Pattern::de_predict(block,residual_block,0,0,h-1,w-1,block_buffer_pool,pos16,para);
		residual_block=temp;  //  ying gai ding yi kao bei gou zao han shu
	}
	else
	{                     // 采用8*8进行预测
	
		int i_offset = (residual_block.block_id / para.block_num_per_row) * h;	// 当前block的起始像素所在Frame的行
		int j_offset = (residual_block.block_id % para.block_num_per_row) * w;
		residual_block.type_slice=1;
		for(int i=0;i<4;++i){
		Pattern::predict(save_block,residual_block,position[i].left_top_x,position[i].left_top_y,position[i].right_bottom_x,position[i].right_bottom_y,block_buffer_pool,_8_8_type[i],para);
		quantization(position[i].left_top_x,position[i].left_top_y,position[i].right_bottom_x,position[i].right_bottom_y,residual_block,para);
		residual_block.node.push_back(_8_8_type[i]);		
		if(i==0)
			temp=residual_block;
		else
		{
			for (int m = position[i].left_top_x; m <= position[i].right_bottom_x &&m + i_offset < para.height;m++)
			{
				for (int n = position[i].left_top_y; n <= position[i].right_bottom_y &&n + j_offset < para.width; n++) 
				{
					temp.data[m * w + n] = residual_block.data[m * w + n];
				}
			}
			temp.node.push_back(_8_8_type[i]);
		}
		Reverse_quantization(position[i].left_top_x,position[i].left_top_y,position[i].right_bottom_x,position[i].right_bottom_y,residual_block,para);
		Pattern::de_predict(save_block,residual_block,position[i].left_top_x,position[i].left_top_y,position[i].right_bottom_x,position[i].right_bottom_y,block_buffer_pool,_8_8_type[i],para);
		
		}
		residual_block=temp;
		block=save_block;

	}

	return 0;
}
inline int encode_one_component(vector<Block> & blocks, std::vector<ResidualBlock> & residual_blocks,AVFormat &para,FrameBufferPool & frame_pool){

	BlockBufferPool  * decode_buffer = new BlockBufferPool(para.height,para.width);
	int block_height,block_width;
	blocks[0].getBlockSize(para,block_height,block_width);

	for(int i=0;i<blocks.size();++i){
		residual_blocks[i].clear();
		encode_one_block(blocks[i],residual_blocks[i],para,*decode_buffer,frame_pool);		
	}
	frame_pool.add_frame_to_pool(decode_buffer);
	return 0;
}
int encode(Frame &frame,AVFormat &para,PKT &pkt,vector<FrameBufferPool>  &frame_pool){
	encode_one_component(frame.Yblock,pkt.Ylist,para,frame_pool[0]);
	encode_one_component(frame.Ublock,pkt.Ulist,para,frame_pool[1]);
	encode_one_component(frame.Vblock,pkt.Vlist,para,frame_pool[2]);
	return 0;
}

