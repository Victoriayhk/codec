#pragma once


#include <list>
#include <vector>

using namespace std;

typedef unsigned char uint8_t;
//#define INTER_PREDICTION 1;
//#define INTRA_PREDICTION 2;
class AVFormat	//视频基本信息以及视频编码各项参数
{
	/**
	*  视频名称
	*/
public:
	FILE* video;
	char file_name[100];
	/**
	*  视频的高度与宽度
	*/
	int height;
	int width;

	/**
	*  视频的fps与视频总帧数
	*/
	int fps;
	int frame_num;

	/**
	*  宏块大小
	*/
	int block_width;
	int block_height;

	/**
	*  当前frameid
	*/
	int frame_id;

	//视频编码各项参数
	AVFormat()
	{
		video = nullptr;
	}
};

struct Block
{
public:
	/**
	*  宏块id
	*/
	int block_id;

	vector<uint8_t> data;
	//uint8_t* Udata;
	//uint8_t* Vdata;

	/**
	*  宏块采用的预测方法
	*  INTER_PREDICTION：帧间预测
	*  INTRA_PREDICTION：帧内预测
	*/
	//int pre_type;
	enum {
		NONE,
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

	Block()
	{
		//data = nullptr;
	}
	Block(int h , int w):data(h*w)
	{
	//	data = new uint8_t[h*w];
	}
	~Block()
	{
		//if(data) delete []data;
	}
};

struct Frame	//视频帧信息
{
public:
	/**
	*  帧id
	*/
	int frame_id;

	/**
	*  编码id
	*/
	int encode_id;

	/**
	*  帧类型
	*/
	enum{
		I,
		B,
		P,
	}frame_type;

	/**
	*  帧宏块 
	*/
	vector<Block> Yblock;
	vector<Block> Ublock;
	vector<Block> Vblock;
	//Block *Yblock;
	//Block *Ublock;
	//Block *Vblock;
	Frame()
	{
	}
	//Frame(int h, int w , int block_h, int block_w)
	//{
	//	Yblock.resize(h*w/block_h/block_w, nullptr);
	//	Ublock.resize(h*w/block_h/block_w/4,nullptr);
	//	Vblock.resize(h*w/block_h/block_w/4,nullptr);
	//}
	~Frame()
	{
	}
};

class PKT
{
public:
	list<Block> Ylist;
	list<Block> Ulist;
	list<Block> Vlist;

	~PKT()
	{
	}

	//Block *Yblock;
	//Block *Ublock;
	//Block *Vblock;
};