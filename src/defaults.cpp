#include"defaults.h"

void defaults::audioInit() const{
#ifdef _WIN32
	//SDL stuff
	putenv("SDL_AUDIODRIVER=DirectSound");
#endif
	//SDL_ClearError();
	SDL_Init(SDL_INIT_AUDIO);
	//printf("%s\n", SDL_GetError());
	//SDL_ClearError();
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	//printf("%s\n", SDL_GetError());
	sound = Mix_LoadWAV(sound_file);
	

}

void defaults::playAudio() const{
	//play audio
	Mix_PlayChannel(-1, sound, 0);
}



void defaults::videoInit(char* str, int w, int h, int scale) const{
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

void defaults::drawVideo(uint8_t* videoBuffer) const{

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

void defaults::inputInit() const{
	pressedKey = 0xfe;

}

void defaults::delayTime(uint32_t input) const{
	SDL_Delay(input);
}

void defaults::updateTitle(char* str, int cpuspeed, int fps, int frametime) const{
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
