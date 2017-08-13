#include "dctInterface.h"


/**
* dct ±ä»»api
*/
int dct_trans(ResidualBlock & residual_block,int left_row,int left_col,int right_row,int right_col,int h,int w)
{
	DCT4x4Solver dct_tr;
    dct_tr.dct(residual_block.data.data(),left_row,left_col,right_row,right_col,h,w);
	return 0;
}


int reverse_dct_trans(ResidualBlock & residual_block,int left_row,int left_col,int right_row,int right_col,int h,int w)
{
	DCT4x4Solver dct_tr;
    dct_tr.reverse_dct(residual_block.data.data(),left_row,left_col,right_row,right_col,h,w);
	return 0;
}
