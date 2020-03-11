#pragma once
/*

defaults.h

-change default api easily by modifying this header

*/

#include<SDL.h>
#include<SDL_mixer.h>
#include<stdlib.h>
#include<stdio.h>

#define SCREEN_FPS 960
#define SKIP_FRAME 16

typedef unsigned char uint8;
typedef unsigned short uint16;

typedef unsigned int uint32;	//for tick

class defaults{
	Mix_Chunk* sound = NULL;
	char* sound_file = "klik.wav";


	//SDL stuff
	SDL_Renderer* renderer;
	SDL_Window* window;
	SDL_Surface* screenSurface;
	SDL_Rect* pixelRect;

	int screenWidth;
	int screenHeight;

	uint8 pressedKey;

	//fps timer
	uint32 screenFps = SCREEN_FPS;
	uint32 screenTicksPerFrame = 1000 / screenFps;

	uint32 frameCount;

	uint32 prevTick;
	int holdTick;

public:

	void audioInit();
	void playAudio();

	void videoInit(char* str, int w, int h, int scale);
	void drawVideo(uint8* videoBuffer);

	void inputInit();
	uint8 getInput();

	//fps timer
	void startTime();
	void endTime();
};



