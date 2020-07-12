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

	ControllerOp controllerOp;	//after cpu processes its stuff, next is chip8 controller output job
	bool isEndlessLoop = false;	//notify debugger that this is endless loop

	int whichInterpreter; //choose interpreter method

	bool drawFlag = false; //possible speed optimization
	
	
	
	//translator core stuff
	bool switchToInterpreter = false;
	bool hintFallback = false;
	uint32_t x_val = 0;
	uint32_t y_val = 0;
	uint32_t nx = 0;
	uint32_t nnx = 0;
	uint32_t nnnx = 0;
	
	uint32_t baseClock;


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
		
		this->baseClock = baseClock;
		
		this->cpu = cpu;
		this->video = video;
		this->memory = memory;
		this->audio = audio;
		cache = new Cache();
		translator = new Translator(cpu,	//cpu variables
			(uint32_t)&switchToInterpreter,
			(uint32_t)&hintFallback,
			(uint32_t)&x_val,
			(uint32_t)&y_val, 
			(uint32_t)&nx, 
			(uint32_t)&nnx, 
			(uint32_t)&nnnx);	//core variables
	}







	uint16_t updateRecompiler(){

		//ready icache
		if (cache->checkCacheExists(cpu->programCounter)) return currentOpcode;
		translator->init(cache->createCache(cpu->programCounter));

		//compile ahead while backup pc
		uint16_t pcTemp = cpu->programCounter;

		//recompile n
		for (int i = 0; i < baseClock && !translator->checkFallback() && !translator->checkEndDecode(); i++){

			//fetch
			previousOpcode = currentOpcode;
			currentOpcode = cpu->fetch();


			//update xy for next opcode
			x_val = (currentOpcode & 0x0f00) >> 8;
			y_val = (currentOpcode & 0x00f0) >> 4;
			nx = currentOpcode & 0x000F;
			nnx = currentOpcode & 0x00FF;
			nnnx = currentOpcode & 0x0FFF;

			//cut one full jiffy
			//ret at the end
			if (i == baseClock - 1) translator->decode(true);
			else translator->decode();

			//won't reach if translator decodes a fallback
			//next opcode
			cpu->programCounter += 2;
		}

#ifdef DEBUG_ME
		cache->printCache(pcTemp);

#endif

		//restore original pc
		cpu->programCounter = pcTemp;

		return currentOpcode;
	}





	void executeBlock(){

		ICache* temp = cache->getCache(cpu->programCounter);
		
		if (!temp->isFallback){
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

		}
		else{
			cpu->decode_jumboLUT();
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
					video->copySprite(currentOpcode, cpu, memory, video);
					break;
				case ControllerOp::setSoundTimer:
					audio->setSoundTimer(currentOpcode, cpu);
					break;
			}
		}

		
		
	}

};