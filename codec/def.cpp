#include "def.h"
#include <string>
#include <fstream>
AVFormat::AVFormat():
	video(nullptr),
	out_video(nullptr),
	stream_writer(nullptr),
	stream_reader(nullptr)
{

	strcpy(file_name,"D:\\00_2017_mini\\01_≤‚ ‘–Ú¡–\\youjiyingxiong.yuv");
	strcpy(out_file_name,"D:\\1234.yuv");


	height = 720;
	width = 1280;
	pattern_num=4;

	tree_mini_block_width=8;
	tree_mini_block_height=8;
	fps = 25;
    frame_num = 750;

	block_width = 16;
	block_height = 16;

	block_num_per_row = ceil(1.0 * width / block_width); 
	block_num_per_col = ceil(1.0 * height /  block_height);
	block_num = block_num_per_row * block_num_per_col;
	
	frame_id = 0;

	quantizationY = 10;
	quantizationU = 5;
	quantizationV = 5;

	is_tree = true;

}

AVFormat::~AVFormat()
{
}

void AVFormat::setValue(const string & key, const string & value){

	if(key.compare("input_video") == 0){
		strcpy(file_name,value.c_str());
		
	}else if(key.compare("output_video") == 0){
		strcpy(out_file_name,value.c_str());
	}else if(key.compare("stream_file_name") == 0){
		strcpy(stream_file_name,value.c_str());
	}else if(key.compare("height") == 0){
		height = std::stoi(value);
	}else if(key.compare("width") == 0){
		width = std::stoi(value);
	}else if(key.compare("block_width") == 0){
		block_width = std::stoi(value);
	}else if(key.compare("block_height") == 0){
		block_height = std::stoi(value);
	}else if(key.compare("quantizationY") == 0){
		quantizationY = std::stod(value);
	}else if(key.compare("quantizationU") == 0){
		quantizationU = std::stod(value);
	}else if(key.compare("quantizationV") == 0){
		quantizationV = std::stod(value);

	}else if(key.compare("fps")==0){
		fps = std::stoi(value);
	}else if(key.compare("frame_num") == 0){
		frame_num = std::stoi(value);
	}else if(key.compare("is_tree") == 0){
		if(value.compare("true") == 0){
			is_tree = true;
		}else{
			is_tree = false;
		}
		
	}

	block_num_per_row = (int)ceil(1.0 * width / block_width); 
	block_num_per_col = (int)ceil(1.0 * height /  block_height);
	block_num = block_num_per_row * block_num_per_col;

}
int AVFormat::load(const string & config_file ){
	string line;
	ifstream fin(config_file);
	while(getline(fin,line)){
		if(line[0] == '#') continue;
		int index = line.find_first_of("=");
		if(index == -1) continue;
		setValue(line.substr(0,index),line.substr(index+1));
	}


	fin.close();
	return 0;
}

int AVFormat::load(int argc, char * argv[]){
	if(argc == 1) return 0;
	for(int i = 1; i < argc-1; i += 2 ){
		string key = argv[i];
		key = key.substr(1);

		string value = argv[i+1];
		if(key.compare("config") == 0){
			load(value);
		}else{
			setValue(key,value);
		}
	}
	return 0;
}





void AVFormat::getBlockSize(Block::BlockType  block_type, int& height, int& width)
{
	if(block_type == Block::Y)
	{
		height = block_height;
		width = block_width;
	}
	else
	{
		height = block_height/2;
		width = block_width/2;
	}
}

Block::Block()
{
}

Block::Block(int h , int w):
		data(h*w)
{
	//	data = new uint8_t[h*w];
}

Block::~Block()
{
}

void Block::print(AVFormat &para){
	int h;
	int w;
	getBlockSize(para,h,w);
	for(int i = 0; i < h; ++i){
		for(int j = 0; j < w; ++j){
			printf("%d ",data[i * w + j]);
		}
		printf("\n");
	}
};

void Block::getBlockSize(AVFormat &para, int& height, int& width)
{
	if(block_type == Block::Y)
	{
		height = para.block_height;
		width = para.block_width;
	}
	else 
	{
		height = para.block_height / 2;
		width = para.block_width / 2;
	}
}

Block::Block(Block::BlockType type,int height , int width):block_type(type),data(width * height){}

Frame::Frame()
{
}

Frame::~Frame()
{
}
int Frame::init(AVFormat& para){
	int h,w;
	para.getBlockSize(Block::Y,h,w);
	Yblock.resize(para.block_num,Block(Block::Y,h,w));
	para.getBlockSize(Block::U,h,w);
	Ublock.resize(para.block_num,Block(Block::U,h,w));
	para.getBlockSize(Block::V,h,w);
	Vblock.resize(para.block_num,Block(Block::V,h,w));

	return 0;
}
