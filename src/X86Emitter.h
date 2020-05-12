#pragma once

/*
	super simple and easy to understand x86(non-64bit) emitter - by hoholee12

	https://c9x.me/x86/
	http://ref.x86asm.net/coder32.html
	^great reference sites^

	some word instructions are similar with dword instructions and are separated with the existence of prefix(66h) on the first byte.

	http://www.c-jump.com/CIS77/CPU/x86/lecture.html
	^check this for detailed x86 opcode structures and stuff^

	x86 structure:
	prefix 0,4bytes | opcode 1,2bytes | modrm 0,1byte | sib 0,1byte | displacement 0,1,2,4bytes | immediate 0,1,2,4bytes	= 1 ~ 16 bytes long

	prefix -> idgaf

	opcode -> 1byte opcode(000000 + d + s) or 0x0F(extension) + 1byte opcode(000000 + d + s)
	000000 -> opcode(for example this is add)
	d ->	0: reg->r/m
			1: r/m->reg
	s ->	0: byte
			1: word or dword


	modrm -> mod 2b | reg 3b | r/m 3b	= 1byte
	
	mod ->	00: register indirect addressing, or sib with no disp(lacement)(r/m = 100), or displacement only addressing(r/m = 101)
			01: 1byte signed disp follows addressing bytes 
			10: 4byte signed disp follows addressing bytes 
			11: register addressing

	reg ->	000: Areg
			001: Creg
			010: Dreg
			011: Breg
			100: ah, sp, esp
			101: ch, bp, ebp (or disp if mod is 00)
			110: dh, si, esi
			111: bh, di, edi

	r/m (detailed list on what if mod is not 00):
			mod r/m
			 00 000 [ eax ]
			 01 000 [ eax + disp8 ]              
			 10 000 [ eax + disp32 ]
			 11 000 register  ( al / ax / eax )  
			 00 001 [ ecx ]
			 01 001 [ ecx + disp8 ]
			 10 001 [ ecx + disp32 ]
			 11 001 register  ( cl / cx / ecx )
			 00 010 [ edx ]
			 01 010 [ edx + disp8 ]
			 10 010 [ edx + disp32 ]
			 11 010 register  ( dl / dx / edx )
			 00 011 [ ebx ]
			 01 011 [ ebx + disp8 ]
			 10 011 [ ebx + disp32 ]
			 11 011 register  ( bl / bx / ebx )
			 00 100 SIB  Mode                     
			 01 100 SIB  +  disp8  Mode
			 10 100 SIB  +  disp32  Mode
			 11 100 register  ( ah / sp / esp )
			 00 101 32-bit Displacement-Only Mode
			 01 101 [ ebp + disp8 ]
			 10 101 [ ebp + disp32 ]
			 11 101 register  ( ch / bp / ebp )
			 00 110 [ esi ]
			 01 110 [ esi + disp8 ]
			 10 110 [ esi + disp32 ]
			 11 110 register  ( dh / si / esi )
			 00 111 [ edi ]
			 01 111 [ edi + disp8 ]
			 10 111 [ edi + disp32 ]
			 11 111 register  ( bh / di / edi )


	sib(scaled index byte) -> scale 2b | index 3b | base 3b		= 1byte
	
	scale ->	00: index * 1
				01: index * 2
				10: index * 4
				11: index * 8

	index ->	000: eax
				001: ecx
				010: edx
				011: ebx
				100: illegal
				101: ebp
				110: esi
				111: edi

	base ->		000: eax
				001: ecx
				010: edx
				011: ebx
				100: esp
				101: disp if mod = 00, ebp if mod = 01 or 10
				110: esi
				111: edi


	ex) add cl, al ->	000000 00 11 000 001	= 00 C1

	opcode ->	000000: 1byte add
	d ->		0: reg->mem
	s ->		0: byte (l)
	mod ->		11: r/m is a register
	reg ->		000: src Areg (al)
	r/m ->		001: dest Creg (cl)

	ex) add ecx, eax ->	000000 01 11 000 001	= 01 C1
	ex) add edx, disp -> 000000 11 00 011 101 disp32	= 03 1D ?? ?? ?? ??
	ex) add edi, [ebx] -> 000000 11 00 111 011		= 03 3B

	ex) movzx eax, al -> 0F B6 C0 = 00001111 101101 1 0 11 000 000
									 extend  movzx  d s mod reg r/m

	ex) mov al, memoryaddr -> 88 05 ?? ?? ?? ?? = 100010 0 0 00 000 101 ????...
													mov  d s mod reg r/m
	ex) mov memoryaddr, al -> 8A 05 ?? ?? ?? ?? = 100010 1 0 00 000 101 ????...
													mov  d s mod reg r/m
	ex) mov al, imm -> B0 ?? = 101100 0 0 ????...
								mov   d s 
	ex) mov ax, imm -> 66 B8 ?? ?? = 01100110 101110 0 0 00 ????...
									prefix    mov    d s mod
*/

#include<iostream>
#include<vector>
#include<stdint.h>
#include<string>

using vect8 = std::vector<uint8_t>; //tryinig really hard to shorten code here;-;

class X86Emitter{
private:

	vect8* memoryBlock;
	
	//host endianness sensitive
	typedef union{
		struct{
			uint8_t byte0;
			uint8_t byte1;
			uint8_t byte2;
			uint8_t byte3;
		};
		uint8_t byte;
		uint16_t word;
		uint32_t dword;
	} ByteRegs;
	ByteRegs byteRegs;

