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
#include "huffman.h"
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
		int start_time=clock();
		int errno1 = encode(frame,para,pkt,frame_pool);
		pkt.stream_write(para);
		int end_time=clock();
		std::cout<< "Running time is: "<<static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;	
		for(auto i =0; i != 8;++i){
			printf("%d ",pkt.Ylist[0].data[i]);
		}
		printf("\n");

	}
printf("=================================================================\n");
fclose(para.stream_writer);
para.stream_writer = nullptr;
for(int i = 0; i < 50; ++i){
		int start_time=clock();
		pkt1.stream_read(para);		
		int errno2 = decode(frame1,para,pkt1,frame_pool);
		int result = yuv_write(para, frame1);
		int end_time=clock();
		std::cout<< "Running time is: "<<static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;	
		printf("decode Frame %d\n",i);
}
fclose(para.out_video);	
fclose(para.video);	
fclose(para.stream_reader);
para.stream_reader = nullptr;
//	int a;
//	scanf("%d",&a);
	return 0;

}