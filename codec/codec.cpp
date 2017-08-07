// codec.cpp : 定义控制台应用程序的入口点。
//


#include "encode.h"
#include "def.h"
#include "io.h"
#include "quantization.h"
#include "decode.h"
#include <time.h>
#include <iostream>

#include "huffman.h"

int main(int argc, char * argv[])
{
	//head_test();
	//return 0;

	Frame frame;

	AVFormat para;
	para.frame_num = 750;
	para.height = 720;
	para.width = 1280;
	para.quantizationY = 1;
	para.quantizationU = 1;
	para.quantizationV = 1;

	strcpy(para.file_name,"D:\\00_2017_mini\\01_测试序列\\youjiyingxiong.yuv");
	//para.file_name = "C:\\Users\\upperli\\Desktop\\00_2017_mini\\01_测试序列\\dongman.yuv";
	para.video = fopen(para.file_name,"rb");
	para.block_height = 16;
	para.block_width = 16;
	para.block_num = ceil(1.0 * para.height /  para.block_height) *   ceil(1.0 * para.width / para.block_width);
	char out_file_name[100];
	strcpy(out_file_name,"D:\\1234.yuv");
	strcpy(para.stream_file_name,"D:\\1234");
	para.out_video = fopen(out_file_name,"wb");


	unsigned char * buff = new unsigned char[1000000000];
	Frame frame1;
	PKT pkt;
	PKT pkt1;
	pkt.init(para);
	frame.init(para);
	frame1.init(para);
	pkt1.init(para);



	vector<FrameBufferPool> frame_pool(3,FrameBufferPool(10));

	for(int i = 0; i < 5; ++i){



		yuv_read(para,frame);


		int start_time=clock();

		int errno1 = encode(frame,para,pkt,frame_pool);

		////fclose(para.stream_writer);
		////para.stream_writer = nullptr;

		int end_time=clock();

		std::cout<< "Running time is: "<<static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;	

		//for(auto i =0; i != 8;++i){
		//	printf("%d ",pkt.Ylist[0].data[i]);
		//}
		//printf("\n");
		//pkt.to_stream(buff);

		//pkt1.stream_read(para);		//从文件流中读取pkt信息


		//////pkt1.from_stream(buff,para);

		//////for(auto i =0; i != 8 * 8;++i){
		//////	printf("%d\n",pkt1.Ylist[0].data[i]);
		//////}
		//printf("=================================================================\n");
		////;
		//int errno2 = decode(frame1,para,pkt1,frame_pool);

		////if(para.stream_writer != nullptr)
		////{
		////	fclose(para.stream_writer);
		////	para.stream_writer = nullptr;
		////}

		////for(auto i =0; i != 8  * 8;++i){
		////	printf("%d\n",frame1.Yblock[0].data[i]);
		////}


		//int result = yuv_write(para, frame1);

	}

	fclose(para.stream_writer);
	para.stream_writer = nullptr;

	for(int i = 0; i < 5; ++i){
		int start_time=clock();
		pkt1.stream_read(para);
		int errno2 = decode(frame1,para,pkt1,frame_pool);
		int result = yuv_write(para, frame1);
		int end_time=clock();
		std::cout<< "Running time is: "<<static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<std::endl;	
	}


	fclose(para.stream_reader);
	para.stream_writer = nullptr;

	fclose(para.out_video);	
	int a;
	scanf("%d",&a);

	return 0;
}

