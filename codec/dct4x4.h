#ifndef _DCT4X4_H_
#define _DCT4X4_H_

#include "def.h"
#include "stdint.h"

class DCT4x4Solver {
	int QP; // ��������

	int D[4][4];  //�м����  
	int Di[4][4]; //�м����
	int W[4][4];  //�˾���  
	int Z[4][4];  //QDCT����  
	int Wi[4][4]; //Wi����  
	int Xi[4][4]; //����Ĳв����  

	// ��������
	void matrix_transform(int a[][4]);
	void matrix_subtract(int a[][4],int b[][4]);
	void matrix_multiply(const int a[][4], const int b[][4], int c[][4]);

	// ����
	void quantize(int W[][4]);
	void reverse_quantize(int Z[][4]);

	// �ӹ���
	void dct_1d(double *in, double *out, const int n);
	void dct_2d(int16_t *data, int start_idx, int w);
	void reverse_dct_2d(int16_t *data, int start_idx, int w);

public:
	DCT4x4Solver(){
		QP = 0;  
	}
	void set_qp(int qt);
	int dct(int16_t *data, int h, int w);
	int reverse_dct(int16_t *data, int h, int w);
};

#endif