#pragma once

//windows.h header MUST BE DECLARED FIRST!!
#ifdef _WIN32
#include <windows.h>
#elif __LINUX__
#include <sys/mman.h>
#endif

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

	bool isEndlessLoop = false;	//notify debugger that this is endless loop

	int whichInterpreter; //choose interpreter method

	bool drawFlag = false; //possible speed optimization
	
	
	
	//for executeBlock()
	//...similar ones on the translator.h are only for updateRecompiler()
	bool switchToInterpreter = false;
	bool hintFallback = false;
	bool delayNext = false;

	//baseClock is max size of stateArr
	std::vector<TranslatorState> stateArr;
	uint32_t stateArrPtr = 0;
	
	uint32_t baseClock;

	uint32_t leftoverCycle = 0;

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


		//determine all states for one jiffy
		stateArr.resize(baseClock);


		
		translator = new Translator(cpu,	//cpu variables
			(uint32_t)&switchToInterpreter,
			(uint32_t)&hintFallback,
			(uint32_t)&delayNext);	//core variables
	}






	/*
	updateRecompiler
		:internalLoop
	-first time of jiffy
	-if cycle remains after break off, keep looping - dont let lowerhalf update
	-if cache exists, break off
	-breakoff when prev leftover cycle is done
	*/
	bool updateRecompiler(){
#ifdef DEBUG_ME
		printf("==========================================================\nleftover cycle: %d\n", leftoverCycle);
#endif
		//ready icache

		//if cache already exists
		if (cache->checkCacheExists(cpu->programCounter)){
			leftoverCycle = baseClock - cache->getOpcodeCount(cpu->programCounter);
			return leftoverCycle != 0;	//to the next leftoverCycle
		}

		//else make cache
		translator->init(cache->createCache(cpu->programCounter));

		//backup pc for executeBlock
		uint16_t pcTemp = cpu->programCounter;

		//recompile n opcodes
		int i = 0;

		//no leftover
		if (leftoverCycle == 0){
			for (; i < baseClock && !translator->checkFallback() && !translator->checkEndDecode(); i++) internalLoop(i);

			//leftover cycle
			leftoverCycle = baseClock - i;

			//ensure block is closed
			if (!translator->checkEndDecode()) translator->endBlock();

			//restore original pc
			cpu->programCounter = pcTemp;

			//if everything is filled
#ifdef DEBUG_ME
			cache->printCache(pcTemp);

#endif


		}
		//yes leftover
		else{
			
			//leftover remaining
			for (; i < leftoverCycle && !translator->checkFallback() && !translator->checkEndDecode(); i++) internalLoop(i);

			//leftover cycle
			leftoverCycle -= i;

			//ensure block is closed
			if (!translator->checkEndDecode()) translator->endBlock();

			//restore original pc
			cpu->programCounter = pcTemp;

			//if everything is filled
#ifdef DEBUG_ME
			cache->printCache(pcTemp);

#endif


			//keep running until its completely filled
			if (leftoverCycle != 0) return true;
		}



		return false;
	}

	//one opcode into icache
	void internalLoop(int i){

		//fetch
		previousOpcode = currentOpcode;
		currentOpcode = cpu->fetch();


		//update xyn for next opcode
		stateArr[i].x_val = (currentOpcode & 0x0f00) >> 8;
		stateArr[i].y_val = (currentOpcode & 0x00f0) >> 4;
		stateArr[i].nx = currentOpcode & 0x000F;
		stateArr[i].nnx = currentOpcode & 0x00FF;
		stateArr[i].nnnx = currentOpcode & 0x0FFF;

		//cut one full jiffy
		//ret at the end
		if (i == baseClock - 1) translator->decode(&stateArr[i], true);
		else translator->decode(&stateArr[i]);

		//one opcode into icache count
		cache->setOpcodeCount(cpu->programCounter, cache->getOpcodeCount(cpu->programCounter) + 1);

		//won't reach if translator decodes a fallback
		//next opcode
		cpu->programCounter += 2;
	}



	void executeBlock(){
		
		ICache* temp = cache->getCache(cpu->programCounter);
		
#ifdef DEBUG_ME
		printf("executing block...\n");
#endif
#ifdef _WIN32

		SYSTEM_INFO system_info;
		GetSystemInfo(&system_info);
		auto const page_size = system_info.dwPageSize;

		// prepare the memory in which the machine code will be put (it's not executable yet):
		void* buffer = VirtualAlloc(nullptr, page_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		// copy the machine code into that memory:
		memcpy(buffer, temp->cache.data(), temp->cache.size());

		// mark the memory as executable:
		DWORD dummy;
		VirtualProtect(buffer, temp->cache.size(), PAGE_EXECUTE_READ, &dummy);

		// interpret the beginning of the (now) executable memory as the entry
		// point of a function taking no arguments and returning a 4-byte int:


		typedef int32_t(*dank)(void);
		using weed = int32_t(*)(void);
		dank function_ptr = (weed)buffer;

		//auto const function_ptr = reinterpret_cast<std::int32_t(*)()>(buffer);


		// call the function and store the result in a local std::int32_t object:
		function_ptr();

		// free the executable memory:
		VirtualFree(buffer, 0, MEM_RELEASE);

#elif __LINUX__
		void *buffer = mmap(NULL,             // address
			4096,             // size
			PROT_READ | PROT_WRITE | PROT_EXEC,
			MAP_PRIVATE | MAP_ANONYMOUS,
			-1,               // fd (not used here)
			0);               // offset (not used here)

		memcpy(buffer, temp->cache.data(), temp->cache.size());

		typedef int32_t(*dank)(void);
		using weed = int32_t(*)(void);
		dank function_ptr = (weed)buffer;

		function_ptr();

#endif

		//if hintFallback flipped, continue to execute fallback
		if (!hintFallback) return;


#ifdef DEBUG_ME
		printf("executing fallback...\n");
#endif
		//delayNext for controllerOp
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
				video->copySprite(currentOpcode, cpu, memory, video);
				break;
			case ControllerOp::setSoundTimer:
				audio->setSoundTimer(currentOpcode, cpu);
				break;
		}

		//reset
		switchToInterpreter = false;
		hintFallback = false;
		delayNext = false;
	}

};