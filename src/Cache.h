#pragma once
#include<iostream>
#include<vector>
#include<cstdint>
#include<string>
#include "Memory.h"

/*
	Dyanarec the executable cache club

*/

using vect8 = std::vector<uint8_t>;

class Cache{
public:

	vect8 iCache[FULL_MEM_SIZE];
	bool iCheck[FULL_MEM_SIZE];

	Cache(){
		for (int i = 0; i < FULL_MEM_SIZE; i++) iCheck[i] = false;
	}

	bool checkCacheExists(int pc){ return iCheck[pc]; }

	vect8* createCache(int pc){
		iCheck[pc] = true;
		iCache[pc].clear();
		return &iCache[pc];
	}

	void printCache(int pc){
		if (!iCheck[pc]) return;
		printf("pc = %02X cache dump:\n", pc);
		for (int i = 0; i < iCache[pc].size(); i++) printf("%02X ", iCache[pc].at(i));
	}


};