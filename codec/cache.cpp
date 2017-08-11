#include "cache.h"



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
	if(dp[i].find(key) == dp[i].end()){
		dp[i][key] = cur[i]++;
		++cur[i];
	}
	return &tree_buff[i][dp[i][key]];

}

void cache::reset(int i){
	
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