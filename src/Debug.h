#pragma once
#include"defaults.h"

class Debug{
private:
public:

	//put in: static int count++, programCounter, stack[sp - 1], and current opcode
	void printDebug(uint16_t pc, uint16_t stack, uint16_t opcode);


};