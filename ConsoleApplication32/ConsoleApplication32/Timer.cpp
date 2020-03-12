#include"Timer.h"

void Timer::init(){
	delayTimer = 0x0;
}


uint8 Timer::getDelayTimer(){ return delayTimer; }
void Timer::setDelayTimer(uint8 input){ delayTimer = input; }

bool Timer::cycleDelayTimer(uint32 cycleCount, uint32 delayTimerPerFrame){

	//delay
	if (cycleCount % delayTimerPerFrame == 0){
		if (delayTimer > 0x0) delayTimer--;
		return true;
	}

	return false;

}