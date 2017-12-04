#include <stdio.h>
#include <string.h>
#include "simulator.h"

#ifndef PIPELINEREGISTERS_H_
#define PIPELINEREGISTERS_H_

struct regzero{
	unsigned int PC;
};
typedef struct regzero RegZero;

struct regpcif{
	RegZero left;
	RegZero right;
};
typedef struct regpcif RegPCIf;

RegZero initialRegZero(RegZero R0){
	R0.PC = 0;
	return R0;
}

struct regone{
	char OPtype[10];
	unsigned int Instruction;
	unsigned int PCPlus4;
};
typedef struct regone RegOne;

RegOne initialRegOne(RegOne R1){
	strcpy(R1.OPtype, "NOP");
	R1.Instruction = 0;
	R1.PCPlus4 = 0;
	return R1;
}

struct regifid{
	RegOne left;
	RegOne right;
};
typedef struct regifid RegIfId;

struct regtwo{
	char OPtype[10];
	int RegWrite;
	int MemtoReg;
	int MemWrite;
	int ALUControl;
	int ALUSrc;
	int RegDst;
	int ShamtSignal;
	int Jal;
	int ExRsUse;
	int ExRtUse;
	unsigned int RsValue;
	unsigned int RtValue;
	unsigned int Shamt;	
	unsigned char Rs;
	unsigned char Rt;	
	unsigned char Rd;
	unsigned int Immediate;
	unsigned int PCPlus4;
};
typedef struct regtwo RegTwo;

RegTwo initialRegTwo(RegTwo R2){
	strcpy(R2.OPtype, "NOP");
	R2.RegWrite = 0;
	R2.MemtoReg = 0;
	R2.MemWrite = 0;
	R2.ALUControl = 0;
	R2.ALUSrc = 0;
	R2.RegDst = 0;
	R2.ShamtSignal = 0;
	R2.Jal = 0;
	R2.ExRsUse = 0;
	R2.ExRtUse = 0;
	R2.RsValue = 0;
	R2.RtValue = 0;
	R2.Shamt = 0;
	R2.Rs = 0;
	R2.Rt = 0;
	R2.Rd = 0;	
	R2.Immediate = 0;
	R2.PCPlus4 = 0;
	return R2;
}

struct regidex{
	RegTwo left;
	RegTwo right;
};
typedef struct regidex RegIdEx;

struct regthree{
	char OPtype[10];
	int RegWrite;
	int MemtoReg;
	int MemWrite;	
	unsigned int ALUOut;
	unsigned int WriteData;
	unsigned char WriteReg;	
};
typedef struct regthree RegThree;

RegThree initialRegThree(RegThree R3){
	strcpy(R3.OPtype, "NOP");
	R3.RegWrite = 0;
	R3.MemtoReg = 0;
	R3.MemWrite = 0;
	R3.ALUOut = 0;
	R3.WriteData = 0;
	R3.WriteReg = 0;
	return R3;
}

struct regexdm{
	RegThree left;
	RegThree right;
};
typedef struct regexdm RegExDm;

struct regfour{
	char OPtype[10];
	int RegWrite;
	int MemtoReg;
	unsigned int ReadData;
	unsigned int ALUOut;
	unsigned char WriteReg;
};
typedef struct regfour RegFour;

RegFour initialRegFour(RegFour R4){
	strcpy(R4.OPtype, "NOP");
	R4.MemtoReg = 0;
	R4.RegWrite = 0;
	R4.ReadData = 0;
	R4.ALUOut = 0;
	R4.WriteReg = 0;
	return R4;
}

struct regdmwb{
	RegFour left;
	RegFour right;
};
typedef struct regdmwb RegDmWb;

struct pipelineregs{
	RegPCIf pcif;
	RegIfId ifid;
	RegIdEx idex;
	RegExDm exdm;
	RegDmWb dmwb;
	char WbRightOptype[10];	
};
typedef struct pipelineregs PipeLineRegs;

PipeLineRegs initialPipeLine(PipeLineRegs PR){
	PR.pcif.left = initialRegZero(PR.pcif.left);
	PR.pcif.right = initialRegZero(PR.pcif.right);
	PR.ifid.left = initialRegOne(PR.ifid.left);
	PR.ifid.right = initialRegOne(PR.ifid.right);
	PR.idex.left = initialRegTwo(PR.idex.left);
	PR.idex.right = initialRegTwo(PR.idex.right);
	PR.exdm.left = initialRegThree(PR.exdm.left);
	PR.exdm.right = initialRegThree(PR.exdm.right);	
	PR.dmwb.left = initialRegFour(PR.dmwb.left);
	PR.dmwb.right = initialRegFour(PR.dmwb.right);
	strcpy(PR.WbRightOptype, "NOP");
	return PR;		
}

PipeLineRegs advancePipeLine(PipeLineRegs PR){
	PR.dmwb.right = PR.dmwb.left;
	PR.exdm.right = PR.exdm.left;
	PR.idex.right = PR.idex.left;
	PR.ifid.right = PR.ifid.left;
	PR.pcif.right = PR.pcif.left;
	return PR;
}

int HaltDetect(PipeLineRegs PR)
{
	if( ( getOpcode(PR.ifid.left.Instruction) == HALT) && ( strcmp(PR.idex.left.OPtype, "HALT") == 0) && 
	    ( strcmp(PR.exdm.left.OPtype, "HALT") == 0) && ( strcmp(PR.dmwb.left.OPtype, "HALT") == 0) && 
		( strcmp(PR.WbRightOptype, "HALT") == 0) ){
		return 1;
	}else{
		return 0;
	}		
}

#endif
