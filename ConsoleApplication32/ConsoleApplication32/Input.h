#pragma once
#include"defaults.h"

/*

pressedKey:

0xfe is no input
0xff is quit

*/

class Input: public defaults{
private:
	uint8 pressedKey;
public:

	
	uint8 getKey();
	

	//loop this with system
	void checkKeyInput();
	void init();



	//standalone
	uint8 defaultKey = 0xfe;
	bool isKeyPressed(uint8 pressedKey);

};