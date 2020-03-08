#pragma once
#include<SDL/SDL.h>

typedef unsigned char uint8;
typedef unsigned short uint16;

/*

pressedKey:

0xfe is no input
0xff is quit

*/

class Input{
public:

	

	uint8 pressedKey;

	//loop this with system
	uint8 checkKeyInput();
	void init();


	//standalone
	uint8 defaultKey = 0xfe;
	bool isKeyPressed(uint8 pressedKey);
};