#pragma once
#include"CPU.h"
#include"Memory.h"
#include"defaults.h"

#define SCREEN_WIDTH 0x40
#define SCREEN_HEIGHT 0x20
#define SCALE 10

class Video{
private:

	uint8 videoBuffer[SCREEN_WIDTH * SCREEN_HEIGHT]; //video buffer
	
public:
	void writeVBuffer(uint16 addr, uint8 input);
	uint8 readVBuffer(uint16 addr);

	void clearVBuffer();

	void copySprite(uint16 input, CPU* cpu, Memory* memory);

	void draw(defaults* mainwindow); //updates screen
	
	void init(char* str, defaults* mainwindow); //i do this only to display filename on window bar

};