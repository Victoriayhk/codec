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
	*  ���id
	*/
	int block_id;

	vector<uint8_t> data;
	//uint8_t* Udata;
	//uint8_t* Vdata;

	/**
	*  �������
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

class AVFormat	//��Ƶ������Ϣ�Լ���Ƶ����������
{
	/**
	*  ��Ƶ����
	*/
public:
	FILE* video;
	FILE* out_video;
	char file_name[100];
	/**
	*  ��Ƶ�ĸ߶�����
	*/
	int height;
	int width;

	/**
	*  ��Ƶ��fps����Ƶ��֡��
	*/
	int fps;
	int frame_num;

	/**
	*  ����С
	*/
	int block_width;
	int block_height;
	/**
	* ������
	*/
	int block_num;
	/**
	*  ��ǰframeid
	*/
	int frame_id;

	/**
	*  ����ʱʹ�õ���ֵ
	*/
	double quantizationY;
	double quantizationU;
	double quantizationV;

	//��Ƶ����������
public:
	AVFormat();
	~AVFormat();

	void getBlockSize(Block::BlockType block_type, int& height, int& width);
};

struct Frame	//��Ƶ֡��Ϣ
{
public:
	/**
	*  ֡id
	*/
	int frame_id;

	/**
	*  ����id
	*/
	int encode_id;

	/**
	*  ֡����
	*/
	enum{
		I,
		B,
		P,
	}frame_type;

	/**
	*  ֡��� 
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

