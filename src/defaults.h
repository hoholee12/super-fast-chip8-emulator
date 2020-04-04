#pragma once
/*

defaults.h

-change default api easily by modifying this header

*/

#ifdef _WIN32
#include<SDL.h>
#include<SDL_mixer.h>	//this guy apparently requires SDL.h not SDL/SDL.h
#else
#include<SDL2/SDL.h>
#include<SDL2/SDL_mixer.h>
#endif

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<string>
#include<stdint.h>


//#define DEBUG_ME


class defaults{
public:
	Mix_Chunk* sound = NULL;
	const char* sound_file = "data/sound/klik.wav";


	//SDL stuff
	SDL_Renderer* renderer;
	SDL_Window* window;
	SDL_Surface* screenSurface;
	SDL_Rect* pixelRect;

	int screenWidth;
	int screenHeight;

	uint8_t pressedKey;

	//title stuff
	char a0[100];
	const char* a1 = ": cpu speed=";
	const char* a2 = " fps=";
	const char* a3 = " frametime=";
	const char* a4 = "ms";
	char a5[100];



	void audioInit();
	void playAudio();

	void videoInit(char* str, int w, int h, int scale);
	void drawVideo(uint8_t* videoBuffer);

	void inputInit();
	uint8_t getInput();

	//fps timer
	uint32_t checkTime();
	void delayTime(uint32_t input); //sleep

	void updateTitle(char* str, int cpuspeed, int fps, int frametime);
};

//inline getters
inline uint32_t defaults::checkTime(){
	return SDL_GetTicks();
}

inline uint8_t defaults::getInput(){

	SDL_Event e;
	while (SDL_PollEvent(&e)){
		if (e.type == SDL_QUIT) pressedKey = 0xff;
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
		if (e.type == SDL_KEYUP) pressedKey = 0xfe;
	}

	return pressedKey;
}

