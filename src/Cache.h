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

	uint8_t x_val = 0;
	uint8_t y_val = 0;
	uint16_t nx = 0;
	uint16_t nnx = 0;
	uint16_t nnnx = 0;

	//immutable: do not modify!
	uint8_t z_val = 0x0;
	uint8_t f_val = 0xf;

} TranslatorState;

using CompOps = struct _CompOps{
	uint16_t opcode = 0;
	uint32_t opsize = 0;
};


using vect8 = std::vector<uint8_t>;
using vectCO = std::vector<CompOps>;


using ICache = struct _ICache{
	vect8 cache;
	vectCO oplist;
	bool check = false;
	uint16_t linkedPC = 0;
	uint32_t opcodeCount = 0;

	//

	int bytesize = 0;
	//executable block ptr
	void* execBlock = NULL;
};

class Cache: public defaults{

	ICache iCache[FULL_MEM_SIZE / 2];
	ICache bCache[FULL_MEM_SIZE / 2];	//for individual opcodes
public:
	
	Cache(){
#ifdef DEBUG_CACHE
		printf("iCache starting point: %08X\n"
			"bCache starting point: %08X\n", iCache, bCache);
		uint16_t offender = 0x246;
		printf("offending pc(%03X): %08X\n", offender, iCache + sizeof(ICache) * offender / 2);

#endif
	}

	//switch i(default)/b(true) cache
	ICache* whichCache(int pc, bool flag = false){ return flag ? &bCache[pc / 2] : &iCache[pc / 2]; }

	uint32_t getOpcodeCount(int pc, bool flag = false){ return whichCache(pc, flag)->opcodeCount; }
	void setOpcodeCount(int pc, uint32_t val, bool flag = false){ whichCache(pc, flag)->opcodeCount = val; }

	bool checkCacheExists(int pc, bool flag = false){ return whichCache(pc, flag)->check; }

	ICache* createCache(int pc, bool flag = false){
		whichCache(pc, flag)->opcodeCount = 0;
		whichCache(pc, flag)->check = true;
		whichCache(pc, flag)->cache.clear();
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


	//mark executable if not, return true if already filled
	bool autoMarkExec(int pc, bool flag = false){
		if (!whichCache(pc, flag)->execBlock){
			int i = 0;
			do{
				whichCache(pc, flag)->execBlock = defaults::getExecBuffer();
#ifdef DEBUG_CACHE
				printf("pc:%3x -> make ebuffer:%8X\n", pc, whichCache(pc, flag)->execBlock);
#endif
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

			return false;
		}
#ifdef DEBUG_CACHE
		printf("skipped marking,\t");
#endif
		return true;
	}


	void insertOplist(uint16_t opcode, uint32_t opsize, int pc, bool flag = false){
		if (flag) return; //not for bCache
		CompOps temp;
		temp.opcode = opcode;
		temp.opsize = opsize;
		whichCache(pc, flag)->oplist.push_back(temp);
		whichCache(pc, flag)->bytesize += opsize;
	}

	void populateLocal(int pc, bool flag = false){
		if (flag){
			//OOPSIE
			autoMarkExec(pc, flag);
			return;
		}

#ifdef DEBUG_CACHE
		if (pc == 0x246) 
			printf("break here: %08X\n", &whichCache(pc, flag)->execBlock);
#endif

		if (autoMarkExec(pc, flag)) return; //do not touch if block already exists

		int localpc = pc + 2; //start on next opcode
		int limit = whichCache(pc, flag)->oplist.size() - 1;

		char* appendtemp = (char*)whichCache(pc, flag)->execBlock;
#ifdef DEBUG_CACHE
		printf("build op:%08X, pc:%3x\n", whichCache(pc, flag)->execBlock, pc);
#endif
		for (int i = 0; i < limit; i++, localpc += 2){

			if (!whichCache(localpc, flag)->execBlock){	//if empty
				whichCache(localpc, flag)->check = true;
				appendtemp += whichCache(pc, flag)->oplist[i].opsize;
#ifdef DEBUG_CACHE
				printf("appending:\n", whichCache(pc, flag)->clist[i].opsize);
#endif
				whichCache(localpc, flag)->execBlock = (void*)appendtemp;
				whichCache(localpc, flag)->linkedPC = pc;
#ifdef DEBUG_CACHE
				printf("link op:%08X, pc:%3x\n", whichCache(localpc, flag)->execBlock, localpc);
#endif
			}
		}
	}

	//recursive
	void destroyCache(int pc, bool flag = false){
		//recursion to root
		if (whichCache(pc, flag)->linkedPC != 0) destroyCache(whichCache(pc, flag)->linkedPC, flag);
		else{
			//if root
			//purge
			defaults::purgeExecBuffer(whichCache(pc, flag)->execBlock);

		}
		whichCache(pc, flag)->cache.clear();
		whichCache(pc, flag)->oplist.clear();
		whichCache(pc, flag)->check = false;
		whichCache(pc, flag)->linkedPC = 0;
		whichCache(pc, flag)->opcodeCount = 0;

		whichCache(pc, flag)->bytesize = 0;
		//executable block ptr
		whichCache(pc, flag)->execBlock = NULL;

	}

};
