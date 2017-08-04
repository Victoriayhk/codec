// codec.cpp : 定义控制台应用程序的入口点。
//


#include "encode.h"
#include "def.h"
#include "io.h"
#include "quantization.h"
#include "decode.h"

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
	strcpy(para.file_name,"D:\\00_2017_mini\\01_测试序列\\dongman.yuv");
	//para.file_name = "C:\\Users\\upperli\\Desktop\\00_2017_mini\\01_测试序列\\dongman.yuv";
	para.video = fopen(para.file_name,"rb");
	para.block_height = 16;
	para.block_width = 16;
	char out_file_name[100];
	strcpy(out_file_name,"D:\\1.yuv");
	para.out_video = fopen(out_file_name,"wb");


	PKT pkt;


	yuv_read(para,frame);
	/*
	frame.Yblock[0].print(para);
	printf("\n");
	
	frame.Ublock[0].print(para);
	printf("\n");
	frame.Vblock[0].print(para);
	
	printf("\n");
	*/
	//printf("a\n");
	
	vector<FrameBufferPool> frame_pool(3,FrameBufferPool(10));



	int errno1 = encode(frame,para,pkt,frame_pool);
		
	unsigned char * buff = new unsigned char[1000000000];
	pkt.to_stream(buff);

	PKT pkt1;
	pkt1.from_stream(buff,para);

	for(auto i =0; i != 8 * 8;++i){
		printf("%d\n",pkt1.Ylist[0].data[i]);
	}
	printf("=================================================================\n");
	Frame frame1;
	int errno2 = decode(frame1,para,pkt1,frame_pool);


	for(auto i =0; i != 8 * 8;++i){
		printf("%d\n",frame1.Yblock[0].data[i]);
	}

	
	int result = yuv_write(para, frame1);

	fclose(para.out_video);	
	int a;
	scanf("%d",&a);

	return 0;
}

