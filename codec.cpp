// codec.cpp : �������̨Ӧ�ó������ڵ㡣
//


#include "encode.h"
#include "def.h"

int main(int argc, char * argv[])
{
	Frame frame;
	AVFormat para;
	pkt pkt;
	vector<FrameBufferPool> frame_pool(3,FrameBufferPool(10));
	int errno1 = encode(frame,para,pkt,frame_pool);

	return 0;
}

