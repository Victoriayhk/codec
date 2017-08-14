
//#define SIMPLE_SLICE

/**
** �򵥺�黮�֣��������νṹ���֮ǰ�ĸ�ģ����֤������
** ������
*/
#ifdef SIMPLE_SLICE


#include "predict.h"
#include "encode.h"
#include <queue>
#include "quantization.h"
#include <time.h>
#include "Pattern.h"
#include "ResidualBlock.h"
#include "dct.h"
#include <vector>
#include "dctInterface.h"
using namespace std;

const int N=3;  //֡��Ԥ�ⷽ�������� �����λ��������ϵ�AVFormat����������δ����




/*
position.push_back(pos(0,0,h/2-1,w/2-1));
position.push_back(pos(0,w/2,h/2-1,w-1));
position.push_back(pos(h/2,0,h-1,w/2-1));		
position.push_back(pos(h/2,w/2,h-1,w-1));
����8*8�����ĸ�С�������
*/

extern vector<pos> position_Y;
extern vector<pos> position_UV;



/*
** ���ڷ��ؼ������������Ԫ�ص�λ�ú����Ԫ�ص�ֵ
** ����
** p ����ָ��
** N ����Ԫ�صĸ���
** pos ���ص�Ԫ�ص�λ��
** value ���ص�Ԫ�ص����ֵ
** ������
*/
int block_min(int *p,int N,int &pos,int &value)
{
	if(N<1)                   //����Ԫ�ظ�������С��1
		return -1; 
	value=p[0];
	pos=0;
	for(int i=1;i<N;++i)
	{
		if(p[i]<value)
		{
			pos=i;
			value=p[i];
		}
	}
	return 0;
}

/*
** �����λ��ַ�ʽ�¼���16*16��UV8*8�������е�����Ԥ�⣬ʹ��֡�ڻ���֡��
** ������
** block ����ĺ�����ڼ���в�
** residual_block ����Ĳв�ģ�� 
**  whole_frame ֡��Ԥ���õ��Ļ����
** FrameBufferPool ֡��Ԥ���õ��Ļ����
** h,w ���ĳ���
** para  ϵͳ�趨�ı������
** best_value_16 ����16*16�����µ�rdo��Сֵ
** ������
*/


mini_block cal_best_predict_16(Block block,ResidualBlock r_block,int h,int w,BlockBufferPool& whole_frame, FrameBufferPool & frame_pool��AVFormat &para,int &best_value_16)
{
	mini_block res;
	
   /*
   ** ����֡��Ԥ�����Сֵ�Ͷ�Ӧ��ģʽ
   */
   	double 	_16_16_pattern[N]; //֡��Ԥ��ķ���ֵ
	for(int i=0;i<N;++i){
	 _16_16_pattern[i]=Pattern::predict(block,residual_block,0,0,h-1,w-1,block_buffer_pool,i,para); //��������֡��Ԥ���rdoֵ
	}

	int pos16;
	double intra_value16;
	block_min(_16_16_pattern,N,pos16,intra_value16);  //����max��������16*16ģʽ�е���Сֵ
	
	
   /*
   ** ����֡��Ԥ�����Сֵ�Ͷ�Ӧ�˶�����
   */
	double 	inter_value16; 
	InterMV inter_mv;
	inter_16_16=Pattern::inter_predict(block, residual_block, 0, 0, h-1, w-1, frame_pool, inter_mv, para);

	if(inter_value16>0&&inter_value16< intra_value16)           //֡��Ԥ������
	{
		res.predit_type=1;
		res.inter_node=inter_mv;	
		best_value_16=inter_value16;
	}
	else{                                                      //֡��Ԥ������
		res.predit_type=0;
		res.intra_type=pos16;
		best_value_16=intra_value16;
	}

	return res;

}


/*
** �����λ��ַ�ʽ�¼���8*8��UV4*4�������е�����Ԥ�⣬ʹ��֡�ڻ���֡��
** ������
** block ����ĺ�����ڼ���в�
** residual_block ����Ĳв�ģ�� 
** whole_frame ֡��Ԥ���õ��Ļ����
** FrameBufferPool ֡��Ԥ���õ��Ļ����
** para  ϵͳ�趨�ı������
** best_value_8 ����8*8�����µ�rdo��Сֵ
** position ����8*8����4*4��UV����������
** ������
*/

