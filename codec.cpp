// codec.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "encode.h"
#include "def.h"

int _tmain(int argc, _TCHAR* argv[])
{
	Frame frame();
	AVFormat para();
	pkt pkt();
	vector<FrameBufferPool> frame_pool(3);
	int errno1 = encode(frame,para,pkt,frame_pool);

	return 0;
}

