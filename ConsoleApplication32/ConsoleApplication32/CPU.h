#pragma once
#include<SDL/SDL.h>
#include<time.h>
#include<stdlib.h>

typedef unsigned char uint8;
typedef unsigned short uint16;

#define FULL_MEM_SIZE 0x1000
#define MID_MEM_SIZE 0x600
#define START_MEM_SIZE 0x200
#define STACK_SIZE 16
#define V_REGISTER_SIZE 0x10
#define SCREEN_WIDTH 0x40
#define SCREEN_HEIGHT 0x20
#define FONT_COUNT 0x10
#define SCALE 10

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
	uint8 videoBuffer[SCREEN_WIDTH * SCREEN_HEIGHT]; //video buffer
	SDL_Rect pixelRect[SCREEN_WIDTH * SCREEN_HEIGHT];

	char* filestr;

	//SDL stuff
	SDL_Renderer* renderer;
	SDL_Window* window;
	SDL_Surface* screenSurface;
	bool running;

	void decode(uint16 input); //current opcode decoder
	int checkKeyInput(uint8* vx, int flag = 0);

public:

	void load(); //load file
	void init(); //init sdl and registers
	void run();	//loop
	void update(); //fetch and decode
	void draw();
	
	void start(char* str); //start of emulation
	
};