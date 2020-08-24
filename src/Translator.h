#pragma once

/*
	Dynarec chip8 translator

*/

#include "X86Emitter.h"
#include"CPU.h"
#include"Cache.h"

class Translator;
typedef void(Translator::*Table) (void);

class Translator: public X86Emitter{

private:
	//big endians(will be converted automatically on X86Emitter.h)
	//original interpreter registers(will still be used for other parts of the emulator not covered by dynarec)
	uint32_t controllerOp;		//dword
	uint32_t programCounter;	//word
	uint32_t stack;				//word
	uint32_t stackPointer;		//byte
	uint32_t flag;				//dword
	uint32_t jmpHint;			//byte
	uint32_t v;					//byte
	uint32_t indexRegister;		//word
	uint32_t pressedKey;		//byte
	uint32_t delayRegister;		//byte
	uint32_t throwError;		//byte
	uint32_t mem;				//byte
	uint32_t currentOpcode;		//word

	uint32_t hintFallback;		//byte
	uint32_t interpreterSwitch;	//byte
	uint32_t delayNext;			//byte

	uint32_t vxPointer;			//byte
	uint32_t vyPointer;			//byte
	uint32_t vfPointer;			//byte
	uint32_t vzPointer;			//byte

	uint32_t n;					//word
	uint32_t nn;				//word
	uint32_t nnn;				//word

	int* index;
	ICache* memoryBlock;

	bool endDecode = false;
	bool doIncrementPC = false;
	bool doFallback = false;

	//jumboLUT version of opcode table
#define JUMBO_TABLE_SIZE 0x10000
	Table jumbo_table[JUMBO_TABLE_SIZE];

	
public:

	//internal: do not use
	void init(ICache* memoryBlock){
		this->memoryBlock = memoryBlock;

		//start fresh
		endDecode = false;
		doIncrementPC = false;
		doFallback = false;
		
	}


