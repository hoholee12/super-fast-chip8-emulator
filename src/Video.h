#pragma once
#include"CPU.h"
#include"Memory.h"
#include"defaults.h"

#define SCREEN_WIDTH 0x40
#define SCREEN_HEIGHT 0x20
#define SCALE 20

class Video{
private:

	uint8_t videoBuffer[SCREEN_WIDTH * SCREEN_HEIGHT]; //video buffer
	
public:
	void writeVBuffer(uint16_t addr, uint8_t input);
	uint8_t readVBuffer(uint16_t addr);

	void clearVBuffer();

	void copySprite(uint16_t input, CPU* cpu, Memory* memory);

	void draw(defaults* mainwindow); //updates screen
	
	void init(char* str, defaults* mainwindow); //i do this only to display filename on window bar

};
