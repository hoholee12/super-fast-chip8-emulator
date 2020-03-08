#include"Chip8.h"


//argc argv is essential for SDL program!!!
int main(int argc, char** argv){
	Chip8 chip;
	chip.start(argv[1]);

	return 0;
}