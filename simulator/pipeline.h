#include <stdio.h>
#include <string.h>
#include "simulator.h"
#include "pipelineregisters.h"
#include "controlunit.h"

#ifndef PIPELINE_H_
#define PIPELINE_H_

struct forwardingunit{
	int ForwardA;
	int ForwardB;	
};
typedef struct forwardingunit ForwardingUnit;

void initialForwardingUnit(ForwardingUnit *FU)
{
	FU->ForwardA = 0;
	FU->ForwardB = 0;
	return;
}

void Print_stage(PipeLineRegs PR, ForwardingUnit *FUD, ForwardingUnit *FUE, int checkFlush, int checkStall, FILE *fps)
{	
	//IF
	fprintf(fps, "IF: 0x%08X", PR.ifid.left.Instruction);
	if(checkStall > 0){
		fprintf(fps, " to_be_stalled");
	}else if(checkFlush != 0){
		fprintf(fps, " to_be_flushed");
	}
	fprintf(fps, "\n");	
	
	//ID
	fprintf(fps, "ID: %s", PR.idex.left.OPtype);
	if(checkStall > 0){
		fprintf(fps, " to_be_stalled");
	}else{
		if(FUD->ForwardA == 1){
			fprintf(fps, " fwd_EX-DM_rs_$%d", PR.exdm.right.WriteReg);
		}
		if(FUD->ForwardB == 1){
			fprintf(fps, " fwd_EX-DM_rt_$%d", PR.exdm.right.WriteReg);
		}
	}
	fprintf(fps, "\n");	
	
	//EX
	fprintf(fps, "EX: %s", PR.exdm.left.OPtype);
	if(FUE->ForwardA == 2){
		fprintf(fps, " fwd_EX-DM_rs_$%d", PR.exdm.right.WriteReg);
	}else if(FUE->ForwardA == 1){
		fprintf(fps, " fwd_DM-WB_rs_$%d", PR.dmwb.right.WriteReg);
	}
	if(FUE->ForwardB == 2){
		fprintf(fps, " fwd_EX-DM_rt_$%d", PR.exdm.right.WriteReg);
	}else if(FUE->ForwardB == 1){
		fprintf(fps, " fwd_DM-WB_rt_$%d", PR.dmwb.right.WriteReg);
	}
	fprintf(fps, "\n");	
	
	//DM
	fprintf(fps, "DM: %s\n", PR.dmwb.left.OPtype);
	
	//WB
	fprintf(fps, "WB: %s\n\n\n", PR.WbRightOptype);
		
	return;
}

void MakeExFwdSignal(PipeLineRegs PR, ForwardingUnit *FUE)
{
	initialForwardingUnit(FUE);
	
	// input來源： 2 為 DM stage, 1 為 WB stage
	if( PR.dmwb.right.RegWrite && (PR.dmwb.right.WriteReg != 0) && PR.idex.right.ExRsUse && 
	    !( PR.exdm.right.RegWrite && (PR.exdm.right.WriteReg != 0) && (PR.exdm.right.WriteReg == PR.idex.right.Rs) ) && 
		(PR.dmwb.right.WriteReg == PR.idex.right.Rs) ){
		FUE->ForwardA = 1;
	}else if( PR.exdm.right.RegWrite && (PR.exdm.right.WriteReg != 0) && PR.idex.right.ExRsUse && 
	          (PR.exdm.right.WriteReg == PR.idex.right.Rs) ){
		FUE->ForwardA = 2;
	}
	//printf("wb.RegWrite:%d ex.ExRtUse:%d wb.WriteReg:%d ex.Rt:%d\n", PR.dmwb.right.RegWrite, PR.idex.right.ExRtUse, PR.dmwb.right.WriteReg, PR.idex.right.Rt);
	
	if ( PR.dmwb.right.RegWrite && (PR.dmwb.right.WriteReg != 0) && PR.idex.right.ExRtUse && 
	     !( PR.exdm.right.RegWrite && (PR.exdm.right.WriteReg != 0) && (PR.exdm.right.WriteReg == PR.idex.right.Rt)) && 
		 (PR.dmwb.right.WriteReg == PR.idex.right.Rt) ){
		FUE->ForwardB = 1;
	}else if( PR.exdm.right.RegWrite && (PR.exdm.right.WriteReg != 0) && PR.idex.right.ExRtUse && 
	          (PR.exdm.right.WriteReg == PR.idex.right.Rt)){
		FUE->ForwardB = 2;
	}
	return;
}

void MakeIdFwdSignal(PipeLineRegs PR, ForwardingUnit *FUD, int IdRsUse, int IdRtUse)
{
	int RsD = getRs(PR.ifid.right.Instruction);
	int RtD = getRt(PR.ifid.right.Instruction);
	
	initialForwardingUnit(FUD);
	
	//input來源只有DM stage 
	if( PR.exdm.right.RegWrite && (PR.exdm.right.WriteReg != 0) && IdRsUse && (PR.exdm.right.WriteReg == RsD) ){
		FUD->ForwardA = 1;
	}else{
		FUD->ForwardA = 0;
	}
	//printf("dm.RegWrite:%d IdRtUse:%d dm.WriteReg:%d RtD:%d\n", PR.exdm.right.RegWrite, IdRtUse, PR.exdm.right.WriteReg, RtD);
	if( PR.exdm.right.RegWrite && (PR.exdm.right.WriteReg != 0) && IdRtUse && (PR.exdm.right.WriteReg == RtD) ){
		FUD->ForwardB = 1;
	}else{
		FUD->ForwardB = 0;
	}
	
	return;
}

int stallDetect(PipeLineRegs PR, MainControl *MC, int WriteRegE, int WriteRegM, int RsD, int RtD)
{
	//ID input use
	if( MC->IdRsUse ){
		if( PR.idex.right.RegWrite && ( WriteRegE != 0 ) && ( WriteRegE == RsD ) ){
			return 1;
		}else if( PR.exdm.right.MemtoReg && ( WriteRegM != 0 ) && ( WriteRegM == RsD ) ){
			return 1;
		}
	}
	if( MC->IdRtUse ){
		if( PR.idex.right.RegWrite && ( WriteRegE != 0 ) && ( WriteRegE == RtD ) ){
			return 1;
		}else if( PR.exdm.right.MemtoReg && ( WriteRegM != 0 ) && ( WriteRegM == RtD ) ){
			return 1;
		}
	}
	
	//Ex input use
	if( MC->ExRsUse ){
		if( PR.idex.right.MemtoReg && ( WriteRegE != 0 ) && ( WriteRegE == RsD ) ){
			return 1;
		}
	}
	if( MC->ExRtUse ){
		if( PR.idex.right.MemtoReg && ( WriteRegE != 0 ) && ( WriteRegE == RtD ) ){
			return 1;
		}
	}
	
	return 0;
			
}

#endif

