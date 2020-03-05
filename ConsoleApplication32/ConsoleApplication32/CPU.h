#pragma once
#include<SDL/SDL.h>

typedef unsigned char uint8;
typedef unsigned short uint16;

#define FULL_MEM_SIZE 0xfff
#define MID_MEM_SIZE 0x600
#define START_MEM_SIZE 0x200
#define STACK_SIZE 16
#define V_REGISTER_SIZE 0xf

class CPU{
private:

	uint8 mem[FULL_MEM_SIZE];
	uint16 programCounter;
	uint8 stackPointer;
	uint16 indexRegister;	//I register
	uint16 stack[STACK_SIZE];
	uint8 v[V_REGISTER_SIZE];
	
	uint16 currentOpcode;

	uint8 key[16]; //16 keys
	uint8 delayTimer; //delay timer
	uint8 soundTimer; //sound timer
	uint8 videoBuffer[32 * 64]; //video buffer

	//SDL stuff
	SDL_Renderer* renderer;
	SDL_Window* window;
	SDL_Surface* screenSurface;
	bool running;

	void decode(uint16 input); //current opcode decoder
	void clearScreen(); //clearscreen

public:

	void load(); //load file
	void init(); //init sdl and registers
	void run();	//loop
	void update(); //fetch and decode and draw
	
	void start(); //start of emulation
	
};