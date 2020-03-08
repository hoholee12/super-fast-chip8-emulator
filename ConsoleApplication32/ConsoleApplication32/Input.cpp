#include"Input.h"
#include<stdio.h>

void Input::init(){
	pressedKey = 0xfe;

	defaults::inputInit();
}

uint8 Input::checkKeyInput(){


	pressedKey = defaults::getInput();
	//printf("pressed key : %x\n", pressedKey);

	return pressedKey;
}


bool Input::isKeyPressed(uint8 pressedKey){ return (pressedKey != defaultKey) ? true : false; }