#pragma once

/*
	super simple and easy to understand x86 emitter

*/


#include "CPU.h"

class X86Emitter{
private:

	uint8_t* memoryBlock;
	int* index;
	uint8_t byte0, byte1, byte2, byte3;
	void convertEndianness(uint16_t word){
		byte0 = (word & 0x00ff);
		byte1 = (word & 0xff00) >> 8;
	}
	void convertEndianness(uint32_t dword){
		byte0 = (dword & 0x000000ff);
		byte1 = (dword & 0x0000ff00) >> 8;
		byte2 = (dword & 0x00ff0000) >> 16;
		byte3 = (dword & 0xff000000) >> 24;
	}

	void addByte(uint8_t byte){
		memoryBlock[*index++] = byte;
	}
	void addWord(uint16_t word){
		convertEndianness(word);
		memoryBlock[*index++] = byte0;
		memoryBlock[*index++] = byte1;
	}
	void addDword(uint32_t dword){
		convertEndianness(dword);
		memoryBlock[*index++] = byte0;
		memoryBlock[*index++] = byte1;
		memoryBlock[*index++] = byte2;
		memoryBlock[*index++] = byte3;
	}
	//get memoryBlock from outside
	//memoryBlock must have exact size!
	void init(uint8_t* memoryBlock, int* index){
		this->memoryBlock = memoryBlock;
		this->index = index;
		byte0 = byte1 = byte2 = byte3 = 0;
	}
	
public:

	//converter byte -> word
	void movsx_bl_to_ax(uint8_t* memoryBlock, int* index){ init(memoryBlock, index); addByte(0x66);  addByte(0x0F); addByte(0xBE); addByte(0xC3); }
	void movsx_al_to_bx(uint8_t* memoryBlock, int* index){ init(memoryBlock, index); addByte(0x66);  addByte(0x0F); addByte(0xBE); addByte(0xD8); }
	//converter word -> dword
	void movsx_bx_to_eax(uint8_t* memoryBlock, int* index){ init(memoryBlock, index); addByte(0x0F); addByte(0xBF); addByte(0xC3); }
	void movsx_ax_to_ebx(uint8_t* memoryBlock, int* index){ init(memoryBlock, index); addByte(0x0F); addByte(0xBF); addByte(0xD8); }

	//mov dword
	void mov_immediate_to_eax(uint8_t* memoryBlock, uint32_t dword, int* index){ init(memoryBlock, index); addByte(0xB8); addDword(dword); }
	void mov_immediate_to_ebx(uint8_t* memoryBlock, uint32_t dword, int *index){ init(memoryBlock, index); addByte(0xBB); addDword(dword); }
	void mov_eax_to_memoryval(uint8_t* memoryBlock, uint32_t dword, int *index){ init(memoryBlock, index); addByte(0xA3); addDword(dword); }
	void mov_ebx_to_memoryval(uint8_t* memoryBlock, uint32_t dword, int *index){ init(memoryBlock, index); addByte(0x89); addByte(0x1D); addDword(dword); }
	void mov_memoryval_to_eax(uint8_t* memoryBlock, uint32_t dword, int *index){ init(memoryBlock, index); addByte(0xA1); addDword(dword); }
	void mov_memoryval_to_ebx(uint8_t* memoryBlock, uint32_t dword, int *index){ init(memoryBlock, index); addByte(0x8B); addByte(0x1D); addDword(dword); }
	void mov_eax_to_ebxaddr(uint8_t* memoryBlock, int *index){ init(memoryBlock, index); addByte(0x89); addByte(0x03); }
	void mov_ebx_to_eaxaddr(uint8_t* memoryBlock, int *index){ init(memoryBlock, index); addByte(0x89); addByte(0x18); }

	//use other mov instructions if its not here. no repetitive BS
	//mov word
	void mov_ax_to_bxaddr(uint8_t* memoryBlock, int *index){}
	void mov_bx_to_axaddr(uint8_t* memoryBlock, int *index){}
	void mov_ax_to_memoryval(uint8_t* memoryBlock, uint16_t word, int *index){}
	void mov_bx_to_memoryval(uint8_t* memoryBlock, uint16_t word, int *index){}
	void mov_memoryval_to_ax(uint8_t* memoryBlock, uint16_t word, int *index){}
	void mov_memoryval_to_bx(uint8_t* memoryBlock, uint16_t word, int *index){}
	//mov byte
	void mov_al_to_bladdr(uint8_t* memoryBlock, int *index){}
	void mov_bl_to_aladdr(uint8_t* memoryBlock, int *index){}
	void mov_al_to_memoryval(uint8_t* memoryBlock, uint8_t byte, int *index){}
	void mov_bl_to_memoryval(uint8_t* memoryBlock, uint8_t byte, int *index){}
	void mov_memoryval_to_al(uint8_t* memoryBlock, uint8_t byte, int *index){}
	void mov_memoryval_to_bl(uint8_t* memoryBlock, uint8_t byte, int *index){}

	//add dword
	void add_eax_to_ebx(uint8_t* memoryBlock, int *index){ init(memoryBlock, index); addByte(0x01); addByte(0xC3); }
	void add_immediate_to_eax(uint8_t* memoryBlock, uint32_t dword, int *index){ init(memoryBlock, index); addByte(0x03); addByte(0x05); addDword(dword); }
	void add_immediate_to_ebx(uint8_t* memoryBlock, uint32_t dword, int *index){ init(memoryBlock, index); addByte(0x03); addByte(0x1D); addDword(dword); }

	//sub dword
	void dec_eax(uint8_t* memoryBlock, int* index){ init(memoryBlock, index); addByte(0x48); }

	//bitwise dword
	void and_ebx_to_eax(uint8_t* memoryBlock, int *index){ init(memoryBlock, index); addByte(0x21); addByte(0xD8); }
	
};

