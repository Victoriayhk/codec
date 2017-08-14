#include "Pattern.h"
#include <math.h>

// Pattern::FUN Pattern::getPattern(int pattern){
//	return (FUN)patterns[pattern];
//}
//int Pattern::get_pattern_number(){
//	return pattern_number; 
//}

extern vector<int>  Square_table;
extern int TABLE[1500][1500];
#define IN_AREA(i, j, h, w) (0 <= i && i < h && 0 <= j && j <= w) 

/*
**	֡��Ԥ��
**  ������
**  block�������Ԥ���  r_block�����صĲв�� 
**	start_r ... end_c :С�������ڴ�����������ҵ�����
**  whole_frame :���ο���֡�ڳ�
**  pattern_type��֡��Ԥ���ģʽ��Χ��0~3
**	para��ѹ���Ĳ����������������С�ȡ�
**	i_offset,j_offset ÿһ֡�����ֳ����ɺ����д洢�������������ʾ������λ��ƫ��
**	������
*/

int Pattern::predict(Block& block,ResidualBlock & r_block,int start_r,int start_c,int end_r,int end_c, BlockBufferPool& whole_frame,int pattern_type, AVFormat &para,int i_offset,int j_offset){
	int block_w, block_h;
	r_block.getBlockSize(para, block_w, block_h);							// ��ǰblock�Ŀ�͸�

	/**
	** �������ƫ�ƣ�Ϊ�˱����ظ����㣬��������λ�õĴ�������ͨ���������д���
	*/
	//int i_offset = TABLE[r_block.block_id / para.block_num_per_row][block_h];// ��ǰblock����ʼ��������Frame����
	//int j_offset = TABLE[(r_block.block_id % para.block_num_per_row)][block_w];	// ��ǰblock����ʼ��������Frame����
	int diff = 0;		
	for (int i = start_r; i <= end_r && i + i_offset < para.height; i++)
	{
		int pos=TABLE[i][block_w];                  //ͨ��������˷�������i*block_w
		for (int j = start_c; j <= end_c && j + j_offset < para.width; j++) 
		{
			int16_t block_pool=128;   				//����Ԥ�������ֵ��Ĭ��Ϊ128�����û�вο����ؾ�ȡ128
			int c_i=-1,c_j=-1;  					//�����ÿһ֡������,����Ԥ������ݶ�һ��֡���������洢
			if(pattern_type == 0&&i_offset>0)		// ����Ԥ��, �ο�С���Ϸ���һ������	
			{
				c_i=start_r+i_offset - 1;
				c_j=j_offset + j;
				block_pool=(int16_t)whole_frame.getValue(c_i, c_j);
			}
			else if(pattern_type == 1&&j_offset>0)//����Ԥ��, �ο�С���󷽵�һ������	
			{
				c_i=i_offset + i;
				c_j=j_offset - 1+start_c;
				block_pool=(int16_t)whole_frame.getValue(c_i, c_j);
			}
			else if (pattern_type == 2)//DCԤ��ģʽ	
			{
				/*
				** �������λ�ú�����λ�õ�ƽ��ֵ����Ԥ�����
				*/
				int left_row=0,left_col=0;
				int top_row=0,top_col=0;
				int left=128;int top=128;
				if(i_offset>0)
				{
					top_row=start_r+i_offset - 1;
					top_col=j_offset + j;
					top=whole_frame.getValue(top_row, top_col);
				}
				if(j_offset>0)
				{
					left_row=i_offset + i;
					left_col=j_offset - 1+start_c;
					left=whole_frame.getValue(left_row,left_col);
				}
				block_pool=(top+left)/2;			
			}
			else if (pattern_type == 3&&(i_offset>0&&j_offset>0))//��б45��Ԥ��, �ο�б���Ϸ���һ������	
			{
				if(i-start_r>j-start_c)
				{		//c_i=i-j+start_c+start_c-start_r-1+i_offset;
					c_i=i-j+start_c-1+i_offset;
					c_j=start_c-1+j_offset;
		        }
				else
				{
					c_i=start_r-1+i_offset;
						//c_j=j-i-start_c+start_r+start_r-1+j_offset;
					c_j=j-i+start_r-1+j_offset;
				}
				block_pool=(int16_t)whole_frame.getValue(c_i, c_j);
			}
			else
			{
			}
			r_block.data[pos + j] = (int16_t)block.data[pos + j] - block_pool;    //ͨ��С����ֵ��Ԥ���ֵ��в�
			diff +=  Square_table[abs(r_block.data[pos + j])];                    //����ƽ�������ڼ���RDO
		}
	}
	return diff;
}
/*
** ֡�䷴Ԥ��ģ�飬������֡��Ԥ��ģ��Ĳ���������ͬ��ע���ڷ�Ԥ��ʱ����и���Ԥ��صĲ���
** ������
*/
void Pattern::de_predict(Block& blk,ResidualBlock & r_block,int start_r,int start_c,int end_r,int end_c, BlockBufferPool& b_pool,int pattern_type, AVFormat &para) {
	int block_w, block_h;
	r_block.getBlockSize(para, block_w, block_h);								// ��ǰblock�Ŀ�͸�
	int i_offset = TABLE[r_block.block_id / para.block_num_per_row][block_h];	// ��ǰblock����ʼ��������Frame����
	int j_offset = TABLE[(r_block.block_id % para.block_num_per_row)][block_w];	// ��ǰblock����ʼ��������Frame����

	for (int i = start_r; i <= end_r && i + i_offset < para.height; i++)
	{
		int pos=TABLE[i][block_w];
		for (int j = start_c; j <= end_c && j + j_offset < para.width; j++) 
		{
			int16_t block_pool=128;
			int c_i=-1,c_j=-1;
			if(pattern_type == 0&&i_offset>0)		//����Ԥ��, �ο�С���Ϸ���һ������	
			{
				c_i=start_r+i_offset - 1;
				c_j=j_offset + j;
				block_pool=(int16_t)b_pool.getValue(c_i, c_j);
			}
			else if(pattern_type == 1&&j_offset>0)	//����Ԥ��, �ο�С���Ϸ���һ������	
			{
				c_i=i_offset + i;
				c_j=j_offset - 1+start_c;
				block_pool=(int16_t)b_pool.getValue(c_i, c_j);
			}
			else if (pattern_type == 2)				//DCԤ��ģʽ	
			{
				
				int left_row=0,left_col=0;
				int top_row=0,top_col=0;
				int left=128;int top=128;
				if(i_offset>0)
				{
					top_row=start_r+i_offset - 1;
					top_col=j_offset + j;
					top=(int16_t)b_pool.getValue(top_row, top_col);
				}
				if(j_offset>0)
				{
					left_row=i_offset + i;
					left_col=j_offset - 1+start_c;
					left=(int16_t)b_pool.getValue(left_row,left_col);
				}
				block_pool=(top+left)/2;
			}
			else if (pattern_type == 3&&i_offset>0&&j_offset>0)	//��б45��Ԥ��, �ο�б���Ϸ���һ������	
			{
				if(i-start_r>j-start_c)                        	//��Ҫ�ж����õ�������������ػ������������
				{
					//c_i=i-j+start_c+start_c-start_r-1+i_offset;
					c_i=i-j+start_c-1+i_offset;
					c_j=start_c-1+j_offset;
		        }
				else
				{
					c_i=start_r-1+i_offset;
					//c_j=j-i-start_c+start_r+start_r-1+j_offset;
					c_j=j-i+start_r-1+j_offset;

				}
				block_pool=(int16_t)b_pool.getValue(c_i, c_j);
			}
			else
			{
			}
			int16_t tmp=(int16_t)r_block.data[pos + j] + block_pool; 		//����ת������Ҫ�ж��Ƿ�Խ��
			if(tmp<0)
				tmp=0;
			else if(tmp>255)
				tmp=255;
			else
			{}
			blk.data[pos + j] = tmp;                                 
			b_pool.setValue(i_offset + i, j_offset +j, blk.data[pos + j]); 	//����Ԥ��ĳأ���������ĺ��Ĳο�
		}
	}
}


