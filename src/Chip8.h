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



class Chip8 final:public defaults, Debug{
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


//experimental loop inlining
//always be cautious on this function - it needs to loop a million times
inline void Chip8::updateInterpreter_switch(){
	controllerOp = 0x0; //safe measure

	//fetch
	previousOpcode = currentOpcode;
	currentOpcode = cpu->fetch(memory);

#ifdef DEBUG_ME
	//debugger
	printf("delayReg = %x,\t", delayRegister);
	debugMe();

#endif

	//decode
	controllerOp = cpu->decode(memory, &delayRegister, currentOpcode, keyinput);

	//controller
	if (controllerOp != 0x0)		//optimization
		switch (controllerOp){
		case 0x1:
			video->clearVBuffer();
			break;
		case 0x2:
			video->copySprite(currentOpcode, cpu, memory);
			break;
		case 0x3:
			audio->setSoundTimer(currentOpcode, cpu);
			break;
	}

	//120hz for extra cycle optimization
	if (fsbInstance->checkTimer()){

		//delay timer - 60hz
		if (delayTimerInstance->checkTimer()){
			if (delayRegister > 0x0) delayRegister--;

			//audio - 60hz
			audio->audioProcess();

			//input - 60hz?
			input->checkKeyInput();
			keyinput = input->getKey(); //keyinput maybe needed for other instances
		}

		//video - loop based on fskip hz value, may skip a bit more if indivisable
		if (fskipTimerInstance->checkTimer()){
			video->draw(mainwindow);	//draw

		}

		//window - 1hz
		if (windowTimerInstance->checkTimer()){
			mainwindow->updateTitle(title, fskip->getCpuSpeed(), fskip->getBackupFps(), fskip->getHoldTick());
		}

		if (keyinput == 0xff) running = false;	//shutdown emulator

		//cycle optimizations - 120hz
		optimizations();
		useSpeedHack(); //quite buggy atm

		//update internal timers
		delayTimerInstance->updateTimer();
		windowTimerInstance->updateTimer();
		fskipTimerInstance->updateTimer();
	}

	//frameskip
	////user framerate most of the time indivisable by 60hz
	if (videoTimerInstance->checkTimer()){

		fskip->endTime();			//end timer
		fskip->calculateSkip();		//calculate
		fskip->videoDelay();		//delay

		fskip->startTime();			//next timer


	}

	//update timers
	fsbInstance->updateTimer();
	videoTimerInstance->updateTimer();

}
//always be cautious on this function - it needs to loop a million times
inline void Chip8::updateInterpreter_LUT(){
	controllerOp = 0x0; //safe measure

	//fetch
	previousOpcode = currentOpcode;
	currentOpcode = cpu->fetch();

#ifdef DEBUG_ME
	//debugger
	printf("delayReg = %x,\t", delayRegister);
	debugMe();
	
#endif

	//decode
	controllerOp = cpu->decode();

	//controller
	if (controllerOp != 0x0)		//optimization
		switch (controllerOp){
		case 0x1:
			video->clearVBuffer();
			break;
		case 0x2:
			video->copySprite(currentOpcode, cpu, memory);
			break;
		case 0x3:
			audio->setSoundTimer(currentOpcode, cpu);
			break;
		}

	//120hz for extra cycle optimization
	if (fsbInstance->checkTimer()){
		
		//delay timer - 60hz
		if (delayTimerInstance->checkTimer()){
			if (delayRegister > 0x0) delayRegister--;

			//audio - 60hz
			audio->audioProcess();

			//input - 60hz?
			input->checkKeyInput();
			keyinput = input->getKey(); //keyinput maybe needed for other instances
		}

		//video - loop based on fskip hz value, may skip a bit more if indivisable
		if (fskipTimerInstance->checkTimer()){
			video->draw(mainwindow);	//draw

		}

		//window - 1hz
		if (windowTimerInstance->checkTimer()){
			mainwindow->updateTitle(title, fskip->getCpuSpeed(), fskip->getBackupFps(), fskip->getHoldTick());
		}

		if (keyinput == 0xff) running = false;	//shutdown emulator

		//cycle optimizations - 120hz
		optimizations();
		useSpeedHack(); //quite buggy atm
		
		//update internal timers
		delayTimerInstance->updateTimer();
		windowTimerInstance->updateTimer();
		fskipTimerInstance->updateTimer();
	}

	//frameskip
	////user framerate most of the time indivisable by 60hz
	if (videoTimerInstance->checkTimer()){

		fskip->endTime();			//end timer
		fskip->calculateSkip();		//calculate
		fskip->videoDelay();		//delay
		
		fskip->startTime();			//next timer
		

	}
	
	//update timers
	fsbInstance->updateTimer();
	videoTimerInstance->updateTimer();

}

//always be cautious on this function - it needs to loop a million times
inline void Chip8::updateInterpreter_jumboLUT(){
	controllerOp = 0x0; //safe measure

	//fetch
	previousOpcode = currentOpcode;
	currentOpcode = cpu->fetch();

#ifdef DEBUG_ME
	//debugger
	printf("delayReg = %x,\t", delayRegister);
	debugMe();

#endif

	//decode
	controllerOp = cpu->decode_jumboLUT();

	//controller
	if (controllerOp != 0x0)		//optimization
		switch (controllerOp){
		case 0x1:
			video->clearVBuffer();
			break;
		case 0x2:
			video->copySprite(currentOpcode, cpu, memory);
			break;
		case 0x3:
			audio->setSoundTimer(currentOpcode, cpu);
			break;
	}

	//120hz for extra cycle optimization
	if (fsbInstance->checkTimer()){

		//delay timer - 60hz
		if (delayTimerInstance->checkTimer()){
			if (delayRegister > 0x0) delayRegister--;

			//audio - 60hz
			audio->audioProcess();

			//input - 60hz?
			input->checkKeyInput();
			keyinput = input->getKey(); //keyinput maybe needed for other instances
		}

		//video - loop based on fskip hz value, may skip a bit more if indivisable
		if (fskipTimerInstance->checkTimer()){
			video->draw(mainwindow);	//draw

		}

		//window - 1hz
		if (windowTimerInstance->checkTimer()){
			mainwindow->updateTitle(title, fskip->getCpuSpeed(), fskip->getBackupFps(), fskip->getHoldTick());
		}

		if (keyinput == 0xff) running = false;	//shutdown emulator

		//cycle optimizations - 120hz
		optimizations();
		useSpeedHack(); //quite buggy atm

		//update internal timers
		delayTimerInstance->updateTimer();
		windowTimerInstance->updateTimer();
		fskipTimerInstance->updateTimer();
	}

	//frameskip
	////user framerate most of the time indivisable by 60hz
	if (videoTimerInstance->checkTimer()){

		fskip->endTime();			//end timer
		fskip->calculateSkip();		//calculate
		fskip->videoDelay();		//delay

		fskip->startTime();			//next timer


	}

	//update timers
	fsbInstance->updateTimer();
	videoTimerInstance->updateTimer();

}

