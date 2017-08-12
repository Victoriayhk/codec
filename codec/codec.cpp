
#define ENCODE
//#define DECODE

#ifdef ENCODE
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


	Frame frame;
	PKT pkt;
	pkt.init(para);
	frame.init(para);


	vector<FrameBufferPool *> frame_pool(3);
	frame_pool[0] = new FrameBufferPool(10,BlockBufferPool(para.height,para.width));
	frame_pool[1] = new FrameBufferPool(10,BlockBufferPool(para.height/2,para.width/2));
	frame_pool[2] = new FrameBufferPool(10,BlockBufferPool(para.height/2,para.width/2));
	para.video = fopen(para.file_name,"rb");

	int errno1;
	for(int i = 0; i < para.frame_num; ++i){
//		cout<<i<<endl;
		yuv_read(para,frame);	
		errno1 = tree_encode(frame,para,pkt,frame_pool);  //encode
		pkt.stream_write(para);                    //pkt_write
	}

	fclose(para.stream_writer);
	para.stream_writer = nullptr;

	fclose(para.video);	
	for(int i = 0; i < 3; ++i)
		delete frame_pool[i];

	proc_end=clock();

	std::cout<< "encode running time is: "<<static_cast<double>(proc_end-proc_start)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;

	return 0;

}


#endif


#ifdef DECODE
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



int main(int argc, char * argv[])
{
	settable();
	int proc_start,proc_end;
	int start_time,end_time;
	proc_start=clock();
	AVFormat para;

	para.load(argc,argv);
	for(int i=0;i<600;++i)
		Square_table.push_back(i*i);

	Frame frame1;
	PKT pkt1;
	frame1.init(para);
	pkt1.init(para);
	vector<FrameBufferPool *> frame_pool(3);
	frame_pool[0] = new FrameBufferPool(10,BlockBufferPool(para.height,para.width));
	frame_pool[1] = new FrameBufferPool(10,BlockBufferPool(para.height/2,para.width/2));
	frame_pool[2] = new FrameBufferPool(10,BlockBufferPool(para.height/2,para.width/2));

	para.out_video = fopen(para.out_file_name,"wb");
	if(para.out_video==NULL)
	{
		cout<<"你的输出文件处于打开状态！！！！！！！！！！！！！！"<<endl;
		return 0;
	}
	int errno2;

	for(int i = 0; i < para.frame_num; ++i){
		pkt1.stream_read(para);	
		errno2 = tree_decode(frame1,para,pkt1,frame_pool);
		int result = yuv_write(para, frame1);		
}

	fclose(para.out_video);	
	fclose(para.stream_reader);
	para.stream_reader = nullptr;
	for(int i = 0; i < 3; ++i)
		delete frame_pool[i];

	proc_end=clock();
	std::cout<< "decode running time is: "<<static_cast<double>(proc_end-proc_start)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;

	return 0;

}




#endif