const int MAX_INTER_SEARCH_RANGE = 4;


/*
* �Ӱ���.getValue(i, j)/.get_width()/.get_height()������ʵ����, ����.getValue(i, j)������
* ���i, jԽ��, �򷵻�128
*
* �׻ݿ�
*/
int16_t get_value_or_128(BlockBufferPool &sort_of_array, int i, int j) {
	if (i < 0 || i >= sort_of_array.get_height() || j < 0 || j >= sort_of_array.get_width()) return 128;
	else return sort_of_array.getValue(i, j);
}


/* ��ȡ������Ϊ(i + i1, j + j1)���Ĳ�ֵ
* Args:
*	i, j, i1, j1��ʾ��������
*
* �׻ݿ�
*/
int16_t Pattern::interpolate(BlockBufferPool & b_pool, int i, int j, double i1, double j1) {
	double a = get_value_or_128(b_pool, i,     j);
	double b = get_value_or_128(b_pool, i + 1, j);
	double c = get_value_or_128(b_pool, i,     j + 1);
	double d = get_value_or_128(b_pool, i + 1, j + 1);
	double i2 = 1.0 - i1;
	double j2 = 1.0 - j1;
	return (int16_t)(a * i2 * i2 + b * i1 * j2 + c * i2 * j1 + d * i1 * j1);
}


