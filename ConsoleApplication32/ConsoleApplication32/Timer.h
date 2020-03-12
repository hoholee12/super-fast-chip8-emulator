#pragma once
#include"defaults.h"


class Timer{
private:
	uint8 delayTimer; //delay timer

public:
	void init();
	uint8 getDelayTimer();
	void setDelayTimer(uint8 input);

	//return true if reached an end of delay
	bool cycleDelayTimer(uint32 cycleCount, uint32 delayTimerPerFrame);
};