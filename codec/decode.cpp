
//define NO_TREE
#ifdef NO_TREE


#include "decode.h"
#include "quantization.h"
#include "Pattern.h"
#include "encode.h"
#include "ResidualBlock.h"


extern vector<pos> position_Y;
extern vector<pos> position_UV;
/*  
	position中包含8*8划分的四个小块的左上角和右下角的坐标
	position.push_back(pos(0,0,h/2-1,w/2-1));
	position.push_back(pos(0,w/2,h/2-1,w-1));
	position.push_back(pos(h/2,0,h-1,w/2-1));
	position.push_back(pos(h/2,w/2,h-1,w-1));
*/



/*
** 非树形划分的解码模块，采用简单划分，只考虑划分成4个8*8或者16*16 后用树形结构代替
** 董辰辰
** 参数：
** block 解码恢复的宏块 
** residual_block 残差宏块
** para 定义了一些编码参数 
** block_buffer_pool 帧内预测的池
** frame_pool  帧间预测的池
*/ 

inline int decode_one_block(Block & block,ResidualBlock & residual_block,AVFormat &para,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool){
	int h,w;
	residual_block.getBlockSize(para,h,w);     //获取宏块的宽高
	vector<pos> position;
	if(residual_block.block_type==residual_block::Y)
		position=position_Y;
	else
		position=position_UV;
	block.block_id = residual_block.block_id;
	block.block_type = residual_block.block_type;
	if(residual_block.type_slice==0)  			//划分方式为16*16
	{
		/*
		** 执行反量化，反dct变换，反预测就可以完成解码
		*/
		Reverse_quantization(0 ,0 ,h-1 ,w-1 , residual_block , para);
		reverse_dct_trans(residual_block,0 ,0 ,h-1 ,w-1 ,h,w);
		Pattern::de_predict(block,residual_block,0,0,h-1,w-1,block_buffer_pool,residual_block.node[0],para);
	}
	else if(residual_block.type_slice==1)  //划分方式为8*8
	{
		vector<mini_block> _8_8_type=residual_block.child_block;	
		for(int i=0;i<4;++i){
			if(_8_8_type[i].predit_type==0)               //小块使用帧内预测
			{
				Reverse_quantization(position[i].left_top_x,position[i].left_top_y,
									position[i].right_bottom_x,position[i].right_bottom_y,residual_block,para);
				reverse_dct_trans(residual_block,position[i].left_top_x,position[i].left_top_y,position[i].right_bottom_x,position[i].right_bottom_y,h,w);
				Pattern::de_predict(block,residual_block,position[i].left_top_x,position[i].left_top_y,position[i].right_bottom_x,position[i].right_bottom_y,block_buffer_pool,_8_8_type[i].intra_type,para);
			}
			else                                          //小块使用帧间预测
			{
				Reverse_quantization(position[i].left_top_x,position[i].left_top_y,position[i].right_bottom_x,position[i].right_bottom_y , residual_block , para);	
				reverse_dct_trans(residual_block,position[i].left_top_x,position[i].left_top_y,position[i].right_bottom_x,position[i].right_bottom_y,h,w);
				Pattern::inter_predict_reverse(block, residual_block, position[i].left_top_x,position[i].left_top_y,position[i].right_bottom_x,position[i].right_bottom_y,frame_pool,block_buffer_pool,_8_8_type[i].inter_node,para);
			}
		}
	}
	else
	{}
	return 0;
}

inline int decode_one_component(vector<Block> & blocks, std::vector<ResidualBlock> & residual_blocks,AVFormat &para,FrameBufferPool & frame_pool){

	BlockBufferPool  & decode_buffer = frame_pool.new_back();
	int h,w;
	residual_blocks[0].getBlockSize(para,h,w);
	for(int i=0;i< residual_blocks.size();++i){
		decode_one_block(blocks[i],residual_blocks[i],para,decode_buffer,frame_pool);
	}
	//frame_pool.add_frame_to_pool( decode_buffer );
	return 0;

}
int decode(Frame &frame,AVFormat &para,PKT &pkt,vector<FrameBufferPool*>  &frame_pool){
	decode_one_component(frame.Yblock,pkt.Ylist,para,*frame_pool[0]);
	decode_one_component(frame.Ublock,pkt.Ulist,para,*frame_pool[1]);
	decode_one_component(frame.Vblock,pkt.Vlist,para,*frame_pool[2]);
	return 0;
}


#endif