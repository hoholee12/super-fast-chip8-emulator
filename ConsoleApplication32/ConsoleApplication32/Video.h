#pragma once
#include<SDL/SDL.h>
#include"CPU.h"
#include"Memory.h"

typedef unsigned char uint8;
typedef unsigned short uint16;

#define SCREEN_WIDTH 0x40
#define SCREEN_HEIGHT 0x20
#define SCALE 10

class Video{
private:

	uint8 videoBuffer[SCREEN_WIDTH * SCREEN_HEIGHT]; //video buffer
	SDL_Rect pixelRect[SCREEN_WIDTH * SCREEN_HEIGHT];
	//SDL stuff
	SDL_Renderer* renderer;
	SDL_Window* window;
	SDL_Surface* screenSurface;
public:
	void writeVBuffer(uint16 addr, uint8 input);
	uint8 readVBuffer(uint16 addr);

	void clearVBuffer();

	void copySprite(uint16 input, CPU* cpu, Memory* memory);

	void draw(); //updates screen
	
	void init(char* str); //i do this only to display filename on window bar

};