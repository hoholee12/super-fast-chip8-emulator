#pragma once
#include"defaults.h"

class Timer{
private:
	uint8 delayTimer; //delay timer

public:
	void init();
	uint8 getDelayTimer();
	void setDelayTimer(uint8 input);

	void cycleDelayTimer(uint32 cycleCount, uint32 delayTimerPerFrame);
};