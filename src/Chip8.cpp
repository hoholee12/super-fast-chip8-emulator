#include"Chip8.h"

void Chip8::init(Status* stat){

	//invoke copy
	prev_imstat = *stat;

	//backup
	this->title = prev_imstat.get_post_title();
	this->cpuspeed = prev_imstat.get_post_cpuspeed();
	this->fps = prev_imstat.get_post_fps();
	this->whichInterpreter = prev_imstat.get_post_whichInterpreter();

	//create instances
	if(!mainwindow && !prev_imstat.get_reset()) mainwindow = new defaults();
	if(!cpu) cpu = new CPU();
	if(!memory) memory = new Memory();
	if(!input) input = new Input();
	if(!video && !prev_imstat.get_reset()) video = new Video();
	if(!audio) audio = new Audio();
	if(!fskip) fskip = new Frameskip();

	if(!dynarec) dynarec = new Dynarec();

	//init timers
	if(!videoTimerInstance) videoTimerInstance = new Timer();
	if(!fskipTimerInstance) fskipTimerInstance = new Timer();
	if(!delayTimerInstance) delayTimerInstance = new Timer();
	if(!windowTimerInstance) windowTimerInstance = new Timer();
	if(!fsbInstance) fsbInstance = new Timer();

	if(!scheduler) scheduler = new TimeSched();

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
	memory->init(title, prev_imstat.get_post_ignore());
	input->init();
	keyinput = input->getKey();
	if(!prev_imstat.get_reset()) video->init(title, mainwindow, &imstat, prev_imstat.get_post_flickerOffset());
	else{
		video->clearVBuffer();
	}
	audio->init();
	//test
	audio->playAudio();
	audio->pauseAudio();

	dynarec->init(cpu, video, memory, audio, scheduler->getBaseClock());


	//READY.
	printf("\nREADY.\n");

}

void Chip8::start(const char* title, bool ignore, int cpuspeed, int fps, int whichInterpreter, int flickerOffset){
	imstat.set_reset(false);
	imstat.set_post_title(title);
	imstat.set_post_ignore(ignore);
	imstat.set_post_cpuspeed(cpuspeed);
	imstat.set_post_fps(fps);
	imstat.set_post_whichInterpreter(whichInterpreter);
	imstat.set_post_flickerOffset(flickerOffset);
	
	init(&imstat);
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
	do{
		running = true;
		
		if(imstat.get_reset()){
			destroy();
			//configure imstat in defaults.h first!
			init(&imstat);
		}
		imstat.set_reset(false);
		switch (whichInterpreter){
		case 1:
			while (running){
				while (scheduler->baseLoop()){
					updateInterpreter_switch();
					update_controller();
	#ifdef DEBUG_ME
					debugMe();
	#endif
				}
				update_lowerhalf();
				if(imstat.get_reset()) running = false;
			}
			break;
		case 2:
			while (running){
				while (scheduler->baseLoop()){
					updateInterpreter_LUT();
					update_controller();
	#ifdef DEBUG_ME
					debugMe();
	#endif
				}
				update_lowerhalf();
				if(imstat.get_reset()) running = false;
			}
			break;
		case 3:
			while (running){
				while (scheduler->baseLoop()){
					updateInterpreter_jumboLUT();
					update_controller();
	#ifdef DEBUG_ME
					debugMe();
	#endif
				}
				update_lowerhalf();
				if(imstat.get_reset()) running = false;
			}
			break;
		case 4:
			while (running){
				
				dynarec->updateRecompiler();
				do{
					dynarec->executeBlock();	//cpu & controller
	#ifdef DEBUG_ME
					debugMe();
	#endif
				} while (dynarec->updateRecompiler());
				dynarec->executeBlock();	//cpu & controller
	#ifdef DEBUG_ME
				debugMe();
	#endif
				update_lowerhalf();
				if(imstat.get_reset()) running = false;
			}
			break;
		default:
			fprintf(stderr, "configuration error!\n");
		}

	}while(imstat.get_reset());
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

#ifdef DEBUG_ME
void Chip8::debugMe(){
	//debugger
	static int count = 0;
	static bool doOnce = false;
	if (!isEndlessLoop){
		doOnce = false;
		debug->printDebug(cpu->getV(0x0), *cpu->getProgramCounter(), cpu->getStack(0x0), *cpu->getStackPointer(), cpu->currentOpcode, mainwindow, memory, audio, delayRegister);
	}
	else{
		if (doOnce == false){
			printf("endless loop from here\n");
			doOnce = true;
		}
	}
}
#endif