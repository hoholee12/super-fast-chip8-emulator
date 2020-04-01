#pragma once
#include"defaults.h"

/*

pressedKey:

0xfe is no input
0xff is quit

*/

class Input: public defaults{
private:
	uint8_t pressedKey;
public:

	
	uint8_t getKey();
	

	//loop this with system
	void checkKeyInput();
	void init();



	//standalone
	uint8_t defaultKey = 0xfe;
	bool isKeyPressed(uint8_t pressedKey);

};