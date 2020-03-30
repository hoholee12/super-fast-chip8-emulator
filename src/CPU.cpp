#include<stdlib.h>
#include<stdio.h>
#include"CPU.h"



uint16 CPU::decode(){

	//opcode table
	(this->*(opcode_table[head]))();
	
	if (throwError == true)
		fprintf(stderr, "unsupported opcode!: %x\n", currentOpcode);

	if (flag != 1)	//only if its not jump
	programCounter += 2; //increment after fetch


	

	return controllerOp;

}

uint16 CPU::fetch(){

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

uint16* CPU::getProgramCounter(){ return &programCounter; }
uint8* CPU::getStackPointer(){ return &stackPointer; }
uint16* CPU::getIndexRegister(){ return &indexRegister; }
uint16* CPU::getStack(uint8 input){ return &stack[input]; }
uint8* CPU::getV(uint8 input){ return &v[input]; }

//get everything here instead
void CPU::init(Memory* memory, uint8* delayRegister, uint8* pressedKey){
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
	for (uint16 i = 0; i < 0xe0; i++) opcode_table_0[i] = &CPU::opcodenull;		//exception
	opcode_table_0[0xe0] = &CPU::opcode00e0;
	opcode_table_0[0xee] = &CPU::opcode00ee;
	for (uint16 i = 0xef; i < OPCODE_TABLE_0_SIZE; i++) opcode_table_0[i] = &CPU::opcodenull;		//exception

	//table 8
	opcode_table_8[0x0] = &CPU::opcode8xy0;
	opcode_table_8[0x1] = &CPU::opcode8xy1;
	opcode_table_8[0x2] = &CPU::opcode8xy2;
	opcode_table_8[0x3] = &CPU::opcode8xy3;
	opcode_table_8[0x4] = &CPU::opcode8xy4;
	opcode_table_8[0x5] = &CPU::opcode8xy5;
	opcode_table_8[0x6] = &CPU::opcode8xy6;
	opcode_table_8[0x7] = &CPU::opcode8xy7;
	for (uint16 i = 0x8; i < 0xe; i++) opcode_table_8[i] = &CPU::opcodenull;	//exception
	opcode_table_8[0xe] = &CPU::opcode8xye;
	for (uint16 i = 0xf; i < OPCODE_TABLE_8_SIZE; i++) opcode_table_8[i] = &CPU::opcodenull;		//exception
	
	//table e
	for (uint16 i = 0x0; i < 0x9e; i++) opcode_table_e[i] = &CPU::opcodenull;	//exception
	opcode_table_e[0x9e] = &CPU::opcodeex9e;
	for (uint16 i = 0x9f; i < 0xa1; i++) opcode_table_e[i] = &CPU::opcodenull;	//exception
	opcode_table_e[0xa1] = &CPU::opcodeexa1;
	for (uint16 i = 0xa2; i < OPCODE_TABLE_E_SIZE; i++) opcode_table_e[i] = &CPU::opcodenull;		//exception

	//table f
	for (uint16 i = 0x0; i < 0x01; i++) opcode_table_f[i] = &CPU::opcodenull;	//exception
	opcode_table_f[0x01] = &CPU::opcodefx01;
	for (uint16 i = 0x02; i < 0x07; i++) opcode_table_f[i] = &CPU::opcodenull;	//exception
	opcode_table_f[0x07] = &CPU::opcodefx07;
	for (uint16 i = 0x08; i < 0x0a; i++) opcode_table_f[i] = &CPU::opcodenull;	//exception
	opcode_table_f[0x0a] = &CPU::opcodefx0a;
	for (uint16 i = 0x0b; i < 0x15; i++) opcode_table_f[i] = &CPU::opcodenull;	//exception
	opcode_table_f[0x15] = &CPU::opcodefx15;
	for (uint16 i = 0x16; i < 0x18; i++) opcode_table_f[i] = &CPU::opcodenull;	//exception
	opcode_table_f[0x18] = &CPU::opcodefx18;
	for (uint16 i = 0x19; i < 0x1e; i++) opcode_table_f[i] = &CPU::opcodenull;	//exception
	opcode_table_f[0x1e] = &CPU::opcodefx1e;
	for (uint16 i = 0x1f; i < 0x29; i++) opcode_table_f[i] = &CPU::opcodenull;	//exception
	opcode_table_f[0x29] = &CPU::opcodefx29;
	for (uint16 i = 0x2a; i < 0x33; i++) opcode_table_f[i] = &CPU::opcodenull;	//exception
	opcode_table_f[0x33] = &CPU::opcodefx33;
	for (uint16 i = 0x34; i < 0x55; i++) opcode_table_f[i] = &CPU::opcodenull;	//exception
	opcode_table_f[0x55] = &CPU::opcodefx55;
	for (uint16 i = 0x56; i < 0x65; i++) opcode_table_f[i] = &CPU::opcodenull;	//exception
	opcode_table_f[0x65] = &CPU::opcodefx65;
	for (uint16 i = 0x66; i < OPCODE_TABLE_F_SIZE; i++) opcode_table_f[i] = &CPU::opcodenull;		//exception

	
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
void CPU::opcodefx01(){
	*vx = *delayRegister;
}
void CPU::opcodefx07(){
	controllerOp = 0x3;
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