#pragma once

/*
	Dynarec main core

*/

#include"Translator.h"
#include"Cache.h"

//replace updateInterpreter_??()

class Dynarec{
	Cache* cache;
	Translator* translator;
	CPU* cpu;

	bool running;
	uint16_t currentOpcode;
	uint16_t previousOpcode;	//for some optimization

	ControllerOp controllerOp;	//after cpu processes its stuff, next is chip8 controller output job


	uint32_t cycle = 0;

	void init(CPU* cpu){
		this->cpu = cpu;
		cache = new Cache();
		translator = new Translator(cpu);
		cycle = 0;
	}

	void updateRecompiler(){
		
		if (cycle == 0){
			if (cache->checkCacheExists(cpu->programCounter)) return;
			translator->init(cache->createCache(cpu->programCounter));
		}
		cycle++;



		controllerOp = ControllerOp::none; //safe measure

		//fetch
		previousOpcode = currentOpcode;
		currentOpcode = cpu->fetch();
		

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

	void finishLoop(){
		cycle = 0;
	}

};