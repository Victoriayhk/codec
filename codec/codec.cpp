// codec.cpp : �������̨Ӧ�ó������ڵ㡣
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
	para.quantizationY = 2;
	para.quantizationU = 4;
	para.quantizationV = 4;
	strcpy(para.file_name,"D:\\00_2017_mini\\01_��������\\dongman.yuv");
	//para.file_name = "C:\\Users\\upperli\\Desktop\\00_2017_mini\\01_��������\\dongman.yuv";
	para.video = fopen(para.file_name,"rb");
	para.block_height = 16;
	para.block_width = 16;
	PKT pkt;


	yuv_read(para,frame);


	vector<FrameBufferPool> frame_pool(3,FrameBufferPool(10));



	int errno1 = encode(frame,para,pkt,frame_pool);
		
	for(auto i =0; i != 8 * 8;++i){
	//	printf("%d\n",pkt.Ylist[0].data[i]);
	}
	printf("=================================================================\n");
	Frame frame1;
	int errno2 = decode(frame1,para,pkt,frame_pool);


	for(auto i =0; i != 8 * 8;++i){
		//printf("%d\n",frame1.Yblock[0].data[i]);
	}
	
	int a;
	scanf("%d",&a);
	return 0;
}

