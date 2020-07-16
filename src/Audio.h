#pragma once
#include"defaults.h"
#include"CPU.h"

class Audio final:public defaults{
private:
	uint8_t soundTimer;
	
public:
	void init();
	void audioProcess();	//opcode and cpu

	//setter
	void setSoundTimer(CPU* cpu);
	uint8_t getSoundTimer(){ return soundTimer; }
};
