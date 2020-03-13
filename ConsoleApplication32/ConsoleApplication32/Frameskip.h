#pragma once
#include"defaults.h"

//timer now controlled by Frameskip class
#define CPU_SPEED 1000	//clockspeed
#define TIMER_SPEED 60	//this is original implementation do not touch
#define SCREEN_FPS 60	//fps
#define WINDOW_FPS 30	//window
#define SKIP_VALUE 2	//fps / skipValue

class Frameskip:public defaults{
private:
	//fps timer
	uint32 cpuSpeed = CPU_SPEED;
	uint32 timerSpeed = TIMER_SPEED;
	uint32 screenFps = SCREEN_FPS;
	uint32 windowFps = WINDOW_FPS;
	//for reference. it will get reinitialized in start() and updateNewTimerSet()
	uint32 screenTicksPerFrame = cpuSpeed / screenFps;	//cycles
	uint32 delayTimerPerFrame = cpuSpeed / timerSpeed;	//cycles
	uint32 screenDelayPerFrame = 1000 / screenFps;		//milliseconds
	uint32 windowTicksPerFrame = cpuSpeed / windowFps;	//cycles

	uint32 prevTick = 0;
	int holdTick = 0;
	int holdTick_fskip = 0;
	uint32 currTick = 0;

	int skipValue = SKIP_VALUE; //frameskipper
	int skipMultiplier = 1;
	uint32 backupFps;
	uint32 backupTicksPerFrame = cpuSpeed / backupFps;
	uint32 backupDelayPerFrame = 1000 / backupFps;

public:

	void init(int cpuspeed, int fps);

	uint32* getVideoTimer();
	uint32* getFskipTimer();
	uint32* getDelayTimer();
	uint32* getWindowTimer();


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