#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "simulator.h"
#include "pipelineregisters.h"
#include "controlunit.h"
#include "pipeline.h"
#include "errordetect.h"
#include "stagefunction.h"


int main(void) 
{
	FILE *fp, *fps, *fpe;
	int PCa[4], SPa[4], wordsI[4], wordsD[4], temp[4];
	int iNum, dNum;
	unsigned int PC, SP, PC_origin;
	unsigned int Imem[MemorySize], Dmem[MemorySize], registers[REGS];
	int i, j, tempn;
	
		
	//Initialize memory & registers
	for(i = 0; i < MemorySize; i++) {
		Imem[i] = 0;
		Dmem[i] = 0;
	}
	for(i = 0; i < REGS; i++) {
		registers[i] = 0x0;
	}
	
	
	//Insert input data (memory SP PC)
	fp = fopen("iimage.bin","rb");
	assert(fp != NULL);
	
	for(i = 0; i < 4; i++) {
		PCa[i]=fgetc(fp);
	}
	for(i = 0; i < 4; i++) {
		wordsI[i]=fgetc(fp);
	}		
	iNum = BtoW(wordsI);
	
	for(i = 0; i < iNum; i++) {
		for( j = 0; j < 4; j++) {
			temp[j]=fgetc(fp);
		}
		Imem[i] = BtoW(temp);
	}
	
	fclose(fp);	
	
	fp = fopen("dimage.bin","rb");
	assert(fp != NULL);
	for(i = 0; i < 4; i++) {
		SPa[i]=fgetc(fp);
	}
	for(i = 0; i < 4; i++) {
		wordsD[i]=fgetc(fp);
	}	
	dNum = BtoW(wordsD);
		
	for(i = 0; i < dNum; i++) {
		for( j = 0; j < 4; j++) {
			temp[j] = fgetc(fp);
		}
		Dmem[i] = BtoW(temp);
	}
	
	fclose(fp);
			
	PC = BtoW(PCa);
	SP = BtoW(SPa);
	PC_origin = PC;
	registers[29] = SP;
		
	
	//Setup for pipeline
	PipeLineRegs PR;
	MainControl maincon;
	MainControl *MC = &maincon;
	ForwardingUnit exforwardunit;
	ForwardingUnit *FUE = &exforwardunit;
	ForwardingUnit idforwardunit;
	ForwardingUnit *FUD = &idforwardunit;
	unsigned int ResultW, SrcAE, SrcBE, WriteRegE, RsAfterFwdE, RtAfterFwdE, ALUOutE;
	unsigned int ShamtD, ImmediateD, SignedImmediateD, AddressD, RsAfterFwdD, RtAfterFwdD;
	unsigned char RsD, RtD, RdD;
	unsigned int InstructionF, PCPlus4F, PCBranch, PCtnotJump, PCJump, PCJr;	
	int BranchSignalP, checkStall = 0, checkFlush = 0, checkHalt = 0;
	int cycleCount = 0, checkW0 = 0, checkALUNO = 0, checkBranchNO = 0, checkAO = 0, checkMA = 0;
	
	
	//Initialize pipeline unit
	PR = initialPipeLine(PR);
	initialMainControl(MC);
	initialForwardingUnit(FUE);
	initialForwardingUnit(FUD);
	
	
	//Insert original PC
	PR.pcif.right.PC = PC_origin;
	
	
	//Run the pipeline
	fps = fopen("snapshot.rpt", "w");
	fpe = fopen("error_dump.rpt", "w");	
	
	while( !checkHalt ) {
		
		//Print
		Print_reg(registers, PR.pcif.right.PC, cycleCount, fps);
		
		
		//WB stage + check Write $0
		strcpy( PR.WbRightOptype, PR.dmwb.right.OPtype);
		ResultW = Mux_two( PR.dmwb.right.ALUOut, PR.dmwb.right.ReadData, PR.dmwb.right.MemtoReg);
		if( PR.dmwb.right.RegWrite ){
			if( ( PR.dmwb.right.WriteReg == 0) && ( strcmp(PR.dmwb.right.OPtype, "NOP") != 0 ) ){
				checkW0 = 1;
			}else{
				registers[PR.dmwb.right.WriteReg] = ResultW;
			}
		}
		
		
		
		//DM stage
		strcpy( PR.dmwb.left.OPtype, PR.exdm.right.OPtype);
		PR.dmwb.left.RegWrite = PR.exdm.right.RegWrite;
		PR.dmwb.left.MemtoReg = PR.exdm.right.MemtoReg;
		
			//check DM AO+MA
		if( PR.exdm.right.MemWrite || PR.exdm.right.MemtoReg ){
			checkAO = check_AO( PR.exdm.right.ALUOut, PR.exdm.right.OPtype);
			checkMA = check_MA( PR.exdm.right.ALUOut, PR.exdm.right.OPtype);
		}	
		
		if( checkAO == 0 && checkMA == 0 ){
			if(PR.exdm.right.MemWrite){
				writeDM( PR.exdm.right.WriteData, PR.exdm.right.ALUOut, PR.exdm.right.OPtype, Dmem);
			}
			if(PR.exdm.right.MemtoReg){
				PR.dmwb.left.ReadData = ReadDM(PR.exdm.right.ALUOut, PR.exdm.right.OPtype, Dmem);
			}
		}
		
		PR.dmwb.left.ALUOut = PR.exdm.right.ALUOut;
		PR.dmwb.left.WriteReg = PR.exdm.right.WriteReg;
		
		
		//EX stage		
		strcpy(PR.exdm.left.OPtype, PR.idex.right.OPtype);
		PR.exdm.left.RegWrite = PR.idex.right.RegWrite;
		PR.exdm.left.MemtoReg = PR.idex.right.MemtoReg;
		PR.exdm.left.MemWrite = PR.idex.right.MemWrite;		
		
			//FUE Forwarding Detect
		MakeExFwdSignal(PR, FUE);		
		
		RsAfterFwdE = Mux_three( PR.idex.right.RsValue, ResultW, PR.exdm.right.ALUOut, FUE->ForwardA);
		RtAfterFwdE = Mux_three( PR.idex.right.RtValue, ResultW, PR.exdm.right.ALUOut, FUE->ForwardB);
		SrcAE = Mux_two( RsAfterFwdE, PR.idex.right.Shamt, PR.idex.right.ShamtSignal);
		SrcBE = Mux_two( RtAfterFwdE, PR.idex.right.Immediate, PR.idex.right.ALUSrc);
		
			//check ALU NO
		checkALUNO = check_ALUNO( SrcAE, SrcBE, PR.idex.right.ALUControl);
		
		ALUOutE = ALU( SrcAE, SrcBE, PR.idex.right.ALUControl);
		WriteRegE = Mux_three( PR.idex.right.Rt, PR.idex.right.Rd, 31, PR.idex.right.RegDst);
		PR.exdm.left.ALUOut = Mux_two( ALUOutE, PR.idex.right.PCPlus4, PR.idex.right.Jal);
		PR.exdm.left.WriteData = RtAfterFwdE;
		PR.exdm.left.WriteReg = WriteRegE;
		
		
		//ID stage
		strcpy(PR.idex.left.OPtype, PR.ifid.right.OPtype);
		
		MakeControlSignal(MC, PR.ifid.right.Instruction);
		PR.idex.left.RegWrite = MC->RegWrite;
		PR.idex.left.MemtoReg = MC->MemtoReg;
		PR.idex.left.MemWrite = MC->MemWrite;
		PR.idex.left.ALUControl = MC->ALUControl;
		PR.idex.left.ALUSrc = MC->ALUSrc;
		PR.idex.left.RegDst = MC->RegDst;
		PR.idex.left.ShamtSignal = MC->Shamt;
		PR.idex.left.ExRsUse = MC->ExRsUse;
		PR.idex.left.ExRtUse = MC->ExRtUse;
		PR.idex.left.Jal = MC->Jal;
		
		RsD = getRs(PR.ifid.right.Instruction);
		RtD = getRt(PR.ifid.right.Instruction);
		RdD = getRd(PR.ifid.right.Instruction);
		ShamtD = getShamt(PR.ifid.right.Instruction);
		ImmediateD = getImmediate(PR.ifid.right.Instruction);
		AddressD = getAddress(PR.ifid.right.Instruction);
		SignedImmediateD = Signed_extend( ImmediateD );
		
			//FUD Forwarding Detect
		MakeIdFwdSignal(PR, FUD, MC->IdRsUse, MC->IdRtUse);
		
		RsAfterFwdD = Mux_two( registers[RsD], PR.exdm.right.ALUOut, FUD->ForwardA);
		RtAfterFwdD = Mux_two( registers[RtD], PR.exdm.right.ALUOut, FUD->ForwardB);
		PR.idex.left.RsValue = RsAfterFwdD;
		PR.idex.left.RtValue = RtAfterFwdD;
		PR.idex.left.Shamt = ShamtD;
		PR.idex.left.Rs = RsD;
		PR.idex.left.Rt = RtD;
		PR.idex.left.Rd = RdD;
		PR.idex.left.Immediate = Mux_two( ImmediateD, SignedImmediateD, MC->Signed);
		PR.idex.left.PCPlus4 = 	PR.ifid.right.PCPlus4;
		
		
		//IF stage
		if(PR.pcif.right.PC >= PC_origin){
			InstructionF = Imem[(PR.pcif.right.PC-PC_origin)/4];
		}else{
			InstructionF = 0;
		}
		
		getOptype( PR.ifid.left.OPtype, InstructionF);
		PR.ifid.left.Instruction = InstructionF;
		PCPlus4F = Add(PR.pcif.right.PC, 4);
		PR.ifid.left.PCPlus4 = PCPlus4F;
		
		
		//PC stage 	
		BranchSignalP = Branch_check( RsAfterFwdD - RtAfterFwdD	, MC->Branch);
		
			//check Branch NO
		checkBranchNO = check_BranchNO( SignedImmediateD << 2, PR.ifid.right.PCPlus4, MC->Branch, checkStall);	
			
		PCBranch = Add(SignedImmediateD << 2, PR.ifid.right.PCPlus4);
		PCtnotJump = Mux_two( PCPlus4F, PCBranch, BranchSignalP);
		PCJump = ( PR.ifid.right.PCPlus4 & 0xF0000000 ) | ( AddressD << 2 );
		PCJr = Mux_two( registers[RsD], PR.exdm.right.ALUOut, FUD->ForwardA);
		PR.pcif.left.PC = Mux_three( PCtnotJump, PCJump, PCJr, MC->Jump);
		
		
		//Hazard detect
		checkStall = stallDetect(PR, MC, WriteRegE, PR.exdm.right.WriteReg, RsD, RtD);
		checkFlush = ( (MC->Jump == 1) || ( (!checkStall) && ( (MC->Jump == 2) || BranchSignalP ) ) );
		
		
		//Print Stage
		Print_stage(PR, FUD, FUE, checkFlush, checkStall, fps);
		
		
		//Hazard handle
		if(checkFlush){
			PR.ifid.left = initialRegOne(PR.ifid.left);
		}
		if(checkStall){
			PR.idex.left = initialRegTwo(PR.idex.left);
			PR.pcif.left = PR.pcif.right;
			PR.ifid.left = PR.ifid.right;
		}
		
		
		//Advance	
		PR = advancePipeLine(PR);
		cycleCount++;
		
		
		//Error handle		
		Print_Error(checkW0, checkALUNO, checkBranchNO, checkAO, checkMA, cycleCount, fpe);		
		if( checkMA || checkAO ){
			break;
		}
		checkW0 = 0; checkALUNO = 0; checkBranchNO = 0; checkAO = 0; checkMA = 0;
		
		//check_Halt
		checkHalt = HaltDetect(PR);
			
	}
	
	fclose(fps);
	fclose(fpe);
	
	return 0;
}
