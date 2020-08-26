#pragma once
#include<iostream>
#include<vector>
#include<cstdint>
#include<string>
#include "Memory.h"

/*
	cache me baby

	i is for optimized blocks
	b is for single opcodes

	...shitty design but does it for now
*/


typedef struct _translatorState{

	uint32_t x_val = 0;
	uint32_t y_val = 0;
	uint32_t nx = 0;
	uint32_t nnx = 0;
	uint32_t nnnx = 0;

	//immutable: do not modify!
	uint32_t z_val = 0x0;
	uint32_t f_val = 0xf;

} TranslatorState;

using vect8 = std::vector<uint8_t>;
//using vectTS = std::vector<TranslatorState>;

using ICache = struct _ICache{
	vect8 cache;
	bool check = false;
	uint32_t opcodeCount = 0;
	//vectTS TScache;	//Translator.h does reference to these variables, not copy.

	//executable block ptr
	void* execBlock = NULL;
};

class Cache: public defaults{

	ICache iCache[FULL_MEM_SIZE];
	ICache bCache[FULL_MEM_SIZE];	//for individual opcodes
public:


	//switch i(default)/b(true) cache
	ICache* whichCache(int pc, bool flag = false){ return flag ? &bCache[pc] : &iCache[pc]; }

	uint32_t getOpcodeCount(int pc, bool flag = false){ return whichCache(pc, flag)->opcodeCount; }
	void setOpcodeCount(int pc, uint32_t val, bool flag = false){ whichCache(pc, flag)->opcodeCount = val; }

	bool checkCacheExists(int pc, bool flag = false){ return whichCache(pc, flag)->check; }

	ICache* createCache(int pc, bool flag = false){
		whichCache(pc, flag)->opcodeCount = 0;
		whichCache(pc, flag)->check = true;
		whichCache(pc, flag)->cache.clear();
		//whichCache(pc, flag)->TScache.clear();
		return whichCache(pc, flag);
	}

	ICache* getCache(int pc, bool flag = false){
		if (!checkCacheExists(pc, flag)){
			createCache(pc, flag);
		}
		return whichCache(pc, flag);
	}

	void printCache(int pc, bool flag = false){
		if (!whichCache(pc, flag)->check) return;
		printf("pc = %02X cache dump:\n", pc);
		for (int i = 0; i < whichCache(pc, flag)->cache.size(); i++){ 
			if (i % 0x10 == 0) printf("\n");
			printf("%02X ", whichCache(pc, flag)->cache.at(i));
			
		}
		printf("\n");
	}


	//mark executable if not
	void autoMarkExec(int pc, bool flag = false){
		if (!whichCache(pc, flag)->execBlock){
			
			int i = 0;
			do{
				whichCache(pc, flag)->execBlock = defaults::getExecBuffer();
				
				switch (i){
				case 0: break;
				case 11: printf("fuck\n"); exit(1);
#ifdef DEBUG_CACHE
				case 1: defaults::debugmsg("couldn't allocate executable memory for the %dst time. trying again\n", i); break;
				case 2: defaults::debugmsg("couldn't allocate executable memory for the %dnd time. trying again\n", i); break;
				case 3: defaults::debugmsg("couldn't allocate executable memory for the %drd time. trying again\n", i); break;
				default: defaults::debugmsg("couldn't allocate executable memory for the %dth time. trying again\n", i); break;
#endif
				}
				i++;

			}while(!whichCache(pc, flag)->execBlock);

			//copy buffer
#ifdef DEBUG_CACHE
			defaults::debugmsg("sizeof buffer to be marked = %d\n", whichCache(pc, flag)->cache.size());
#endif
			memcpy(whichCache(pc, flag)->execBlock, whichCache(pc, flag)->cache.data(), whichCache(pc, flag)->cache.size());
			//free old buffer
			whichCache(pc, flag)->cache.clear();
		}
	}

};
