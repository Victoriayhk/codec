#include "Pattern.h"


// Pattern::FUN Pattern::getPattern(int pattern){
//	return (FUN)patterns[pattern];
//}
//int Pattern::get_pattern_number(){
//	return pattern_number; 
//}

#define IN_AREA(i, j, h, w) (0 <= i && i < h && 0 <= j && j <= w) 

double Pattern::predict(Block& block,ResidualBlock & r_block,int start_r,int start_c,int end_r,int end_c, BlockBufferPool& whole_frame,int pattern_type, AVFormat &para){
	int block_w, block_h;
	r_block.getBlockSize(para, block_w, block_h);							// ��ǰblock�Ŀ�͸�
	int i_offset = (r_block.block_id / para.block_num_per_row) * block_h;	// ��ǰblock����ʼ��������Frame����
	int j_offset = (r_block.block_id % para.block_num_per_row) * block_w;	// ��ǰblock����ʼ��������Frame����

	double diff = 0;
	if (pattern_type == 0) {		// ����Ԥ��, �ο�С���Ϸ���һ������
		if (i_offset > 0) {			// ��֡��, �Ϸ��пɲο�������
			for (int i = start_r; i <= end_r && i + i_offset < para.height; i++) {
				for (int j = start_c; j <= end_c && j + j_offset < para.width; j++) {
					r_block.data[i * block_w + j] = (int16_t)block.data[i * block_w + j] - whole_frame.getValue(i_offset - 1, j_offset + j);
					diff += r_block.data[i * block_w + j] * r_block.data[i * block_w + j];
				}
			}
		} else {					// ��֡��, �Ϸ��пɲο�������, ��128���
			for (int i = start_r; i <= end_r && i + i_offset < para.height; i++) {
				for (int j = start_c; j <= end_c && j + j_offset < para.width; j++) {
					r_block.data[i * block_w + j] = (int16_t)block.data[i * block_w + j] - 128;
					diff += r_block.data[i * block_w + j] * r_block.data[i * block_w + j];
				}
			}
		}
	} else if (pattern_type == 1) {	// ������Ԥ��, �ο�С���󷽵�һ������
		if (j_offset > 0) {			// ��֡��, ���пɲο�������
			for (int i = start_r; i <= end_r && i + i_offset < para.height; i++) {
				for (int j = start_c; j <= end_c && j + j_offset < para.width; j++) {
					r_block.data[i * block_w + j] = (int16_t)block.data[i * block_w + j] - whole_frame.getValue(i_offset + i, j_offset - 1);
					diff += r_block.data[i * block_w + j] * r_block.data[i * block_w + j];
				}
			}
		} else {					// ��֡��, ���пɲο�������, ��128���
			for (int i = start_r; i <= end_r && i + i_offset < para.height; i++) {
				for (int j = start_c; j <= end_c && j + j_offset < para.width; j++) {
					r_block.data[i * block_w + j] = (int16_t)block.data[i * block_w + j] - 128;
					diff += r_block.data[i * block_w + j] * r_block.data[i * block_w + j];
				}
			}
		}
	} else if (pattern_type == 2) {	// ���Ϸ�����Ԥ��
		for (int i = start_r; i <= end_r && i + i_offset < para.height; i++) {
			for (int j = start_c; j <= end_c && j + j_offset < para.width; j++) {
				int c_i = max(i_offset - 1, i - (j - j_offset));
				int c_j = max(j_offset - 1, j - (i - i_offset));
				if (c_i > 0 && c_j > 0) {
					r_block.data[i * block_w + j] = (int16_t)block.data[i * block_w + j] - whole_frame.getValue(c_i, c_j);
				} else {	
					r_block.data[i * block_w + j] = (int16_t)block.data[i * block_w + j] - 128;
				}
				diff += r_block.data[i * block_w + j] * r_block.data[i * block_w + j];
			}
		}
	}

	return diff;
}

void Pattern::de_predict(Block& blk,ResidualBlock & r_block,int start_r,int start_c,int end_r,int end_c, BlockBufferPool& b_pool,int pattern_type, AVFormat &para) {
	int block_w, block_h;
	r_block.getBlockSize(para, block_w, block_h);							// ��ǰblock�Ŀ�͸�
	int i_offset = (r_block.block_id / para.block_num_per_row) * block_h;	// ��ǰblock����ʼ��������Frame����
	int j_offset = (r_block.block_id % para.block_num_per_row) * block_w;	// ��ǰblock����ʼ��������Frame����

	if (pattern_type == 0) {		// ����Ԥ��, �ο�С���Ϸ���һ������
		if (i_offset > 0) {			// ��֡��, �Ϸ��пɲο�������s
			for (int i = start_r; i <= end_r && i + i_offset < para.height; i++) {
				for (int j = start_c; j <= end_c && j + j_offset < para.width; j++) {
					blk.data[i * block_w + j] = r_block.data[i * block_w + j] + b_pool.getValue(i_offset - 1, j_offset + j);
					b_pool.setValue(i_offset + i, j_offset +j, r_block.data[i * block_w + j] + b_pool.getValue(i_offset - 1, j_offset + j));
				}
			}
		} else {					// ��֡��, �Ϸ��пɲο�������, ��128���
			for (int i = start_r; i <= end_r && i + i_offset < para.height; i++) {
				for (int j = start_c; j <= end_c && j + j_offset < para.width; j++) {
					blk.data[i * block_w + j] = r_block.data[i * block_w + j] + 128;
					b_pool.setValue(i_offset + i, j_offset +j, r_block.data[i * block_w + j] + 128);
				}
			}
		}
	} else if (pattern_type == 1) {	// ������Ԥ��, �ο�С���󷽵�һ������
		if (j_offset > 0) {			// ��֡��, ���пɲο�������
			for (int i = start_r; i <= end_r && i + i_offset < para.height; i++) {
				for (int j = start_c; j <= end_c && j + j_offset < para.width; j++) {
					blk.data[i * block_w + j] = r_block.data[i * block_w + j] + b_pool.getValue(i_offset + i, j_offset - 1);
					b_pool.setValue(i_offset + i, j_offset +j, (int16_t)r_block.data[i * block_w + j] + b_pool.getValue(i_offset + i, j_offset - 1));
				}
			}
		} else {					// ��֡��, ���пɲο�������, ��128���
			for (int i = start_r; i <= end_r && i + i_offset < para.height; i++) {
				for (int j = start_c; j <= end_c && j + j_offset < para.width; j++) {
					blk.data[i * block_w + j] = r_block.data[i * block_w + j] + 128;
					b_pool.setValue(i_offset + i, j_offset +j, (int16_t)r_block.data[i * block_w + j] + 128);
				}
			}
		}
	} else if (pattern_type == 2) {	// ���Ϸ�����Ԥ��
		for (int i = start_r; i <= end_r && i + i_offset < para.height; i++) {
				for (int j = start_c; j <= end_c && j + j_offset < para.width; j++) {
				int c_i = max(i_offset - 1, i - (j - j_offset));
				int c_j = max(j_offset - 1, j - (i - i_offset));
				
				if (c_i > 0 && c_j > 0) {
					blk.data[i * block_w + j] = r_block.data[i * block_w + j] + b_pool.getValue(c_i, c_j);
					b_pool.setValue(i_offset + i, j_offset +j, (int16_t)r_block.data[i * block_w + j] + b_pool.getValue(c_i, c_j));
				} else {	
					blk.data[i * block_w + j] = r_block.data[i * block_w + j] + 128;
					b_pool.setValue(i_offset + i, j_offset +j, (int16_t)r_block.data[i * block_w + j] + 128);
				}
			}
		}
	}
}
