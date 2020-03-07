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
	//kk -byte(2 nibbles)
	//nnn -addr(3 nibbles)
	//x, y -Vx, Vy registers
	//index register

	/*
	00E0 -cls
	00EE -return from subroutine. read pc from stack. stackpointer -= 1
	
	1nnn -jump to nnn
	2nnn -call subroutine from nnn

	3xkk -skip to next instruction if Vx == kk
	4xkk -skip to next instruction if Vx != kk

	5xy0 -skip to next instruction if Vx == Vy

	6xkk -Vx = kk
	7xkk -Vx += kk

	8xy0 -Vx = Vy
	8xy1 -Vx |= Vy	OR
	8xy2 -Vx &= Vy	AND
	8xy3 -Vx ^= Vy	XOR
	8xy4 -Vx += Vy	ADD, VF is = carry		(if add value is greater than 255)
	8xy5 -Vx -= Vy	SUB, VF is = not borrow	(set 1 if result is positive. if 0 or less, set 0)
	
	8xy6 -Vx >>= 1		SHR 1, if least significant bit is 1 then VF = 1, otherwise 0 and Vx is shift right 1
	8xy7 -Vx = Vy - Vx	SUBN, VF is = not borrow
	8xyE -Vx <<= 1		SHL 1, if least significant bit is 1 then VF = 1, otherwise 0 and Vx is shift left 1

	9xy0 -skip to next instruction if Vx != Vy

	Annn -L = nnn

	Bnnn -jump to nnn + V0

	Cxkk -Vx = rand & kk

	Dxyn -display n bytes of sprite starting in the address L, to the monitor location (Vx, Vy). VF = collision(if pixel overlaps)

	Ex9E -skip to next instruction if value in Vx is keypressed
	ExA1 -skip to next instruction if value in Vx is Not keypressed

	Fx07 -Vx = delay_timer_value
	Fx0A -wait for keypress, store the value of the key to Vx

	Fx15 -delay_timer_value = Vx

	Fx18 -sound_timer_value = Vx

	Fx1E -L += Vx

	Fx29 -L = location of sprite for Vx?

	Fx33 -store BCD code of Vx in memory locations of L, L+1, L+2

	Fx55 -copy V0~Vx to memory location starting at L

	Fx65 -store V0~Vx from memory location starting at L

	*/

	//?x??
	uint8 *vx = &v[(input & 0x0f00) >> 8];

	//??y?
	uint8 *vy = &v[(input & 0x00f0) >> 4];

	//vf
	uint8 *vf = &v[0xf];

	//??kk
	uint8 kk = input & 0x00ff;

	//nnn
	uint16 nnn = input & 0x0fff;

	uint8 n = input & 0x000f;

	//1 = is a jump; dont increment pc
	int flag = 0;

	//first nibble
	switch ((input & 0xf000) >> 12){
	case 0x0:
		switch (input & 0x00ff){
		case 0xe0:	clearScreen(); //clearscreen
			break;
		case 0xee:	programCounter = stack[--stackPointer]; flag = 1;//return from subroutine
			break;
		}
		break;
	case 0x1:	programCounter = nnn; flag = 1;//jump to nnn
		break;
	case 0x2:	stack[stackPointer++] = programCounter; programCounter = input & 0x0fff; flag = 1;//call subroutine from nnn
		break;
	case 0x3:	if (*vx == kk) programCounter += 2; //skip if ==
		break;
	case 0x4:	if (*vx != kk) programCounter += 2; //skip if !=
		break;
	case 0x5:	if (*vx == *vy) programCounter += 2; //skip if vx == vy
		break;
	case 0x6:	*vx = kk; //into
		break;
	case 0x7:	*vx += *vx + kk;
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
		case 0x5:	*vf = (*vx > *vy) ? 0x1 : 0x0; *vx -= *vy;
			break;
		case 0x6:	*vf = (*vx & 0x000f == 0x1) ? 0x1 : 0x0; *vx >>= 1;
			break;
		case 0x7:	*vf = (*vx < *vy) ? 0x1 : 0x0; *vx = *vy - *vx;
			break;
		case 0xe:	*vf = ((*vx & 0xf000) >> 12 == 0x1) ? 0x1 : 0x0; *vx <<= 1;
			break;
		}
		break;
	case 0x9:	if (*vx != *vy) programCounter += 2; //skip if vx != vy
		break;
	case 0xa:	indexRegister = nnn;
		break;
	case 0xb:	programCounter = nnn + v[0]; flag = 1;
		break;
	case 0xc:	*vx = (rand() % 0xff) & kk;	//random
		break;
	case 0xd:	for (int i = 0; i < n; i++) videoBuffer[*vx * *vy + i] = mem[indexRegister + i];	//draw screen
		break;
	case 0xe:	
		switch (input & 0x00ff){
		case 0x9e:
			if (checkKeyInput(vx) == 1) programCounter += 2;
			break;
		case 0xa1:
			if (checkKeyInput(vx) != 1) programCounter += 2;
			break;
		}
		break;
	case 0xf:
		switch (input & 0x00ff){
		case 0x07:	*vx = delayTimer;
			break;
		case 0x0a:	checkKeyInput(vx, 1);
			break;
		case 0x15:	delayTimer = *vx;
			break;
		case 0x18:	soundTimer = *vx;
			break;
		case 0x1e:	indexRegister += *vx;
			break;
		case 0x29:	indexRegister = *vx * 5;	//TODO: ???
			break;
		case 0x33:	//bcd code
			mem[indexRegister] = *vx % 1000 / 100;
			mem[indexRegister + 1] = *vx % 100 / 10;
			mem[indexRegister + 2] = *vx % 10;
			break;
		case 0x55:	for (int i = 0; i < 16; i++) mem[indexRegister + i] = v[i];
			break;
		case 0x65:	for (int i = 0; i < 16; i++) v[i] = mem[indexRegister + i];
			break;
		}
		break;
	}

	if (flag != 1)	//only if its not jump
	programCounter += 2; //increment after fetch

}


