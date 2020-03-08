#pragma once
#include"CPU.h"
#include"Memory.h"
#include"Input.h"
#include"Video.h"
#include"Audio.h"

#include"defaults.h"

class Chip8{
private:
	CPU* cpu;
	Memory* memory;
	Input* input;
	Video* video;
	Audio* audio;

	bool running;
	uint16 currentOpcode;
	uint16 controllerOp;	//after cpu processes its stuff, next is chip8 controller output job

	uint8 keyinput;

public:
	void run();	//looper
	void update(); //all logic in here


	void start(char* str); //start of emulation

};