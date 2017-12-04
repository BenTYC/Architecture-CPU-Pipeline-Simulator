#include <stdio.h>
#include <string.h>

#ifndef STAGEFUNCTION_H_
#define STAGEFUNCTION_H_

unsigned int Mux_two(unsigned int zero, unsigned int one, int ControlSignal)
{
	if(ControlSignal == 0){
		return zero;
	}else{
		return one;
	}		
}

unsigned int Mux_three(unsigned int zero, unsigned int one, unsigned int two, int ControlSignal)
{
	if(ControlSignal == 0){
		return zero;
	}else if(ControlSignal == 1){
		return one;
	}else{
		return two;
	}		
}

unsigned int Add( unsigned int A, unsigned int B)
{
	return A + B;
}

unsigned int Signed_extend( unsigned int A)
{
	int SignedA = A;
	SignedA  <<= 16;
	SignedA  >>= 16;
	
	return SignedA ;
}

int Branch_check(int diff, int Branch)
{
	if(Branch == 1){		
		if(diff == 0){
			return 1;
		}else{
			return 0;
		}
	}else if(Branch == 2){
		if(diff != 0){
			return 1;
		}else{
			return 0;
		}
	}else{
		return 0;
	}
}

unsigned int ALU(unsigned int A, unsigned int B, int ALUControl)
{
	unsigned int ALUResult;
	int temp, temp2;
	switch(ALUControl){
		case 0:  //AND
			ALUResult = A & B;
			break;
		case 1:  //OR
			ALUResult = A | B;
			break;
		case 2:  //ADD
			ALUResult = A + B;
			break;
		case 3:  //XOR
			ALUResult = A ^ B;
			break;
		case 4:  //NAND
			ALUResult = ~(A & B);
			break;
		case 6:  //SUB
			ALUResult = A - B;
			break;
		case 7:  //SLT
			temp = A;
			temp2 = B;
			ALUResult = (temp < temp2);
			break;
		case 8:  //SLL
			ALUResult = B << A;
			break;
		case 9:  //SRL
			ALUResult = B >> A;
			break;
		case 10:  //SRA
			temp = B;
			ALUResult = temp >> A;
			break;
		case 12:  //NOR
			ALUResult = ~( A | B );
			break;
		case 13:  //LUI
			ALUResult = B << 16;
			break;
		default:
			
			break;
	}		
	return ALUResult;
}

void writeDM(unsigned int WriteData, unsigned int ALUResult, char *OPtype, unsigned int *Dmem)
{	
	if(strcmp(OPtype, "SW") == 0){
		Dmem[ALUResult/4] = WriteData;	
	}else if(strcmp(OPtype, "SH") == 0){
		if( ALUResult % 4 == 0){
			Dmem[ALUResult/4] = (Dmem[ALUResult/4] & 0xFFFF) | (WriteData << 16);
		}else{
			Dmem[ALUResult/4] = (Dmem[ALUResult/4] & 0xFFFF0000) | (WriteData & 0xFFFF);	
		}
	}else if(strcmp(OPtype, "SB") == 0){
		if( ALUResult % 4 == 0){
			Dmem[ALUResult/4] = (Dmem[ALUResult/4] & 0x00FFFFFF) | (WriteData << 24);
		}else if( ALUResult % 4 == 1){
			Dmem[ALUResult/4] = (Dmem[ALUResult/4] & 0xFF00FFFF) | ((WriteData << 16) & 0xFF0000);
		}else if( ALUResult % 4 == 2){
			Dmem[ALUResult/4] = (Dmem[ALUResult/4] & 0xFFFF00FF) | ((WriteData << 8) & 0xFF00);
		}else {
			Dmem[ALUResult/4] = (Dmem[ALUResult/4] & 0xFFFFFF00) | (WriteData & 0xFF);
		}
	}else{
		//printf("writeDM Error\n");
	}	
}

unsigned int ReadDM(unsigned int Address, char *OPtype, unsigned int *Dmem)
{
	unsigned int ReadData;
	int tempn;
	if(strcmp(OPtype, "LW") == 0){
		ReadData = Dmem[Address/4];
	}else if(strcmp(OPtype, "LH") == 0){
		if( Address % 4 == 0){
			tempn = Dmem[Address/4];
			tempn >>= 16;
			ReadData = tempn;
		}else{
			tempn = Dmem[Address/4];
			tempn <<= 16;
			tempn >>= 16;
			ReadData = tempn;
		}
	}else if(strcmp(OPtype, "LHU") == 0){
		if( Address % 4 == 0){
			ReadData = Dmem[Address/4] >> 16;
		}else{
			ReadData = Dmem[Address/4] & 0xFFFF;
		}
	}else if(strcmp(OPtype, "LB") == 0){
		if( Address % 4 == 0){
			tempn = Dmem[Address/4];
			tempn >>= 24;
			ReadData = tempn;
		}else if( Address % 4 == 1){
			tempn = Dmem[Address/4];
			tempn <<= 8;
			tempn >>= 24;
			ReadData = tempn;
		}else if( Address % 4 == 2){
			tempn = Dmem[Address/4];
			tempn <<= 16;
			tempn >>= 24;
			ReadData = tempn;
		}else {
			tempn = Dmem[Address/4];
			tempn <<= 24;
			tempn >>= 24;
			ReadData = tempn;
		}
	}else if(strcmp(OPtype, "LBU") == 0){
		if( Address % 4 == 0){
			ReadData = Dmem[Address/4] >> 24;
		}else if( Address % 4 == 1){
			ReadData = (Dmem[Address/4] & 0xFF0000) >> 16;
		}else if( Address % 4 == 2){
			ReadData = (Dmem[Address/4] & 0xFF00) >> 8;
		}else {
			ReadData = Dmem[Address/4] & 0xFF;
		}
	}else{
		//printf("ReadDM Error\n");
	}
	return ReadData;
}

#endif
