// codec.cpp : 定义控制台应用程序的入口点。
//
#include "StdAfx.h"

#include "encode.h"
#include "def.h"
#include "io.h"
#include "quantization.h"

int main(int argc, char * argv[])
{
	Frame frame;
	/*
	Block blk(10,10);
	for(int i = 0; i < 10; ++i){
		blk.data[i] = i;
	}
	frame.Yblock.push_back(blk);
	*/
	AVFormat para;
	para.frame_num = 750;
	para.height = 720;
	para.width = 1280;
	strcpy(para.file_name,"D:\\00_2017_mini\\01_测试序列\\dongman.yuv");
	//para.file_name = "C:\\Users\\upperli\\Desktop\\00_2017_mini\\01_测试序列\\dongman.yuv";
	para.video = fopen(para.file_name,"rb");
	para.block_height = 16;
	para.block_width = 16;
	PKT pkt;


	yuv_read(para,frame);


	vector<FrameBufferPool> frame_pool(3,FrameBufferPool(10));
	int errno1 = encode(frame,para,pkt,frame_pool);
		
	/*
	for(auto i =0; i != 8 * 8;++i){
		printf("%d\n",(++pkt.Ulist.begin())->data[i]);
	}
	*/
	int a;
	scanf("%d",&a);
	return 0;
}

