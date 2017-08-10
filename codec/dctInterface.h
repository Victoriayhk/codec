#pragma once


#include "ResidualBlock.h"
#include "dct.h"


int reverse_dct_trans(ResidualBlock & residual_block,int left_row,int left_col,int right_row,int right_col,int h,int w);

int dct_trans(ResidualBlock & residual_block,int left_row,int left_col,int right_row,int right_col,int h,int w);