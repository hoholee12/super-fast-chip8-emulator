#pragma once
/*

defaults.h

-change default api easily by modifying this header

*/

#include<SDL.h>
#include<SDL_mixer.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>


typedef unsigned char uint8;
typedef unsigned short uint16;

typedef unsigned int uint32;	//for tick

class defaults{
public:
	Mix_Chunk* sound = NULL;
	const char* sound_file = "klik.wav";


	//SDL stuff
	SDL_Renderer* renderer;
	SDL_Window* window;
	SDL_Surface* screenSurface;
	SDL_Rect* pixelRect;

	int screenWidth;
	int screenHeight;

	uint8 pressedKey;

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
	void drawVideo(uint8* videoBuffer);

	void inputInit();
	uint8 getInput();

	//fps timer
	uint32 checkTime();
	void delayTime(uint32 input); //sleep

	void updateTitle(char* str, int cpuspeed, int fps, int frametime);
};



