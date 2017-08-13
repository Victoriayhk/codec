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


class AVFormat;
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
	Block(Block::BlockType type,int height , int width);
	void getBlockSize(AVFormat &para,int &height, int &width);

	void print(AVFormat &para);
	~Block();
};

class AVFormat	//视频基本信息以及视频编码各项参数
{
public:
	/**
	*  视频名称
	*/
	char file_name[100];

	/**
	*  视频写入写出文件
	*/
	FILE* video;
	FILE* out_video;

	char out_file_name[100];
	/**
	*  流文件写入写出文件
	*/
	char stream_file_name[100];
	FILE* stream_writer;
	FILE* stream_reader;


	/**
	*  视频的高度与宽度
	*/
	int height;
	int width;

	/**
	*  帧内预测模式数
	*/
	int pattern_num;
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
	*	帧内和帧间控制参数
	*/

	int contral_lamda;
	/**
	*  控制树的高度
	*/
	int tree_mini_block_width;
	int tree_mini_block_height;
	/**
	* 宏块个数
	*/
	int block_num_per_row;
	int block_num_per_col;
	int block_num;
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

	/**
	*  树的减枝系数
	*/
	double split_gamma;
	bool is_tree;
	/**
	*  熵编码一次作用的宏块数目
	*/
	static const int entropy_silce_size = 256;

	//视频编码各项参数
public:
	AVFormat();
	~AVFormat();
	int load(const string & config_file);
	int load(int argc, char * argv[]);
	void setValue(const string & key, const string & value);
	void getBlockSize(Block::BlockType block_type, int& height, int& width);
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
	int init(AVFormat& para);
	~Frame();
};


struct pos{
		int left_top_x;
		int left_top_y;
		int right_bottom_x;
		int right_bottom_y; 
		pos(){}
		pos(int p1,int p2,int p3,int p4):left_top_x(p1),left_top_y(p2),right_bottom_x(p3),right_bottom_y(p4){} 
};

