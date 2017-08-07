#include "dct4x4.h"
#include "def.h"
#include <math.h>
#include <cstdlib>

#ifndef M_PI
#define M_PI 3.14159265358979323846

namespace dct {
	//Cf矩阵  
	int Cf[4][4]=  
	{  
		1, 1,  1, 1,  
		2, 1, -1, -2,  
		1,-1, -1, 1,  
		1,-2,  2, -1  
	};  
  
	//Ci矩阵        
	int Ci[4][4]=  
	{  
		2,  2,  2,  1,  
		2,  1, -2, -2,  
		2, -1, -2,  2,  
		2, -2,  2, -1  
	};  
  
	//MF矩阵  
	int MF[6][3]=  
	{  
		13107, 5243, 8066,  
		11916, 4660, 7490,  
		10082, 4194, 6554,  
		9362,  3647, 5825,  
		8192,  3355, 5243,  
		7282,  2893, 4559  
	};  
  
	//Qstep矩阵  
	int V[6][3]=  
	{  
		10, 16, 13,  
		11, 18, 14,  
		13, 20, 16,  
		14, 23, 18,  
		16, 25, 20,  
		18, 29, 23  
	};  
}
using namespace dct;

//矩阵转置  
void DCT4x4Solver::matrix_transform(int a[][4]) {  
    int tmp;  
    for(int i = 0; i < 4; i++) {  
        for(int j = i + 1; j < 4; j++) {  
            tmp     = a[i][j];  
            a[i][j] = a[j][i];  
            a[j][i] = tmp;
        }  
    }
}  
  
//矩阵求差  
void  DCT4x4Solver::matrix_subtract(int a[][4],int b[][4]) { 
    for(int i = 0;i < 4; i++) {  
        for(int j = 0; j < 4; j++) {
            a[i][j] -= b[i][j];  
        }  
    } 
}

//矩阵求积 
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

// DCT正过程 4*4
void DCT4x4Solver::dct_2d(int16_t *data, int start_idx, int w) {
		int x[4][4];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			x[i][j] = data[start_idx + i * w + j];
		}
	}
	//此时orgYUV变为残差矩阵
    matrix_multiply(Cf, x, D);
    matrix_transform(Cf);
    matrix_multiply(D, Cf, W);    
    matrix_transform(Cf);    

	//得到的W即为核
	//cout << "Matrix Core（W） is" << endl;
 //   matrixShow(W);

	//利用核W来得到QDCT（Z）
    quantize(W);
	//cout << "Matrix QDCT(Z) is" << endl;
 //   matrixShow(Z); 

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			//if (x[i][j] > 1024) x[i][j] = 1024;
			data[start_idx + i * w + j] = (int16_t) Z[i][j];
		}
	}
}

// DCT逆过程 4*4
void DCT4x4Solver::reverse_dct_2d(int16_t *data, int start_idx, int w) {
	int x[4][4];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			x[i][j] = data[start_idx + i * w + j];
		}
	}

	//利用QDCT(Z)得到解码端的DCT(Wi).(Wi与编码端DCT必然不同）
    reverse_quantize(x);

    //利用Wi得到解码的残差矩阵Xi
    matrix_multiply(Ci, Wi, Di); 
	matrix_transform(Ci);        
    matrix_multiply(Di, Ci, Xi);
	matrix_transform(Ci); 
    for(int i = 0;i < 4; i++)
	{
        for(int j = 0; j < 4; j++)
		{
			Xi[i][j] = int( Xi[i][j] / 256.0 + 0.5 );
		}
	}

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			data[start_idx + i * w + j] = (int16_t) Xi[i][j];
		}
	}
}

// 设置质量
void DCT4x4Solver::set_qp(int val) {
	this->QP = val;
}

// 求QDCT  
void DCT4x4Solver::quantize(int W[][4]) {
    // QP决定了qbits和f, QP和位置(i, j)共同决定了mf  
    int mf, k;  
    int qbits = 15 + floor(QP / 6.0);  
    int f = (int)( pow(2.0, qbits) / 3 );  
  
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if((0 == i || 2 == i) && (0 == j || 2 == j))  
                k = 0;  
            else if((1 == i || 3 == i) && (1 == j || 3 == j))  
                k = 1;  
            else  
                k = 2;  
  
            mf = MF[QP % 6][k];  
            Z[i][j] = ( abs(W[i][j]) * mf + f ) >> qbits;  
            if(W[i][j] < 0)  
                Z[i][j] = -Z[i][j];  
        }
    }  
} 


// QDCT逆向 
void DCT4x4Solver::reverse_quantize(int Z[][4]) {    
    int t = floor(QP / 6.0);                  
    int f = (int)pow(2.0, t);   
    int v, i, j, k;  
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            if((0 == i || 2 == i) && (0 == j || 2 == j))  
                k = 0;  
            else if((1 == i || 3 == i) && (1 == j || 3 == j))  
                k = 1;  
            else  
                k = 2;  
            v = V[QP % 6 ][k];  
            Wi[i][j] = Z[i][j] * v * f;  
        }  
    }
}


// DCT正过程
int  DCT4x4Solver::dct(int16_t *data, int h, int w) {
	// 2维矩阵的dct变换:
    // data: 一维存储的待变换数据
    // h:    矩阵的高
    // w:    矩阵的宽
    if (h % 4 != 0 || w % 4 != 0) {
        printf("DCT failed.\n");
        return -1;
    }
	
	for (int i= 0; i < h; i += 4) {
        for (int j = 0; j < w; j += 4) {
            dct_2d(data, i * w + j, w);
        }
    }

	return 0;
}

// DCT逆过程
int  DCT4x4Solver::reverse_dct(int16_t *data, int h, int w) {
	// 2维矩阵的反dct变换:
    // data: 一维存储的待变换数据
    // h:    矩阵的高
    // w:    矩阵的宽
    if (h % 4 != 0 || w % 4 != 0) {
        printf("IDCT failed.\n");
        return -1;
    }

	for (int i= 0; i < h; i += 4) {
        for (int j = 0; j < w; j += 4) {
            reverse_dct_2d(data, i * w + j, w);
        }
    }

	return 0;
}

#endif