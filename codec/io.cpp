#include<cstdio>
#include<vector>
#include<cstring>
#include "def.h"
using namespace std;



/*
* 从指定的开始位置创建一个Y/U/V型的block
Args:
	stream:  当前帧的Y/U/V流
	 start_i: 块的起始行
	 start_j: 块的起始列
	 para:    视频信息
	 channel: 'Y' or 'U' or 'V'
Return:
	一个处理好的block, 包括以下信息:
		1. block.data: 对应通道的像素
		2. block.block_type: block的通道类型 y/u/v
	**没有指定block的id**
*/
inline Block make_block(uint8_t *stream, int start_i, int start_j, AVFormat & para, int channel) {
	Block block;
	if (channel == 'Y') {  // Y通道, 大小为para.block_height * para.block_width
		block.data.resize(para.block_height * para.block_width, 0);
		int end_i = min(start_i + para.block_height, para.height);
		int real_width = min(start_j + para.block_width, para.width) - start_j;
		for (int i = start_i; i < end_i; i++) {
			memcpy(&block.data[(i-start_i) * para.block_width], &stream[i*para.width + start_j], sizeof(uint8_t)*real_width);
		}
		block.block_type = Block::Y;
	} else {				// U/V通道, 大小只有Y的一半
		block.data.resize(para.block_height * para.block_width/4, 0);
		int end_i = min(start_i + para.block_height/2, para.height/2);
		int real_width = min(start_j + para.block_width/2, para.width/2) - start_j;
		for (int i = start_i; i < end_i; i++) {
			memcpy(&block.data[(i-start_i) * para.block_width/2], &stream[i*para.width/2 + start_j], sizeof(uint8_t)*real_width);
		}
		if (channel == 'U') {
			block.block_type = Block::U;
		} else if (channel == 'V') {
			block.block_type = Block::V;
		}
	}
	return block;
}


inline int block_to_stream(uint8_t *stream, Block &block, int start_i, int start_j, AVFormat para, char channel) {
	if (channel == 'Y') {  // Y通道, 大小为para.block_height * para.block_width
		int end_i = min(start_i + para.block_height, para.height);
		int real_width = min(start_j + para.block_width, para.width) - start_j;
		for (int i = start_i; i < end_i; i++) {
			memcpy(&stream[i*para.width + start_j], &block.data[(i-start_i) * para.block_width], sizeof(uint8_t)*real_width);
		}
	} else {				// U/V通道, 大小只有Y的一半
		block.data.resize(para.block_height * para.block_width/4, 0);
		int end_i = min(start_i + para.block_height/2, para.height/2);
		int real_width = min(start_j + para.block_width/2, para.width/2) - start_j;
		for (int i = start_i; i < end_i; i++) {
			memcpy(&stream[i*para.width/2 + start_j], &block.data[(i-start_i) * para.block_width/2], sizeof(uint8_t)*real_width);
		}
	}
	return 0;
}


/*
* 从AVFormat para中指定的文件的当前位置读入一帧数据
Args: 
	para: 统一参数配置类
	frame: 目标帧
Returns:
	读取成功与否的状态, -1表示失败
*/
int yuv_read(AVFormat & para, Frame &frame) {
	FILE * fin = para.video;
	static int frame_cnt = 0;
	frame_cnt ++;
	if (frame_cnt > para.frame_num) {
		return -1;
	}
	const int nsize = para.height * para.width;
	uint8_t *Ys = new uint8_t[nsize];
	uint8_t *Us = new uint8_t[nsize/4];
	uint8_t *Vs = new uint8_t[nsize/4];
	
	if (fread(Ys, sizeof(uint8_t), nsize, fin) == -1) return -1;
	if (fread(Us, sizeof(uint8_t), nsize/4, fin) == -1) return -1;
	if (fread(Vs, sizeof(uint8_t), nsize/4, fin) == -1) return -1;

	for (int i = 0; i < para.height; i += para.block_height) {
		for (int j = 0; j < para.width; j += para.block_width) {
			frame.Yblock.push_back(make_block(Ys, i, j, para, 'Y'));
			frame.Ublock.push_back(make_block(Us, i/2, j/2, para, 'U'));
			frame.Vblock.push_back(make_block(Vs, i/2, j/2, para, 'V'));
		}
	}
	for (size_t i = 0; i < (int)frame.Yblock.size(); i++) {
		frame.Yblock[i].block_id = i;
		frame.Ublock[i].block_id = i;
		frame.Vblock[i].block_id = i;
	}
	delete Ys;
	delete Us;
	delete Vs;

	return 0;
}

int yuv_write(AVFormat & para, Frame &frame) {
	FILE * fout = para.out_video;
	static int frame_cnt = 0;
	frame_cnt ++;
	if (frame_cnt > para.frame_num) {
		return -1;
	}

	const int nsize = para.height * para.width;
	uint8_t *Ys = new uint8_t[nsize];
	uint8_t *Us = new uint8_t[nsize/4];
	uint8_t *Vs = new uint8_t[nsize/4];
	
	int cnt = 0;
	for (int i = 0; i < para.height; i += para.block_height) {
		for (int j = 0; j < para.width; j += para.block_width) {
			block_to_stream(Ys, frame.Yblock[cnt], i, j, para, 'Y');
			block_to_stream(Us, frame.Ublock[cnt], i / 2, j / 2, para, 'U');
			block_to_stream(Vs, frame.Vblock[cnt], i / 2, j / 2, para, 'V');
			cnt ++;
		}
	}

	fwrite(Ys, sizeof(uint8_t), nsize, fout);
	fwrite(Us, sizeof(uint8_t), nsize/4, fout);
	fwrite(Vs, sizeof(uint8_t), nsize/4, fout);
	delete Ys;
	delete Us;
	delete Vs;
	return 0;
}

// int write_frames_to_file(Frame *pframe, FILE *fout) {
// 	for (int i = 0; i < video.nFrame; i++) {
// 		fwrite(pframe[i].Ys, sizeof(unsigned char), Frame::nsize, fout);
// 		fwrite(pframe[i].Us, sizeof(unsigned char), Frame::nsize/4, fout);
// 		fwrite(pframe[i].Vs, sizeof(unsigned char), Frame::nsize/4, fout);
// 	}

// 	return 0;
// }
/*
int main(int argc, char *argv[]) {
	FILE *fin, *fout;
	fin = fopen(argv[1], "rb");
	fout = fopen(argv[2], "wb");


	Frame *frames = new Frame[video.nFrame];
	for (int i = 0; i < video.nFrame; i++) {
		printf("%d: %d\n", i, yuv_read(fin, frames[i]));
	}
	// write_frames_to_file(frames, fout);
}
*/