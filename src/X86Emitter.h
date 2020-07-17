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
#include<cstdint>
#include<string>
#include<cstring>

#pragma warning(disable: 4018)
using vect8 = std::vector<uint8_t>; //tryinig really hard to shorten code here;-;

class X86Emitter{
private:

	mutable vect8* memoryBlock;
	
	//host endianness sensitive
	using ByteRegs = union{
		struct{
			uint8_t byte0;
			uint8_t byte1;
			uint8_t byte2;
			uint8_t byte3;
		};
		uint8_t byte;
		uint16_t word;
		uint32_t dword;
	};
	mutable ByteRegs byteRegs;

	void addByte(uint8_t byte) const{
		byteRegs.byte = byte;
		memoryBlock->push_back(byteRegs.byte0);
	}
	void addWord(uint16_t word) const{
		byteRegs.word = word;
		memoryBlock->push_back(byteRegs.byte0);
		memoryBlock->push_back(byteRegs.byte1);
	}
	void addDword(uint32_t dword) const{
		byteRegs.dword = dword;
		memoryBlock->push_back(byteRegs.byte0);
		memoryBlock->push_back(byteRegs.byte1);
		memoryBlock->push_back(byteRegs.byte2);
		memoryBlock->push_back(byteRegs.byte3);
	}

	//get memoryBlock from outside
	//memoryBlock optimization
	void init(vect8* inputMemoryBlock, int index = 0) const{
		memoryBlock = inputMemoryBlock;
		memoryBlock->reserve(memoryBlock->capacity() + index);
		byteRegs.dword = 0;
	}


public:

	/*everything is byte sized except addWord and addDword*/

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
	using Disp = struct _Disp{
		union{
			uint8_t byte;
			uint16_t word;
			uint32_t dword = 0;
		};
		_Disp(){ this->dword = 0; }	//give this a constructor for '= Disp()'

		void operator=(uint32_t dword){ this->dword = dword; }
		void operator=(uint16_t word){ this->word = word; }
		void operator=(uint8_t byte){ this->byte = byte; }

	};

	Disp insertDisp(int temp) const{ Disp disp; disp = (uint32_t)temp; return disp; }
	Disp insertAddr(int temp) const{ return insertDisp(temp); }


	//sib
	using Scale = enum{ x1 = 0x0, x2 = 0x1, x4 = 0x2, x8 = 0x3 };
	using Index = X86Regs;
	using Base = X86Regs;
	using Sib = struct{
		Scale scale;
		Index index;
		Base base;
		void operator=(Scale scale){ this->scale = scale; }
	};

	//byte
	void addExtension() const{ addByte(0x0F); }
	//byte
	void addPrefix() const{ addByte(0x66); }
	//byte
	void addOpcode(uint8_t opcode, Direction direction, Bitsize bitsize) const{
		uint8_t d = 0x0;
		uint8_t s = 0x0;
		if (direction == 1) d = 0x2;
		if (bitsize == 1) s = 0x1;
		opcode |= d;
		opcode |= s;
		addByte(opcode);
	}

	template<typename Dbit, typename Tbit, typename Tbit_2>
	struct _MiddleGround{
		Dbit dbit;
		Tbit tbit;
		Tbit_2 tbit_2;
	};

	template<typename Dbit, typename Tbit, typename Tbit_2>
	void addMiddleGround(_MiddleGround<Dbit, Tbit, Tbit_2> mground) const{
		uint8_t opcode = 0x0;
		opcode |= ((uint8_t)mground.dbit << 6);
		opcode |= ((uint8_t)mground.tbit << 3);
		opcode |= (uint8_t)mground.tbit_2;
		addByte(opcode);
	}

	struct ModrmGround{
		using MiddleGround = _MiddleGround<Mod, X86Regs, X86Regs>;
	};

	struct SibGround{
		using MiddleGround = _MiddleGround<Scale, Index, Base>;
	};
	//using ModrmGround = struct MiddleGround<Mod, X86Regs, X86Regs>;
	//using SibGround = struct MiddleGround<Scale, Index, Base>;

	//byte
	void addModrm(ModrmGround::MiddleGround modrmGround) const{ addMiddleGround(modrmGround); }
	//byte
	void addSib(SibGround::MiddleGround sibGround) const{ addMiddleGround(sibGround); }

