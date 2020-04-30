#pragma once

/*
	Dynarec chip8 translator

*/
#include "CPU.h"
#include "X86Emitter.h"

class Translator: public X86Emitter{

private:
	//big endians(will be converted automatically on X86Emitter.h)
	//original interpreter registers(will still be used for other parts of the emulator not covered by dynarec)
	uint32_t controllerOp;
	uint16_t programCounter;
	uint16_t stack;
	uint8_t stackPointer;
	uint32_t flag;
	uint8_t jmpHint;
	uint8_t v;
	uint16_t indexRegister;
	uint8_t pressedKey;
	uint8_t delayRegister;
	uint8_t throwError;
	uint8_t mem;

	uint16_t currentOpcode;

	uint32_t temp;

	void init(CPU* cpu){
		controllerOp = (uint32_t)&cpu->controllerOp;
		programCounter = (uint16_t)&cpu->programCounter;
		stack = (uint16_t)&cpu->stack;
		stackPointer = (uint8_t)&cpu->stackPointer;
		flag = (uint32_t)&cpu->flag;
		jmpHint = (uint8_t)&cpu->jmpHint;
		v = (uint8_t)&cpu->v;
		indexRegister = (uint16_t)&cpu->indexRegister;
		pressedKey = (uint8_t)&cpu->pressedKey;
		delayRegister = (uint8_t)&cpu->delayRegister;
		throwError = (uint8_t)&cpu->throwError;
		mem = (uint8_t)&cpu->memory->mem;

		currentOpcode = (uint16_t)&cpu->currentOpcode;

		temp = 0;
		temp = (uint32_t)&temp;
	}

	int* index;
	uint8_t* memoryBlock;

	void init(uint8_t* memoryBlock, int* index){
		this->index = index;
		this->memoryBlock = memoryBlock;
	}


	void incrementPC(){
		X86Emitter::mov_memoryval_to_eax(memoryBlock, programCounter, index);
		//+2
		X86Emitter::add_immediate_to_eax(memoryBlock, 0x00000002, index);
		X86Emitter::mov_eax_to_memoryval(memoryBlock, programCounter, index);
	}

public:

	//opcodes
	void opcode00e0(){
		X86Emitter::mov_immediate_to_eax(memoryBlock, (uint32_t)ControllerOp::clearScreen, index);
		X86Emitter::mov_eax_to_memoryval(memoryBlock, controllerOp, index);

		incrementPC();
		//original
		//controllerOp = ControllerOp::clearScreen;
	}

	void opcode00ee(){
		//--stackPointer
		X86Emitter::mov_memoryval_to_eax(memoryBlock, stackPointer, index);
		X86Emitter::dec_eax(memoryBlock, index);
		X86Emitter::mov_eax_to_memoryval(memoryBlock, stackPointer, index);
		//stack[stackPointer] (DO NOT DO stack + stackPointer!!!! stackPointer is not fixed)
		X86Emitter::mov_immediate_to_ebx(memoryBlock, stack, index);
		X86Emitter::add_eax_to_ebx(memoryBlock, index);
		X86Emitter::mov_ebx_to_memoryval(memoryBlock, temp, index);
		//to programCounter
		X86Emitter::mov_memoryval_to_eax(memoryBlock, temp, index);
		X86Emitter::mov_eax_to_memoryval(memoryBlock, programCounter, index);

		incrementPC();
		//original
		//programCounter = stack[--stackPointer]; //return from SUBroutine	(and increment pc after to get out of loop)
	}
	void opcode0nnn(){
		
		//stack[stackPointer]
		X86Emitter::mov_memoryval_to_eax(memoryBlock, stackPointer, index);
		X86Emitter::mov_immediate_to_ebx(memoryBlock, stack, index);
		X86Emitter::add_eax_to_ebx(memoryBlock, index);
		//addr is on ebx
		//programCounter to stack[stackPointer]
		X86Emitter::mov_memoryval_to_eax(memoryBlock, programCounter, index);
		X86Emitter::mov_eax_to_ebxaddr(memoryBlock, index);
		//0x0FFF for NNN to ebx, opcode to eax
		X86Emitter::mov_immediate_to_ebx(memoryBlock, 0x00000FFF, index);
		X86Emitter::mov_memoryval_to_eax(memoryBlock, currentOpcode, index);
		X86Emitter::and_ebx_to_eax(memoryBlock, index);
		//NNN to programCounter
		X86Emitter::mov_eax_to_memoryval(memoryBlock, programCounter, index);

		//original
		//stack[stackPointer++] = programCounter; programCounter = NNN; flag = 1;//call SUBroutine from nnn	(dont increment pc)
	}
	void opcode1nnn(){
		//0x0FFF for NNN to ebx, opcode to eax
		X86Emitter::mov_immediate_to_ebx(memoryBlock, 0x00000FFF, index);
		X86Emitter::mov_memoryval_to_eax(memoryBlock, currentOpcode, index);
		X86Emitter::and_ebx_to_eax(memoryBlock, index);
		//NNN to programCounter
		X86Emitter::mov_eax_to_memoryval(memoryBlock, programCounter, index);

		//programCounter = NNN; flag = 1;//jump to nnn	(dont increment pc)
		//jmpHint = true; //hint for video flicker loop
	}
	void opcode2nnn(){
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