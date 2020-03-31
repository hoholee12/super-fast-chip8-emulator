#include"Timer.h"

void Timer::init(uint32_t* input){
	delayTimerPerFrame = input;
	cycleCount = 0;
}

void Timer::updateTimer(uint32_t timerInput){
	cycleCount++;
	if (cycleCount > (*delayTimerPerFrame / timerInput) - 1) cycleCount = 0;
	
}

bool Timer::checkTimer(){

	if (cycleCount == 0){
		return true;
	}

	return false;

}