/* ���㵱ǰ����ο����SSD
* Args:
*	block: ��ǰ�����ں��
*	start_r, ..., j_offset: ������ο���Ķ�Ӧ����
*	r_i, r_j: �����������λ��, ֵΪ��ʵ�����MV*(1<<pt)
*	b_pool: �ο�������֡
* 
* �׻ݿ�
*/
int Pattern::calc_SSD_inter(const Block &block, int start_r, int start_c, int end_r, int end_c,
	int r_i, int r_j, int block_w, int i_offset, int j_offset,
	BlockBufferPool & b_pool) {

	int diff = 0;
	for (int i = start_r; i <= end_r; i++) {		// ö�ٿ����
		for (int j = start_c; j <= end_c; j++) {	// ö�ٿ����
			int c_i = i_offset + i + r_i;			// �ο����������
			int c_j = j_offset + j + r_j;			// �ο����������

			int tmp = (int16_t)block.data[TABLE[i][block_w] + j] - get_value_or_128(b_pool, c_i, c_j);

			diff +=TABLE[abs(tmp)][abs(tmp)];
		}
	}
	return diff;
}

/* �������Χ�ӿ����ο���Ĳв�
* Args:
*	block: ��ǰ�����ں��
*	start_r, ..., j_offset: ������ο���Ķ�Ӧ����
*	r_i, r_j: �����������λ��, ֵΪ��ʵ�����MV*(1<<pt)
*	b_pool: �ο�������֡
*
* �׻ݿ�
*/
void Pattern::predict_inter_sub(const Block &block,ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c,
	int r_i, int r_j, int block_w, int i_offset, int j_offset,
	BlockBufferPool & b_pool) {

	int diff = 0;
	for (int i = start_r; i <= end_r; i++) {		// ö�ٿ����
		for (int j = start_c; j <= end_c; j++) {	// ö�ٿ����
			int c_i = i_offset + i + r_i;			// �ο����������
			int c_j = j_offset + j + r_j;			// �ο����������

			int tmp = (int16_t)block.data[TABLE[i][block_w]+ j] - get_value_or_128(b_pool, c_i, c_j);
			r_block.data[TABLE[i][block_w] + j] = tmp;
		}
	}
}

/* �ɸ�����Χ�ӿ�Ĳв�Ͳο���, ��ԭ�����ӿ��YUV������Ϣ
* Args:
*	block: ��ǰ�����ں��
*	start_r, ..., j_offset: ������ο���Ķ�Ӧ����
*	r_i, r_j: �����������λ��, ֵΪ��ʵ�����MV*(1<<pt)
*	cur_pool: ��ǰ������֡
*	ref_pool: �ο�������֡
*
* �׻ݿ�
*/
void Pattern::predict_inter_add(Block &block, const ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c,
	int r_i, int r_j, int block_w, int i_offset, int j_offset,
	BlockBufferPool & ref_pool, BlockBufferPool &cur_pool) {

	int diff = 0;
	for (int i = start_r; i <= end_r; i++) {		// ö�ٿ����
		for (int j = start_c; j <= end_c; j++) {	// ö�ٿ����
			int c_i = i_offset + i + r_i;			// �ο����������
			int c_j = j_offset + j + r_j;			// �ο����������

			int16_t tmp = (int16_t)r_block.data[TABLE[i][block_w] + j] + get_value_or_128(ref_pool, c_i, c_j);
			if(tmp < 0)
				tmp = 0;
			else if (tmp > 255)
				tmp=255;
			else
			{}
			block.data[TABLE[i][block_w] + j] = tmp;
			cur_pool.setValue(i+i_offset, j+j_offset, tmp);
		}
	}
}

