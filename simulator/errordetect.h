#include <stdio.h>
#include <string.h>

#ifndef ERRORDETECT_H_
#define ERRORDETECT_H_

void Print_Error(int checkW0, int checkALUNO, int checkBranchNO, int checkAO, int checkMA, int cycleCount, FILE *fpe)
{
	if(checkW0){
		fprintf( fpe , "In cycle %d: Write $0 Error\n", cycleCount); 
	}
	if(checkAO){
		fprintf( fpe , "In cycle %d: Address Overflow\n", cycleCount); 
	}
	if(checkMA){
		fprintf( fpe , "In cycle %d: Misalignment Error\n", cycleCount); 
	}
	if( checkALUNO || checkBranchNO ){
		fprintf( fpe , "In cycle %d: Number Overflow\n", cycleCount); 
	}
	return;
}

int check_AO(unsigned int Address, char *OPtype)
{
	if( Address < 0 || Address > 1023){
		return 1;
	}
	if( strcmp(OPtype, "LW")==0 || strcmp(OPtype, "SW")==0 ){
		Address += 3;
	}else if( strcmp(OPtype, "LH")==0 || strcmp(OPtype, "LHU")==0 || strcmp(OPtype, "SH")==0 ){
		Address += 1; 
	}
	if( Address < 0 || Address > 1023){
		return 1;
	}
	return 0;	
}

int check_MA(unsigned int Address, char *OPtype)
{
	if( strcmp(OPtype, "LW")==0 || strcmp(OPtype, "SW")==0 ){
		if( Address % 4 != 0){
			return 1;
		}
	}else if( strcmp(OPtype, "LH")==0 || strcmp(OPtype, "LHU")==0 || strcmp(OPtype, "SH")==0 ){
		if( Address % 2 != 0){
			return 1;
		}
	}
	return 0;
}

int check_overflow(unsigned int A, unsigned int B)
{
	int a = A; 
	int b = B;
	int sum = a + b;
	if ( (sum ^ a) >= 0 || (sum ^ b) >= 0 ){ //use xor to check
		return 0;
	}else{ 
		return 1; //overflow happen
	}
}

int check_ALUNO( unsigned int Rs, unsigned int Rt, int ALUControl)
{
	if( ALUControl == 6 ) {
		Rt = ~Rt + 1;
		ALUControl = 2;
	}
	if( ALUControl == 2 ){
		if( check_overflow(Rs, Rt) ){
			return 1;
		}
	}
	return 0;
}

int check_BranchNO( unsigned int Rs, unsigned int Rt, int MCBranch, int checkStall)
{
	if( MCBranch && !checkStall){
		if(check_overflow(Rs, Rt)){
			return 1;
		}
	}
	return 0;
}
#endif
