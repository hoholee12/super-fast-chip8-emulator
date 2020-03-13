#pragma once
#include"defaults.h"

//wrote this to make it faster than modulo op... instead it has become slower epic fail lol
//but im keeping it for readability, and overflow prevention sake
class Timer{
private:
	uint32 *delayTimerPerFrame; //delay timer
	uint32 cycleCount;

public:
	void init(uint32* input);
	void updateTimer();

	//return true if reached an end of delay
	bool checkTimer();
};