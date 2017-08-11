#include<cstdio>
#include<vector>
#include<cstring>
#include "def.h"
using namespace std;

extern int TABLE[1500][1500];

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
	
	frame.frame_id = frame_cnt - 1;

	for (int i = 0; i < para.block_num_per_col; i++) {
		for (int j = 0; j < para.block_height; j++) {
			for (int k = 0; k < para.block_num_per_row; k++) {
				int cnt =TABLE[i][para.block_num_per_row]+ k;
				frame.Yblock[cnt].block_id = cnt;
				frame.Yblock[cnt].block_type = Block::Y;
				fread(&(frame.Yblock[cnt].data[TABLE[j][para.block_width]]), sizeof(uint8_t), para.block_height, fin);
			}
		}
	}
	for (int i = 0; i < para.block_num_per_col; i++) {
		for (int j = 0; j < para.block_height/2; j++) {
			for (int k = 0; k < para.block_num_per_row; k++) {
				int cnt = TABLE[i][para.block_num_per_row] + k;
				frame.Ublock[cnt].block_id = cnt;
				frame.Ublock[cnt].block_type = Block::U;
				fread(&(frame.Ublock[cnt].data[TABLE[j][para.block_width/2]]), sizeof(uint8_t), para.block_height/2, fin);
			}
		}
	}
	for (int i = 0; i < para.block_num_per_col; i++) {
		for (int j = 0; j < para.block_height/2; j++) {
			for (int k = 0; k < para.block_num_per_row; k++) {
				int cnt = TABLE[i][para.block_num_per_row] + k;
				frame.Vblock[cnt].block_id = cnt;
				frame.Vblock[cnt].block_type = Block::V;
				fread(&(frame.Vblock[cnt].data[TABLE[j][para.block_width/2]]), sizeof(uint8_t), para.block_height/2, fin);
			}
		}
	}
	return 0;
}

int yuv_write(AVFormat & para, Frame &frame) {
	FILE * fout = para.out_video;
	static int frame_cnt = 0;
	frame_cnt ++;
	if (frame_cnt > para.frame_num) {
		return -1;
	}

	for (int i = 0; i < para.block_num_per_col; i++) {
		for (int j = 0; j < para.block_height; j++) {
			for (int k = 0; k < para.block_num_per_row; k++) {
				int cnt = TABLE[i][para.block_num_per_row] + k;
				fwrite(&(frame.Yblock[cnt].data[TABLE[j][para.block_width]]), sizeof(uint8_t), para.block_height, fout);
			}
		}
	}
	for (int i = 0; i < para.block_num_per_col; i++) {
		for (int j = 0; j < para.block_height/2; j++) {
			for (int k = 0; k < para.block_num_per_row; k++) {
				int cnt =TABLE[i][para.block_num_per_row]+ k;
				fwrite(&(frame.Ublock[cnt].data[TABLE[j][para.block_width/2]]), sizeof(uint8_t), para.block_height/2, fout);
			}
		}
	}
	for (int i = 0; i < para.block_num_per_col; i++) {
		for (int j = 0; j < para.block_height/2; j++) {
			for (int k = 0; k < para.block_num_per_row; k++) {
				int cnt = TABLE[i][para.block_num_per_row] + k;
				fwrite(&(frame.Vblock[cnt].data[TABLE[j][para.block_width/2]]), sizeof(uint8_t), para.block_height/2, fout);
			}
		}
	}
	return 0;
}