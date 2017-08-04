#pragma once

#include "ResidualBlock.h"
#include "def.h"

/*
* 对块进行量化操作
* 输入：f_x,f_y 块的左上角坐标
*		l_x,l_y 块的右下角坐标
*		ResidualBlock 残差宏块，并将结果保存在残差宏块之中
*		avFormat 视频参数
*/
int quantization(int f_x,int f_y,int l_x,int l_y, ResidualBlock&, AVFormat& avFormat);

/*
* 对块进行反量化操作
* 输入：f_x,f_y 块的左上角坐标
*		l_x,l_y 块的右下角坐标
*		ResidualBlock 残差宏块，并将结果保存在残差宏块之中
*		avFormat 视频参数
*/
int Reverse_quantization(int ,int ,int ,int , ResidualBlock&, AVFormat& avFormat);


//int quantization_test();