	//constructor
	Translator(CPU* cpu,
		uint32_t interpreterSwitch,
		uint32_t hintFallback,
		uint32_t delayNext
		){
		controllerOp = (uint32_t)&cpu->controllerOp;
		programCounter = (uint32_t)&cpu->programCounter;
		
		stackPointer = (uint32_t)&cpu->stackPointer;
		flag = (uint32_t)&cpu->flag;
		jmpHint = (uint32_t)&cpu->jmpHint;
		
		indexRegister = (uint32_t)&cpu->indexRegister;
		throwError = (uint32_t)&cpu->throwError;
		currentOpcode = (uint32_t)&cpu->currentOpcode;

		//arrays
		v = (uint32_t)cpu->v;
		stack = (uint32_t)cpu->stack;

		//these are chip8 variables referenced from cpu
		pressedKey = (uint32_t)cpu->pressedKey;
		delayRegister = (uint32_t)cpu->delayRegister;

		//TODO
		mem = (uint32_t)cpu->memory->mem;

		this->interpreterSwitch = interpreterSwitch;
		this->hintFallback = hintFallback;
		this->delayNext = delayNext;
		


#ifdef DEBUG_ME
		//for checking proper translator input
		printf(
			"\tregex me!\n"
			"Find what:\n"
			"(0x%02X)|"
			"(0x%02X)|"
			"(0x%02X)|"
			"(0x%02X)|"
			"(0x%02X)|"
			"(0x%02X)|"
			"(0x%02X)|"
			"(0x%02X)|"
			"(0x%02X)|"
			"(0x%02X)|"
			"(0x%02X)|"
			"(0x%02X)|"
			"(0x%02X)|"
			"(0x%02X)|"
			"(0x%02X)"
			"\nReplace with:\n"
			"(?1interpreterSwitch)"
			"(?2hintFallback)"
			"(?3Vreg)"
			"(?4controllerOp)"
			"(?5programCounter)"
			"(?6stack)"
			"(?7stackPointer)"
			"(?8flag)"
			"(?9jmpHint)"
			"(?10indexRegister)"
			"(?11pressedKey)"
			"(?12delayRegister)"
			"(?13throwError)"
			"(?14mem)"
			"(?15currentOpcode)"
			"\n\n",
			interpreterSwitch,
			hintFallback,
			v,
			controllerOp,
			programCounter,
			stack,
			stackPointer,
			flag,
			jmpHint,
			indexRegister,
			pressedKey,
			delayRegister,
			throwError,
			mem,
			currentOpcode);

#endif
		
		for (uint32_t i = 0x0; i < JUMBO_TABLE_SIZE; i++) jumbo_table[i] = &Translator::opcodefall;		//all fallback

		//TODO: fix all commented out opcodes
		//		need to fix test_opcode.ch8 not displaying anything
		/*
		jumbo_table[0x00e0] = &Translator::opcode00e0;
		jumbo_table[0x00ee] = &Translator::opcode00ee;
		
		for (uint32_t i = 0x1000; i < 0x2000; i++) jumbo_table[i] = &Translator::opcode1nnn;
		*/
		for (uint32_t i = 0x2000; i < 0x3000; i++) jumbo_table[i] = &Translator::opcode2nnn;
		
		//for (uint32_t i = 0x3000; i < 0x4000; i++) jumbo_table[i] = &Translator::opcode3xnn;
		
		//for (uint32_t i = 0x4000; i < 0x5000; i++) jumbo_table[i] = &Translator::opcode4xnn;
		
		//for (uint32_t i = 0x5000; i < 0x6000; i++) if ((i & 0x000f) == 0x0) jumbo_table[i] = &Translator::opcode5xy0;
		/*
		for (uint32_t i = 0x6000; i < 0x7000; i++) jumbo_table[i] = &Translator::opcode6xnn;
		
		for (uint32_t i = 0x7000; i < 0x8000; i++) jumbo_table[i] = &Translator::opcode7xnn;
		
		for (uint32_t i = 0x8000; i < 0x9000; i++){
			if ((i & 0x000f) == 0x0) jumbo_table[i] = &Translator::opcode8xy0;
			//if ((i & 0x000f) == 0x1) jumbo_table[i] = &Translator::opcode8xy1;
			//if ((i & 0x000f) == 0x2) jumbo_table[i] = &Translator::opcode8xy2;
			//if ((i & 0x000f) == 0x3) jumbo_table[i] = &Translator::opcode8xy3;
			//if ((i & 0x000f) == 0x4) jumbo_table[i] = &Translator::opcode8xy4;
			//if ((i & 0x000f) == 0x5) jumbo_table[i] = &Translator::opcode8xy5;
			if ((i & 0x000f) == 0x6) jumbo_table[i] = &Translator::opcode8xy6;
			if ((i & 0x000f) == 0x7) jumbo_table[i] = &Translator::opcode8xy7;
			//if ((i & 0x000f) == 0xe) jumbo_table[i] = &Translator::opcode8xye;
		}
		
		for (uint32_t i = 0x9000; i < 0xa000; i++) if (i % 0x10 == 0x0) jumbo_table[i] = &Translator::opcode9xy0;
		
		for (uint32_t i = 0xa000; i < 0xb000; i++) jumbo_table[i] = &Translator::opcodeannn;
		
		for (uint32_t i = 0xb000; i < 0xc000; i++) jumbo_table[i] = &Translator::opcodebnnn;
		
		for (uint32_t i = 0xc000; i < 0xd000; i++) jumbo_table[i] = &Translator::opcodecxnn;
		
		//for (uint32_t i = 0xd000; i < 0xe000; i++) jumbo_table[i] = &Translator::opcodedxyn;
		
		for (uint32_t i = 0xe000; i < 0xf000; i++){
			if ((i & 0x00ff) == 0x9e) jumbo_table[i] = &Translator::opcodeex9e;
			//if ((i & 0x00ff) == 0xa1) jumbo_table[i] = &Translator::opcodeexa1;
		}
		
		for (uint32_t i = 0xf000; i < JUMBO_TABLE_SIZE; i++){
			if ((i & 0x00ff) == 0x07) jumbo_table[i] = &Translator::opcodefx07;
			if ((i & 0x00ff) == 0x0a) jumbo_table[i] = &Translator::opcodefx0a;
			if ((i & 0x00ff) == 0x15) jumbo_table[i] = &Translator::opcodefx15;
			//if ((i & 0x00ff) == 0x18) jumbo_table[i] = &Translator::opcodefx18;
			if ((i & 0x00ff) == 0x1e) jumbo_table[i] = &Translator::opcodefx1e;
			//if ((i & 0x00ff) == 0x29) jumbo_table[i] = &Translator::opcodefx29;
			if ((i & 0x00ff) == 0x33) jumbo_table[i] = &Translator::opcodefx33;
			if ((i & 0x00ff) == 0x55) jumbo_table[i] = &Translator::opcodefx55;
			if ((i & 0x00ff) == 0x65) jumbo_table[i] = &Translator::opcodefx65;
		}
		*/
		
	}


