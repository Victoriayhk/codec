
//#define SIMPLE_SLICE

/**
** 简单宏块划分，用于树形结构完成之前的各模块验证，划分
** 董辰辰
*/
#ifdef SIMPLE_SLICE


#include "predict.h"
#include "encode.h"
#include <queue>
#include "quantization.h"
#include <time.h>
#include "Pattern.h"
#include "ResidualBlock.h"
#include "dct.h"
#include <vector>
#include "dctInterface.h"
using namespace std;

const int N=3;  //帧内预测方法的数量 在树形划分中整合到AVFormat参数中这里未设置




/*
position.push_back(pos(0,0,h/2-1,w/2-1));
position.push_back(pos(0,w/2,h/2-1,w-1));
position.push_back(pos(h/2,0,h-1,w/2-1));		
position.push_back(pos(h/2,w/2,h-1,w-1));
采用8*8划分四个小块的坐标
*/

extern vector<pos> position_Y;
extern vector<pos> position_UV;



/*
** 用于返回计算数组中最大元素的位置和最大元素的值
** 参数
** p 数组指针
** N 数组元素的个数
** pos 返回的元素的位置
** value 返回的元素的最大值
** 董辰辰
*/
int block_min(int *p,int N,int &pos,int &value)
{
	if(N<1)                   //数组元素个数不能小于1
		return -1; 
	value=p[0];
	pos=0;
	for(int i=1;i<N;++i)
	{
		if(p[i]<value)
		{
			pos=i;
			value=p[i];
		}
	}
	return 0;
}

/*
** 非树形划分方式下计算16*16（UV8*8）划分中的最优预测，使用帧内还是帧间
** 参数：
** block 输入的宏块用于计算残差
** residual_block 输出的残差模块 
**  whole_frame 帧内预测用到的缓冲池
** FrameBufferPool 帧间预测用到的缓冲池
** h,w 宏块的长宽
** para  系统设定的编码参数
** best_value_16 返回16*16划分下的rdo最小值
** 董辰辰
*/


mini_block cal_best_predict_16(Block block,ResidualBlock r_block,int h,int w,BlockBufferPool& whole_frame, FrameBufferPool & frame_pool，AVFormat &para,int &best_value_16)
{
	mini_block res;
	
   /*
   ** 计算帧内预测的最小值和对应的模式
   */
   	double 	_16_16_pattern[N]; //帧内预测的返回值
	for(int i=0;i<N;++i){
	 _16_16_pattern[i]=Pattern::predict(block,residual_block,0,0,h-1,w-1,block_buffer_pool,i,para); //返回所有帧内预测的rdo值
	}

	int pos16;
	double intra_value16;
	block_min(_16_16_pattern,N,pos16,intra_value16);  //重载max函数计算16*16模式中的最小值
	
	
   /*
   ** 计算帧间预测的最小值和对应运动向量
   */
	double 	inter_value16; 
	InterMV inter_mv;
	inter_16_16=Pattern::inter_predict(block, residual_block, 0, 0, h-1, w-1, frame_pool, inter_mv, para);

	if(inter_value16>0&&inter_value16< intra_value16)           //帧间预测最优
	{
		res.predit_type=1;
		res.inter_node=inter_mv;	
		best_value_16=inter_value16;
	}
	else{                                                      //帧内预测最优
		res.predit_type=0;
		res.intra_type=pos16;
		best_value_16=intra_value16;
	}

	return res;

}


/*
** 非树形划分方式下计算8*8（UV4*4）划分中的最优预测，使用帧内还是帧间
** 参数：
** block 输入的宏块用于计算残差
** residual_block 输出的残差模块 
** whole_frame 帧内预测用到的缓冲池
** FrameBufferPool 帧间预测用到的缓冲池
** para  系统设定的编码参数
** best_value_8 返回8*8划分下的rdo最小值
** position 保存8*8或者4*4（UV）划分坐标
** 董辰辰
*/

