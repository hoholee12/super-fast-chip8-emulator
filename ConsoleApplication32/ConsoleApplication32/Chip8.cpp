#include"Chip8.h"

void Chip8::start(char* title, int cpuspeed, int fps){
	this->title = title;


	//frameskip init
	screenFps = fps;
	cpuSpeed = cpuspeed;
	updateNewTimerSet();
	calculateSkip();

	currentOpcode = 0;
	keyinput = 0;

	mainwindow = new defaults();
	cpu = new CPU();
	memory = new Memory();
	input = new Input();
	video = new Video();
	audio = new Audio();

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
	videoTimerInstance->init(&screenTicksPerFrame);
	fskipTimerInstance->init(&backupTicksPerFrame);
	delayTimerInstance->init(&delayTimerPerFrame);
	windowTimerInstance->init(&windowTicksPerFrame);
	delayRegister = 0x0;

	keyinput = input->getKey();
	run();

	//start timer
	endTime();			//end timer
	calculateSkip();	//calculate
	startTime();		//next timer
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
	if (fskipTimerInstance->checkTimer())
		video->draw(mainwindow);	//draw


	//frameskip
	////check time before drawing next frame
	if (videoTimerInstance->checkTimer()){
		endTime();			//end timer
		calculateSkip();	//calculate
		videoDelay();		//delay
		startTime();		//next timer
	}


	//window
	if (windowTimerInstance->checkTimer()){
		mainwindow->updateTitle(title, cpuSpeed, backupFps, holdTick);
		if (keyinput == 0xff) running = false;
	}


	//update timers
	videoTimerInstance->updateTimer();
	fskipTimerInstance->updateTimer();
	delayTimerInstance->updateTimer();
	windowTimerInstance->updateTimer();
}


void Chip8::startTime(){
	prevTick = defaults::checkTime();
}

void Chip8::endTime(){
	currTick = defaults::checkTime() - prevTick;	//current tick checked here

	holdTick = screenDelayPerFrame - currTick;		//for original speed
	
}

void Chip8::videoDelay(){

	holdTick_fskip = backupDelayPerFrame - currTick;	//for frameskip speed

	
	if(holdTick > 0) defaults::delayTime(holdTick);
	

}

void Chip8::updateNewTimerSet(){

	screenTicksPerFrame = cpuSpeed / screenFps;	//cycles
	delayTimerPerFrame = cpuSpeed / timerSpeed;	//cycles
	screenDelayPerFrame = 1000 / screenFps;		//milliseconds
	windowTicksPerFrame = cpuSpeed / windowFps;	//cycles
}

void Chip8::calculateSkip(){
	
	//skip mechanism
	int tempTick = holdTick;
	if (holdTick < 0){
		tempTick *= -1; //convert to positive
		skipMultiplier = tempTick / screenDelayPerFrame + 1;
		backupFps = screenFps / (skipValue * skipMultiplier);
	}
	else backupFps = screenFps;

	if (backupFps == 0) backupFps = 1;		//prevent div by zero

	//printf("holdtick = %d holdtickfskip = %d skipmultiplier = %d\n", holdTick, holdTick_fskip, skipMultiplier);

	//apply skip value
	backupTicksPerFrame = cpuSpeed / backupFps;
	backupDelayPerFrame = 1000 / backupFps;		//milliseconds
	

}