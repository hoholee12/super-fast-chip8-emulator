CC=g++
CFLAGS=`pkg-config --cflags --libs sdl2` -lSDL2main -lSDL2 -lSDL2_mixer


final: Audio.o Input.o Video.o Chip8.o defaults.o Frameskip.o Timer.o Memory.o CPU.o main.o Debug.o
	$(CC) -o chip8-emu Audio.o Input.o Video.o Chip8.o defaults.o Frameskip.o Timer.o Memory.o CPU.o main.o Debug.o $(CFLAGS)

Audio.o: Audio.cpp Audio.h
	$(CC) -g -c Audio.cpp

Input.o: Input.cpp Input.h
	$(CC) -g -c Input.cpp

Video.o: Video.cpp Video.h
	$(CC) -g -c Video.cpp

Chip8.o: Chip8.cpp Chip8.h
	$(CC) -g -c Chip8.cpp

defaults.o: defaults.cpp defaults.h
	$(CC) -g -c defaults.cpp

Frameskip.o: Frameskip.cpp Frameskip.h
	$(CC) -g -c Frameskip.cpp

Timer.o: Timer.cpp Timer.h
	$(CC) -g -c Timer.cpp

Memory.o: Memory.cpp Memory.h
	$(CC) -g -c Memory.cpp

CPU.o: CPU.cpp CPU.h
	$(CC) -g -c CPU.cpp

main.o: main.cpp
	$(CC) -g -c main.cpp

Debug.o: Debug.cpp Debug.h
	$(CC) -g -c Debug.cpp	

clean:
	rm -rf chip8-emu *.o
