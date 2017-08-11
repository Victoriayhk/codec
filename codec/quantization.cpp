
#include "quantization.h"
#include <iostream>
#include <math.h>

using namespace std;


extern int TABLE[1500][1500];
double round(double val)
{
    return (val> 0.0) ? floor(val+ 0.5) : ceil(val- 0.5);
}

int quantization(int f_x, int f_y, int l_x ,int l_y , ResidualBlock& block, AVFormat avFormat)
{
	if(f_x>=l_x||f_y>=l_y) return -1;

	double AC_quantization;

	double quantization_num;
	int width = avFormat.block_width;
	int height = avFormat.block_height;

	if(block.block_type == Block::Y)
	{
		quantization_num = avFormat.quantizationY;
	}
	else if(block.block_type == Block::U)
	{
		quantization_num = avFormat.quantizationU;
		width /= 2;
		height /= 2;
	}
	else
	{
		quantization_num = avFormat.quantizationV;
		width /= 2;
		height /= 2;
	}
	AC_quantization = 8.0;//直流分量的量化不得小于8
	AC_quantization = quantization_num<AC_quantization?AC_quantization:quantization_num;

	if(l_x>=width||l_y>=height) return -1;

	double tmp;
	for(int i = f_x; i <= l_x ; ++i){
		for(int j = f_y; j <= l_y ; ++j)
		{
			//double tmp;
			if(i % 4 == 0 && j % 4 == 0)
			{
				tmp = round((double)block.data[TABLE[i][width] +j]/AC_quantization);
			}

			tmp = round((double)block.data[TABLE[i][width] +j]/quantization_num);
			block.data[i*width +j] = (int16_t)tmp;
		}
	}
	return 0;
}

int Reverse_quantization(int f_x, int f_y, int l_x ,int l_y , ResidualBlock& block, AVFormat avFormat)
{
	if(f_x>=l_x||f_y>=l_y) return -1;

	double AC_quantization = 8.0;
	double quantization_num;
	int width = avFormat.block_width;
	int height = avFormat.block_height;

	if(block.block_type == Block::Y)
	{
		quantization_num = avFormat.quantizationY;
	}
	else if(block.block_type == Block::U)
	{
		quantization_num = avFormat.quantizationU;
		width /= 2;
		height /= 2;
	}
	else
	{
		quantization_num = avFormat.quantizationV;
		width /= 2;
		height /= 2;
	}

	AC_quantization = 8.0;
	AC_quantization = quantization_num<AC_quantization?AC_quantization:quantization_num;

	if(l_x>=width||l_y>=height) return -1;

	for(int i = f_x; i <= l_x ; ++i)
	{
		for(int j = f_y; j <= l_y ; ++j)
		{
			block.data[TABLE[i][width] + j] *= quantization_num;
		}
	}
	return 0;
}