	void addByte(uint8_t byte){
		byteRegs.byte = byte;
		memoryBlock->push_back(byteRegs.byte0);
	}
	void addWord(uint16_t word){
		byteRegs.word = word;
		memoryBlock->push_back(byteRegs.byte0);
		memoryBlock->push_back(byteRegs.byte1);
	}
	void addDword(uint32_t dword){
		byteRegs.dword = dword;
		memoryBlock->push_back(byteRegs.byte0);
		memoryBlock->push_back(byteRegs.byte1);
		memoryBlock->push_back(byteRegs.byte2);
		memoryBlock->push_back(byteRegs.byte3);
	}

	//get memoryBlock from outside
	//memoryBlock optimization
	void init(vect8* inputMemoryBlock, int index = 0){
		memoryBlock = inputMemoryBlock;
		memoryBlock->reserve(memoryBlock->capacity() + index);
		byteRegs.dword = 0;
	}


public:

	enum Direction{ srcToDest, destToSrc = 1 }; //Direction -> srcToDest = 0, destToSrc = 1
	enum Bitsize{ byteOnly, wordAndDword = 1 }; //Bitsize -> byteOnly = 0, wordAndDword = 1

	//Mod -> forDisp = 00, byteSignedDisp = 01, dwordSignedDisp = 10, forReg = 11
	enum Mod{ forDisp = 0x0, byteSignedDisp = 0x1, dwordSignedDisp = 0x2, forReg = 0x3 };

	//X86Regs -> Areg = 000, Creg = 001, Dreg = 010, Breg = 011, illegal = 100, memaddr = 101
	enum X86Regs{ Areg = 0x0, Creg = 0x1, Dreg = 0x2, Breg = 0x3, illegal = 0x4, memaddr = 0x5 };

	enum Movsize{ movByte, movWord, movDword }; //for mov only

	//modrm
	using Modrm = struct{
		Mod mod;
		X86Regs src;
		X86Regs dest;
	};

	//displacement
	using Disp = struct{
		union{
			uint8_t byte;
			uint16_t word;
			uint32_t dword = 0;
		};

		void operator=(uint32_t dword){ this->dword = dword; }
		void operator=(uint16_t word){ this->word = word; }
		void operator=(uint8_t byte){ this->byte = byte; }

	};

	Disp insertDisp(int temp){ Disp disp; disp = (uint32_t)temp; return disp; }
	Disp insertAddr(int temp){ Disp disp; disp = (uint32_t)temp; return disp; }


	//sib
	enum Scale{ x1 = 0x0, x2 = 0x1, x4 = 0x2, x8 = 0x3 };
	using Index = X86Regs;
	using Base = X86Regs;
	using Sib = struct{
		Scale scale;
		Index index;
		Base base;
		void operator=(Scale scale){ this->scale = scale; }
	};

	//byte
	void addExtension(){ addByte(0x0F); }
	//byte
	void addPrefix(){ addByte(0x66); }
	//byte
	void addOpcode(uint8_t opcode, Direction direction, Bitsize bitsize){
		uint8_t d = 0x0;
		uint8_t s = 0x0;
		if (direction == 1) d = 0x2;
		if (bitsize == 1) s = 0x1;
		opcode |= d;
		opcode |= s;
		addByte(opcode);
	}
	//byte
	void addModrm(Mod mod, X86Regs src, X86Regs dest){
		uint8_t opcode = 0x0;
		opcode |= ((uint8_t)mod << 6);
		opcode |= ((uint8_t)src << 3);
		opcode |= (uint8_t)dest;
		addByte(opcode);
	}
	//byte
	void addSib(Scale scale, Index index, Base base){
		uint8_t opcode = 0x0;
		opcode |= ((uint8_t)scale << 6);
		opcode |= ((uint8_t)index << 3);
		opcode |= (uint8_t)base;
		addByte(opcode);
	}

	//use this template for jmp instructions
	enum OperandSizes{
		none = 0,
		movzxByteToDwordSize = 3,
		movzxWordToDwordSize = 3,
		movToMemaddrByteSize = 6,
		movToMemaddrDwordSize = 6,
		movToMemaddrWordSize = 7,
		movFromMemaddrByteSize = 6,
		movFromMemaddrDwordSize = 6,
		movFromMemaddrWordSize = 7,
		movDwordRegToRegSize = 2,
		movByteRegToMemSize = 2,
		movDwordRegToMemSize = 2,
		movWordRegToMemSize = 3,
		movByteMemToRegSize = 2,
		movDwordMemToRegSize = 2,
		movWordMemToRegSize = 3,
		dwordMovImmToAregSize = 5,
		dwordMovImmToBregSize = 5,
		dwordMovImmToCregSize = 5,
		dwordMovImmToDregSize = 5,
		dwordAddSize = 2,
		dwordAddImmToRegSize = 6,
		byteAddImmToMemaddrSize = 7,
		wordAddImmToMemaddrSize = 9,
		dwordAddImmToMemaddrSize = 10,
		dwordSubSize = 2,
		dwordAndSize = 2,
		dwordOrSize = 2,
		dwordXorSize = 2,
		dwordShiftLeftSize = 3,
		dwordShiftRightSize = 3,
		cmpSize = 2,
		byteRelJmpSize = 2,
		wordRelJmpSize = 4,
		dwordRelJmpSize = 5,
		byteRelJeSize = 2,
		byteRelJneSize = 2,
		byteRelJaSize = 2,
		byteRelJaeSize = 2,
		byteRelJbSize = 2,
		byteRelJbeSize = 2,
		leaWithDispSize = 7,
		leaWithoutDispSize = 3,

		retSize = 1,
		nopSize = 1,

