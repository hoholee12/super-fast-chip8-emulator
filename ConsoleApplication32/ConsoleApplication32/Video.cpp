#include"Video.h"

void Video::init(char* str){
	//clear videobuffer
	for (int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); i++) videoBuffer[i] = 0;


	//SDL stuff
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow(str, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	int scan = 0;
	for (int y = 0; y < SCREEN_HEIGHT; y++){
		for (int x = 0; x < SCREEN_WIDTH; x++){
			scan = SCREEN_WIDTH * y + x;
			pixelRect[scan].x = x * SCALE;
			pixelRect[scan].y = y * SCALE;
			pixelRect[scan].w = SCALE;
			pixelRect[scan].h = SCALE;
		}

	}

	draw();

}

void Video::draw(){

	int scan = 0;

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer); //clear to blackscreen
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	for (int y = 0; y < SCREEN_HEIGHT; y++){
		for (int x = 0; x < SCREEN_WIDTH; x++){
			scan = SCREEN_WIDTH * y + x;
			if (videoBuffer[scan] > 0) SDL_SetRenderDrawColor(renderer, x * 4, y * 4, x * y * 16, 255);
			else SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

			SDL_RenderFillRect(renderer, &pixelRect[scan]);
		}

	}

	SDL_RenderPresent(renderer); //update

}

void Video::writeVBuffer(uint16 addr, uint8 input){
	videoBuffer[addr] = input;
}

uint8 Video::readVBuffer(uint16 addr){
	return videoBuffer[addr];
}

void Video::clearVBuffer(){
	for(int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); i++) videoBuffer[i] = 0;
}

void Video::copySprite(uint16 input, CPU* cpu, Memory* memory){
	//?x??
	uint8 *vx = cpu->getV((input & 0x0f00) >> 8);

	//??y?
	uint8 *vy = cpu->getV((input & 0x00f0) >> 4);

	//vf
	uint8 *vf = cpu->getV(0xf);

	//??nn
	uint8 n = input & 0x000f;
	*vf = 0x0; //default


	for (int y = 0; y < n; y++){
		for (int x = 0; x < 8; x++){
			int check1 = SCREEN_WIDTH * (*vy + y) + *vx + x;
			uint8 check2 = memory->read(*cpu->getIndexRegister() + y) << x;
			check2 >>= 7;
			//printf("%d ", check2);
			if ((videoBuffer[check1] & check2) != 0) *vf = 0x1;
			videoBuffer[check1] ^= check2;
		}
		//printf("\n");
	}
}