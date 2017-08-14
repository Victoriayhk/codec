//
//  dct.h
//  miniVideo
//
//  Created by Jules on 2017/8/6.
//  Copyright © 2017年 Jules. All rights reserved.
//

#ifndef dct_h
#define dct_h

#include <cmath>

template<class T>
class DCT4x4Solver {
	int QP; // 量化参数

	int D[4][4];  //中间矩阵
	int Di[4][4]; //中间矩阵
	int W[4][4];  //核矩阵
	int Z[4][4];  //QDCT矩阵
	int Wi[4][4]; //Wi矩阵
	int Xi[4][4]; //解码的残差矩阵

	// 矩阵运算
	void matrix_transform(int a[][4]);
	void matrix_subtract(int a[][4],int b[][4]);
	void matrix_multiply(const int a[][4], const int b[][4], int c[][4]);

	// 量化
	void quantize(int W[][4]);
	void reverse_quantize(int Z[][4]);

	// 子过程
	void dct_1d(double *in, double *out, const int n);
	void dct_2d(T &data, int i, int j);
	void reverse_dct_2d(T &data, int start_idx, int w);

public:
	DCT4x4Solver(){
		QP = 0;
	}
	void set_qp(int qt);
	int dct(T &data, int h, int w);
	int reverse_dct(T &data, int h, int w);
};

namespace dct {
	//Cf矩阵
	int Cf[4][4] = {
		1, 1,  1, 1,
		2, 1, -1, -2,
		1,-1, -1, 1,
		1,-2,  2, -1
	};

	//Ci矩阵
	int Ci[4][4] = {
		2,  2,  2,  1,
		2,  1, -2, -2,
		2, -1, -2,  2,
		2, -2,  2, -1
	};

	//MF矩阵
	int MF[6][3] = {
		13107, 5243, 8066,
		11916, 4660, 7490,
		10082, 4194, 6554,
		9362,  3647, 5825,
		8192,  3355, 5243,
		7282,  2893, 4559
	};

	//Qstep矩阵
	int V[6][3] = {
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
void DCT4x4Solver::matrix_subtract(int a[][4],int b[][4]) {
    for(int i = 0;i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            a[i][j] -= b[i][j];
        }
    }
}

//矩阵求积
void DCT4x4Solver::matrix_multiply(const int a[][4], const int b[][4], int c[][4]) {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            c[i][j] = 0;
            for(int k = 0; k < 4; k++) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

#ifndef M_PI
#define M_PI 3.14159265358979323846


// DCT正过程 4*4
void DCT4x4Solver::dct_2d(int16 **data, int start_i, int start_j) {
	int x[4][4];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			x[i][j] = data[start_i + i][start_j + j];
		}
	}
	//此时orgYUV变为残差矩阵
    matrix_multiply(Cf, x, D);
    matrix_transform(Cf);
    matrix_multiply(D, Cf, W);
    matrix_transform(Cf);
    quantize(W);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			data[start_i + i][start_j + i] = Z[i][j];
		}
	}
}

// DCT逆过程 4*4
void DCT4x4Solver::reverse_dct_2d(T &data, int start_i, int start_j) {
	int x[4][4];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			x[i][j] = data[start_i + i][start_j +j];
		}
	}
    reverse_quantize(x);
    matrix_multiply(Ci, Wi, Di);
	matrix_transform(Ci);
    matrix_multiply(Di, Ci, Xi);
	matrix_transform(Ci);
    for(int i = 0;i < 4; i++) {
        for(int j = 0; j < 4; j++) {
			Xi[i][j] = int( Xi[i][j] / 256.0 + 0.5 );
		}
	}

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			data[start_i + i][start_j + i] = Xi[i][j];
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
template<class T>
void DCT4x4Solver<T>::reverse_quantize(int Z[][4]) {
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
template<class T>
int  DCT4x4Solver::dct(T &data, int h, int w) {
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
            dct_2d(data, i, j);
        }
    }

	return 0;
}

// DCT逆过程
template<class T>
int  DCT4x4Solver::reverse_dct(T &data, int h, int w) {
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
            reverse_dct_2d(data, i, j);
        }
    }

	return 0;
}

#endif  /* M_PI */
#endif /* dct_h */
