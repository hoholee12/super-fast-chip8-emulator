#pragma once
#include<stdio.h>
#include<stdlib.h>	//FILE I/O
#include"defaults.h"

#define FONT_COUNT 0x10
#define FULL_MEM_SIZE 0x1000
#define MID_MEM_SIZE 0x600
#define START_MEM_SIZE 0x200


//write default fontdata to memory[0~]
const uint8_t fontSet[FONT_COUNT * 5] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

class MemoryInstance{
public:
	uint8_t mem[FULL_MEM_SIZE];


	//inline getter
	uint8_t read(uint16_t addr);
	void write(uint16_t addr, uint8_t input);

	void init(const char* str);	//file load included

	//copy initializer
	MemoryInstance(){}
	MemoryInstance(const MemoryInstance& obj){
		for (int i = 0; i < FULL_MEM_SIZE; i++){
			mem[i] = obj.mem[i];
		}
	}

};

inline uint8_t MemoryInstance::read(uint16_t addr){
	if (addr >= FULL_MEM_SIZE) return 0x0;	//no OOB
	return mem[addr];

}

//RAII
class Memory{
	MemoryInstance* a;
	MemoryInstance* b;
public:
	Memory(){
		a = new MemoryInstance();
		b = new MemoryInstance();
	}
	~Memory(){
		delete(a);
		delete(b);
	}
	void init(const char* str){
		a->init(str);
		for (int i = 0; i < FULL_MEM_SIZE; i++){
			b->write(i, a->read(i));
		}
	}

	//flag = true: backup memory
	MemoryInstance* whichMemory(bool flag = false){ return (!flag) ? a : b; }

	uint8_t read(uint16_t addr, bool flag = false){ return whichMemory(flag)->read(addr); }

	void write(uint16_t addr, uint8_t input, bool flag = false){ whichMemory(flag)->write(addr, input); }

	uint8_t& operator[](uint16_t addr){ return a->mem[addr]; }



};

