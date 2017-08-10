#ifndef _DCT_H_
#define _DCT_H_

#include "def.h"
#include <stdint.h>

class DCT8x8Solver {
	void matrix_dct(double org[][8], double des[][8]);
	void matrix_idct(double org[][8], double des[][8]);
	void dct_2d(int16_t *data, int start_idx, int w);
	void reverse_dct_2d(int16_t *data, int start_idx, int w);

public:
	DCT8x8Solver(){}
	int dct(int16_t *data, int h, int w);
	int reverse_dct(int16_t *data, int h, int w);

	int dct(int16_t *data, int left_row,int left_col,int right_row,int right_col,int h,int w);
	int reverse_dct(int16_t *data, int left_row,int left_col,int right_row,int right_col,int h,int w);
};

class DCT4x4Solver {
	int QP; // 量化参数
	// 矩阵运算
	void matrix_multiply(const int a[][4], const int b[][4], int c[][4]);

	// 量化
	void quantize(int x[][4], int y[][4]);
	void reverse_quantize(int x[][4], int y[][4]);

	// 直接对子块的量化
	void quantize(int16_t *data, int start_idx, int w, int y[][4]);
	void reverse_quantize(int16_t *data, int start_idx, int w, int y[][4]);

	// 子过程
	void dct_1d(double *in, double *out, const int n);
	void dct_2d(int16_t *data, int start_idx, int w);
	void reverse_dct_2d(int16_t *data, int start_idx, int w);

public:
	DCT4x4Solver(){
		QP = 0;  
	}
	void set_qp(int qt);
	int dct(int16_t *data, int h, int w);
	int reverse_dct(int16_t *data, int h, int w);

	int dct(int16_t *data, int left_row,int left_col,int right_row,int right_col,int h,int w);
	int reverse_dct(int16_t *data, int left_row,int left_col,int right_row,int right_col,int h,int w);
	
};

#endif