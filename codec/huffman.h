/*
 *  huffman_coder - Encode/Decode files using Huffman encoding.
 *  http://huffman.sourceforge.net
 *  Copyright (C) 2003  Douglas Ryan Richardson
 */

#ifndef HUFFMAN_HUFFMAN_H
#define HUFFMAN_HUFFMAN_H

#include <stdio.h>
#include <stdint.h>
#include "ResidualBlock.h"

#define BLOCK_NUM 1

#define positive 0
#define negative 1

void entropy_test();

int entropy_coding_slice(ResidualBlock* rBlock ,int block_len, AVFormat& para, uint8_t **stream,unsigned int* len);
int entropy_decode_slice(ResidualBlock* rBlock,int block_len , AVFormat& para, uint8_t *stream, unsigned int buff_length);


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
int entropy_coding_block(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t **stream, unsigned int *buff_length);

/*
* 将流逆熵编码并将数据保存在输入的残差block中
* 输入：
* f_x f_y l_x l_y	残差的左上角及右下角坐标
* rBlock			残差块
* para				配置文件
* stream			返回编码后的流
* buff_length		流长度
*/
int entropy_decode_block(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t *stream, unsigned int buff_length);

int entropy_to_stream(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t** stream);
int entropy_from_stream(int f_x, int f_y, int l_x, int l_y, ResidualBlock& rBlock, AVFormat& para, uint8_t* stream);

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

#endif
