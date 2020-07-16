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
#define QUEUE_OFFSET 0

typedef struct _QueueType{
	//uint16_t indexReg;
	uint16_t opcode;
	//uint8_t vx;
	//uint8_t vy;
	//uint8_t vf;
	bool jmpState;
	bool overwriteState;

	const bool operator==(const _QueueType& queueOp){
		return //(indexReg == queueOp.indexReg)
			(opcode == queueOp.opcode)
			//&& (vx == queueOp.vx)
			//&& (vy == queueOp.vy)
			//&& (vf == queueOp.vf)
			&& ((jmpState == true)
			&& (queueOp.jmpState == true))
			//&& ((overwriteState == false)
			//&& (queueOp.overwriteState == false))
			;
	}
} QueueType;



class Video final{
private:

	uint8_t videoBuffer[SCREEN_WIDTH * SCREEN_HEIGHT]; //video buffer
	uint8_t frameBuffer[SCREEN_WIDTH * SCREEN_HEIGHT]; //one for real display

	bool overwriteHint = false;
	

	//simple queue
	int offset_count = 0;
	int offset_limit = QUEUE_OFFSET; //how much loops to ignore before flush, disable deflicker if -1
	bool copyFlag = false; //signal to copy to fbuffer
	QueueType opcodeQueue[QUEUE_SIZE];
	uint32_t queuePointer = 0;	//first

	uint32_t prevOp = 0;

	void enqueue(QueueType* inputQueue){ 

		prevOp = inputQueue->opcode;
		opcodeQueue[queueMask(queuePointer++)] = *inputQueue; 
#ifdef DEBUG_ME
		printf(">>insert queue: %d draws recorded\n", queuePointer);

		for (int i = 0; i < queuePointer; i++) printf("%d:\topcode:%02X\tjmp:%d\trewrite:%d\n"
			, i + 1, opcodeQueue->opcode, opcodeQueue->jmpState, opcodeQueue->overwriteState);
#endif
	}
	
	void empty_queue(){ 
#ifdef DEBUG_ME
		printf(">>flush queue\n");
#endif
		queuePointer = 0; }
	
	
	void findOpcodefromQueue(QueueType* inputQueue){
		for(uint32_t i = 0; i < queuePointer; i++){
			//detect loop
			if (*inputQueue == opcodeQueue[queueMask(i)]){

				//offset cycle delay
				if (++offset_count > offset_limit){

					//>>>>>either copy when loop found
					copyFlag = true;
					offset_count = 0;
					return;
				}
			}
		}

	}


	int queueMask(int i){
		if(i > queuePointer - 1){
			i = 0;

			//>>>>>or copy when limit exceeded
			copyFlag = true;
		}
		else if (i < 0){
			i = queuePointer - 1;
		}
		return i;
	}


public:

	bool prevOverwriteHint(){
		bool temp = overwriteHint;
		overwriteHint = false;	//reset
		return temp;
	}


	//clone to fbuffer
	void copyToFbuffer(){ for (int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); i++) frameBuffer[i] = videoBuffer[i]; }

	void clearVBuffer();

	void copySprite(CPU* cpu, Memory* memory, Video* video);

	//for chip8 endless looop
	void forceFlush(){
		empty_queue();
		copyToFbuffer();
	}

	void draw(defaults* mainwindow); //updates screen
	
	void init(char* str, defaults* mainwindow, int queue_offset = QUEUE_OFFSET); //i do this only to display filename on window bar

	void pre_optimizations(QueueType* inputQueue);	//separate optimizations for video
	void post_optimizations();

};

inline void Video::pre_optimizations(QueueType* inputQueue){

	//deflicker
	findOpcodefromQueue(inputQueue);
	if(copyFlag){
#ifndef DEBUG_ME
		if (queuePointer < 2) offset_limit = -1;	//hax for tetris
		if (prevOp == inputQueue->opcode) offset_limit = -1;	//hax for random
#endif
		forceFlush();
		copyFlag = false;
	}

	enqueue(inputQueue);
}

inline void Video::post_optimizations(){
	if( offset_limit < 0){
		forceFlush();
		copyFlag = false;
	}
}
