#pragma once
#include"CPU.h"
#include"Memory.h"
#include"Input.h"
#include"Video.h"
#include"Audio.h"
#include"Timer.h"
#include"Frameskip.h"

#include"defaults.h"

//timer now controlled by main interpreter
#define CPU_SPEED 1000	//clockspeed
#define TIMER_SPEED 60	//this is original implementation do not touch
#define SCREEN_FPS 60	//fps
#define WINDOW_FPS 30	//window
#define SKIP_VALUE 2	//fps / skipValue

class Chip8:public defaults{
private:
	char* title;
	defaults* mainwindow;

	CPU* cpu;
	Memory* memory;
	Input* input;
	Video* video;
	Audio* audio;

	bool running;
	uint16 currentOpcode;
	uint16 controllerOp;	//after cpu processes its stuff, next is chip8 controller output job

	uint8 keyinput;
	uint8 delayRegister; //delay register

	Timer* videoTimerInstance;
	Timer* fskipTimerInstance;
	Timer* delayTimerInstance;
	Timer* windowTimerInstance;


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

	//fps timer
	void startTime();
	void endTime();
	void videoDelay();	//set delay on video because primary output needs to be smooth

	void updateNewTimerSet();
	void calculateSkip();

public:
	void run();	//looper
	void update(); //all logic in here

	void start(char* title, int cpuspeed = CPU_SPEED, int fps = SCREEN_FPS); //start of emulation

};