vector<mini_block> cal_best_predict_8(Block block,ResidualBlock r_block,BlockBufferPool& whole_frame, FrameBufferPool & frame_pool��AVFormat &para,int &best_value_8��vector<pos>& position)
{

	vector<mini_block> vec_8_8;
	double min_value8 = 0; //8*8min_value
	
	
	/*
	**֡��Ԥ�����
	*/
	int pos8;
	double _8_8_pattern[4][N];  //�ĸ�С��ÿ��N��֡��Ԥ��ֵ���б���
	double intra_value8;		//intra min value
	

	/*
	**֡��Ԥ�����
	*/
	InterMV inter_mv;
	double inter_value8;   //inter min value


	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < N; ++j)
		{
			_8_8_pattern[i][j] = Pattern::predict(block, residual_block, position[i].left_top_x, position[i].left_top_y,
												  position[i].right_bottom_x, position[i].right_bottom_y, block_buffer_pool, j, para);
		}
		block_min(_8_8_pattern[i], N, pos8, intra_value8); //  ����ĳ��С��֡��Ԥ������ֵ�Ͷ�Ӧ��λ��

		double inter_value8 = Pattern::inter_predict(block, residual_block, position[i].left_top_x, position[i].left_top_y,
													 position[i].right_bottom_x, position[i].right_bottom_y, frame_pool, inter_mv, para); //Сģ�����֡�����Сֵ
		
				
        if (inter_value8 > 0 && inter_value8 < intra_value8)	//���֡��ѹ��Ч������
		{
			mini_block temp;
			temp.predit_type = 1;
			temp.inter_node = inter_mv;
			vec_8_8.push_back(temp);                        //�������Сģ����õ�Ԥ�ⷽʽ

			/*
			** �����Сģ�����������֡��������� Ԥ�� ���� dct �任 
			*/
			Pattern::inter_predict(block, residual_block, position[i].left_top_x, position[i].left_top_y,
								   position[i].right_bottom_x, position[i].right_bottom_y, frame_pool, inter_mv, para);
			dct_trans(residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, h, w);			quantization(position[i].left_top_x, position[i].left_top_y,
						 position[i].right_bottom_x, position[i].right_bottom_y, residual_block, para);
			Reverse_quantization(position[i].left_top_x, position[i].left_top_y,
								 position[i].right_bottom_x, position[i].right_bottom_y, residual_block, para);
			reverse_dct_trans(residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, h, w);
			Pattern::inter_predict_reverse(block, residual_block, position[i].left_top_x, position[i].left_top_y,
										   position[i].right_bottom_x, position[i].right_bottom_y, frame_pool, block_buffer_pool, inter_mv, para);
	
			min_value8 +=inter_value8;
	
		}
		else //���Сģ��֡��ѹ��Ч������
		{
			mini_block temp;
			temp.predit_type = 0;   // intra predit
			temp.intra_type = pos8; //intra predit type
			vec_8_8.push_back(temp);

			/*
			** �����Сģ�����������֡�ڱ�������
			*/
			Pattern::predict(block, residual_block, position[i].left_top_x, position[i].left_top_y,
							 position[i].right_bottom_x, position[i].right_bottom_y, block_buffer_pool, pos8, para);
			dct_trans(residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, h, w);
			quantization(position[i].left_top_x, position[i].left_top_y,
						 position[i].right_bottom_x, position[i].right_bottom_y, residual_block, para);
			Reverse_quantization(position[i].left_top_x, position[i].left_top_y,
								 position[i].right_bottom_x, position[i].right_bottom_y, residual_block, para);
			reverse_dct_trans(residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, h, w);
			Pattern::de_predict(block, residual_block, position[i].left_top_x, position[i].left_top_y,
								position[i].right_bottom_x, position[i].right_bottom_y, block_buffer_pool, pos8, para);

			min_value8 +=intra_value8;
		}
	}
	best_value_8=min_value8��
	return vec_8_8��
}


/*
** �����λ��ַ�ʽ�¼����16*16�����½��б���
** ������
** mini_blk ������洢��Ϣ
** block ����ĺ�����ڼ���в�
** residual_block ����Ĳв�ģ�� 
** whole_frame ֡��Ԥ���õ��Ļ����
** FrameBufferPool ֡��Ԥ���õ��Ļ����
** para  ϵͳ�趨�ı������
** ������
*/