vector<mini_block> cal_best_predict_8(Block block,ResidualBlock r_block,BlockBufferPool& whole_frame, FrameBufferPool & frame_pool，AVFormat &para,int &best_value_8，vector<pos>& position)
{

	vector<mini_block> vec_8_8;
	double min_value8 = 0; //8*8min_value
	
	
	/*
	**帧内预测参数
	*/
	int pos8;
	double _8_8_pattern[4][N];  //四个小块每个N中帧内预测值进行保存
	double intra_value8;		//intra min value
	

	/*
	**帧间预测参数
	*/
	InterMV inter_mv;
	double inter_value8;   //inter min value


	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < N; ++j)
		{
			_8_8_pattern[i][j] = Pattern::predict(block, residual_block, position[i].left_top_x, position[i].left_top_y,
												  position[i].right_bottom_x, position[i].right_bottom_y, block_buffer_pool, j, para);
		}
		block_min(_8_8_pattern[i], N, pos8, intra_value8); //  计算某个小块帧内预测的最好值和对应的位置

		double inter_value8 = Pattern::inter_predict(block, residual_block, position[i].left_top_x, position[i].left_top_y,
													 position[i].right_bottom_x, position[i].right_bottom_y, frame_pool, inter_mv, para); //小模块采用帧间的最小值
		
				
        if (inter_value8 > 0 && inter_value8 < intra_value8)	//如果帧间压缩效果更好
		{
			mini_block temp;
			temp.predit_type = 1;
			temp.inter_node = inter_mv;
			vec_8_8.push_back(temp);                        //保存这个小模块采用的预测方式

			/*
			** 对这个小模块完成整个的帧间编解码过程 预测 量化 dct 变换 
			*/
			Pattern::inter_predict(block, residual_block, position[i].left_top_x, position[i].left_top_y,
								   position[i].right_bottom_x, position[i].right_bottom_y, frame_pool, inter_mv, para);
			dct_trans(residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, h, w);			quantization(position[i].left_top_x, position[i].left_top_y,
						 position[i].right_bottom_x, position[i].right_bottom_y, residual_block, para);
			Reverse_quantization(position[i].left_top_x, position[i].left_top_y,
								 position[i].right_bottom_x, position[i].right_bottom_y, residual_block, para);
			reverse_dct_trans(residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, h, w);
			Pattern::inter_predict_reverse(block, residual_block, position[i].left_top_x, position[i].left_top_y,
										   position[i].right_bottom_x, position[i].right_bottom_y, frame_pool, block_buffer_pool, inter_mv, para);
	
			min_value8 +=inter_value8;
	
		}
		else //如果小模块帧内压缩效果更好
		{
			mini_block temp;
			temp.predit_type = 0;   // intra predit
			temp.intra_type = pos8; //intra predit type
			vec_8_8.push_back(temp);

			/*
			** 对这个小模块完成整个的帧内编解码过程
			*/
			Pattern::predict(block, residual_block, position[i].left_top_x, position[i].left_top_y,
							 position[i].right_bottom_x, position[i].right_bottom_y, block_buffer_pool, pos8, para);
			dct_trans(residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, h, w);
			quantization(position[i].left_top_x, position[i].left_top_y,
						 position[i].right_bottom_x, position[i].right_bottom_y, residual_block, para);
			Reverse_quantization(position[i].left_top_x, position[i].left_top_y,
								 position[i].right_bottom_x, position[i].right_bottom_y, residual_block, para);
			reverse_dct_trans(residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, h, w);
			Pattern::de_predict(block, residual_block, position[i].left_top_x, position[i].left_top_y,
								position[i].right_bottom_x, position[i].right_bottom_y, block_buffer_pool, pos8, para);

			min_value8 +=intra_value8;
		}
	}
	best_value_8=min_value8；
	return vec_8_8；
}


/*
** 非树形划分方式下计算对16*16划分下进行编码
** 参数：
** mini_blk 编码宏块存储信息
** block 输入的宏块用于计算残差
** residual_block 输出的残差模块 
** whole_frame 帧内预测用到的缓冲池
** FrameBufferPool 帧间预测用到的缓冲池
** para  系统设定的编码参数
** 董辰辰
*/

