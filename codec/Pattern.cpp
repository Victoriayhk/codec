#include "Pattern.h"

// Pattern::FUN Pattern::getPattern(int pattern){
//	return (FUN)patterns[pattern];
//}
//int Pattern::get_pattern_number(){
//	return pattern_number; 
//}

#define IN_AREA(i, j, h, w) (0 <= i && i < h && 0 <= j && j <= w) 

double Pattern::predict(Block& block,ResidualBlock & r_block,int start_r,int start_c,int end_r,int end_c, BlockBufferPool& whole_frame,int pattern_type, AVFormat &para){
	int block_w, block_h;
	r_block.getBlockSize(para, block_w, block_h);							// 当前block的宽和高
	int i_offset = (r_block.block_id / para.block_num_per_row) * block_h;	// 当前block的起始像素所在Frame的行
	int j_offset = (r_block.block_id % para.block_num_per_row) * block_w;	// 当前block的起始像素所在Frame的列
	double diff = 0;		
	for (int i = start_r; i <= end_r && i + i_offset < para.height; i++)
	{
		for (int j = start_c; j <= end_c && j + j_offset < para.width; j++) 
		{
			int16_t block_pool;
			int c_i=-1,c_j=-1;  //相对于每一帧的坐标
			if(pattern_type == 0&&i_offset>0)// 竖向预测, 参考小块上方的一条像素	
			{
				c_i=start_r+i_offset - 1;
				c_j=j_offset + j;
			}
			else if(pattern_type == 1&&j_offset>0)//横向预测, 参考小块左方的一条像素	
			{
				c_i=i_offset + i;
				c_j=j_offset - 1+start_c;
			}
			else if (pattern_type == 2&&i_offset>0&&j_offset>0)//右斜45度预测, 参考斜左上方的一条像素	
			{
				if(i-start_r>j-start_c)
				{
						c_i=i-j+start_c+start_c-start_r-1+i_offset;
						c_j=start_r-1+j_offset;
		        }
				else
				{
						c_i=start_c-1+i_offset;
						c_j=j-i-start_c+start_r+start_r-1+j_offset;
				}					
			}
			if(c_i!=-1&&c_j!=-1)
				block_pool=(int16_t)whole_frame.getValue(c_i, c_j);
			else
				block_pool=128;// 本帧内上方无可参考的像素, 与128求差
			r_block.data[i * block_w + j] = (int16_t)block.data[i * block_w + j] - block_pool;
			diff += r_block.data[i * block_w + j] * r_block.data[i * block_w + j];
		}
	}
	return diff;
}

void Pattern::de_predict(Block& blk,ResidualBlock & r_block,int start_r,int start_c,int end_r,int end_c, BlockBufferPool& b_pool,int pattern_type, AVFormat &para) {
	int block_w, block_h;
	r_block.getBlockSize(para, block_w, block_h);							// 当前block的宽和高
	int i_offset = (r_block.block_id / para.block_num_per_row) * block_h;	// 当前block的起始像素所在Frame的行
	int j_offset = (r_block.block_id % para.block_num_per_row) * block_w;	// 当前block的起始像素所在Frame的列

	for (int i = start_r; i <= end_r && i + i_offset < para.height; i++)
	{
		for (int j = start_c; j <= end_c && j + j_offset < para.width; j++) 
		{
			int16_t block_pool;
			int c_i=-1,c_j=-1;
			if(pattern_type == 0&&i_offset>0)// 竖向反预测, 参考小块上方的一条像素	
			{
				c_i=start_r+i_offset - 1;
				c_j=j_offset + j;
			}
			else if(pattern_type == 1&&j_offset>0)//横向反预测, 参考小块上方的一条像素	
			{
				c_i=i_offset + i;
				c_j=j_offset - 1+start_c;
			}
			else if (pattern_type == 2&&i_offset>0&&j_offset>0)//右斜45度预测, 参考斜左上方的一条像素	
			{
				if(i-start_r>j-start_c)
				{
						c_i=i-j+start_c+start_c-start_r-1+i_offset;
						c_j=start_r-1+j_offset;
		        }
				else
				{
						c_i=start_c-1+i_offset;
						c_j=j-i-start_c+start_r+start_r-1+j_offset;
				}					
			}
			if(c_i!=-1&&c_j!=-1)
				block_pool=(int16_t)b_pool.getValue(c_i, c_j);
			else
				block_pool=128;// 本帧内上方无可参考的像素, 与128求差
			blk.data[i * block_w + j] = (int16_t)r_block.data[i * block_w + j] + block_pool;
			b_pool.setValue(i_offset + i, j_offset +j, blk.data[i * block_w + j]);
		}
	}
}
