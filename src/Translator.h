#pragma once

/*
	Dynarec chip8 translator

*/

#include "X86Emitter.h"
#include"CPU.h"

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

	uint32_t interpreterSwitch;	//byte

	uint32_t tempx;				//dword
	uint32_t temp;				//dword

	uint16_t nx;
	uint32_t n;					//word
	uint16_t nnx;
	uint32_t nn;				//word
	uint16_t nnnx;
	uint32_t nnn;				//word

	bool endMemoryBlock = false;

	int* index;
	std::vector<uint8_t>* memoryBlock;

	//internal: do not use
	void init(std::vector<uint8_t>* memoryBlock){
		this->memoryBlock = memoryBlock;
		endMemoryBlock = false;
	}

public:

	//constructor
	Translator(CPU* cpu){
		controllerOp = (uint32_t)&cpu->controllerOp;
		programCounter = (uint32_t)&cpu->programCounter;
		stack = (uint32_t)&cpu->stack;
		stackPointer = (uint32_t)&cpu->stackPointer;
		flag = (uint32_t)&cpu->flag;
		jmpHint = (uint32_t)&cpu->jmpHint;
		v = (uint32_t)&cpu->v;
		indexRegister = (uint32_t)&cpu->indexRegister;
		pressedKey = (uint32_t)&cpu->pressedKey;
		delayRegister = (uint32_t)&cpu->delayRegister;
		throwError = (uint32_t)&cpu->throwError;
		mem = (uint32_t)&cpu->memory->mem;
		currentOpcode = (uint32_t)&cpu->currentOpcode;

		cpu->interpreterSwitch = false;
		interpreterSwitch = (uint32_t)&cpu->interpreterSwitch;

		tempx = 0;
		temp = (uint32_t)&tempx;
		nx = cpu->currentOpcode & 0x000F;
		nnx = cpu->currentOpcode & 0x00FF;
		nnnx = cpu->currentOpcode & 0x0FFF;
		n = (uint32_t)&nx;
		nn = (uint32_t)&nnx;
		nnn = (uint32_t)&nnnx;
		
	}

	//memoryblock status
	//check to see if it should end append job
	bool getEndMemoryBlock(){ return endMemoryBlock; }

	//increment program counter by 2
	void incrementPC(){
		X86Emitter::loadWordToDwordRegA(memoryBlock, programCounter);
		//+2
		X86Emitter::add_imm_to_eax(memoryBlock, 0x00000002);
		X86Emitter::mov_ax_to_memoryaddr(memoryBlock, programCounter);
	}

	//interpreterSwitch = true;
	//fallback to interpreter, switch back whenever its done. but not here
	//some opcodes are just too complicated to recreate in jit
	//...or jump
	void switchToInterpreter(){
		X86Emitter::mov_imm_to_eax(memoryBlock, 0x00000001);
		X86Emitter::mov_al_to_memoryaddr(memoryBlock, interpreterSwitch);
		endMemoryBlock = true;	//end block
	}

	//opcodes
	void opcode00e0(){
		X86Emitter::mov_imm_to_eax(memoryBlock, (uint32_t)ControllerOp::clearScreen);
		X86Emitter::mov_eax_to_memoryaddr(memoryBlock, controllerOp);

		incrementPC();
		//original
		//controllerOp = ControllerOp::clearScreen;
	}

	void opcode00ee(){
		//stack = ebx, stackptr = eax, pc = ecx
		//--stackPointer
		X86Emitter::loadByteToDwordRegA(memoryBlock, stackPointer);
		X86Emitter::dec_eax(memoryBlock);
		X86Emitter::mov_ax_to_memoryaddr(memoryBlock, stackPointer);
		//stack[stackPointer] (DO NOT DO stack + stackPointer!!!! stackPointer is not fixed)
		X86Emitter::mov_imm_to_ebx(memoryBlock, stack);
		X86Emitter::add_eax_to_ebx(memoryBlock);
		X86Emitter::mov_ebxaddr_to_ax(memoryBlock);
		X86Emitter::movzx_ax_to_eax(memoryBlock);
		//to programCounter
		X86Emitter::mov_ax_to_memoryaddr(memoryBlock, programCounter);

		incrementPC();
		switchToInterpreter();
		//programCounter = stack[--stackPointer]; //return from SUBroutine	(and increment pc after to get out of loop)
	}
	void opcode0nnn(){
		
		//stack = eax, stackptr = ebx, pc = ecx
		//stack[stackpointer]
		X86Emitter::mov_imm_to_eax(memoryBlock, stack);
		X86Emitter::loadByteToDwordRegB(memoryBlock, stackPointer);
		X86Emitter::add_ebx_to_eax(memoryBlock);
		//= programCounter
		X86Emitter::loadWordToDwordRegC(memoryBlock, programCounter);
		X86Emitter::mov_cx_to_eaxaddr(memoryBlock);
		//stackPointer++
		X86Emitter::inc_ebx(memoryBlock);
		X86Emitter::mov_bl_to_memoryaddr(memoryBlock, stackPointer);

		//NNN to programCounter
		X86Emitter::loadWordToDwordRegA(memoryBlock, nnn);
		X86Emitter::mov_ax_to_memoryaddr(memoryBlock, programCounter);

		switchToInterpreter();
		//stack[stackPointer++] = programCounter; programCounter = NNN; flag = 1;//call SUBroutine from nnn	(dont increment pc)
	}
	void opcode1nnn(){
		//NNN to programCounter
		X86Emitter::loadWordToDwordRegA(memoryBlock, nnn);
		X86Emitter::mov_ax_to_memoryaddr(memoryBlock, programCounter);

		//jmpHint = true
		X86Emitter::mov_imm_to_eax(memoryBlock, 0x00000001);
		X86Emitter::mov_al_to_memoryaddr(memoryBlock, jmpHint);

		switchToInterpreter();
		//programCounter = NNN; flag = 1;//jump to nnn	(dont increment pc)
		//jmpHint = true; //hint for video flicker loop
	}
	void opcode2nnn(){
		//stack = eax, stackptr = ebx, pc = ecx
		//stack[stackpointer]
		X86Emitter::mov_imm_to_eax(memoryBlock, stack);
		X86Emitter::loadByteToDwordRegB(memoryBlock, stackPointer);
		X86Emitter::add_ebx_to_eax(memoryBlock);
		//= programCounter
		X86Emitter::loadWordToDwordRegC(memoryBlock, programCounter);
		X86Emitter::mov_cx_to_eaxaddr(memoryBlock);
		//stackPointer++
		X86Emitter::inc_ebx(memoryBlock);
		X86Emitter::mov_bl_to_memoryaddr(memoryBlock, stackPointer);

		//NNN to programCounter
		X86Emitter::loadWordToDwordRegA(memoryBlock, nnn);
		X86Emitter::mov_ax_to_memoryaddr(memoryBlock, programCounter);

		switchToInterpreter();
		//stack[stackPointer++] = programCounter; programCounter = NNN; flag = 1;//call SUBroutine from nnn	(dont increment pc)
	}
	void opcode3xnn(){

		incrementPC();
		//if (VX == NN) programCounter += 2; //skip if ==
	}
	void opcode4xnn(){

		incrementPC();
		//if (VX != NN) programCounter += 2; //skip if !=
	}
	void opcode5xy0(){

		incrementPC();
		//if (VX == VY) programCounter += 2; //skip if vx == vy
	}
	void opcode6xnn(){

		incrementPC();
		//VX = NN; //into
	}
	void opcode7xnn(){

		incrementPC();
		//VX += NN;
	}
	void opcode8xy0(){

		incrementPC();
		//VX = VY;
	}
	void opcode8xy1(){

		incrementPC();
		//VX |= VY;
	}
	void opcode8xy2(){

		incrementPC();
		//VX &= VY;
	}
	void opcode8xy3(){

		incrementPC();
		//VX ^= VY;
	}
	void opcode8xy4(){

		incrementPC();
		//VF = (VX + VY > 0xff) ? 0x1 : 0x0; VX += VY;
	}
	void opcode8xy5(){

		incrementPC();
		//VF = (VX < VY) ? 0x0 : 0x1; VX -= VY;
	}
	void opcode8xy6(){

		incrementPC();
		//VF = VX << 7; VF >>= 7; VX >>= 1;
	}
	void opcode8xy7(){

		incrementPC();
		//VF = (VY < VX) ? 0x0 : 0x1; VX = VY - VX;
	}
	void opcode8xye(){

		incrementPC();
		//VF = VX >> 7; VX <<= 1;
	}
	void opcode9xy0(){

		incrementPC();
		//if (VX != VY) programCounter += 2; //skip if vx != vy
	}
	void opcodeannn(){

		incrementPC();
		//indexRegister = NNN;
	}
	void opcodebnnn(){

		incrementPC();
		//programCounter = NNN + v[0]; flag = 1; //(dont increment pc)
	}
	void opcodecxnn(){

		incrementPC();
		//VX = (rand() % 0x100) & NN;	//random
	}
	void opcodedxyn(){

		incrementPC();
		//controllerOp = ControllerOp::drawVideo;
	}
	void opcodeex9e(){

		incrementPC();
		//if (*pressedKey == VX) programCounter += 2;
	}
	void opcodeexa1(){

		incrementPC();
		//if (*pressedKey != VX) programCounter += 2;
	}
	void opcodefx07(){

		incrementPC();
		//VX = *delayRegister;
	}
	void opcodefx0a(){

		incrementPC();
		//if (Input::isKeyPressed(*pressedKey) == true) VX = *pressedKey; else flag = 1; //wait again	(dont increment pc)
	}
	void opcodefx15(){

		incrementPC();
		//*delayRegister = VX;
	}
	void opcodefx18(){

		incrementPC();
		//controllerOp = ControllerOp::setSoundTimer;
	}
	void opcodefx1e(){

		incrementPC();
		//indexRegister += VX;
	}
	void opcodefx29(){

		incrementPC();
		//indexRegister = VX * 5;	//font is stored at mem[0 ~ FONT_COUNT * 5]
	}
	void opcodefx33(){

		incrementPC();
		//memory->write(indexRegister, VX / 100);
		//memory->write(indexRegister + 1, (VX / 10) % 10);
		//memory->write(indexRegister + 2, VX % 10);
	}
	void opcodefx55(){

		incrementPC();
		//for (int i = 0; i <= (currentOpcode & 0x0f00) >> 8; i++) memory->write(indexRegister + i, v[i]);
	}
	void opcodefx65(){

		incrementPC();
		//for (int i = 0; i <= (currentOpcode & 0x0f00) >> 8; i++) v[i] = memory->read(indexRegister + i);
	}

	//exception
	void opcodenull(){

		incrementPC();
		//throwError = true;
	}


};