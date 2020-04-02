#include<stdlib.h>
#include<stdio.h>
#include"CPU.h"

uint16_t CPU::decode_jumboLUT(){
	throwError = false;
	controllerOp = 0x0; //this motherfucker

	//opcode table
	(this->*(jumbo_table[currentOpcode]))();

	if (throwError == true)
		fprintf(stderr, "unsupported opcode!: %x\n", currentOpcode);

	if (flag != 1)	//only if its not jump
		programCounter += 2; //increment after fetch	

	return controllerOp;

}

uint16_t CPU::decode(){
	throwError = false;
	controllerOp = 0x0; //this motherfucker

	//opcode table
	(this->*(opcode_table[head]))();
	
	if (throwError == true)
		fprintf(stderr, "unsupported opcode!: %x\n", currentOpcode);

	if (flag != 1)	//only if its not jump
	programCounter += 2; //increment after fetch	

	return controllerOp;

}

uint16_t CPU::fetch(){

	//fetch - mem is 8bit, opcode is 16bit, big endian
	//mem[pc] as top 8bit + mem[pc+1] as bottom 8bit = 16bit
	currentOpcode = memory->read(programCounter) << 8;
	currentOpcode |= memory->read(programCounter + 1);


	//program counter increments by 2. 16bit addr
	//nn -byte(2 nibbles)
	//nnn -addr(3 nibbles)
	//x, y -Vx, Vy registers
	//index register

	//opcode parser
	head = currentOpcode >> 12;
	sub = currentOpcode & 0x000f;
	sub_dual = currentOpcode & 0x00ff;
	

	//?x??
	vx = &v[(currentOpcode & 0x0f00) >> 8];

	//??y?
	vy = &v[(currentOpcode & 0x00f0) >> 4];

	//vf
	vf = &v[0xf];

	//??nn
	nn = currentOpcode & 0x00ff;

	//nnn
	nnn = currentOpcode & 0x0fff;

	n = currentOpcode & 0x000f;

	//1 = is a jump; dont increment pc
	flag = 0;

	//just in case some other parts of the machine needs opcode
	return currentOpcode;
}

