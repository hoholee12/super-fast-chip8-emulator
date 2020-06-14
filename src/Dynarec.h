#pragma once

/*
	Dynarec main core

*/

#include"Translator.h"
#include"Cache.h"
#include"Video.h"
#include"Memory.h"
#include"Audio.h"


//replace updateInterpreter_??()

class Dynarec{
	Cache* cache;
	Translator* translator;
	CPU* cpu;
	Video* video;
	Memory* memory;
	Audio* audio;

	bool running;
	uint16_t currentOpcode;
	uint16_t previousOpcode;	//for some optimization

	ControllerOp controllerOp;	//after cpu processes its stuff, next is chip8 controller output job
	bool isEndlessLoop = false;	//notify debugger that this is endless loop

	int whichInterpreter; //choose interpreter method

	bool drawFlag = false; //possible speed optimization

	bool switchToInterpreter = false;
	bool hintFallback = false;
	uint32_t x_val;
	uint32_t y_val;

	//scheduling arrays
	uint16_t* itpArr;
	using vect8ptr = std::vector<vect8*>;
	vect8ptr blockArr;
	uint32_t itpIterator; uint32_t blockIterator = 0;
	


	/*
		TODO:

		8 cycles
			recompile 1 opcode
				put block arr(1~) translator.h
			if fallback
				put fallbackhint opcode arr(0~) cpu.h
				new block arr(2~) translator.h
			if jmp
				executeblock
				run jmp opcode cpu.h
	
		executeblock
			for loop until last block
				execute block arr(1~) translator.h
				run fallback interpreter opcode(0~) cpu.h
			init all arr
	
	*/



public:
	void init(CPU* cpu, Video* video, Memory* memory, Audio* audio, uint32_t baseClock){
		itpArr = new uint16_t(baseClock);
		blockArr.resize(baseClock);

		
		this->cpu = cpu;
		this->video = video;
		this->memory = memory;
		this->audio = audio;
		cache = new Cache();
		translator = new Translator(cpu, (uint32_t)&switchToInterpreter, (uint32_t)&hintFallback, (uint32_t)&x_val, (uint32_t)&y_val);
	}







	void updateRecompiler(){

		//fetch
		previousOpcode = currentOpcode;
		currentOpcode = cpu->fetch();

		//for jit stuff
		x_val = (currentOpcode & 0x0f00) >> 8;
		y_val = (currentOpcode & 0x00f0) >> 4;


		if (cache->checkCacheExists(cpu->programCounter)) return;
		translator->init(cache->createCache(cpu->programCounter));

		


		
#ifdef DEBUG_ME
		//debugger
		printf("delayReg = %x,\t", delayRegister);
		debugMe();
#endif
		translator->decode();
		cpu->decode();
		//decode
		//controllerOp = cpu->decode();
	}






	void executeBlock(){


		//controller
		if (controllerOp != ControllerOp::none)		//optimization
			switch (controllerOp){
			case ControllerOp::clearScreen:
				drawFlag = true;
				break;
			case ControllerOp::drawVideo:
				if (drawFlag){
					video->clearVBuffer();
					drawFlag = false;
				}
				video->copySprite(currentOpcode, cpu, memory);
				break;
			case ControllerOp::setSoundTimer:
				audio->setSoundTimer(currentOpcode, cpu);
				break;
		}
		
	}

};