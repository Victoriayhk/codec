#include<cstdio>
#include<vector>
#include<cstring>
#include "def.h"
using namespace std;

////////////////////////////�ݶ�

typedef unsigned char uint8_t;


///////////////////////////////

inline Block make_block(uint8_t *stream, int col_id, int block_h, int block_w,int frame_width) {
	Block block(block_h , block_w);
	//block.data = new uint8_t[block_h * block_w];
	for (int i = 0; i < block_h; i++) {
		memcpy(&block.data[i * block_w], &stream[i *frame_width + col_id * block_w], sizeof(uint8_t)*block_w);
	}
	return block;
}

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
	
	// if (fread(frame.Ys, sizeof(unsigned char), Frame::nSize, fin) == -1) return -1;
	// if (fread(frame.Us, sizeof(unsigned char), frame.nSize/4, fin) == -1) return -1;
	// if (fread(frame.Vs, sizeof(unsigned char), frame.nSize/4, fin) == -1) return -1;

	if (fread(Ys, sizeof(unsigned char), nsize, fin) == -1) return -1;
	unsigned char *tmp = new unsigned char[nsize/2];
	if (fread(tmp, sizeof(unsigned char), nsize/2, fin) == -1) return -1;
	for (int i = 0; i < nsize/2; i+=2) {
		Us[i/2] = tmp[i];
		Vs[i/2] = tmp[i+1];
	}
	//if (fread(Us, sizeof(unsigned char), nsize/4, fin) == -1) return -1;
	//if (fread(Vs, sizeof(unsigned char), nsize/4, fin) == -1) return -1;

	int n_row = para.height / para.block_height;
	int n_col = para.width / para.block_width;
	for (int i = 0; i < n_row; i++) {
		for (int j = 0; j < n_col; j++) {
			frame.Yblock.push_back(make_block(&Ys[i * para.block_height], j, para.block_height, para.block_width,para.width));
			frame.Ublock.push_back(make_block(&Us[i * para.block_height/2], j, para.block_height/2, para.block_width/2,para.width));
			frame.Vblock.push_back(make_block(&Vs[i * para.block_height/2], j, para.block_height/2, para.block_width/2,para.width));
		}
	}


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