int encode_16_slice(mini_block &mini_blk,Block & block,ResidualBlock & residual_block,AVFormat &para,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool,vector<pos> &position)
{
	residual_block.child_block.push_back(mini_blk);    //将模块的信息存储在节点里

	if (mini_block.predit_type == 0) //采用16*16帧内预测
	{

		Pattern::predict(block, residual_block, 0, 0, h - 1, w - 1, block_buffer_pool, mini_blk.intra_type, para);
		dct_trans(residual_block, 0, 0, h - 1, w - 1, h, w);
		quantization(0, 0, h - 1, w - 1, residual_block, para);
		temp = residual_block;
		Reverse_quantization(0, 0, h - 1, w - 1, residual_block, para);
		reverse_dct_trans(residual_block, 0, 0, h - 1, w - 1, h, w);
		Pattern::de_predict(block, residual_block, 0, 0, h - 1, w - 1, block_buffer_pool, mini_blk.intra_type, para);
		residual_block = temp; //  ying gai ding yi kao bei gou zao han shu
	}
	else 							// 采用16*16的帧间预测
	{

		Pattern::inter_predict(block, residual_block, 0, 0, h - 1, w - 1, frame_pool, mini_blk.inter_node, para);
		dct_trans(residual_block, 0, 0, h - 1, w - 1, h, w);
		quantization(0, 0, h - 1, w - 1, residual_block, para);
		temp = residual_block;
		Reverse_quantization(0, 0, h - 1, w - 1, residual_block, para);
		reverse_dct_trans(residual_block, 0, 0, h - 1, w - 1, h, w);
		Pattern::inter_predict_reverse(block, residual_block, 0, 0, h - 1, w - 1, frame_pool, block_buffer_pool, mini_blk.inter_node, para);
		residual_block = temp; //  更新编码之后的值
	}
	return 0;
}



/*
** 非树形划分方式下计算对8*8划分下进行编码
** 参数：
** vec_8_8 编码的4个宏块存储信息
** block 输入的宏块用于计算残差
** residual_block 输出的残差模块 
** whole_frame 帧内预测用到的缓冲池
** FrameBufferPool 帧间预测用到的缓冲池
** para  系统设定的编码参数
** 董辰辰
*/

int encode_8_slice(vector<mini_block> &vec_8_8,Block & block,ResidualBlock & residual_block,AVFormat &para,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool，vector<pos> &position)
{
	ResidualBlock temp;   //对于模块划分
	residual_block.child_block = vec_8_8;
	int i_offset = (residual_block.block_id / para.block_num_per_row) * h; // 当前block的起始像素所在Frame的行
	int j_offset = (residual_block.block_id % para.block_num_per_row) * w; // 当前block的起始像素所在Frame的列
	for (int i = 0; i < 4; ++i)
	{
		if (vec_8_8[i].predit_type == 0) //对8*8的帧内编解码
		{

			Pattern::predict(block, residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, block_buffer_pool, vec_8_8[i].intra_type, para);
			dct_trans(residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, h, w);
			quantization(position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, residual_block, para);
		
			/*
			** 在上面预测，dct，量化完成之后需要保存中间的残差的数据，防止进行逆变换之后数据发生转换
			*/			
			if (i == 0)                           
				temp = residual_block;
			else
			{
				for (int m = position[i].left_top_x; m <= position[i].right_bottom_x && m + i_offset < para.height; m++)
				{
					for (int n = position[i].left_top_y; n <= position[i].right_bottom_y && n + j_offset < para.width; n++)
					{
						temp.data[m * w + n] = residual_block.data[m * w + n];
					}
				}
			}

			Reverse_quantization(position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, residual_block, para);
			reverse_dct_trans(residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, h, w);
			Pattern::de_predict(block, residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, block_buffer_pool, vec_8_8[i].intra_type, para);
		}
		else //对8*8的帧间编解码
		{
			Pattern::inter_predict(block, residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, frame_pool,vec_8_8[i].inter_node, para);
			dct_trans(residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, h, w);
			quantization(position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, residual_block, para);
			if (i == 0)
				temp = residual_block;
			else
			{
				for (int m = position[i].left_top_x; m <= position[i].right_bottom_x && m + i_offset < para.height; m++)
				{
					for (int n = position[i].left_top_y; n <= position[i].right_bottom_y && n + j_offset < para.width; n++)
					{
						temp.data[m * w + n] = residual_block.data[m * w + n];
					}
				}
			}
			Reverse_quantization(position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, residual_block, para);
			reverse_dct_trans(residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, h, w);
			Pattern::inter_predict_reverse(block, residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, frame_pool, block_buffer_pool,vec_8_8[i].inter_node, para);
		}
	}
	residual_block = temp;
	return 0;
}


