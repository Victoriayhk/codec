#pragma once
#include <map>
#include "Pattern.h"
#include "ResidualBlock.h"
#include "decode_buffer_pool.h"

class Pattern
{
public:
	//typedef void (*FUN)(Block&,ResidualBlock &,int,int,int,int,BlockBufferPool&);
	//static int get_pattern_number();
	//static FUN getPattern(int pattern);
	//Pattern(void);
	//~Pattern(void);



	//static std::map<int,int> patterns;
	//static int pattern_number; 
	static void pattern(Block&,ResidualBlock &,int,int,int,int,BlockBufferPool&,int pattern_type);


};
//int Pattern::pattern_number = 0;
