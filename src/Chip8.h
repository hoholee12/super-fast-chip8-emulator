#pragma once
#include"CPU.h"
#include"Memory.h"
#include"Input.h"
#include"Video.h"
#include"Audio.h"
#include"Timer.h"
#include"Frameskip.h"

#include"defaults.h"
#include"Debug.h"



class Chip8:public defaults, Debug{
private:
	char* title;
	int cpuspeed;
	int fps;
	defaults* mainwindow;

	CPU* cpu;
	Memory* memory;
	Input* input;
	Video* video;
	Audio* audio;
	Frameskip* fskip;

	bool running;
	uint16_t currentOpcode;
	uint16_t previousOpcode;	//for some optimization
	uint16_t controllerOp;	//after cpu processes its stuff, next is chip8 controller output job

	uint8_t keyinput;
	uint8_t delayRegister; //delay register

	Timer* fsbInstance;
	Timer* videoTimerInstance;
	Timer* fskipTimerInstance;
	Timer* delayTimerInstance;
	Timer* windowTimerInstance;

	int speedHack = -1; // -1: neutral, 0: low, 0<: high

	void initSpeed(int cpuspeed, int fps);	//in case of reinitialization

	void useSpeedHack();
	
	bool isEndlessLoop = false;	//notify debugger that this is endless loop

	int whichInterpreter; //choose interpreter method

public:
	void run();	//looper
	void updateInterpreter_switch(); //all logic in here
	void updateInterpreter_LUT(); //all logic in here
	void updateInterpreter_jumboLUT(); //all logic in here

	void start(char* title, int cpuspeed = CPU_SPEED, int fps = SCREEN_FPS, int whichInterpreter = 1); //start of emulation
	void debugMe();
	void optimizations(); //cycle optimizations
};