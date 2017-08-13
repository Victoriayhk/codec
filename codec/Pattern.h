#pragma once
#include <map>
#include "Pattern.h"
#include "ResidualBlock.h"
#include "decode_buffer_pool.h"

/* 没有压缩的MV和参考帧编号
*
* 易惠康
*/
struct InterMV {
	int fi;				// 帧的相对ID
	pair<int, int> mv;	// mv
};


/* 用一个Byte保存MV信息
* 前2位: fi, 参考帧相对位置(0, 1, 2, 3)
* 接下来3位: mv1, [-4, 4)
* 最后3位: mv2, [-4, 4]
*
* 易惠康
*/
struct InterMVConverter {
	unsigned char value;
	int get_fi() { return (short)((value >> 6) & 3); }
	int get_mv1() {
		int ret = ((value >> 3) & 7);
		if (ret & 4) return -(~ret&3)-1;
		else return ret;
	}
	int get_mv2() {
		short ret = value & 7;
		if (ret & 4) return -(~ret&3)-1;
		else return ret;
	}

	int set_value(int fi, int mv1, int mv2) {
		value = 0;
		value |= ((fi) & 3) << 6;
		
		if (mv1 < 0) {
			value |= 32;
			value |= (~(-mv1-1) & 3) << 3;
		} else {
			value |= (mv1 & 3) << 3;
		}

		if (mv2 < 0) {
			value |= 4;
			value |= (~(-mv2-1) & 3);
		} else {
			value |= (mv2 & 3);
		}
		return 0;
	}
    string to_string() {
        string str;
        for (int i = 7; i >= 0; i--) {
            if (value & (1 << i)) str += '1';
            else str += '0';
        }
        return str;
    }
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
	//static int predict(Block&,ResidualBlock &,int,int,int,int,BlockBufferPool&,int pattern_type, AVFormat &para);
	static void de_predict(Block&,ResidualBlock & r_block,int start_r,int start_c,int end_r,int end_c, BlockBufferPool& b_pool,int pattern_type, AVFormat &para);

	static int predict(Block&,ResidualBlock &,int,int,int,int,BlockBufferPool&,int pattern_type, AVFormat &para,int,int);
	//static void de_predict(Block&,ResidualBlock & r_block,int start_r,int start_c,int end_r,int end_c, BlockBufferPool& b_pool,int pattern_type, AVFormat &para,int,int);


	static int16_t interpolate(BlockBufferPool & b_pool, int i, int j, double i_p, double j_p);
	static int calc_SAD_inter(const Block &block, int start_r, int start_c, int end_r, int end_c, int r_i, int r_j, int block_w, int i_offset, int j_offset, BlockBufferPool & bpool);
	static void predict_inter_sub(const Block &block,ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c, int r_i, int r_j, int block_w, int i_offset, int j_offset, BlockBufferPool & bpool);
	static int inter_predict(Block& block, ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c, FrameBufferPool &f_pool, InterMV &inter_mv, AVFormat &para, int diff_threshold,int,int);
	
	static void predict_inter_add(Block &block, const ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c, int r_i, int r_j, int block_w, int i_offset, int j_offset, BlockBufferPool & ref_pool , BlockBufferPool &cur_pool);
	static void inter_predict_reverse(Block& block, ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c, FrameBufferPool &f_pool,BlockBufferPool & block_buffer_pool, InterMV &inter_mv, AVFormat &para);

	static void inter_predict_setvalue(Block& block, ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c, FrameBufferPool &f_pool, InterMV &inter_mv, AVFormat &para,int,int);
};
//int Pattern::pattern_number = 0;
