#pragma once

#include <vector>
#include <stdint.h>

class Tree;
class Node{
	/**
	*  ���õ�Ԥ�ⷽ��
	*  INTER_PREDICTION��֡��Ԥ��
	*  INTRA_PREDICTION��֡��Ԥ��
	*/
	enum {
		//NONE,
		INTER_PREDICTION,
		INTRA_PREDICTION,
	}pre_type;

	/**
	*  ֡��Ԥ����˶�ʸ��
	*/
	int mv[2];

	/**
	*  ֡��Ԥ��ķ���
	*/
	int prediction;

	/**
	* ��������
	*/
	int quantization;

	/**
	* ת����ʽ
	*/
	int convertion;
public:

	friend Tree;
};



class Tree{
	/**
	* �ӿ�����
	*/
	Node * data;
	/**
	* ���ַ�ʽ
	* HORIZONTAL ˮƽ����
	* VERTICAL ��ֱ����
	*/
	enum {
		NONE,
		HORIZONTAL,
		VERTICAL
	}split_direction;

	/**
	* ������ڵ�
	*/
	Tree * left;
	/**
	* �����ҽڵ�
	*/
	Tree * right;


	/**
	* ���Ͻ��������
	*/
	int left_top_h;
	int left_top_w;
	
	/**
	* ���½��������
	*/
	int right_bottom_h;
	int right_bottom_w;

public:
	/**
	* �ṩ��ǰ�ڵ��ʾ������Χ
	*/
	Tree(int left_top_h,int left_top_w,int right_bottom_h,int right_bottom_w);
};



class ResidualBlock
{
	/**
	*  ���id
	*/
public:
	int block_id;

	std::vector<int16_t> data;

	/**
	* �������л�˳��
	*/
	int order;

	/**
	*  �������
	*/
	enum{
		Y,
		U,
		V,
	}block_type;

	/**
	* �ӿ黮����
	*/
	Tree tree;
	
public:
	/**
	* �ṩ�в����С
	* height �߶�
	* width ����
	*/
	ResidualBlock(int height , int width);
};

class PKT
{
public:
	std::vector<ResidualBlock> Ylist;
	std::vector<ResidualBlock> Ulist;
	std::vector<ResidualBlock> Vlist;
};