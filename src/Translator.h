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

	uint32_t vxPointer;			//byte
	uint32_t vyPointer;			//byte
	uint8_t vfPointerx;
	uint32_t vfPointer;			//byte
	uint8_t vzPointerx;
	uint32_t vzPointer;			//byte

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

		vxPointer = (uint32_t)&cpu->x_val;
		vyPointer = (uint32_t)&cpu->y_val;
		vfPointerx = 0xF;
		vfPointer = (uint32_t)&vfPointerx;
		vzPointerx = 0x0;
		vzPointer = (uint32_t)&vzPointerx;
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
		X86Emitter::addWordToMemaddr(memoryBlock, programCounter, 2);
	}

	//interpreterSwitch = true;
	//fallback to interpreter, switch back whenever its done. but not here
	//some opcodes are just too complicated to recreate in jit
	//...or jump
	void hintFallBack(){
		X86Emitter::setByteToMemaddr(memoryBlock, interpreterSwitch, 0x1);
	}
	void switchToInterpreter(){
		X86Emitter::ret(memoryBlock);
		endMemoryBlock = true;	//this is hint for translator to cut blocks
	}

	//opcodes
	void opcode00e0(){
		X86Emitter::setDwordToMemaddr(memoryBlock, controllerOp, (uint32_t)ControllerOp::clearScreen);

		incrementPC();
		switchToInterpreter();
		//original
		//controllerOp = ControllerOp::clearScreen;
	}

	void opcode00ee(){
		//stack = ebx, stackptr = eax, pc = ecx
		//--stackPointer
		X86Emitter::add_imm(memoryBlock, byteAddImmToMemaddrMode, insertAddr(stackPointer), insertDisp(-1));
		//X86Emitter::dec_byte_memaddr(memoryBlock, stackPointer);

		//stack[stackPointer]
		X86Emitter::loadWordArray_AregAsResult(memoryBlock, stack, stackPointer);

		//to programCounter
		X86Emitter::mov(memoryBlock, movWordRegToMemMode, Areg, insertAddr(programCounter));
		//X86Emitter::mov_ax_to_memoryaddr(memoryBlock, programCounter);

		incrementPC();
		switchToInterpreter();
		//programCounter = stack[--stackPointer]; //return from SUBroutine	(and increment pc after to get out of loop)
	}
	void opcode0nnn(){
		
		//= programCounter
		X86Emitter::loadWordToDwordRegA(memoryBlock, programCounter);

		//stack[stackpointer]
		X86Emitter::storeWordArray_AregAsInput(memoryBlock, stack, stackPointer);

		//stackPointer++
		X86Emitter::add_imm(memoryBlock, byteAddImmToMemaddrMode, insertAddr(stackPointer), insertDisp(1));
		//X86Emitter::inc_byte_memaddr(memoryBlock, stackPointer);

		//NNN(is an immediate) to programCounter
		X86Emitter::setWordToMemaddr(memoryBlock, programCounter, nnn);

		switchToInterpreter();
		//stack[stackPointer++] = programCounter; programCounter = NNN; flag = 1;//call SUBroutine from nnn	(dont increment pc)
	}
	void opcode1nnn(){
		//NNN to programCounter
		X86Emitter::setWordToMemaddr(memoryBlock, programCounter, nnn);

		//jmpHint = true
		X86Emitter::setByteToMemaddr(memoryBlock, jmpHint, 1);

		switchToInterpreter();
		//programCounter = NNN; flag = 1;//jump to nnn	(dont increment pc)
		//jmpHint = true; //hint for video flicker loop
	}
	void opcode2nnn(){
		//= programCounter
		X86Emitter::loadWordToDwordRegA(memoryBlock, programCounter);

		//stack[stackpointer]
		X86Emitter::storeWordArray_AregAsInput(memoryBlock, stack, stackPointer);

		//stackPointer++
		X86Emitter::add_imm(memoryBlock, byteAddImmToMemaddrMode, insertAddr(stackPointer), insertDisp(1));
		//X86Emitter::inc_byte_memaddr(memoryBlock, stackPointer);

		//NNN(is an immediate) to programCounter
		X86Emitter::setWordToMemaddr(memoryBlock, programCounter, nnn);

		switchToInterpreter();
		//stack[stackPointer++] = programCounter; programCounter = NNN; flag = 1;//call SUBroutine from nnn	(dont increment pc)
	}

	void opcode3xnn(){
		//vx = eax, nn = ebx
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vxPointer);
		X86Emitter::loadWordToDwordRegB(memoryBlock, nn);

		X86Emitter::cmp(memoryBlock, cmpMode, Areg, Breg);
		//X86Emitter::cmp_eax_to_ebx(memoryBlock);
		X86Emitter::jcc(memoryBlock, byteRelJneMode, insertDisp(addWordToMemaddrSize));
		//X86Emitter::short_jne(memoryBlock, addWordToMemaddrSize);
		X86Emitter::addWordToMemaddr(memoryBlock, programCounter, 2);

		incrementPC();
		//if (VX == NN) programCounter += 2; //skip if ==
	}
	void opcode4xnn(){
		//vx = eax, nn = ebx
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vxPointer);
		X86Emitter::loadWordToDwordRegB(memoryBlock, nn);

		X86Emitter::cmp(memoryBlock, cmpMode, Areg, Breg);
		X86Emitter::jcc(memoryBlock, byteRelJeMode, insertDisp(addWordToMemaddrSize));
		X86Emitter::addWordToMemaddr(memoryBlock, programCounter, 2);

		incrementPC();
		//if (VX != NN) programCounter += 2; //skip if !=
	}
	void opcode5xy0(){
		//vx = eax, vy = edx
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vyPointer);
		X86Emitter::mov(memoryBlock, movDwordRegToRegMode, Areg, Dreg);
		//X86Emitter::mov_eax_to_edx(memoryBlock);
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vxPointer);

		X86Emitter::cmp(memoryBlock, cmpMode, Areg, Breg);
		X86Emitter::jcc(memoryBlock, byteRelJneMode, insertDisp(addWordToMemaddrSize));
		X86Emitter::addWordToMemaddr(memoryBlock, programCounter, 2);

		incrementPC();
		//if (VX == VY) programCounter += 2; //skip if vx == vy
	}
	void opcode6xnn(){

		X86Emitter::loadWordToDwordRegA(memoryBlock, nn);
		X86Emitter::storeByteArray_AregAsInput(memoryBlock, v, vxPointer);

		incrementPC();
		//VX = NN; //into
	}
	void opcode7xnn(){

		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vxPointer);
		X86Emitter::loadWordToDwordRegB(memoryBlock, nn);
		X86Emitter::add(memoryBlock, dwordAddMode, Breg, Areg);
		//X86Emitter::add_ebx_to_eax(memoryBlock);

		X86Emitter::storeByteArray_AregAsInput(memoryBlock, v, vxPointer);

		incrementPC();
		//VX += NN;
	}
	void opcode8xy0(){

		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vyPointer);

		X86Emitter::storeByteArray_AregAsInput(memoryBlock, v, vxPointer);

		incrementPC();
		//VX = VY;
	}
	void opcode8xy1(){

		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vyPointer);
		X86Emitter::mov(memoryBlock, movDwordRegToRegMode, Areg, Breg);
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vxPointer);
		X86Emitter::or(memoryBlock, dwordOrMode, Breg, Areg);
		//X86Emitter::or_ebx_to_eax(memoryBlock);

		X86Emitter::storeByteArray_AregAsInput(memoryBlock, v, vxPointer);

		incrementPC();
		//VX |= VY;
	}
	void opcode8xy2(){

		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vyPointer);
		X86Emitter::mov(memoryBlock, movDwordRegToRegMode, Areg, Breg);
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vxPointer);
		X86Emitter::and(memoryBlock, dwordOrMode, Breg, Areg);

		X86Emitter::storeByteArray_AregAsInput(memoryBlock, v, vxPointer);

		incrementPC();
		//VX &= VY;
	}
	void opcode8xy3(){

		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vyPointer);
		X86Emitter::mov(memoryBlock, movDwordRegToRegMode, Areg, Breg);
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vxPointer);
		X86Emitter::xor(memoryBlock, dwordOrMode, Breg, Areg);

		X86Emitter::storeByteArray_AregAsInput(memoryBlock, v, vxPointer);

		incrementPC();
		//VX ^= VY;
	}
	void opcode8xy4(){
		//ebx = vx + vy
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vyPointer);
		X86Emitter::mov(memoryBlock, movDwordRegToRegMode, Areg, Breg);
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vxPointer);
		X86Emitter::add(memoryBlock, dwordAddMode, Areg, Breg);

		//ecx = 0xff
		X86Emitter::mov_imm(memoryBlock, dwordMovImmToCregMode, insertDisp(0xff));
		//X86Emitter::mov_imm_to_ecx(memoryBlock, 0xff);

		//>?
		X86Emitter::mov_imm(memoryBlock, dwordMovImmToAregMode, insertDisp(0x0));
		X86Emitter::cmp(memoryBlock, cmpMode, Breg, Creg);
		X86Emitter::jcc(memoryBlock, byteRelJbeMode, insertDisp(dwordAddImmToRegSize));

		X86Emitter::add_imm(memoryBlock, dwordAddImmToRegMode, insertDisp(1), Areg);

		//=
		X86Emitter::storeByteArray_AregAsInput(memoryBlock, v, vfPointer);

		//vx += vy
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vyPointer);
		X86Emitter::mov(memoryBlock, movDwordRegToRegMode, Areg, Breg);
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vxPointer);
		X86Emitter::add(memoryBlock, dwordAddMode, Breg, Areg);
		X86Emitter::storeByteArray_AregAsInput(memoryBlock, v, vxPointer);

		incrementPC();
		//VF = (VX + VY > 0xff) ? 0x1 : 0x0; VX += VY;
	}
	void opcode8xy5(){

		//ebx = vy
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vyPointer);
		X86Emitter::mov(memoryBlock, movDwordRegToRegMode, Areg, Breg);

		//ecx = vx
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vxPointer);
		X86Emitter::mov(memoryBlock, movDwordRegToRegMode, Areg, Creg);


		//<?
		X86Emitter::mov_imm(memoryBlock, dwordMovImmToAregMode, insertDisp(1));
		X86Emitter::cmp(memoryBlock, cmpMode, Creg, Breg);
		X86Emitter::jcc(memoryBlock, byteRelJaMode, insertDisp(dwordAddImmToRegSize));

		X86Emitter::add_imm(memoryBlock, dwordAddImmToRegMode, insertDisp(-1), Areg);

		//=
		X86Emitter::storeByteArray_AregAsInput(memoryBlock, v, vfPointer);

		//vx -= vy
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vyPointer);
		X86Emitter::mov(memoryBlock, movDwordRegToRegMode, Areg, Breg);

		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vxPointer);
		X86Emitter::sub(memoryBlock, dwordSubMode, Breg, Areg);

		X86Emitter::storeByteArray_AregAsInput(memoryBlock, v, vxPointer);

		incrementPC();
		//VF = (VX < VY) ? 0x0 : 0x1; VX -= VY;
	}
	void opcode8xy6(){
		//vf = vx << 7
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vxPointer);
		X86Emitter::shift(memoryBlock, dwordShiftLeftMode, insertDisp(7), Areg);
		X86Emitter::movzx(memoryBlock, movzxByteToDwordMode, Areg, Areg);//make sure to cut off
		
		//vf >>= 7
		X86Emitter::shift(memoryBlock, dwordShiftRightMode, insertDisp(7), Areg);
		X86Emitter::storeByteArray_AregAsInput(memoryBlock, v, vfPointer);

		//vx >>= 1
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vxPointer);
		X86Emitter::shift(memoryBlock, dwordShiftRightMode, insertDisp(1), Areg);
		X86Emitter::storeByteArray_AregAsInput(memoryBlock, v, vxPointer);

		incrementPC();
		//VF = VX << 7; VF >>= 7; VX >>= 1;
	}
	void opcode8xy7(){

		//ebx = vy
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vyPointer);
		X86Emitter::mov(memoryBlock, movDwordRegToRegMode, Areg, Breg);

		//ecx = vx
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vxPointer);
		X86Emitter::mov(memoryBlock, movDwordRegToRegMode, Areg, Creg);


		//<?
		X86Emitter::mov_imm(memoryBlock, dwordMovImmToAregMode, insertDisp(1));
		X86Emitter::cmp(memoryBlock, cmpMode, Creg, Breg);
		X86Emitter::jcc(memoryBlock, byteRelJbMode, insertDisp(dwordAddImmToRegSize));

		X86Emitter::add_imm(memoryBlock, dwordAddImmToRegMode, insertDisp(-1), Areg);

		//=
		X86Emitter::storeByteArray_AregAsInput(memoryBlock, v, vfPointer);

		//vx -= vy
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vxPointer);
		X86Emitter::mov(memoryBlock, movDwordRegToRegMode, Areg, Breg);

		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vyPointer);
		X86Emitter::sub(memoryBlock, dwordSubMode, Breg, Areg);

		X86Emitter::storeByteArray_AregAsInput(memoryBlock, v, vxPointer);

		incrementPC();
		//VF = (VY < VX) ? 0x0 : 0x1; VX = VY - VX;
	}
	void opcode8xye(){

		//vf = vx >> 7
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vxPointer);
		X86Emitter::shift(memoryBlock, dwordShiftRightMode, insertDisp(7), Areg);
		X86Emitter::movzx(memoryBlock, movzxByteToDwordMode, Areg, Areg);//make sure to cut off

		//vx <<= 1
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vxPointer);
		X86Emitter::shift(memoryBlock, dwordShiftLeftMode, insertDisp(1), Areg);
		X86Emitter::storeByteArray_AregAsInput(memoryBlock, v, vxPointer);

		incrementPC();
		//VF = VX >> 7; VX <<= 1;
	}
	void opcode9xy0(){
		//vx = eax, vy = edx
		X86Emitter::loadWordArray_AregAsResult(memoryBlock, v, vyPointer);
		X86Emitter::mov(memoryBlock, movDwordRegToRegMode, Areg, Dreg);
		X86Emitter::loadWordArray_AregAsResult(memoryBlock, v, vxPointer);

		X86Emitter::cmp(memoryBlock, cmpMode, Areg, Dreg);
		X86Emitter::jcc(memoryBlock, byteRelJeMode, insertDisp(addWordToMemaddrSize));
		
		X86Emitter::addWordToMemaddr(memoryBlock, programCounter, 2);


		incrementPC();
		//if (VX != VY) programCounter += 2; //skip if vx != vy
	}
	void opcodeannn(){

		X86Emitter::setWordToMemaddr(memoryBlock, indexRegister, nnn);

		incrementPC();
		//indexRegister = NNN;
	}
	void opcodebnnn(){
		X86Emitter::setWordToMemaddr(memoryBlock, programCounter, nnn);
		X86Emitter::addWordToMemaddr(memoryBlock, programCounter, stack);

		//programCounter = NNN + v[0]; flag = 1; //(dont increment pc)
	}
	void opcodecxnn(){
		hintFallBack();
		switchToInterpreter();
		//incrementPC();
		//VX = (rand() % 0x100) & NN;	//random
	}
	void opcodedxyn(){
		X86Emitter::setDwordToMemaddr(memoryBlock, controllerOp, (uint32_t)ControllerOp::drawVideo);

		incrementPC();
		switchToInterpreter();
		//controllerOp = ControllerOp::drawVideo;
	}
	void opcodeex9e(){
		X86Emitter::mov(memoryBlock, movByteMemToRegMode, Breg, insertDisp(pressedKey));
		X86Emitter::loadWordArray_AregAsResult(memoryBlock, stack, stackPointer);
		X86Emitter::cmp(memoryBlock, cmpMode, Areg, Breg);
		X86Emitter::jcc(memoryBlock, byteRelJneMode, insertDisp(addWordToMemaddrSize));
		X86Emitter::addWordToMemaddr(memoryBlock, programCounter, 2);

		incrementPC();
		//if (*pressedKey == VX) programCounter += 2;
	}
	void opcodeexa1(){
		X86Emitter::mov(memoryBlock, movByteMemToRegMode, Breg, insertDisp(pressedKey));
		X86Emitter::loadWordArray_AregAsResult(memoryBlock, stack, stackPointer);
		X86Emitter::cmp(memoryBlock, cmpMode, Areg, Breg);
		X86Emitter::jcc(memoryBlock, byteRelJeMode, insertDisp(addWordToMemaddrSize));
		X86Emitter::addWordToMemaddr(memoryBlock, programCounter, 2);

		incrementPC();
		//if (*pressedKey != VX) programCounter += 2;
	}
	void opcodefx07(){
		X86Emitter::mov(memoryBlock, movByteMemToRegMode, Areg, insertDisp(delayRegister));
		X86Emitter::storeByteArray_AregAsInput(memoryBlock, v, vxPointer);

		incrementPC();
		//VX = *delayRegister;
	}
	void opcodefx0a(){
		hintFallBack();
		switchToInterpreter();
		//incrementPC();
		//if (Input::isKeyPressed(*pressedKey) == true) VX = *pressedKey; else flag = 1; //wait again	(dont increment pc)
	}
	void opcodefx15(){
		X86Emitter::loadByteArray_AregAsResult(memoryBlock, v, vxPointer);
		X86Emitter::mov(memoryBlock, movByteRegToMemMode, Areg, insertDisp(delayRegister));
		
		incrementPC();
		//*delayRegister = VX;
	}
	void opcodefx18(){
		X86Emitter::setDwordToMemaddr(memoryBlock, controllerOp, (uint32_t)ControllerOp::setSoundTimer);

		incrementPC();
		switchToInterpreter();

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