	//byte
	void addModrm(Mod mod, X86Regs src, X86Regs dest) const{ addModrm(ModrmGround::MiddleGround{ mod, src, dest }); }
	//byte
	void addSib(Scale scale, Index index, Base base) const{ addSib(SibGround::MiddleGround{ scale, index, base }); }

	//use this template for jmp instructions
	using OperandSizes = enum{
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
		dwordMovImmToCregSize = 5,
		dwordMovImmToDregSize = 5,
		dwordAddSize = 2,
		dwordAddImmToRegSize = 6,
		byteAddImmToMemaddrSize = 7,
		wordAddImmToMemaddrSize = 9,
		dwordAddImmToMemaddrSize = 10,

		dwordAddRegToMemaddrSize = 6,
		wordAddRegToMemaddrSize = 7,
		byteAddRegToMemaddrSize = 6,

		byteAddImmToRegaddrSize = 3,
		wordAddImmToRegaddrSize = 5,
		dwordAddImmToRegaddrSize = 6,

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

	using OperandModes = enum{
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
		dwordMovImmToCregMode,
		dwordMovImmToDregMode,
		dwordAddMode,
		dwordAddImmToRegMode,
		byteAddImmToMemaddrMode,
		wordAddImmToMemaddrMode,
		dwordAddImmToMemaddrMode,

		dwordAddRegToMemaddrMode,
		wordAddRegToMemaddrMode,
		byteAddRegToMemaddrMode,

		byteAddImmToRegaddrMode,
		wordAddImmToRegaddrMode,
		dwordAddImmToRegaddrMode,

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

	OperandSizes opmodeError(const char* str, std::string str2 = std::string()) const{ fprintf(stderr, "%s", str); fprintf(stderr, ": incompatible opmode! -> "); fprintf(stderr, "%s", str2.c_str()); exit(1); return none; }

	//no need for the opposite(use only for zeroing out high area)
	OperandSizes Movzx(vect8* memoryBlock, OperandModes opmode, X86Regs src, X86Regs dest) const{
		uint8_t opcode = 0xB4; //10110100
		switch (opmode){
		case movzxByteToDwordMode: init(memoryBlock, movzxByteToDwordSize); addExtension(); addOpcode(opcode, destToSrc, byteOnly); addModrm(forReg, dest, src); return movzxByteToDwordSize;
		case movzxWordToDwordMode: init(memoryBlock, movzxWordToDwordSize); addExtension(); addOpcode(opcode, destToSrc, wordAndDword); addModrm(forReg, dest, src); return movzxWordToDwordSize;
		default: opmodeError("movzx");
		}
		
		return none;
	}
	//ex) convert byte to dword
	//Movzx(memoryBlock, movzxByteToDwordMode, Areg, Areg);
	
	//ex) convert word to dword
	//Movzx(memoryBlock, movzxWordToDwordMode, Areg, Areg);
	

	//memoryaddr must always be dword!
	OperandSizes Mov(vect8* memoryBlock, OperandModes opmode, X86Regs src, X86Regs dest = memaddr, Disp disp = Disp()) const{
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
	//for memaddr
	OperandSizes Mov(vect8* memoryBlock, OperandModes opmode, X86Regs src, Disp disp = Disp()) const{ return Mov(memoryBlock, opmode, src, Areg, disp); }



	//kinda different - use Direction and Bitsize to point reg
	OperandSizes Mov_imm(vect8* memoryBlock, OperandModes opmode, Disp disp) const{
		uint8_t opcode = 0xB8;
		switch (opmode){
		case dwordMovImmToAregMode: init(memoryBlock, dwordMovImmToAregSize); addOpcode(opcode, srcToDest, byteOnly); addDword(disp.dword); return dwordMovImmToAregSize;
		case dwordMovImmToCregMode: init(memoryBlock, dwordMovImmToCregSize); addOpcode(opcode, srcToDest, wordAndDword); addDword(disp.dword); return dwordMovImmToCregSize;
		case dwordMovImmToDregMode: init(memoryBlock, dwordMovImmToDregSize); addOpcode(opcode, destToSrc, byteOnly); addDword(disp.dword); return dwordMovImmToDregSize;

		default: opmodeError("mov_imm");
		}
		return none;
		
	}

	//dword add
	OperandSizes Add(vect8* memoryBlock, OperandModes opmode, X86Regs src, X86Regs dest, Disp addr = Disp()) const{
		uint8_t opcode = 0x00;
		switch (opmode){
		case dwordAddMode: init(memoryBlock, dwordAddSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forReg, src, dest); return dwordAddSize;
		
			//000000 0 1 00 010 101 addr = add dword ptr [extra], edx
			//word  = dword + prefix
			//000000 0 0 00 010 101 addr = add byte ptr [extra], edx
			//add    d s md arg drg
		case dwordAddRegToMemaddrMode: init(memoryBlock, dwordAddRegToMemaddrSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forDisp, src, memaddr); addDword(addr.dword); return dwordAddRegToMemaddrSize;
		case wordAddRegToMemaddrMode: init(memoryBlock, wordAddRegToMemaddrSize); addPrefix(); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forDisp, src, memaddr); addDword(addr.dword); return wordAddRegToMemaddrSize;
		case byteAddRegToMemaddrMode: init(memoryBlock, byteAddRegToMemaddrSize); addOpcode(opcode, srcToDest, byteOnly); addModrm(forDisp, src, memaddr); addDword(addr.dword); return byteAddRegToMemaddrSize;
			
		default: opmodeError("add");
		}
		return none;
	}

	//memaddr ops shortcut
	OperandSizes Add(vect8* memoryBlock, OperandModes opmode, X86Regs src, Disp addr = Disp()) const{ return Add(memoryBlock, opmode, src, memaddr, addr); }

	//dword imm add
	OperandSizes Add_imm(vect8* memoryBlock, OperandModes opmode, Disp addr, Disp disp = Disp(), X86Regs dest = memaddr) const{
		uint8_t opcode = 0x80;
		switch (opmode){
		case dwordAddImmToRegMode: init(memoryBlock, dwordAddImmToRegSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forReg, Areg, dest); addDword(addr.dword); return dwordAddImmToRegSize;
	
		case byteAddImmToMemaddrMode: init(memoryBlock, byteAddImmToMemaddrSize); addOpcode(opcode, srcToDest, byteOnly); addModrm(forDisp, Areg, memaddr); addDword(addr.dword); addByte(disp.byte); return byteAddImmToMemaddrSize;
		case wordAddImmToMemaddrMode: init(memoryBlock, wordAddImmToMemaddrSize); addPrefix(); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forDisp, Areg, memaddr); addDword(addr.dword); addWord(disp.word); return wordAddImmToMemaddrSize;
		case dwordAddImmToMemaddrMode: init(memoryBlock, dwordAddImmToMemaddrSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forDisp, Areg, memaddr); addDword(addr.dword); addDword(disp.dword); return dwordAddImmToMemaddrSize;
		
			//100000 0 0 00 000 010
			//add    d s md arg drg
		case byteAddImmToRegaddrMode: init(memoryBlock, byteAddImmToRegaddrSize); addOpcode(opcode, srcToDest, byteOnly); addModrm(forDisp, Areg, dest); addByte(addr.byte); return byteAddImmToRegaddrSize;
		case wordAddImmToRegaddrMode: init(memoryBlock, wordAddImmToRegaddrSize); addPrefix(); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forDisp, Areg, dest); addWord(addr.word); return wordAddImmToRegaddrSize;
		case dwordAddImmToRegaddrMode: init(memoryBlock, dwordAddImmToRegaddrSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forDisp, Areg, dest); addDword(addr.dword); return dwordAddImmToRegaddrSize;

		
		default: opmodeError("add_imm");
		}
		return none;
	}
	//override
	OperandSizes Add_imm(vect8* memoryBlock, OperandModes opmode, Disp disp, X86Regs dest = memaddr) const{ return Add_imm(memoryBlock, opmode, disp, insertDisp(0x0), dest); }

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
	OperandSizes Sub(vect8* memoryBlock, OperandModes opmode, X86Regs src, X86Regs dest) const{
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
	OperandSizes And(vect8* memoryBlock, OperandModes opmode, X86Regs src, X86Regs dest) const{
		uint8_t opcode = 0x20; //001000
		switch (opmode){
		case dwordAndMode: init(memoryBlock, dwordAndSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forReg, src, dest); return dwordAndSize;
		default: opmodeError("and");
		}
		return none;
		
	}

	//dword or
	OperandSizes Or(vect8* memoryBlock, OperandModes opmode, X86Regs src, X86Regs dest) const{
		uint8_t opcode = 0x08; //000010
		switch (opmode){
		case dwordOrMode: init(memoryBlock, dwordOrSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forReg, src, dest); return dwordOrSize;
		default: opmodeError("or");
		}
		return none;

	}

	//dword xor
	OperandSizes Xor(vect8* memoryBlock, OperandModes opmode, X86Regs src, X86Regs dest) const{
		uint8_t opcode = 0x30; //001100
		switch (opmode){
		case dwordXorMode: init(memoryBlock, dwordXorSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forReg, src, dest); return dwordXorSize;
		default: opmodeError("xor");
		}
		return none;

	}

	//bitwise shl by immediate dword

	OperandSizes Shift(vect8* memoryBlock, OperandModes opmode, Disp disp, X86Regs dest) const{
		uint8_t opcode = 0xC0;	//110000
		switch (opmode){
		case dwordShiftLeftMode: init(memoryBlock, dwordShiftLeftSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forReg, illegal, dest); addByte(disp.byte); return dwordShiftLeftSize;
		case dwordShiftRightMode: init(memoryBlock, dwordShiftRightSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forReg, memaddr, dest); addByte(disp.byte); return dwordShiftRightSize;
		default: opmodeError("shift");
		}
		return none;
		
	}

	//cmp
	OperandSizes Cmp(vect8* memoryBlock, OperandModes opmode, X86Regs src, X86Regs dest) const{
		uint8_t opcode = 0x38; //001110
		switch (opmode){
		case cmpMode: init(memoryBlock, cmpSize); addOpcode(opcode, srcToDest, wordAndDword); addModrm(forReg, dest, src); return cmpSize;
		default: opmodeError("cmp");
		}
		return none;
	}


	//jmp	jump
	OperandSizes Jmp(vect8* memoryBlock, OperandModes opmode, Disp disp) const{
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
	OperandSizes Jcc(vect8* memoryBlock, OperandModes opmode, Disp disp) const{
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
	OperandSizes Jcc2(vect8* memoryBlock, OperandModes opmode, Disp disp) const{
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
	OperandSizes Ret(vect8* memoryBlock) const{ init(memoryBlock, retSize); addByte(0xC3); return retSize; }

	//nop
	OperandSizes Nop(vect8* memoryBlock) const{ init(memoryBlock, nopSize); addByte(0x90); return nopSize; }

	//load effective address - lea <- shift + add
	//bse -> memaddr: some dword immediate(disp32) you can add along with main(multiplcation)
	//bse -> not memaddr: some reg(any reg) you can add along with main(multiplcation)
	OperandSizes Lea(vect8* memoryBlock, OperandModes opmode, X86Regs src, Scale scale, X86Regs idx, X86Regs bse, Disp disp = Disp()) const{
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

		A, C, D registers are caller-saved (volatile)
		B, SI, DI are callee-saved.

		*/
	//easy shortcut to load to register and expand
	using ExpandSizes = enum{ Byte, Word, Dword };

	OperandSizes loadMemToDwordReg(vect8* memoryBlock, uint32_t addr, X86Regs Xreg, ExpandSizes Size) const{
		switch (Size){
		case Byte: Mov(memoryBlock, movFromMemaddrByteMode, Xreg, insertDisp(addr)); Movzx(memoryBlock, movzxByteToDwordMode, Xreg, Xreg); return loadByteShortcutSize;
		case Word: Mov(memoryBlock, movFromMemaddrWordMode, Xreg, insertDisp(addr)); Movzx(memoryBlock, movzxWordToDwordMode, Xreg, Xreg); return loadWordShortcutSize;
		case Dword: Mov(memoryBlock, movFromMemaddrDwordMode, Xreg, insertDisp(addr)); return loadDwordShortcutSize;
		}
		return none;
	}

	OperandSizes loadArray_AregAsResult(vect8* memoryBlock, uint32_t arr, uint32_t arrptr, ExpandSizes Size) const{
		Mov_imm(memoryBlock, dwordMovImmToAregMode, insertDisp(arr));
		loadMemToDwordReg(memoryBlock, arrptr, Creg, Byte);
		switch (Size){
		case Byte: Add(memoryBlock, dwordAddMode, Areg, Creg); Mov(memoryBlock, movByteMemToRegMode, Creg, Areg); Movzx(memoryBlock, movzxByteToDwordMode, Areg, Areg); return loadByteArraySize;
		case Word: Lea(memoryBlock, leaWithoutDispMode, Creg, x2, Creg, Areg); Mov(memoryBlock, movWordMemToRegMode, Creg, Areg); Movzx(memoryBlock, movzxWordToDwordMode, Areg, Areg); return loadWordArraySize;
		case Dword: Lea(memoryBlock, leaWithoutDispMode, Creg, x4, Creg, Areg); Mov(memoryBlock, movDwordMemToRegMode, Creg, Areg); return loadDwordArraySize;
		}
		return none;
	}

	OperandSizes storeArray_AregAsInput(vect8* memoryBlock, uint32_t arr, uint32_t arrptr, ExpandSizes Size) const{
		Mov_imm(memoryBlock, dwordMovImmToDregMode, insertDisp(arr));
		loadMemToDwordReg(memoryBlock, arrptr, Creg, Byte);
		switch (Size){
		case Byte: Add(memoryBlock, dwordAddMode, Dreg, Creg); Mov(memoryBlock, movByteRegToMemMode, Areg, Creg); return storeByteArraySize;
		case Word: Lea(memoryBlock, leaWithoutDispMode, Creg, x2, Creg, Dreg); Mov(memoryBlock, movWordRegToMemMode, Areg, Creg); return storeWordArraySize;
		case Dword: Lea(memoryBlock, leaWithoutDispMode, Creg, x4, Creg, Dreg); Mov(memoryBlock, movDwordRegToMemMode, Areg, Creg); return storeDwordArraySize;
		}
		return none;
	}

	uint32_t addToMemaddr(vect8* memoryBlock, uint32_t memvar, uint32_t immval, ExpandSizes Size) const{
		switch (Size){
		case Byte: loadMemToDwordReg(memoryBlock, memvar, Areg, Byte); break;
		case Word: loadMemToDwordReg(memoryBlock, memvar, Areg, Word); break;
		case Dword: loadMemToDwordReg(memoryBlock, memvar, Areg, Dword); break;
		}
		
		Add_imm(memoryBlock, dwordAddImmToRegMode, insertDisp(immval), Areg);
		switch (Size){
		case Byte: Mov(memoryBlock, movToMemaddrByteMode, Areg, insertDisp(memvar)); return addByteToMemaddrSize;
		case Word: Mov(memoryBlock, movToMemaddrWordMode, Areg, insertDisp(memvar)); return addWordToMemaddrSize;
		case Dword: Mov(memoryBlock, movToMemaddrDwordMode, Areg, insertDisp(memvar)); return addDwordToMemaddrSize;
		}
		return none;
	}

	OperandSizes setToMemaddr(vect8* memoryBlock, uint32_t memvar, uint32_t immval, ExpandSizes Size) const{
		Mov_imm(memoryBlock, dwordMovImmToAregMode, insertDisp(immval));
		switch (Size){
		case Byte: Mov(memoryBlock, movToMemaddrByteMode, Areg, insertDisp(memvar)); return setByteToMemaddrSize;
		case Word: Mov(memoryBlock, movToMemaddrWordMode, Areg, insertDisp(memvar)); return setWordToMemaddrSize;
		case Dword: Mov(memoryBlock, movToMemaddrDwordMode, Areg, insertDisp(memvar)); return setDwordToMemaddrSize;
		}
		return none;
	}


	/*
	
		text assembler(please dont write bs in it error checking is hard af)
	
		TODO: implement
	*/




#define regNum 12
#define regData { "al", "bl", "cl", "dl", "ax", "bx", "cx", "dx", "eax", "ebx", "ecx", "edx" };
	using string = std::string;

	mutable string* regNames;
	string convertLowercase(string* str) const{
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
		Disp addr;
		Disp disp;
	};

	bool isByte(string* str) const{ if ((str->find("byte") != string::npos) || (str->find("l") != string::npos)) return true; else return false; }
	bool isWord(string* str) const{ if ((str->find("word") != string::npos) || ((str->find("x") != string::npos) && (str->find("e") == string::npos))) return true; else return false; }
	bool isDword(string* str) const{ if ((str->find("dword") != string::npos) || ((str->find("x") != string::npos) && (str->find("e") != string::npos))) return true; else return false; }
	bool isPtr(string* str) const{ if ((str->find("ptr") != string::npos) || (str->find("[") != string::npos)) return true; else return false; }
	bool isImm(string* str) const{
		if (str->find("extra") != string::npos) return true;
		
		for (int i = regNum - 1; i > -1; i--){
			if (str->find(regNames[i]) != string::npos) return false;
		}
		return true;
	}
	bool isReg(string* str) const{ return !isImm(str); }
	bool isMem(string* str) const{ return (isImm(str) && isPtr(str)); }
	bool autoInsertExtra(Disp* disp, string* str, Disp extra) const{
		if (str->find("extra") != string::npos){
			disp->dword = extra.dword;
			return true;
		}
		return false;
	}

	void insertSrc(ParserType* parserType, string* src_str) const{
		int check = -1;
		for (int i = 0; i < regNum; i++){ if (src_str->find(regNames[i]) != string::npos) check = i; }
		if (check == -1) return;
		switch (check % 4){
		case 0: parserType->modrm.src = Areg; return;
		case 1: parserType->modrm.src = Breg; return;
		case 2: parserType->modrm.src = Creg; return;
		case 3: parserType->modrm.src = Dreg; return;
		
		}
	}
	void insertDest(ParserType* parserType, string* dest_str) const{
		int check = -1;
		for (int i = 0; i < regNum; i++){ if (dest_str->find(regNames[i]) != string::npos) check = i; }
		if (check == -1) return;
		switch (check % 4){
		case 0: parserType->modrm.dest = Areg; return;
		case 1: parserType->modrm.dest = Breg; return;
		case 2: parserType->modrm.dest = Creg; return;
		case 3: parserType->modrm.dest = Dreg; return;

		}
	}
	void insertImm(Disp* disp, string* imm_str) const{
		//hex
		if (imm_str->find("0x") != string::npos) disp->dword = (uint32_t)std::stoi(*imm_str, 0, 16);
		//dec
		else disp->dword = (uint32_t)std::stoi(*imm_str, 0, 10);
	}

	void parse_op(ParserType* parserType, string* op_str, string* src_str, string* dest_str, Disp extra) const{
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
			if (autoInsertExtra(&parserType->addr, src_str, extra) && isPtr(src_str)){	//from Memaddr
				insertDest(parserType, dest_str);
				if (isReg(dest_str) && !isPtr(dest_str)){
					if (isByte(dest_str)) parserType->opmode = movFromMemaddrByteMode;
					else if (isWord(dest_str)) parserType->opmode = movFromMemaddrWordMode;
					else if (isDword(dest_str)) parserType->opmode = movFromMemaddrDwordMode;
				}
			}
			else if (autoInsertExtra(&parserType->addr, dest_str, extra) && isPtr(dest_str)){	//to Memaddr
				insertSrc(parserType, src_str);
				if (isReg(src_str) && !isPtr(src_str)){
					if (isByte(src_str)) parserType->opmode = movToMemaddrByteMode;
					else if (isWord(src_str)) parserType->opmode = movToMemaddrWordMode;
					else if (isDword(src_str)) parserType->opmode = movToMemaddrDwordMode;
				}
			}
			else if (isReg(src_str) && isReg(dest_str)){
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
			else if (isDword(dest_str) && isReg(dest_str) && !isPtr(src_str)){
				if (autoInsertExtra(&parserType->disp, src_str, extra)){}
				else if (isImm(src_str))insertImm(&parserType->disp, src_str);
				insertDest(parserType, dest_str);
				if (parserType->modrm.dest == Areg) parserType->opmode = dwordMovImmToAregMode;
				else if (parserType->modrm.dest == Creg) parserType->opmode = dwordMovImmToCregMode;
				else if (parserType->modrm.dest == Dreg) parserType->opmode = dwordMovImmToDregMode;

				
			}
		}
		else if(!op_str->compare("add")){
			if (autoInsertExtra(&parserType->addr, dest_str, extra) && !isReg(dest_str)){
				if (isImm(src_str) && !isPtr(src_str)){
					insertImm(&parserType->disp, src_str);
					if (isByte(dest_str)) parserType->opmode = byteAddImmToMemaddrMode;
					else if (isWord(dest_str)) parserType->opmode = wordAddImmToMemaddrMode;
					else if (isDword(dest_str)) parserType->opmode = dwordAddImmToMemaddrMode;
				}
				else if (isReg(src_str)){
					insertSrc(parserType, src_str);
					if (isByte(dest_str)) parserType->opmode = byteAddRegToMemaddrMode;
					else if (isWord(dest_str)) parserType->opmode = wordAddRegToMemaddrMode;
					else if (isDword(dest_str)) parserType->opmode = dwordAddRegToMemaddrMode;
				}
			
			}
			else if (autoInsertExtra(&parserType->disp, src_str, extra) && isReg(dest_str)){
				insertDest(parserType, dest_str);
				if (isByte(dest_str)) parserType->opmode = byteAddImmToRegaddrMode;
				else if (isWord(dest_str)) parserType->opmode = wordAddImmToRegaddrMode;
				else if (isDword(dest_str)) parserType->opmode = dwordAddImmToRegaddrMode;

			}
			else if (isReg(src_str) && !isPtr(src_str) && isReg(dest_str) && !isPtr(dest_str)){
				insertSrc(parserType, src_str); insertDest(parserType, dest_str);
				if (isDword(src_str) && isDword(dest_str)) parserType->opmode = dwordAddMode;

				
			}
			else if (isImm(src_str) && !isPtr(src_str) && isReg(dest_str) && !isPtr(dest_str)){
				insertImm(&parserType->disp, src_str); insertDest(parserType, dest_str);
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
				insertImm(&parserType->disp, src_str); insertDest(parserType, dest_str);
				if (isDword(dest_str)) parserType->opmode = dwordShiftLeftMode;

				
			}
		}
		else if (!op_str->compare("shr")){ 
			if (isImm(src_str) && !isPtr(src_str) && isReg(dest_str) && !isPtr(dest_str)){
				insertImm(&parserType->disp, src_str); insertDest(parserType, dest_str);
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
				if (autoInsertExtra(&parserType->disp, dest_str, extra)) parserType->opmode = byteRelJmpMode;
				else insertImm(&parserType->disp, dest_str); //if its not extra then insert imm
				if (isByte(dest_str)) parserType->opmode = byteRelJmpMode;
				else if (isWord(dest_str)) parserType->opmode = wordRelJmpMode;
				else if (isDword(dest_str)) parserType->opmode = dwordRelJmpMode;

				
			}
		}
		else if(!op_str->compare("je")){ 
			if (isImm(dest_str) && !isPtr(dest_str)){
				if (autoInsertExtra(&parserType->disp, dest_str, extra)) parserType->opmode = byteRelJeMode;
				else insertImm(&parserType->disp, dest_str); //if its not extra then insert imm
				if (isByte(dest_str)) parserType->opmode = byteRelJeMode;

				
			}
		}
		else if (!op_str->compare("jne")){ 
			if (isImm(dest_str) && !isPtr(dest_str)){
				if (autoInsertExtra(&parserType->disp, dest_str, extra)) parserType->opmode = byteRelJneMode;
				else insertImm(&parserType->disp, dest_str); //if its not extra then insert imm
				if (isByte(dest_str)) parserType->opmode = byteRelJneMode;

				
			}
		}
		else if (!op_str->compare("ja")){ 
			if (isImm(dest_str) && !isPtr(dest_str)){
				if (autoInsertExtra(&parserType->disp, dest_str, extra)) parserType->opmode = byteRelJaMode;
				else insertImm(&parserType->disp, dest_str); //if its not extra then insert imm
				if (isByte(dest_str)) parserType->opmode = byteRelJaMode;

				
			}
		}
		else if (!op_str->compare("jb")){ 
			if (isImm(dest_str) && !isPtr(dest_str)){
				if (autoInsertExtra(&parserType->disp, dest_str, extra)) parserType->opmode = byteRelJbMode;
				else insertImm(&parserType->disp, dest_str); //if its not extra then insert imm
				if (isByte(dest_str)) parserType->opmode = byteRelJbMode;

				
			}
		}
		else if (!op_str->compare("jae")){ 
			if (isImm(dest_str) && !isPtr(dest_str)){
				if (autoInsertExtra(&parserType->disp, dest_str, extra)) parserType->opmode = byteRelJaeMode;
				else insertImm(&parserType->disp, dest_str); //if its not extra then insert imm
				if (isByte(dest_str)) parserType->opmode = byteRelJaeMode;

				
			}
		}
		else if (!op_str->compare("jbe")){ 
			if (isImm(dest_str) && !isPtr(dest_str)){
				if (autoInsertExtra(&parserType->disp, dest_str, extra)) parserType->opmode = byteRelJbeMode;
				else insertImm(&parserType->disp, dest_str); //if its not extra then insert imm
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

	


	OperandSizes run_op(vect8* memoryBlock, ParserType* parserType, const char* str) const{
		//all must return OperandSizes
		switch (parserType->opmode){
		case movzxByteToDwordMode: 
		case movzxWordToDwordMode: return Movzx(memoryBlock, parserType->opmode, parserType->modrm.src, parserType->modrm.dest);
		case movToMemaddrByteMode:
		case movToMemaddrDwordMode: 
		case movToMemaddrWordMode: 
		case movFromMemaddrByteMode: 
		case movFromMemaddrDwordMode: 
		case movFromMemaddrWordMode: return Mov(memoryBlock, parserType->opmode, parserType->modrm.src, parserType->modrm.dest, parserType->addr);
		case movDwordRegToRegMode: 
		case movByteRegToMemMode: 
		case movDwordRegToMemMode: 
		case movWordRegToMemMode: 
		case movByteMemToRegMode: 
		case movDwordMemToRegMode: 
		case movWordMemToRegMode: return Mov(memoryBlock, parserType->opmode, parserType->modrm.src, parserType->modrm.dest);
		case dwordMovImmToAregMode: 
		case dwordMovImmToCregMode: 
		case dwordMovImmToDregMode: return Mov_imm(memoryBlock, parserType->opmode, parserType->disp);
		case dwordAddMode: return Add(memoryBlock, parserType->opmode, parserType->modrm.src, parserType->modrm.dest);
		case dwordAddRegToMemaddrMode:
		case wordAddRegToMemaddrMode:
		case byteAddRegToMemaddrMode: return Add(memoryBlock, parserType->opmode, parserType->modrm.src, parserType->addr);
		case dwordAddImmToRegMode: return Add_imm(memoryBlock, parserType->opmode, parserType->disp, parserType->modrm.dest);
		case byteAddImmToMemaddrMode: 
		case wordAddImmToMemaddrMode: 
		case dwordAddImmToMemaddrMode:  return Add_imm(memoryBlock, parserType->opmode, parserType->addr, parserType->disp);
		case byteAddImmToRegaddrMode:
		case wordAddImmToRegaddrMode:
		case dwordAddImmToRegaddrMode:	return Add_imm(memoryBlock, parserType->opmode, parserType->disp, parserType->modrm.dest);
		case dwordSubMode: return Sub(memoryBlock, parserType->opmode, parserType->modrm.src, parserType->modrm.dest);
		case dwordAndMode: return And(memoryBlock, parserType->opmode, parserType->modrm.src, parserType->modrm.dest);
		case dwordOrMode: return Or(memoryBlock, parserType->opmode, parserType->modrm.src, parserType->modrm.dest);
		case dwordXorMode: return Xor(memoryBlock, parserType->opmode, parserType->modrm.src, parserType->modrm.dest);
		case dwordShiftLeftMode: 
		case dwordShiftRightMode: return Shift(memoryBlock, parserType->opmode, parserType->disp, parserType->modrm.dest);
		case cmpMode: return Cmp(memoryBlock, parserType->opmode, parserType->modrm.src, parserType->modrm.dest);
		case byteRelJmpMode: 
		case wordRelJmpMode: 
		case dwordRelJmpMode: return Jmp(memoryBlock, parserType->opmode, parserType->disp);
		case byteRelJeMode: 
		case byteRelJneMode: 
		case byteRelJaMode: 
		case byteRelJbeMode: return Jcc(memoryBlock, parserType->opmode, parserType->disp);

		case byteRelJbMode:
		case byteRelJaeMode: return Jcc2(memoryBlock, parserType->opmode, parserType->disp);
		
		
		case leaWithDispMode: 
		case leaWithoutDispMode: return opmodeError("lea", "i dont want to parse this... please use Lea() instead");

		case retMode: return Ret(memoryBlock);
		case nopMode: return Nop(memoryBlock);
		
		default: opmodeError("parse", str); return none;
		}
	
		return none;
	}

	string trim(const string& str) const{
		size_t first = str.find_first_not_of(' ');
		if (string::npos == first)
		{
			return str;
		}
		size_t last = str.find_last_not_of(' ');
		if (first == string::npos) return "";
		return str.substr(first, (last - first + 1));
	}

	OperandSizes parse(vect8* memoryBlock, const char* str, Disp extra = Disp()) const{
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

		regNames = new string[regNum] regData;

		//to op
		parse_op(&parserType, &op_str, &src_str, &dest_str, extra);

		return run_op(memoryBlock, &parserType, str);

	}


};

