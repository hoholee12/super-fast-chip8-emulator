#include"Chip8.h"

void Chip8::start(char* title, int cpuspeed, int fps){
	this->title = title;

	screenFps = fps;
	cpuSpeed = cpuspeed;
	updateNewTimerSet();


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
	startTime();
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



	//video
	////check time just before drawing next frame
	if (cycleCount % screenTicksPerFrame == 0){
		video->draw(mainwindow);
		videoDelay();			//sets skipFlag
		updateNewTimerSet();	//frameskipper

		startTime();			//next timer
	}

	//window
	if (cycleCount % windowTicksPerFrame == 0)
		mainwindow->updateTitle(title, cpuSpeed, backupFps, holdTick);

	cycleCount++;

}


void Chip8::startTime(){
	prevTick = defaults::checkTime();
}

void Chip8::videoDelay(){
	uint32 currTick = defaults::checkTime() - prevTick;

	holdTick = screenDelayPerFrame - currTick;
	skipFlag = false;
	if (holdTick > 0) defaults::delayTime(holdTick);
	else {
		//static int test = 0;
		//printf("delay skipped: %d times!\n", ++test);
		skipFlag = true;	//halve next framerate
	}

}

void Chip8::updateNewTimerSet(){

	//skip mechanism
	if (skipFlag == true){
		backupFps /= skipValue;
	}
	else if (backupFps < screenFps){
		backupFps *= skipValue;
	}
	else backupFps = screenFps;

	screenTicksPerFrame = cpuSpeed / backupFps;	//cycles
	delayTimerPerFrame = cpuSpeed / timerSpeed;	//cycles
	screenDelayPerFrame = 1000 / backupFps;		//milliseconds
	windowTicksPerFrame = cpuSpeed / windowFps;	//cycles
}