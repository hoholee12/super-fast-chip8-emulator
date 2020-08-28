#pragma once

/*
	Dynarec chip8 translator

	TODO: convert g++ -O1 dump to proper assembly for translator

	caution: any opcode that mingles with programCounter must call interpreterSwitch_func()!

*/

#include "X86Emitter.h"
#include"CPU.h"
#include"Cache.h"

class Translator;
typedef void(Translator::*Table) (void);

class Translator: public X86Emitter, public defaults{

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

	//static vars
	uint8_t vxPointer;			//byte static
	uint8_t vyPointer;			//byte static
	uint8_t vfPointer;			//byte static
	uint8_t vzPointer;			//byte static
	uint16_t n;					//word static
	uint16_t nn;				//word static
	uint16_t nnn;				//word static

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
		
		jumbo_table[0x00e0] = &Translator::opcode00e0;
		jumbo_table[0x00ee] = &Translator::opcode00ee;
		
		for (uint32_t i = 0x1000; i < 0x2000; i++) jumbo_table[i] = &Translator::opcode1nnn;
		
		for (uint32_t i = 0x2000; i < 0x3000; i++) jumbo_table[i] = &Translator::opcode2nnn;
		
		for (uint32_t i = 0x3000; i < 0x4000; i++) jumbo_table[i] = &Translator::opcode3xnn;
		
		for (uint32_t i = 0x4000; i < 0x5000; i++) jumbo_table[i] = &Translator::opcode4xnn;
		
		for (uint32_t i = 0x5000; i < 0x6000; i++) if ((i & 0x000f) == 0x0) jumbo_table[i] = &Translator::opcode5xy0;
		
		//for (uint32_t i = 0x6000; i < 0x7000; i++) jumbo_table[i] = &Translator::opcode6xnn;
		/*
		for (uint32_t i = 0x7000; i < 0x8000; i++) jumbo_table[i] = &Translator::opcode7xnn;
		
		for (uint32_t i = 0x8000; i < 0x9000; i++){
			if ((i & 0x000f) == 0x0) jumbo_table[i] = &Translator::opcode8xy0;
			if ((i & 0x000f) == 0x1) jumbo_table[i] = &Translator::opcode8xy1;
			if ((i & 0x000f) == 0x2) jumbo_table[i] = &Translator::opcode8xy2;
			if ((i & 0x000f) == 0x3) jumbo_table[i] = &Translator::opcode8xy3;
			if ((i & 0x000f) == 0x4) jumbo_table[i] = &Translator::opcode8xy4;
			//if ((i & 0x000f) == 0x5) jumbo_table[i] = &Translator::opcode8xy5;
			//if ((i & 0x000f) == 0x6) jumbo_table[i] = &Translator::opcode8xy6;
			//if ((i & 0x000f) == 0x7) jumbo_table[i] = &Translator::opcode8xy7;
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
		vxPointer = state->x_val;
		vyPointer = state->y_val;
		n = state->nx;
		nn = state->nnx;
		nnn = state->nnnx;
		vzPointer = state->z_val;
		vfPointer = state->f_val;


#ifdef DEBUG_CACHE
		//for checking proper translator input
		if (cpuOp != 0x0){
			defaults::debugmsg("compiled opcode: %02X, xPointer: %X, yPointer: %X, nnn: %X\n",
			cpuOp,
			vxPointer,
			vyPointer,
			nnn);
		}
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
		/*
		void CPU::opcode00e0(){ 
			controllerOp = ControllerOp::clearScreen;
			19e6:	8b 44 24 04          	mov    eax,DWORD PTR [esp+0x4]
			19ea:	c7 80 d4 19 08 00 01 	mov    DWORD PTR [eax+0x819d4],0x1
			19f1:	00 00 00 
		}
			19f4:	c3                   	ret    
			19f5:	90                   	nop
		*/

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
		/*
		void CPU::opcode00ee(){
			19f6:	8b 44 24 04          	mov    eax,DWORD PTR [esp+0x4]
			stackPointer--;
			19fa:	0f b6 90 9e 00 00 00 	movzx  edx,BYTE PTR [eax+0x9e]
			1a01:	83 ea 01             	sub    edx,0x1
			1a04:	88 90 9e 00 00 00    	mov    BYTE PTR [eax+0x9e],dl
			programCounter = stack[stackPointer]; //return from SUBroutine	(and increment pc after to get out of loop)
			1a0a:	0f b6 90 9e 00 00 00 	movzx  edx,BYTE PTR [eax+0x9e]
			1a11:	0f b6 d2             	movzx  edx,dl
			1a14:	83 c2 50             	add    edx,0x50
			1a17:	0f b7 54 50 02       	movzx  edx,WORD PTR [eax+edx*2+0x2]
			1a1c:	66 89 90 9c 00 00 00 	mov    WORD PTR [eax+0x9c],dx
		}
			1a23:	c3                   	ret
		*/

		//--stackPointer
		X86Emitter::parse(&memoryBlock->cache, "add BYTE PTR [extra], -1", insertAddr(stackPointer));

