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


	printf("do you want to change emulation settings? (y/n):");
	switch (getchar()){
	case 'y':
		printf("clockspeed: ");
		scanf("%d", &clockspeed);
		printf("fps: ");
		scanf("%d", &fps);
		printf("\nREADY.\n");


		chip.start(argv[1], clockspeed, fps);
		break;
	default:


		printf("\nREADY.\n");


		chip.start(argv[1]);
	}

	return 0;
}
