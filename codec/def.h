#pragma once


#include <list>
#include <vector>

using namespace std;

typedef unsigned char			uint8_t;
//typedef char					int8_t;
//typedef short					int16_t;
typedef unsigned short			uint16_t;

//#define INTER_PREDICTION 1;
//#define INTRA_PREDICTION 2;
class Block
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
	*  宏块类型
	*/
	enum BlockType{
		Y,
		U,
		V,
	}block_type;

public:
	Block();
	Block(int h , int w);
	~Block();
};

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

	/**
	*  量化时使用的数值
	*/
	double quantizationY;
	double quantizationU;
	double quantizationV;

	//视频编码各项参数
public:
	AVFormat();
	~AVFormat();

	void getBlockSize(Block block, int& height, int& width);
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
public:
	Frame();
	//Frame(int h, int w , int block_h, int block_w)
	//{
	//	Yblock.resize(h*w/block_h/block_w, nullptr);
	//	Ublock.resize(h*w/block_h/block_w/4,nullptr);
	//	Vblock.resize(h*w/block_h/block_w/4,nullptr);
	//}
	~Frame();
};

//class PKT
//{
//public:
//	list<Block> Ylist;
//	list<Block> Ulist;
//	list<Block> Vlist;
//
//	~PKT()
//	{
//	}
//
//	//Block *Yblock;
//	//Block *Ublock;
//	//Block *Vblock;
//};