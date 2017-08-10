// codec.cpp : 定义控制台应用程序的入口点。
//

#include "encode.h"
#include "tree_encode.h"
#include "tree_decode.h"
#include "def.h"
#include "io.h"
#include "quantization.h"
#include "decode.h"
#include <time.h>
#include "Pattern.h"
#include <iostream>
#include "huffman.h"
#include <fstream>
#include <string>
#include "decode_buffer_pool.h"

#define DEBUG

int main(int argc, char * argv[])
{
	int proc_start,proc_end;
	int start_time,end_time;
	proc_start=clock();
	AVFormat para;
	para.load(argc,argv);

#ifdef DEBUG
	para.quantizationY=10;
    para.quantizationU=15;
	para.quantizationV=15;
	para.frame_num=2;
#endif

	Frame frame;
	Frame frame1;
	PKT pkt;
	PKT pkt1;
	pkt.init(para);
	frame.init(para);
	frame1.init(para);
	pkt1.init(para);
	vector<FrameBufferPool *> frame_pool(3);
	frame_pool[0] = new FrameBufferPool(10,BlockBufferPool(para.height,para.width));
	frame_pool[1] = new FrameBufferPool(10,BlockBufferPool(para.height/2,para.width/2));
	frame_pool[2] = new FrameBufferPool(10,BlockBufferPool(para.height/2,para.width/2));
	para.video = fopen(para.file_name,"rb");
	para.out_video = fopen(para.out_file_name,"wb");	
	for(int i = 0; i < para.frame_num; ++i){
		yuv_read(para,frame);	
		int errno1 = encode(frame,para,pkt,frame_pool);  //encode
		int errno2 = decode(frame1,para,pkt1,frame_pool);
		int result = yuv_write(para, frame1);			
		std::cout<<"Frame "<<i<<endl;	
		for(auto i =0; i != 8;++i){
			printf("%d ",pkt.Ylist[0].data[i]);
		}
		printf("\n");

	}
	fclose(para.video);	
	fclose(para.out_video);	
	for(int i = 0; i < 3; ++i)
		delete frame_pool[i];
	system("pause");
	return 0;

}