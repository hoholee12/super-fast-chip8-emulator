#pragma once

#include"defaults.h"
#include"Timer.h"

#define QUEUE_SIZE 0x100
class TimeSched{
private:
	Timer* timeQueue[QUEUE_SIZE];	//components with each cycle required
	uint32_t flowQueue[QUEUE_SIZE];	//recalculation of those components for a row of loops

	uint32_t baseClock = UINT32_MAX;	//biggest one in the queue
	uint32_t subClock = 0;

	uint32_t countBase = 0;
	uint32_t countSub = 0;

	uint32_t queuePointer = -1;	//-1 because next queue stinky code
	uint32_t queueSize = 0;

	void checkBaseClock(){
		for (uint32_t i = 0; i < queueSize; i++){
			if (baseClock > flowQueue[i]) baseClock = flowQueue[i] + 1; //hax
			if (subClock < flowQueue[i]) subClock = flowQueue[i] - 1;	//hax
		}
	}	//check which clock is the biggest and store that one as base. selection

	uint32_t pointerWrap(uint32_t queuePointer){
		return (queuePointer > queueSize - 1) ? 0 : queuePointer;
	}

public:

	
	//for cpu while loop
	bool baseLoop(){
		countBase++;	//increment base
		
		if (countBase == baseClock) {	//if limit
			countSub += (countBase - 1);	//increment sub
			if (countSub > subClock) countSub = 0;	//if limit reset sub
			
			countBase = 0;	//reset base
			return false;
		}
		
		return true;
	}

	//for other component ifs
	bool shouldOneRunThis(){
		//next queue
		queuePointer = pointerWrap(queuePointer + 1);

		if (countSub % flowQueue[queuePointer] == 0) return true;
		return false;

	} //ALL TASKS MUST BE RUNNING IN ORDER HOW IT WAS ADDED!!!
	//queuePointer will increment every end and start with new timer.

	void reinitTimer(){
		for (uint32_t i = 0; i < queueSize; i++)
			flowQueue[i] = timeQueue[i]->getCycleLimit();
		checkBaseClock();
	}	//in case of clockspeed/fps readjustment in realtime

	void addTimeQueue(Timer *timer){
		timeQueue[queueSize++] = timer;
		reinitTimer();
	}	//add



};
