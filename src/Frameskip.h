#pragma once
#include"defaults.h"

//timer now controlled by Frameskip class
#define CPU_SPEED 1000	//clockspeed - do not go under TIMER_SPEED ----> upto 3500000hz(3.5mhz) possible with 60fps on intel 5th gen 3.1ghz
#define FSB_SPEED 120
#define TIMER_SPEED 60	//this is original implementation do not touch
#define SCREEN_FPS 60	//fps - do not go over TIMER_SPEED
#define WINDOW_FPS 1	//window - do not go over TIMER_SPEED
#define SKIP_VALUE 2	//fps / skipValue

class Frameskip:public defaults{
private:
	//fps timer
	uint32_t cpuSpeed = CPU_SPEED;
	uint32_t timerSpeed = TIMER_SPEED;
	uint32_t screenFps = SCREEN_FPS;
	uint32_t windowFps = WINDOW_FPS;
	uint32_t fsbSpeed = FSB_SPEED;
	//for reference. it will get reinitialized in start() and updateNewTimerSet()
	uint32_t screenTicksPerFrame = cpuSpeed / screenFps;	//cycles
	uint32_t delayTimerPerFrame = cpuSpeed / timerSpeed;	//cycles
	uint32_t screenDelayPerFrame = 1000 / screenFps;		//milliseconds
	uint32_t windowTicksPerFrame = cpuSpeed / windowFps;	//cycles
	uint32_t fsbTicksPerFrame = cpuSpeed / fsbSpeed;		//cycles

	uint32_t prevTick = 0;
	int holdTick = 0;
	int holdTick_fskip = 0;
	uint32_t currTick = 0;

	int skipValue = SKIP_VALUE; //frameskipper
	int skipMultiplier = 1;
	uint32_t backupFps = screenFps;
	uint32_t backupTicksPerFrame = cpuSpeed / backupFps;
	uint32_t backupDelayPerFrame = 1000 / backupFps;

public:

	void init(int cpuspeed, int fps);

	uint32_t* getVideoTimer();
	uint32_t* getFskipTimer();
	uint32_t* getDelayTimer();
	uint32_t* getWindowTimer();
	uint32_t* getFsbTimer();

	//for window
	uint32_t getCpuSpeed();
	uint32_t getBackupFps();
	uint32_t getHoldTick();

	void endTime();
	void calculateSkip();
	void videoDelay();
	void startTime();

	void updateNewTimerSet();

};
