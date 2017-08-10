#include "Pattern.h"
#include <math.h>

// Pattern::FUN Pattern::getPattern(int pattern){
//	return (FUN)patterns[pattern];
//}
//int Pattern::get_pattern_number(){
//	return pattern_number; 
//}

extern vector<int> Square_table;
#define IN_AREA(i, j, h, w) (0 <= i && i < h && 0 <= j && j <= w) 

double Pattern::predict(Block& block,ResidualBlock & r_block,int start_r,int start_c,int end_r,int end_c, BlockBufferPool& whole_frame,int pattern_type, AVFormat &para){
	int block_w, block_h;
	r_block.getBlockSize(para, block_w, block_h);							// 当前block的宽和高
	int i_offset = (r_block.block_id / para.block_num_per_row) * block_h;	// 当前block的起始像素所在Frame的行
	int j_offset = (r_block.block_id % para.block_num_per_row) * block_w;	// 当前block的起始像素所在Frame的列
	double diff = 0;		
	for (int i = start_r; i <= end_r && i + i_offset < para.height; i++)
	{
		int pos=i*block_w;
		for (int j = start_c; j <= end_c && j + j_offset < para.width; j++) 
		{
			int16_t block_pool=128;
			int c_i=-1,c_j=-1;  //相对于每一帧的坐标
			if(pattern_type == 0&&i_offset>0)// 竖向预测, 参考小块上方的一条像素	
			{
				c_i=start_r+i_offset - 1;
				c_j=j_offset + j;
				block_pool=(int16_t)whole_frame.getValue(c_i, c_j);
			}
			else if(pattern_type == 1&&j_offset>0)//横向预测, 参考小块左方的一条像素	
			{
				c_i=i_offset + i;
				c_j=j_offset - 1+start_c;
				block_pool=(int16_t)whole_frame.getValue(c_i, c_j);
			}
			else if (pattern_type == 2)//DC预测模式	
			{
				int left_row=0,left_col=0;
				int top_row=0,top_col=0;
				int left=128;int top=128;
				if(i_offset>0)
				{
					top_row=start_r+i_offset - 1;
					top_col=j_offset + j;
					top=whole_frame.getValue(top_row, top_col);
				}
				if(j_offset>0)
				{
					left_row=i_offset + i;
					left_col=j_offset - 1+start_c;
					left=whole_frame.getValue(left_row,left_col);
				}
				block_pool=(top+left)/2;			
			}
			else if (pattern_type == 3&&(i_offset>0&&j_offset>0))//右斜45度预测, 参考斜左上方的一条像素	
			{
				if(i-start_r>j-start_c)
				{		//c_i=i-j+start_c+start_c-start_r-1+i_offset;
						c_i=i-j+start_c-1+i_offset;
						c_j=start_c-1+j_offset;
		        }
				else
				{
						c_i=start_r-1+i_offset;
						//c_j=j-i-start_c+start_r+start_r-1+j_offset;
						c_j=j-i+start_r-1+j_offset;
				}
				if(c_i>=720||c_j>=1280){
					printf("vector 越界\n");
					system("pause");
				}
				block_pool=(int16_t)whole_frame.getValue(c_i, c_j);
			}
			else
			{
			}
			r_block.data[pos + j] = (int16_t)block.data[pos + j] - block_pool;
			diff +=  Square_table[abs(r_block.data[pos + j])];
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
		int pos=i*block_w;
		for (int j = start_c; j <= end_c && j + j_offset < para.width; j++) 
		{
			int16_t block_pool=128;
			int c_i=-1,c_j=-1;
			if(pattern_type == 0&&i_offset>0)// 竖向反预测, 参考小块上方的一条像素	
			{
				c_i=start_r+i_offset - 1;
				c_j=j_offset + j;
				block_pool=(int16_t)b_pool.getValue(c_i, c_j);
			}
			else if(pattern_type == 1&&j_offset>0)//横向反预测, 参考小块上方的一条像素	
			{
				c_i=i_offset + i;
				c_j=j_offset - 1+start_c;
				block_pool=(int16_t)b_pool.getValue(c_i, c_j);
			}
			else if (pattern_type == 2)//DC预测模式	
			{
				int left_row=0,left_col=0;
				int top_row=0,top_col=0;
				int left=128;int top=128;
				if(i_offset>0)
				{
					top_row=start_r+i_offset - 1;
					top_col=j_offset + j;
					top=(int16_t)b_pool.getValue(top_row, top_col);
				}
				if(j_offset>0)
				{
					left_row=i_offset + i;
					left_col=j_offset - 1+start_c;
					left=(int16_t)b_pool.getValue(left_row,left_col);
				}
				block_pool=(top+left)/2;
			}
			else if (pattern_type == 3&&i_offset>0&&j_offset>0)//右斜45度预测, 参考斜左上方的一条像素	
			{
				if(i-start_r>j-start_c)
				{
						//c_i=i-j+start_c+start_c-start_r-1+i_offset;
						c_i=i-j+start_c-1+i_offset;
						c_j=start_c-1+j_offset;
		        }
				else
				{
						c_i=start_r-1+i_offset;
						//c_j=j-i-start_c+start_r+start_r-1+j_offset;
						c_j=j-i+start_r-1+j_offset;

				}
				block_pool=(int16_t)b_pool.getValue(c_i, c_j);
			}
			else
			{
			}
			blk.data[pos + j] = (int16_t)r_block.data[pos + j] + block_pool;
			b_pool.setValue(i_offset + i, j_offset +j, blk.data[pos + j]);
		}
	}
}



/*
* 从包含.getValue(i, j)/.get_width()/.get_height()方法的实体中, 读入.getValue(i, j)的数据
* 如果i, j越界, 则返回128
*/
int16_t get_value_or_128(BlockBufferPool &sort_of_array, int i, int j) {
	if (i < 0 || i >= sort_of_array.get_height() || j < 0 || j >= sort_of_array.get_width()) return 128;
	else return sort_of_array.getValue(i, j);
}

//template<T>
//inline void check_mv_if_better(InterMV &des, const InterMV &org, T &des_diff, const T &org_dff) {
//	if (org_diff < des_diff) {
//		des_diff = org_diff;
//		memcpy(des, org, sizeof(InterMV));
//	}
//}

double Pattern::calc_SAD_inter(const Block &block, int start_r, int start_c, int end_r, int end_c,
	int r_i, int r_j, int block_w, int i_offset, int j_offset,
	BlockBufferPool & bpool) {

	double diff = 0;
	for (int i = start_r; i <= end_r; i++) {		// 枚举块的行
		for (int j = start_c; j <= end_c; j++) {	// 枚举块的列
			int c_i = i_offset + (i + r_i);			// 参考块的行坐标
			int c_j = j_offset + (j + r_j);			// 参考块的纵坐标
			int tmp = (int16_t)block.data[i * block_w + j] - get_value_or_128(bpool, c_i, c_j);
			diff += Square_table[abs(tmp)];
		}
	}
	return diff;
}

void Pattern::predict_inter_sub(const Block &block,ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c,
	int r_i, int r_j, int block_w, int i_offset, int j_offset,
	BlockBufferPool & bpool) {
	for (int i = start_r; i <= end_r; i++) {		// 枚举块的行
		for (int j = start_c; j <= end_c; j++) {	// 枚举块的列
			int c_i = i_offset + (i + r_i);			// 参考块的行坐标
			int c_j = j_offset + (j + r_j);			// 参考块的纵坐标
			r_block.data[i * block_w + j] = (int16_t)block.data[i * block_w + j] - get_value_or_128(bpool, c_i, c_j);
		}
	}
}

double Pattern::inter_predict(Block& block, ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c, FrameBufferPool &f_pool, InterMV &inter_mv, AVFormat &para) {
	int search_dir[4][2] = {
		-1, 0,
		0, -1,
		0, 1,
		1, 0,
	};
	int frame_width = para.width;
	int frame_height = para.height;
	int block_w, block_h;
	r_block.getBlockSize(para, block_w, block_h);
	int i_offset = (r_block.block_id / para.block_num_per_row) * block_h;	// 当前block的起始像素所在Frame的行
	int j_offset = (r_block.block_id % para.block_num_per_row) * block_w;	// 当前block的起始像素所在Frame的列

	if(f_pool.size()<=1)
		return -1.0;
	double diff = 1e20;
	int best_fi;
	std::pair<int, int> best_mv, mv, nmv;
	std::map< std::pair<int, int>, double> hash;
	for (int fi = f_pool.size() - 2; fi >= 0; fi--) {
		mv.first = 0;
		mv.second = 0;
		int cur_diff = calc_SAD_inter(block, start_r, start_c, end_r, end_c, mv.first, mv.second, block_w, i_offset, j_offset, f_pool[fi]);
		hash.clear();
		hash[mv] = cur_diff;
		int search_count=0;
		int step_length = 16;
		while (search_count<10 && step_length > 0) {
			int best_d_p = -1, best_new_diff = -1;
			for (int d_p = 0; d_p < 4; d_p ++) {
				nmv.first = mv.first + search_dir[d_p][0] * step_length;
				nmv.second = mv.second + search_dir[d_p][1] * step_length;
				int new_diff;
				if (hash.find(nmv) != hash.end()) {
					new_diff = hash[nmv];
				} else {
					new_diff = calc_SAD_inter(block, start_r, start_c, end_r, end_c, nmv.first, nmv.second, block_w, i_offset, j_offset, f_pool[fi])+para.eva_bit; //帧间预测量化参数
					hash[nmv] = new_diff;
				}
				if (new_diff > best_new_diff) {
					best_new_diff = new_diff;
					best_d_p = d_p;
				}
			}
			if (best_d_p != -1) {
				mv.first += search_dir[best_d_p][0];
				mv.second += search_dir[best_d_p][1];
				cur_diff = best_new_diff;
			} else {
				step_length >>= 1;
			};
			
			++search_count;
		}

		if (cur_diff < diff) {
			diff = cur_diff;
			best_mv = mv;
			best_fi = fi;
		} else {
			break;
		}
	}

	//predict_inter_sub(block,r_block,start_r, start_c, end_r, end_c, best_mv.first, best_mv.second, block_w, i_offset, j_offset, f_pool[best_fi]);
	inter_mv.fi = best_fi;
	inter_mv.mv = best_mv;
	return diff;
}


void Pattern::inter_predict_setvalue(Block& block, ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c, FrameBufferPool &f_pool, InterMV &inter_mv, AVFormat &para) {
	int search_dir[4][2] = {
		-1, 0,
		0, -1,
		0, 1,
		1, 0,
	};
	int frame_width = para.width;
	int frame_height = para.height;
	int block_w, block_h;
	r_block.getBlockSize(para, block_w, block_h);
	int i_offset = (r_block.block_id / para.block_num_per_row) * block_h;	// 当前block的起始像素所在Frame的行
	int j_offset = (r_block.block_id % para.block_num_per_row) * block_w;	// 当前block的起始像素所在Frame的列

	predict_inter_sub(block,r_block,start_r, start_c, end_r, end_c, inter_mv.mv.first, inter_mv.mv.second, block_w, i_offset, j_offset, f_pool[inter_mv.fi]);
}


void Pattern::inter_predict_reverse(Block& block, ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c, FrameBufferPool &f_pool,BlockBufferPool & block_buffer_pool, InterMV &inter_mv, AVFormat &para) {
	int search_dir[4][2] = {
		-1, 0,
		0, -1,
		0, 1,
		1, 0,
	};
	int frame_width = para.width;
	int frame_height = para.height;
	int block_w, block_h;
	r_block.getBlockSize(para, block_w, block_h);
	int i_offset = (r_block.block_id / para.block_num_per_row) * block_h;	// 当前block的起始像素所在Frame的行
	int j_offset = (r_block.block_id % para.block_num_per_row) * block_w;	// 当前block的起始像素所在Frame的列
	
	BlockBufferPool &b_pool = f_pool[inter_mv.fi];
	for (int i = start_r; i <= end_r; i++) {				// 枚举块的行
		for (int j = start_c; j <= end_c; j++) {			// 枚举块的列
			int c_i = i_offset + (i + inter_mv.mv.first);	// 参考块的行坐标
			int c_j = j_offset + (j + inter_mv.mv.second);	// 参考块的纵坐标
			block.data[i * block_w + j]=r_block.data[i * block_w + j]+get_value_or_128(b_pool, c_i, c_j);
			block_buffer_pool.setValue(i+i_offset, j+j_offset,block.data[i * block_w + j]);
		}
	}
}


