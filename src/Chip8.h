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
	uint16 currentOpcode;
	uint16 previousOpcode;	//for some optimization
	uint16 controllerOp;	//after cpu processes its stuff, next is chip8 controller output job

	uint8 keyinput;
	uint8 delayRegister; //delay register

	Timer* videoTimerInstance;
	Timer* fskipTimerInstance;
	Timer* delayTimerInstance;
	Timer* windowTimerInstance;

	void init(int cpuspeed, int fps);	//in case of reinitialization
	
	bool isEndlessLoop = false;

public:
	void run();	//looper
	void update(); //all logic in here

	void start(char* title, int cpuspeed = CPU_SPEED, int fps = SCREEN_FPS); //start of emulation
	void debugMe();
	void optimizations(); //cycle optimizations
};