	//call this from Dynarec core
	void decode(TranslatorState* state, bool endMe = false){
		//abrupt cut by jiffy
		if (endMe){ interpreterSwitch_func(); return; }


		uint16_t cpuOp = *(uint16_t*)currentOpcode;

		//to each state
		vxPointer = (uint32_t)&state->x_val;
		vyPointer = (uint32_t)&state->y_val;
		n = (uint32_t)&state->nx;
		nn = (uint32_t)&state->nnx;
		nnn = (uint32_t)&state->nnnx;
		vzPointer = (uint32_t)&state->z_val;
		vfPointer = (uint32_t)&state->f_val;


#ifdef DEBUG_ME
		//for checking proper translator input
		if (cpuOp != 0x0)
		printf("compiled opcode: %02X, xPointer: %X, yPointer: %X, nnn: %X\n",
			cpuOp,
			*(uint8_t*)vxPointer,
			*(uint8_t*)vyPointer,
			*(uint16_t*)nnn);
#endif

		(this->*(jumbo_table[cpuOp]))();
		memoryBlock->check = true;
		//memoryBlock->endOp = currentOpcode;
		//TODO
	}


	bool checkIncrementPC(){ return doIncrementPC; }
	bool checkFallback(){ return doFallback; }
	bool checkEndDecode(){ return endDecode; }

	void endBlock(){
		interpreterSwitch_func();
	}
	void startBlock(){
		X86Emitter::BlockInitializer(&memoryBlock->cache);
	}

private:

	//these are for internal use
	//increment program counter by 2
	void incrementPC(){
		X86Emitter::addToMemaddr(&memoryBlock->cache, programCounter, 2, Word);
		doIncrementPC = true;
		//X86Emitter::addWordToMemaddr(memoryBlock, programCounter, 2);
	}

	//interpreterSwitch = true;
	//fallback to interpreter, switch back whenever its done. but not here
	//some opcodes are just too complicated to recreate in jit
	//...or jump
	void hintFallback_func(){
		X86Emitter::setToMemaddr(&memoryBlock->cache, hintFallback, 0x1, Byte);

		//for updateRecompiler
		doFallback = true;
		
	}
	void delayNext_func(){
		//for controllerOp operations(use with hintFallback_func)
		X86Emitter::setToMemaddr(&memoryBlock->cache, delayNext, 0x1, Byte);


	}
	void interpreterSwitch_func(){
		X86Emitter::setToMemaddr(&memoryBlock->cache, interpreterSwitch, 0x1, Byte);
		X86Emitter::BlockFinisher(&memoryBlock->cache);
		endDecode = true;
	}

