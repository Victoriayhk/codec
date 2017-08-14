#pragma once

#include <vector>
#include <stdint.h>
#include "def.h"
class Tree;
/*
* �Ң
*/
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

	/*
	*  ֡��ģʽ
	*/
	uint8_t inter_value;

	friend Tree;

	
	int to_stream(unsigned char * stream);

	int from_stream(unsigned char * stream);
};



/** 
* �Ң
*/
class Tree{
	void deserialize(unsigned char * stream,int &byte,int &bit,Node * node_list,int &idx,int block_id,int type);
	void serialize(unsigned char * stream,int &byte,int &bit,int * used_node_ids,int &idx);
public:
	
    /**
	* �ӿ�����
	*/
	uint32_t node_id;

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

	int score;
	/**
	* �ṩ��ǰ�ڵ��ʾ������Χ
	*/
	

	Tree();
	Tree(int left_top_h,int left_top_w,int right_bottom_h,int right_bottom_w);


	int to_stream(unsigned char * stream,int * used_node_ids,int& num);

	int from_stream(unsigned char * stream,Node * node_list,int &num,int block_id,int type);

};


/**
*   �в��
*
*/
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

	Node node_list[1024];

    /**
    * ����ʹ��
    */
	int used_node_ids[1024];
	
	unsigned char tree_buff[1024];
	int tree_byte;

	int curr_node;

	uint8_t left_zero_num;
	
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
	int copy(const ResidualBlock & src);
	void getBlockSize(AVFormat &, int&, int&);
	Node & get_node(int & id);
	int to_stream(unsigned char *stream,AVFormat &para);
	int from_stream(unsigned char *stream, int block_size, AVFormat &para);

	int tree_to_stream();

	int head_to_stream(unsigned char *stream,AVFormat &para);
	int head_from_stream(unsigned char *stream, AVFormat &para);

	int data_to_stream(unsigned char *stream,AVFormat &para);
	int data_from_stream(unsigned char *stream, int block_size, AVFormat &para);

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
	int stream_write_one_component(AVFormat& para,std::vector<ResidualBlock> & list);
	int PKT::stream_read_one_component(AVFormat& para,std::vector<ResidualBlock> & list,Block::BlockType type);
	int stream_read(AVFormat& para);

	int block_head2stream(AVFormat& para,uint8_t** stream, ResidualBlock& rBlock, int *buff_len);
	int block_stream2head(AVFormat& para, uint8_t* stream, ResidualBlock& rBlock, int buff_len, int *head_length);

	int clear();

	static void free_buff();

private:
	static uint8_t* stream_buff;
	static uint8_t* head_buff;
};
