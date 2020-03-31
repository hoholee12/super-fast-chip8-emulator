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



