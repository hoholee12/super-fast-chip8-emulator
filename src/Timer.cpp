#include"Timer.h"

//for main
void Timer::init(uint32_t* input){
	delayTimerPerFrame = input;
	//cycleCount = 0;
	temp = 1;
	secondaryInput = &temp;										//save as 1

	calcLimit();												//calculate new limit
}

//for sub
void Timer::init(uint32_t* input, uint32_t *secondaryInput){
	delayTimerPerFrame = input;
	//cycleCount = 0;
	this->secondaryInput = secondaryInput;						//save for later

	calcLimit();												//calculate new limit
}

void Timer::calcLimit(){
	cycleLimit = (*delayTimerPerFrame / *secondaryInput) - 1;	//pre calculation
}


void Timer::updateTimer(){

	cycleCount++;
	if (cycleCount > cycleLimit) cycleCount = 0;
}

bool Timer::checkTimer(){

	if (cycleCount == 0){
		calcLimit();											//calculate new limit
		return true;
	}

	return false;

}