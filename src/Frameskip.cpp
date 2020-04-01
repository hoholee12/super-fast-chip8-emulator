#include"Frameskip.h"


void Frameskip::init(int cpuspeed, int fps){

	//frameskip init
	screenFps = fps;
	if (screenFps > timerSpeed) screenFps = timerSpeed; //no more than timerSpeed 60hz
	if (fsbSpeed < timerSpeed) fsbSpeed = timerSpeed;
	cpuSpeed = cpuspeed;
	if (cpuSpeed < fsbSpeed) cpuSpeed = fsbSpeed; //prevent division by zero
	updateNewTimerSet();
	calculateSkip();

}

uint32_t* Frameskip::getVideoTimer(){ return &screenTicksPerFrame; }
uint32_t* Frameskip::getFskipTimer(){ return &backupTicksPerFrame; }
uint32_t* Frameskip::getDelayTimer(){ return &delayTimerPerFrame; }
uint32_t* Frameskip::getWindowTimer(){ return &windowTicksPerFrame; }
uint32_t* Frameskip::getFsbTimer(){ return &fsbTicksPerFrame; }

uint32_t Frameskip::getCpuSpeed(){ return cpuSpeed; }
uint32_t Frameskip::getBackupFps(){ return backupFps; }
uint32_t Frameskip::getHoldTick(){ return holdTick; }

void Frameskip::startTime(){
	prevTick = defaults::checkTime();
}

void Frameskip::endTime(){
	currTick = defaults::checkTime() - prevTick;	//current tick checked here

	holdTick = screenDelayPerFrame - currTick;		//for original speed

}

void Frameskip::videoDelay(){

	if (holdTick > 0) defaults::delayTime(holdTick);


}

void Frameskip::updateNewTimerSet(){

	screenTicksPerFrame = cpuSpeed / screenFps;		//cycles
	delayTimerPerFrame = cpuSpeed / timerSpeed;		//cycles
	screenDelayPerFrame = 1000 / screenFps;			//milliseconds
	windowTicksPerFrame = cpuSpeed / windowFps;		//cycles
	fsbTicksPerFrame = cpuSpeed / fsbSpeed;	//cycles
}

void Frameskip::calculateSkip(){

	//skip mechanism
	int tempTick = holdTick;
	if (holdTick < 0){
		tempTick *= -1; //convert to positive
		skipMultiplier = tempTick / screenDelayPerFrame + 1;
		backupFps = screenFps / (skipValue * skipMultiplier);
	}
	else backupFps = screenFps;

	if (backupFps == 0) backupFps = 1;		//prevent div by zero

	//printf("holdtick = %d holdtickfskip = %d skipmultiplier = %d\n", holdTick, holdTick_fskip, skipMultiplier);

	//apply skip value
	backupTicksPerFrame = cpuSpeed / backupFps;
	//backupDelayPerFrame = 1000 / screenFps;		//milliseconds
	//this wouldnt make sense, new sleep calculation means slower speed with slower fps lol


}
