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

//function type
typedef uint32_t(*func_ptr)(void);
//using func_ptr = uint32_t(*)(void);

class Dynarec{
	Cache* cache;
	Translator* translator;
	CPU* cpu;
	Video* video;
	Memory* memory;
	Audio* audio;

	bool running;
	uint16_t currentOpcode = 0;
	uint16_t previousOpcode;	//for some optimization

	bool isEndlessLoop = false;	//notify debugger that this is endless loop

	int whichInterpreter; //choose interpreter method

	bool drawFlag = false; //possible speed optimization
	
	
	
	//for executeBlock()
	//...similar ones on the translator.h are only for updateRecompiler()
	bool switchToInterpreter = false;
	bool hintFallback = false;
	bool delayNext = false;

	
	uint32_t baseClock;

	int64_t leftoverCycle = 0; //deals with negative numbers

	//cache mode switcher
	bool use_bCache = false;

	/*
	structure:

		loop until n cycle filled
			n cycles
				break off if cache exist
				recompile n opcodes
				break off for certain ops
			
			executeblock
				execute
				if fallback
					interpreter

		run other_peripherals
	
	*/



public:
	void init(CPU* cpu, Video* video, Memory* memory, Audio* audio, uint32_t baseClock){
		
		this->baseClock = baseClock;
		
		this->cpu = cpu;
		this->video = video;
		this->memory = memory;
		this->audio = audio;
		cache = new Cache();


		
		translator = new Translator(cpu,	//cpu variables
			(uint32_t)&switchToInterpreter,
			(uint32_t)&hintFallback,
			(uint32_t)&delayNext);	//core variables


		//precompile single opcodes
		precompiler();
	}


	void precompiler(){
		uint16_t pcTemp = cpu->programCounter;
		


		for (; cpu->programCounter < FULL_MEM_SIZE;){
			translator->init(cache->createCache(cpu->programCounter, true));
			//cache->getCache(cpu->programCounter, true)->TScache.resize(1);
			translator->startBlock();
			internalLoop(0, cpu->programCounter, true);

			// ensure block is closed
			if (!translator->checkEndDecode()) translator->endBlock();

		}

		//restore original pc
		cpu->programCounter = pcTemp;

	}

#ifdef DEBUG_ME
	uint16_t getCurrentOpcode(){
		return currentOpcode;
	}

#endif

	/*
	updateRecompiler
		:internalLoop
	-first time of jiffy
	-if cycle remains after break off, keep looping - dont let lowerhalf update
	-if cache exists, break off
	-breakoff when prev leftover cycle is done
	*/
	bool updateRecompiler(){

		//ready icache


		//backup pc for executeBlock
		uint16_t pcTemp = cpu->programCounter;

		/*
		*
		*
		*
		*******	if cache already exists:
		*
		*
		*
		*/

		//reset mode
		use_bCache = false;

		if (cache->checkCacheExists(cpu->programCounter)){

			if (leftoverCycle == 0){

				leftoverCycle = baseClock - cache->getOpcodeCount(cpu->programCounter);
			
			}
			else if ((leftoverCycle - cache->getOpcodeCount(cpu->programCounter)) >= 0){

				leftoverCycle -= cache->getOpcodeCount(cpu->programCounter);
				
			}
			else{

				//use bCache
				leftoverCycle -= cache->getOpcodeCount(cpu->programCounter, true);
				use_bCache = true;
			}


			return (leftoverCycle != 0);	//loop if cycle remaining
			
		}




		/*
		*
		*
		*
		*******	else make cache:
		*
		*
		*
		*/
		translator->init(cache->createCache(pcTemp));

		//update xyn for next opcode
		//cache->getCache(pcTemp)->TScache.resize(baseClock);
		
		translator->startBlock();

		//recompile n opcodes
		int i = 0;

		//no leftover
		if (leftoverCycle == 0){
			for (; i < baseClock && !translator->checkFallback() && !translator->checkEndDecode(); i++) internalLoop(i, pcTemp);

			//leftover cycle
			leftoverCycle = baseClock - i;

			//ensure block is closed
			if (!translator->checkEndDecode()) translator->endBlock();

			//restore original pc
			cpu->programCounter = pcTemp;




		}
		//yes leftover
		else{
			
			//leftover remaining
			for (; i < leftoverCycle && !translator->checkFallback() && !translator->checkEndDecode(); i++) internalLoop(i, pcTemp);

			//leftover cycle
			leftoverCycle -= i;

			//ensure block is closed
			if (!translator->checkEndDecode()) translator->endBlock();

			//restore original pc
			cpu->programCounter = pcTemp;




			//keep running until its completely filled
			if (leftoverCycle != 0) return true;
		}



		return false;
	}

	//one opcode into icache
	void internalLoop(int i, uint16_t pcTemp, bool flag = false){

		//fetch
		previousOpcode = currentOpcode;
		currentOpcode = cpu->fetch();

		TranslatorState TScache;
		TScache.x_val = (currentOpcode & 0x0f00) >> 8;
		TScache.y_val = (currentOpcode & 0x00f0) >> 4;
		TScache.nx = currentOpcode & 0x000F;
		TScache.nnx = currentOpcode & 0x00FF;
		TScache.nnnx = currentOpcode & 0x0FFF;

		/*
		cache->getCache(pcTemp, flag)->TScache[i].x_val = (currentOpcode & 0x0f00) >> 8;
		cache->getCache(pcTemp, flag)->TScache[i].y_val = (currentOpcode & 0x00f0) >> 4;
		cache->getCache(pcTemp, flag)->TScache[i].nx = currentOpcode & 0x000F;
		cache->getCache(pcTemp, flag)->TScache[i].nnx = currentOpcode & 0x00FF;
		cache->getCache(pcTemp, flag)->TScache[i].nnnx = currentOpcode & 0x0FFF;
		*/

		//cut one full jiffy
		//ret at the end
		if (i == baseClock - 1) translator->decode(&TScache, true);
		else translator->decode(&TScache);

		//one opcode into icache count
		cache->setOpcodeCount(pcTemp, cache->getOpcodeCount(pcTemp, flag) + 1, flag);

		//won't reach if translator decodes a fallback
		//next opcode
		cpu->programCounter += 2;
	}



	void executeBlock(){
		
		ICache* temp = cache->getCache(cpu->programCounter, use_bCache);
		
#ifdef NO
		cache->printCache(cpu->programCounter);
#endif


		//TODO
		cache->autoMarkExec(cpu->programCounter, use_bCache);

		//execute
		func_ptr executeFunc = (func_ptr)cache->getCache(cpu->programCounter, use_bCache)->execBlock;
		executeFunc();
		
		
		
		

		//if hintFallback flipped, continue to execute fallback
		if (!hintFallback) return;


		//delayNext for controllerOp

		//fetch
		previousOpcode = currentOpcode;
		currentOpcode = cpu->fetch();
		cpu->currentOpcode = currentOpcode;

		if(!delayNext) cpu->decode_jumboLUT();
		//controller
		if (cpu->controllerOp != ControllerOp::none)		//optimization
			switch (cpu->controllerOp){
			case ControllerOp::clearScreen:
				drawFlag = true;
				break;
			case ControllerOp::drawVideo:
				if (drawFlag){
					video->clearVBuffer();
					drawFlag = false;
				}
				video->copySprite(cpu, memory, video);
				break;
			case ControllerOp::setSoundTimer:
				audio->setSoundTimer(cpu);
				break;
		}

		cpu->controllerOp = ControllerOp::none;

		//reset
		switchToInterpreter = false;
		hintFallback = false;
		delayNext = false;
	}

};