		//stack[stackPointer]
		X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, stack, stackPointer, false, Word);

		//to programCounter
		X86Emitter::parse(&memoryBlock->cache, "mov WORD PTR [extra], ax", insertAddr(programCounter));

		incrementPC();
		interpreterSwitch_func();
		//programCounter = stack[--stackPointer]; //return from SUBroutine	(and increment pc after to get out of loop)
	}
	void opcode0nnn(){
		/*
		void CPU::opcode0nnn(){
			2aa2:	8b 44 24 04          	mov    eax,DWORD PTR [esp+0x4]
			stack[stackPointer] = programCounter;
			2aa6:	0f b6 90 9e 00 00 00 	movzx  edx,BYTE PTR [eax+0x9e]
			2aad:	0f b6 d2             	movzx  edx,dl
			2ab0:	0f b7 88 9c 00 00 00 	movzx  ecx,WORD PTR [eax+0x9c]
			2ab7:	83 c2 50             	add    edx,0x50
			2aba:	66 89 4c 50 02       	mov    WORD PTR [eax+edx*2+0x2],cx
			stackPointer++;
			2abf:	0f b6 90 9e 00 00 00 	movzx  edx,BYTE PTR [eax+0x9e]
			2ac6:	83 c2 01             	add    edx,0x1
			2ac9:	88 90 9e 00 00 00    	mov    BYTE PTR [eax+0x9e],dl
			programCounter = NNN;
			2acf:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			2ad6:	66 81 e2 ff 0f       	and    dx,0xfff
			2adb:	66 89 90 9c 00 00 00 	mov    WORD PTR [eax+0x9c],dx
			flag = 1;	//call SUBroutine from nnn	(dont increment pc)
			2ae2:	c7 80 dc 19 08 00 01 	mov    DWORD PTR [eax+0x819dc],0x1
			2ae9:	00 00 00 
		}
			2aec:	c3                   	ret    
			2aed:	90                   	nop
		*/

		//= programCounter
		X86Emitter::loadMemToDwordReg(&memoryBlock->cache, programCounter, Areg, Word);

		//stack[stackpointer]
		X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, stack, stackPointer, false, Word);

		//stackPointer++
		X86Emitter::parse(&memoryBlock->cache, "add BYTE PTR [extra], 1", insertAddr(stackPointer));

		//NNN(is an immediate) to programCounter
		X86Emitter::setToMemaddr(&memoryBlock->cache, programCounter, nnn, Word);

		interpreterSwitch_func();
		//stack[stackPointer++] = programCounter; programCounter = NNN; flag = 1;//call SUBroutine from nnn	(dont increment pc)
	}
	void opcode1nnn(){
		/*
		void CPU::opcode1nnn(){
			1a24:	8b 44 24 04          	mov    eax,DWORD PTR [esp+0x4]
			programCounter = NNN;
			1a28:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			1a2f:	66 81 e2 ff 0f       	and    dx,0xfff
			1a34:	66 89 90 9c 00 00 00 	mov    WORD PTR [eax+0x9c],dx
			flag = 1;	//jump to nnn	(dont increment pc)
			1a3b:	c7 80 dc 19 08 00 01 	mov    DWORD PTR [eax+0x819dc],0x1
			1a42:	00 00 00 
			jmpHint = true;	//hint for video flicker loop
			1a45:	c6 80 ec 19 08 00 01 	mov    BYTE PTR [eax+0x819ec],0x1
		}
			1a4c:	c3                   	ret    
			1a4d:	90                   	nop
		*/


		//NNN to programCounter
		X86Emitter::setToMemaddr(&memoryBlock->cache, programCounter, nnn, Word);

		//jmpHint = true
		X86Emitter::setToMemaddr(&memoryBlock->cache, jmpHint, 1, Byte);

		interpreterSwitch_func();
		//programCounter = NNN; flag = 1;//jump to nnn	(dont increment pc)
		//jmpHint = true; //hint for video flicker loop
	}
	void opcode2nnn(){
		/*
		opcode0nnn(); //call SUBroutine from nnn	(dont increment pc)
			2aee:	ff 74 24 04          	push   DWORD PTR [esp+0x4]
			2af2:	e8 ab ff ff ff       	call   2aa2 <_ZN3CPU10opcode0nnnEv>
		}
			2af7:	83 c4 04             	add    esp,0x4
			2afa:	c3                   	ret    
			2afb:	90                   	nop
		*/
		opcode0nnn();
	}

	void opcode3xnn(){
		/*
		void CPU::opcode3xnn(){
			1a4e:	8b 54 24 04          	mov    edx,DWORD PTR [esp+0x4]
			if (VX == NN)
			1a52:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1a59:	0f b6 c4             	movzx  eax,ah
			1a5c:	83 e0 0f             	and    eax,0xf
			1a5f:	0f b6 8c 02 c2 00 00 	movzx  ecx,BYTE PTR [edx+eax*1+0xc2]
			1a66:	00 
			1a67:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1a6e:	0f b6 c9             	movzx  ecx,cl
			1a71:	0f b6 c0             	movzx  eax,al
			1a74:	39 c1                	cmp    ecx,eax
			1a76:	74 02                	je     1a7a <_ZN3CPU10opcode3xnnEv+0x2c>
				programCounter += 2; //skip if ==
		}
			1a78:	f3 c3                	repz ret 
				programCounter += 2; //skip if ==
			1a7a:	0f b7 82 9c 00 00 00 	movzx  eax,WORD PTR [edx+0x9c]
			1a81:	83 c0 02             	add    eax,0x2
			1a84:	66 89 82 9c 00 00 00 	mov    WORD PTR [edx+0x9c],ax
		}
			1a8b:	eb eb                	jmp    1a78 <_ZN3CPU10opcode3xnnEv+0x2a>
			1a8d:	90                   	nop
		*/

		//vx = eax
		X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, true, Byte);

		//nn = ecx
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(nn));

		X86Emitter::parse(&memoryBlock->cache, "cmp eax, ecx");

		X86Emitter::parse(&memoryBlock->cache, "jne extra", insertDisp(addWordToMemaddrSize));

		X86Emitter::addToMemaddr(&memoryBlock->cache, programCounter, 2, Word);

		incrementPC();

		interpreterSwitch_func();
		//if (VX == NN) programCounter += 2; //skip if ==
	}
	void opcode4xnn(){
		/*
		void CPU::opcode4xnn(){
			1a8e:	8b 54 24 04          	mov    edx,DWORD PTR [esp+0x4]
			if (VX != NN)
			1a92:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1a99:	0f b6 c4             	movzx  eax,ah
			1a9c:	83 e0 0f             	and    eax,0xf
			1a9f:	0f b6 8c 02 c2 00 00 	movzx  ecx,BYTE PTR [edx+eax*1+0xc2]
			1aa6:	00 
			1aa7:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1aae:	0f b6 c9             	movzx  ecx,cl
			1ab1:	0f b6 c0             	movzx  eax,al
			1ab4:	39 c1                	cmp    ecx,eax
			1ab6:	74 11                	je     1ac9 <_ZN3CPU10opcode4xnnEv+0x3b>
				programCounter += 2; //skip if !=
			1ab8:	0f b7 82 9c 00 00 00 	movzx  eax,WORD PTR [edx+0x9c]
			1abf:	83 c0 02             	add    eax,0x2
			1ac2:	66 89 82 9c 00 00 00 	mov    WORD PTR [edx+0x9c],ax
		}
			1ac9:	f3 c3                	repz ret 
			1acb:	90                   	nop
		*/

		//vx = eax
		X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, true, Byte);

		//nn = ecx
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(nn));

		X86Emitter::parse(&memoryBlock->cache, "cmp eax, ecx");

		X86Emitter::parse(&memoryBlock->cache, "je extra", insertDisp(addWordToMemaddrSize));

		X86Emitter::addToMemaddr(&memoryBlock->cache, programCounter, 2, Word);

		incrementPC();

		interpreterSwitch_func();
		//if (VX != NN) programCounter += 2; //skip if !=
	}
	void opcode5xy0(){
		/*
		void CPU::opcode5xy0(){
			1acc:	8b 54 24 04          	mov    edx,DWORD PTR [esp+0x4]
			if (VX == VY)
			1ad0:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1ad7:	0f b6 c4             	movzx  eax,ah
			1ada:	83 e0 0f             	and    eax,0xf
			1add:	0f b6 8c 02 c2 00 00 	movzx  ecx,BYTE PTR [edx+eax*1+0xc2]
			1ae4:	00 
			1ae5:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1aec:	c1 e8 04             	shr    eax,0x4
			1aef:	83 e0 0f             	and    eax,0xf
			1af2:	0f b6 84 02 c2 00 00 	movzx  eax,BYTE PTR [edx+eax*1+0xc2]
			1af9:	00 
			1afa:	38 c1                	cmp    cl,al
			1afc:	74 02                	je     1b00 <_ZN3CPU10opcode5xy0Ev+0x34>
				programCounter += 2; //skip if vx == vy
		}
			1afe:	f3 c3                	repz ret 
				programCounter += 2; //skip if vx == vy
			1b00:	0f b7 82 9c 00 00 00 	movzx  eax,WORD PTR [edx+0x9c]
			1b07:	83 c0 02             	add    eax,0x2
			1b0a:	66 89 82 9c 00 00 00 	mov    WORD PTR [edx+0x9c],ax
		}
			1b11:	eb eb                	jmp    1afe <_ZN3CPU10opcode5xy0Ev+0x32>
			1b13:	90                   	nop
		*/

		//vx = eax, vy = edx
		X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, true, Byte);

		X86Emitter::Mov(&memoryBlock->cache, movDwordRegToRegMode, Areg, Dreg);
		
		X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, true, Byte);

		X86Emitter::parse(&memoryBlock->cache, "cmp eax, edx");

		X86Emitter::parse(&memoryBlock->cache, "jne extra", insertDisp(addWordToMemaddrSize));
		
		X86Emitter::addToMemaddr(&memoryBlock->cache, programCounter, 2, Word);

		incrementPC();

		interpreterSwitch_func();
		//if (VX == VY) programCounter += 2; //skip if vx == vy
	}
	void opcode6xnn(){
		/*
		void CPU::opcode6xnn(){
			1b14:	8b 54 24 04          	mov    edx,DWORD PTR [esp+0x4]
			VX = NN; //into
			1b18:	0f b7 8a 9a 00 00 00 	movzx  ecx,WORD PTR [edx+0x9a]
			1b1f:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1b26:	0f b6 c4             	movzx  eax,ah
			1b29:	83 e0 0f             	and    eax,0xf
			1b2c:	88 8c 02 c2 00 00 00 	mov    BYTE PTR [edx+eax*1+0xc2],cl
		}
			1b33:	c3                   	ret
		*/
		

		X86Emitter::parse(&memoryBlock->cache, "mov ax, extra", insertDisp(nn));

		X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, true, Byte);

		incrementPC();
		//VX = NN; //into
	}
	void opcode7xnn(){
		/*
		void CPU::opcode7xnn(){
			1b34:	53                   	push   ebx
			1b35:	8b 54 24 08          	mov    edx,DWORD PTR [esp+0x8]
			VX += NN;
			1b39:	0f b7 9a 9a 00 00 00 	movzx  ebx,WORD PTR [edx+0x9a]
			1b40:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1b47:	0f b6 c4             	movzx  eax,ah
			1b4a:	83 e0 0f             	and    eax,0xf
			1b4d:	0f b6 8c 02 c2 00 00 	movzx  ecx,BYTE PTR [edx+eax*1+0xc2]
			1b54:	00 
			1b55:	01 d9                	add    ecx,ebx
			1b57:	88 8c 02 c2 00 00 00 	mov    BYTE PTR [edx+eax*1+0xc2],cl
		}
			1b5e:	5b                   	pop    ebx
			1b5f:	c3                   	ret  
		*/

		//vx = eax, nn = ecx
		X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, true, Byte);

		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(nn));

		X86Emitter::parse(&memoryBlock->cache, "add eax, ecx");

		X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, true, Byte);

		incrementPC();
		//VX += NN;
	}
	void opcode8xy0(){
		/*
		void CPU::opcode8xy0(){
			1b60:	8b 4c 24 04          	mov    ecx,DWORD PTR [esp+0x4]
			VX = VY;
			1b64:	0f b7 91 9a 00 00 00 	movzx  edx,WORD PTR [ecx+0x9a]
			1b6b:	c1 ea 04             	shr    edx,0x4
			1b6e:	83 e2 0f             	and    edx,0xf
			1b71:	0f b7 81 9a 00 00 00 	movzx  eax,WORD PTR [ecx+0x9a]
			1b78:	0f b6 c4             	movzx  eax,ah
			1b7b:	83 e0 0f             	and    eax,0xf
			1b7e:	0f b6 94 11 c2 00 00 	movzx  edx,BYTE PTR [ecx+edx*1+0xc2]
			1b85:	00 
			1b86:	88 94 01 c2 00 00 00 	mov    BYTE PTR [ecx+eax*1+0xc2],dl
		}
			1b8d:	c3                   	ret  
		*/

		X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, true, Byte);

		X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, true, Byte);
		
		incrementPC();
		//VX = VY;
	}
	void opcode8xy1(){
		/*
		void CPU::opcode8xy1(){
			1b8e:	53                   	push   ebx
			1b8f:	8b 54 24 08          	mov    edx,DWORD PTR [esp+0x8]
			VX |= VY;
			1b93:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1b9a:	c1 e8 04             	shr    eax,0x4
			1b9d:	83 e0 0f             	and    eax,0xf
			1ba0:	0f b6 9c 02 c2 00 00 	movzx  ebx,BYTE PTR [edx+eax*1+0xc2]
			1ba7:	00 
			1ba8:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1baf:	0f b6 c4             	movzx  eax,ah
			1bb2:	83 e0 0f             	and    eax,0xf
			1bb5:	0f b6 8c 02 c2 00 00 	movzx  ecx,BYTE PTR [edx+eax*1+0xc2]
			1bbc:	00 
			1bbd:	09 d9                	or     ecx,ebx
			1bbf:	88 8c 02 c2 00 00 00 	mov    BYTE PTR [edx+eax*1+0xc2],cl
		}
			1bc6:	5b                   	pop    ebx
			1bc7:	c3                   	ret   
		*/

		//vx = eax, vy = ecx
		X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, true, Byte);

		X86Emitter::parse(&memoryBlock->cache, "mov ecx, eax");

		X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, true, Byte);

		X86Emitter::parse(&memoryBlock->cache, "or eax, ecx");

		X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, true, Byte);

		incrementPC();
		//VX |= VY;
	}
	void opcode8xy2(){
		/*
		void CPU::opcode8xy2(){
			1bc8:	53                   	push   ebx
			1bc9:	8b 54 24 08          	mov    edx,DWORD PTR [esp+0x8]
			VX &= VY;
			1bcd:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1bd4:	c1 e8 04             	shr    eax,0x4
			1bd7:	83 e0 0f             	and    eax,0xf
			1bda:	0f b6 9c 02 c2 00 00 	movzx  ebx,BYTE PTR [edx+eax*1+0xc2]
			1be1:	00 
			1be2:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1be9:	0f b6 c4             	movzx  eax,ah
			1bec:	83 e0 0f             	and    eax,0xf
			1bef:	0f b6 8c 02 c2 00 00 	movzx  ecx,BYTE PTR [edx+eax*1+0xc2]
			1bf6:	00 
			1bf7:	21 d9                	and    ecx,ebx
			1bf9:	88 8c 02 c2 00 00 00 	mov    BYTE PTR [edx+eax*1+0xc2],cl
		}
			1c00:	5b                   	pop    ebx
			1c01:	c3                   	ret
		*/

		//vx = eax, vy = ecx
		X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, true, Byte);

		X86Emitter::parse(&memoryBlock->cache, "mov ecx, eax");

		X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, true, Byte);

		X86Emitter::parse(&memoryBlock->cache, "and eax, ecx");

		X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, true, Byte);

		incrementPC();
		//VX &= VY;
	}
	void opcode8xy3(){
		/*
		void CPU::opcode8xy3(){
			1c02:	53                   	push   ebx
			1c03:	8b 54 24 08          	mov    edx,DWORD PTR [esp+0x8]
			VX ^= VY;
			1c07:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1c0e:	c1 e8 04             	shr    eax,0x4
			1c11:	83 e0 0f             	and    eax,0xf
			1c14:	0f b6 9c 02 c2 00 00 	movzx  ebx,BYTE PTR [edx+eax*1+0xc2]
			1c1b:	00 
			1c1c:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1c23:	0f b6 c4             	movzx  eax,ah
			1c26:	83 e0 0f             	and    eax,0xf
			1c29:	0f b6 8c 02 c2 00 00 	movzx  ecx,BYTE PTR [edx+eax*1+0xc2]
			1c30:	00 
			1c31:	31 d9                	xor    ecx,ebx
			1c33:	88 8c 02 c2 00 00 00 	mov    BYTE PTR [edx+eax*1+0xc2],cl
		}
			1c3a:	5b                   	pop    ebx
			1c3b:	c3                   	ret    
		*/

		//vx = eax, vy = ecx
		X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, true, Byte);

		X86Emitter::parse(&memoryBlock->cache, "mov ecx, eax");

		X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, true, Byte);

		X86Emitter::parse(&memoryBlock->cache, "xor eax, ecx");

		X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, true, Byte);

		incrementPC();
		//VX ^= VY;
	}
	void opcode8xy4(){
		/*
		void CPU::opcode8xy4(){
			1c3c:	53                   	push   ebx
			1c3d:	8b 44 24 08          	mov    eax,DWORD PTR [esp+0x8]
			VF = (VX + VY > 0xff);
			1c41:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			1c48:	0f b6 d6             	movzx  edx,dh
			1c4b:	83 e2 0f             	and    edx,0xf
			1c4e:	0f b6 94 10 c2 00 00 	movzx  edx,BYTE PTR [eax+edx*1+0xc2]
			1c55:	00 
			1c56:	0f b7 88 9a 00 00 00 	movzx  ecx,WORD PTR [eax+0x9a]
			1c5d:	c1 e9 04             	shr    ecx,0x4
			1c60:	83 e1 0f             	and    ecx,0xf
			1c63:	0f b6 8c 08 c2 00 00 	movzx  ecx,BYTE PTR [eax+ecx*1+0xc2]
			1c6a:	00 
			1c6b:	0f b6 d2             	movzx  edx,dl
			1c6e:	0f b6 c9             	movzx  ecx,cl
			1c71:	01 ca                	add    edx,ecx
			1c73:	81 fa ff 00 00 00    	cmp    edx,0xff
			1c79:	0f 9f c2             	setg   dl
			1c7c:	88 90 d1 00 00 00    	mov    BYTE PTR [eax+0xd1],dl
			VX += VY;
			1c82:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			1c89:	c1 ea 04             	shr    edx,0x4
			1c8c:	83 e2 0f             	and    edx,0xf
			1c8f:	0f b6 9c 10 c2 00 00 	movzx  ebx,BYTE PTR [eax+edx*1+0xc2]
			1c96:	00 
			1c97:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			1c9e:	0f b6 d6             	movzx  edx,dh
			1ca1:	83 e2 0f             	and    edx,0xf
			1ca4:	0f b6 8c 10 c2 00 00 	movzx  ecx,BYTE PTR [eax+edx*1+0xc2]
			1cab:	00 
			1cac:	01 d9                	add    ecx,ebx
			1cae:	88 8c 10 c2 00 00 00 	mov    BYTE PTR [eax+edx*1+0xc2],cl
		}
			1cb5:	5b                   	pop    ebx
			1cb6:	c3                   	ret    
			1cb7:	90                   	nop
		*/


		//ebx = vx + vy
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");
		
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, eax");
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
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
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vfPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");
		

		//vx += vy
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::parse(&memoryBlock->cache, "mov ebx, eax");

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Add(&memoryBlock->cache, dwordAddMode, Breg, Areg);

		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		incrementPC();
		//VF = (VX + VY > 0xff) ? 0x1 : 0x0; VX += VY;
	}
	void opcode8xy5(){
		/*
		void CPU::opcode8xy5(){
			1cb8:	53                   	push   ebx
			1cb9:	8b 44 24 08          	mov    eax,DWORD PTR [esp+0x8]
			VF = !(VX < VY);
			1cbd:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			1cc4:	0f b6 d6             	movzx  edx,dh
			1cc7:	83 e2 0f             	and    edx,0xf
			1cca:	0f b6 8c 10 c2 00 00 	movzx  ecx,BYTE PTR [eax+edx*1+0xc2]
			1cd1:	00 
			1cd2:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			1cd9:	c1 ea 04             	shr    edx,0x4
			1cdc:	83 e2 0f             	and    edx,0xf
			1cdf:	0f b6 94 10 c2 00 00 	movzx  edx,BYTE PTR [eax+edx*1+0xc2]
			1ce6:	00 
			1ce7:	38 d1                	cmp    cl,dl
			1ce9:	0f 93 c2             	setae  dl
			1cec:	88 90 d1 00 00 00    	mov    BYTE PTR [eax+0xd1],dl
			VX -= VY;
			1cf2:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			1cf9:	c1 ea 04             	shr    edx,0x4
			1cfc:	83 e2 0f             	and    edx,0xf
			1cff:	0f b6 9c 10 c2 00 00 	movzx  ebx,BYTE PTR [eax+edx*1+0xc2]
			1d06:	00 
			1d07:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			1d0e:	0f b6 d6             	movzx  edx,dh
			1d11:	83 e2 0f             	and    edx,0xf
			1d14:	0f b6 8c 10 c2 00 00 	movzx  ecx,BYTE PTR [eax+edx*1+0xc2]
			1d1b:	00 
			1d1c:	29 d9                	sub    ecx,ebx
			1d1e:	88 8c 10 c2 00 00 00 	mov    BYTE PTR [eax+edx*1+0xc2],cl
		}
			1d25:	5b                   	pop    ebx
			1d26:	c3                   	ret    
			1d27:	90                   	nop
		*/

		//ebx = vy
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::parse(&memoryBlock->cache, "mov ebx, eax");

		//ecx = vx
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
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
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vfPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		//vx -= vy
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Mov(&memoryBlock->cache, movDwordRegToRegMode, Areg, Breg);

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Sub(&memoryBlock->cache, dwordSubMode, Breg, Areg);

		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		incrementPC();
		//VF = (VX < VY) ? 0x0 : 0x1; VX -= VY;
	}
	void opcode8xy6(){
		/*
		void CPU::opcode8xy6(){
			1d28:	8b 44 24 04          	mov    eax,DWORD PTR [esp+0x4]
			VF = VX << 7;
			1d2c:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			1d33:	0f b6 d6             	movzx  edx,dh
			1d36:	83 e2 0f             	and    edx,0xf
			1d39:	0f b6 94 10 c2 00 00 	movzx  edx,BYTE PTR [eax+edx*1+0xc2]
			1d40:	00 
			1d41:	c1 e2 07             	shl    edx,0x7
			1d44:	88 90 d1 00 00 00    	mov    BYTE PTR [eax+0xd1],dl
			VF >>= 7;
			1d4a:	0f b6 90 d1 00 00 00 	movzx  edx,BYTE PTR [eax+0xd1]
			1d51:	0f b6 d2             	movzx  edx,dl
			1d54:	c1 fa 07             	sar    edx,0x7
			1d57:	88 90 d1 00 00 00    	mov    BYTE PTR [eax+0xd1],dl
			VX >>= 1;
			1d5d:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			1d64:	0f b6 d6             	movzx  edx,dh
			1d67:	83 e2 0f             	and    edx,0xf
			1d6a:	0f b6 8c 10 c2 00 00 	movzx  ecx,BYTE PTR [eax+edx*1+0xc2]
			1d71:	00 
			1d72:	0f b6 c9             	movzx  ecx,cl
			1d75:	d1 f9                	sar    ecx,1
			1d77:	88 8c 10 c2 00 00 00 	mov    BYTE PTR [eax+edx*1+0xc2],cl
		}
			1d7e:	c3                   	ret    
			1d7f:	90                   	nop
		*/


		//vf = vx << 7
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Shift(&memoryBlock->cache, dwordShiftLeftMode, insertDisp(7), Areg);
		X86Emitter::Movzx(&memoryBlock->cache, movzxByteToDwordMode, Areg, Areg);//make sure to cut off
		
		//vf >>= 7
		X86Emitter::Shift(&memoryBlock->cache, dwordShiftRightMode, insertDisp(7), Areg);
		
		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vfPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vfPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		//vx >>= 1
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Shift(&memoryBlock->cache, dwordShiftRightMode, insertDisp(1), Areg);

		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		incrementPC();
		//VF = VX << 7; VF >>= 7; VX >>= 1;
	}
	void opcode8xy7(){
		/*
		void CPU::opcode8xy7(){
			1d80:	53                   	push   ebx
			1d81:	8b 44 24 08          	mov    eax,DWORD PTR [esp+0x8]
			VF = !(VY < VX);
			1d85:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			1d8c:	c1 ea 04             	shr    edx,0x4
			1d8f:	83 e2 0f             	and    edx,0xf
			1d92:	0f b6 8c 10 c2 00 00 	movzx  ecx,BYTE PTR [eax+edx*1+0xc2]
			1d99:	00 
			1d9a:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			1da1:	0f b6 d6             	movzx  edx,dh
			1da4:	83 e2 0f             	and    edx,0xf
			1da7:	0f b6 94 10 c2 00 00 	movzx  edx,BYTE PTR [eax+edx*1+0xc2]
			1dae:	00 
			1daf:	38 d1                	cmp    cl,dl
			1db1:	0f 93 c2             	setae  dl
			1db4:	88 90 d1 00 00 00    	mov    BYTE PTR [eax+0xd1],dl
			VX = VY - VX;
			1dba:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			1dc1:	c1 ea 04             	shr    edx,0x4
			1dc4:	83 e2 0f             	and    edx,0xf
			1dc7:	0f b6 8c 10 c2 00 00 	movzx  ecx,BYTE PTR [eax+edx*1+0xc2]
			1dce:	00 
			1dcf:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			1dd6:	0f b6 d6             	movzx  edx,dh
			1dd9:	83 e2 0f             	and    edx,0xf
			1ddc:	0f b6 9c 10 c2 00 00 	movzx  ebx,BYTE PTR [eax+edx*1+0xc2]
			1de3:	00 
			1de4:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			1deb:	0f b6 d6             	movzx  edx,dh
			1dee:	83 e2 0f             	and    edx,0xf
			1df1:	29 d9                	sub    ecx,ebx
			1df3:	88 8c 10 c2 00 00 00 	mov    BYTE PTR [eax+edx*1+0xc2],cl
		}
			1dfa:	5b                   	pop    ebx
			1dfb:	c3                   	ret  
		*/

		//ebx = vy
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Mov(&memoryBlock->cache, movDwordRegToRegMode, Areg, Breg);

		//ecx = vx
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
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
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vfPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		//vx -= vy
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Mov(&memoryBlock->cache, movDwordRegToRegMode, Areg, Breg);

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Sub(&memoryBlock->cache, dwordSubMode, Breg, Areg);

		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		incrementPC();
		//VF = (VY < VX) ? 0x0 : 0x1; VX = VY - VX;
	}
	void opcode8xye(){
		/*
		void CPU::opcode8xye(){
			1dfc:	8b 54 24 04          	mov    edx,DWORD PTR [esp+0x4]
			VF = VX >> 7;
			1e00:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1e07:	0f b6 c4             	movzx  eax,ah
			1e0a:	83 e0 0f             	and    eax,0xf
			1e0d:	0f b6 84 02 c2 00 00 	movzx  eax,BYTE PTR [edx+eax*1+0xc2]
			1e14:	00 
			1e15:	0f b6 c0             	movzx  eax,al
			1e18:	c1 f8 07             	sar    eax,0x7
			1e1b:	88 82 d1 00 00 00    	mov    BYTE PTR [edx+0xd1],al
			VX <<= 1;
			1e21:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1e28:	0f b6 c4             	movzx  eax,ah
			1e2b:	83 e0 0f             	and    eax,0xf
			1e2e:	0f b6 8c 02 c2 00 00 	movzx  ecx,BYTE PTR [edx+eax*1+0xc2]
			1e35:	00 
			1e36:	01 c9                	add    ecx,ecx
			1e38:	88 8c 02 c2 00 00 00 	mov    BYTE PTR [edx+eax*1+0xc2],cl
		}
			1e3f:	c3                   	ret  
		*/

		//vf = vx >> 7
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Shift(&memoryBlock->cache, dwordShiftRightMode, insertDisp(7), Areg);
		X86Emitter::Movzx(&memoryBlock->cache, movzxByteToDwordMode, Areg, Areg);//make sure to cut off

		//vx <<= 1
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Shift(&memoryBlock->cache, dwordShiftLeftMode, insertDisp(1), Areg);

		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");


		incrementPC();
		//VF = VX >> 7; VX <<= 1;
	}
	void opcode9xy0(){
		/*
		void CPU::opcode9xy0(){
			1e40:	8b 44 24 04          	mov    eax,DWORD PTR [esp+0x4]
			if (VX != VY)
			1e44:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			1e4b:	0f b6 d6             	movzx  edx,dh
			1e4e:	83 e2 0f             	and    edx,0xf
			1e51:	0f b6 8c 10 c2 00 00 	movzx  ecx,BYTE PTR [eax+edx*1+0xc2]
			1e58:	00 
			1e59:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			1e60:	c1 ea 04             	shr    edx,0x4
			1e63:	83 e2 0f             	and    edx,0xf
			1e66:	0f b6 94 10 c2 00 00 	movzx  edx,BYTE PTR [eax+edx*1+0xc2]
			1e6d:	00 
			1e6e:	38 d1                	cmp    cl,dl
			1e70:	74 11                	je     1e83 <_ZN3CPU10opcode9xy0Ev+0x43>
				programCounter += 2; //skip if vx != vy
			1e72:	0f b7 90 9c 00 00 00 	movzx  edx,WORD PTR [eax+0x9c]
			1e79:	83 c2 02             	add    edx,0x2
			1e7c:	66 89 90 9c 00 00 00 	mov    WORD PTR [eax+0x9c],dx
		}
			1e83:	f3 c3                	repz ret 
			1e85:	90                   	nop
		*/

		//vx = eax, vy = edx
		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vyPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vyPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Mov(&memoryBlock->cache, movDwordRegToRegMode, Areg, Dreg);

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
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
		/*
		void CPU::opcodeannn(){
			1e86:	8b 54 24 04          	mov    edx,DWORD PTR [esp+0x4]
			indexRegister = NNN;
			1e8a:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1e91:	66 25 ff 0f          	and    ax,0xfff
			1e95:	66 89 82 a0 00 00 00 	mov    WORD PTR [edx+0xa0],ax
		}
			1e9c:	c3                   	ret    
			1e9d:	90                   	nop
		*/

		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(nnn));
		//X86Emitter::parse(&memoryBlock->cache, "mov WORD PTR [extra], eax", insertDisp(indexRegister));
		//X86Emitter::setToMemaddr(&memoryBlock->cache, indexRegister, nnn, Word);

		incrementPC();
		//indexRegister = NNN;
	}
	void opcodebnnn(){
		/*
		void CPU::opcodebnnn(){
			1e9e:	8b 54 24 04          	mov    edx,DWORD PTR [esp+0x4]
			programCounter = NNN + v[0];
			1ea2:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1ea9:	0f b6 8a c2 00 00 00 	movzx  ecx,BYTE PTR [edx+0xc2]
			1eb0:	66 25 ff 0f          	and    ax,0xfff
			1eb4:	0f b6 c9             	movzx  ecx,cl
			1eb7:	01 c8                	add    eax,ecx
			1eb9:	66 89 82 9c 00 00 00 	mov    WORD PTR [edx+0x9c],ax
			flag = 1; //(dont increment pc)
			1ec0:	c7 82 dc 19 08 00 01 	mov    DWORD PTR [edx+0x819dc],0x1
			1ec7:	00 00 00 
		}
			1eca:	c3                   	ret    
			1ecb:	90                   	nop
		*/

		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(nnn));
		X86Emitter::parse(&memoryBlock->cache, "mov WORD PTR [extra], eax", insertDisp(programCounter));
		//X86Emitter::setToMemaddr(&memoryBlock->cache, programCounter, nnn, Word);
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, BYTE PTR [extra]", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "add WORD PTR [extra], ebx", insertDisp(programCounter));
		//X86Emitter::addToMemaddr(&memoryBlock->cache, programCounter, v, Word);

		//programCounter = NNN + v[0]; flag = 1; //(dont increment pc)
	}
	void opcodecxnn(){
		/*
		void CPU::opcodecxnn(){
			2092:	56                   	push   esi
			2093:	53                   	push   ebx
			2094:	83 ec 04             	sub    esp,0x4
			2097:	e8 14 f7 ff ff       	call   17b0 <__x86.get_pc_thunk.bx>
			209c:	81 c3 2c 3e 01 00    	add    ebx,0x13e2c
			20a2:	8b 74 24 10          	mov    esi,DWORD PTR [esp+0x10]
			VX = (rand() % 0x100) & NN;	//random
			20a6:	e8 05 f5 ff ff       	call   15b0 <rand@plt>
			20ab:	0f b7 8e 9a 00 00 00 	movzx  ecx,WORD PTR [esi+0x9a]
			20b2:	0f b7 96 9a 00 00 00 	movzx  edx,WORD PTR [esi+0x9a]
			20b9:	0f b6 d6             	movzx  edx,dh
			20bc:	83 e2 0f             	and    edx,0xf
			20bf:	21 c8                	and    eax,ecx
			20c1:	88 84 16 c2 00 00 00 	mov    BYTE PTR [esi+edx*1+0xc2],al
		}
			20c8:	83 c4 04             	add    esp,0x4
			20cb:	5b                   	pop    ebx
			20cc:	5e                   	pop    esi
			20cd:	c3                   	ret 
		*/

		hintFallback_func();
		interpreterSwitch_func();
		//incrementPC();
		//VX = (rand() % 0x100) & NN;	//random
	}
	void opcodedxyn(){
		/*
		void CPU::opcodedxyn(){
			controllerOp = ControllerOp::drawVideo;
			1ecc:	8b 44 24 04          	mov    eax,DWORD PTR [esp+0x4]
			1ed0:	c7 80 d4 19 08 00 02 	mov    DWORD PTR [eax+0x819d4],0x2
			1ed7:	00 00 00 
		}
			1eda:	c3                   	ret    
			1edb:	90                   	nop
		*/

		X86Emitter::setToMemaddr(&memoryBlock->cache, controllerOp, ControllerOp::drawVideo, Dword);

		incrementPC();
		delayNext_func();
		hintFallback_func();
		interpreterSwitch_func();
		//controllerOp = ControllerOp::drawVideo;
	}
	void opcodeex9e(){
		/*
		void CPU::opcodeex9e(){
			1edc:	8b 54 24 04          	mov    edx,DWORD PTR [esp+0x4]
			if (*pressedKey == VX)
			1ee0:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1ee7:	0f b6 c4             	movzx  eax,ah
			1eea:	83 e0 0f             	and    eax,0xf
			1eed:	0f b6 84 02 c2 00 00 	movzx  eax,BYTE PTR [edx+eax*1+0xc2]
			1ef4:	00 
			1ef5:	8b 8a e8 19 08 00    	mov    ecx,DWORD PTR [edx+0x819e8]
			1efb:	38 01                	cmp    BYTE PTR [ecx],al
			1efd:	74 02                	je     1f01 <_ZN3CPU10opcodeex9eEv+0x25>
				programCounter += 2;
		}
			1eff:	f3 c3                	repz ret 
				programCounter += 2;
			1f01:	0f b7 82 9c 00 00 00 	movzx  eax,WORD PTR [edx+0x9c]
			1f08:	83 c0 02             	add    eax,0x2
			1f0b:	66 89 82 9c 00 00 00 	mov    WORD PTR [edx+0x9c],ax
		}
			1f12:	eb eb                	jmp    1eff <_ZN3CPU10opcodeex9eEv+0x23>
		*/

		X86Emitter::Mov(&memoryBlock->cache, movFromMemaddrByteMode, Breg, insertAddr(pressedKey));
		X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, stack, stackPointer, true, Word);
		X86Emitter::Cmp(&memoryBlock->cache, cmpMode, Areg, Breg);
		X86Emitter::parse(&memoryBlock->cache, "jne extra", insertDisp(addWordToMemaddrSize));
		//X86Emitter::Jcc(&memoryBlock->cache, byteRelJneMode, insertDisp(addWordToMemaddrSize));
		X86Emitter::addToMemaddr(&memoryBlock->cache, programCounter, 2, Word);

		incrementPC();
		//if (*pressedKey == VX) programCounter += 2;
	}
	void opcodeexa1(){
		/*
		void CPU::opcodeexa1(){
			1f14:	8b 54 24 04          	mov    edx,DWORD PTR [esp+0x4]
			if (*pressedKey != VX)
			1f18:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1f1f:	0f b6 c4             	movzx  eax,ah
			1f22:	83 e0 0f             	and    eax,0xf
			1f25:	0f b6 84 02 c2 00 00 	movzx  eax,BYTE PTR [edx+eax*1+0xc2]
			1f2c:	00 
			1f2d:	8b 8a e8 19 08 00    	mov    ecx,DWORD PTR [edx+0x819e8]
			1f33:	38 01                	cmp    BYTE PTR [ecx],al
			1f35:	74 11                	je     1f48 <_ZN3CPU10opcodeexa1Ev+0x34>
				programCounter += 2;
			1f37:	0f b7 82 9c 00 00 00 	movzx  eax,WORD PTR [edx+0x9c]
			1f3e:	83 c0 02             	add    eax,0x2
			1f41:	66 89 82 9c 00 00 00 	mov    WORD PTR [edx+0x9c],ax
		}
			1f48:	f3 c3                	repz ret 
		*/

		X86Emitter::Mov(&memoryBlock->cache, movFromMemaddrByteMode, Breg, insertAddr(pressedKey));
		X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, stack, stackPointer, true, Word);
		X86Emitter::Cmp(&memoryBlock->cache, cmpMode, Areg, Breg);
		X86Emitter::parse(&memoryBlock->cache, "je extra", insertDisp(addWordToMemaddrSize));
		//X86Emitter::Jcc(&memoryBlock->cache, byteRelJeMode, insertDisp(addWordToMemaddrSize));
		addToMemaddr(&memoryBlock->cache, programCounter, 2, Word);

		incrementPC();
		//if (*pressedKey != VX) programCounter += 2;
	}
	void opcodefx07(){
		/*
		void CPU::opcodefx07(){
			1f4a:	8b 54 24 04          	mov    edx,DWORD PTR [esp+0x4]
			VX = *delayRegister;
			1f4e:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1f55:	0f b6 c4             	movzx  eax,ah
			1f58:	83 e0 0f             	and    eax,0xf
			1f5b:	8b 8a e4 19 08 00    	mov    ecx,DWORD PTR [edx+0x819e4]
			1f61:	0f b6 09             	movzx  ecx,BYTE PTR [ecx]
			1f64:	88 8c 02 c2 00 00 00 	mov    BYTE PTR [edx+eax*1+0xc2],cl
		}
			1f6b:	c3                   	ret 
		*/
		X86Emitter::Mov(&memoryBlock->cache, movFromMemaddrByteMode, Areg, insertAddr(delayRegister));
		
		//X86Emitter::storeArray_AregAsInput(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov ecx, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, ecx");
		X86Emitter::parse(&memoryBlock->cache, "mov BYTE PTR [ebx], eax");

		incrementPC();
		//VX = *delayRegister;
	}
	void opcodefx0a(){
		/*
		void CPU::opcodefx0a(){
			1f6c:	8b 44 24 04          	mov    eax,DWORD PTR [esp+0x4]
			if (Input::isKeyPressed(*pressedKey) == true)
			1f70:	8b 90 e8 19 08 00    	mov    edx,DWORD PTR [eax+0x819e8]
			1f76:	0f b6 0a             	movzx  ecx,BYTE PTR [edx]
			1f79:	3a 88 99 00 00 00    	cmp    cl,BYTE PTR [eax+0x99]
			1f7f:	74 15                	je     1f96 <_ZN3CPU10opcodefx0aEv+0x2a>
				VX = *pressedKey;
			1f81:	0f b7 90 9a 00 00 00 	movzx  edx,WORD PTR [eax+0x9a]
			1f88:	0f b6 d6             	movzx  edx,dh
			1f8b:	83 e2 0f             	and    edx,0xf
			1f8e:	88 8c 10 c2 00 00 00 	mov    BYTE PTR [eax+edx*1+0xc2],cl
			1f95:	c3                   	ret    
			else
				flag = 1; //wait again	(dont increment pc)
			1f96:	c7 80 dc 19 08 00 01 	mov    DWORD PTR [eax+0x819dc],0x1
			1f9d:	00 00 00 
		}
			1fa0:	c3                   	ret    
			1fa1:	90                   	nop
		*/

		hintFallback_func();
		interpreterSwitch_func();
		//incrementPC();
		//if (Input::isKeyPressed(*pressedKey) == true) VX = *pressedKey; else flag = 1; //wait again	(dont increment pc)
	}
	void opcodefx15(){
		/*
		void CPU::opcodefx15(){
			1fa2:	8b 54 24 04          	mov    edx,DWORD PTR [esp+0x4]
			*delayRegister = VX;
			1fa6:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1fad:	0f b6 c4             	movzx  eax,ah
			1fb0:	83 e0 0f             	and    eax,0xf
			1fb3:	8b 8a e4 19 08 00    	mov    ecx,DWORD PTR [edx+0x819e4]
			1fb9:	0f b6 84 02 c2 00 00 	movzx  eax,BYTE PTR [edx+eax*1+0xc2]
			1fc0:	00 
			1fc1:	88 01                	mov    BYTE PTR [ecx],al
		}
			1fc3:	c3                   	ret  
		*/

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Mov(&memoryBlock->cache, movToMemaddrByteMode, Areg, insertAddr(delayRegister));
		
		incrementPC();
		//*delayRegister = VX;
	}
	void opcodefx18(){
		/*
		void CPU::opcodefx18(){
			controllerOp = ControllerOp::setSoundTimer;
			1fc4:	8b 44 24 04          	mov    eax,DWORD PTR [esp+0x4]
			1fc8:	c7 80 d4 19 08 00 03 	mov    DWORD PTR [eax+0x819d4],0x3
			1fcf:	00 00 00 
		}
			1fd2:	c3                   	ret    
			1fd3:	90                   	nop
		*/


		X86Emitter::setToMemaddr(&memoryBlock->cache, controllerOp, ControllerOp::setSoundTimer, Dword);
		//X86Emitter::setDwordToMemaddr(&memoryBlock->cache, controllerOp, (uint32_t)ControllerOp::clearScreen);

		incrementPC();
		delayNext_func();
		hintFallback_func();
		interpreterSwitch_func();
		//controllerOp = ControllerOp::setSoundTimer;
	}
	void opcodefx1e(){
		/*
		void CPU::opcodefx1e(){
			1fd4:	8b 54 24 04          	mov    edx,DWORD PTR [esp+0x4]
			indexRegister += VX;
			1fd8:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			1fdf:	0f b6 c4             	movzx  eax,ah
			1fe2:	83 e0 0f             	and    eax,0xf
			1fe5:	0f b6 84 02 c2 00 00 	movzx  eax,BYTE PTR [edx+eax*1+0xc2]
			1fec:	00 
			1fed:	0f b7 8a a0 00 00 00 	movzx  ecx,WORD PTR [edx+0xa0]
			1ff4:	0f b6 c0             	movzx  eax,al
			1ff7:	01 c8                	add    eax,ecx
			1ff9:	66 89 82 a0 00 00 00 	mov    WORD PTR [edx+0xa0],ax
		}
			2000:	c3                   	ret    
			2001:	90                   	nop
		*/

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::parse(&memoryBlock->cache, "add WORD PTR [extra], eax", insertAddr(indexRegister));

		incrementPC();
		//indexRegister += VX;
	}
	void opcodefx29(){
		/*
		void CPU::opcodefx29(){
			2002:	8b 54 24 04          	mov    edx,DWORD PTR [esp+0x4]
			indexRegister = VX * 5;	//font is stored at mem[0 ~ FONT_COUNT * 5]
			2006:	0f b7 82 9a 00 00 00 	movzx  eax,WORD PTR [edx+0x9a]
			200d:	0f b6 c4             	movzx  eax,ah
			2010:	83 e0 0f             	and    eax,0xf
			2013:	0f b6 84 02 c2 00 00 	movzx  eax,BYTE PTR [edx+eax*1+0xc2]
			201a:	00 
			201b:	0f b6 c0             	movzx  eax,al
			201e:	8d 04 80             	lea    eax,[eax+eax*4]
			2021:	66 89 82 a0 00 00 00 	mov    WORD PTR [edx+0xa0],ax
		}
			2028:	c3                   	ret    
			2029:	90                   	nop
		*/

		//X86Emitter::loadArray_AregAsResult(&memoryBlock->cache, v, vxPointer, Byte);
		X86Emitter::parse(&memoryBlock->cache, "mov eax, extra", insertDisp(v));
		X86Emitter::parse(&memoryBlock->cache, "mov ebx, extra", insertDisp(vxPointer));
		X86Emitter::parse(&memoryBlock->cache, "add ebx, eax");
		X86Emitter::parse(&memoryBlock->cache, "mov eax, BYTE PTR [ebx]");

		X86Emitter::Lea(&memoryBlock->cache, leaWithoutDispMode, Areg, x4, Areg, Areg);
		X86Emitter::parse(&memoryBlock->cache, "mov WORD PTR [extra], eax", insertAddr(indexRegister));

		incrementPC();
		//indexRegister = VX * 5;	//font is stored at mem[0 ~ FONT_COUNT * 5]
	}
	void opcodefx33(){
		/*
		void CPU::opcodefx33(){
			20ce:	57                   	push   edi
			20cf:	56                   	push   esi
			20d0:	53                   	push   ebx
			20d1:	e8 da f6 ff ff       	call   17b0 <__x86.get_pc_thunk.bx>
			20d6:	81 c3 f2 3d 01 00    	add    ebx,0x13df2
			20dc:	8b 74 24 10          	mov    esi,DWORD PTR [esp+0x10]
			memory->write(indexRegister, VX / 100);
			20e0:	0f b7 86 9a 00 00 00 	movzx  eax,WORD PTR [esi+0x9a]
			20e7:	0f b6 c4             	movzx  eax,ah
			20ea:	83 e0 0f             	and    eax,0xf
			20ed:	0f b6 8c 06 c2 00 00 	movzx  ecx,BYTE PTR [esi+eax*1+0xc2]
			20f4:	00 
			20f5:	0f b7 96 a0 00 00 00 	movzx  edx,WORD PTR [esi+0xa0]
			20fc:	83 ec 04             	sub    esp,0x4
			20ff:	0f b6 c9             	movzx  ecx,cl
			2102:	8d 04 89             	lea    eax,[ecx+ecx*4]
			2105:	8d 04 c1             	lea    eax,[ecx+eax*8]
			2108:	66 c1 e8 0c          	shr    ax,0xc
			210c:	0f b6 c0             	movzx  eax,al
			210f:	50                   	push   eax
			2110:	0f b7 d2             	movzx  edx,dx
			2113:	52                   	push   edx
			2114:	ff b6 e0 19 08 00    	push   DWORD PTR [esi+0x819e0]
			211a:	e8 69 cf 00 00       	call   f088 <_ZN6Memory5writeEth>
			memory->write(indexRegister + 1, (VX / 10) % 10);
			211f:	0f b7 86 9a 00 00 00 	movzx  eax,WORD PTR [esi+0x9a]
			2126:	0f b6 c4             	movzx  eax,ah
			2129:	83 e0 0f             	and    eax,0xf
			212c:	0f b6 94 06 c2 00 00 	movzx  edx,BYTE PTR [esi+eax*1+0xc2]
			2133:	00 
			2134:	0f b7 8e a0 00 00 00 	movzx  ecx,WORD PTR [esi+0xa0]
			213b:	83 c4 0c             	add    esp,0xc
			213e:	0f b6 d2             	movzx  edx,dl
			2141:	8d 04 92             	lea    eax,[edx+edx*4]
			2144:	8d 04 c2             	lea    eax,[edx+eax*8]
			2147:	8d 04 80             	lea    eax,[eax+eax*4]
			214a:	66 c1 e8 0b          	shr    ax,0xb
			214e:	0f b6 f8             	movzx  edi,al
			2151:	8d 14 bf             	lea    edx,[edi+edi*4]
			2154:	8d 14 d7             	lea    edx,[edi+edx*8]
			2157:	8d 14 92             	lea    edx,[edx+edx*4]
			215a:	66 c1 ea 0b          	shr    dx,0xb
			215e:	8d 14 92             	lea    edx,[edx+edx*4]
			2161:	01 d2                	add    edx,edx
			2163:	29 d0                	sub    eax,edx
			2165:	0f b6 c0             	movzx  eax,al
			2168:	50                   	push   eax
			2169:	83 c1 01             	add    ecx,0x1
			216c:	0f b7 c9             	movzx  ecx,cx
			216f:	51                   	push   ecx
			2170:	ff b6 e0 19 08 00    	push   DWORD PTR [esi+0x819e0]
			2176:	e8 0d cf 00 00       	call   f088 <_ZN6Memory5writeEth>
			memory->write(indexRegister + 2, VX % 10);
			217b:	0f b7 86 9a 00 00 00 	movzx  eax,WORD PTR [esi+0x9a]
			2182:	0f b6 c4             	movzx  eax,ah
			2185:	83 e0 0f             	and    eax,0xf
			2188:	0f b6 94 06 c2 00 00 	movzx  edx,BYTE PTR [esi+eax*1+0xc2]
			218f:	00 
			2190:	0f b7 8e a0 00 00 00 	movzx  ecx,WORD PTR [esi+0xa0]
			2197:	83 c4 0c             	add    esp,0xc
			219a:	0f b6 fa             	movzx  edi,dl
			219d:	8d 04 bf             	lea    eax,[edi+edi*4]
			21a0:	8d 04 c7             	lea    eax,[edi+eax*8]
			21a3:	8d 04 80             	lea    eax,[eax+eax*4]
			21a6:	66 c1 e8 0b          	shr    ax,0xb
			21aa:	8d 04 80             	lea    eax,[eax+eax*4]
			21ad:	01 c0                	add    eax,eax
			21af:	29 c2                	sub    edx,eax
			21b1:	0f b6 d2             	movzx  edx,dl
			21b4:	52                   	push   edx
			21b5:	83 c1 02             	add    ecx,0x2
			21b8:	0f b7 c9             	movzx  ecx,cx
			21bb:	51                   	push   ecx
			21bc:	ff b6 e0 19 08 00    	push   DWORD PTR [esi+0x819e0]
			21c2:	e8 c1 ce 00 00       	call   f088 <_ZN6Memory5writeEth>
		}
			21c7:	83 c4 10             	add    esp,0x10
			21ca:	5b                   	pop    ebx
			21cb:	5e                   	pop    esi
			21cc:	5f                   	pop    edi
			21cd:	c3                   	ret    
		*/

		hintFallback_func();
		interpreterSwitch_func();
		//incrementPC();
		//memory->write(indexRegister, VX / 100);
		//memory->write(indexRegister + 1, (VX / 10) % 10);
		//memory->write(indexRegister + 2, VX % 10);
	}
	void opcodefx55(){
		/*
		void CPU::opcodefx55(){
			21ce:	57                   	push   edi
			21cf:	56                   	push   esi
			21d0:	53                   	push   ebx
			21d1:	e8 da f5 ff ff       	call   17b0 <__x86.get_pc_thunk.bx>
			21d6:	81 c3 f2 3c 01 00    	add    ebx,0x13cf2
			21dc:	8b 7c 24 10          	mov    edi,DWORD PTR [esp+0x10]
			for (int i = 0; i <= (currentOpcode & 0x0f00) >> 8; i++)
			21e0:	0f b7 87 9a 00 00 00 	movzx  eax,WORD PTR [edi+0x9a]
			21e7:	be 00 00 00 00       	mov    esi,0x0
				memory->write(indexRegister + i, v[i]);
			21ec:	0f b6 94 37 c2 00 00 	movzx  edx,BYTE PTR [edi+esi*1+0xc2]
			21f3:	00 
			21f4:	0f b7 87 a0 00 00 00 	movzx  eax,WORD PTR [edi+0xa0]
			21fb:	83 ec 04             	sub    esp,0x4
			21fe:	0f b6 d2             	movzx  edx,dl
			2201:	52                   	push   edx
			2202:	01 f0                	add    eax,esi
			2204:	0f b7 c0             	movzx  eax,ax
			2207:	50                   	push   eax
			2208:	ff b7 e0 19 08 00    	push   DWORD PTR [edi+0x819e0]
			220e:	e8 75 ce 00 00       	call   f088 <_ZN6Memory5writeEth>
			for (int i = 0; i <= (currentOpcode & 0x0f00) >> 8; i++)
			2213:	83 c6 01             	add    esi,0x1
			2216:	0f b7 87 9a 00 00 00 	movzx  eax,WORD PTR [edi+0x9a]
			221d:	0f b6 c4             	movzx  eax,ah
			2220:	83 e0 0f             	and    eax,0xf
			2223:	83 c4 10             	add    esp,0x10
			2226:	39 f0                	cmp    eax,esi
			2228:	7d c2                	jge    21ec <_ZN3CPU10opcodefx55Ev+0x1e>
		}
			222a:	5b                   	pop    ebx
			222b:	5e                   	pop    esi
			222c:	5f                   	pop    edi
			222d:	c3                   	ret    
		*/

		hintFallback_func();
		interpreterSwitch_func();
		//incrementPC();
		//for (int i = 0; i <= (currentOpcode & 0x0f00) >> 8; i++) memory->write(indexRegister + i, v[i]);
	}
	void opcodefx65(){
		/*
		void CPU::opcodefx65(){
			202a:	57                   	push   edi
			202b:	56                   	push   esi
			202c:	53                   	push   ebx
			202d:	8b 4c 24 10          	mov    ecx,DWORD PTR [esp+0x10]
			for (int i = 0; i <= (currentOpcode & 0x0f00) >> 8; i++)
			2031:	0f b7 81 9a 00 00 00 	movzx  eax,WORD PTR [ecx+0x9a]
				v[i] = memory->read(indexRegister + i);
			2038:	8b b1 e0 19 08 00    	mov    esi,DWORD PTR [ecx+0x819e0]
			for (int i = 0; i <= (currentOpcode & 0x0f00) >> 8; i++)
			203e:	ba 00 00 00 00       	mov    edx,0x0


		};

		inline uint8_t Memory::read(uint16_t addr){
			if (addr >= FULL_MEM_SIZE) return 0x0;	//no OOB
			2043:	bb 00 00 00 00       	mov    ebx,0x0
			2048:	eb 1d                	jmp    2067 <_ZN3CPU10opcodefx65Ev+0x3d>
				v[i] = memory->read(indexRegister + i);
			204a:	89 f8                	mov    eax,edi
			204c:	88 84 11 c2 00 00 00 	mov    BYTE PTR [ecx+edx*1+0xc2],al
			for (int i = 0; i <= (currentOpcode & 0x0f00) >> 8; i++)
			2053:	83 c2 01             	add    edx,0x1
			2056:	0f b7 81 9a 00 00 00 	movzx  eax,WORD PTR [ecx+0x9a]
			205d:	0f b6 c4             	movzx  eax,ah
			2060:	83 e0 0f             	and    eax,0xf
			2063:	39 d0                	cmp    eax,edx
			2065:	7c 1a                	jl     2081 <_ZN3CPU10opcodefx65Ev+0x57>
				v[i] = memory->read(indexRegister + i);
			2067:	0f b7 81 a0 00 00 00 	movzx  eax,WORD PTR [ecx+0xa0]
			206e:	01 d0                	add    eax,edx
			2070:	89 df                	mov    edi,ebx
			2072:	66 3d ff 0f          	cmp    ax,0xfff
			2076:	77 d2                	ja     204a <_ZN3CPU10opcodefx65Ev+0x20>
			2078:	0f b7 c0             	movzx  eax,ax
			return mem[addr];
			207b:	0f b6 3c 06          	movzx  edi,BYTE PTR [esi+eax*1]
			207f:	eb c9                	jmp    204a <_ZN3CPU10opcodefx65Ev+0x20>
		}
			2081:	5b                   	pop    ebx
			2082:	5e                   	pop    esi
			2083:	5f                   	pop    edi
			2084:	c3                   	ret    
			2085:	90                   	nop
		*/

		hintFallback_func();
		interpreterSwitch_func();
		//incrementPC();
		//for (int i = 0; i <= (currentOpcode & 0x0f00) >> 8; i++) v[i] = memory->read(indexRegister + i);
	}

	//exception
	void opcodenull(){
		/*
		void CPU::opcodenull(){
			throwError = true;
			2086:	8b 44 24 04          	mov    eax,DWORD PTR [esp+0x4]
			208a:	c6 80 d8 19 08 00 01 	mov    BYTE PTR [eax+0x819d8],0x1
		}
			2091:	c3                   	ret
		*/

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