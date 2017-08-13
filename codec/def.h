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
public:
	/**
	*  ��Ƶ����
	*/
	char file_name[100];

	/**
	*  ��Ƶд��д���ļ�
	*/
	FILE* video;
	FILE* out_video;

	char out_file_name[100];
	/**
	*  ���ļ�д��д���ļ�
	*/
	char stream_file_name[100];
	FILE* stream_writer;
	FILE* stream_reader;


	/**
	*  ��Ƶ�ĸ߶�����
	*/
	int height;
	int width;

	/**
	*  ֡��Ԥ��ģʽ��
	*/
	int pattern_num;
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
	*	֡�ں�֡����Ʋ���
	*/

	int contral_lamda;
	/**
	*  �������ĸ߶�
	*/
	int tree_mini_block_width;
	int tree_mini_block_height;
	/**
	* ������
	*/
	int block_num_per_row;
	int block_num_per_col;
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

	/**
	*  ���ļ�֦ϵ��
	*/
	double split_gamma;
	bool is_tree;
	/**
	*  �ر���һ�����õĺ����Ŀ
	*/
	static const int entropy_silce_size = 256;

	//��Ƶ����������
public:
	AVFormat();
	~AVFormat();
	int load(const string & config_file);
	int load(int argc, char * argv[]);
	void setValue(const string & key, const string & value);
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


struct pos{
		int left_top_x;
		int left_top_y;
		int right_bottom_x;
		int right_bottom_y; 
		pos(){}
		pos(int p1,int p2,int p3,int p4):left_top_x(p1),left_top_y(p2),right_bottom_x(p3),right_bottom_y(p4){} 
};

