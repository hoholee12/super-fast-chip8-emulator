#pragma once
#include"defaults.h"

class Debug{
private:
public:

	//put in: static int count++, programCounter, stack[sp - 1], and current opcode
	void printDebug(uint16 pc, uint16 stack, uint16 opcode);


};