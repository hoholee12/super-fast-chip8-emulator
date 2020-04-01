#pragma once
#include"defaults.h"

//wrote this to make it faster than modulo op... instead it has become slower epic fail lol
//but im keeping it for readability, and overflow prevention sake
class Timer{
private:
	uint32_t *delayTimerPerFrame; //delay timer
	uint32_t *secondaryInput;

	uint32_t cycleCount = 0;
	uint32_t cycleLimit;

	uint32_t temp;

	void calcLimit();

public:
	void init(uint32_t* input);	//for main
	void init(uint32_t* input, uint32_t* secondaryInput);	//for sub
	void updateTimer();

	//return true if reached an end of delay
	bool checkTimer();
};