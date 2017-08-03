
#include "quantization.h"
#include <iostream>
#include <math.h>

using namespace std;

double round(double val)
{
    return (val> 0.0) ? floor(val+ 0.5) : ceil(val- 0.5);
}

int quantization(int f_x, int f_y, int l_x ,int l_y , ResidualBlock& block, AVFormat avFormat)
{
	if(f_x>=l_x||f_y>=l_y) return -1;


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

	if(l_x>=width||l_y>=height) return -1;

	for(int j = f_y; j <= l_y ; ++j)
	{
		for(int i = f_x; i <= l_x ; ++i)
		{
			double tmp = round((double)block.data[i + width * j]/quantization_num);
			block.data[i + width * j] = (int16_t)tmp;
		}
	}
}

int Reverse_quantization(int f_x, int f_y, int l_x ,int l_y , ResidualBlock& block, AVFormat avFormat)
{
	if(f_x>=l_x||f_y>=l_y) return -1;

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

	if(l_x>=width||l_y>=height) return -1;

	for(int j = f_y; j <= l_y ; ++j)
	{
		for(int i = f_x; i <= l_x ; ++i)
		{
			block.data[i + width * j] *= quantization_num;
		}
	}
}