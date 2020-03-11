#include"Chip8.h"

void Chip8::start(char* str){
	currentOpcode = 0;
	keyinput = 0;

	cpu = new CPU();
	memory = new Memory();
	input = new Input();
	video = new Video();
	audio = new Audio();

	cpu->init();
	memory->init(str);
	input->init();
	video->init(str);
	audio->init();
	audio->playAudio(); //test

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

	defaults::startTime();

	//fetch
	currentOpcode = cpu->fetch(memory);
	
	
	//decode
	controllerOp = cpu->decode(memory, currentOpcode, keyinput);

	//keyInput
	keyinput = input->checkKeyInput();

	//audio
	audio->audioProcess();

	//video - we now have frameskip
	video->draw();

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

	defaults::endTime();
	
}

