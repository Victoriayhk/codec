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









		int errno2 = decode(frame1,para,pkt1,frame_pool);
		printf("=================================================================\n");


		//int errno2 = decode(frame1,para,pkt1,frame_pool);


	}
	fclose(para.out_video);	
	int a;
	scanf("%d",&a);

	return 0;
}

