#include"Input.h"
#include<stdio.h>

void Input::init(){
	pressedKey = 0xfe;

	defaults::inputInit();
}

void Input::checkKeyInput(){


	pressedKey = defaults::getInput();
	//printf("pressed key : %x\n", pressedKey);

}

uint8_t Input::getKey(){ return pressedKey; }

bool Input::isKeyPressed(uint8_t pressedKey){ return (pressedKey != defaultKey) ? true : false; }