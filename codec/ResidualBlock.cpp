
#include "ResidualBlock.h"



ResidualBlock::ResidualBlock(int height , int width):tree(0,0,height,width),data(width * height){}


Tree::Tree(int left_top_h,int left_top_w,int right_bottom_h,int right_bottom_w):data(nullptr),
				split_direction(NONE),left(nullptr),right(nullptr),
				left_top_h(left_top_h),left_top_w(left_top_w),right_bottom_h(right_bottom_h),right_bottom_w(right_bottom_w){
}