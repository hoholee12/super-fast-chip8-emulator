#include"Chip8.h"

void Chip8::start(char* title, int cpuspeed, int fps, int whichInterpreter, int flickerOffset){
	//backup
	this->title = title;
	this->cpuspeed = cpuspeed;
	this->fps = fps;

	//create instances
	mainwindow = new defaults();
	cpu = new CPU();
	memory = new Memory();
	input = new Input();
	video = new Video();
	audio = new Audio();
	fskip = new Frameskip();

	dynarec = new Dynarec();

	//init timers
	videoTimerInstance = new Timer();
	fskipTimerInstance = new Timer();
	delayTimerInstance = new Timer();
	windowTimerInstance = new Timer();
	fsbInstance = new Timer();

	scheduler = new TimeSched();

	initSpeed(cpuspeed, fps);
	//must be added in order!!
	scheduler->addTimeQueue(delayTimerInstance);
	scheduler->addTimeQueue(fskipTimerInstance);
	scheduler->addTimeQueue(windowTimerInstance);
	scheduler->addTimeQueue(fsbInstance);
	scheduler->addTimeQueue(videoTimerInstance);
	
	scheduler->initSched();

	//init components
	currentOpcode = 0;
	keyinput = 0;
	isEndlessLoop = false;
	delayRegister = 0x0;
	cpu->init(memory, &delayRegister, &keyinput);
	memory->init(title);
	input->init();
	keyinput = input->getKey();
	video->init(title, mainwindow, flickerOffset);
	audio->init();
	audio->playAudio(); //test

	dynarec->init(cpu, video, memory, audio, scheduler->getBaseClock());

	this->whichInterpreter = whichInterpreter;


	//READY.
	printf("\nREADY.\n");

	run();	//start looping!!!

}

void Chip8::initSpeed(int cpuspeed, int fps){
	//init fskip
	fskip->init(cpuspeed, fps);
	/*
	//main timers
	videoTimerInstance->init(fskip->getVideoTimer());
	fsbInstance->init(fskip->getFsbTimer());
	
	//timers inside fsb timer
	fskipTimerInstance->init(fskip->getFskipTimer(), fskip->getFsbTimer());
	delayTimerInstance->init(fskip->getDelayTimer(), fskip->getFsbTimer());
	windowTimerInstance->init(fskip->getWindowTimer(), fskip->getFsbTimer());
	*/

	//for scheduler
	videoTimerInstance->init(fskip->getVideoTimer());
	fsbInstance->init(fskip->getFsbTimer());
	fskipTimerInstance->init(fskip->getFskipTimer());
	delayTimerInstance->init(fskip->getDelayTimer());
	windowTimerInstance->init(fskip->getWindowTimer());

	scheduler->reinitSched();


}

void Chip8::run(){
	running = true;
	switch (whichInterpreter){
	case 1:
		while (running){
			while (scheduler->baseLoop()){
				updateInterpreter_switch();
				update_controller();
			}
			update_lowerhalf();
		}
		break;
	case 2:
		while (running){
			while (scheduler->baseLoop()){
				updateInterpreter_LUT();
				update_controller();
			}
			update_lowerhalf();
		}
		break;
	case 3:
		while (running){
			while (scheduler->baseLoop()){
				updateInterpreter_jumboLUT();
				update_controller();
			}
			update_lowerhalf();
		}
		break;
	case 4:
		while (running){
			
			dynarec->updateRecompiler();
			do{
				dynarec->executeBlock();	//cpu & controller
#ifdef DEBUG_ME
				//debugger
				printf("delayReg = %x,\t", delayRegister);
				debugMe();
#endif
			} while (dynarec->updateRecompiler());
			dynarec->executeBlock();	//cpu & controller

#ifdef DEBUG_ME
			//debugger
			printf("delayReg = %x,\t", delayRegister);
			debugMe();
			printf("run lowerhalf!!\n");
#endif
			
			update_lowerhalf();
		}
	default:
		fprintf(stderr, "configuration error!\n");
	}
	exit(0);
}


// -1: neutral, 0: low, 0<: high
void Chip8::useSpeedHack(){
	if (speedHack > 0){
		initSpeed(speedHack, fps);
		speedHack = -1;	//neutral
	}
	else if (speedHack == 0){
		initSpeed(cpuspeed, fps);
		speedHack = -1;	//neutral
	}
}

//try to make codeblocks so as to not interfere with one another
void Chip8::optimizations(){

	//optimize endless loops(jump)
	{
		if (((currentOpcode >> 12) == 0x1) && previousOpcode == currentOpcode){
			static bool doOnce = false;
			if (doOnce == false){
				speedHack = 1;
				isEndlessLoop = true;
				doOnce = true;
			}
		}
	}

	//optimize three inst loops(delay, check, jump)
	{
		static int count = 0;
		static bool doOnce = false;
		if (((currentOpcode >> 12) == 0xf) && ((currentOpcode & 0x00ff) == 0x07) && count == 0){
			count++;
		}
		else if (((currentOpcode >> 12) == 0x3) && count == 1){
			count++;
		}
		else if (((currentOpcode >> 12) == 0x1) && count == 2){
			count = 0;
			if (doOnce == false){
				speedHack = 1;
				isEndlessLoop = true;
				doOnce = true;
			}
		}
		else{
			if (doOnce == true){
				speedHack = 0;
				isEndlessLoop = false;
				doOnce = false;

				count = 0;
			}
		}
	}

	
}

void Chip8::debugMe(){
	//debugger
	static int count = 0;
	static bool doOnce = false;
	if (!isEndlessLoop){
		doOnce = false;
		//universal stuff to debug here
		printf("indexRegister = %x\n", *cpu->getIndexRegister());

		Debug::printDebug(cpu->v, *cpu->getProgramCounter(), *cpu->getStack(*cpu->getStackPointer() - 1), currentOpcode, mainwindow);
	}
	else{
		if (doOnce == false){
			printf("endless loop from here\n");
			doOnce = true;
		}
	}
}
