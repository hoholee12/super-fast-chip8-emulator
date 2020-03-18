#pragma once

#include<stdlib.h>
#include<time.h>	//for random seed

#include"Memory.h"
#include"Input.h"
#include"defaults.h"
#include"Timer.h"

#define STACK_SIZE 16
#define V_REGISTER_SIZE 0x10

class CPU: public Input{
private:
	
	uint16 programCounter;
	uint8 stackPointer;
	uint16 indexRegister;	//I register
	uint16 stack[STACK_SIZE];
	uint8 v[V_REGISTER_SIZE];
	
	

public:
	//getters
	uint16* getProgramCounter();
	uint8* getStackPointer();
	uint16* getIndexRegister();	//I register
	uint16* getStack(uint8 input);
	uint8* getV(uint8 input);


	//interpreter needs memory to access, a 60hz delay register(not implemented in cpu), a fetched opcode, and input key
	uint16 decode(Memory* memory, uint8 *delayRegister, uint16 input, uint8 pressedKey); //current opcode decoder
	uint16 fetch(Memory* memory);
	
	void init();

};