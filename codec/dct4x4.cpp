#include "dct.h"
#include "def.h"

#include <math.h>
#include <cstdlib>

// http://blog.csdn.net/stpeace/article/details/8119041

/* 4x4����DCT�任
* �׻ݿ�
*/

#ifndef M_PI
#define M_PI 3.14159265358979323846

extern int TABLE[1500][1500];
namespace dct {
	//Cf����  
	const int Cf[4][4]=  
	{  
		1, 1,  1, 1,  
		2, 1, -1, -2,  
		1,-1, -1, 1,  
		1,-2,  2, -1  
	};  

	//Cf����ת�� 
	const int Cf_t[4][4]=
	{  
		1, 2,  1, 1,  
		1, 1, -1, -2,  
		1,-1, -1, 2,  
		1,-2,  1, -1  
	};
  
	//Ci����        
	const int Ci[4][4]=  
	{  
		2,  2,  2,  1,  
		2,  1, -2, -2,  
		2, -1, -2,  2,  
		2, -2,  2, -1  
	};  

	//Ci����ת��        
	const int Ci_t[4][4]=  
	{  
		2,  2,  2,  2,  
		2,  1, -1, -2,  
		2, -2, -2,  2,  
		1, -2,  2, -1  
	};  
  
	//MF����  
	const int MF[6][3]=  
	{  
		13107, 5243, 8066,  
		11916, 4660, 7490,  
		10082, 4194, 6554,  
		9362,  3647, 5825,  
		8192,  3355, 5243,  
		7282,  2893, 4559  
	};  
  
	//Qstep����  
	const int V[6][3]=  
	{  
		10, 16, 13,  
		11, 18, 14,  
		13, 20, 16,  
		14, 23, 18,  
		16, 25, 20,  
		18, 29, 23
	};  
} 

//������� 
void  DCT4x4Solver::matrix_multiply(const int a[][4], const int b[][4], int c[][4]) {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            c[i][j] = 0;  
            for(int k = 0; k < 4; k++) {  
                c[i][j] += a[i][k] * b[k][j];  
            }  
        }
    }        
}

// DCT������ 4*4
void DCT4x4Solver::dct_2d(int16_t *data, int start_idx, int w) {
		int x[4][4], y[4][4];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			x[i][j] = data[start_idx +TABLE[i][w] + j];
		}
	}
    matrix_multiply(dct::Cf, x, y);
    matrix_multiply(y, dct::Cf_t, x); 
    quantize(x, y);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			data[start_idx +TABLE[i][w] + j] = (int16_t)y[i][j];
		}
	}
}

// DCT����� 4*4
void DCT4x4Solver::reverse_dct_2d(int16_t *data, int start_idx, int w) {
	int x[4][4], y[4][4];
    reverse_quantize(data, start_idx, w, y);
    matrix_multiply(dct::Ci, y, x); 
    matrix_multiply(x, dct::Ci_t, y);
    for(int i = 0;i < 4; i++) {
        for(int j = 0; j < 4; j++) {
			data[start_idx + TABLE[i][w]+ j] = int( y[i][j] / 256.0 + 0.5 );
		}
	}
}

// ��������
void DCT4x4Solver::set_qp(int val) {
	this->QP = val;
}

// ��QDCT  
void DCT4x4Solver::quantize(int16_t *data, int start_idx, int w, int y[][4]) {
    // QP������qbits��f, QP��λ��(i, j)��ͬ������mf  
    int mf, k;
    int qbits = 15 + floor(QP / 6.0);
    int f = (int)( pow(2.0, qbits) / 3 );
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if (!(i & 1) && !(j & 1)) {
                k = 0;
			} else if ((i & 1) && (j & 1)) {
                k = 1;
			} else {
				k = 2;
			}
            y[i][j] = ( abs(data[start_idx + TABLE[i][w] + j]) * dct::MF[QP % 6][k] + f ) >> qbits;  
            if(data[start_idx + w + TABLE[i][w] + j] < 0) {
                y[i][j] = -y[i][j];
			}
        }
    }
} 


