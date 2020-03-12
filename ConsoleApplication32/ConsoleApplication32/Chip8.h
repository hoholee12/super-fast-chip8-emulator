#pragma once
#include"CPU.h"
#include"Memory.h"
#include"Input.h"
#include"Video.h"
#include"Audio.h"
#include"Timer.h"

#include"defaults.h"

//timer now controlled by main interpreter
#define CPU_SPEED 1000	//clockspeed
#define TIMER_SPEED 60	//this is original implementation do not touch
#define SCREEN_FPS 60	//fps

class Chip8:public defaults{
private:
	CPU* cpu;
	Memory* memory;
	Input* input;
	Video* video;
	Audio* audio;
	Timer* timer;

	bool running;
	uint16 currentOpcode;
	uint16 controllerOp;	//after cpu processes its stuff, next is chip8 controller output job

	uint8 keyinput;

	//fps timer
	uint32 screenTicksPerFrame = CPU_SPEED / SCREEN_FPS;
	uint32 delayTimerPerFrame = CPU_SPEED / TIMER_SPEED;

	uint32 cycleCount;

	uint32 prevTick;
	int holdTick;
	//fps timer
	void startTime();
	void videoDelay();	//set delay on video because primary output needs to be smooth

public:
	void run();	//looper
	void update(); //all logic in here

	void start(char* str); //start of emulation

};