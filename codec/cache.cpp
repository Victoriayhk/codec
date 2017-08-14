#include "cache.h"


/**
* 树的缓存结构，节省堆空间开销
* 李春尧
**/
Tree cache::tree_buff[3][3600 * 3 * 128];
map<uint64_t,uint32_t> cache::dp[3];
int cache::cur[3];


cache::cache(void)
{
}


cache::~cache(void)
{
}
Tree * cache::getTree(int i,uint64_t key){
	/**
	*  获取一棵树，如果已经存在，使用老树
	*/
	if(dp[i].find(key) == dp[i].end()){
		dp[i][key] = cur[i]++;
		++cur[i];
	}
	return &tree_buff[i][dp[i][key]];

}

void cache::reset(int i){
	/**
	*  每一帧对数据结构清零
	*/
	for (map<uint64_t,uint32_t>::iterator p= dp[i].begin(); p!=dp[i].end(); ++p) 
	{
		tree_buff[i][p->second].score = -1;
		tree_buff[i][p->second].left = nullptr;
		tree_buff[i][p->second].right = nullptr;
		tree_buff[i][p->second].data = nullptr;
		tree_buff[i][p->second].node_id = 1024;
	}
	dp[i].clear();
	cur[i] = 0;

}
