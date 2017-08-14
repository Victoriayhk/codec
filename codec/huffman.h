/*
 *  huffman_coder - Encode/Decode files using Huffman encoding.
 *  http://huffman.sourceforge.net
 *  Copyright (C) 2003  Douglas Ryan Richardson
 */

/*
*	2017-8-13 gaowenkui
*	ʵ�������������ر������
*	��ɳ�Huffman�������庯������Ĳ���
*	����֡������ģʽ
*	��鼶����ģʽ��slice������ģʽδ����
*/
#ifndef HUFFMAN_HUFFMAN_H
#define HUFFMAN_HUFFMAN_H

#define BLOCK_NUM 8

#include <stdio.h>
#include <stdint.h>
#include "ResidualBlock.h"

#define positive 0
#define negative 1

//�����ú���
//void entropy_test();
void head_test();
void zag_zig_test();

//����
int entropy_encode_pkt(PKT& pkt, AVFormat& para, uint8_t **stream, unsigned int *len);

//int entropy_coding_slice(ResidualBlock* rBlock, int block_num, AVFormat& para, uint8_t **stream);
//int entropy_decode_slice(ResidualBlock* rBlock, int block_num, AVFormat& para, uint8_t *stream, int buff_length);
/*
* �Բв�block�����ر���ת������
* ���룺
* f_x f_y l_x l_y	�в�����ϽǼ����½�����
* rBlock			�в��
* para				�����ļ�
* stream			���ر�������
*/

// slice����������
int entropy_encode_slice(ResidualBlock* rBlock ,int block_len, AVFormat& para, uint8_t **stream,unsigned int* len);
// slice������������
int entropy_decode_slice(ResidualBlock* rBlock,int block_len , AVFormat& para, uint8_t *stream, unsigned int buff_length);

// ֡����������
int entropy_encode_by_frame(ResidualBlock* rBlock ,int block_len, AVFormat& para, uint8_t **stream,unsigned int* len);
// ֡������������
int entropy_decode_by_frame(ResidualBlock* rBlock,int block_len , AVFormat& para, uint8_t *stream, unsigned int buff_length);

// Zɨ�����
int zag_zig(ResidualBlock& rBlock, AVFormat& para, uint8_t* zag_zig_stream);

// ��Zɨ�����
int unzag_zig(ResidualBlock& rBlock, AVFormat& para, uint8_t* zag_zig_stream, int zero_num);

// ��鼶��������
int entropy_encode_block(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t **stream);

// ��鼶����������
int entropy_encode_block_2(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t **stream);

// ֡���µĺ����������
int entropy_encode_block_by_frame(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t **stream);

// ��鼶����������
int entropy_decode_block(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t *stream, int buff_length);

// ֡���µĺ�鷴��������
int entropy_decode_block_by_frame(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t *stream);

// �в��ת��������
int entropy_to_stream(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t** stream);	//�в��ת9bit��������
int entropy_to_stream_bit(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t** stream, int bit_len = 8);

// ��������ת�в��
int entropy_from_stream(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t* stream);	//9bit��������ת�в��
int entropy_from_stream_bit(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t* stream, int bit_len = 8);

/*
	ʹ�õ�huffman���뺯��
*/
int huffman_encode_file(FILE *in, FILE *out);
int huffman_decode_file(FILE *in, FILE *out);
int huffman_encode_memory(const unsigned char *bufin,
						  uint32_t bufinlen,
						  unsigned char **pbufout,
						  uint32_t *pbufoutlen);
int huffman_decode_memory(const unsigned char *bufin,
						  uint32_t bufinlen,
						  unsigned char **bufout,
						  uint32_t *pbufoutlen);

						  

template<typename T>
inline void fromch4(T& result, uint8_t* val)	// ���������͵���ת��Ϊuint8_t������
{
	result = 0;
	int len = sizeof(T);
	for(int i = 0;i<len;++i)
	{
		result <<= 8;
		result |= val[i];
	}
}

template<typename T>
inline void toch4(T val, uint8_t* result)	// ���������͵���ת��Ϊuint8_t������
{
	int len = sizeof(T);
	for(int i = 0;i<len;++i)
	{
		result[i] = (uint8_t)(val>>(8*(len-i -1)) & 0x000000ff);
	}
}
#endif