/*
*flag = 0 : check if match
*flag = 1 : vx new input
*/
int CPU::checkKeyInput(uint8* vx, int flag){
	int match = 0;
	uint8 temp;
	SDL_Event e;
	while (SDL_PollEvent(&e)){
		switch (e.type){
		case SDLK_0: temp = 0x0;
			break;
		case SDLK_1: temp = 0x1;
			break;
		case SDLK_2: temp = 0x2;
			break;
		case SDLK_3: temp = 0x3;
			break;
		case SDLK_4: temp = 0x4;
			break;
		case SDLK_5: temp = 0x5;
			break;
		case SDLK_6: temp = 0x6;
			break;
		case SDLK_7: temp = 0x7;
			break;
		case SDLK_8: temp = 0x8;
			break;
		case SDLK_9: temp = 0x9;
			break;
		case SDLK_a: temp = 0xa;
			break;
		case SDLK_b: temp = 0xb;
			break;
		case SDLK_c: temp = 0xc;
			break;
		case SDLK_d: temp = 0xd;
			break;
		case SDLK_e: temp = 0xe;
			break;
		case SDLK_f: temp = 0xf;
			break;
		default:	temp = 0x0;
			break;
		}
	}

	if (temp == *vx) match = 1;
	if (flag == 1) *vx = temp;

	return match;
}

void CPU::clearScreen(){
	for (int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); i++){
		videoBuffer[i] = 0;
	}
}

void CPU::init(){
	srand(time(NULL));

	programCounter = 0x200; //start at 0x200
	stackPointer = 0;
	indexRegister = 0;

	delayTimer = 0;
	soundTimer = 0;

	clearScreen();
	load();

	//load fontset from beginning of memory
	for (int i = 0; i < FONT_COUNT * 5; i++){
		mem[i] = fontSet[i];
	}

	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("chip8 emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * 10, SCREEN_HEIGHT * 10, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);


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
	

	//decode
	decode(currentOpcode);

	SDL_Event e;
	while (SDL_PollEvent(&e)){
		if (e.type == SDL_QUIT) running = false;
	}
	SDL_Delay(1000 / 60); //60fps

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
	draw();


}

void CPU::draw(){

	int scan = 0;

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer); //clear to blackscreen
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_Rect rect;
	for (int i = 0; i < SCREEN_WIDTH; i++){
		for (int j = 0; j < SCREEN_HEIGHT; j++){
			scan = SCREEN_HEIGHT * i + j;
			if (videoBuffer[scan] > 0){
				rect.x = i * 10;
				rect.y = j * 10;
				rect.w = 10;
				rect.h = 10;
				SDL_RenderFillRect(renderer, &rect);
			}
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