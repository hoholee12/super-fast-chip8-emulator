#include"Chip8.h"

void Chip8::start(char* str){
	currentOpcode = 0;
	keyinput = 0;

	cpu = new CPU();
	memory = new Memory();
	input = new Input();
	video = new Video();

	cpu->init();
	memory->init(str);
	input->init();
	video->init(str);

	run();

}

void Chip8::run(){
	running = true;
	while (running){
		update();
	}
	SDL_Quit();
}


void Chip8::update(){

	//fetch
	currentOpcode = cpu->fetch(memory);
	
	
	//decode
	controllerOp = cpu->decode(memory, currentOpcode, keyinput);

	//keyInput
	keyinput = input->checkKeyInput();


	//controller job
	switch (controllerOp){
	case 0x1:
		video->clearVBuffer();
		video->draw();
		break;
	case 0x2:
		video->copySprite(currentOpcode, cpu, memory);
		video->draw();
		break;
	}
	if (keyinput == 0xff) running = false;


	//SDL_Delay(1000 / 60); //60fps

}

