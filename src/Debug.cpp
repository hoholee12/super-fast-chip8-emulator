#include"Debug.h"

void Debug::printDebug(uint8_t* v, uint16_t pc, uint16_t stack, uint16_t input, defaults* mainwindow, Memory* memory, Audio* audio, uint8_t delayRegister){
	static int count = 0;
	static bool inCall = false;

	/*memory dump*/
	/*
	int32_t spc = pc & 0xfff0;
	
	if ((spc -= 0x20) < 0) spc = 0;

	printf("Memory dump %02X~%02X:\n<", spc, spc + 0x40);
	printf("%02X ", memory->read(spc));
	for (uint16_t i = 0x1; i < 0x40; i++){
		if (i % 0x10 == 0) printf("\n");
		printf("%02X ", memory->read(spc + i));
	}
	printf(">\n");
	*/


	/*delay register*/
	if (delayRegister != 0x0) printf("delayReg = %x\n", delayRegister);




	/*Vreg dump*/
	printf("Vreg dump: <");
	for (int i = 0; i < 0x10; i++){
		printf("%02X ", v[i]);
	}
	printf(">\n");




	/*sound timer*/
	uint8_t soundtimer = audio->getSoundTimer();
	if (soundtimer != 0){
		printf("soundTimer = %x\n", soundtimer);
	}
	
	
	
	/*opcode parse*/
	if (inCall) printf("(call)\t");
	printf("%d:\tpc: %x\t\tstack: %x\topcode: %x\t", ++count, pc, stack, input);

	switch ((input & 0xf000) >> 12){
	case 0x0:
		switch (input & 0x00ff){
		case 0xe0: printf("video\n");
			break;
		case 0xee:	printf("ret\n"); inCall = false;//return from subroutine
			break;
		default: printf("call\n"); inCall = true; //call subroutine from nnn
			break;
		}
		break;
	case 0x1: printf("jmp\n");//jump to nnn
		break;
	case 0x2:	 printf("call\n"); inCall = true; //call subroutine from nnn
		break;
	case 0x3:	 printf("skip\n");//skip if ==
		break;
	case 0x4:	 printf("skip\n");//skip if !=
		break;
	case 0x5:	 printf("skip\n");//skip if vx == vy
		break;
	case 0x6:	printf("setreg\n");//into
		break;
	case 0x7:	printf("setreg\n");
		break;
	case 0x8: printf("setreg\n");
		break;
	case 0x9:  printf("skip\n");//skip if vx != vy
		break;
	case 0xa: printf("setreg\n");
		break;
	case 0xb: printf("jmp\n");
		break;
	case 0xc:	printf("setreg\n");//random
		break;
	case 0xd: printf("video\n");
		break;
	case 0xe:  printf("input\n");
		break;
	case 0xf:
		switch (input & 0x00ff){
		case 0x07:	printf("setreg\n");
			break;
		case 0x0a: printf("input\n");//wait again
			break;
		case 0x15:	printf("setreg\n");
			break;
		case 0x18:	printf("sound\n");
			break;
		case 0x1e:	printf("setreg\n");
			break;
		case 0x29:	printf("setreg\n");//font is stored at mem[0 ~ FONT_COUNT * 5]
			break;
		case 0x33: printf("mem\n");
			break;
		case 0x55: printf("mem\n");
			break;
		case 0x65:	printf("mem\n");
			break;
		default: 
			break;
		}
		break;
	default:
		break;
	}

	//debug time
	mainwindow->delayTime(DEBUG_TIME);
}