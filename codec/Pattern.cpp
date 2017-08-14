#include "Pattern.h"
#include <math.h>

// Pattern::FUN Pattern::getPattern(int pattern){
//	return (FUN)patterns[pattern];
//}
//int Pattern::get_pattern_number(){
//	return pattern_number; 
//}

extern vector<int>  Square_table;
extern int TABLE[1500][1500];
#define IN_AREA(i, j, h, w) (0 <= i && i < h && 0 <= j && j <= w) 

/*
**	帧内预测
**  参数：
**  block：输入的预测块  r_block：返回的残差块 
**	start_r ... end_c :小宏块相对于大宏块的上下左右的坐标
**  whole_frame :被参考的帧内池
**  pattern_type：帧内预测的模式范围从0~3
**	para：压缩的参数，包括定义宏块大小等。
**	i_offset,j_offset 每一帧被划分成若干宏块进行存储，这两个坐标表示出宏块的位置偏移
**	董辰辰
*/

int Pattern::predict(Block& block,ResidualBlock & r_block,int start_r,int start_c,int end_r,int end_c, BlockBufferPool& whole_frame,int pattern_type, AVFormat &para,int i_offset,int j_offset){
	int block_w, block_h;
	r_block.getBlockSize(para, block_w, block_h);							// 当前block的宽和高

	/**
	** 计算宏块的偏移，为了避免重复计算，将计算宏块位置的代码上移通过参数进行传递
	*/
	//int i_offset = TABLE[r_block.block_id / para.block_num_per_row][block_h];// 当前block的起始像素所在Frame的行
	//int j_offset = TABLE[(r_block.block_id % para.block_num_per_row)][block_w];	// 当前block的起始像素所在Frame的列
	int diff = 0;		
	for (int i = start_r; i <= end_r && i + i_offset < para.height; i++)
	{
		int pos=TABLE[i][block_w];                  //通过查表计算乘法，代表i*block_w
		for (int j = start_c; j <= end_c && j + j_offset < para.width; j++) 
		{
			int16_t block_pool=128;   				//用于预测的像素值，默认为128，如果没有参考像素就取128
			int c_i=-1,c_j=-1;  					//相对于每一帧的坐标,用于预测的数据对一个帧进行连续存储
			if(pattern_type == 0&&i_offset>0)		// 竖向预测, 参考小块上方的一条像素	
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
				/*
				** 计算左边位置和上面位置的平均值进行预测估计
				*/
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
				block_pool=(int16_t)whole_frame.getValue(c_i, c_j);
			}
			else
			{
			}
			r_block.data[pos + j] = (int16_t)block.data[pos + j] - block_pool;    //通过小宏块的值和预测的值求残差
			diff +=  Square_table[abs(r_block.data[pos + j])];                    //计算平方和用于计算RDO
		}
	}
	return diff;
}
/*
** 帧间反预测模块，参数和帧间预测模块的参数意义相同，注意在反预测时会进行更新预测池的操作
** 董辰辰
*/
void Pattern::de_predict(Block& blk,ResidualBlock & r_block,int start_r,int start_c,int end_r,int end_c, BlockBufferPool& b_pool,int pattern_type, AVFormat &para) {
	int block_w, block_h;
	r_block.getBlockSize(para, block_w, block_h);								// 当前block的宽和高
	int i_offset = TABLE[r_block.block_id / para.block_num_per_row][block_h];	// 当前block的起始像素所在Frame的行
	int j_offset = TABLE[(r_block.block_id % para.block_num_per_row)][block_w];	// 当前block的起始像素所在Frame的列

	for (int i = start_r; i <= end_r && i + i_offset < para.height; i++)
	{
		int pos=TABLE[i][block_w];
		for (int j = start_c; j <= end_c && j + j_offset < para.width; j++) 
		{
			int16_t block_pool=128;
			int c_i=-1,c_j=-1;
			if(pattern_type == 0&&i_offset>0)		//竖向反预测, 参考小块上方的一条像素	
			{
				c_i=start_r+i_offset - 1;
				c_j=j_offset + j;
				block_pool=(int16_t)b_pool.getValue(c_i, c_j);
			}
			else if(pattern_type == 1&&j_offset>0)	//横向反预测, 参考小块上方的一条像素	
			{
				c_i=i_offset + i;
				c_j=j_offset - 1+start_c;
				block_pool=(int16_t)b_pool.getValue(c_i, c_j);
			}
			else if (pattern_type == 2)				//DC预测模式	
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
			else if (pattern_type == 3&&i_offset>0&&j_offset>0)	//右斜45度预测, 参考斜左上方的一条像素	
			{
				if(i-start_r>j-start_c)                        	//需要判断其用到的是坐标的像素还是上面的像素
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
			int16_t tmp=(int16_t)r_block.data[pos + j] + block_pool; 		//类型转换，需要判断是否越界
			if(tmp<0)
				tmp=0;
			else if(tmp>255)
				tmp=255;
			else
			{}
			blk.data[pos + j] = tmp;                                 
			b_pool.setValue(i_offset + i, j_offset +j, blk.data[pos + j]); 	//更新预测的池，用于另外的宏块的参考
		}
	}
}


const int MAX_INTER_SEARCH_RANGE = 4;


/*
* 从包含.getValue(i, j)/.get_width()/.get_height()方法的实体中, 读入.getValue(i, j)的数据
* 如果i, j越界, 则返回128
*
* 易惠康
*/
int16_t get_value_or_128(BlockBufferPool &sort_of_array, int i, int j) {
	if (i < 0 || i >= sort_of_array.get_height() || j < 0 || j >= sort_of_array.get_width()) return 128;
	else return sort_of_array.getValue(i, j);
}


/* 获取在坐标为(i + i1, j + j1)处的插值
* Args:
*	i, j, i1, j1表示所求坐标
*
* 易惠康
*/
int16_t Pattern::interpolate(BlockBufferPool & b_pool, int i, int j, double i1, double j1) {
	double a = get_value_or_128(b_pool, i,     j);
	double b = get_value_or_128(b_pool, i + 1, j);
	double c = get_value_or_128(b_pool, i,     j + 1);
	double d = get_value_or_128(b_pool, i + 1, j + 1);
	double i2 = 1.0 - i1;
	double j2 = 1.0 - j1;
	return (int16_t)(a * i2 * i2 + b * i1 * j2 + c * i2 * j1 + d * i1 * j1);
}


/* 计算当前块与参考块的SSD
* Args:
*	block: 当前块所在宏块
*	start_r, ..., j_offset: 用来算参考块的对应坐标
*	r_i, r_j: 带分量的相对位移, 值为真实代表的MV*(1<<pt)
*	b_pool: 参考块所在帧
* 
* 易惠康
*/
int Pattern::calc_SSD_inter(const Block &block, int start_r, int start_c, int end_r, int end_c,
	int r_i, int r_j, int block_w, int i_offset, int j_offset,
	BlockBufferPool & b_pool) {

	int diff = 0;
	for (int i = start_r; i <= end_r; i++) {		// 枚举块的行
		for (int j = start_c; j <= end_c; j++) {	// 枚举块的列
			int c_i = i_offset + i + r_i;			// 参考块的行坐标
			int c_j = j_offset + j + r_j;			// 参考块的纵坐标

			int tmp = (int16_t)block.data[TABLE[i][block_w] + j] - get_value_or_128(b_pool, c_i, c_j);

			diff +=TABLE[abs(tmp)][abs(tmp)];
		}
	}
	return diff;
}

/* 求给定范围子块的与参考块的残差
* Args:
*	block: 当前块所在宏块
*	start_r, ..., j_offset: 用来算参考块的对应坐标
*	r_i, r_j: 带分量的相对位移, 值为真实代表的MV*(1<<pt)
*	b_pool: 参考块所在帧
*
* 易惠康
*/
void Pattern::predict_inter_sub(const Block &block,ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c,
	int r_i, int r_j, int block_w, int i_offset, int j_offset,
	BlockBufferPool & b_pool) {

	int diff = 0;
	for (int i = start_r; i <= end_r; i++) {		// 枚举块的行
		for (int j = start_c; j <= end_c; j++) {	// 枚举块的列
			int c_i = i_offset + i + r_i;			// 参考块的行坐标
			int c_j = j_offset + j + r_j;			// 参考块的纵坐标

			int tmp = (int16_t)block.data[TABLE[i][block_w]+ j] - get_value_or_128(b_pool, c_i, c_j);
			r_block.data[TABLE[i][block_w] + j] = tmp;
		}
	}
}

/* 由给定范围子块的残差和参考块, 还原出该子块的YUV分量信息
* Args:
*	block: 当前块所在宏块
*	start_r, ..., j_offset: 用来算参考块的对应坐标
*	r_i, r_j: 带分量的相对位移, 值为真实代表的MV*(1<<pt)
*	cur_pool: 当前块所在帧
*	ref_pool: 参考块所在帧
*
* 易惠康
*/
void Pattern::predict_inter_add(Block &block, const ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c,
	int r_i, int r_j, int block_w, int i_offset, int j_offset,
	BlockBufferPool & ref_pool, BlockBufferPool &cur_pool) {

	int diff = 0;
	for (int i = start_r; i <= end_r; i++) {		// 枚举块的行
		for (int j = start_c; j <= end_c; j++) {	// 枚举块的列
			int c_i = i_offset + i + r_i;			// 参考块的行坐标
			int c_j = j_offset + j + r_j;			// 参考块的纵坐标

			int16_t tmp = (int16_t)r_block.data[TABLE[i][block_w] + j] + get_value_or_128(ref_pool, c_i, c_j);
			if(tmp < 0)
				tmp = 0;
			else if (tmp > 255)
				tmp=255;
			else
			{}
			block.data[TABLE[i][block_w] + j] = tmp;
			cur_pool.setValue(i+i_offset, j+j_offset, tmp);
		}
	}
}

/* 帧间预测搜索算最佳SSD
* 使用菱形搜索/折半步长法,
*/
int Pattern::inter_predict(Block& block, ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c,
	FrameBufferPool &f_pool, InterMV &inter_mv, AVFormat &para, int diff_thresshold,int i_offset,int j_offset) {
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
	//
	//int i_offset = TABLE[r_block.block_id / para.block_num_per_row][block_h];// 当前block的起始像素所在Frame的行
	//int j_offset = TABLE[(r_block.block_id % para.block_num_per_row)][block_w];	// 当前block的起始像素所在Frame的列

	if(f_pool.size() <= 1) {
		return INT_MAX;
	}

	int diff = INT_MAX;
	int best_fi = 0;
	std::pair<int, int> best_mv, mv, nmv;
	std::map< std::pair<int, int>, int> hash;
	for (int fi = f_pool.size() - 2; fi >= 0 && fi >= f_pool.size() - 5; fi--) {
		mv.first = 0;
		mv.second = 0;
		int cur_diff = calc_SSD_inter(block, start_r, start_c, end_r, end_c, mv.first, mv.second, block_w, i_offset, j_offset, f_pool[fi]);
		hash.clear();
		hash[mv] = cur_diff;

		
//		int search_count=0;
		int step_length = 2;
		while (step_length > 0) {
			int best_d_p = -1, best_new_diff = INT_MAX;
			for (int d_p = 0; d_p < 4; d_p ++) {
				nmv.first = mv.first + (search_dir[d_p][0] * step_length);
				nmv.second = mv.second + (search_dir[d_p][1] * step_length);
				if (nmv.first < -MAX_INTER_SEARCH_RANGE || nmv.first >= MAX_INTER_SEARCH_RANGE) continue;
				if (nmv.second < -MAX_INTER_SEARCH_RANGE || nmv.second >= MAX_INTER_SEARCH_RANGE) continue;
				
				//计算SSD值
				int new_diff;
				if (hash.find(nmv) != hash.end()) {
					new_diff = hash[nmv];
				} else {
					new_diff = calc_SSD_inter(block, start_r, start_c, end_r, end_c, nmv.first, nmv.second, block_w, i_offset, j_offset, f_pool[fi]);
					hash[nmv] = new_diff;
				}

				// 更菱形四角上的最好SSD
				if (new_diff < best_new_diff) {
					best_new_diff = new_diff;
					best_d_p = d_p;
				}
			}

			// 更新当前帧最好的SSD
			if (best_new_diff < cur_diff) {
				mv.first += (search_dir[best_d_p][0] * step_length);
				mv.second += (search_dir[best_d_p][1] * step_length);
				cur_diff = best_new_diff;
			} else { // 更新不到, 则缩减步长
				step_length >>= 1;
			};
		}

		if (cur_diff < diff) {
			diff = cur_diff;
			best_mv = mv;
			best_fi = fi;
		} else {
			break;
		}
	}

	// 范围内最好SSD, mv, 参考帧位置(帧进入frame_pool中的方式在解码加密中一致, 记录绝对帧在池中相对当前帧的位置)
	inter_mv.fi = f_pool.size() - 2 - best_fi;
	inter_mv.mv = best_mv;

	return diff;
}


void Pattern::inter_predict_setvalue(Block& block, ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c, FrameBufferPool &f_pool, InterMV &inter_mv, AVFormat &para,int i_offset,int j_offset) {
	int frame_width = para.width;
	int frame_height = para.height;
	int block_w, block_h;
	r_block.getBlockSize(para, block_w, block_h);
	
	int fi = f_pool.size() - 2 - inter_mv.fi;
	predict_inter_sub(block,r_block,start_r, start_c, end_r, end_c, inter_mv.mv.first, inter_mv.mv.second, block_w, i_offset, j_offset, f_pool[fi]);
}


void Pattern::inter_predict_reverse(Block& block, ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c, FrameBufferPool &f_pool,BlockBufferPool & block_buffer_pool, InterMV &inter_mv, AVFormat &para) {
	int frame_width = para.width;
	int frame_height = para.height;
	int block_w, block_h;
	r_block.getBlockSize(para, block_w, block_h);
	int i_offset = TABLE[r_block.block_id / para.block_num_per_row][block_h];// 当前block的起始像素所在Frame的行
	int j_offset = TABLE[(r_block.block_id % para.block_num_per_row)][block_w];	// 当前block的起始像素所在Frame的列
	
	int r_fi = f_pool.size() - 2 - inter_mv.fi;
	int r_i = inter_mv.mv.first;
	int r_j = inter_mv.mv.second;

	predict_inter_add(block,r_block,start_r, start_c, end_r, end_c, inter_mv.mv.first, inter_mv.mv.second, block_w, i_offset, j_offset, f_pool[r_fi], block_buffer_pool);
}


