#include<stdlib.h>
#include<stdio.h>
#include"CPU.h"

uint8 fontSet[FONT_COUNT * 5] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void CPU::decode(uint16 input){
	//TODO
	bool throwError = false; //if illegal instruction

	//program counter increments by 2. 16bit addr
	//nn -byte(2 nibbles)
	//nnn -addr(3 nibbles)
	//x, y -Vx, Vy registers
	//index register

	//?x??
	uint8 *vx = &v[(input & 0x0f00) >> 8];

	//??y?
	uint8 *vy = &v[(input & 0x00f0) >> 4];

	//vf
	uint8 *vf = &v[0xf];

	//??nn
	uint8 nn = input & 0x00ff;

	//nnn
	uint16 nnn = input & 0x0fff;

	uint16 n = input & 0x000f;

	//1 = is a jump; dont increment pc
	int flag = 0;

	//first nibble
	switch ((input & 0xf000) >> 12){
	case 0x0:
		switch (input & 0x00ff){
		case 0xe0:	for (int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); i++) videoBuffer[i] = 0;
			break;
		case 0xee:	programCounter = stack[--stackPointer]; flag = 1;//return from subroutine
			break;
		default:	stack[stackPointer++] = programCounter; programCounter = nnn; flag = 1;//call subroutine from nnn
			break;
		}
		break;
	case 0x1:	programCounter = nnn; flag = 1;//jump to nnn
		break;
	case 0x2:	stack[stackPointer++] = programCounter; programCounter = nnn; flag = 1;//call subroutine from nnn
		break;
	case 0x3:	if (*vx == nn) programCounter += 2; //skip if ==
		break;
	case 0x4:	if (*vx != nn) programCounter += 2; //skip if !=
		break;
	case 0x5:	if (*vx == *vy) programCounter += 2; //skip if vx == vy
		break;
	case 0x6:	*vx = nn; //into
		break;
	case 0x7:	*vx += nn;
		break;
	case 0x8:	
		switch (input & 0x000f){
		case 0x0:	*vx = *vy;
			break;
		case 0x1:	*vx |= *vy;
			break;
		case 0x2:	*vx &= *vy;
			break;
		case 0x3:	*vx ^= *vy;
			break;
		case 0x4:	*vf = (*vx + *vy > 0xff) ? 0x1 : 0x0; *vx += *vy;
			break;
		case 0x5:	*vf = (*vx < *vy) ? 0x0 : 0x1; *vx -= *vy;
			break;
		case 0x6:	*vf = *vx & 0x000f; *vx >>= 1;
			break;
		case 0x7:	*vf = (*vy < *vx) ? 0x0 : 0x1; *vx = *vy - *vx;
			break;
		case 0xe:	*vf = *vx & 0xf000; *vx <<= 1;
			break;
		default: throwError = true;
			break;
		}
		break;
	case 0x9:	if (*vx != *vy) programCounter += 2; //skip if vx != vy
		break;
	case 0xa:	indexRegister = nnn;
		break;
	case 0xb:	programCounter = nnn + v[0]; flag = 1;
		break;
	case 0xc:	*vx = (rand() % 0xff) & nn;	//random
		break;
	case 0xd:
		*vf = 0x0; //default
		for (int y = 0; y < n; y++){
			for (int x = 0; x < 8; x++){
				int check1 = SCREEN_WIDTH * (*vy + y) + *vx + x;
				uint8 check2 = mem[indexRegister + y] << x;
				check2 >>= 7;
				//printf("%d ", check2);
				if (videoBuffer[check1] & check2 != 0) *vf = 1;
				videoBuffer[check1] ^= check2;
			}
			//printf("\n");
		}
		break;
	case 0xe:	
		switch (input & 0x00ff){
		case 0x9e:
			if (pressedKey == *vx) programCounter += 2;
			break;
		case 0xa1:
			if (pressedKey != *vx) programCounter += 2;
			break;
		default: throwError = true;
			break;
		}
		break;
	case 0xf:
		switch (input & 0x00ff){
		case 0x07:	*vx = delayTimer;
			break;
		case 0x0a:	if (pressedKey != 0x10) *vx = pressedKey; else flag = 1; //wait again
			break;
		case 0x15:	delayTimer = *vx;
			break;
		case 0x18:	soundTimer = *vx;
			break;
		case 0x1e:	indexRegister += *vx;
			break;
		case 0x29:	indexRegister = *vx * 5;	//font is stored at mem[0 ~ FONT_COUNT * 5]
			break;
		case 0x33:	//bcd code
			mem[indexRegister] = *vx / 100;
			mem[indexRegister + 1] = (*vx / 10) % 10;
			mem[indexRegister + 2] = *vx % 10;
			break;
		case 0x55:	for (int i = 0; i <= (input & 0x0f00) >> 8; i++) mem[indexRegister++] = v[i];
			break;
		case 0x65:	for (int i = 0; i <= (input & 0x0f00) >> 8; i++) v[i] = mem[indexRegister++];
			break;
		default: throwError = true;
			break;
		}
		break;
	default: throwError = true;
		break;
	}

	if (throwError == true)
		fprintf(stderr, "unsupported opcode!: %x\n", input);

	if (flag != 1)	//only if its not jump
	programCounter += 2; //increment after fetch

}


