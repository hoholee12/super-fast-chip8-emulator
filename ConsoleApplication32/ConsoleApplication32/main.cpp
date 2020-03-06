#include<SDL/SDL.h>
#include<stdio.h>
#include"CPU.h"

//argc argv is essential for SDL program!!!
int main(int argc, char** argv){
	CPU _cpu;
	_cpu.start(argv[1]);

	return 0;
}