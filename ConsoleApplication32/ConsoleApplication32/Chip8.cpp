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

	cycleCount = 0;

	mainwindow = new defaults();
	cpu = new CPU();
	memory = new Memory();
	input = new Input();
	video = new Video();
	audio = new Audio();
	timer = new Timer();

	cpu->init();
	memory->init(title);
	input->init();
	video->init(title, mainwindow);
	audio->init();
	audio->playAudio(); //test
	timer->init();

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
	controllerOp = cpu->decode(memory, timer, currentOpcode, keyinput);

	//controller
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
	if (keyinput == 0xff) running = false;


	//delay timer - with audio, input process
	if (timer->cycleDelayTimer(cycleCount, delayTimerPerFrame)){
		audio->audioProcess();
		input->checkKeyInput();
		//get stored keyinput value
		keyinput = input->getKey();
	}

	//video - loop based on frameskip value
	if (cycleCount % backupTicksPerFrame == 0)
		video->draw(mainwindow);	//draw



	//frameskip
	////check time before drawing next frame
	if (cycleCount % screenTicksPerFrame == 0){
		endTime();			//end timer
		calculateSkip();	//calculate
		videoDelay();		//delay
		startTime();		//next timer
	}


	//window
	if (cycleCount % windowTicksPerFrame == 0)
		mainwindow->updateTitle(title, cpuSpeed, backupFps, holdTick);

	cycleCount++;

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