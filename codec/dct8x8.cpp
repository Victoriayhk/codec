#include "dct.h"
#include "def.h"
#include <math.h>
#include <cstdlib>
#include <cstdio>

#ifndef M_PI
#define M_PI 3.14159265358979323846

/* 8x8 DCT变换
* 易惠康
*/

void DCT8x8Solver::matrix_dct(double org[][8], double des[][8]) {
	//printf("run_dct_8x8\n");
    for (int u = 0; u < 8; ++u) {
        for (int v = 0; v < 8; ++v) {
			des[u][v] = 0;
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    des[u][v] += org[i][j] * cos(M_PI*0.125*(i+0.5)*u)*cos(M_PI*0.125*(j+0.5)*v);
                }               
            }
        }
    }
}


void DCT8x8Solver::matrix_idct(double org[][8], double des[][8]) {
	for (int u = 0; u < 8; ++u) {
        for (int v = 0; v < 8; ++v) {
			des[u][v] = 1/4.*org[0][0];
			for(int i = 1; i < 8; i++){
				des[u][v] += 0.5 * org[i][0];
			}
			for(int j = 1; j < 8; j++) {
				des[u][v] += 0.5 * org[0][j];
			}

			for (int i = 1; i < 8; i++) {
				for (int j = 1; j < 8; j++) {
					org[u][v] += org[i][j] * cos(M_PI * 0.125 *(u+1./2.)*i)*cos(M_PI*0.125*(v+0.5)*j);
				}               
			}
			des[u][v] *= 0.0625;
        }
    }
}

// DCT正过程 4*4
void DCT8x8Solver::dct_2d(int16_t *data, int start_idx, int w) {
	double x[8][8], y[8][8];
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			x[i][j] = data[start_idx + i * w + j];
		}
	}

	matrix_dct(x, y);

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			data[start_idx + i * w + j] = (int16_t) y[i][j];
		}
	}
}

// DCT逆过程 8*8
void DCT8x8Solver::reverse_dct_2d(int16_t *data, int start_idx, int w) {
	double x[8][8], y[8][8];
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			x[i][j] = data[start_idx + i * w + j];
		}
	}

	matrix_idct(x, y);

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			data[start_idx + i * w + j] = (int16_t) y[i][j];
		}
	}
}


#endif

// DCT正过程
int  DCT8x8Solver::dct(int16_t *data, int h, int w) {
	// 2维矩阵的dct变换:
    // data: 一维存储的待变换数据
    // h:    矩阵的高
    // w:    矩阵的宽
    if (h % 8 != 0 || w % 8 != 0) {
        printf("DCT failed.\n");
        return -1;
    }
	
	for (int i= 0; i < h; i += 8) {
        for (int j = 0; j < w; j += 8) {
            dct_2d(data, i * w + j, w);
        }
    }

	return 0;
}

// DCT逆过程
int  DCT8x8Solver::reverse_dct(int16_t *data, int h, int w) {
	// 2维矩阵的反dct变换:
    // data: 一维存储的待变换数据
    // h:    矩阵的高
    // w:    矩阵的宽
    if (h % 8 != 0 || w % 8 != 0) {
        printf("IDCT failed.\n");
        return -1;
    }

	for (int i= 0; i < h; i += 8) {
        for (int j = 0; j < w; j += 8) {
            reverse_dct_2d(data, i * w + j, w);
        }
    }

	return 0;
}

// DCT正过程
int  DCT8x8Solver::dct(int16_t *data, int left_row,int left_col,int right_row,int right_col,int h, int w) {
	// 2维矩阵的dct变换:
    // data: 一维存储的待变换数据
    // h:    矩阵的高
    // w:    矩阵的宽
    if ((right_row - left_row + 1) % 8 != 0 || (right_row - left_row + 1) % 8 != 0) {
        printf("DCT failed.\n");
        return -1;
    }
	
	for (int i = left_row; i <= right_row; i += 8) {
        for (int j = left_col; j <= right_col; j += 8) {
            dct_2d(data, i * w + j, w);
        }
    }

	return 0;
}

// DCT逆过程
int  DCT8x8Solver::reverse_dct(int16_t *data, int left_row,int left_col,int right_row,int right_col,int h,int w) {
	// 2维矩阵的反dct变换:
    // data: 一维存储的待变换数据
    // h:    矩阵的高
    // w:    矩阵的宽
    if ((right_row - left_row + 1) % 8 != 0 || (right_row - left_row + 1) % 8 != 0) {
        printf("DCT failed.\n");
        return -1;
    }
	
	for (int i = left_row; i <= right_row; i += 8) {
        for (int j = left_col; j <= right_col; j += 8) {
            reverse_dct_2d(data, i * w + j, w);
        }
    }

	return 0;
}
