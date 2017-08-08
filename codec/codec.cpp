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
	//unsigned char * buff = new unsigned char[1000000000];
	Frame frame1;
	PKT pkt;
	PKT pkt1;
	pkt.init(para);
	frame.init(para);
	frame1.init(para);
	pkt1.init(para);

	vector<FrameBufferPool> frame_pool(3,FrameBufferPool(10));
	//para.frame_num = 100;
	for(int i = 0; i < para.frame_num; ++i){
		yuv_read(para,frame);
		int start_time=clock();
		int errno1 = encode(frame,para,pkt,frame_pool);

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

	}
	printf("=================================================================\n");
	fclose(para.stream_writer);
	para.stream_writer = nullptr;
	for(int i = 0; i < para.frame_num; ++i){
		int start_time=clock();

		int start_time_read=clock();
		pkt1.stream_read(para);
		int end_time_read=clock();
		//std::cout<< "Read Running time is: "<<static_cast<double>(end_time_read-start_time_read)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;	
		int errno2 = decode(frame1,para,pkt1,frame_pool);
		int result = yuv_write(para, frame1);
		int end_time=clock();
		printf("decode Frame %d\n",i);
		std::cout<<i<< "Running time is: "<<static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;	
		std::cout<< "Read Running time is: "<<static_cast<double>(end_time_read-start_time_read)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;
		//printf("decode Frame %d\n",i);
	}
	fclose(para.out_video);	
	fclose(para.video);	
	fclose(para.stream_reader);
	para.stream_reader = nullptr;
	//	int a;
	//	scanf("%d",&a);
	return 0;

}