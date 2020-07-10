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

using ICache = struct _ICache{
	vect8 cache;
	uint16_t endOp = NULL;
	bool check = false;
};

class Cache{

	ICache iCache[FULL_MEM_SIZE];
public:


	bool checkCacheExists(int pc){ return iCache[pc].check; }

	ICache* createCache(int pc){
		iCache[pc].endOp = pc;
		iCache[pc].check = true;
		iCache[pc].cache.clear();
		return &iCache[pc];
	}

	ICache* getCache(int pc){
		if (!checkCacheExists(pc)){
			createCache(pc);
		}
		return &iCache[pc];
	}

	void printCache(int pc){
		if (!iCache[pc].check) return;
		printf("pc = %02X cache dump:\n", pc);
		for (int i = 0; i < iCache[pc].cache.size(); i++) printf("%02X ", iCache[pc].cache.at(i));
	}


};