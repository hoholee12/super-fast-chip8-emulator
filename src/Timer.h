#pragma once
#include"defaults.h"

//wrote this to make it faster than modulo op... instead it has become slower epic fail lol
//but im keeping it for readability, and overflow prevention sake
class Timer{
private:
	uint32_t *delayTimerPerFrame; //delay timer
	uint32_t cycleCount;

public:
	void init(uint32_t* input);
	void updateTimer(uint32_t timerInput = 1);

	//return true if reached an end of delay
	bool checkTimer();
};