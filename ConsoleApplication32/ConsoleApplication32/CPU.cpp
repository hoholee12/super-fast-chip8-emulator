#include<stdlib.h>
#include<stdio.h>
#include"CPU.h"




uint16 CPU::decode(Memory* memory, uint16 input, uint8 pressedKey){
	uint16 controllerOp = 0x0;

	//TODO
	bool throwError = false; //if illegal instruction

	//program counter increments by 2. 16bit addr
	//nn -byte(2 nibbles)
	//nnn -addr(3 nibbles)
	//x, y -Vx, Vy registers
	//index register

	//?x??
	uint8 *vx = &v[(input & 0x0f00) >> 8];

	//??y?
	uint8 *vy = &v[(input & 0x00f0) >> 4];

	//vf
	uint8 *vf = &v[0xf];

	//??nn
	uint8 nn = input & 0x00ff;

	//nnn
	uint16 nnn = input & 0x0fff;

	uint16 n = input & 0x000f;

	//1 = is a jump; dont increment pc
	int flag = 0;

	//first nibble
	switch ((input & 0xf000) >> 12){
	case 0x0:
		switch (input & 0x00ff){
		case 0xe0:	controllerOp = 0x1;
			break;
		case 0xee:	programCounter = stack[--stackPointer]; flag = 1;//return from subroutine
			break;
		default:	stack[stackPointer++] = programCounter; programCounter = nnn; flag = 1;//call subroutine from nnn
			break;
		}
		break;
	case 0x1:	programCounter = nnn; flag = 1;//jump to nnn
		break;
	case 0x2:	stack[stackPointer++] = programCounter; programCounter = nnn; flag = 1;//call subroutine from nnn
		break;
	case 0x3:	if (*vx == nn) programCounter += 2; //skip if ==
		break;
	case 0x4:	if (*vx != nn) programCounter += 2; //skip if !=
		break;
	case 0x5:	if (*vx == *vy) programCounter += 2; //skip if vx == vy
		break;
	case 0x6:	*vx = nn; //into
		break;
	case 0x7:	*vx += nn;
		break;
	case 0x8:	
		switch (input & 0x000f){
		case 0x0:	*vx = *vy;
			break;
		case 0x1:	*vx |= *vy;
			break;
		case 0x2:	*vx &= *vy;
			break;
		case 0x3:	*vx ^= *vy;
			break;
		case 0x4:	*vf = (*vx + *vy > 0xff) ? 0x1 : 0x0; *vx += *vy;
			break;
		case 0x5:	*vf = (*vx < *vy) ? 0x0 : 0x1; *vx -= *vy;
			break;
		case 0x6:	*vf = *vx << 7; *vf >>= 7; *vx >>= 1;
			break;
		case 0x7:	*vf = (*vy < *vx) ? 0x0 : 0x1; *vx = *vy - *vx;
			break;
		case 0xe:	*vf = *vx >> 7; *vx <<= 1;
			break;
		default: throwError = true;
			break;
		}
		break;
	case 0x9:	if (*vx != *vy) programCounter += 2; //skip if vx != vy
		break;
	case 0xa:	indexRegister = nnn;
		break;
	case 0xb:	programCounter = nnn + v[0]; flag = 1;
		break;
	case 0xc:	*vx = (rand() % 0xff) & nn;	//random
		break;
	case 0xd:	controllerOp = 0x2;
		break;
	case 0xe:	
		switch (input & 0x00ff){
		case 0x9e:
			if (pressedKey == *vx) programCounter += 2;
			break;
		case 0xa1:
			if (pressedKey != *vx) programCounter += 2;
			break;
		default: throwError = true;
			break;
		}
		break;
	case 0xf:
		switch (input & 0x00ff){
		case 0x07:	*vx = delayTimer;
			break;
		case 0x0a:	if (Input::isKeyPressed(pressedKey) == true) *vx = pressedKey; else flag = 1; //wait again
			break;
		case 0x15:	delayTimer = *vx;
			break;
		case 0x18:	controllerOp = 0x3;
			break;
		case 0x1e:	indexRegister += *vx;
			break;
		case 0x29:	indexRegister = *vx * 5;	//font is stored at mem[0 ~ FONT_COUNT * 5]
			break;
		case 0x33:	//bcd code
			memory->write(indexRegister, *vx / 100);
			memory->write(indexRegister + 1, (*vx / 10) % 10);
			memory->write(indexRegister + 2, *vx % 10);
			break;
		case 0x55:	for (int i = 0; i <= (input & 0x0f00) >> 8; i++) memory->write(indexRegister + i, v[i]);
			break;
		case 0x65:	for (int i = 0; i <= (input & 0x0f00) >> 8; i++) v[i] = memory->read(indexRegister + i);
			break;
		default: throwError = true;
			break;
		}
		break;
	default: throwError = true;
		break;
	}

	if (throwError == true)
		fprintf(stderr, "unsupported opcode!: %x\n", input);

	if (flag != 1)	//only if its not jump
	programCounter += 2; //increment after fetch


	//delay timer
	if (delayTimer > 0x0){
		delayTimer--;
	}

	return controllerOp;

}

uint16 CPU::fetch(Memory* memory){
	uint16 currentOpcode = 0;

	//fetch - mem is 8bit, opcode is 16bit, big endian
	//mem[pc] as top 8bit + mem[pc+1] as bottom 8bit = 16bit
	currentOpcode = memory->read(programCounter) << 8;
	currentOpcode |= memory->read(programCounter + 1);
	return currentOpcode;
}

uint16* CPU::getProgramCounter(){ return &programCounter; }
uint8* CPU::getStackPointer(){ return &stackPointer; }
uint16* CPU::getIndexRegister(){ return &indexRegister; }
uint16* CPU::getStack(uint8 input){ return &stack[input]; }
uint8* CPU::getV(uint8 input){ return &v[input]; }


void CPU::init(){
	srand(time(NULL));

	programCounter = 0x200; //start at 0x200
	stackPointer = 0;
	indexRegister = 0;

	delayTimer = 0;

	//init v register
	for (int i = 0; i < V_REGISTER_SIZE; i++) v[i] = 0x0;
}
