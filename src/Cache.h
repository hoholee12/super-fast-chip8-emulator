#pragma once
#include<iostream>
#include<vector>
#include<cstdint>
#include<string>
#include "Memory.h"

/*
	Dyanarec the executable cache club

*/


typedef struct _translatorState{

	uint32_t x_val = 0;
	uint32_t y_val = 0;
	uint32_t nx = 0;
	uint32_t nnx = 0;
	uint32_t nnnx = 0;

} TranslatorState;

using vect8 = std::vector<uint8_t>;
using vectTS = std::vector<TranslatorState>;

using ICache = struct _ICache{
	vect8 cache;
	bool check = false;
	uint32_t opcodeCount = 0;
	vectTS TScache;	//Translator.h does reference to these variables, not copy.
};

class Cache{

	ICache iCache[FULL_MEM_SIZE];
public:

	uint32_t getOpcodeCount(int pc){ return iCache[pc].opcodeCount; }
	void setOpcodeCount(int pc, uint32_t val){ iCache[pc].opcodeCount = val; }

	bool checkCacheExists(int pc){ return iCache[pc].check; }

	ICache* createCache(int pc){
		iCache[pc].opcodeCount = 0;
		iCache[pc].check = true;
		iCache[pc].cache.clear();
		iCache[pc].TScache.clear();
		return &iCache[pc];
	}

	ICache* createFallback(int pc){
		iCache[pc].check = true;
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
		printf("\n");
	}


};