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

using vect8 = std::vector<uint8_t>; //tryinig really hard to shorten code here;-;

class X86Emitter{
private:

	vect8* memoryBlock;
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
		memoryBlock->push_back(byte);
	}
	void addWord(uint16_t word){
		convertEndianness(word);
		memoryBlock->push_back(byte0);
		memoryBlock->push_back(byte1);
	}
	void addDword(uint32_t dword){
		convertEndianness(dword);
		memoryBlock->push_back(byte0);
		memoryBlock->push_back(byte1);
		memoryBlock->push_back(byte2);
		memoryBlock->push_back(byte3);
	}

	//get memoryBlock from outside
	//memoryBlock optimization
	void init(vect8* inputMemoryBlock, int index = 0){
		memoryBlock = inputMemoryBlock;
		memoryBlock->reserve(memoryBlock->capacity() + index);
		byte0 = byte1 = byte2 = byte3 = 0;
	}





public:

	enum Direction{ srcToDest, destToSrc = 1 }; //Direction -> srcToDest = 0, destToSrc = 1
	enum Bitsize{ byteOnly, wordAndDword = 1 }; //Bitsize -> byteOnly = 0, wordAndDword = 1

	//Mod -> forDisp = 00, byteSignedDisp = 01, dwordSignedDisp = 10, forReg = 11
	enum Mod{ forDisp = 0x0, byteSignedDisp = 0x1, dwordSignedDisp = 0x2, forReg = 0x3 };

	//X86Regs -> Areg = 000, Creg = 001, Dreg = 010, Breg = 011, illegal = 100, memaddr = 101
	enum X86Regs{ Areg = 0x0, Creg = 0x1, Dreg = 0x2, Breg = 0x3, illegal = 0x4, memaddr = 0x5 };

	enum Movsize{ movByte, movWord, movDword }; //for mov only


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
	Disp disp;

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
	Sib sib;

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
		movzxSize = 3,
		movMemaddrByteSize = 6,
		movMemaddrDwordSize = 6,
		movMemaddrWordSize = 7,
		movByteSize = 2,
		movDwordSize = 2,
		movWordSize = 3,
		dwordMovImmSize = 5,
		dwordAddSize = 2,
		dwordAddImmSize = 6,
		incSize = 1,
		dwordSubSize = 2,
		decSize = 1,
		dwordAndSize = 2,
		dwordOrSize = 2,
		dwordXorSize = 2,
		dwordShiftSize = 3,
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

		/*shortcuts!*/
		//one thing to note - memaddr->reg is movXXSize, not movMemaddrXXSize
		loadByteShortcutSize = movByteSize + movzxSize,
		loadWordShortcutSize = movWordSize + movzxSize,
		loadDwordShortcutSize = movDwordSize,
		loadByteArraySize = dwordMovImmSize + loadByteShortcutSize + dwordAddSize + movByteSize + movzxSize,
		loadWordArraySize = dwordMovImmSize + loadByteShortcutSize + leaWithoutDispSize + movWordSize + movzxSize,
		loadDwordArraySize = dwordMovImmSize + loadByteShortcutSize + leaWithoutDispSize + movDwordSize,
		storeByteArraySize = dwordMovImmSize + loadByteShortcutSize + dwordAddSize + movByteSize,
		storeWordArraySize = dwordMovImmSize + loadByteShortcutSize + leaWithoutDispSize + movWordSize,
		storeDwordArraySize = dwordMovImmSize + loadByteShortcutSize + leaWithoutDispSize + movDwordSize,
		addByteToMemaddrSize = loadByteArraySize + dwordAddImmSize + movMemaddrByteSize,
		addWordToMemaddrSize = loadWordArraySize + dwordAddImmSize + movMemaddrWordSize,
		addDwordToMemaddrSize = loadDwordArraySize + dwordAddImmSize + movMemaddrDwordSize

	};

	//no need for the opposite(use only for zeroing out high area)
	void movzx(vect8* memoryBlock, Direction direction, Bitsize bitsize, Mod mod, X86Regs src, X86Regs dest){
		init(memoryBlock, movzxSize);
		addExtension();
		addOpcode(0xB4, direction, bitsize); //10110100
		addModrm(mod, src, dest);
	}
	//convert byte to dword
	void movzx_al_to_eax(vect8* memoryBlock){ movzx(memoryBlock, destToSrc, byteOnly, forReg, Areg, Areg); }
	void movzx_bl_to_ebx(vect8* memoryBlock){ movzx(memoryBlock, destToSrc, byteOnly, forReg, Breg, Breg); }
	void movzx_cl_to_ecx(vect8* memoryBlock){ movzx(memoryBlock, destToSrc, byteOnly, forReg, Creg, Creg); }
	void movzx_dl_to_edx(vect8* memoryBlock){ movzx(memoryBlock, destToSrc, byteOnly, forReg, Dreg, Dreg); }

	//convert word to dword
	void movzx_ax_to_eax(vect8* memoryBlock){ movzx(memoryBlock, destToSrc, wordAndDword, forReg, Areg, Areg); }
	void movzx_bx_to_ebx(vect8* memoryBlock){ movzx(memoryBlock, destToSrc, wordAndDword, forReg, Breg, Breg); }
	void movzx_cx_to_ecx(vect8* memoryBlock){ movzx(memoryBlock, destToSrc, wordAndDword, forReg, Creg, Creg); }
	void movzx_dx_to_edx(vect8* memoryBlock){ movzx(memoryBlock, destToSrc, wordAndDword, forReg, Dreg, Dreg); }


	//memoryaddr must always be dword!
	void mov(vect8* memoryBlock, Movsize getMovsize, Direction direction, Bitsize bitsize, Mod mod, X86Regs src, X86Regs dest){
		if ((getMovsize == movByte || getMovsize == movDword) && dest == memaddr)
			init(memoryBlock, movMemaddrByteSize);
		else if ((getMovsize == movWord) && dest == memaddr){
			init(memoryBlock, movMemaddrWordSize);
			addPrefix();
		}

		else if (getMovsize == movByte || getMovsize == movDword)
			init(memoryBlock, movByteSize);
		else if (getMovsize == movWord){
			init(memoryBlock, movWordSize);
			addPrefix();
		}

		addOpcode(0x88, direction, bitsize); //10001000
		addModrm(mod, src, dest);

		if (dest == memaddr) addDword(disp.dword);
	}
	//load store byte
	void mov_memoryaddr_to_al(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movByte, destToSrc, byteOnly, forDisp, Areg, memaddr); }
	void mov_al_to_memoryaddr(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movByte, srcToDest, byteOnly, forDisp, Areg, memaddr); }
	void mov_memoryaddr_to_bl(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movByte, destToSrc, byteOnly, forDisp, Breg, memaddr); }
	void mov_bl_to_memoryaddr(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movByte, srcToDest, byteOnly, forDisp, Breg, memaddr); }
	void mov_memoryaddr_to_cl(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movByte, destToSrc, byteOnly, forDisp, Creg, memaddr); }
	void mov_cl_to_memoryaddr(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movByte, srcToDest, byteOnly, forDisp, Creg, memaddr); }
	void mov_memoryaddr_to_dl(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movByte, destToSrc, byteOnly, forDisp, Dreg, memaddr); }
	void mov_dl_to_memoryaddr(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movByte, srcToDest, byteOnly, forDisp, Dreg, memaddr); }

	//load store word
	void mov_memoryaddr_to_ax(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movWord, destToSrc, wordAndDword, forDisp, Areg, memaddr); }
	void mov_ax_to_memoryaddr(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movWord, srcToDest, wordAndDword, forDisp, Areg, memaddr); }
	void mov_memoryaddr_to_bx(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movWord, destToSrc, wordAndDword, forDisp, Breg, memaddr); }
	void mov_bx_to_memoryaddr(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movWord, srcToDest, wordAndDword, forDisp, Breg, memaddr); }
	void mov_memoryaddr_to_cx(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movWord, destToSrc, wordAndDword, forDisp, Creg, memaddr); }
	void mov_cx_to_memoryaddr(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movWord, srcToDest, wordAndDword, forDisp, Creg, memaddr); }
	void mov_memoryaddr_to_dx(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movWord, destToSrc, wordAndDword, forDisp, Dreg, memaddr); }
	void mov_dx_to_memoryaddr(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movWord, srcToDest, wordAndDword, forDisp, Dreg, memaddr); }

	//load store dword
	void mov_memoryaddr_to_eax(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movDword, destToSrc, wordAndDword, forDisp, Areg, memaddr); }
	void mov_eax_to_memoryaddr(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movDword, srcToDest, wordAndDword, forDisp, Areg, memaddr); }
	void mov_memoryaddr_to_ebx(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movDword, destToSrc, wordAndDword, forDisp, Breg, memaddr); }
	void mov_ebx_to_memoryaddr(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movDword, srcToDest, wordAndDword, forDisp, Breg, memaddr); }
	void mov_memoryaddr_to_ecx(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movDword, destToSrc, wordAndDword, forDisp, Creg, memaddr); }
	void mov_ecx_to_memoryaddr(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movDword, srcToDest, wordAndDword, forDisp, Creg, memaddr); }
	void mov_memoryaddr_to_edx(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movDword, destToSrc, wordAndDword, forDisp, Dreg, memaddr); }
	void mov_edx_to_memoryaddr(vect8* memoryBlock, uint32_t dword){ disp = dword; mov(memoryBlock, movDword, srcToDest, wordAndDword, forDisp, Dreg, memaddr); }

	//store to regaddr byte (AB <-> CD)

	//100010 0 0 00 000 011
	//opcode d s mod src dst
	void mov_al_to_ebxaddr(vect8* memoryBlock){ mov(memoryBlock, movByte, srcToDest, byteOnly, forDisp, Areg, Breg); }
	void mov_al_to_ecxaddr(vect8* memoryBlock){ mov(memoryBlock, movByte, srcToDest, byteOnly, forDisp, Areg, Creg); }
	void mov_al_to_edxaddr(vect8* memoryBlock){ mov(memoryBlock, movByte, srcToDest, byteOnly, forDisp, Areg, Dreg); }

	void mov_bl_to_eaxaddr(vect8* memoryBlock){ mov(memoryBlock, movByte, srcToDest, byteOnly, forDisp, Breg, Areg); }
	void mov_bl_to_ecxaddr(vect8* memoryBlock){ mov(memoryBlock, movByte, srcToDest, byteOnly, forDisp, Breg, Creg); }
	void mov_bl_to_edxaddr(vect8* memoryBlock){ mov(memoryBlock, movByte, srcToDest, byteOnly, forDisp, Breg, Dreg); }

	void mov_cl_to_eaxaddr(vect8* memoryBlock){ mov(memoryBlock, movByte, srcToDest, byteOnly, forDisp, Creg, Areg); }
	void mov_cl_to_ebxaddr(vect8* memoryBlock){ mov(memoryBlock, movByte, srcToDest, byteOnly, forDisp, Creg, Breg); }
	void mov_cl_to_edxaddr(vect8* memoryBlock){ mov(memoryBlock, movByte, srcToDest, byteOnly, forDisp, Creg, Dreg); }

	void mov_dl_to_eaxaddr(vect8* memoryBlock){ mov(memoryBlock, movByte, srcToDest, byteOnly, forDisp, Dreg, Areg); }
	void mov_dl_to_ebxaddr(vect8* memoryBlock){ mov(memoryBlock, movByte, srcToDest, byteOnly, forDisp, Dreg, Breg); }
	void mov_dl_to_ecxaddr(vect8* memoryBlock){ mov(memoryBlock, movByte, srcToDest, byteOnly, forDisp, Dreg, Creg); }

	//store to regaddr word (AB <-> CD)
	//prefix 100010 0 1 00 000 011
	void mov_ax_to_ebxaddr(vect8* memoryBlock){ mov(memoryBlock, movWord, srcToDest, wordAndDword, forDisp, Areg, Breg); }
	void mov_ax_to_ecxaddr(vect8* memoryBlock){ mov(memoryBlock, movWord, srcToDest, wordAndDword, forDisp, Areg, Creg); }
	void mov_ax_to_edxaddr(vect8* memoryBlock){ mov(memoryBlock, movWord, srcToDest, wordAndDword, forDisp, Areg, Dreg); }

	void mov_bx_to_eaxaddr(vect8* memoryBlock){ mov(memoryBlock, movWord, srcToDest, wordAndDword, forDisp, Breg, Areg); }
	void mov_bx_to_ecxaddr(vect8* memoryBlock){ mov(memoryBlock, movWord, srcToDest, wordAndDword, forDisp, Breg, Creg); }
	void mov_bx_to_edxaddr(vect8* memoryBlock){ mov(memoryBlock, movWord, srcToDest, wordAndDword, forDisp, Breg, Dreg); }

	void mov_cx_to_eaxaddr(vect8* memoryBlock){ mov(memoryBlock, movWord, srcToDest, wordAndDword, forDisp, Creg, Areg); }
	void mov_cx_to_ebxaddr(vect8* memoryBlock){ mov(memoryBlock, movWord, srcToDest, wordAndDword, forDisp, Creg, Breg); }
	void mov_cx_to_edxaddr(vect8* memoryBlock){ mov(memoryBlock, movWord, srcToDest, wordAndDword, forDisp, Creg, Dreg); }

	void mov_dx_to_eaxaddr(vect8* memoryBlock){ mov(memoryBlock, movWord, srcToDest, wordAndDword, forDisp, Dreg, Areg); }
	void mov_dx_to_ebxaddr(vect8* memoryBlock){ mov(memoryBlock, movWord, srcToDest, wordAndDword, forDisp, Dreg, Breg); }
	void mov_dx_to_ecxaddr(vect8* memoryBlock){ mov(memoryBlock, movWord, srcToDest, wordAndDword, forDisp, Dreg, Creg); }

	//store to regaddr dword (AB <-> CD)
	//100010 0 1 00 000 011
	void mov_eax_to_ebxaddr(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forDisp, Areg, Breg); }
	void mov_eax_to_ecxaddr(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forDisp, Areg, Creg); }
	void mov_eax_to_edxaddr(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forDisp, Areg, Dreg); }

	void mov_ebx_to_eaxaddr(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forDisp, Breg, Areg); }
	void mov_ebx_to_ecxaddr(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forDisp, Breg, Creg); }
	void mov_ebx_to_edxaddr(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forDisp, Breg, Dreg); }

	void mov_ecx_to_eaxaddr(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forDisp, Creg, Areg); }
	void mov_ecx_to_ebxaddr(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forDisp, Creg, Breg); }
	void mov_ecx_to_edxaddr(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forDisp, Creg, Dreg); }

	void mov_edx_to_eaxaddr(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forDisp, Dreg, Areg); }
	void mov_edx_to_ebxaddr(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forDisp, Dreg, Breg); }
	void mov_edx_to_ecxaddr(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forDisp, Dreg, Creg); }

	//get from reg addr byte
	//100010 1 0 00 011 000
	void mov_eaxaddr_to_bl(vect8* memoryBlock){ mov(memoryBlock, movByte, destToSrc, byteOnly, forDisp, Breg, Areg); }
	void mov_eaxaddr_to_cl(vect8* memoryBlock){ mov(memoryBlock, movByte, destToSrc, byteOnly, forDisp, Creg, Areg); }
	void mov_eaxaddr_to_dl(vect8* memoryBlock){ mov(memoryBlock, movByte, destToSrc, byteOnly, forDisp, Dreg, Areg); }

	void mov_ebxaddr_to_al(vect8* memoryBlock){ mov(memoryBlock, movByte, destToSrc, byteOnly, forDisp, Areg, Breg); }
	void mov_ebxaddr_to_cl(vect8* memoryBlock){ mov(memoryBlock, movByte, destToSrc, byteOnly, forDisp, Creg, Breg); }
	void mov_ebxaddr_to_dl(vect8* memoryBlock){ mov(memoryBlock, movByte, destToSrc, byteOnly, forDisp, Dreg, Breg); }

	void mov_ecxaddr_to_al(vect8* memoryBlock){ mov(memoryBlock, movByte, destToSrc, byteOnly, forDisp, Areg, Creg); }
	void mov_ecxaddr_to_bl(vect8* memoryBlock){ mov(memoryBlock, movByte, destToSrc, byteOnly, forDisp, Breg, Creg); }
	void mov_ecxaddr_to_dl(vect8* memoryBlock){ mov(memoryBlock, movByte, destToSrc, byteOnly, forDisp, Dreg, Creg); }

	void mov_edxaddr_to_al(vect8* memoryBlock){ mov(memoryBlock, movByte, destToSrc, byteOnly, forDisp, Areg, Dreg); }
	void mov_edxaddr_to_bl(vect8* memoryBlock){ mov(memoryBlock, movByte, destToSrc, byteOnly, forDisp, Breg, Dreg); }
	void mov_edxaddr_to_cl(vect8* memoryBlock){ mov(memoryBlock, movByte, destToSrc, byteOnly, forDisp, Creg, Dreg); }


	//get from reg addr word
	//prefix 100010 1 1 00 011 000
	void mov_eaxaddr_to_bx(vect8* memoryBlock){ mov(memoryBlock, movWord, destToSrc, wordAndDword, forDisp, Breg, Areg); }
	void mov_eaxaddr_to_cx(vect8* memoryBlock){ mov(memoryBlock, movWord, destToSrc, wordAndDword, forDisp, Creg, Areg); }
	void mov_eaxaddr_to_dx(vect8* memoryBlock){ mov(memoryBlock, movWord, destToSrc, wordAndDword, forDisp, Dreg, Areg); }

	void mov_ebxaddr_to_ax(vect8* memoryBlock){ mov(memoryBlock, movWord, destToSrc, wordAndDword, forDisp, Areg, Breg); }
	void mov_ebxaddr_to_cx(vect8* memoryBlock){ mov(memoryBlock, movWord, destToSrc, wordAndDword, forDisp, Creg, Breg); }
	void mov_ebxaddr_to_dx(vect8* memoryBlock){ mov(memoryBlock, movWord, destToSrc, wordAndDword, forDisp, Dreg, Breg); }

	void mov_ecxaddr_to_ax(vect8* memoryBlock){ mov(memoryBlock, movWord, destToSrc, wordAndDword, forDisp, Areg, Creg); }
	void mov_ecxaddr_to_bx(vect8* memoryBlock){ mov(memoryBlock, movWord, destToSrc, wordAndDword, forDisp, Breg, Creg); }
	void mov_ecxaddr_to_dx(vect8* memoryBlock){ mov(memoryBlock, movWord, destToSrc, wordAndDword, forDisp, Dreg, Creg); }

	void mov_edxaddr_to_ax(vect8* memoryBlock){ mov(memoryBlock, movWord, destToSrc, wordAndDword, forDisp, Areg, Dreg); }
	void mov_edxaddr_to_bx(vect8* memoryBlock){ mov(memoryBlock, movWord, destToSrc, wordAndDword, forDisp, Breg, Dreg); }
	void mov_edxaddr_to_cx(vect8* memoryBlock){ mov(memoryBlock, movWord, destToSrc, wordAndDword, forDisp, Creg, Dreg); }


	//get from reg addr dword
	//100010 1 1 00 011 000
	void mov_eaxaddr_to_ebx(vect8* memoryBlock){ mov(memoryBlock, movDword, destToSrc, wordAndDword, forDisp, Breg, Areg); }
	void mov_eaxaddr_to_ecx(vect8* memoryBlock){ mov(memoryBlock, movDword, destToSrc, wordAndDword, forDisp, Creg, Areg); }
	void mov_eaxaddr_to_edx(vect8* memoryBlock){ mov(memoryBlock, movDword, destToSrc, wordAndDword, forDisp, Dreg, Areg); }

	void mov_ebxaddr_to_eax(vect8* memoryBlock){ mov(memoryBlock, movDword, destToSrc, wordAndDword, forDisp, Areg, Breg); }
	void mov_ebxaddr_to_ecx(vect8* memoryBlock){ mov(memoryBlock, movDword, destToSrc, wordAndDword, forDisp, Creg, Breg); }
	void mov_ebxaddr_to_edx(vect8* memoryBlock){ mov(memoryBlock, movDword, destToSrc, wordAndDword, forDisp, Dreg, Breg); }

	void mov_ecxaddr_to_eax(vect8* memoryBlock){ mov(memoryBlock, movDword, destToSrc, wordAndDword, forDisp, Areg, Creg); }
	void mov_ecxaddr_to_ebx(vect8* memoryBlock){ mov(memoryBlock, movDword, destToSrc, wordAndDword, forDisp, Breg, Creg); }
	void mov_ecxaddr_to_edx(vect8* memoryBlock){ mov(memoryBlock, movDword, destToSrc, wordAndDword, forDisp, Dreg, Creg); }

	void mov_edxaddr_to_eax(vect8* memoryBlock){ mov(memoryBlock, movDword, destToSrc, wordAndDword, forDisp, Areg, Dreg); }
	void mov_edxaddr_to_ebx(vect8* memoryBlock){ mov(memoryBlock, movDword, destToSrc, wordAndDword, forDisp, Breg, Dreg); }
	void mov_edxaddr_to_ecx(vect8* memoryBlock){ mov(memoryBlock, movDword, destToSrc, wordAndDword, forDisp, Creg, Dreg); }

	//simple dword move
	void mov_eax_to_ebx(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forReg, Areg, Breg); }
	void mov_eax_to_ecx(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forReg, Areg, Creg); }
	void mov_eax_to_edx(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forReg, Areg, Dreg); }

	void mov_ebx_to_eax(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forReg, Breg, Areg); }
	void mov_ebx_to_ecx(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forReg, Breg, Creg); }
	void mov_ebx_to_edx(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forReg, Breg, Dreg); }

	void mov_ecx_to_eax(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forReg, Creg, Areg); }
	void mov_ecx_to_ebx(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forReg, Creg, Breg); }
	void mov_ecx_to_edx(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forReg, Creg, Dreg); }

	void mov_edx_to_eax(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forReg, Dreg, Areg); }
	void mov_edx_to_ebx(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forReg, Dreg, Breg); }
	void mov_edx_to_ecx(vect8* memoryBlock){ mov(memoryBlock, movDword, srcToDest, wordAndDword, forReg, Dreg, Creg); }



	//kinda different - use Direction and Bitsize to point reg
	void dword_mov_imm(vect8* memoryBlock, Direction direction, Bitsize bitsize){
		init(memoryBlock, dwordMovImmSize);
		addOpcode(0xB8, direction, bitsize);
		addDword(disp.dword);
	}

	//load immediate dword
	void mov_imm_to_eax(vect8* memoryBlock, uint32_t dword){ disp = dword; dword_mov_imm(memoryBlock, srcToDest, byteOnly); }
	void mov_imm_to_ebx(vect8* memoryBlock, uint32_t dword){ disp = dword; dword_mov_imm(memoryBlock, destToSrc, wordAndDword); }
	void mov_imm_to_ecx(vect8* memoryBlock, uint32_t dword){ disp = dword; dword_mov_imm(memoryBlock, srcToDest, wordAndDword); }
	void mov_imm_to_edx(vect8* memoryBlock, uint32_t dword){ disp = dword; dword_mov_imm(memoryBlock, destToSrc, byteOnly); }


	//dword add
	void dword_add(vect8* memoryBlock, Direction direction, Bitsize bitsize, Mod mod, X86Regs src, X86Regs dest){
		init(memoryBlock, dwordAddSize);
		addOpcode(0x00, direction, bitsize); //000000
		addModrm(mod, src, dest);
	}
	//add regs dword (AB <-> CD)

	//000000 0 1 11 000 011
	void add_eax_to_ebx(vect8* memoryBlock){ dword_add(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Breg); }
	void add_eax_to_ecx(vect8* memoryBlock){ dword_add(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Creg); }
	void add_eax_to_edx(vect8* memoryBlock){ dword_add(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Dreg); }

	void add_ebx_to_eax(vect8* memoryBlock){ dword_add(memoryBlock, srcToDest, wordAndDword, forReg, Breg, Areg); }
	void add_ebx_to_ecx(vect8* memoryBlock){ dword_add(memoryBlock, srcToDest, wordAndDword, forReg, Breg, Creg); }
	void add_ebx_to_edx(vect8* memoryBlock){ dword_add(memoryBlock, srcToDest, wordAndDword, forReg, Breg, Dreg); }

	void add_ecx_to_eax(vect8* memoryBlock){ dword_add(memoryBlock, srcToDest, wordAndDword, forReg, Creg, Areg); }
	void add_ecx_to_ebx(vect8* memoryBlock){ dword_add(memoryBlock, srcToDest, wordAndDword, forReg, Creg, Breg); }
	void add_ecx_to_edx(vect8* memoryBlock){ dword_add(memoryBlock, srcToDest, wordAndDword, forReg, Creg, Dreg); }

	void add_edx_to_eax(vect8* memoryBlock){ dword_add(memoryBlock, srcToDest, wordAndDword, forReg, Dreg, Areg); }
	void add_edx_to_ebx(vect8* memoryBlock){ dword_add(memoryBlock, srcToDest, wordAndDword, forReg, Dreg, Breg); }
	void add_edx_to_ecx(vect8* memoryBlock){ dword_add(memoryBlock, srcToDest, wordAndDword, forReg, Dreg, Creg); }

	//dword imm add
	void dword_add_imm(vect8* memoryBlock, Direction direction, Bitsize bitsize, Mod mod, X86Regs src, X86Regs dest){
		init(memoryBlock, dwordAddImmSize);
		addOpcode(0x80, direction, bitsize);	//100000
		addModrm(mod, src, dest);
		addDword(disp.dword);
	}
	//add imm dword (AB <-> CD)
	//100000 0 1 11 000(?) 000 disp32
	void add_imm_to_eax(vect8* memoryBlock, uint32_t dword){ disp = dword; dword_add_imm(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Areg); }
	void add_imm_to_ebx(vect8* memoryBlock, uint32_t dword){ disp = dword; dword_add_imm(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Breg); }
	void add_imm_to_ecx(vect8* memoryBlock, uint32_t dword){ disp = dword; dword_add_imm(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Creg); }
	void add_imm_to_edx(vect8* memoryBlock, uint32_t dword){ disp = dword; dword_add_imm(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Dreg); }

	//dword inc
	void inc(vect8* memoryBlock, Direction direction, Bitsize bitsize){
		init(memoryBlock, incSize);
		addOpcode(0x40, direction, bitsize);
	}
	//inc dword
	//010000 0 0
	void inc_eax(vect8* memoryBlock){ inc(memoryBlock, srcToDest, byteOnly); }
	void inc_ebx(vect8* memoryBlock){ inc(memoryBlock, destToSrc, wordAndDword); }
	void inc_ecx(vect8* memoryBlock){ inc(memoryBlock, srcToDest, wordAndDword); }
	void inc_edx(vect8* memoryBlock){ inc(memoryBlock, destToSrc, byteOnly); }

	//dword sub
	void dword_sub(vect8* memoryBlock, Direction direction, Bitsize bitsize, Mod mod, X86Regs src, X86Regs dest){
		init(memoryBlock, dwordSubSize);
		addOpcode(0x28, direction, bitsize); //001010
		addModrm(mod, src, dest);
	}
	//sub dword (AB <-> CD)
	//001010 0 1 11 000 011
	void sub_eax_to_ebx(vect8* memoryBlock){ dword_sub(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Breg); }
	void sub_eax_to_ecx(vect8* memoryBlock){ dword_sub(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Creg); }
	void sub_eax_to_edx(vect8* memoryBlock){ dword_sub(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Dreg); }

	void sub_ebx_to_eax(vect8* memoryBlock){ dword_sub(memoryBlock, srcToDest, wordAndDword, forReg, Breg, Areg); }
	void sub_ebx_to_ecx(vect8* memoryBlock){ dword_sub(memoryBlock, srcToDest, wordAndDword, forReg, Breg, Creg); }
	void sub_ebx_to_edx(vect8* memoryBlock){ dword_sub(memoryBlock, srcToDest, wordAndDword, forReg, Breg, Dreg); }

	void sub_ecx_to_eax(vect8* memoryBlock){ dword_sub(memoryBlock, srcToDest, wordAndDword, forReg, Creg, Areg); }
	void sub_ecx_to_ebx(vect8* memoryBlock){ dword_sub(memoryBlock, srcToDest, wordAndDword, forReg, Creg, Breg); }
	void sub_ecx_to_edx(vect8* memoryBlock){ dword_sub(memoryBlock, srcToDest, wordAndDword, forReg, Creg, Dreg); }

	void sub_edx_to_eax(vect8* memoryBlock){ dword_sub(memoryBlock, srcToDest, wordAndDword, forReg, Dreg, Areg); }
	void sub_edx_to_ebx(vect8* memoryBlock){ dword_sub(memoryBlock, srcToDest, wordAndDword, forReg, Dreg, Breg); }
	void sub_edx_to_ecx(vect8* memoryBlock){ dword_sub(memoryBlock, srcToDest, wordAndDword, forReg, Dreg, Creg); }

	//dword dec
	void dec(vect8* memoryBlock, Direction direction, Bitsize bitsize){
		init(memoryBlock, decSize);
		addOpcode(0x48, direction, bitsize);	//010010 0 0
	}
	//dec dword
	void dec_eax(vect8* memoryBlock){ dec(memoryBlock, srcToDest, byteOnly); }
	void dec_ebx(vect8* memoryBlock){ dec(memoryBlock, destToSrc, wordAndDword); }
	void dec_ecx(vect8* memoryBlock){ dec(memoryBlock, srcToDest, wordAndDword); }
	void dec_edx(vect8* memoryBlock){ dec(memoryBlock, destToSrc, byteOnly); }


	//dword and
	void dword_and(vect8* memoryBlock, Direction direction, Bitsize bitsize, Mod mod, X86Regs src, X86Regs dest){
		init(memoryBlock, dwordAndSize);
		addOpcode(0x20, direction, bitsize); //001000
		addModrm(mod, src, dest);
	}
	//001000 0 1 11 000 011
	void and_eax_to_ebx(vect8* memoryBlock){ dword_and(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Breg); }
	void and_eax_to_ecx(vect8* memoryBlock){ dword_and(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Creg); }
	void and_eax_to_edx(vect8* memoryBlock){ dword_and(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Dreg); }
	void and_ebx_to_eax(vect8* memoryBlock){ dword_and(memoryBlock, srcToDest, wordAndDword, forReg, Breg, Areg); }
	void and_ebx_to_ecx(vect8* memoryBlock){ dword_and(memoryBlock, srcToDest, wordAndDword, forReg, Breg, Creg); }
	void and_ebx_to_edx(vect8* memoryBlock){ dword_and(memoryBlock, srcToDest, wordAndDword, forReg, Breg, Dreg); }
	void and_ecx_to_eax(vect8* memoryBlock){ dword_and(memoryBlock, srcToDest, wordAndDword, forReg, Creg, Areg); }
	void and_ecx_to_ebx(vect8* memoryBlock){ dword_and(memoryBlock, srcToDest, wordAndDword, forReg, Creg, Breg); }
	void and_ecx_to_edx(vect8* memoryBlock){ dword_and(memoryBlock, srcToDest, wordAndDword, forReg, Creg, Dreg); }
	void and_edx_to_eax(vect8* memoryBlock){ dword_and(memoryBlock, srcToDest, wordAndDword, forReg, Dreg, Areg); }
	void and_edx_to_ebx(vect8* memoryBlock){ dword_and(memoryBlock, srcToDest, wordAndDword, forReg, Dreg, Breg); }
	void and_edx_to_ecx(vect8* memoryBlock){ dword_and(memoryBlock, srcToDest, wordAndDword, forReg, Dreg, Creg); }
	//dword or
	void dword_or(vect8* memoryBlock, Direction direction, Bitsize bitsize, Mod mod, X86Regs src, X86Regs dest){
		init(memoryBlock, dwordOrSize);
		addOpcode(0x08, direction, bitsize); //000010
		addModrm(mod, src, dest);
	}
	//000010 0 1 11 000 011
	void or_eax_to_ebx(vect8* memoryBlock){ dword_or(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Breg); }
	void or_eax_to_ecx(vect8* memoryBlock){ dword_or(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Creg); }
	void or_eax_to_edx(vect8* memoryBlock){ dword_or(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Dreg); }
	void or_ebx_to_eax(vect8* memoryBlock){ dword_or(memoryBlock, srcToDest, wordAndDword, forReg, Breg, Areg); }
	void or_ebx_to_ecx(vect8* memoryBlock){ dword_or(memoryBlock, srcToDest, wordAndDword, forReg, Breg, Creg); }
	void or_ebx_to_edx(vect8* memoryBlock){ dword_or(memoryBlock, srcToDest, wordAndDword, forReg, Breg, Dreg); }
	void or_ecx_to_eax(vect8* memoryBlock){ dword_or(memoryBlock, srcToDest, wordAndDword, forReg, Creg, Areg); }
	void or_ecx_to_ebx(vect8* memoryBlock){ dword_or(memoryBlock, srcToDest, wordAndDword, forReg, Creg, Breg); }
	void or_ecx_to_edx(vect8* memoryBlock){ dword_or(memoryBlock, srcToDest, wordAndDword, forReg, Creg, Dreg); }
	void or_edx_to_eax(vect8* memoryBlock){ dword_or(memoryBlock, srcToDest, wordAndDword, forReg, Dreg, Areg); }
	void or_edx_to_ebx(vect8* memoryBlock){ dword_or(memoryBlock, srcToDest, wordAndDword, forReg, Dreg, Breg); }
	void or_edx_to_ecx(vect8* memoryBlock){ dword_or(memoryBlock, srcToDest, wordAndDword, forReg, Dreg, Creg); }
	//dword xor
	void dword_xor(vect8* memoryBlock, Direction direction, Bitsize bitsize, Mod mod, X86Regs src, X86Regs dest){
		init(memoryBlock, dwordXorSize);
		addOpcode(0x30, direction, bitsize); //001100
		addModrm(mod, src, dest);
	}
	//001100 0 1 11 000 011
	void xor_eax_to_ebx(vect8* memoryBlock){ dword_xor(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Breg); }
	void xor_eax_to_ecx(vect8* memoryBlock){ dword_xor(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Creg); }
	void xor_eax_to_edx(vect8* memoryBlock){ dword_xor(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Dreg); }
	void xor_ebx_to_eax(vect8* memoryBlock){ dword_xor(memoryBlock, srcToDest, wordAndDword, forReg, Breg, Areg); }
	void xor_ebx_to_ecx(vect8* memoryBlock){ dword_xor(memoryBlock, srcToDest, wordAndDword, forReg, Breg, Creg); }
	void xor_ebx_to_edx(vect8* memoryBlock){ dword_xor(memoryBlock, srcToDest, wordAndDword, forReg, Breg, Dreg); }
	void xor_ecx_to_eax(vect8* memoryBlock){ dword_xor(memoryBlock, srcToDest, wordAndDword, forReg, Creg, Areg); }
	void xor_ecx_to_ebx(vect8* memoryBlock){ dword_xor(memoryBlock, srcToDest, wordAndDword, forReg, Creg, Breg); }
	void xor_ecx_to_edx(vect8* memoryBlock){ dword_xor(memoryBlock, srcToDest, wordAndDword, forReg, Creg, Dreg); }
	void xor_edx_to_eax(vect8* memoryBlock){ dword_xor(memoryBlock, srcToDest, wordAndDword, forReg, Dreg, Areg); }
	void xor_edx_to_ebx(vect8* memoryBlock){ dword_xor(memoryBlock, srcToDest, wordAndDword, forReg, Dreg, Breg); }
	void xor_edx_to_ecx(vect8* memoryBlock){ dword_xor(memoryBlock, srcToDest, wordAndDword, forReg, Dreg, Creg); }


	//bitwise shl by immediate dword

	void dword_shift(vect8* memoryBlock, Direction direction, Bitsize bitsize, Mod mod, X86Regs src, X86Regs dest){
		init(memoryBlock, dwordShiftSize);
		addOpcode(0xC0, direction, bitsize); //110000
		addModrm(mod, src, dest);
		addByte(disp.byte);
	}
	//110000 0 1 11 100 000
	void shl_imm_eax(vect8* memoryBlock, uint8_t byte){ disp = byte; dword_shift(memoryBlock, srcToDest, wordAndDword, forReg, illegal, Areg); }
	void shl_imm_ebx(vect8* memoryBlock, uint8_t byte){ disp = byte; dword_shift(memoryBlock, srcToDest, wordAndDword, forReg, illegal, Breg); }
	void shl_imm_ecx(vect8* memoryBlock, uint8_t byte){ disp = byte; dword_shift(memoryBlock, srcToDest, wordAndDword, forReg, illegal, Creg); }
	void shl_imm_edx(vect8* memoryBlock, uint8_t byte){ disp = byte; dword_shift(memoryBlock, srcToDest, wordAndDword, forReg, illegal, Dreg); }
	//bitwise shr by immediate dword
	//110000 0 1 11 101 000
	void shr_imm_eax(vect8* memoryBlock, uint8_t byte){ disp = byte; dword_shift(memoryBlock, srcToDest, wordAndDword, forReg, memaddr, Areg); }
	void shr_imm_ebx(vect8* memoryBlock, uint8_t byte){ disp = byte; dword_shift(memoryBlock, srcToDest, wordAndDword, forReg, memaddr, Breg); }
	void shr_imm_ecx(vect8* memoryBlock, uint8_t byte){ disp = byte; dword_shift(memoryBlock, srcToDest, wordAndDword, forReg, memaddr, Creg); }
	void shr_imm_edx(vect8* memoryBlock, uint8_t byte){ disp = byte; dword_shift(memoryBlock, srcToDest, wordAndDword, forReg, memaddr, Dreg); }


	//cmp
	void cmp(vect8* memoryBlock, Direction direction, Bitsize bitsize, Mod mod, X86Regs src, X86Regs dest){
		init(memoryBlock, cmpSize);
		//cmp eax, ebx 001110 0 1 11 011 000
		addOpcode(0x38, direction, bitsize);
		addModrm(mod, src, dest);
	}
	void cmp_eax_to_ebx(vect8* memoryBlock){ cmp(memoryBlock, srcToDest, wordAndDword, forReg, Breg, Areg); }
	void cmp_eax_to_ecx(vect8* memoryBlock){ cmp(memoryBlock, srcToDest, wordAndDword, forReg, Creg, Areg); }
	void cmp_eax_to_edx(vect8* memoryBlock){ cmp(memoryBlock, srcToDest, wordAndDword, forReg, Dreg, Areg); }

	void cmp_ebx_to_eax(vect8* memoryBlock){ cmp(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Breg); }
	void cmp_ebx_to_ecx(vect8* memoryBlock){ cmp(memoryBlock, srcToDest, wordAndDword, forReg, Creg, Breg); }
	void cmp_ebx_to_edx(vect8* memoryBlock){ cmp(memoryBlock, srcToDest, wordAndDword, forReg, Dreg, Breg); }

	void cmp_ecx_to_eax(vect8* memoryBlock){ cmp(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Creg); }
	void cmp_ecx_to_ebx(vect8* memoryBlock){ cmp(memoryBlock, srcToDest, wordAndDword, forReg, Breg, Creg); }
	void cmp_ecx_to_edx(vect8* memoryBlock){ cmp(memoryBlock, srcToDest, wordAndDword, forReg, Dreg, Creg); }

	void cmp_edx_to_eax(vect8* memoryBlock){ cmp(memoryBlock, srcToDest, wordAndDword, forReg, Areg, Dreg); }
	void cmp_edx_to_ebx(vect8* memoryBlock){ cmp(memoryBlock, srcToDest, wordAndDword, forReg, Breg, Dreg); }
	void cmp_edx_to_ecx(vect8* memoryBlock){ cmp(memoryBlock, srcToDest, wordAndDword, forReg, Creg, Dreg); }


	//jmp	jump
	void rel_jmp(vect8* memoryBlock, Movsize getMovsize, Direction direction, Bitsize bitsize){
		if(direction == destToSrc) init(memoryBlock, byteRelJmpSize);
		else if (getMovsize == movWord){ init(memoryBlock, wordRelJmpSize); addPrefix(); }
		else init(memoryBlock, dwordRelJmpSize);

		addOpcode(0xE8, direction, bitsize); //111010 0 0
		if (direction == destToSrc) addByte(disp.byte);
		else if (getMovsize == movWord) addWord(disp.word);
		else addDword(disp.dword);
	}
	//111010 1 1 disp8
	void short_jmp(vect8* memoryBlock, uint8_t byte){ disp = byte; rel_jmp(memoryBlock, movByte, destToSrc, wordAndDword); }
	void near16_jmp(vect8* memoryBlock, uint16_t word){ disp = word; rel_jmp(memoryBlock, movWord, srcToDest, wordAndDword); }
	void near32_jmp(vect8* memoryBlock, uint32_t dword){ disp = dword; rel_jmp(memoryBlock, movDword, srcToDest, wordAndDword); }
	//je	jump equals
	//011101 0 0 11 001 011
	void byte_rel_je(vect8* memoryBlock, Direction direction, Bitsize bitsize){
		init(memoryBlock, byteRelJeSize);
		addOpcode(0x74, direction, bitsize);
		addByte(disp.byte);
	}
	void short_je(vect8* memoryBlock, uint8_t byte){ disp = byte; byte_rel_je(memoryBlock, srcToDest, byteOnly); }
	void short_jne(vect8* memoryBlock, uint8_t byte){ disp = byte; byte_rel_je(memoryBlock, srcToDest, wordAndDword); }
	//jne	jump not equals

	//jb	jump less unsigned
	//jbe	jump less equals unsigned
	void byte_rel_jb(vect8* memoryBlock, Direction direction, Bitsize bitsize){
		init(memoryBlock, byteRelJbSize);
		addOpcode(0x70, direction, bitsize);
		addByte(disp.byte);
	}
	void short_jb(vect8* memoryBlock, uint8_t byte){ disp = byte; byte_rel_jb(memoryBlock, destToSrc, byteOnly); }
	void short_jbe(vect8* memoryBlock, uint8_t byte){ disp = byte; byte_rel_je(memoryBlock, destToSrc, byteOnly); }
	

	//ja	jump greater unsigned
	//jae	jump greater equals unsigned
	void short_ja(vect8* memoryBlock, uint8_t byte){ disp = byte; byte_rel_je(memoryBlock, destToSrc, wordAndDword); }
	void short_jae(vect8* memoryBlock, uint8_t byte){ disp = byte; byte_rel_jb(memoryBlock, destToSrc, wordAndDword); }
	

	//return from eax
	void ret(vect8* memoryBlock){ init(memoryBlock, 1); addByte(0xC3); }

	//load effective address - lea <- shift + add
	//bse -> memaddr: some dword immediate(disp32) you can add along with main(multiplcation)
	//bse -> not memaddr: some reg(any reg) you can add along with main(multiplcation)
	void lea_regToReg(vect8* memoryBlock, Direction direction, Bitsize bitsize, Mod mod, X86Regs src, X86Regs dest){
		if (sib.base == memaddr) init(memoryBlock, leaWithDispSize);
		else init(memoryBlock, leaWithoutDispSize);
		addOpcode(0x8C, direction, bitsize);	//100011 0 1 00 011 100 01 000 000
												//lea    d s md src dst sc idx bse
												//              Brg ill x2 Arg Crg	->	lea ebx, [eax*2 + ecx]
		addModrm(mod, src, dest);
		addSib(sib.scale, sib.index, sib.base);
		if (sib.base == memaddr) addDword(disp.dword);

	}




	/*
	
		shortcuts!
	
	
	*/
	//easy shortcut to load to register and expand
	void loadByteToDwordRegA(vect8* memoryBlock, uint32_t dword){ mov_memoryaddr_to_al(memoryBlock, dword); movzx_al_to_eax(memoryBlock); }
	void loadWordToDwordRegA(vect8* memoryBlock, uint32_t dword){ mov_memoryaddr_to_ax(memoryBlock, dword); movzx_ax_to_eax(memoryBlock); }
	void loadDwordToDwordRegA(vect8* memoryBlock, uint32_t dword){ mov_memoryaddr_to_eax(memoryBlock, dword); }

	void loadByteToDwordRegB(vect8* memoryBlock, uint32_t dword){ mov_memoryaddr_to_bl(memoryBlock, dword); movzx_bl_to_ebx(memoryBlock); }
	void loadWordToDwordRegB(vect8* memoryBlock, uint32_t dword){ mov_memoryaddr_to_bx(memoryBlock, dword); movzx_bx_to_ebx(memoryBlock); }
	void loadDwordToDwordRegB(vect8* memoryBlock, uint32_t dword){ mov_memoryaddr_to_ebx(memoryBlock, dword); }

	void loadByteToDwordRegC(vect8* memoryBlock, uint32_t dword){ mov_memoryaddr_to_cl(memoryBlock, dword); movzx_cl_to_ecx(memoryBlock); }
	void loadWordToDwordRegC(vect8* memoryBlock, uint32_t dword){ mov_memoryaddr_to_cx(memoryBlock, dword); movzx_cx_to_ecx(memoryBlock); }
	void loadDwordToDwordRegC(vect8* memoryBlock, uint32_t dword){ mov_memoryaddr_to_ecx(memoryBlock, dword); }

	void loadByteToDwordRegD(vect8* memoryBlock, uint32_t dword){ mov_memoryaddr_to_dl(memoryBlock, dword); movzx_dl_to_edx(memoryBlock); }
	void loadWordToDwordRegD(vect8* memoryBlock, uint32_t dword){ mov_memoryaddr_to_dx(memoryBlock, dword); movzx_dx_to_edx(memoryBlock); }
	void loadDwordToDwordRegD(vect8* memoryBlock, uint32_t dword){ mov_memoryaddr_to_edx(memoryBlock, dword); }

	//preferred way to load/store array elements to register, ABC regs will get occupied, backup shit before using.
	void loadByteArray_AregAsResult(vect8* memoryBlock, uint32_t array, uint32_t arrayptr){
		mov_imm_to_eax(memoryBlock, array);	//A for array
		loadByteToDwordRegB(memoryBlock, arrayptr);	//B for arrayptr
		
		//shl_imm_ebx(memoryBlock, 1);	//byte
		add_eax_to_ebx(memoryBlock); //B as target element addr
		
		mov_ebxaddr_to_al(memoryBlock);	//to Areg
		movzx_al_to_eax(memoryBlock);	//expand
	}
	//preferred way to load/store array elements to register, ABC regs will get occupied, backup shit before using.
	void loadWordArray_AregAsResult(vect8* memoryBlock, uint32_t array, uint32_t arrayptr){
		mov_imm_to_eax(memoryBlock, array);	//A for array
		loadByteToDwordRegB(memoryBlock, arrayptr);	//B for arrayptr

		sib.scale = x2;
		sib.index = Breg;
		sib.base = Areg;
		lea_regToReg(memoryBlock, srcToDest, wordAndDword, forDisp, Breg, illegal);
		//shl_imm_ebx(memoryBlock, 1);	//word
		//add_eax_to_ebx(memoryBlock); //B as target element addr

		mov_ebxaddr_to_ax(memoryBlock);	//to Areg
		movzx_ax_to_eax(memoryBlock);	//expand
	}
	//preferred way to load/store array elements to register, ABC regs will get occupied, backup shit before using.
	void loadDwordArray_AregAsResult(vect8* memoryBlock, uint32_t array, uint32_t arrayptr){
		mov_imm_to_eax(memoryBlock, array);	//A for array
		loadByteToDwordRegB(memoryBlock, arrayptr);	//B for arrayptr

		sib.scale = x4;
		sib.index = Breg;
		sib.base = Areg;
		lea_regToReg(memoryBlock, srcToDest, wordAndDword, forDisp, Breg, illegal);
		//shl_imm_ebx(memoryBlock, 2);	//dword
		//add_eax_to_ebx(memoryBlock); //B as target element addr

		mov_ebxaddr_to_eax(memoryBlock);	//to Areg
		//movzx_al_to_eax(memoryBlock);		//expand
	}
	//preferred way to load/store array elements to register, ABC regs will get occupied, backup shit before using.
	void storeByteArray_AregAsInput(vect8* memoryBlock, uint32_t array, uint32_t arrayptr){
		mov_imm_to_ebx(memoryBlock, array);	//B for array
		loadByteToDwordRegC(memoryBlock, arrayptr); //C for arrayptr

		//shl_imm_ecx(memoryBlock, 1); //byte
		add_ebx_to_ecx(memoryBlock); //C as target element addr

		mov_al_to_ecxaddr(memoryBlock);		//input to array
	}
	//preferred way to load/store array elements to register, ABC regs will get occupied, backup shit before using.
	void storeWordArray_AregAsInput(vect8* memoryBlock, uint32_t array, uint32_t arrayptr){
		mov_imm_to_ebx(memoryBlock, array);	//B for array
		loadByteToDwordRegC(memoryBlock, arrayptr); //C for arrayptr

		sib.scale = x2;
		sib.index = Creg;
		sib.base = Breg;
		lea_regToReg(memoryBlock, srcToDest, wordAndDword, forDisp, Creg, illegal);
		//shl_imm_ecx(memoryBlock, 1); //word
		//add_ebx_to_ecx(memoryBlock); //C as target element addr

		mov_ax_to_ecxaddr(memoryBlock);		//input to array
	}
	//preferred way to load/store array elements to register, ABC regs will get occupied, backup shit before using.
	void storeDwordArray_AregAsInput(vect8* memoryBlock, uint32_t array, uint32_t arrayptr){
		mov_imm_to_ebx(memoryBlock, array);	//B for array
		loadByteToDwordRegC(memoryBlock, arrayptr); //C for arrayptr

		sib.scale = x4;
		sib.index = Creg;
		sib.base = Breg;
		lea_regToReg(memoryBlock, srcToDest, wordAndDword, forDisp, Creg, illegal);
		//shl_imm_ecx(memoryBlock, 2); //dword
		//add_ebx_to_ecx(memoryBlock); //C as target element addr

		mov_eax_to_ecxaddr(memoryBlock);		//input to array
	}

	//shortcut to change one piece of memory variable without mumbojumbo, Areg is used.
	void addByteToMemaddr(vect8* memoryBlock, uint32_t memvar, uint32_t immval){
		loadByteToDwordRegA(memoryBlock, memvar);
		add_imm_to_eax(memoryBlock, immval);
		mov_al_to_memoryaddr(memoryBlock, memvar);
	}
	//shortcut to change one piece of memory variable without mumbojumbo, Areg is used.
	void addWordToMemaddr(vect8* memoryBlock, uint32_t memvar, uint32_t immval){
		loadWordToDwordRegA(memoryBlock, memvar);
		add_imm_to_eax(memoryBlock, immval);
		mov_ax_to_memoryaddr(memoryBlock, memvar);
	}
	//shortcut to change one piece of memory variable without mumbojumbo, Areg is used.
	void addDwordToMemaddr(vect8* memoryBlock, uint32_t memvar, uint32_t immval){
		loadDwordToDwordRegA(memoryBlock, memvar);
		add_imm_to_eax(memoryBlock, immval);
		mov_eax_to_memoryaddr(memoryBlock, memvar);
	}
};

