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

//#define DEBUG
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
	para.frame_num=1;
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
	#ifdef DEBUG
		start_time=clock();
	#endif

		yuv_read(para,frame);	

	#ifdef DEBUG
		end_time=clock();
		std::cout<<" yuv readtime is: "<<static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;			
	#endif
	#ifdef DEBUG
		start_time=clock();
	#endif

		int errno1 = encode(frame,para,pkt,frame_pool);

	#ifdef DEBUG
		end_time=clock();			
		std::cout<<"encode Frame "<<i<<endl<< " encode time is: "<<static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<endl;
	#endif
	#ifdef DEBUG
		start_time=clock();
	#endif
		pkt.stream_write(para);
	#ifdef DEBUG
		end_time=clock();
		std::cout<<"pkt write time is: "<<static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;			
		for(auto i =0; i != 8;++i){
			printf("%d ",pkt.Ylist[0].data[i]);
		}
		printf("\n");
	#endif
	}
#ifdef DEBUG
	printf("=================================================================\n");
#endif
	fclose(para.stream_writer);
		para.stream_writer = nullptr;
	for(int i = 0; i < para.frame_num; ++i){
	#ifdef DEBUG
		start_time=clock();
	#endif
		pkt1.stream_read(para);	
		printf("decode Frame %d\n",i);
		int errno2 = decode(frame1,para,pkt1,frame_pool);
	#ifdef DEBUG
		end_time=clock();			
		std::cout<<"decode Frame "<<i<<endl<< " decode and pkt read time is: "<<static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<endl;
	#endif
		int result = yuv_write(para, frame1);
}

	fclose(para.video);	
	fclose(para.out_video);	
	fclose(para.stream_reader);
	para.stream_reader = nullptr;
	for(int i = 0; i < 3; ++i)
		delete frame_pool[i];
	proc_end=clock();
	std::cout<< "process running time is: "<<static_cast<double>(proc_end-proc_start)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;
	#ifdef DEBUG
		system("pause");
	#endif
	return 0;

}