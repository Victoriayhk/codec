#pragma once
#include "ResidualBlock.h"
#include <map>




/**
*   ������ǰ����,��������ÿ��new�µĿռ�,ͬʱ��ֹ�ڴ�й¶
*   �Ң 
*/
class cache
{
	//���� 
	static Tree tree_buff[3][3600 * 3 * 128];
	//���ӿ����������һһ��Ӧ 
	static map<uint64_t,uint32_t> dp[3];
	//ʹ�õ����±�
	static int cur[3];
public:
	static Tree * getTree(int i,uint64_t key);
	static void reset(int i);
	cache(void);
	~cache(void);
};


