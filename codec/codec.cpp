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

//#include "huffman.h"

int main(int argc, char * argv[])
{

	AVFormat para;
	para.load(argc,argv);

	Frame frame;
	
	
	//unsigned char * buff = new unsigned char[1000000000];
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

	para.frame_num = 1;
	for(int i = 0; i < para.frame_num; ++i){

		int start_time=clock();
		yuv_read(para,frame);
		
		int errno1 = encode(frame,para,pkt,frame_pool);

		
		//pkt.stream_write(para);
		//int end_time=clock();

		int start_time_write=clock();
		pkt.stream_write(para);
		int end_time_write=clock();
		//std::cout<<" Stream wirte Running time is: "<<static_cast<double>(end_time_write-start_time_write)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;	
		
		int end_time=clock();
		std::cout<<"decode Frame "<<i<<endl<< " Running time is: "<<static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<endl<<"wirte Running time is:"<<static_cast<double>(end_time_write-start_time_write)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;	

		
		for(auto i =0; i != 8;++i){
			printf("%d ",pkt.Ylist[0].data[i]);
		}
		printf("\n");
		printf("=================================================================\n");
	}	
	fclose(para.stream_writer);
		para.stream_writer = nullptr;
	for(int i = 0; i < para.frame_num; ++i){
		int start_time=clock();
		pkt1.stream_read(para);		
		int errno2 = decode(frame1,para,pkt1,frame_pool);

		int result = yuv_write(para, frame1);
		int end_time=clock();
		std::cout<< "Running time is: "<<static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;	
	}

	

	fclose(para.video);	

	fclose(para.out_video);	

	fclose(para.stream_reader);
	para.stream_reader = nullptr;


	for(int i = 0; i < 3; ++i)
		delete frame_pool[i];

	return 0;

}