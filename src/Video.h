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

	const bool operator==(const _QueueType& queueOp){
		return //(indexReg == queueOp.indexReg)
			(opcode == queueOp.opcode)
			//&& (vx == queueOp.vx)
			//&& (vy == queueOp.vy)
			//&& (vf == queueOp.vf)
			&& (jmpState == true)
			&& (queueOp.jmpState == true)
			;
	}
} QueueType;



class Video final{
private:

	uint8_t videoBuffer[SCREEN_WIDTH * SCREEN_HEIGHT]; //video buffer
	uint8_t frameBuffer[SCREEN_WIDTH * SCREEN_HEIGHT]; //one for real display

	

	//simple queue
	int offset_count = 0;
	int offset_limit = QUEUE_OFFSET; //how much loops to ignore before flush, disable deflicker if -1
	bool copyFlag = false; //signal to copy to fbuffer
	QueueType opcodeQueue[QUEUE_SIZE];
	uint32_t queuePointer = 0;	//first

	void enqueue(QueueType* inputQueue){ opcodeQueue[queueMask(queuePointer++)] = *inputQueue; }
	
	void emptyqueue(){ queuePointer = 0; }
	
	
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


	//clone to fbuffer
	void copyToFbuffer(){ for (int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); i++) frameBuffer[i] = videoBuffer[i]; }

	void clearVBuffer();

	void copySprite(uint16_t opcode, CPU* cpu, Memory* memory);

	//for chip8 endless looop
	void forceFlush(){
		copyToFbuffer();
	}

	void draw(defaults* mainwindow); //updates screen
	
	void init(char* str, defaults* mainwindow, int queue_offset = QUEUE_OFFSET); //i do this only to display filename on window bar

	void optimizations(QueueType* inputQueue);	//separate optimizations for video

};

inline void Video::optimizations(QueueType* inputQueue){

	//deflicker
	findOpcodefromQueue(inputQueue);
	if(copyFlag || offset_limit < 0){
#ifdef DEBUG_ME
		printf(">>flush queue\n");
#endif
		emptyqueue();
		copyToFbuffer();
		copyFlag = false;
	}
#ifdef DEBUG_ME
	printf(">>insert queue\n");
#endif
	enqueue(inputQueue);
}
