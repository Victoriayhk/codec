// codec.cpp : 定义控制台应用程序的入口点。
//


#include "encode.h"
#include "def.h"
#include "io.h"

int main(int argc, char * argv[])
{

	Frame frame;
	Block blk(10,10);
	for(int i = 0; i < 10; ++i){
		blk.data[i] = i;
	}
	frame.Yblock.push_back(blk);

	AVFormat para;
	para.block_height = 64;
	para.block_width = 64;
	PKT pkt;
	
	vector<FrameBufferPool> frame_pool(3,FrameBufferPool(10));
	int errno1 = encode(frame,para,pkt,frame_pool);
		
	
	for(auto i =0; i != 10;++i){
		printf("%d\n",pkt.Ylist.front().data[i]);
	}
	int a;
	scanf("%d",&a);
	return 0;
}

