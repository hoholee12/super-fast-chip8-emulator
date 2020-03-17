#include"Debug.h"

void Debug::printDebug(int count, uint8 pc, uint16 stack, uint16 input){
	printf("%d:\tpc: %x\t\tstack: %x\tcurrent opcode: %x\t", count, pc, stack, input);

	switch ((input & 0xf000) >> 12){
	case 0x0:
		switch (input & 0x00ff){
		case 0xe0: printf("video\n");
			break;
		case 0xee:	printf("ret\n");//return from subroutine
			break;
		default: printf("call\n");//call subroutine from nnn
			break;
		}
		break;
	case 0x1: printf("jmp\n");//jump to nnn
		break;
	case 0x2:	 printf("call\n");//call subroutine from nnn
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
}