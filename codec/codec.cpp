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
#include <stdio.h>

//#include "huffman.h"

vector<int>  Square_table;

int TABLE[1500][1500];
void settable(){
for(int i=0;i<1500;++i)
{
	for(int j=0;j<1500;++j)
		TABLE[i][j]=i*j;
}
}



#define DEBUG

int main(int argc, char * argv[])
{
	settable();
	//return 0;
	int proc_start,proc_end;
	int start_time,end_time;
	proc_start=clock();
	AVFormat para;

	para.load(argc,argv);
	for(int i=0;i<600;++i)
		Square_table.push_back(i*i);

#ifdef DEBUG
	para.quantizationY=15;
	para.quantizationU=30;
	para.quantizationV=30;
	para.frame_num=50;
	para.tree_mini_block_width=8;
	para.tree_mini_block_height=8;
	para.pattern_num=4;
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
	if(para.out_video==NULL)
	{
		cout<<"你的输出文件处于打开状态！！！！！！！！！！！！！！"<<endl;
		return 0;
	}
	int errno1,errno2;
	for(int i = 0; i < para.frame_num; ++i){
	#ifdef DEBUG
		start_time=clock();
	#endif

		yuv_read(para,frame);	

	#ifdef DEBUG
		end_time=clock();
		std::cout<<"Frame "<< i << endl;
		std::cout<<" yuv readtime is: "<<static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;			
		start_time=clock();
	#endif
		if(para.is_tree)
		errno1 = tree_encode(frame,para,pkt,frame_pool);  //encode
		else
		errno1 = encode(frame,para,pkt,frame_pool);  //encode
			
	#ifdef DEBUG
		end_time=clock();			
		std::cout<< " encode time is: "<<static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<endl;
		start_time=clock();
	#endif
		pkt.stream_write(para);                    //pkt_write
	#ifdef DEBUG
		end_time=clock();
		std::cout<<" pkt write time is: "<<static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;			
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


	for (int i = 0; i < 3; i++){
	/*	for(int j = 0; j < frame_pool[i]->capicity(); ++j){
			(*frame_pool[i])[j].clear();
		}*/
		frame_pool[i]->clear();
		
	}


	for(int i = 0; i < para.frame_num; ++i){
	#ifdef DEBUG
		start_time=clock();
	#endif
		pkt1.stream_read(para);	
		printf("decode Frame %d\n",i);

		if(para.is_tree)
		errno2 = tree_decode(frame1,para,pkt1,frame_pool);
		else
		errno2 = decode(frame1,para,pkt1,frame_pool);
	#ifdef DEBUG
		end_time=clock();			
		std::cout<<"decode Frame "<<i<<endl<< " decode and pkt read time is: "<<static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<endl;
	#endif
		int result = yuv_write(para, frame1);
		
}

	fclose(para.video);	
	fclose(para.out_video);	
	//fclose(para.stream_reader);
	para.stream_reader = nullptr;
	for(int i = 0; i < 3; ++i)
		delete frame_pool[i];
	proc_end=clock();
	std::cout<< "process running time is: "<<static_cast<double>(proc_end-proc_start)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;
	std::cerr<< "process running time is: "<<static_cast<double>(proc_end-proc_start)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;
	#ifdef DEBUG
		//system("pause");
	#endif
	return 0;

}