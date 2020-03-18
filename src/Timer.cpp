#include"Timer.h"

void Timer::init(uint32* input){
	delayTimerPerFrame = input;
	cycleCount = 0;
}

void Timer::updateTimer(uint32 timerInput){
	cycleCount++;
	if (cycleCount > (*delayTimerPerFrame / timerInput) - 1) cycleCount = 0;
	
}

bool Timer::checkTimer(){

	if (cycleCount == 0){
		return true;
	}

	return false;

}