#pragma once
/*

	defaults.h - proxy class(sorta)

	-change default api easily by modifying this header

*/


//first declaration
#ifdef __linux__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/mman.h>
#endif

#ifdef _WIN32
#include <windows.h>
#ifdef __cplusplus
extern "C"
#endif
#endif

#pragma warning(disable:4018; disable:4996; disable:4244)

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
#define DEBUG_CACHE
#define DEBUG_TIME 0


class defaults{
public:
	mutable Mix_Chunk* sound = NULL;
	const char* sound_file = "data/sound/klik.wav";


	//SDL stuff
	mutable SDL_Renderer* renderer;
	mutable SDL_Window* window;
	mutable SDL_Surface* screenSurface;
	mutable SDL_Rect* pixelRect;

	mutable int screenWidth;
	mutable int screenHeight;

	mutable uint8_t pressedKey;

	//title stuff
	mutable char a0[100];
	const char* a1 = ": cpu speed=";
	const char* a2 = " fps=";
	const char* a3 = " frametime=";
	const char* a4 = "ms";

	void audioInit() const;
	void playAudio() const;

	void videoInit(char* str, int w, int h, int scale) const;
	void drawVideo(uint8_t* videoBuffer) const;

	void inputInit() const;
	uint8_t getInput() const;

	//fps timer
	uint32_t checkTime() const;
	void delayTime(uint32_t input) const; //sleep

	void updateTitle(char* str, int cpuspeed, int fps, int frametime) const;

	void* getExecBuffer() const;

	//print debug with ticks
	void debugmsg(const char* str, ...){
		va_list args;
		va_start(args, str);

		printf("(debug:%dums): ", checkTime());
		vprintf(str, args);
		
		va_end(args);

	}
};

//inline getters
inline uint32_t defaults::checkTime() const{
	return SDL_GetTicks();
}

inline uint8_t defaults::getInput() const{

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




//execute a executable block
inline void* defaults::getExecBuffer() const{
	static const int pagesize = 1024 * 4;	//4k
	void* buffer = NULL;
#ifdef _WIN32
	buffer = VirtualAlloc(nullptr, pagesize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#elif __linux__
	buffer = mmap(NULL, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	//make sure failed alloc returns zero
	if(buffer == MAP_FAILED) buffer = NULL;
#endif
	
	return buffer;
}
