/*
 *  huffman_coder - Encode/Decode files using Huffman encoding.
 *  http://huffman.sourceforge.net
 *  Copyright (C) 2003  Douglas Ryan Richardson
 */

/*
*	2017-8-13 gaowenkui
*	实现数据流化及熵编码操作
*	完成除Huffman编解码具体函数以外的部分
*	采用帧级流化模式
*	宏块级流化模式和slice级流化模式未启用
*/
#ifndef HUFFMAN_HUFFMAN_H
#define HUFFMAN_HUFFMAN_H

#define BLOCK_NUM 8

#include <stdio.h>
#include <stdint.h>
#include "ResidualBlock.h"

#define positive 0
#define negative 1

//测试用函数
//void entropy_test();
void head_test();
void zag_zig_test();

//弃用
int entropy_encode_pkt(PKT& pkt, AVFormat& para, uint8_t **stream, unsigned int *len);

//int entropy_coding_slice(ResidualBlock* rBlock, int block_num, AVFormat& para, uint8_t **stream);
//int entropy_decode_slice(ResidualBlock* rBlock, int block_num, AVFormat& para, uint8_t *stream, int buff_length);
/*
* 对残差block进行熵编码转化成流
* 输入：
* f_x f_y l_x l_y	残差的左上角及右下角坐标
* rBlock			残差块
* para				配置文件
* stream			返回编码后的流
*/

// slice级流化处理
int entropy_encode_slice(ResidualBlock* rBlock ,int block_len, AVFormat& para, uint8_t **stream,unsigned int* len);
// slice级反流化处理
int entropy_decode_slice(ResidualBlock* rBlock,int block_len , AVFormat& para, uint8_t *stream, unsigned int buff_length);

// 帧级流化处理
int entropy_encode_by_frame(ResidualBlock* rBlock ,int block_len, AVFormat& para, uint8_t **stream,unsigned int* len);
// 帧级反流化处理
int entropy_decode_by_frame(ResidualBlock* rBlock,int block_len , AVFormat& para, uint8_t *stream, unsigned int buff_length);

// Z扫描操作
int zag_zig(ResidualBlock& rBlock, AVFormat& para, uint8_t* zag_zig_stream);

// 反Z扫描操作
int unzag_zig(ResidualBlock& rBlock, AVFormat& para, uint8_t* zag_zig_stream, int zero_num);

// 宏块级流化处理
int entropy_encode_block(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t **stream);

// 宏块级反流化处理
int entropy_encode_block_2(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t **stream);

// 帧级下的宏块流化处理
int entropy_encode_block_by_frame(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t **stream);

// 宏块级反流化处理
int entropy_decode_block(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t *stream, int buff_length);

// 帧级下的宏块反流化处理
int entropy_decode_block_by_frame(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t *stream);

// 残差块转二进制流
int entropy_to_stream(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t** stream);	//残差块转9bit二进制流
int entropy_to_stream_bit(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t** stream, int bit_len = 8);

// 二进制流转残差块
int entropy_from_stream(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t* stream);	//9bit二进制流转残差块
int entropy_from_stream_bit(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t* stream, int bit_len = 8);

/*
	使用的huffman编码函数
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
inline void fromch4(T& result, uint8_t* val)	// 将任意类型的数转化为uint8_t的数组
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
inline void toch4(T val, uint8_t* result)	// 将任意类型的数转化为uint8_t的数组
{
	int len = sizeof(T);
	for(int i = 0;i<len;++i)
	{
		result[i] = (uint8_t)(val>>(8*(len-i -1)) & 0x000000ff);
	}
}
#endif
