#pragma once

#include<time.h>
#include<stdlib.h>
#include"Memory.h"
#include"Input.h"

typedef unsigned char uint8;
typedef unsigned short uint16;

#define STACK_SIZE 16
#define V_REGISTER_SIZE 0x10

class CPU: public Input{
private:
	
	uint16 programCounter;
	uint8 stackPointer;
	uint16 indexRegister;	//I register
	uint16 stack[STACK_SIZE];
	uint8 v[V_REGISTER_SIZE];
	
	uint8 delayTimer; //delay timer
	uint8 soundTimer; //sound timer

public:
	//getters
	uint16* getProgramCounter();
	uint8* getStackPointer();
	uint16* getIndexRegister();	//I register
	uint16* getStack(uint8 input);
	uint8* getV(uint8 input);



	uint16 decode(Memory* memory, uint16 input, uint8 pressedKey); //current opcode decoder
	uint16 fetch(Memory* memory);
	
	void init();

};