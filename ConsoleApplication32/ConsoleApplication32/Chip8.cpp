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

}

void Chip8::run(){
	running = true;
	while (running){
		update();
	}
	exit(0);
}


void Chip8::update(){

	////check time on first frame
	if (cycleCount % screenTicksPerFrame == 0) startTime();

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
	if (cycleCount % screenTicksPerFrame == screenTicksPerFrame - 1){
		video->draw(mainwindow);
		videoDelay();
		mainwindow->updateTitle(title, cpuSpeed, screenFps, holdTick);
	}

	cycleCount++;

}


void Chip8::startTime(){
	prevTick = defaults::checkTime();
}

void Chip8::videoDelay(){
	uint32 currTick = defaults::checkTime() - prevTick;

	holdTick = screenDelayPerFrame - currTick;
	if (holdTick > 0) defaults::delayTime(holdTick);
	else {
		static int test = 0;
		printf("delay skipped: %d times!\n", ++test);
	}

}

void Chip8::updateNewTimerSet(){
	screenTicksPerFrame = cpuSpeed / screenFps;	//cycles
	delayTimerPerFrame = cpuSpeed / timerSpeed;	//cycles
	screenDelayPerFrame = 1000 / screenFps;		//milliseconds
}