		/*shortcuts!*/
		loadByteShortcutSize = movFromMemaddrByteSize + movzxByteToDwordSize,
		loadWordShortcutSize = movFromMemaddrWordSize + movzxWordToDwordSize,
		loadDwordShortcutSize = movFromMemaddrDwordSize,
		loadByteArraySize = dwordMovImmToAregSize + loadByteShortcutSize + dwordAddSize + movByteMemToRegSize + movzxByteToDwordSize,
		loadWordArraySize = dwordMovImmToAregSize + loadByteShortcutSize + leaWithoutDispSize + movWordMemToRegSize + movzxWordToDwordSize,
		loadDwordArraySize = dwordMovImmToAregSize + loadByteShortcutSize + leaWithoutDispSize + movDwordMemToRegSize,
		storeByteArraySize = dwordMovImmToAregSize + loadByteShortcutSize + dwordAddSize + movByteRegToMemSize,
		storeWordArraySize = dwordMovImmToAregSize + loadByteShortcutSize + leaWithoutDispSize + movWordRegToMemSize,
		storeDwordArraySize = dwordMovImmToAregSize + loadByteShortcutSize + leaWithoutDispSize + movDwordRegToMemSize,
		addByteToMemaddrSize = loadByteShortcutSize + dwordAddImmToRegSize + movToMemaddrByteSize,
		addWordToMemaddrSize = loadWordShortcutSize + dwordAddImmToRegSize + movToMemaddrWordSize,
		addDwordToMemaddrSize = loadDwordShortcutSize + dwordAddImmToRegSize + movToMemaddrDwordSize,
		setByteToMemaddrSize = dwordMovImmToAregSize + movToMemaddrByteSize,
		setWordToMemaddrSize = dwordMovImmToAregSize + movToMemaddrWordSize,
		setDwordToMemaddrSize = dwordMovImmToAregSize + movToMemaddrDwordSize,
	};

	enum OperandModes{
		movzxByteToDwordMode,
		movzxWordToDwordMode,
		movToMemaddrByteMode,
		movToMemaddrDwordMode,
		movToMemaddrWordMode,
		movFromMemaddrByteMode,
		movFromMemaddrDwordMode,
		movFromMemaddrWordMode,
		movDwordRegToRegMode,
		movByteRegToMemMode,
		movDwordRegToMemMode,
		movWordRegToMemMode,
		movByteMemToRegMode,
		movDwordMemToRegMode,
		movWordMemToRegMode,
		dwordMovImmToAregMode,
		dwordMovImmToBregMode,
		dwordMovImmToCregMode,
		dwordMovImmToDregMode,
		dwordAddMode,
		dwordAddImmToRegMode,
		byteAddImmToMemaddrMode,
		wordAddImmToMemaddrMode,
		dwordAddImmToMemaddrMode,
		dwordSubMode,
		dwordAndMode,
		dwordOrMode,
		dwordXorMode,
		dwordShiftLeftMode,
		dwordShiftRightMode,
		cmpMode,
		byteRelJmpMode,
		wordRelJmpMode,
		dwordRelJmpMode,
		byteRelJeMode,
		byteRelJneMode,
		byteRelJaMode,
		byteRelJaeMode,
		byteRelJbMode,
		byteRelJbeMode,
		leaWithDispMode,
		leaWithoutDispMode,

		retMode,
		nopMode,

		/*shortcuts!*/
		loadByteShortcutMode,
		loadWordShortcutMode,
		loadDwordShortcutMode,
		loadByteArrayMode,
		loadWordArrayMode,
		loadDwordArrayMode,
		storeByteArrayMode,
		storeWordArrayMode,
		storeDwordArrayMode,
		addByteToMemaddrMode,
		addWordToMemaddrMode,
		addDwordToMemaddrMode,
		setByteToMemaddrMode,
		setWordToMemaddrMode,
		setDwordToMemaddrMode,
	};

	void opmodeError(const char* str){ fprintf(stderr, str); fprintf(stderr, ": incompatible opmode!"); exit(1); }

	//no need for the opposite(use only for zeroing out high area)
	OperandSizes movzx(vect8* memoryBlock, OperandModes opmode, X86Regs src, X86Regs dest){
		uint8_t opcode = 0xB4; //10110100
		switch (opmode){
		case movzxByteToDwordMode: init(memoryBlock, movzxByteToDwordSize); addExtension(); addOpcode(opcode, destToSrc, byteOnly); addModrm(forReg, src, dest); return movzxByteToDwordSize;
		case movzxWordToDwordMode: init(memoryBlock, movzxWordToDwordSize); addExtension(); addOpcode(opcode, destToSrc, wordAndDword); addModrm(forReg, src, dest); return movzxWordToDwordSize;
		default: opmodeError("movzx");
		}
		
		return none;
	}
	//ex) convert byte to dword
	//movzx(memoryBlock, movzxByteToDwordMode, Areg, Areg);
	
	//ex) convert word to dword
	//movzx(memoryBlock, movzxWordToDwordMode, Areg, Areg);
	

