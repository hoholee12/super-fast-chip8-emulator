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

void Audio::setSoundTimer(CPU* cpu){
	//?x??
	uint8_t *vx = cpu->getV((cpu->currentOpcode & 0x0f00) >> 8);
	soundTimer = *vx;
}