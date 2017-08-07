// codec.cpp : 定义控制台应用程序的入口点。
//


#include "tree_encode.h"
#include "def.h"
#include "io.h"
#include "quantization.h"
#include "decode.h"
#include <time.h>
#include <omp.h>
#include <iostream>

int main(int argc, char * argv[])
{
	Frame frame;

	AVFormat para;
	para.frame_num = 750;
	para.height = 720;
	para.width = 1280;
	para.quantizationY = 1.1;
	para.quantizationU = 4;
	para.quantizationV = 4;
	
	strcpy(para.file_name,"D:\\00_2017_mini\\01_测试序列\\youjiyingxiong.yuv");
	//para.file_name = "C:\\Users\\upperli\\Desktop\\00_2017_mini\\01_测试序列\\dongman.yuv";
	para.video = fopen(para.file_name,"rb");
	para.block_height = 16;
	para.block_width = 16;
	para.block_num_per_row =  ceil(1.0 * para.width / para.block_width);
	para.block_num = ceil(1.0 * para.height /  para.block_height) *   ceil(1.0 * para.width / para.block_width);
	char out_file_name[100];
	strcpy(out_file_name,"D:\\2.yuv");
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

	for(int i = 0; i < 50; ++i){



	yuv_read(para,frame);


	//int start_time=clock();
	double start_time = omp_get_wtime( );
	int errno1 = tree_encode(frame,para,pkt,frame_pool);
	//int end_time=clock();
	double end_time = omp_get_wtime( );
	std::cout<< "Running time is: "<<static_cast<double>(end_time-start_time)<<"s"<<std::endl;	

	for(auto i =0; i != 8;++i){
		printf("%d ",pkt.Ylist[0].data[i]);
	}
	printf("\n");
	pkt.to_stream(buff);

	


	pkt1.from_stream(buff,para);

	//for(auto i =0; i != 8 * 8;++i){
	//	printf("%d\n",pkt1.Ylist[0].data[i]);
	//}
	printf("=================================================================\n");
;
	int errno2 = decode(frame1,para,pkt1,frame_pool);


	//for(auto i =0; i != 8  * 8;++i){
	//	printf("%d\n",frame1.Yblock[0].data[i]);
	//}

	
	int result = yuv_write(para, frame1);

	}
	fclose(para.out_video);	
	int a;
	scanf("%d",&a);

	return 0;
}

