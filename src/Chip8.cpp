#include"Chip8.h"

void Chip8::start(char* title, int cpuspeed, int fps){
	//backup
	this->title = title;
	this->cpuspeed = cpuspeed;
	this->fps = fps;

	currentOpcode = 0;
	keyinput = 0;

	isEndlessLoop = false;

	mainwindow = new defaults();
	cpu = new CPU();
	memory = new Memory();
	input = new Input();
	video = new Video();
	audio = new Audio();
	fskip = new Frameskip();

	videoTimerInstance = new Timer();
	fskipTimerInstance = new Timer();
	delayTimerInstance = new Timer();
	windowTimerInstance = new Timer();
	fsbInstance = new Timer();

	cpu->init();
	memory->init(title);
	input->init();
	video->init(title, mainwindow);
	audio->init();
	audio->playAudio(); //test
	
	initSpeed(cpuspeed, fps);

	delayRegister = 0x0;

	keyinput = input->getKey();
	run();

}

void Chip8::initSpeed(int cpuspeed, int fps){
	fskip->init(cpuspeed, fps);
	videoTimerInstance->init(fskip->getVideoTimer());
	fskipTimerInstance->init(fskip->getFskipTimer());
	delayTimerInstance->init(fskip->getDelayTimer());
	windowTimerInstance->init(fskip->getWindowTimer());
	fsbInstance->init(fskip->getFsbTimer());
}

void Chip8::run(){
	running = true;
	while (running){
		update();
	}
	exit(0);
}


//always be cautious on this function - it needs to loop a million times
void Chip8::update(){

	//fetch
	previousOpcode = currentOpcode;
	currentOpcode = cpu->fetch(memory);

#ifdef DEBUG_ME
	//debugger
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
		//cycle optimizations - 120hz
		optimizations();
		
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
			if (keyinput == 0xff) running = false;	//shutdown emulator
		}

		
		//update internal timers
		delayTimerInstance->updateTimer(*fskip->getFsbTimer());
		windowTimerInstance->updateTimer(*fskip->getFsbTimer());
		fskipTimerInstance->updateTimer(*fskip->getFsbTimer());
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

void Chip8::optimizations(){
	//optimize endless loops
	if (previousOpcode == currentOpcode){
		static bool doOnce = false;
		if (doOnce == false){
			initSpeed(1, fps);
			isEndlessLoop = true;
			doOnce = true;
		}
	}

	//optimize three inst loops
	static int count = 0;
	static bool doOnce = false;
	if (((currentOpcode >> 12) == 0xf) && ((currentOpcode & 0x00ff) == 0x07)  && count == 0){
		count++;
	}
	else if (((currentOpcode >> 12) == 0x3) && count == 1){
		count++;
	}
	else if (((currentOpcode >> 12) == 0x1) && count == 2){
		count = 0;
		if (doOnce == false){
			initSpeed(1, fps);
			isEndlessLoop = true;
			doOnce = true;
		}
	}
	else{
		if (doOnce == true){
			initSpeed(cpuspeed, fps);
			isEndlessLoop = false;
			doOnce = false;

			count = 0;
			isEndlessLoop = false;
		}
	}
}

void Chip8::debugMe(){
	//debugger
	static int count = 0;
	static bool doOnce = false;
	if (!isEndlessLoop){
		doOnce = false;
		Debug::printDebug(*cpu->getProgramCounter(), *cpu->getStack(*cpu->getStackPointer() - 1), currentOpcode);
	}
	else{
		if (doOnce == false){
			printf("endless loop from here\n");
			doOnce = true;
		}
	}
}