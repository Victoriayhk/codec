#pragma once

#include "ResidualBlock.h"
#include "def.h"

/*
* �Կ������������
* ���룺f_x,f_y ������Ͻ�����
*		l_x,l_y ������½�����
*		ResidualBlock �в��飬������������ڲв���֮��
*		avFormat ��Ƶ����
*/
int quantization(int f_x,int f_y,int l_x,int l_y, ResidualBlock&, AVFormat& avFormat);

/*
* �Կ���з���������
* ���룺f_x,f_y ������Ͻ�����
*		l_x,l_y ������½�����
*		ResidualBlock �в��飬������������ڲв���֮��
*		avFormat ��Ƶ����
*/
int Reverse_quantization(int ,int ,int ,int , ResidualBlock&, AVFormat& avFormat);


//int quantization_test();