#pragma once
#include"CPU.h"
#include"Memory.h"
#include"defaults.h"

#define SCREEN_WIDTH 0x40
#define SCREEN_HEIGHT 0x20
#define SCALE 20

//queue size
#define QUEUE_SIZE 0x100
//queue offset
#define QUEUE_OFFSET 4

class Video final{
private:

	uint8_t videoBuffer[SCREEN_WIDTH * SCREEN_HEIGHT]; //video buffer
	uint8_t frameBuffer[SCREEN_WIDTH * SCREEN_HEIGHT]; //one for real display

	//simple queue
	uint32_t offset_count = 0;
	uint32_t offset_limit = QUEUE_OFFSET; //how much loops to ignore before flush
	bool copyFlag = false; //signal to copy to fbuffer
	uint16_t opcodeQueue[QUEUE_SIZE];
	uint32_t queuePointer = 0;	//first
	void enqueue(uint16_t opcode){ opcodeQueue[queueMask(queuePointer++)] = opcode; }
	void emptyqueue(){ queuePointer = 0; }
	//either copy when loop found
	void findOpcodefromQueue(uint16_t opcode){
		for(int i = 0; i < queuePointer; i++){
			//detect loop
			if((opcode == opcodeQueue[queueMask(i)]) && (++offset_count > offset_limit)){
				copyFlag = true;
				offset_count = 0;
				return;
			}
		}

	}
	//or copy when limit exceeded
	int queueMask(int i){
		if(i > QUEUE_SIZE - 1){
			copyFlag = true;
		}
		return i % QUEUE_SIZE;
	}


public:

	void copyToFbuffer(); //clone to fbuffer

	void clearVBuffer();

	void copySprite(uint16_t opcode, CPU* cpu, Memory* memory);

	void draw(defaults* mainwindow); //updates screen
	
	void init(char* str, defaults* mainwindow, uint32_t queue_offset = QUEUE_OFFSET); //i do this only to display filename on window bar

	void optimizations(uint16_t opcode);	//separate optimizations for video

};

inline void Video::optimizations(uint16_t opcode){
	//deflicker
	findOpcodefromQueue(opcode);
	if(copyFlag){
		emptyqueue();
		copyToFbuffer();
		copyFlag = false;
	}
	enqueue(opcode);
}
