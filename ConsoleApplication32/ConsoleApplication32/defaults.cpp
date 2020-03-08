#include"defaults.h"

void defaults::audioInit(){
	//SDL stuff
	putenv("SDL_AUDIODRIVER=DirectSound");
	//SDL_ClearError();
	SDL_Init(SDL_INIT_AUDIO);
	//printf("%s\n", SDL_GetError());
	//SDL_ClearError();
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	//printf("%s\n", SDL_GetError());
	sound = Mix_LoadWAV(sound_file);
	

}

void defaults::playAudio(){
	//play audio
	Mix_PlayChannel(-1, sound, 0);
}



void defaults::videoInit(char* str, int w, int h, int scale){
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

void defaults::drawVideo(uint8* videoBuffer){
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

void defaults::inputInit(){
	pressedKey = 0xfe;

}

uint8 defaults::getInput(){

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

void defaults::startTime(){
	prevTick = SDL_GetTicks();
}

void defaults::endTime(){
	uint32 currTick = SDL_GetTicks() - prevTick;
	holdTick = screenTicksPerFrame - currTick;
	if (holdTick > 0) SDL_Delay(holdTick);
	//printf("prevTick = %d, currTick = %d, holdTick = %d\n", prevTick, currTick, holdTick);
}