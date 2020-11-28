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

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>

//#define DEBUG_ME
//#define DEBUG_CACHE
#define DEBUG_TIME 0

#include"Status.h"

class defaults{
public:

	mutable Status* imstat;

	mutable SDL_AudioDeviceID audio_device;
	mutable SDL_AudioSpec want, have;

	//SDL stuff
	mutable SDL_Renderer* renderer;
	mutable SDL_Window* window;
	mutable SDL_Texture* texture;
	mutable SDL_Rect* pixelRect;

	//imgui stuff
	mutable ImGuiIO io;
	// Our state
    mutable bool show_demo_window = true;
    mutable bool show_another_window = false;
    mutable ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
	//opengl stuff
	mutable float vertices[4290];
	mutable uint8_t tempBuffer[64 * 32] = {0};
	mutable unsigned int VBO, VAO, EBO;
	mutable int shaderProgram;
	mutable unsigned int indices[12288] = {0};	//square for each pixel

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

	void videoInit(const char* str, int w, int h, int scale, Status* imstat) const;
	void drawVideo(uint8_t* videoBuffer) const;

	void inputInit() const;
	uint8_t getInput() const;

	//fps timer
	uint32_t checkTime() const;
	void delayTime(uint32_t input) const; //sleep

	void updateTitle(const char* str, int cpuspeed, int fps, int frametime) const;

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

static int convertVideoToIndices(uint8_t* videoBuffer, unsigned int* indices, int screenHeight, int screenWidth){
		memset(indices, 0, 12288 * sizeof(unsigned int));
		int scan = 0;
		int currentIndex = 0;
		for(int y = 0; y < screenHeight; y++){
			for(int x = 0; x < screenWidth; x++){
				scan = screenWidth * y + x;
				if(videoBuffer[scan] > 0){
					//triangle 1
					indices[currentIndex++] = 65 * y + x;
					indices[currentIndex++] = 65 * y + x + 1;
					indices[currentIndex++] = 65 * (y + 1) + x;
					//triangle 2
					indices[currentIndex++] = 65 * (y + 1) + x;
					indices[currentIndex++] = 65 * (y + 1) + x + 1;
					indices[currentIndex++] = 65 * y + x + 1;
				}
				else{
					currentIndex += 6;
				}
			}
		}
		//printf("currentIndex = %d\n", currentIndex);
		return currentIndex;	//indices length
	}


inline void defaults::videoInit(const char* str, int w, int h, int scale, Status* imstat) const{
	//load state
	this->imstat = imstat;
	if(imstat->get_reset()) return;	//skip the init on reset


	screenWidth = w;
	screenHeight = h;
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER);

	// GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	// Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
	printf("screenWidth = %d, screenHeight = %d\n", screenWidth, screenHeight);
    window = SDL_CreateWindow(str, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth * scale, screenHeight * scale, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

	gladLoadGL();

	 // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);


	//vertex shader
	const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	//fragment shader
	const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
    "}\n\0";
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	//link them together
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	//dont need this anymore
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//vertices for 64x32 display
	for(int y = 0; y < h + 1; y++){
		for(int x = 0; x < w + 1; x++){
			vertices[y * 130 + (x * 2)] = (-1.0f + x * 2.0f / 64.0f);
			//printf("vertices[%d] = %lf, ", y * 130 + (x * 2), vertices[y * 130 + (x * 2)]);
			vertices[y * 130 + (x * 2) + 1] = (1.0f - y * 2.0f / 32.0f);
			//printf("vertices[%d] = %lf\n", y * 130 + (x * 2) + 1, vertices[y * 130 + (x * 2) + 1]);
		}
	}

	int indicesLength = convertVideoToIndices(tempBuffer, indices, screenHeight, screenWidth);

	//generate EBO, VAO, VBO
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// 1. bind Vertex Array Object
	glBindVertexArray(VAO);
	// 2. copy our vertices array in a vertex buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// 3. copy our index array in a element buffer for OpenGL to use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indicesLength, indices, GL_DYNAMIC_DRAW);
	// 4. then set the vertex attributes pointers
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

}

inline void defaults::drawVideo(uint8_t* videoBuffer) const{

	

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	// imgui frame
	if (show_demo_window)
		if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Settings"))
        {
            if(ImGui::MenuItem("load rom", "")){


			}
			if(ImGui::MenuItem("reset", "")){
				imstat->set_reset(true);


			}
			ImGui::Separator();
			if(ImGui::MenuItem("quit", "")){
				exit(0);
			}

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Info"))
        {
			if(ImGui::MenuItem("vm status", "")){

			}
			if(ImGui::MenuItem("info", "")){
				

			}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

	// start rendering
	ImGui::Render();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);

	//video render
	int indicesLength = convertVideoToIndices(videoBuffer, indices, screenHeight, screenWidth);
	// 3. copy our index array in a element buffer for OpenGL to use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indicesLength, indices, GL_DYNAMIC_DRAW);

	//run shader
	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indicesLength, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);


	//imgui render
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	
	//sync
	SDL_GL_SwapWindow(window);

}

inline void defaults::inputInit() const{
	pressedKey = 0xfe;

}

inline void defaults::delayTime(uint32_t input) const{
	SDL_Delay(input);
}

inline void defaults::updateTitle(const char* str, int cpuspeed, int fps, int frametime) const{
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
