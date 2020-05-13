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
		noop,
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

	OperandSizes opmodeError(const char* str){ fprintf(stderr, str); fprintf(stderr, ": incompatible opmode!"); exit(1); return none; }

	//no need for the opposite(use only for zeroing out high area)
	OperandSizes movzx(vect8* memoryBlock, OperandModes opmode, X86Regs src, X86Regs dest){
		uint8_t opcode = 0xB4; //10110100
		switch (opmode){
		case movzxByteToDwordMode: init(memoryBlock, movzxByteToDwordSize); addExtension(); addOpcode(opcode, destToSrc, byteOnly); addModrm(forReg, dest, src); return movzxByteToDwordSize;
		case movzxWordToDwordMode: init(memoryBlock, movzxWordToDwordSize); addExtension(); addOpcode(opcode, destToSrc, wordAndDword); addModrm(forReg, dest, src); return movzxWordToDwordSize;
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





	using string = std::string;
	string convertLowercase(string* str){
		string result;
		
		for (uint32_t i = 0; i < str->size(); i++){
			result.push_back(tolower(str->at(i)));
		}
		return result;
	}


	using ParserType = struct{
		OperandModes opmode;
		Modrm modrm;
		Sib sib;
		Disp disp;
		int opusage;
	};

	bool isByte(string* str){ if ((str->find("byte") != string::npos) || (str->find("l") != string::npos)) return true; else return false; }
	bool isWord(string* str){ if ((str->find("word") != string::npos) || ((str->find("x") != string::npos) && (str->find("e") == string::npos))) return true; else return false; }
	bool isDword(string* str){ if ((str->find("dword") != string::npos) || ((str->find("x") != string::npos) && (str->find("e") != string::npos))) return true; else return false; }
	bool isPtr(string* str){ if ((str->find("ptr") != string::npos) || (str->find("[") != string::npos)) return true; else return false; }
	bool isImm(string* str){
		string regNames[12] = { "al", "bl", "cl", "dl", "ax", "bx", "cx", "dx", "eax", "ebx", "ecx", "edx" };
		for (int i = 11; i > -1; i--){
			if (str->find(regNames[i]) != string::npos) return false;
		}
		return true;
	}
	bool isReg(string* str){ return !isImm(str); }
	bool isMem(string* str){ return (isImm(str) && isPtr(str)); }
	bool autoInsertExtra(ParserType* parserType, string* str, Disp extra){
		if (str->find("extra") != string::npos){
			parserType->disp.dword = extra.dword;
			return true;
		}
		return false;
	}

	void insertSrc(ParserType* parserType, string* src_str){
		if (src_str->find("a") != string::npos) parserType->modrm.src = Areg;
		else if (src_str->find("b") != string::npos) parserType->modrm.src = Breg;
		else if (src_str->find("c") != string::npos) parserType->modrm.src = Creg;
		else if (src_str->find("d") != string::npos) parserType->modrm.src = Dreg;
	}
	void insertDest(ParserType* parserType, string* dest_str){
		if (dest_str->find("a") != string::npos) parserType->modrm.dest = Areg;
		else if (dest_str->find("b") != string::npos) parserType->modrm.dest = Breg;
		else if (dest_str->find("c") != string::npos) parserType->modrm.dest = Creg;
		else if (dest_str->find("d") != string::npos) parserType->modrm.dest = Dreg;
	}
	void insertImm(ParserType* parserType, string* imm_str){
		//hex
		if (imm_str->find("0x") != string::npos) parserType->disp.dword = (uint32_t)std::stoi(*imm_str, 0, 16);
		//dec
		else parserType->disp.dword = (uint32_t)std::stoi(*imm_str, 0, 10);
	}

	void parse_op(ParserType* parserType, string* op_str, string* src_str, string* dest_str, Disp extra){
		if (!op_str->compare("movzx")){
			if (isReg(src_str) && isReg(dest_str)){
				insertSrc(parserType, src_str); insertDest(parserType, dest_str);
				if (isByte(src_str) && isDword(dest_str))
					parserType->opmode = movzxByteToDwordMode;
				else if (isWord(src_str) && isDword(dest_str))
					parserType->opmode = movzxWordToDwordMode;
				
				
			}
		}
		else if(!op_str->compare("mov")){
			if (isReg(src_str) && isReg(dest_str)){
				insertSrc(parserType, src_str); insertDest(parserType, dest_str);
				if (isByte(src_str) && isDword(dest_str) && isPtr(dest_str))
					parserType->opmode = movByteRegToMemMode;
				else if (isWord(src_str) && isDword(dest_str) && isPtr(dest_str))
					parserType->opmode = movWordRegToMemMode;
				else if (isDword(src_str) && isDword(dest_str) && isPtr(dest_str))
					parserType->opmode = movDwordRegToMemMode;
				else if (isDword(src_str) && isDword(dest_str) && !isPtr(src_str) && !isPtr(dest_str))
					parserType->opmode = movDwordRegToRegMode;
				else if (isByte(dest_str) && isDword(src_str) && isPtr(src_str))
					parserType->opmode = movByteMemToRegMode;
				else if (isWord(dest_str) && isDword(src_str) && isPtr(src_str))
					parserType->opmode = movWordMemToRegMode;
				else if (isDword(dest_str) && isDword(src_str) && isPtr(src_str))
					parserType->opmode = movDwordMemToRegMode;

				
			}
			//mov_imm
			else if (isImm(src_str) && isReg(dest_str) && ! isPtr(src_str)){
				insertImm(parserType, src_str); insertDest(parserType, dest_str);
				if (parserType->modrm.dest == Areg) parserType->opmode = dwordMovImmToAregMode;
				else if (parserType->modrm.dest == Breg) parserType->opmode = dwordMovImmToBregMode;
				else if (parserType->modrm.dest == Creg) parserType->opmode = dwordMovImmToCregMode;
				else if (parserType->modrm.dest == Dreg) parserType->opmode = dwordMovImmToDregMode;

				
			}
		}
		else if(!op_str->compare("add")){
			if (isReg(src_str) && !isPtr(src_str) && isReg(dest_str) && !isPtr(dest_str)){
				insertSrc(parserType, src_str); insertDest(parserType, dest_str);
				if (isDword(src_str) && isDword(dest_str)) parserType->opmode = dwordAddMode;

				
			}
			else if (isImm(src_str) && !isPtr(src_str) && isReg(dest_str) && !isPtr(dest_str)){
				insertImm(parserType, src_str); insertDest(parserType, dest_str);
				if (isDword(dest_str)) parserType->opmode = dwordAddImmToRegMode;

				
			}
		}
		else if(!op_str->compare("sub")){
			if (isReg(src_str) && !isPtr(src_str) && isReg(dest_str) && !isPtr(dest_str)){
				insertSrc(parserType, src_str); insertDest(parserType, dest_str);
				if (isDword(src_str) && isDword(dest_str)) parserType->opmode = dwordSubMode;

				
			}
		}
		else if(!op_str->compare("and")){
			if (isReg(src_str) && !isPtr(src_str) && isReg(dest_str) && !isPtr(dest_str)){
				insertSrc(parserType, src_str); insertDest(parserType, dest_str);
				if (isDword(src_str) && isDword(dest_str)) parserType->opmode = dwordAndMode;

				
			}
		}
		else if(!op_str->compare("or")){ 
			if (isReg(src_str) && !isPtr(src_str) && isReg(dest_str) && !isPtr(dest_str)){
				insertSrc(parserType, src_str); insertDest(parserType, dest_str);
				if (isDword(src_str) && isDword(dest_str)) parserType->opmode = dwordOrMode;

				
			}
		}
		else if(!op_str->compare("xor")){ 
			if (isReg(src_str) && !isPtr(src_str) && isReg(dest_str) && !isPtr(dest_str)){
				insertSrc(parserType, src_str); insertDest(parserType, dest_str);
				if (isDword(src_str) && isDword(dest_str)) parserType->opmode = dwordXorMode;

				
			}
		}
		else if(!op_str->compare("shl")){ 
			if (isImm(src_str) && !isPtr(src_str) && isReg(dest_str) && !isPtr(dest_str)){
				insertImm(parserType, src_str); insertDest(parserType, dest_str);
				if (isDword(dest_str)) parserType->opmode = dwordShiftLeftMode;

				
			}
		}
		else if (!op_str->compare("shr")){ 
			if (isImm(src_str) && !isPtr(src_str) && isReg(dest_str) && !isPtr(dest_str)){
				insertImm(parserType, src_str); insertDest(parserType, dest_str);
				if (isDword(dest_str)) parserType->opmode = dwordShiftRightMode;

				
			}
		}
		else if(!op_str->compare("cmp")){
			if (isReg(src_str) && !isPtr(src_str) && isReg(dest_str) && !isPtr(dest_str)){
				insertSrc(parserType, dest_str); insertDest(parserType, src_str);	//inversion
				if (isDword(src_str), isDword(dest_str)) parserType->opmode = cmpMode;

				
			}
		}

		//src_str is empty in operand with one input
		else if(!op_str->compare("jmp")){ 
			if (isImm(dest_str) && !isPtr(dest_str)){
				if (autoInsertExtra(parserType, dest_str, extra)) parserType->opmode = byteRelJmpMode;
				else insertImm(parserType, dest_str); //if its not extra then insert imm
				if (isByte(dest_str)) parserType->opmode = byteRelJmpMode;
				else if (isWord(dest_str)) parserType->opmode = wordRelJmpMode;
				else if (isDword(dest_str)) parserType->opmode = dwordRelJmpMode;

				
			}
		}
		else if(!op_str->compare("je")){ 
			if (isImm(dest_str) && !isPtr(dest_str)){
				if (autoInsertExtra(parserType, dest_str, extra)) parserType->opmode = byteRelJeMode;
				else insertImm(parserType, dest_str); //if its not extra then insert imm
				if (isByte(dest_str)) parserType->opmode = byteRelJeMode;

				
			}
		}
		else if (!op_str->compare("jne")){ 
			if (isImm(dest_str) && !isPtr(dest_str)){
				if (autoInsertExtra(parserType, dest_str, extra)) parserType->opmode = byteRelJneMode;
				else insertImm(parserType, dest_str); //if its not extra then insert imm
				if (isByte(dest_str)) parserType->opmode = byteRelJneMode;

				
			}
		}
		else if (!op_str->compare("ja")){ 
			if (isImm(dest_str) && !isPtr(dest_str)){
				if (autoInsertExtra(parserType, dest_str, extra)) parserType->opmode = byteRelJaMode;
				else insertImm(parserType, dest_str); //if its not extra then insert imm
				if (isByte(dest_str)) parserType->opmode = byteRelJaMode;

				
			}
		}
		else if (!op_str->compare("jb")){ 
			if (isImm(dest_str) && !isPtr(dest_str)){
				if (autoInsertExtra(parserType, dest_str, extra)) parserType->opmode = byteRelJbMode;
				else insertImm(parserType, dest_str); //if its not extra then insert imm
				if (isByte(dest_str)) parserType->opmode = byteRelJbMode;

				
			}
		}
		else if (!op_str->compare("jae")){ 
			if (isImm(dest_str) && !isPtr(dest_str)){
				if (autoInsertExtra(parserType, dest_str, extra)) parserType->opmode = byteRelJaeMode;
				else insertImm(parserType, dest_str); //if its not extra then insert imm
				if (isByte(dest_str)) parserType->opmode = byteRelJaeMode;

				
			}
		}
		else if (!op_str->compare("jbe")){ 
			if (isImm(dest_str) && !isPtr(dest_str)){
				if (autoInsertExtra(parserType, dest_str, extra)) parserType->opmode = byteRelJbeMode;
				else insertImm(parserType, dest_str); //if its not extra then insert imm
				if (isByte(dest_str)) parserType->opmode = byteRelJbeMode;

				
			}
		}
		else if(!op_str->compare("ret")){ 
			parserType->opmode = retMode;
		}
		else if(!op_str->compare("nop")){ 
			parserType->opmode = nopMode;
		}
		else if(!op_str->compare("lea")){
			parserType->opmode = leaWithDispMode;
		}

	}

	


	OperandSizes run_op(vect8* memoryBlock, ParserType* parserType){
		//all must return OperandSizes
		switch (parserType->opmode){
		case movzxByteToDwordMode: 
		case movzxWordToDwordMode: return movzx(memoryBlock, parserType->opmode, parserType->modrm.src, parserType->modrm.dest);
		case movToMemaddrByteMode:
		case movToMemaddrDwordMode: 
		case movToMemaddrWordMode: 
		case movFromMemaddrByteMode: 
		case movFromMemaddrDwordMode: 
		case movFromMemaddrWordMode: return opmodeError("not supported by parser...yet");
		case movDwordRegToRegMode: 
		case movByteRegToMemMode: 
		case movDwordRegToMemMode: 
		case movWordRegToMemMode: 
		case movByteMemToRegMode: 
		case movDwordMemToRegMode: 
		case movWordMemToRegMode: return mov(memoryBlock, parserType->opmode, parserType->modrm.src, parserType->modrm.dest);
		case dwordMovImmToAregMode: 
		case dwordMovImmToBregMode: 
		case dwordMovImmToCregMode: 
		case dwordMovImmToDregMode: return mov_imm(memoryBlock, parserType->opmode, parserType->disp);
		case dwordAddMode: return add(memoryBlock, parserType->opmode, parserType->modrm.src, parserType->modrm.dest);
		case dwordAddImmToRegMode: return add_imm(memoryBlock, parserType->opmode, parserType->disp, parserType->modrm.dest);
		case byteAddImmToMemaddrMode: 
		case wordAddImmToMemaddrMode: 
		case dwordAddImmToMemaddrMode: return opmodeError("not supported by parser...yet");
		case dwordSubMode: return sub(memoryBlock, parserType->opmode, parserType->modrm.src, parserType->modrm.dest);
		case dwordAndMode: return and(memoryBlock, parserType->opmode, parserType->modrm.src, parserType->modrm.dest);
		case dwordOrMode: return or(memoryBlock, parserType->opmode, parserType->modrm.src, parserType->modrm.dest);
		case dwordXorMode: return xor(memoryBlock, parserType->opmode, parserType->modrm.src, parserType->modrm.dest);
		case dwordShiftLeftMode: 
		case dwordShiftRightMode: return shift(memoryBlock, parserType->opmode, parserType->disp, parserType->modrm.dest);
		case cmpMode: return cmp(memoryBlock, parserType->opmode, parserType->modrm.src, parserType->modrm.dest);
		case byteRelJmpMode: 
		case wordRelJmpMode: 
		case dwordRelJmpMode: return jmp(memoryBlock, parserType->opmode, parserType->disp);
		case byteRelJeMode: 
		case byteRelJneMode: 
		case byteRelJaMode: 
		case byteRelJbeMode: return jcc(memoryBlock, parserType->opmode, parserType->disp);

		case byteRelJbMode:
		case byteRelJaeMode: return jcc2(memoryBlock, parserType->opmode, parserType->disp);
		
		
		case leaWithDispMode: 
		case leaWithoutDispMode: return opmodeError("i dont want to parse this... please use lea() instead");

		case retMode: return ret(memoryBlock);
		case nopMode: return nop(memoryBlock);
		
		default: opmodeError("parse"); return none;
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

	OperandSizes parse(vect8* memoryBlock, const char* str, Disp extra = Disp()){
		ParserType parserType;
		string op_str;
		string src_str;
		string dest_str;
		string input;
		input = str;
		input = convertLowercase(&input);
		src_str = input.substr(input.find(",") + 1);
		src_str = trim(src_str);
		input = input.substr(0, input.find(","));
		dest_str = input.substr(input.find(" ") + 1);
		dest_str = trim(dest_str);
		op_str = input.substr(0, input.find(" "));
		op_str = trim(op_str);
		if (src_str.find(op_str) != string::npos) src_str.clear();	//hax

		//to op
		parse_op(&parserType, &op_str, &src_str, &dest_str, extra);

		return run_op(memoryBlock, &parserType);

	}


};

