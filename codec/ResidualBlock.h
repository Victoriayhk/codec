#pragma once

#include <vector>
#include <stdint.h>
#include "def.h"
class Tree;
class Node{
	public:
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


	friend Tree;
};



class Tree{
public:
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
	uint8_t left_top_h;
	uint8_t left_top_w;
	
	/**
	* ���½��������
	*/
	uint8_t right_bottom_h;
	uint8_t right_bottom_w;

	double score;
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

	Block::BlockType block_type;

	/**
	* �ӿ黮����
	*/
	Tree tree;
	
public:
	/**
	* �ṩ�в����С
	* height �߶�
	* width ���
	*/
	ResidualBlock();
	ResidualBlock(const Block &);
	ResidualBlock(int );
	ResidualBlock(Block::BlockType type,int height , int width);
	void getBlockSize(AVFormat &, int&, int&);

	int to_stream(unsigned char *stream);
	int from_stream(unsigned char *stream, int block_size);
};

class PKT
{
public:
	std::vector<ResidualBlock> Ylist;
	std::vector<ResidualBlock> Ulist;
	std::vector<ResidualBlock> Vlist;
	
	int reserve(int size);
	int init(AVFormat& para);
	//PKT()
	int to_stream(unsigned char *stream);
	int from_stream(unsigned char *stream, AVFormat &para);
};