int encode_16_slice(mini_block &mini_blk,Block & block,ResidualBlock & residual_block,AVFormat &para,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool,vector<pos> &position)
{
	residual_block.child_block.push_back(mini_blk);    //��ģ�����Ϣ�洢�ڽڵ���

	if (mini_block.predit_type == 0) //����16*16֡��Ԥ��
	{

		Pattern::predict(block, residual_block, 0, 0, h - 1, w - 1, block_buffer_pool, mini_blk.intra_type, para);
		dct_trans(residual_block, 0, 0, h - 1, w - 1, h, w);
		quantization(0, 0, h - 1, w - 1, residual_block, para);
		temp = residual_block;
		Reverse_quantization(0, 0, h - 1, w - 1, residual_block, para);
		reverse_dct_trans(residual_block, 0, 0, h - 1, w - 1, h, w);
		Pattern::de_predict(block, residual_block, 0, 0, h - 1, w - 1, block_buffer_pool, mini_blk.intra_type, para);
		residual_block = temp; //  ying gai ding yi kao bei gou zao han shu
	}
	else 							// ����16*16��֡��Ԥ��
	{

		Pattern::inter_predict(block, residual_block, 0, 0, h - 1, w - 1, frame_pool, mini_blk.inter_node, para);
		dct_trans(residual_block, 0, 0, h - 1, w - 1, h, w);
		quantization(0, 0, h - 1, w - 1, residual_block, para);
		temp = residual_block;
		Reverse_quantization(0, 0, h - 1, w - 1, residual_block, para);
		reverse_dct_trans(residual_block, 0, 0, h - 1, w - 1, h, w);
		Pattern::inter_predict_reverse(block, residual_block, 0, 0, h - 1, w - 1, frame_pool, block_buffer_pool, mini_blk.inter_node, para);
		residual_block = temp; //  ���±���֮���ֵ
	}
	return 0;
}



/*
** �����λ��ַ�ʽ�¼����8*8�����½��б���
** ������
** vec_8_8 �����4�����洢��Ϣ
** block ����ĺ�����ڼ���в�
** residual_block ����Ĳв�ģ�� 
** whole_frame ֡��Ԥ���õ��Ļ����
** FrameBufferPool ֡��Ԥ���õ��Ļ����
** para  ϵͳ�趨�ı������
** ������
*/

int encode_8_slice(vector<mini_block> &vec_8_8,Block & block,ResidualBlock & residual_block,AVFormat &para,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool��vector<pos> &position)
{
	ResidualBlock temp;   //����ģ�黮��
	residual_block.child_block = vec_8_8;
	int i_offset = (residual_block.block_id / para.block_num_per_row) * h; // ��ǰblock����ʼ��������Frame����
	int j_offset = (residual_block.block_id % para.block_num_per_row) * w; // ��ǰblock����ʼ��������Frame����
	for (int i = 0; i < 4; ++i)
	{
		if (vec_8_8[i].predit_type == 0) //��8*8��֡�ڱ����
		{

			Pattern::predict(block, residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, block_buffer_pool, vec_8_8[i].intra_type, para);
			dct_trans(residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, h, w);
			quantization(position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, residual_block, para);
		
			/*
			** ������Ԥ�⣬dct���������֮����Ҫ�����м�Ĳв�����ݣ���ֹ������任֮�����ݷ���ת��
			*/			
			if (i == 0)                           
				temp = residual_block;
			else
			{
				for (int m = position[i].left_top_x; m <= position[i].right_bottom_x && m + i_offset < para.height; m++)
				{
					for (int n = position[i].left_top_y; n <= position[i].right_bottom_y && n + j_offset < para.width; n++)
					{
						temp.data[m * w + n] = residual_block.data[m * w + n];
					}
				}
			}

			Reverse_quantization(position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, residual_block, para);
			reverse_dct_trans(residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, h, w);
			Pattern::de_predict(block, residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, block_buffer_pool, vec_8_8[i].intra_type, para);
		}
		else //��8*8��֡������
		{
			Pattern::inter_predict(block, residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, frame_pool,vec_8_8[i].inter_node, para);
			dct_trans(residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, h, w);
			quantization(position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, residual_block, para);
			if (i == 0)
				temp = residual_block;
			else
			{
				for (int m = position[i].left_top_x; m <= position[i].right_bottom_x && m + i_offset < para.height; m++)
				{
					for (int n = position[i].left_top_y; n <= position[i].right_bottom_y && n + j_offset < para.width; n++)
					{
						temp.data[m * w + n] = residual_block.data[m * w + n];
					}
				}
			}
			Reverse_quantization(position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, residual_block, para);
			reverse_dct_trans(residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, h, w);
			Pattern::inter_predict_reverse(block, residual_block, position[i].left_top_x, position[i].left_top_y, position[i].right_bottom_x, position[i].right_bottom_y, frame_pool, block_buffer_pool,vec_8_8[i].inter_node, para);
		}
	}
	residual_block = temp;
	return 0;
}


