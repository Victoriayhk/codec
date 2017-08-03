#pragma once

#include <vector>
#include <stdint.h>

class Tree;
class Node{
	/**
	*  采用的预测方法
	*  INTER_PREDICTION：帧间预测
	*  INTRA_PREDICTION：帧内预测
	*/
	enum {
		//NONE,
		INTER_PREDICTION,
		INTRA_PREDICTION,
	}pre_type;

	/**
	*  帧间预测的运动矢量
	*/
	int mv[2];

	/**
	*  帧内预测的方法
	*/
	int prediction;

	/**
	* 量化方法
	*/
	int quantization;

	/**
	* 转换方式
	*/
	int convertion;
public:

	friend Tree;
};



class Tree{
	/**
	* 子块数据
	*/
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
	int left_top_h;
	int left_top_w;
	
	/**
	* 右下角相对坐标
	*/
	int right_bottom_h;
	int right_bottom_w;

public:
	/**
	* 提供当前节点表示的区域范围
	*/
	Tree(int left_top_h,int left_top_w,int right_bottom_h,int right_bottom_w);
};



class ResidualBlock
{
	/**
	*  宏块id
	*/
public:
	int block_id;

	std::vector<int16_t> data;

	/**
	* 矩阵序列化顺序
	*/
	int order;

	/**
	*  宏块类型
	*/
	enum{
		Y,
		U,
		V,
	}block_type;

	/**
	* 子块划分树
	*/
	Tree tree;
	
public:
	/**
	* 提供残差宏块大小
	* height 高度
	* width 宽度
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