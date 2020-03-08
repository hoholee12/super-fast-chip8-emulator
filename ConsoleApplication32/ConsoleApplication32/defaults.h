#pragma once
/*

defaults.h

-change default api easily by modifying this header

*/

#include<SDL.h>
#include<SDL_mixer.h>

typedef unsigned char uint8;
typedef unsigned short uint16;

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

public:

	void audioInit();
	void playAudio();

	void videoInit(char* str, int w, int h, int scale);
	void drawVideo(uint8* videoBuffer);

	void inputInit();
	uint8 getInput();
};



