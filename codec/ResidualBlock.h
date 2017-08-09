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
	*  1 bit
	*/
	enum {
		INTER_PREDICTION,
		INTRA_PREDICTION,
	}pre_type;

	/**
	*  ֡��Ԥ����˶�ʸ��
	*  2 byte
	*/
	int8_t mv[2];

	/**
	*  ֡��Ԥ��ķ���
	*  7 bit
	*/
	uint8_t prediction;

	/**
	* ��������
	* 4bit 
	*/
	int quantization;

	/**
	* ת����ʽ
	* 4bit
	*/
	int convertion;


	friend Tree;

	
	int to_stream(unsigned char * stream);

	int from_stream(unsigned char * stream);
};



class Tree{

	void deserialize(unsigned char * stream,int &byte,int &bit,int &idx);
	void serialize(unsigned char * stream,int &byte,int &bit,uint8_t * used_node_ids,int &idx);
public:
	/**
	* �ӿ�����
	*/
	uint8_t node_id;

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


	int to_stream(unsigned char * stream,uint8_t * used_node_ids,int& num);

	int from_stream(unsigned char * stream,int &num);

};



class ResidualBlock
{
	/**
	*  ���id
	*/
public:
	int block_id;
	int type_slice;
	vector<int> node;
	std::vector<int16_t> data;
	bool is_tree;
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

	Node node_list[128];


	uint8_t used_node_ids[128];


	int curr_node;
	
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
	Node & get_node(int & id);
	int to_stream(unsigned char *stream,AVFormat &para);
	int from_stream(unsigned char *stream, int block_size, AVFormat &para);

	int clear();
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
	int to_stream(unsigned char *stream, AVFormat &para);
	int from_stream(unsigned char *stream, AVFormat &para);

	int stream_write(AVFormat& para);
	int stream_read(AVFormat& para);

	int block_head2stream(AVFormat& para,uint8_t** stream, ResidualBlock& rBlock, int *buff_len);
	int block_stream2head(AVFormat& para, uint8_t* stream, ResidualBlock& rBlock, int buff_len, int *head_length);

	int clear();
};