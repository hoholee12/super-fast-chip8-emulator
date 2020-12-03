#pragma once
#include"defaults.h"

#include"Dynarec.h"
#include"CPU.h"
#include"Memory.h"
#include"Input.h"
#include"Video.h"
#include"Audio.h"
#include"Timer.h"
#include"TimeSched.h"
#include"Frameskip.h"
#include"Debug.h"

class Chip8: public defaults{
private:
	Status imstat;	//temp
	Status prev_imstat;	//current

	const char* title;
	int cpuspeed;
	int fps;
	defaults* mainwindow;

	CPU* cpu;
	Memory* memory;
	Input* input;
	Video* video;
	Audio* audio;
	Frameskip* fskip;

	Dynarec* dynarec;

	bool running;
	uint16_t currentOpcode;
	uint16_t previousOpcode;	//for some optimization
	
	ControllerOp controllerOp;	//after cpu processes its stuff, next is chip8 controller output job

	uint8_t keyinput;
	uint8_t delayRegister; //delay register

	Timer* fsbInstance;
	Timer* videoTimerInstance;
	Timer* fskipTimerInstance;
	Timer* delayTimerInstance;
	Timer* windowTimerInstance;

	TimeSched* scheduler;

	Debug* debug = new Debug();

	int speedHack = -1; // -1: neutral, 0: low, 0<: high

	void initSpeed(int cpuspeed, int fps);	//in case of reinitialization

	void useSpeedHack();
	
	bool isEndlessLoop = false;	//notify debugger that this is endless loop

	int whichInterpreter; //choose interpreter method

	bool drawFlag = false; //possible speed optimization

public:

	Chip8(){
		mainwindow = NULL;
		cpu = NULL;
		memory = NULL;
		input = NULL;
		video = NULL;
		audio = NULL;
		fskip = NULL;

		dynarec = NULL;

		//init timers
		videoTimerInstance = NULL;
		fskipTimerInstance = NULL;
		delayTimerInstance = NULL;
		windowTimerInstance = NULL;
		fsbInstance = NULL;

		scheduler = NULL;

	}
	void run();	//looper
	void updateInterpreter_switch(); //all logic in here
	void updateInterpreter_LUT(); //all logic in here
	void updateInterpreter_jumboLUT(); //all logic in here
	void update_controller();
	void update_lowerhalf();

	void init(Status* stat); //start of emulation
	void start(const char* title, bool ignore = false, int cpuspeed = CPU_SPEED, int fps = SCREEN_FPS, int whichInterpreter = 1, int flickerOffset = 0); //start of emulation
	void destroy(){
		//create instances
		//delete mainwindow;
		delete cpu;
		delete memory;
		delete input;
		//delete video;
		delete audio;
		delete fskip;

		delete dynarec;

		//init timers
		delete videoTimerInstance;
		delete fskipTimerInstance;
		delete delayTimerInstance;
		delete windowTimerInstance;
		delete fsbInstance;

		delete scheduler;

		//mainwindow = NULL;
		cpu = NULL;
		memory = NULL;
		input = NULL;
		//video = NULL;
		audio = NULL;
		fskip = NULL;

		dynarec = NULL;

		//init timers
		videoTimerInstance = NULL;
		fskipTimerInstance = NULL;
		delayTimerInstance = NULL;
		windowTimerInstance = NULL;
		fsbInstance = NULL;

		scheduler = NULL;
	}
	void debugMe();
	void optimizations(); //cycle optimizations


};


//experimental loop inlining
//always be cautious on this function - it needs to loop a million times
inline void Chip8::updateInterpreter_switch(){
	controllerOp = ControllerOp::none; //safe measure

	//fetch
	previousOpcode = currentOpcode;
	currentOpcode = cpu->fetch(memory);
	cpu->currentOpcode = currentOpcode;

	//decode
	controllerOp = cpu->decode(memory, &delayRegister, currentOpcode, keyinput);


}
//always be cautious on this function - it needs to loop a million times
inline void Chip8::updateInterpreter_LUT(){
	controllerOp = ControllerOp::none; //safe measure

	//fetch
	previousOpcode = currentOpcode;
	currentOpcode = cpu->fetch();
	cpu->currentOpcode = currentOpcode;

	//decode
	controllerOp = cpu->decode();


}

//always be cautious on this function - it needs to loop a million times
inline void Chip8::updateInterpreter_jumboLUT(){
	controllerOp = ControllerOp::none; //safe measure

	//fetch
	previousOpcode = currentOpcode;
	currentOpcode = cpu->fetch();
	cpu->currentOpcode = currentOpcode;

	//decode
	controllerOp = cpu->decode_jumboLUT();

}

inline void Chip8::update_controller(){
	//controller
	if (controllerOp != ControllerOp::none)		//optimization
		switch (controllerOp){
		case ControllerOp::clearScreen:
			drawFlag = true;
			break;
		case ControllerOp::drawVideo:
			if (drawFlag){
				video->clearVBuffer();
				drawFlag = false;
			}
			video->copySprite(cpu, memory, video);
			break;
		case ControllerOp::setSoundTimer:
			audio->setSoundTimer(cpu);
			break;
	}
}

//rest of the logic
inline void Chip8::update_lowerhalf(){

	//delay timer - 60hz
	if (scheduler->subCheck()){

		if (delayRegister > 0x0) delayRegister--;

		//audio - 60hz
		audio->audioProcess();

		//input - 60hz?
		input->checkKeyInput();
		keyinput = input->getKey(); //keyinput maybe needed for other instances
	}

	//video - loop based on fskip hz value, may skip a bit more if indivisable
	if (scheduler->subCheck()){

		if (isEndlessLoop) video->forceFlush(); //force flush if endlessloop
		video->draw(mainwindow);	//draw
	}

	//window - 1hz
	if (scheduler->subCheck()){

		mainwindow->updateTitle(title, fskip->getCpuSpeed(), fskip->getBackupFps(), fskip->getHoldTick());
	}

	

	//120hz for extra cycle optimization
	if (scheduler->subCheck()){

		//cycle optimizations - 120hz
		optimizations();
		//useSpeedHack(); //quite buggy atm
	}



	//frameskip
	////user framerate most of the time indivisable by 60hz
	if (scheduler->subCheck()){


		fskip->endTime();			//end timer
		fskip->calculateSkip();		//calculate
		fskip->videoDelay();		//delay

		fskip->startTime();			//next timer


	}


	if (keyinput == 0xff){
#ifdef DEBUG_ME
		debug->closeDebug();
#endif
		running = false;
	}// running = false;	//shutdown emulator

}
