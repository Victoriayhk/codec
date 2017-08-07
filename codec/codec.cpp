// codec.cpp : 定义控制台应用程序的入口点。
//

#include "encode.h"
#include "tree_encode.h"
#include "def.h"
#include "io.h"
#include "quantization.h"
#include "decode.h"
#include <time.h>
#include "Pattern.h"
#include <iostream>
#include <fstream>
#include <string>
//#include "huffman.h"

int main(int argc, char * argv[])
{
	
	AVFormat para;
	para.load(argc,argv);

	Frame frame;
	para.video = fopen(para.file_name,"rb");
	para.out_video = fopen(para.out_file_name,"wb");	
	unsigned char * buff = new unsigned char[1000000000];
	Frame frame1;
	PKT pkt;
	PKT pkt1;
	pkt.init(para);
	frame.init(para);
	frame1.init(para);
	pkt1.init(para);

	vector<FrameBufferPool> frame_pool(3,FrameBufferPool(10));

	for(int i = 0; i < 50; ++i){
		yuv_read(para,frame);
		int errno1 = encode(frame,para,pkt,frame_pool);
		int errno2 = decode(frame1,para,pkt,frame_pool);
		yuv_write(para,frame1);
		printf("Encode Frame %d\n",i);
	}
	fclose(para.out_video);	
	int a;
	scanf("%d",&a);

	return 0;
}