	//opcodes
	void opcode00e0(){
		X86Emitter::setToMemaddr(&memoryBlock->cache, controllerOp, ControllerOp::clearScreen, Dword);
		//X86Emitter::setDwordToMemaddr(&memoryBlock->cache, controllerOp, (uint32_t)ControllerOp::clearScreen);

		incrementPC();
		delayNext_func();
		hintFallback_func();
		interpreterSwitch_func();
		//original
		//controllerOp = ControllerOp::clearScreen;
	}

	void opcode00ee(){
		//stack = ebx, stackptr = eax, pc = ecx
		//--stackPointer
		X86Emitter::Add_imm(&memoryBlock->cache, byteAddImmToMemaddrMode, insertAddr(stackPointer), insertDisp(-1));
		//X86Emitter::dec_byte_memaddr(&memoryBlock->cache, stackPointer);

		//stack[stackPointer]
		X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, stack, stackPointer, Word);

		//to programCounter
		X86Emitter::Mov(&memoryBlock->cache, movToMemaddrWordMode, Areg, insertAddr(programCounter));
		//X86Emitter::mov_ax_to_memoryaddr(&memoryBlock->cache, programCounter);

		incrementPC();
		interpreterSwitch_func();
		//programCounter = stack[--stackPointer]; //return from SUBroutine	(and increment pc after to get out of loop)
	}
	void opcode0nnn(){
		
		//= programCounter
		X86Emitter::loadMemToDwordReg(&memoryBlock->cache, programCounter, Areg, Word);

		//stack[stackpointer]
		X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, stack, stackPointer, Word);

		//stackPointer++
		X86Emitter::Add_imm(&memoryBlock->cache, byteAddImmToMemaddrMode, insertAddr(stackPointer), insertDisp(1));
		//X86Emitter::inc_byte_memaddr(&memoryBlock->cache, stackPointer);

		//NNN(is an immediate) to programCounter
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(*(uint32_t*)nnn));
		X86Emitter::parse(&memoryBlock->cache, "mov WORD PTR [extra], eax", insertDisp(programCounter));
		//X86Emitter::setToMemaddr(&memoryBlock->cache, programCounter, *(uint32_t*)nnn, Word);
		//X86Emitter::setWordToMemaddr(&memoryBlock->cache, programCounter, nnn);

		interpreterSwitch_func();
		//stack[stackPointer++] = programCounter; programCounter = NNN; flag = 1;//call SUBroutine from nnn	(dont increment pc)
	}
	void opcode1nnn(){
		//NNN to programCounter
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(*(uint32_t*)nnn));
		X86Emitter::parse(&memoryBlock->cache, "mov WORD PTR [extra], eax", insertDisp(programCounter));
		//X86Emitter::setToMemaddr(&memoryBlock->cache, programCounter, *(uint32_t*)nnn, Word);

		//jmpHint = true
		X86Emitter::setToMemaddr(&memoryBlock->cache, jmpHint, 1, Byte);
		//X86Emitter::setByteToMemaddr(&memoryBlock->cache, jmpHint, 1);

		interpreterSwitch_func();
		//programCounter = NNN; flag = 1;//jump to nnn	(dont increment pc)
		//jmpHint = true; //hint for video flicker loop
	}
	void opcode2nnn(){
		//Breakpoint(&memoryBlock->cache);
		//= programCounter
		loadMemToDwordReg(&memoryBlock->cache, programCounter, Areg, Word);

		//stack[stackpointer]
		X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, stack, stackPointer, Word);

		//stackPointer++
		X86Emitter::Add_imm(&memoryBlock->cache, byteAddImmToMemaddrMode, insertAddr(stackPointer), insertDisp(1));
		//X86Emitter::inc_byte_memaddr(&memoryBlock->cache, stackPointer);

		//NNN(is an immediate) to programCounter
		//printf("2nnn: %02X\n", *(uint32_t*)nnn);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(*(uint32_t*)nnn));
		X86Emitter::parse(&memoryBlock->cache, "mov WORD PTR [extra], eax", insertDisp(programCounter));
		//X86Emitter::setToMemaddr(&memoryBlock->cache, programCounter, *(uint32_t*)nnn, Word);

		interpreterSwitch_func();
		//stack[stackPointer++] = programCounter; programCounter = NNN; flag = 1;//call SUBroutine from nnn	(dont increment pc)
	}

	void opcode3xnn(){
		//vx = eax, nn = ebx

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::loadMemToDwordReg(&memoryBlock->cache, nn, Breg, Word);

		X86Emitter::Cmp(&memoryBlock->cache, cmpMode, Areg, Breg);
		//X86Emitter::cmp_eax_to_ebx(&memoryBlock->cache);
		X86Emitter::parse(&memoryBlock->cache, "jne extra", insertDisp(addWordToMemaddrSize));
		//X86Emitter::Jcc(&memoryBlock->cache, byteRelJneMode, insertDisp(addWordToMemaddrSize));
		//X86Emitter::short_jne(&memoryBlock->cache, addWordToMemaddrSize);
		X86Emitter::addToMemaddr(&memoryBlock->cache, programCounter, 2, Word);

		incrementPC();
		//if (VX == NN) programCounter += 2; //skip if ==
	}
	void opcode4xnn(){
		//vx = eax, nn = ebx
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)nn));

		X86Emitter::Cmp(&memoryBlock->cache, cmpMode, Areg, Breg);
		X86Emitter::parse(&memoryBlock->cache, "je extra", insertDisp(addWordToMemaddrSize));
		//X86Emitter::Jcc(&memoryBlock->cache, byteRelJeMode, insertDisp(addWordToMemaddrSize));
		X86Emitter::addToMemaddr(&memoryBlock->cache, programCounter, 2, Word);

		incrementPC();
		//if (VX != NN) programCounter += 2; //skip if !=
	}
	void opcode5xy0(){
		//vx = eax, vy = edx
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Mov(&memoryBlock->cache, movDwordRegToRegMode, Areg, Dreg);
		//X86Emitter::mov_eax_to_edx(&memoryBlock->cache);
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Cmp(&memoryBlock->cache, cmpMode, Areg, Breg);
		X86Emitter::parse(&memoryBlock->cache, "jne extra", insertDisp(addWordToMemaddrSize));
		//X86Emitter::Jcc(&memoryBlock->cache, byteRelJneMode, insertDisp(addWordToMemaddrSize));
		X86Emitter::addToMemaddr(&memoryBlock->cache, programCounter, 2, Word);

		incrementPC();
		//if (VX == VY) programCounter += 2; //skip if vx == vy
	}
	void opcode6xnn(){

		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(*(uint32_t*)nn));

		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		incrementPC();
		//VX = NN; //into
	}
	void opcode7xnn(){

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)nn));

		X86Emitter::Add(&memoryBlock->cache, dwordAddMode, Breg, Areg);
		//X86Emitter::add_ebx_to_eax(&memoryBlock->cache);

		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		incrementPC();
		//VX += NN;
	}
	void opcode8xy0(){

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		incrementPC();
		//VX = VY;
	}
	void opcode8xy1(){

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Mov(&memoryBlock->cache, movDwordRegToRegMode, Areg, Breg);

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Or(&memoryBlock->cache, dwordOrMode, Breg, Areg);
		//X86Emitter::or_ebx_to_eax(&memoryBlock->cache);

		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		incrementPC();
		//VX |= VY;
	}
	void opcode8xy2(){

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Mov(&memoryBlock->cache, movDwordRegToRegMode, Areg, Breg);

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::And(&memoryBlock->cache, dwordAndMode, Breg, Areg);

		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		incrementPC();
		//VX &= VY;
	}
	void opcode8xy3(){

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Mov(&memoryBlock->cache, movDwordRegToRegMode, Areg, Breg);

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Xor(&memoryBlock->cache, dwordXorMode, Breg, Areg);

		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		incrementPC();
		//VX ^= VY;
	}
	void opcode8xy4(){
		//ebx = vx + vy
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");
		
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, eax");
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");

		//ecx = 0xff
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, 0xff");

		//>?
		X86Emitter::parse(&memoryBlock->cache, "mov eax, 0");
		X86Emitter::Cmp(&memoryBlock->cache, cmpMode, Breg, Creg);
		X86Emitter::parse(&memoryBlock->cache, "jbe extra", insertDisp(dwordAddImmToRegSize));
		//X86Emitter::Jcc(&memoryBlock->cache, byteRelJbeMode, insertDisp(dwordAddImmToRegSize));

		X86Emitter::parse(&memoryBlock->cache, "add eax, 1");

		//=
		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vfPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vfPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");
		

		//vx += vy
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::parse(&memoryBlock->cache, "mov ebx, eax");

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Add(&memoryBlock->cache, dwordAddMode, Breg, Areg);

		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		incrementPC();
		//VF = (VX + VY > 0xff) ? 0x1 : 0x0; VX += VY;
	}
	void opcode8xy5(){

		//ebx = vy
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::parse(&memoryBlock->cache, "mov ebx, eax");

		//ecx = vx
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::parse(&memoryBlock->cache, "mov ecx, eax");


		//<?
		X86Emitter::parse(&memoryBlock->cache, "mov eax, 1");
		X86Emitter::Cmp(&memoryBlock->cache, cmpMode, Creg, Breg);
		X86Emitter::parse(&memoryBlock->cache, "ja extra", insertDisp(dwordAddImmToRegSize));
		//X86Emitter::Jcc(&memoryBlock->cache, byteRelJaMode, insertDisp(dwordAddImmToRegSize));

		X86Emitter::Add_imm(&memoryBlock->cache, dwordAddImmToRegMode, insertDisp(-1), Areg);

		//=
		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vfPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vfPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		//vx -= vy
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Mov(&memoryBlock->cache, movDwordRegToRegMode, Areg, Breg);

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Sub(&memoryBlock->cache, dwordSubMode, Breg, Areg);

		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		incrementPC();
		//VF = (VX < VY) ? 0x0 : 0x1; VX -= VY;
	}
	void opcode8xy6(){
		//vf = vx << 7
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Shift(&memoryBlock->cache, dwordShiftLeftMode, insertDisp(7), Areg);
		X86Emitter::Movzx(&memoryBlock->cache, movzxByteToDwordMode, Areg, Areg);//make sure to cut off
		
		//vf >>= 7
		X86Emitter::Shift(&memoryBlock->cache, dwordShiftRightMode, insertDisp(7), Areg);
		
		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vfPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vfPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		//vx >>= 1
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Shift(&memoryBlock->cache, dwordShiftRightMode, insertDisp(1), Areg);

		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		incrementPC();
		//VF = VX << 7; VF >>= 7; VX >>= 1;
	}
	void opcode8xy7(){

		//ebx = vy
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Mov(&memoryBlock->cache, movDwordRegToRegMode, Areg, Breg);

		//ecx = vx
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Mov(&memoryBlock->cache, movDwordRegToRegMode, Areg, Creg);


		//<?
		X86Emitter::parse(&memoryBlock->cache, "mov eax, 1");
		X86Emitter::Cmp(&memoryBlock->cache, cmpMode, Creg, Breg);
		X86Emitter::parse(&memoryBlock->cache, "jb extra", insertDisp(dwordAddImmToRegSize));
		//X86Emitter::Jcc(&memoryBlock->cache, byteRelJbMode, insertDisp(dwordAddImmToRegSize));

		X86Emitter::Add_imm(&memoryBlock->cache, dwordAddImmToRegMode, insertDisp(-1), Areg);

		//=
		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vfPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vfPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		//vx -= vy
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Mov(&memoryBlock->cache, movDwordRegToRegMode, Areg, Breg);

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Sub(&memoryBlock->cache, dwordSubMode, Breg, Areg);

		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		incrementPC();
		//VF = (VY < VX) ? 0x0 : 0x1; VX = VY - VX;
	}
	void opcode8xye(){

		//vf = vx >> 7
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Shift(&memoryBlock->cache, dwordShiftRightMode, insertDisp(7), Areg);
		X86Emitter::Movzx(&memoryBlock->cache, movzxByteToDwordMode, Areg, Areg);//make sure to cut off

		//vx <<= 1
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Shift(&memoryBlock->cache, dwordShiftLeftMode, insertDisp(1), Areg);

		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");


		incrementPC();
		//VF = VX >> 7; VX <<= 1;
	}
	void opcode9xy0(){
		//vx = eax, vy = edx
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Mov(&memoryBlock->cache, movDwordRegToRegMode, Areg, Dreg);

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Cmp(&memoryBlock->cache, cmpMode, Areg, Dreg);
		X86Emitter::parse(&memoryBlock->cache, "je extra", insertDisp(addWordToMemaddrSize));
		//X86Emitter::Jcc(&memoryBlock->cache, byteRelJeMode, insertDisp(addWordToMemaddrSize));
		
		addToMemaddr(&memoryBlock->cache, programCounter, 2, Word);


		incrementPC();
		//if (VX != VY) programCounter += 2; //skip if vx != vy
	}
	void opcodeannn(){

		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(*(uint32_t*)nnn));
		//X86Emitter::parse(&memoryBlock->cache, "mov WORD PTR [extra], eax", insertDisp(indexRegister));
		//X86Emitter::setToMemaddr(&memoryBlock->cache, indexRegister, *(uint32_t*)nnn, Word);

		incrementPC();
		//indexRegister = NNN;
	}
	void opcodebnnn(){
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(*(uint32_t*)nnn));
		X86Emitter::parse(&memoryBlock->cache, "mov WORD PTR [extra], eax", insertDisp(programCounter));
		//X86Emitter::setToMemaddr(&memoryBlock->cache, programCounter, *(uint32_t*)nnn, Word);
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, BYTE PTR [extra]", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "add WORD PTR [extra], ebx", insertDisp(programCounter));
		//X86Emitter::addToMemaddr(&memoryBlock->cache, programCounter, v, Word);

		//programCounter = NNN + v[0]; flag = 1; //(dont increment pc)
	}
	void opcodecxnn(){
		hintFallback_func();
		interpreterSwitch_func();
		//incrementPC();
		//VX = (rand() % 0x100) & NN;	//random
	}
	void opcodedxyn(){
		X86Emitter::setToMemaddr(&memoryBlock->cache, controllerOp, ControllerOp::drawVideo, Dword);

		incrementPC();
		delayNext_func();
		hintFallback_func();
		interpreterSwitch_func();
		//controllerOp = ControllerOp::drawVideo;
	}
	void opcodeex9e(){
		X86Emitter::Mov(&memoryBlock->cache, movFromMemaddrByteMode, Breg, insertAddr(pressedKey));
		X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, stack, stackPointer, Word);
		X86Emitter::Cmp(&memoryBlock->cache, cmpMode, Areg, Breg);
		X86Emitter::parse(&memoryBlock->cache, "jne extra", insertDisp(addWordToMemaddrSize));
		//X86Emitter::Jcc(&memoryBlock->cache, byteRelJneMode, insertDisp(addWordToMemaddrSize));
		X86Emitter::addToMemaddr(&memoryBlock->cache, programCounter, 2, Word);

		incrementPC();
		//if (*pressedKey == VX) programCounter += 2;
	}
	void opcodeexa1(){
		X86Emitter::Mov(&memoryBlock->cache, movFromMemaddrByteMode, Breg, insertAddr(pressedKey));
		X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, stack, stackPointer, Word);
		X86Emitter::Cmp(&memoryBlock->cache, cmpMode, Areg, Breg);
		X86Emitter::parse(&memoryBlock->cache, "je extra", insertDisp(addWordToMemaddrSize));
		//X86Emitter::Jcc(&memoryBlock->cache, byteRelJeMode, insertDisp(addWordToMemaddrSize));
		addToMemaddr(&memoryBlock->cache, programCounter, 2, Word);

		incrementPC();
		//if (*pressedKey != VX) programCounter += 2;
	}
	void opcodefx07(){
		X86Emitter::Mov(&memoryBlock->cache, movFromMemaddrByteMode, Areg, insertAddr(delayRegister));
		
		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		incrementPC();
		//VX = *delayRegister;
	}
	void opcodefx0a(){
		hintFallback_func();
		interpreterSwitch_func();
		//incrementPC();
		//if (Input::isKeyPressed(*pressedKey) == true) VX = *pressedKey; else flag = 1; //wait again	(dont increment pc)
	}
	void opcodefx15(){
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Mov(&memoryBlock->cache, movToMemaddrByteMode, Areg, insertAddr(delayRegister));
		
		incrementPC();
		//*delayRegister = VX;
	}
	void opcodefx18(){
		X86Emitter::setToMemaddr(&memoryBlock->cache, controllerOp, ControllerOp::setSoundTimer, Dword);
		//X86Emitter::setDwordToMemaddr(&memoryBlock->cache, controllerOp, (uint32_t)ControllerOp::clearScreen);

		incrementPC();
		delayNext_func();
		hintFallback_func();
		interpreterSwitch_func();
		//controllerOp = ControllerOp::setSoundTimer;
	}
	void opcodefx1e(){
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::parse(&memoryBlock->cache, "add WORD PTR [extra], eax", insertAddr(indexRegister));

		incrementPC();
		//indexRegister += VX;
	}
	void opcodefx29(){
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(*(uint32_t*)vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Lea(&memoryBlock->cache, leaWithoutDispMode, Areg, x4, Areg, Areg);
		X86Emitter::parse(&memoryBlock->cache, "mov WORD PTR [extra], eax", insertAddr(indexRegister));

		incrementPC();
		//indexRegister = VX * 5;	//font is stored at mem[0 ~ FONT_COUNT * 5]
	}
	void opcodefx33(){
		hintFallback_func();
		interpreterSwitch_func();
		//incrementPC();
		//memory->write(indexRegister, VX / 100);
		//memory->write(indexRegister + 1, (VX / 10) % 10);
		//memory->write(indexRegister + 2, VX % 10);
	}
	void opcodefx55(){
		hintFallback_func();
		interpreterSwitch_func();
		//incrementPC();
		//for (int i = 0; i <= (currentOpcode & 0x0f00) >> 8; i++) memory->write(indexRegister + i, v[i]);
	}
	void opcodefx65(){
		hintFallback_func();
		interpreterSwitch_func();
		//incrementPC();
		//for (int i = 0; i <= (currentOpcode & 0x0f00) >> 8; i++) v[i] = memory->read(indexRegister + i);
	}

	//exception
	void opcodenull(){
		hintFallback_func();
		interpreterSwitch_func();
		//incrementPC();
		//throwError = true;
	}

	//for testing purpose
	void opcodefall(){
		hintFallback_func();
		interpreterSwitch_func();
#ifdef NO
		printf("immediate registers: vx = %01X, vy = %01X, n = %01X, nn = %02X, nnn = %03X, vz = %01X, vf = %01X\n",
			*(uint8_t*)vxPointer,
			*(uint8_t*)vyPointer, 
			*(uint16_t*)n, 
			*(uint16_t*)nn, 
			*(uint16_t*)nnn, 
			*(uint8_t*)vzPointer, 
			*(uint8_t*)vfPointer
			);
#endif
	}

};