//for switch method
uint16_t CPU::decode(Memory* memory, uint8_t* delayRegister, uint16_t input, uint8_t pressedKey){
	uint16_t controllerOp = 0x0;

	//TODO
	bool throwError = false; //if illegal instruction

	//program counter increments by 2. 16bit addr
	//nn -byte(2 nibbles)
	//nnn -addr(3 nibbles)
	//x, y -Vx, Vy registers
	//index register

	//?x??
	uint8_t *vx = &v[(input & 0x0f00) >> 8];

	//??y?
	uint8_t *vy = &v[(input & 0x00f0) >> 4];

	//vf
	uint8_t *vf = &v[0xf];

	//??nn
	uint8_t nn = input & 0x00ff;

	//nnn
	uint16_t nnn = input & 0x0fff;

	uint16_t n = input & 0x000f;

	//1 = is a jump; dont increment pc
	int flag = 0;

	//first nibble
	switch (input >> 12){
	case 0x0:
		switch (input & 0x00ff){
		case 0xe0:	controllerOp = 0x1;
			break;
		case 0xee:	programCounter = stack[--stackPointer]; //return from subroutine	(and increment pc after to get out of loop)
			break;
		default:	stack[stackPointer++] = programCounter; programCounter = nnn; flag = 1;//call subroutine from nnn	(dont increment pc)
			break;
		}
		break;
	case 0x1:	programCounter = nnn; flag = 1;//jump to nnn	(dont increment pc)
		break;
	case 0x2:	stack[stackPointer++] = programCounter; programCounter = nnn; flag = 1;//call subroutine from nnn	(dont increment pc)
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
	case 0xb:	programCounter = nnn + v[0]; flag = 1; //(dont increment pc)
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
		case 0x07:	*vx = *delayRegister;
			break;
		case 0x0a:	if (Input::isKeyPressed(pressedKey) == true) *vx = pressedKey; else flag = 1; //wait again	(dont increment pc)
			break;
		case 0x15:	*delayRegister = *vx;
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




	return controllerOp;

}

uint16_t CPU::fetch(Memory* memory){
	uint16_t currentOpcode = 0;

	//fetch - mem is 8bit, opcode is 16bit, big endian
	//mem[pc] as top 8bit + mem[pc+1] as bottom 8bit = 16bit
	currentOpcode = memory->read(programCounter) << 8;
	currentOpcode |= memory->read(programCounter + 1);


	//this is fetch -> programCounter++ -> execute
	//programCounter += 2;


	return currentOpcode;
}

uint16_t* CPU::getProgramCounter(){ return &programCounter; }
uint8_t* CPU::getStackPointer(){ return &stackPointer; }
uint16_t* CPU::getIndexRegister(){ return &indexRegister; }
uint16_t* CPU::getStack(uint8_t input){ return &stack[input]; }
uint8_t* CPU::getV(uint8_t input){ return &v[input]; }

//get everything here instead
void CPU::init(Memory* memory, uint8_t* delayRegister, uint8_t* pressedKey){
	this->memory = memory;
	this->delayRegister = delayRegister;
	this->pressedKey = pressedKey;
	

	//create opcode table
	//main table
	opcode_table[0x0] = &CPU::opcodetoTable0;	//CPU:: because array was declared before these members were identified
	opcode_table[0x1] = &CPU::opcode1nnn;
	opcode_table[0x2] = &CPU::opcode2nnn;
	opcode_table[0x3] = &CPU::opcode3xnn;
	opcode_table[0x4] = &CPU::opcode4xnn;
	opcode_table[0x5] = &CPU::opcode5xy0;
	opcode_table[0x6] = &CPU::opcode6xnn;
	opcode_table[0x7] = &CPU::opcode7xnn;
	opcode_table[0x8] = &CPU::opcodetoTable8;
	opcode_table[0x9] = &CPU::opcode9xy0;
	opcode_table[0xa] = &CPU::opcodeannn;
	opcode_table[0xb] = &CPU::opcodebnnn;
	opcode_table[0xc] = &CPU::opcodecxnn;
	opcode_table[0xd] = &CPU::opcodedxyn;
	opcode_table[0xe] = &CPU::opcodetoTablee;
	opcode_table[0xf] = &CPU::opcodetoTablef;

	//table 0
	for (uint16_t i = 0; i < OPCODE_TABLE_0_SIZE; i++) opcode_table_0[i] = &CPU::opcodenull;		//exception
	opcode_table_0[0xe0] = &CPU::opcode00e0;
	opcode_table_0[0xee] = &CPU::opcode00ee;

	//table 8
	for (uint16_t i = 0x0; i < OPCODE_TABLE_8_SIZE; i++) opcode_table_8[i] = &CPU::opcodenull;		//exception
	opcode_table_8[0x0] = &CPU::opcode8xy0;
	opcode_table_8[0x1] = &CPU::opcode8xy1;
	opcode_table_8[0x2] = &CPU::opcode8xy2;
	opcode_table_8[0x3] = &CPU::opcode8xy3;
	opcode_table_8[0x4] = &CPU::opcode8xy4;
	opcode_table_8[0x5] = &CPU::opcode8xy5;
	opcode_table_8[0x6] = &CPU::opcode8xy6;
	opcode_table_8[0x7] = &CPU::opcode8xy7;
	opcode_table_8[0xe] = &CPU::opcode8xye;

	//table e
	for (uint16_t i = 0x0; i < OPCODE_TABLE_E_SIZE; i++) opcode_table_e[i] = &CPU::opcodenull;		//exception
	opcode_table_e[0x9e] = &CPU::opcodeex9e;
	opcode_table_e[0xa1] = &CPU::opcodeexa1;
	
	//table f
	for (uint16_t i = 0x0; i < OPCODE_TABLE_F_SIZE; i++) opcode_table_f[i] = &CPU::opcodenull;		//exception
	opcode_table_f[0x07] = &CPU::opcodefx07;
	opcode_table_f[0x0a] = &CPU::opcodefx0a;
	opcode_table_f[0x15] = &CPU::opcodefx15;
	opcode_table_f[0x18] = &CPU::opcodefx18;
	opcode_table_f[0x1e] = &CPU::opcodefx1e;
	opcode_table_f[0x29] = &CPU::opcodefx29;
	opcode_table_f[0x33] = &CPU::opcodefx33;
	opcode_table_f[0x55] = &CPU::opcodefx55;
	opcode_table_f[0x65] = &CPU::opcodefx65;

	//...or a jumbo table (32bit count)
	for (uint32_t i = 0x0; i < JUMBO_TABLE_SIZE; i++) jumbo_table[i] = &CPU::opcodenull;		//exception
	jumbo_table[0x00e0] = &CPU::opcode00e0;
	jumbo_table[0x00ee] = &CPU::opcode00ee;
	for (uint32_t i = 0x1000; i < 0x2000; i++) jumbo_table[i] = &CPU::opcode1nnn;
	for (uint32_t i = 0x2000; i < 0x3000; i++) jumbo_table[i] = &CPU::opcode2nnn;
	for (uint32_t i = 0x3000; i < 0x4000; i++) jumbo_table[i] = &CPU::opcode3xnn;
	for (uint32_t i = 0x4000; i < 0x5000; i++) jumbo_table[i] = &CPU::opcode4xnn;
	for (uint32_t i = 0x5000; i < 0x6000; i++) if ((i & 0x000f) == 0x0) jumbo_table[i] = &CPU::opcode5xy0;
	for (uint32_t i = 0x6000; i < 0x7000; i++) jumbo_table[i] = &CPU::opcode6xnn;
	for (uint32_t i = 0x7000; i < 0x8000; i++) jumbo_table[i] = &CPU::opcode7xnn;
	for (uint32_t i = 0x8000; i < 0x9000; i++){
		if ((i & 0x000f) == 0x0) jumbo_table[i] = &CPU::opcode8xy0;
		if ((i & 0x000f) == 0x1) jumbo_table[i] = &CPU::opcode8xy1;
		if ((i & 0x000f) == 0x2) jumbo_table[i] = &CPU::opcode8xy2;
		if ((i & 0x000f) == 0x3) jumbo_table[i] = &CPU::opcode8xy3;
		if ((i & 0x000f) == 0x4) jumbo_table[i] = &CPU::opcode8xy4;
		if ((i & 0x000f) == 0x5) jumbo_table[i] = &CPU::opcode8xy5;
		if ((i & 0x000f) == 0x6) jumbo_table[i] = &CPU::opcode8xy6;
		if ((i & 0x000f) == 0x7) jumbo_table[i] = &CPU::opcode8xy7;
		if ((i & 0x000f) == 0xe) jumbo_table[i] = &CPU::opcode8xye;
	}
	for (uint32_t i = 0x9000; i < 0xa000; i++) if (i % 0x10 == 0x0) jumbo_table[i] = &CPU::opcode9xy0;
	for (uint32_t i = 0xa000; i < 0xb000; i++) jumbo_table[i] = &CPU::opcodeannn;
	for (uint32_t i = 0xb000; i < 0xc000; i++) jumbo_table[i] = &CPU::opcodebnnn;
	for (uint32_t i = 0xc000; i < 0xd000; i++) jumbo_table[i] = &CPU::opcodecxnn;
	for (uint32_t i = 0xd000; i < 0xe000; i++) jumbo_table[i] = &CPU::opcodedxyn;
	for (uint32_t i = 0xe000; i < 0xf000; i++){
		if ((i & 0x00ff) == 0x9e) jumbo_table[i] = &CPU::opcodeex9e;
		if ((i & 0x00ff) == 0xa1) jumbo_table[i] = &CPU::opcodeexa1;
	}
	for (uint32_t i = 0xf000; i < JUMBO_TABLE_SIZE; i++){
		if ((i & 0x00ff) == 0x07) jumbo_table[i] = &CPU::opcodefx07;
		if ((i & 0x00ff) == 0x0a) jumbo_table[i] = &CPU::opcodefx0a;
		if ((i & 0x00ff) == 0x15) jumbo_table[i] = &CPU::opcodefx15;
		if ((i & 0x00ff) == 0x18) jumbo_table[i] = &CPU::opcodefx18;
		if ((i & 0x00ff) == 0x1e) jumbo_table[i] = &CPU::opcodefx1e;
		if ((i & 0x00ff) == 0x29) jumbo_table[i] = &CPU::opcodefx29;
		if ((i & 0x00ff) == 0x33) jumbo_table[i] = &CPU::opcodefx33;
		if ((i & 0x00ff) == 0x55) jumbo_table[i] = &CPU::opcodefx55;
		if ((i & 0x00ff) == 0x65) jumbo_table[i] = &CPU::opcodefx65;
	}

	srand(time(NULL));	//random seed

	programCounter = 0x200; //start at 0x200
	stackPointer = 0x0;
	indexRegister = 0x0;
	controllerOp = 0x0;
	//TODO
	throwError = false; //if illegal instruction

	//init v register
	for (int i = 0; i < V_REGISTER_SIZE; i++) v[i] = 0x0;
}

//opcodes to specific tables for more opcodes
//do this by dereferencing a member function
void CPU::opcodetoTable0(){
	(this->*(opcode_table_0[sub_dual]))();
}
void CPU::opcodetoTable8(){
	(this->*(opcode_table_8[sub]))();
}
void CPU::opcodetoTablee(){
	(this->*(opcode_table_e[sub_dual]))();
}
void CPU::opcodetoTablef(){
	(this->*(opcode_table_f[sub_dual]))();
}

//opcodes
void CPU::opcode00e0(){ 
	controllerOp = 0x1;
}
void CPU::opcode00ee(){
	programCounter = stack[--stackPointer]; //return from subroutine	(and increment pc after to get out of loop)
}
void CPU::opcode0nnn(){
	stack[stackPointer++] = programCounter; programCounter = nnn; flag = 1;//call subroutine from nnn	(dont increment pc)
}
void CPU::opcode1nnn(){
	programCounter = nnn; flag = 1;//jump to nnn	(dont increment pc)
}
void CPU::opcode2nnn(){
	stack[stackPointer++] = programCounter; programCounter = nnn; flag = 1;//call subroutine from nnn	(dont increment pc)
}
void CPU::opcode3xnn(){
	if (*vx == nn) programCounter += 2; //skip if ==
}
void CPU::opcode4xnn(){
	if (*vx != nn) programCounter += 2; //skip if !=
}
void CPU::opcode5xy0(){
	if (*vx == *vy) programCounter += 2; //skip if vx == vy
}
void CPU::opcode6xnn(){
	*vx = nn; //into
}
void CPU::opcode7xnn(){
	*vx += nn;
}
void CPU::opcode8xy0(){
	*vx = *vy;
}
void CPU::opcode8xy1(){
	*vx |= *vy;
}
void CPU::opcode8xy2(){
	*vx &= *vy;
}
void CPU::opcode8xy3(){
	*vx ^= *vy;
}
void CPU::opcode8xy4(){
	*vf = (*vx + *vy > 0xff) ? 0x1 : 0x0; *vx += *vy;
}
void CPU::opcode8xy5(){
	*vf = (*vx < *vy) ? 0x0 : 0x1; *vx -= *vy;
}
void CPU::opcode8xy6(){
	*vf = *vx << 7; *vf >>= 7; *vx >>= 1;
}
void CPU::opcode8xy7(){
	*vf = (*vy < *vx) ? 0x0 : 0x1; *vx = *vy - *vx;
}
void CPU::opcode8xye(){
	*vf = *vx >> 7; *vx <<= 1;
}
void CPU::opcode9xy0(){
	if (*vx != *vy) programCounter += 2; //skip if vx != vy
}
void CPU::opcodeannn(){
	indexRegister = nnn;
}
void CPU::opcodebnnn(){
	programCounter = nnn + v[0]; flag = 1; //(dont increment pc)
}
void CPU::opcodecxnn(){
	*vx = (rand() % 0xff) & nn;	//random
}
void CPU::opcodedxyn(){
	controllerOp = 0x2;
}
void CPU::opcodeex9e(){
	if (*pressedKey == *vx) programCounter += 2;
}
void CPU::opcodeexa1(){
	if (*pressedKey != *vx) programCounter += 2;
}
void CPU::opcodefx07(){
	*vx = *delayRegister;
}
void CPU::opcodefx0a(){
	if (Input::isKeyPressed(*pressedKey) == true) *vx = *pressedKey; else flag = 1; //wait again	(dont increment pc)
}
void CPU::opcodefx15(){
	*delayRegister = *vx;
}
void CPU::opcodefx18(){
	controllerOp = 0x3;
}
void CPU::opcodefx1e(){
	indexRegister += *vx;
}
void CPU::opcodefx29(){
	indexRegister = *vx * 5;	//font is stored at mem[0 ~ FONT_COUNT * 5]
}
void CPU::opcodefx33(){
	memory->write(indexRegister, *vx / 100);
	memory->write(indexRegister + 1, (*vx / 10) % 10);
	memory->write(indexRegister + 2, *vx % 10);
}
void CPU::opcodefx55(){
	for (int i = 0; i <= (currentOpcode & 0x0f00) >> 8; i++) memory->write(indexRegister + i, v[i]);
}
void CPU::opcodefx65(){
	for (int i = 0; i <= (currentOpcode & 0x0f00) >> 8; i++) v[i] = memory->read(indexRegister + i);
}

//exception
void CPU::opcodenull(){
	throwError = true;
}