/* ֡��Ԥ�����������SSD
* ʹ����������/�۰벽����,
*/
int Pattern::inter_predict(Block& block, ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c,
	FrameBufferPool &f_pool, InterMV &inter_mv, AVFormat &para, int diff_thresshold,int i_offset,int j_offset) {
	int search_dir[4][2] = {
		-1, 0,
		0, -1,
		0, 1,
		1, 0,
	};
	int frame_width = para.width;
	int frame_height = para.height;
	int block_w, block_h;
	r_block.getBlockSize(para, block_w, block_h);
	//
	//int i_offset = TABLE[r_block.block_id / para.block_num_per_row][block_h];// ��ǰblock����ʼ��������Frame����
	//int j_offset = TABLE[(r_block.block_id % para.block_num_per_row)][block_w];	// ��ǰblock����ʼ��������Frame����

	if(f_pool.size() <= 1) {
		return INT_MAX;
	}

	int diff = INT_MAX;
	int best_fi = 0;
	std::pair<int, int> best_mv, mv, nmv;
	std::map< std::pair<int, int>, int> hash;
	for (int fi = f_pool.size() - 2; fi >= 0 && fi >= f_pool.size() - 5; fi--) {
		mv.first = 0;
		mv.second = 0;
		int cur_diff = calc_SSD_inter(block, start_r, start_c, end_r, end_c, mv.first, mv.second, block_w, i_offset, j_offset, f_pool[fi]);
		hash.clear();
		hash[mv] = cur_diff;

		
//		int search_count=0;
		int step_length = 2;
		while (step_length > 0) {
			int best_d_p = -1, best_new_diff = INT_MAX;
			for (int d_p = 0; d_p < 4; d_p ++) {
				nmv.first = mv.first + (search_dir[d_p][0] * step_length);
				nmv.second = mv.second + (search_dir[d_p][1] * step_length);
				if (nmv.first < -MAX_INTER_SEARCH_RANGE || nmv.first >= MAX_INTER_SEARCH_RANGE) continue;
				if (nmv.second < -MAX_INTER_SEARCH_RANGE || nmv.second >= MAX_INTER_SEARCH_RANGE) continue;
				
				//����SSDֵ
				int new_diff;
				if (hash.find(nmv) != hash.end()) {
					new_diff = hash[nmv];
				} else {
					new_diff = calc_SSD_inter(block, start_r, start_c, end_r, end_c, nmv.first, nmv.second, block_w, i_offset, j_offset, f_pool[fi]);
					hash[nmv] = new_diff;
				}

				// �������Ľ��ϵ����SSD
				if (new_diff < best_new_diff) {
					best_new_diff = new_diff;
					best_d_p = d_p;
				}
			}

			// ���µ�ǰ֡��õ�SSD
			if (best_new_diff < cur_diff) {
				mv.first += (search_dir[best_d_p][0] * step_length);
				mv.second += (search_dir[best_d_p][1] * step_length);
				cur_diff = best_new_diff;
			} else { // ���²���, ����������
				step_length >>= 1;
			};
		}

		if (cur_diff < diff) {
			diff = cur_diff;
			best_mv = mv;
			best_fi = fi;
		} else {
			break;
		}
	}

	// ��Χ�����SSD, mv, �ο�֡λ��(֡����frame_pool�еķ�ʽ�ڽ��������һ��, ��¼����֡�ڳ�����Ե�ǰ֡��λ��)
	inter_mv.fi = f_pool.size() - 2 - best_fi;
	inter_mv.mv = best_mv;

	return diff;
}


void Pattern::inter_predict_setvalue(Block& block, ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c, FrameBufferPool &f_pool, InterMV &inter_mv, AVFormat &para,int i_offset,int j_offset) {
	int frame_width = para.width;
	int frame_height = para.height;
	int block_w, block_h;
	r_block.getBlockSize(para, block_w, block_h);
	
	int fi = f_pool.size() - 2 - inter_mv.fi;
	predict_inter_sub(block,r_block,start_r, start_c, end_r, end_c, inter_mv.mv.first, inter_mv.mv.second, block_w, i_offset, j_offset, f_pool[fi]);
}


void Pattern::inter_predict_reverse(Block& block, ResidualBlock &r_block, int start_r, int start_c, int end_r, int end_c, FrameBufferPool &f_pool,BlockBufferPool & block_buffer_pool, InterMV &inter_mv, AVFormat &para) {
	int frame_width = para.width;
	int frame_height = para.height;
	int block_w, block_h;
	r_block.getBlockSize(para, block_w, block_h);
	int i_offset = TABLE[r_block.block_id / para.block_num_per_row][block_h];// ��ǰblock����ʼ��������Frame����
	int j_offset = TABLE[(r_block.block_id % para.block_num_per_row)][block_w];	// ��ǰblock����ʼ��������Frame����
	
	int r_fi = f_pool.size() - 2 - inter_mv.fi;
	int r_i = inter_mv.mv.first;
	int r_j = inter_mv.mv.second;

	predict_inter_add(block,r_block,start_r, start_c, end_r, end_c, inter_mv.mv.first, inter_mv.mv.second, block_w, i_offset, j_offset, f_pool[r_fi], block_buffer_pool);
}


