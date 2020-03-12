#include"Timer.h"

void Timer::init(){
	delayTimer = 0x0;
}


uint8 Timer::getDelayTimer(){ return delayTimer; }
void Timer::setDelayTimer(uint8 input){ delayTimer = input; }

void Timer::cycleDelayTimer(uint32 cycleCount, uint32 delayTimerPerFrame){
	if (cycleCount % delayTimerPerFrame == 0)
		if (delayTimer > 0x0) delayTimer--;

}