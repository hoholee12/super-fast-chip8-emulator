#include"Video.h"

void Video::init(char* str, defaults* mainwindow, int queue_offset){
	this->offset_limit = queue_offset;

	//clear videobuffer(also does fbuffer)
	clearVBuffer();

	mainwindow->videoInit(str, SCREEN_WIDTH, SCREEN_HEIGHT, SCALE);

	draw(mainwindow);

}

//real final draw
void Video::draw(defaults* mainwindow){

	int scan = 0;

	mainwindow->drawVideo(frameBuffer);


}


//also does fbuffer
void Video::clearVBuffer(){
	for(int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); i++) videoBuffer[i] = 0;
	copyToFbuffer();
}

void Video::copySprite(uint16_t opcode, CPU* cpu, Memory* memory){
	//?x??
	uint8_t *vx = cpu->getV((opcode & 0x0f00) >> 8);
	
	//??y?
	uint8_t *vy = cpu->getV((opcode & 0x00f0) >> 4);

	//vf
	uint8_t *vf = cpu->getV(0xf);

	//??nn
	uint8_t n = opcode & 0x000f;
	*vf = 0x0; //default

	//index register
	uint16_t indexReg = *cpu->getIndexRegister();

	uint8_t wrapX = *vx;// % SCREEN_WIDTH;
	uint8_t wrapY = *vy;// % SCREEN_HEIGHT;

	//draw to fbuffer before jmp
	optimizations(new QueueType{ opcode, cpu->prevJmpHint() });

#ifdef DEBUG_ME
	printf("indexReg = %x, x = %x, y = %x, f = %x, n = %x\n", *cpu->getIndexRegister(), *vx, *vy, *vf, n);
#endif
	for (int y = 0; y < n; y++){
		for (int x = 0; x < 8; x++){
			int check1 = SCREEN_WIDTH * (wrapY + y) + (wrapX + x);
			uint8_t check2 = memory->read(*cpu->getIndexRegister() + y) << x;
			check2 >>= 7;
			//printf("%d ", check2);
			if ((videoBuffer[check1] & check2) != 0) *vf = 0x1;
			videoBuffer[check1] ^= check2;
		}
		//printf("\n");
	}

	
}

