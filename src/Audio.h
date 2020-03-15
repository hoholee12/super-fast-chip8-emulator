#pragma once
#include"defaults.h"
#include"CPU.h"

class Audio:public defaults{
private:
	uint8 soundTimer;
	
public:
	void init();
	void audioProcess();	//opcode and cpu

	//setter
	void setSoundTimer(uint16 input, CPU* cpu);

};