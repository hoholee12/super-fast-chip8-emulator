#pragma once

#include<stdlib.h>
#include<time.h>	//for random seed

#include"Memory.h"
#include"Input.h"
#include"defaults.h"
#include"Timer.h"

#define STACK_SIZE 16
#define V_REGISTER_SIZE 0x10

class CPU;	//forward declaration for cputable
typedef void (CPU::*       CPUTable)();


class CPU: public Input{
private:
	uint16 currentOpcode;
	uint16 programCounter;
	uint8 stackPointer;
	uint16 indexRegister;	//I register
	uint16 stack[STACK_SIZE];
	uint8 v[V_REGISTER_SIZE];


#define OPCODE_TABLE_SIZE 0x10
#define OPCODE_TABLE_0_SIZE 0xff
#define OPCODE_TABLE_8_SIZE 0xf
#define OPCODE_TABLE_E_SIZE 0xff
#define OPCODE_TABLE_F_SIZE 0xff
	//opcode table
	CPUTable opcode_table[OPCODE_TABLE_SIZE];
	CPUTable opcode_table_0[OPCODE_TABLE_0_SIZE];
	CPUTable opcode_table_8[OPCODE_TABLE_8_SIZE];
	CPUTable opcode_table_e[OPCODE_TABLE_E_SIZE];
	CPUTable opcode_table_f[OPCODE_TABLE_F_SIZE];
	
	uint16 controllerOp = 0x0;
	//TODO
	bool throwError;
	uint8 head;
	uint8 sub_dual;
	uint8 sub;
	uint8 *vx;
	uint8 *vy;
	uint8 *vf;
	uint8 nn;
	uint16 nnn;
	uint8 n;

	//1 = is a jump; dont increment pc
	int flag;

	Memory* memory;
	uint8* delayRegister;
	uint8* pressedKey;

public:
	//getters
	uint16* getProgramCounter();
	uint8* getStackPointer();
	uint16* getIndexRegister();	//I register
	uint16* getStack(uint8 input);
	uint8* getV(uint8 input);


	//interpreter needs memory to access, a 60hz delay register(not implemented in cpu), a fetched opcode, and input key
	uint16 decode(); //current opcode decoder
	uint16 fetch();
	
	//opcodes
	void opcodetoTable0();
	void opcodetoTable8();
	void opcodetoTablee();
	void opcodetoTablef();
	void opcode00e0();
	void opcode00ee();
	void opcode0nnn();
	void opcode1nnn();
	void opcode2nnn();
	void opcode3xnn();
	void opcode4xnn();
	void opcode5xy0();
	void opcode6xnn();
	void opcode7xnn();
	void opcode8xy0();
	void opcode8xy1();
	void opcode8xy2();
	void opcode8xy3();
	void opcode8xy4();
	void opcode8xy5();
	void opcode8xy6();
	void opcode8xy7();
	void opcode8xye();
	void opcode9xy0();
	void opcodeannn();
	void opcodebnnn();
	void opcodecxnn();
	void opcodedxyn();
	void opcodeex9e();
	void opcodeexa1();
	void opcodefx01();
	void opcodefx07();
	void opcodefx0a();
	void opcodefx15();
	void opcodefx18();
	void opcodefx1e();
	void opcodefx29();
	void opcodefx33();
	void opcodefx55();
	void opcodefx65();
	void opcodenull();

	void init(Memory* memory, uint8* delayRegister, uint8* pressedKey);
	
};