	//memoryaddr must always be dword!
	OperandSizes mov(vect8* memoryBlock, OperandModes opmode, X86Regs src, X86Regs dest = memaddr, Disp disp = Disp()){
		uint8_t opcode = 0x88; //10001000
		switch (opmode){
		case movToMemaddrByteMode: init(memoryBlock, movToMemaddrByteSize); addOpcode(opcode, srcToDest, byteOnly); addModrm(forDisp, src, memaddr); addDword(disp.dword); return movToMemaddrByteSize;
		case movToMemaddrWordMode: init(memoryBlock, movToMemaddrWordSize); addPrefix(); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forDisp, src, memaddr); addDword(disp.dword); return movToMemaddrWordSize;
		case movToMemaddrDwordMode: init(memoryBlock, movToMemaddrDwordSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forDisp, src, memaddr); addDword(disp.dword); return movToMemaddrDwordSize;
		case movFromMemaddrByteMode: init(memoryBlock, movFromMemaddrByteSize); addOpcode(opcode, destToSrc, byteOnly); addModrm(forDisp, src, memaddr); addDword(disp.dword); return movFromMemaddrByteSize;
		case movFromMemaddrWordMode: init(memoryBlock, movFromMemaddrWordSize); addPrefix(); addOpcode(opcode, destToSrc, wordAndDword); addModrm(forDisp, src, memaddr); addDword(disp.dword); return movFromMemaddrWordSize;
		case movFromMemaddrDwordMode: init(memoryBlock, movFromMemaddrDwordSize); addOpcode(opcode, destToSrc, wordAndDword); addModrm(forDisp, src, memaddr); addDword(disp.dword); return movFromMemaddrDwordSize;
		
		case movByteMemToRegMode: init(memoryBlock, movByteMemToRegSize); addOpcode(opcode, destToSrc, byteOnly); addModrm(forDisp, dest, src); return movByteMemToRegSize;
		case movWordMemToRegMode: init(memoryBlock, movWordMemToRegSize); addPrefix(); addOpcode(opcode, destToSrc, wordAndDword); addModrm(forDisp, dest, src); return movWordMemToRegSize;
		case movDwordMemToRegMode: init(memoryBlock, movDwordMemToRegSize); addOpcode(opcode, destToSrc, wordAndDword); addModrm(forDisp, dest, src); return movDwordMemToRegSize;
		case movByteRegToMemMode: init(memoryBlock, movByteRegToMemSize); addOpcode(opcode, srcToDest, byteOnly); addModrm(forDisp, src, dest); return movByteRegToMemSize;
		case movWordRegToMemMode: init(memoryBlock, movWordRegToMemSize); addPrefix(); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forDisp, src, dest); return movWordRegToMemSize;
		case movDwordRegToMemMode: init(memoryBlock, movDwordRegToMemSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forDisp, src, dest); return movDwordRegToMemSize;
		case movDwordRegToRegMode: init(memoryBlock, movDwordRegToRegSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forReg, src, dest); return movDwordRegToRegSize;
		