// QDCT���� 
void DCT4x4Solver::reverse_quantize(int16_t *data, int start_idx, int w, int y[][4]) {  
	int f = 1 << (int)(QP / 6);
    for(int k, i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
			if (!(i & 1) && !(j & 1)) {
                k = 0;
			} else if ((i & 1) && (j & 1)) {
                k = 1;
			} else {
				k = 2;
			}
            y[i][j] = data[start_idx + TABLE[i][w] + j] * dct::V[QP % 6 ][k] * f;  
        }
    }
}

// ��QDCT  
void DCT4x4Solver::quantize(int x[][4], int y[][4]) {
    // QP������qbits��f, QP��λ��(i, j)��ͬ������mf  
    int mf, k;
    int qbits = 15 + floor(QP / 6.0);
    int f = (int)( pow(2.0, qbits) / 3 );
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if (!(i & 1) && !(j & 1)) {
                k = 0;
			} else if ((i & 1) && (j & 1)) {
                k = 1;
			} else {
				k = 2;
			}
            y[i][j] = ( abs(x[i][j]) * dct::MF[QP % 6][k] + f ) >> qbits;  
            if(x[i][j] < 0) {
                y[i][j] = -y[i][j];
			}
        }
    }  
} 


// QDCT���� 
void DCT4x4Solver::reverse_quantize(int x[][4], int y[][4]) {  
	int f = 1 << (int)(QP / 6);
    for(int k, i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
			if (!(i & 1) && !(j & 1)) {
                k = 0;
			} else if ((i & 1) && (j & 1)) {
                k = 1;
			} else {
				k = 2;
			}
            y[i][j] = x[i][j] * dct::V[QP % 6 ][k] * f;  
        }
    }
}

// DCT������
int  DCT4x4Solver::dct(int16_t *data, int h, int w) {
	// 2ά�����dct�任:
    // data: һά�洢�Ĵ��任����
    // h:    ����ĸ�
    // w:    ����Ŀ�
    if ((h & 3) ||  (w & 3)) {
        printf("DCT failed.\n");
        return -1;
    }
	
	for (int i= 0; i < h; i += 4) {
        for (int j = 0; j < w; j += 4) {
            dct_2d(data,TABLE[i][w] + j, w);
        }
    }

	return 0;
}

// DCT�����
int  DCT4x4Solver::reverse_dct(int16_t *data, int h, int w) {
	// 2ά����ķ�dct�任:
    // data: һά�洢�Ĵ��任����
    // h:    ����ĸ�
    // w:    ����Ŀ�
    if ((h & 3) ||  (w & 3)) {
        printf("IDCT failed.\n");
        return -1;
    }

	for (int i= 0; i < h; i += 4) {
        for (int j = 0; j < w; j += 4) {
            reverse_dct_2d(data, TABLE[i][w] + j, w);
        }
    }

	return 0;
}

// DCT������
int  DCT4x4Solver::dct(int16_t *data, int left_row,int left_col,int right_row,int right_col,int h, int w) {
	// 2ά�����dct�任:
    // data: һά�洢�Ĵ��任����
    // h:    ����ĸ�
    // w:    ����Ŀ�
    if (((right_row - left_row + 1) & 3) || ((right_row - left_row + 1) & 3)) {
        printf("DCT failed.\n");
        return -1;
    }
	
	for (int i = left_row; i <= right_row; i += 4) {
        for (int j = left_col; j <= right_col; j += 4) {
            dct_2d(data, TABLE[i][w]+ j, w);
        }
    }

	return 0;
}

// DCT�����
int  DCT4x4Solver::reverse_dct(int16_t *data, int left_row,int left_col,int right_row,int right_col,int h,int w) {
	// 2ά����ķ�dct�任:
    // data: һά�洢�Ĵ��任����
    // h:    ����ĸ�
    // w:    ����Ŀ�
    if (((right_row - left_row + 1) & 3) || ((right_row - left_row + 1) & 3)) {
        printf("DCT failed.\n");
        return -1;
    }
	
	for (int i = left_row; i <= right_row; i += 4) {
        for (int j = left_col; j <= right_col; j += 4) {
            reverse_dct_2d(data, TABLE[i][w] + j, w);
        }
    }

	return 0;
}


#endif