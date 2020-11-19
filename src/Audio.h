#pragma once
#include"defaults.h"
#include"CPU.h"

class Audio: public defaults{
private:
	uint8_t soundTimer;
	bool soundFlag;
	
public:
	void init();
	void audioProcess();	//opcode and cpu

	//setter
	void setSoundTimer(CPU* cpu);
	uint8_t getSoundTimer(){ return soundTimer; }
};
