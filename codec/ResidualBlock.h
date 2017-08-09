#pragma once

#include <vector>
#include <stdint.h>
#include "def.h"
class Tree;
class Node{
	public:
	/**
	*  采用的预测方法
	*  INTER_PREDICTION：帧间预测
	*  INTRA_PREDICTION：帧内预测
	*  1 bit
	*/
	enum {
		INTER_PREDICTION,
		INTRA_PREDICTION,
	}pre_type;

	/**
	*  帧间预测的运动矢量
	*  2 byte
	*/
	int8_t mv[2];

	/**
	*  帧内预测的方法
	*  7 bit
	*/
	uint8_t prediction;

	/**
	* 量化方法
	* 4bit 
	*/
	int quantization;

	/**
	* 转换方式
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
	* 子块数据
	*/
	uint8_t node_id;

	Node * data;
	/**
	* 划分方式
	* HORIZONTAL 水平划分
	* VERTICAL 竖直划分
	*/
	enum {
		NONE,
		HORIZONTAL,
		VERTICAL
	}split_direction;

	
	/**
	* 树的左节点
	*/
	Tree * left;
	/**
	* 树的右节点
	*/
	Tree * right;


	/**
	* 左上角相对坐标
	*/
	uint8_t left_top_h;
	uint8_t left_top_w;
	
	/**
	* 右下角相对坐标
	*/
	uint8_t right_bottom_h;
	uint8_t right_bottom_w;

	double score;
	/**
	* 提供当前节点表示的区域范围
	*/
	

	Tree(int left_top_h,int left_top_w,int right_bottom_h,int right_bottom_w);


	int to_stream(unsigned char * stream,uint8_t * used_node_ids,int& num);

	int from_stream(unsigned char * stream,int &num);

};



class ResidualBlock
{
	/**
	*  宏块id
	*/
public:
	int block_id;
	int type_slice;
	vector<int> node;
	std::vector<int16_t> data;
	bool is_tree;
	/**
	* 矩阵序列化顺序
	*/
	int order;

	/**
	*  宏块类型
	*/

	Block::BlockType block_type;

	/**
	* 子块划分树
	*/
	Tree tree;

	Node node_list[128];


	uint8_t used_node_ids[128];


	int curr_node;
	
public:
	/**
	* 提供残差宏块大小
	* height 高度
	* width 宽度
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