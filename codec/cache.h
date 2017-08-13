#pragma once
#include "ResidualBlock.h"
#include <map>




/**
*   树的提前创建,这样不用每次new新的空间,同时防止内存泄露
*   李春尧 
*/
class cache
{
	//缓存 
	static Tree tree_buff[3][3600 * 3 * 128];
	//将子块的树跟缓存一一对应 
	static map<uint64_t,uint32_t> dp[3];
	//使用到的下标
	static int cur[3];
public:
	static Tree * getTree(int i,uint64_t key);
	static void reset(int i);
	cache(void);
	~cache(void);
};