/*
** �����λ��ַ�ʽ�µı���ģ��
** ������
** block ����ĺ�����ڼ���в�
** residual_block ����Ĳв�ģ��
** para  ϵͳ�趨�ı������
** block_buffer_pool ֡��Ԥ���õ��Ļ����
** FrameBufferPool ֡��Ԥ���õ��Ļ����
** ������
*/

inline int encode_one_block(Block & block,ResidualBlock & residual_block,AVFormat &para,BlockBufferPool & block_buffer_pool, FrameBufferPool & frame_pool){

	Block save_block=block;  				//����block ��ֹ�ڽ��з�Ԥ���ʱ���blockֵ���иı�
	residual_block.block_id = block.block_id;
	residual_block.block_type = block.block_type;
	int h,w;
	block.getBlockSize(para,h,w);           //��ȡblock�ĳ������

	
	/*
	** ˼·�����ȼ���16*16������֡��Ԥ���֡������Ԥ��ģʽ����Сֵ������¼16*16���ղ���֡�ڻ���֡��Ԥ�⣬��������Ӧ����
	*/

	mini_block mini_blk_16; //����16*16�����µ�����ģʽѡ��֡�ڻ�֡�䡣
	int  best_value_16;     //����16*16�����µ�����ֵ
	mini_blk_16=cal_best_predict_16(block,r_block,h,w,whole_frame,para��best_value_16);

	/*
	**  8*8�����µ�����ģʽ����
	*/
	vector<mini_block> mini_blk_8;  //����8*8�����µ�ÿ��С�������ģʽѡ��֡�ڻ�֡�䡣
	int  best_value_8;     			//����8*8�����µ�����ֵ

	if(block.block_type==Block::Y)
		mini_blk_8=cal_best_predict_8(block,r_block,whole_frame,para��best_value_8,position_Y);
	else
		mini_blk_8=cal_best_predict_8(block,r_block,whole_frame,para��best_value_8,position_UV);
	

	if(best_value_8>best_value_16){    //���16*16��СֵС��8*8��Сֵ������16*16�Ļ���
		residual_block.type_slice=0;   //���õ�һ�ֻ��ַ�ʽ
		if(block.block_type==Block::Y)
			encode_16_slice(mini_blk_16,block,residual_block,para,block_buffer_pool,frame_pool,position_Y);
		else
			encode_16_slice(mini_blk_16,block,residual_block,para,block_buffer_pool,frame_pool,position_UV);
	}
	else      //����8*8Ԥ��
	{
		residual_block.type_slice=1;   //���õ�һ�ֻ��ַ�ʽ
		if(block.block_type==Block::Y)
			encode_8_slice(mini_blk_8,block,residual_block,para,block_buffer_pool,frame_pool,position_Y);
		else
			encode_8_slice(mini_blk_8,block,residual_block,para,block_buffer_pool,frame_pool,position_UV);
	}	
	return 0;
}

inline int encode_one_component(vector<Block> & blocks, std::vector<ResidualBlock> & residual_blocks,AVFormat &para,FrameBufferPool & frame_pool){

	BlockBufferPool & decode_buffer = frame_pool.new_back();
	int block_height,block_width;
	blocks[0].getBlockSize(para,block_height,block_width);

	for(int i=0;i<blocks.size();++i){
		residual_blocks[i].clear();
		encode_one_block(blocks[i],residual_blocks[i],para,decode_buffer,frame_pool);		
	}
	//frame_pool.add_frame_to_pool(decode_buffer);
	return 0;
}
int encode(Frame &frame,AVFormat &para,PKT &pkt,vector<FrameBufferPool*>  &frame_pool){
	encode_one_component(frame.Yblock,pkt.Ylist,para,*frame_pool[0]);
	encode_one_component(frame.Ublock,pkt.Ulist,para,*frame_pool[1]);
	encode_one_component(frame.Vblock,pkt.Vlist,para,*frame_pool[2]);
	return 0;
}

#endif