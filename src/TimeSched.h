#pragma once

#include"defaults.h"
#include"Timer.h"

#define QUEUE_SIZE 0x100
class TimeSched{
private:
	Timer* timeQueue[QUEUE_SIZE];	//components with each cycle required

	uint32_t baseClock = UINT32_MAX;	//smallest one in the queue

	uint32_t countBase = 0;

	uint32_t queuePointer = -1;	//-1 because next queue stinky code
	uint32_t queueSize = 0;

	void checkBaseClock(){
		for (uint32_t i = 0; i < queueSize; i++){
			uint32_t temp = timeQueue[i]->getCycleLimit();
			if (baseClock > temp)
				baseClock = temp;
		}
	}	//check which timer is the smallest and store that one as base. selection

	uint32_t pointerWrap(uint32_t queuePointer){
		return (queuePointer == queueSize) ? 0 : queuePointer;
	}

public:

	uint32_t getBaseClock(){ return baseClock; }
	
	//for cpu while loop
	bool baseLoop(){
		
		if (countBase == baseClock) {	//if limit
			countBase = 0;	//reset base
			return false;
		}
		else countBase++;	//increment base
		
		return true;
	}

	//for other component ifs
	bool subCheck(){
		//next queue
		queuePointer = pointerWrap(queuePointer + 1);
	
		bool temp = timeQueue[queuePointer]->checkTimer();
		timeQueue[queuePointer]->updateTimer(&baseClock);

		return temp;

	} //ALL TASKS MUST BE RUNNING IN ORDER HOW IT WAS ADDED!!!
	//queuePointer will increment every end and start with new timer.

	void reinitSched(){
		checkBaseClock();
		for (uint32_t i = 0; i < queueSize; i++) timeQueue[i]->updateTimer(&baseClock);
	}	//in case of clockspeed/fps readjustment in realtime

	void initSched(){ reinitSched(); }

	void addTimeQueue(Timer *timer){
		timeQueue[queueSize++] = timer;
		checkBaseClock();
	}	//add



};