		default: opmodeError("mov");
		}
		return none;
	}
	//override
	OperandSizes mov(vect8* memoryBlock, OperandModes opmode, X86Regs src, Disp disp = Disp()){ return mov(memoryBlock, opmode, src, Areg, disp); }



	//kinda different - use Direction and Bitsize to point reg
	OperandSizes mov_imm(vect8* memoryBlock, OperandModes opmode, Disp disp){
		uint8_t opcode = 0xB8;
		switch (opmode){
		case dwordMovImmToAregMode: init(memoryBlock, dwordMovImmToAregSize); addOpcode(opcode, srcToDest, byteOnly); addDword(disp.dword); return dwordMovImmToAregSize;
		case dwordMovImmToBregMode: init(memoryBlock, dwordMovImmToBregSize); addOpcode(opcode, destToSrc, wordAndDword); addDword(disp.dword); return dwordMovImmToBregSize;
		case dwordMovImmToCregMode: init(memoryBlock, dwordMovImmToCregSize); addOpcode(opcode, srcToDest, wordAndDword); addDword(disp.dword); return dwordMovImmToCregSize;
		case dwordMovImmToDregMode: init(memoryBlock, dwordMovImmToDregSize); addOpcode(opcode, destToSrc, byteOnly); addDword(disp.dword); return dwordMovImmToDregSize;

		default: opmodeError("mov_imm");
		}
		return none;
		
	}

	//dword add
	OperandSizes add(vect8* memoryBlock, OperandModes opmode, X86Regs src, X86Regs dest){
		uint8_t opcode = 0x00;
		switch (opmode){
		case dwordAddMode: init(memoryBlock, dwordAddSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forReg, src, dest); return dwordAddSize;

		default: opmodeError("add");
		}
		return none;
	}

	//dword imm add
	OperandSizes add_imm(vect8* memoryBlock, OperandModes opmode, Disp addr, Disp disp = Disp(), X86Regs dest = memaddr){
		uint8_t opcode = 0x80;
		switch (opmode){
		case dwordAddImmToRegMode: init(memoryBlock, dwordAddImmToRegSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forReg, Areg, dest); addDword(addr.dword); return dwordAddImmToRegSize;
	
		case byteAddImmToMemaddrMode: init(memoryBlock, byteAddImmToMemaddrSize); addOpcode(opcode, destToSrc, wordAndDword); addModrm(forDisp, Areg, memaddr); addDword(addr.dword); addByte(disp.byte); return byteAddImmToMemaddrSize;
		case wordAddImmToMemaddrMode: init(memoryBlock, wordAddImmToMemaddrSize); addPrefix(); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forDisp, Areg, memaddr); addDword(addr.dword); addWord(disp.word); return wordAddImmToMemaddrSize;
		case dwordAddImmToMemaddrMode: init(memoryBlock, dwordAddImmToMemaddrSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forDisp, Areg, memaddr); addDword(addr.dword); addDword(disp.dword); return dwordAddImmToMemaddrSize;
		default: opmodeError("add_imm");
		}
		return none;
	}
	//override
	OperandSizes add_imm(vect8* memoryBlock, OperandModes opmode, Disp disp, X86Regs dest = memaddr){ return add_imm(memoryBlock, opmode, disp, insertDisp(0x0), dest); }

	//add imm dword (AB <-> CD)
	//100000 0 1 11 000(?) 000 disp32
	/*
	
	TODO: add imm to MEMADDR too!!!!
	100000 1 1 00 000 101(?) disp32 disp8
	              areg

	prefix 100000 0 1 00 000 101(?) disp32 disp16

	100000 0 1 00 000 101(?) disp32 disp32
	*/
	
	
	//sub
	OperandSizes sub(vect8* memoryBlock, OperandModes opmode, X86Regs src, X86Regs dest){
		uint8_t opcode = 0x28;	//001010
		switch (opmode){
		case dwordSubMode: init(memoryBlock, dwordSubSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forReg, src, dest); return dwordSubSize;

		default: opmodeError("sub");
		}
		return none;
	}
	//sub dword (AB <-> CD)
	//001010 0 1 11 000 011


	//dword and
	OperandSizes and(vect8* memoryBlock, OperandModes opmode, X86Regs src, X86Regs dest){
		uint8_t opcode = 0x20; //001000
		switch (opmode){
		case dwordAndMode: init(memoryBlock, dwordAndSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forReg, src, dest); return dwordAndSize;
		default: opmodeError("and");
		}
		return none;
		
	}

	//dword or
	OperandSizes or(vect8* memoryBlock, OperandModes opmode, X86Regs src, X86Regs dest){
		uint8_t opcode = 0x08; //000010
		switch (opmode){
		case dwordOrMode: init(memoryBlock, dwordOrSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forReg, src, dest); return dwordOrSize;
		default: opmodeError("or");
		}
		return none;

	}

	//dword xor
	OperandSizes xor(vect8* memoryBlock, OperandModes opmode, X86Regs src, X86Regs dest){
		uint8_t opcode = 0x30; //001100
		switch (opmode){
		case dwordXorMode: init(memoryBlock, dwordXorSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forReg, src, dest); return dwordXorSize;
		default: opmodeError("xor");
		}
		return none;

	}

	//bitwise shl by immediate dword

	OperandSizes shift(vect8* memoryBlock, OperandModes opmode, Disp disp, X86Regs dest){
		uint8_t opcode = 0xC0;	//110000
		switch (opmode){
		case dwordShiftLeftMode: init(memoryBlock, dwordShiftLeftSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forReg, illegal, dest); addByte(disp.byte); return dwordShiftLeftSize;
		case dwordShiftRightMode: init(memoryBlock, dwordShiftRightSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forReg, memaddr, dest); addByte(disp.byte); return dwordShiftRightSize;
		default: opmodeError("shift");
		}
		return none;
		
	}

	//cmp
	OperandSizes cmp(vect8* memoryBlock, OperandModes opmode, X86Regs src, X86Regs dest){
		uint8_t opcode = 0x38; //001110
		switch (opmode){
		case cmpMode: init(memoryBlock, cmpSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forReg, dest, src); return cmpSize;
		default: opmodeError("cmp");
		}
		return none;
	}


	//jmp	jump
	OperandSizes jmp(vect8* memoryBlock, OperandModes opmode, Disp disp){
		uint8_t opcode = 0xE8; //111010
		switch (opmode){
		case byteRelJmpMode: init(memoryBlock, byteRelJmpSize); addOpcode(opcode, destToSrc, wordAndDword); addByte(disp.byte); return byteRelJmpSize;
		case wordRelJmpMode: init(memoryBlock, wordRelJmpSize); addPrefix(); addOpcode(opcode, srcToDest, wordAndDword); addWord(disp.word); return wordRelJmpSize;
		case dwordRelJmpMode: init(memoryBlock, dwordRelJmpSize); addOpcode(opcode, srcToDest, wordAndDword); addDword(disp.dword); return dwordRelJmpSize;
		default: opmodeError("jmp");
		}
		return none;
	}
	//je	jump equals
	//011101 0 0 11 001 011
	OperandSizes jcc(vect8* memoryBlock, OperandModes opmode, Disp disp){
		uint8_t opcode = 0x74;
		switch (opmode){
		case byteRelJeMode: init(memoryBlock, byteRelJeSize); addOpcode(opcode, srcToDest, byteOnly); addByte(disp.byte); return byteRelJeSize;
		case byteRelJneMode: init(memoryBlock, byteRelJneSize); addOpcode(opcode, srcToDest, wordAndDword); addByte(disp.byte); return byteRelJneSize;
		case byteRelJaMode: init(memoryBlock, byteRelJaSize); addOpcode(opcode, destToSrc, wordAndDword); addByte(disp.byte); return byteRelJaSize;
		case byteRelJbeMode: init(memoryBlock, byteRelJbeSize); addOpcode(opcode, destToSrc, byteOnly); addByte(disp.byte); return byteRelJbeSize;
		default: opmodeError("jcc");
		}
		return none;
		
	}
	//jne	jump not equals

	//jb	jump less unsigned
	//jbe	jump less equals unsigned
	//jcc2
	OperandSizes jcc2(vect8* memoryBlock, OperandModes opmode, Disp disp){
		uint8_t opcode = 0x70;
		switch (opmode){
		case byteRelJbMode: init(memoryBlock, byteRelJbSize); addOpcode(opcode, destToSrc, byteOnly); addByte(disp.byte); return byteRelJbSize;
		case byteRelJaeMode: init(memoryBlock, byteRelJaeSize); addOpcode(opcode, destToSrc, wordAndDword); addByte(disp.byte); return byteRelJaeSize;
		default: opmodeError("jcc2");
		}
		return none;
	}

	//ja	jump greater unsigned
	//jae	jump greater equals unsigned
	
	
	

	//return from eax
	OperandSizes ret(vect8* memoryBlock){ init(memoryBlock, retSize); addByte(0xC3); return retSize; }

	//nop
	OperandSizes nop(vect8* memoryBlock){ init(memoryBlock, nopSize); addByte(0x90); return nopSize; }

	//load effective address - lea <- shift + add
	//bse -> memaddr: some dword immediate(disp32) you can add along with main(multiplcation)
	//bse -> not memaddr: some reg(any reg) you can add along with main(multiplcation)
	OperandSizes lea(vect8* memoryBlock, OperandModes opmode, X86Regs src, Scale scale, X86Regs idx, X86Regs bse, Disp disp = Disp()){
		uint8_t opcode = 0x8C;					//100011 0 1 00 011 100 01 000 001
												//lea    d s md src dst sc idx bse
												//              Brg ill x2 Arg Crg	->	lea ebx, [eax*2 + ecx]

												//100011 0 1 00 000 100 01 000 101 disp32
												//				Arg ill sc Arg mem disp32 -> lea eax, [eax*2 + disp32]
		switch (opmode){
		case leaWithoutDispMode: init(memoryBlock, leaWithoutDispSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forDisp, src, illegal); addSib(scale, idx, bse); return leaWithoutDispSize;
		case leaWithDispMode: init(memoryBlock, leaWithDispSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forDisp, src, illegal); addSib(scale, idx, memaddr); addDword(disp.dword); return leaWithDispSize;

		default: opmodeError("lea");
		}
		return none;
	}



	/*
	
		shortcuts!
	
	
	*/
	//easy shortcut to load to register and expand
	OperandSizes loadByteToDwordRegA(vect8* memoryBlock, uint32_t dword){ mov(memoryBlock, movFromMemaddrByteMode, Areg, insertDisp(dword)); movzx(memoryBlock, movzxByteToDwordMode, Areg, Areg); return loadByteShortcutSize; }
	OperandSizes loadWordToDwordRegA(vect8* memoryBlock, uint32_t dword){ mov(memoryBlock, movFromMemaddrWordMode, Areg, insertDisp(dword)); movzx(memoryBlock, movzxWordToDwordMode, Areg, Areg); return loadWordShortcutSize; }
	OperandSizes loadDwordToDwordRegA(vect8* memoryBlock, uint32_t dword){ mov(memoryBlock, movFromMemaddrDwordMode, Areg, insertDisp(dword)); return loadDwordShortcutSize; }

	OperandSizes loadByteToDwordRegB(vect8* memoryBlock, uint32_t dword){ mov(memoryBlock, movFromMemaddrByteMode, Breg, insertDisp(dword)); movzx(memoryBlock, movzxByteToDwordMode, Breg, Breg); return loadByteShortcutSize; }
	OperandSizes loadWordToDwordRegB(vect8* memoryBlock, uint32_t dword){ mov(memoryBlock, movFromMemaddrWordMode, Breg, insertDisp(dword)); movzx(memoryBlock, movzxWordToDwordMode, Breg, Breg); return loadWordShortcutSize; }
	OperandSizes loadDwordToDwordRegB(vect8* memoryBlock, uint32_t dword){ mov(memoryBlock, movFromMemaddrDwordMode, Breg, insertDisp(dword)); return loadDwordShortcutSize; }

	OperandSizes loadByteToDwordRegC(vect8* memoryBlock, uint32_t dword){ mov(memoryBlock, movFromMemaddrByteMode, Creg, insertDisp(dword)); movzx(memoryBlock, movzxByteToDwordMode, Creg, Creg); return loadByteShortcutSize; }
	OperandSizes loadWordToDwordRegC(vect8* memoryBlock, uint32_t dword){ mov(memoryBlock, movFromMemaddrWordMode, Creg, insertDisp(dword)); movzx(memoryBlock, movzxWordToDwordMode, Creg, Creg); return loadWordShortcutSize; }
	OperandSizes loadDwordToDwordRegC(vect8* memoryBlock, uint32_t dword){ mov(memoryBlock, movFromMemaddrDwordMode, Creg, insertDisp(dword)); return loadDwordShortcutSize; }

	OperandSizes loadByteToDwordRegD(vect8* memoryBlock, uint32_t dword){ mov(memoryBlock, movFromMemaddrByteMode, Dreg, insertDisp(dword)); movzx(memoryBlock, movzxByteToDwordMode, Dreg, Dreg); return loadByteShortcutSize; }
	OperandSizes loadWordToDwordRegD(vect8* memoryBlock, uint32_t dword){ mov(memoryBlock, movFromMemaddrWordMode, Dreg, insertDisp(dword)); movzx(memoryBlock, movzxWordToDwordMode, Dreg, Dreg); return loadWordShortcutSize; }
	OperandSizes loadDwordToDwordRegD(vect8* memoryBlock, uint32_t dword){ mov(memoryBlock, movFromMemaddrDwordMode, Dreg, insertDisp(dword)); return loadDwordShortcutSize; }

	//preferred way to load/store array elements to register, ABC regs will get occupied, backup shit before using.
	OperandSizes loadByteArray_AregAsResult(vect8* memoryBlock, uint32_t array, uint32_t arrayptr){
		mov_imm(memoryBlock, dwordMovImmToAregMode, insertDisp(array)); //A for array
		loadByteToDwordRegB(memoryBlock, arrayptr);	//B for arrayptr
		
		add(memoryBlock, dwordAddMode, Areg, Breg); //B as target element addr
		
		mov(memoryBlock, movByteMemToRegMode, Breg, Areg);  //to Areg
		movzx(memoryBlock, movzxByteToDwordMode, Areg, Areg);	//expand
		return loadByteArraySize;
	}
	//preferred way to load/store array elements to register, ABC regs will get occupied, backup shit before using.
	OperandSizes loadWordArray_AregAsResult(vect8* memoryBlock, uint32_t array, uint32_t arrayptr){
		mov_imm(memoryBlock, dwordMovImmToAregMode, insertDisp(array)); //A for array
		loadByteToDwordRegB(memoryBlock, arrayptr);	//B for arrayptr

		lea(memoryBlock, leaWithoutDispMode, Breg, x2, Breg, Areg);

		mov(memoryBlock, movByteMemToRegMode, Breg, Areg);  //to Areg
		movzx(memoryBlock, movzxByteToDwordMode, Areg, Areg);	//expand
		return loadWordArraySize;
	}
	//preferred way to load/store array elements to register, ABC regs will get occupied, backup shit before using.
	OperandSizes loadDwordArray_AregAsResult(vect8* memoryBlock, uint32_t array, uint32_t arrayptr){
		mov_imm(memoryBlock, dwordMovImmToAregMode, insertDisp(array)); //A for array
		loadByteToDwordRegB(memoryBlock, arrayptr);	//B for arrayptr

		lea(memoryBlock, leaWithoutDispMode, Breg, x4, Breg, Areg);

		mov(memoryBlock, movByteMemToRegMode, Breg, Areg);  //to Areg
		return loadDwordArraySize;
	}
	//preferred way to load/store array elements to register, ABC regs will get occupied, backup shit before using.
	OperandSizes storeByteArray_AregAsInput(vect8* memoryBlock, uint32_t array, uint32_t arrayptr){
		mov_imm(memoryBlock, dwordMovImmToBregMode, insertDisp(array)); //B for array
		loadByteToDwordRegC(memoryBlock, arrayptr);	//C for arrayptr

		add(memoryBlock, dwordAddMode, Breg, Creg); //C as target element addr

		mov(memoryBlock, movByteRegToMemMode, Areg, Creg); //input to array
		return storeByteArraySize;
	}
	//preferred way to load/store array elements to register, ABC regs will get occupied, backup shit before using.
	OperandSizes storeWordArray_AregAsInput(vect8* memoryBlock, uint32_t array, uint32_t arrayptr){
		mov_imm(memoryBlock, dwordMovImmToBregMode, insertDisp(array)); //B for array
		loadByteToDwordRegC(memoryBlock, arrayptr);	//C for arrayptr


		lea(memoryBlock, leaWithoutDispMode, Creg, x2, Creg, Breg);

		mov(memoryBlock, movWordRegToMemMode, Areg, Creg); //input to array
		return storeWordArraySize;
	}
	//preferred way to load/store array elements to register, ABC regs will get occupied, backup shit before using.
	OperandSizes storeDwordArray_AregAsInput(vect8* memoryBlock, uint32_t array, uint32_t arrayptr){
		mov_imm(memoryBlock, dwordMovImmToBregMode, insertDisp(array)); //B for array
		loadByteToDwordRegC(memoryBlock, arrayptr); //C for arrayptr

		lea(memoryBlock, leaWithoutDispMode, Creg, x4, Creg, Breg);

		mov(memoryBlock, movDwordRegToMemMode, Areg, Creg); //input to array
		return storeDwordArraySize;
	}

	//shortcut to change one piece of memory variable without mumbojumbo, Areg is used.
	OperandSizes addByteToMemaddr(vect8* memoryBlock, uint32_t memvar, uint32_t immval){
		loadByteToDwordRegA(memoryBlock, memvar);
		add_imm(memoryBlock, dwordAddImmToRegMode, insertDisp(immval), Areg);
		mov(memoryBlock, movToMemaddrByteMode, Areg, insertDisp(memvar));
		return addByteToMemaddrSize;
	}
	//shortcut to change one piece of memory variable without mumbojumbo, Areg is used.
	OperandSizes addWordToMemaddr(vect8* memoryBlock, uint32_t memvar, uint32_t immval){
		loadWordToDwordRegA(memoryBlock, memvar);
		add_imm(memoryBlock, dwordAddImmToRegMode, insertDisp(immval), Areg);
		mov(memoryBlock, movToMemaddrWordMode, Areg, insertDisp(memvar));
		return addWordToMemaddrSize;
	}
	//shortcut to change one piece of memory variable without mumbojumbo, Areg is used.
	OperandSizes addDwordToMemaddr(vect8* memoryBlock, uint32_t memvar, uint32_t immval){
		loadDwordToDwordRegA(memoryBlock, memvar);
		add_imm(memoryBlock, dwordAddImmToRegMode, insertDisp(immval), Areg);
		mov(memoryBlock, movToMemaddrDwordMode, Areg, insertDisp(memvar));
		return addDwordToMemaddrSize;
	}

	//shortcut to change one piece of memory variable without mumbojumbo, Areg is used.
	OperandSizes setByteToMemaddr(vect8* memoryBlock, uint32_t memvar, uint32_t immval){
		mov_imm(memoryBlock, dwordMovImmToAregMode, insertDisp(immval));
		mov(memoryBlock, movToMemaddrByteMode, Areg, insertDisp(memvar));
		return setByteToMemaddrSize;
	}
	//shortcut to change one piece of memory variable without mumbojumbo, Areg is used.
	OperandSizes setWordToMemaddr(vect8* memoryBlock, uint32_t memvar, uint32_t immval){
		mov_imm(memoryBlock, dwordMovImmToAregMode, insertDisp(immval));
		mov(memoryBlock, movToMemaddrWordMode, Areg, insertDisp(memvar));
		return setWordToMemaddrSize;
	}
	//shortcut to change one piece of memory variable without mumbojumbo, Areg is used.
	OperandSizes setDwordToMemaddr(vect8* memoryBlock, uint32_t memvar, uint32_t immval){
		mov_imm(memoryBlock, dwordMovImmToAregMode, insertDisp(immval));
		mov(memoryBlock, movToMemaddrDwordMode, Areg, insertDisp(memvar));
		return setDwordToMemaddrSize;
	}

	/*
	
		text assembler(please dont write bs in it error checking is hard af)
	
		TODO: implement
	*/



	enum Operands{
		movzxOp,
		movOp,
		addOp,
		subOp,
		andOp,
		orOp,
		xorOp,
		shlOp,
		shrOp,
		cmpOp,
		jmpOp,
		jeOp,
		jneOp,
		jaOp,
		jbOp,
		jaeOp,
		jbeOp,
		retOp,
		nopOp,
		leaOp,

	};


	using string = std::string;
	string convertLowercase(string* str){
		string result;
		
		for (uint32_t i = 0; i < str->size(); i++){
			result.push_back(tolower(str->at(i)));
		}
		return result;
	}


	using Asstype = struct{
		Operands op;
		OperandModes opmode;
		Modrm modrm;
		Sib sib;
		Disp disp;
		int opusage;
	};

	
	
	void parse_op(Asstype* asstype, string* op_str){
		if(!op_str->compare("movzx")) asstype->op = movzxOp;
		else if(!op_str->compare("mov")) asstype->op = movOp;
		else if(!op_str->compare("add")) asstype->op = addOp;
		else if(!op_str->compare("sub")) asstype->op = subOp;
		else if(!op_str->compare("and")) asstype->op = andOp;
		else if(!op_str->compare("or")) asstype->op = orOp;
		else if(!op_str->compare("xor")) asstype->op = xorOp;
		else if(!op_str->compare("shl")) asstype->op = shlOp;
		else if (!op_str->compare("shr")) asstype->op = shrOp;
		else if(!op_str->compare("cmp")) asstype->op = cmpOp;
		else if(!op_str->compare("jmp")) asstype->op = jmpOp;
		else if(!op_str->compare("je")) asstype->op = jeOp;
		else if (!op_str->compare("jne")) asstype->op = jneOp;
		else if (!op_str->compare("ja")) asstype->op = jaOp;
		else if (!op_str->compare("jb")) asstype->op = jbOp;
		else if (!op_str->compare("jae")) asstype->op = jaeOp;
		else if (!op_str->compare("jbe")) asstype->op = jbeOp;
		else if(!op_str->compare("ret")) asstype->op = retOp;
		else if(!op_str->compare("nop")) asstype->op = nopOp;
		else if(!op_str->compare("lea")) asstype->op = leaOp;

	}

	bool isByte(string* str){ if ((str->find("byte") != string::npos) || (str->find("l") != string::npos)) return true; else return false; }
	bool isWord(string* str){ if ((str->find("word") != string::npos) || ((str->find("x") != string::npos) && (str->find("e") == string::npos))) return true; else return false; }
	bool isDword(string* str){ if ((str->find("dword") != string::npos) || (str->find("x") != string::npos)) return true; else return false; }
	bool isPtr(string* str){ if ((str->find("ptr") != string::npos) || (str->find("[") != string::npos)) return true; else return false; }
	bool isImm(string* str){ 
		string regNames[12] = { "al", "bl", "cl", "dl", "ax", "bx", "cx", "dx", "eax", "ebx", "ecx", "edx" };
		size_t shit = string::npos;
		for (int i = 0; i < 12; i++){
			if (str->find(regNames[i])){
				shit = 123; //some other than npos
			}
		}
		if (shit == string::npos) return true;
		else return false;
	}
	bool isReg(string* str){ return !isImm(str); }
	bool isMem(string* str){ return (isImm(str) && isPtr(str)); }

	void insertSrc(Asstype* asstype, string* from_str){
		if (from_str->find("a") != string::npos) asstype->modrm.src = Areg;
		else if (from_str->find("b") != string::npos) asstype->modrm.src = Breg;
		else if (from_str->find("c") != string::npos) asstype->modrm.src = Creg;
		else if (from_str->find("d") != string::npos) asstype->modrm.src = Dreg;
	}
	void insertDest(Asstype* asstype, string* to_str){
		if (to_str->find("a") != string::npos) asstype->modrm.dest = Areg;
		else if (to_str->find("b") != string::npos) asstype->modrm.dest = Breg;
		else if (to_str->find("c") != string::npos) asstype->modrm.dest = Creg;
		else if (to_str->find("d") != string::npos) asstype->modrm.dest = Dreg;
	}

	OperandSizes run_op(vect8* memoryBlock, Asstype* asstype, string* to_str, string* from_str){
		switch (asstype->op){
		case movzxOp:
			if (isReg(to_str) && isReg(from_str)){
				insertSrc(asstype, from_str); insertDest(asstype, to_str);
				if (isByte(from_str) && isDword(to_str)) return movzx(memoryBlock, movzxByteToDwordMode, asstype->modrm.src, asstype->modrm.dest);
				else if (isWord(from_str) && isDword(to_str)) return movzx(memoryBlock, movzxWordToDwordMode, asstype->modrm.src, asstype->modrm.dest);
			}
			break;
		case movOp:
			
			break;
		case addOp: break;
		case subOp: break;
		case andOp: break;
		case orOp: break;
		case xorOp: break;
		case shlOp: break;
		case shrOp: break;
		case cmpOp: break;
		case jmpOp: break;
		case jeOp: break;
		case jneOp: break;
		case jaOp: break;
		case jbOp: break;
		case jaeOp: break;
		case jbeOp: break;
		case retOp: break;
		case nopOp: break;
		case leaOp: break;
		default: opmodeError("ass"); return none;
		}
	
		return none;
	}

	string trim(const string& str){
		size_t first = str.find_first_not_of(' ');
		if (string::npos == first)
		{
			return str;
		}
		size_t last = str.find_last_not_of(' ');
		if (first == string::npos) return "";
		return str.substr(first, (last - first + 1));
	}

	OperandSizes ass(vect8* memoryBlock, const char* str){
		Asstype asstype;
		string op_str;
		string to_str;
		string from_str;
		string input;
		input = str;
		input = convertLowercase(&input);
		from_str = input.substr(input.find(",") + 1);
		from_str = trim(from_str);
		input = input.substr(0, input.find(","));
		to_str = input.substr(input.find(" ") + 1);
		to_str = trim(to_str);
		op_str = input.substr(0, input.find(" "));
		op_str = trim(op_str);

		//to op
		parse_op(&asstype, &op_str);

		return run_op(memoryBlock, &asstype, &to_str, &from_str);

	}


};