void CPU::checkKeyInput(){
	
	SDL_Event e;
	while (SDL_PollEvent(&e)){
		if (e.type == SDL_QUIT) running = false;
		if (e.type == SDL_KEYDOWN) {
			switch (e.key.keysym.sym){
			case SDLK_x: pressedKey = 0x0;
				break;
			case SDLK_1: pressedKey = 0x1;
				break;
			case SDLK_2: pressedKey = 0x2;
				break;
			case SDLK_3: pressedKey = 0x3;
				break;
			case SDLK_q: pressedKey = 0x4;
				break;
			case SDLK_w: pressedKey = 0x5;
				break;
			case SDLK_e: pressedKey = 0x6;
				break;
			case SDLK_a: pressedKey = 0x7;
				break;
			case SDLK_s: pressedKey = 0x8;
				break;
			case SDLK_d: pressedKey = 0x9;
				break;
			case SDLK_z: pressedKey = 0xa;
				break;
			case SDLK_c: pressedKey = 0xb;
				break;
			case SDLK_4: pressedKey = 0xc;
				break;
			case SDLK_r: pressedKey = 0xd;
				break;
			case SDLK_f: pressedKey = 0xe;
				break;
			case SDLK_v: pressedKey = 0xf;
				break;
			}
		}
		if (e.type == SDL_KEYUP) pressedKey = 0x10;
	}
	//printf("pressed key : %x\n", pressedKey);

}

void CPU::init(){
	srand(time(NULL));

	programCounter = 0x200; //start at 0x200
	stackPointer = 0;
	indexRegister = 0;

	delayTimer = 0;
	soundTimer = 0;

	pressedKey = 0x10;

	load();

	//init v register
	for (int i = 0; i < V_REGISTER_SIZE; i++) v[i] = 0x0;

	//clear videobuffer
	for (int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); i++) videoBuffer[i] = 0;

	//load fontset from beginning of memory
	for (int i = 0; i < FONT_COUNT * 5; i++){
		mem[i] = fontSet[i];
	}

	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow(filestr, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, SDL_WINDOW_SHOWN);
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
void CPU::run(){
	running = true;
	while (running){
		update();
	}
	SDL_Quit();
}
void CPU::update(){
	//fetch - mem is 8bit, opcode is 16bit, big endian
	//mem[pc] as top 8bit + mem[pc+1] as bottom 8bit = 16bit
	currentOpcode = mem[programCounter] << 8 | mem[programCounter + 1];

	//keyInput
	checkKeyInput();

	//decode
	decode(currentOpcode);

	
	//SDL_Delay(1000 / 60); //60fps

	//delay timer
	if (delayTimer > 0x0){
		delayTimer--;
	}

	//beep until down to 0
	if (soundTimer > 0x1){
		printf("\a");
		soundTimer--;
	}


	//draw
	if ((currentOpcode & 0xf000) >> 12 == 0xd || (currentOpcode & 0x00ff) == 0xe0)	//only when draw called
		draw();


}

void CPU::draw(){

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

void CPU::start(char* str){
	filestr = str;
	init();
	run();
}
void CPU::load(){
	//load file
	FILE *file = fopen(filestr, "rb");
	if (file == NULL){
		fprintf(stderr, "WHERES THE FILE?!");
		exit(1);
	}

	//file size
	fseek(file, 0l, SEEK_END);
	int fsize = ftell(file);
	fseek(file, 0l, SEEK_SET);

	//file to buffer(not mem)
	uint8* buffer = (uint8*)malloc(fsize);
	fread(buffer, fsize, 1, file);
	fclose(file);

	for (int i = 0; i < fsize; i++){
		//start adding to mem from 0x200
		mem[0x200 + i] = buffer[i];
	}


}