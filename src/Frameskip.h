#pragma once
#include"defaults.h"

//timer now controlled by Frameskip class
#define CPU_SPEED 1000	//clockspeed - do not go under TIMER_SPEED ----> upto 3500000hz(3.5mhz) possible with 60fps on intel 5th gen 3.1ghz
#define FSB_SPEED 60
#define TIMER_SPEED 60	//this is original implementation do not touch
#define SCREEN_FPS 60	//fps - do not go over TIMER_SPEED
#define WINDOW_FPS 2	//window - do not go over TIMER_SPEED
#define SKIP_VALUE 2	//fps / skipValue

class Frameskip:public defaults{
private:
	//fps timer
	uint32 cpuSpeed = CPU_SPEED;
	uint32 timerSpeed = TIMER_SPEED;
	uint32 screenFps = SCREEN_FPS;
	uint32 windowFps = WINDOW_FPS;
	uint32 fsbSpeed = FSB_SPEED;
	//for reference. it will get reinitialized in start() and updateNewTimerSet()
	uint32 screenTicksPerFrame = cpuSpeed / screenFps;	//cycles
	uint32 delayTimerPerFrame = cpuSpeed / timerSpeed;	//cycles
	uint32 screenDelayPerFrame = 1000 / screenFps;		//milliseconds
	uint32 windowTicksPerFrame = cpuSpeed / windowFps;	//cycles
	uint32 fsbTicksPerFrame = cpuSpeed / fsbSpeed;		//cycles

	uint32 prevTick = 0;
	int holdTick = 0;
	int holdTick_fskip = 0;
	uint32 currTick = 0;

	int skipValue = SKIP_VALUE; //frameskipper
	int skipMultiplier = 1;
	uint32 backupFps = screenFps;
	uint32 backupTicksPerFrame = cpuSpeed / backupFps;
	uint32 backupDelayPerFrame = 1000 / backupFps;

public:

	void init(int cpuspeed, int fps);

	uint32* getVideoTimer();
	uint32* getFskipTimer();
	uint32* getDelayTimer();
	uint32* getWindowTimer();
	uint32* getFsbTimer();

	//for window
	uint32 getCpuSpeed();
	uint32 getBackupFps();
	uint32 getHoldTick();

	void endTime();
	void calculateSkip();
	void videoDelay();
	void startTime();

	void updateNewTimerSet();

};
