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

	keyinput = input->checkKeyInput();
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

	startTime();

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



	//keyInput
	keyinput = input->checkKeyInput();

	//audio
	audio->audioProcess();

	//delay timer
	timer->cycleDelayTimer(cycleCount, delayTimerPerFrame);

	//video
	if (cycleCount % screenTicksPerFrame == 0){
		video->draw(mainwindow);
		videoDelay();
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


	//update title
	mainwindow->updateTitle(title, cpuSpeed, screenFps, holdTick);
}

void Chip8::updateNewTimerSet(){
	screenTicksPerFrame = cpuSpeed / screenFps;	//cycles
	delayTimerPerFrame = cpuSpeed / timerSpeed;	//cycles
	screenDelayPerFrame = 1000 / screenFps;		//milliseconds
}