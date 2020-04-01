#pragma once
#include"defaults.h"
#include"CPU.h"

class Audio:public defaults{
private:
	uint8_t soundTimer;
	
public:
	void init();
	void audioProcess();	//opcode and cpu

	//setter
	void setSoundTimer(uint16_t input, CPU* cpu);

};