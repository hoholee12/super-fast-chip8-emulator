#pragma once
#include"defaults.h"
#include"Memory.h"
#include"Audio.h"
#ifndef DEBUG_TIME
#define DEBUG_TIME 100
#endif

class Debug{
private:
public:

	//put in: static int count++, programCounter, stack[sp - 1], and current opcode
	void printDebug(uint8_t* v, uint16_t pc, uint16_t* stack, uint8_t stackptr, uint16_t opcode, defaults* mainwindow, Memory* memory, Audio* audio, uint8_t delayRegister);


};