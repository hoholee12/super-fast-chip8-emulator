#include"Memory.h"

void Memory::write(uint16 addr, uint8 input){
	mem[addr] = input;

}

uint8 Memory::read(uint16 addr){
	return mem[addr];

}

void Memory::init(char* str){
	//init memory
	for (int i = 0; i < FULL_MEM_SIZE; i++) mem[i] = 0;

	//load fontset from beginning of memory
	for (int i = 0; i < FONT_COUNT * 5; i++) mem[i] = fontSet[i];

	//load the goddamn rom
	FILE *file = fopen(str, "rb");
	if (file == NULL){
		fprintf(stderr, "WHERES THE FILE?!");
		exit(1);
	}

	//file size
	fseek(file, 0l, SEEK_END);
	int fsize = ftell(file);
	fseek(file, 0l, SEEK_SET);

	//file to buffer(not mem)
	uint8* buffer = (uint8*)malloc(fsize);
	fread(buffer, fsize, 1, file);
	fclose(file);

	for (int i = 0; i < fsize; i++){
		//start adding to mem from START_MEM_SIZE
		mem[START_MEM_SIZE + i] = buffer[i];
	}

}