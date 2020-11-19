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
#else
#include<SDL2/SDL.h>
#endif


#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<string>
#include<stdint.h>


//#define DEBUG_ME
//#define DEBUG_CACHE
#define DEBUG_TIME 0


class defaults{
public:

	mutable SDL_AudioDeviceID audio_device;
	mutable SDL_AudioSpec want, have;

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
	void pauseAudio() const;

	void videoInit(char* str, int w, int h, int scale) const;
	void drawVideo(uint8_t* videoBuffer) const;

	void inputInit() const;
	uint8_t getInput() const;

	//fps timer
	uint32_t checkTime() const;
	void delayTime(uint32_t input) const; //sleep

	void updateTitle(char* str, int cpuspeed, int fps, int frametime) const;

	void* getExecBuffer() const;
	void purgeExecBuffer(void* addr) const;
	static const int pagesize = 1024 * 4;	//4k

	//print debug with ticks
	void debugmsg(const char* str, ...){
		va_list args;
		va_start(args, str);

		printf("(debug:%dums): ", checkTime());
		vprintf(str, args);
		
		va_end(args);

	}
};

static void audio_callback(void* user, uint8_t* buf, int size){
	//signed 16bit
	int16_t* buffer = (int16_t*)buf;
	int length = size / 2;	//16 -> 2 x 8 bit sample
	int counter = (int)user;

	double samplerate = 44100;
	double gain = 28000;

	for (int i = 0; i < length; i++, counter++){
		double time = (double)counter / samplerate;
		buffer[i] = (int16_t)(gain * sin(2.0f * M_PI * 441.0f * time));
	}

}

inline void defaults::audioInit() const{
#ifdef _WIN32
	//SDL stuff
	putenv("SDL_AUDIODRIVER=DirectSound");
#endif
	SDL_Init(SDL_INIT_AUDIO);
	//printf("%s\n", SDL_GetError());
	//SDL_ClearError();

	//init audio device
	SDL_memset((void*)&want, 0, sizeof(want));
	want.freq = 44100;
	want.format = AUDIO_S16SYS;	//signed 16bit
	want.channels = 1;
	want.samples = 2048;
	want.callback = audio_callback;
	audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
	if (want.format != have.format){
		fprintf(stderr, "%s", SDL_GetError());
		exit(1);
	}

}

inline void defaults::playAudio() const{
	//play audio
	SDL_PauseAudioDevice(audio_device, 0);
}
inline void defaults::pauseAudio() const{
	//pause audio
	SDL_PauseAudioDevice(audio_device, 1);
}


inline void defaults::videoInit(char* str, int w, int h, int scale) const{
	screenWidth = w;
	screenHeight = h;

	//SDL stuff
	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow(str, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w * scale, h * scale, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	pixelRect = new SDL_Rect[w * h];

	int scan = 0;
	for (int y = 0; y < h; y++){
		for (int x = 0; x < w; x++){
			scan = w * y + x;
			pixelRect[scan].x = x * scale;
			pixelRect[scan].y = y * scale;
			pixelRect[scan].w = scale;
			pixelRect[scan].h = scale;
		}

	}



}

inline void defaults::drawVideo(uint8_t* videoBuffer) const{

	int scan = 0;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer); //clear to blackscreen
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	for (int y = 0; y < screenHeight; y++){
		for (int x = 0; x < screenWidth; x++){
			scan = screenWidth * y + x;
			if (videoBuffer[scan] > 0) SDL_SetRenderDrawColor(renderer, x * 4, y * 4, x * y * 16, 255);
			else SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

			SDL_RenderFillRect(renderer, &pixelRect[scan]);
		}

	}

	SDL_RenderPresent(renderer); //update

}

inline void defaults::inputInit() const{
	pressedKey = 0xfe;

}

inline void defaults::delayTime(uint32_t input) const{
	SDL_Delay(input);
}

inline void defaults::updateTitle(char* str, int cpuspeed, int fps, int frametime) const{
	using namespace std;


	strcpy(a0, str);
	strcat(a0, a1);
	strcat(a0, to_string(cpuspeed).c_str());
	//strcat(a0, a5);
	strcat(a0, a2);
	strcat(a0, to_string(fps).c_str());
	//strcat(a0, a5);
	strcat(a0, a3);
	strcat(a0, to_string(frametime).c_str());
	//strcat(a0, a5);
	strcat(a0, a4);

	//printf("%s\n", a0);

	SDL_SetWindowTitle(window, a0);

}


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
	
	void* buffer = NULL;
#ifdef _WIN32
	buffer = VirtualAlloc(NULL, pagesize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#elif __linux__
	buffer = mmap(NULL, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	//make sure failed alloc returns zero
	if(buffer == MAP_FAILED) buffer = NULL;
#endif
	
	return buffer;
}

inline void defaults::purgeExecBuffer(void* addr) const{ 

#ifdef _WIN32
	VirtualFree(addr, pagesize, MEM_RELEASE);
#elif __linux__
	munmap(addr, pagesize);
#endif

}




