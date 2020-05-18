#pragma once
#include<iostream>
#include<vector>
#include<cstdint>
#include<string>

/*
	Dyanarec the executable cache club

*/

using vect8 = std::vector<uint8_t>;

class Cache{
public:
#define CACHE_SIZE 4096
	vect8* iCache[CACHE_SIZE];

	Cache(){
		//init with NULL
		for (int i = 0; i < CACHE_SIZE; i++) iCache[i] = NULL;
	}

	bool checkCacheExists(int pc){
		if (iCache[pc] != NULL) return true;
		else return false;
	}

	vect8* createCache(int pc){
		if (iCache[pc] != NULL) delete iCache[pc];
		iCache[pc] = new vect8();
		return iCache[pc];
	}

	void dumpCache(int pc){
		if (iCache[pc] == NULL) return;
		printf("pc = %02X cache dump:\n", pc);
		for (int i = 0; i < iCache[pc]->size(); i++) printf("%02X ", iCache[pc]->at(i));
	}


};