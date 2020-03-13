#include"Chip8.h"

void Chip8::start(char* title, int cpuspeed, int fps){
	this->title = title;

	currentOpcode = 0;
	keyinput = 0;

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

	cpu->init();
	memory->init(title);
	input->init();
	video->init(title, mainwindow);
	audio->init();
	audio->playAudio(); //test
	
	fskip->init(cpuspeed, fps);
	videoTimerInstance->init(fskip->getVideoTimer());
	fskipTimerInstance->init(fskip->getFskipTimer());
	delayTimerInstance->init(fskip->getDelayTimer());
	windowTimerInstance->init(fskip->getWindowTimer());

	

	delayRegister = 0x0;

	keyinput = input->getKey();
	run();

	//start timer
	fskip->endTime();		//end timer
	fskip->calculateSkip();	//calculate
	fskip->startTime();		//next timer
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
	currentOpcode = cpu->fetch(memory);

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

	//delay timer - with audio, input process
	if (delayTimerInstance->checkTimer()){
		if (delayRegister > 0x0) delayRegister--;
		audio->audioProcess();
		input->checkKeyInput();
		//get stored keyinput value
		keyinput = input->getKey();
	}

	//video - loop based on frameskip value
	if (fskipTimerInstance->checkTimer()){
		video->draw(mainwindow);	//draw
		
	}

	//frameskip
	////check time before drawing next frame
	if (videoTimerInstance->checkTimer()){
		fskip->endTime();			//end timer
		fskip->calculateSkip();		//calculate
		fskip->videoDelay();		//delay
		
		fskip->startTime();			//next timer
	}


	//window
	if (windowTimerInstance->checkTimer()){
		mainwindow->updateTitle(title, fskip->getCpuSpeed(), fskip->getBackupFps(), fskip->getHoldTick());
		if (keyinput == 0xff) running = false;
	}


	//update timers
	videoTimerInstance->updateTimer();
	fskipTimerInstance->updateTimer();
	delayTimerInstance->updateTimer();
	windowTimerInstance->updateTimer();
}

