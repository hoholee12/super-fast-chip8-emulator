#include"Audio.h"

void Audio::init(){
	soundTimer = 0;
	
	
	defaults::audioInit();
}

void Audio::audioProcess(){

	//beep until down to 0
	if (soundTimer > 0x1){
		defaults::playAudio();
		
		soundTimer--;
	}

}

void Audio::setSoundTimer(uint16 input, CPU* cpu){
	//?x??
	uint8 *vx = cpu->getV((input & 0x0f00) >> 8);
	soundTimer = *vx;
}