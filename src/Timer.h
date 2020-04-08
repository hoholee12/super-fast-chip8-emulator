#pragma once
#include"defaults.h"

//wrote this to make it faster than modulo op... instead it has become slower epic fail lol
//but im keeping it for readability, and overflow prevention sake
class Timer final{
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

	uint32_t getCycleLimit(){ return cycleLimit + 1; } //for TimerSched

	//return true if reached an end of delay
	bool checkTimer();
};

//inline getter
inline bool Timer::checkTimer(){

	if (cycleCount == 0){
		calcLimit();											//calculate new limit
		return true;
	}

	return false;

}

//loop inline
inline void Timer::updateTimer(){

	cycleCount++;
	if (cycleCount > cycleLimit) cycleCount = 0;
}
