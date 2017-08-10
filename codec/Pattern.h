#pragma once
#include <map>
#include "Pattern.h"
#include "ResidualBlock.h"
#include "decode_buffer_pool.h"


struct InterMV {
	int fi;				// Ö¡µÄÏà¶ÔID
	pair<int, int> mv;	// mv
};



class Pattern
{
public:
	//typedef void (*FUN)(Block&,ResidualBlock &,int,int,int,int,BlockBufferPool&);
	//static int get_pattern_number();
	//static FUN getPattern(int pattern);
	//Pattern(void);
	//~Pattern(void);



	//static std::map<int,int> patterns;
	//static int pattern_number; 
	//static void pattern(Block&,ResidualBlock &,int,int,int,int,BlockBufferPool&,int pattern_type);
	static double predict(Block&,ResidualBlock &,int,int,int,int,BlockBufferPool&,int pattern_type, AVFormat &para);
	static void de_predict(Block&,ResidualBlock & r_block,int start_r,int start_c,int end_r,int end_c, BlockBufferPool& b_pool,int pattern_type, AVFormat &para);

	static double calc_SAD_inter(const Block &block, int start_r, int start_c, int end_r, int end_c, int r_i, int r_j, int block_w, int i_offset, int j_offset, BlockBufferPool & bpool);
	static void predict_inter_sub(const Block &block,ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c, int r_i, int r_j, int block_w, int i_offset, int j_offset, BlockBufferPool & bpool);
	static double inter_predict(Block& block, ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c, FrameBufferPool &f_pool, InterMV &inter_mv, AVFormat &para);
	static void inter_predict_reverse(Block& block, ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c, FrameBufferPool &f_pool,BlockBufferPool & block_buffer_pool, InterMV &inter_mv, AVFormat &para);

	static void inter_predict_setvalue(Block& block, ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c, FrameBufferPool &f_pool, InterMV &inter_mv, AVFormat &para);
};
//int Pattern::pattern_number = 0;
