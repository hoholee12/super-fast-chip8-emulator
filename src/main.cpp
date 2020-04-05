#include"Chip8.h"
#include<stdio.h>

#ifdef __cplusplus
extern "C"
#endif

//argc argv is essential for SDL program!!!
int main(int argc, char** argv){
	Chip8 chip;
	int clockspeed;
	int fps;
	int method;
	int flickerOffset;

	if(argc < 2){
		fprintf(stderr, "feed me with a chip8 program.\n");
		exit(1);
	}
	printf("would you like to change emulation settings? (y/n):");
	switch (getchar()){
	case 'y':
		printf("interpreter method (1: switch/2: LUT/3: jumboLUT)(anything is fine): ");
		scanf("%d", &method);
		printf("clockspeed (1000hz(1000) works for most apps, try 100mhz(100000000) if you want to go extreme): ");
		scanf("%d", &clockspeed);
		printf("fps (default; 60fps): ");
		scanf("%d", &fps);
		printf("flicker offset (default; 0, however pong game requires 4): ");
		scanf("%d", &flickerOffset);
		printf("\nLOAD.\n");


		chip.start(argv[1], clockspeed, fps, method, flickerOffset);
		break;
	default:


		printf("\nLOAD.\n");


		chip.start(argv[1]);
	}

	return 0;
}
