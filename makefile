src = $(filter-out src/X86Emitter.cpp, $(filter-out src/Translator.cpp,\
 $(filter-out src/Dynarec.cpp, $(filter-out src/Cache.cpp, $(wildcard src/*.cpp)))))
obj = $(src:.cpp=.o)

CC=g++
SDLFLAGS=`pkg-config --cflags --libs sdl2` -lSDL2main -lSDL2 -lSDL2_mixer
CFLAGS=-g -O2 -flto -Wno-unused-result

build: $(obj)
	$(CC) -o chip8-emu $^ $(SDLFLAGS) $(CFLAGS)

clean:
	rm -rf $(obj) chip8-emu