/*
** 非树形划分方式下的编码模块
** 参数：
** block 输入的宏块用于计算残差
** residual_block 输出的残差模块
** para  系统设定的编码参数
** block_buffer_pool 帧内预测用到的缓冲池
** FrameBufferPool 帧间预测用到的缓冲池
** 董辰辰
*/

inline int encode_one_block(Block & block,ResidualBlock & residual_block,AVFormat &para,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool){

	Block save_block=block;  				//保存block 防止在进行反预测的时候对block值进行改变
	residual_block.block_id = block.block_id;
	residual_block.block_type = block.block_type;
	int h,w;
	block.getBlockSize(para,h,w);           //获取block的长宽参数

	
	/*
	** 思路：首先计算16*16划分下帧间预测和帧内所有预测模式中最小值，并记录16*16最终采用帧内还是帧间预测，并保留相应参数
	*/

	mini_block mini_blk_16; //保存16*16划分下的最优模式选择，帧内或帧间。
	int  best_value_16;     //保存16*16划分下的最优值
	mini_blk_16=cal_best_predict_16(block,r_block,h,w,whole_frame,para，best_value_16);

	/*
	**  8*8划分下的最优模式计算
	*/
	vector<mini_block> mini_blk_8;  //保存8*8划分下的每个小块的最优模式选择，帧内或帧间。
	int  best_value_8;     			//保存8*8划分下的最优值

	if(block.block_type==Block::Y)
		mini_blk_8=cal_best_predict_8(block,r_block,whole_frame,para，best_value_8,position_Y);
	else
		mini_blk_8=cal_best_predict_8(block,r_block,whole_frame,para，best_value_8,position_UV);
	

	if(best_value_8>best_value_16){    //如果16*16最小值小于8*8最小值，采用16*16的划分
		residual_block.type_slice=0;   //采用第一种划分方式
		if(block.block_type==Block::Y)
			encode_16_slice(mini_blk_16,block,residual_block,para,block_buffer_pool,frame_pool,position_Y);
		else
			encode_16_slice(mini_blk_16,block,residual_block,para,block_buffer_pool,frame_pool,position_UV);
	}
	else      //采用8*8预测
	{
		residual_block.type_slice=1;   //采用第一种划分方式
		if(block.block_type==Block::Y)
			encode_8_slice(mini_blk_8,block,residual_block,para,block_buffer_pool,frame_pool,position_Y);
		else
			encode_8_slice(mini_blk_8,block,residual_block,para,block_buffer_pool,frame_pool,position_UV);
	}	
	return 0;
}

inline int encode_one_component(vector<Block> & blocks, std::vector<ResidualBlock> & residual_blocks,AVFormat &para,FrameBufferPool & frame_pool){

	BlockBufferPool & decode_buffer = frame_pool.new_back();
	int block_height,block_width;
	blocks[0].getBlockSize(para,block_height,block_width);

	for(int i=0;i<blocks.size();++i){
		residual_blocks[i].clear();
		encode_one_block(blocks[i],residual_blocks[i],para,decode_buffer,frame_pool);		
	}
	//frame_pool.add_frame_to_pool(decode_buffer);
	return 0;
}
int encode(Frame &frame,AVFormat &para,PKT &pkt,vector<FrameBufferPool*>  &frame_pool){
	encode_one_component(frame.Yblock,pkt.Ylist,para,*frame_pool[0]);
	encode_one_component(frame.Ublock,pkt.Ulist,para,*frame_pool[1]);
	encode_one_component(frame.Vblock,pkt.Vlist,para,*frame_pool[2]);
	return 0;
}

#endif