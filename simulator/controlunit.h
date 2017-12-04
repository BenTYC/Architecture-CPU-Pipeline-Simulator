#include <stdio.h>
#include <string.h>
#include "simulator.h"

#ifndef CONTROLUNIT_H_
#define CONTROLUNIT_H_

struct maincontrol{
	char OPtype[10];
	int Jump;
	int Branch;
	int MemtoReg;
	int MemRead;
	int MemWrite;
	int ALUControl;
	int ALUSrc;
	int RegWrite;
	int RegDst;
	int Signed;	
	int Shamt;
	int Jal;
	int IdRsUse;
	int IdRtUse;
	int ExRsUse;
	int ExRtUse;
};
typedef struct maincontrol MainControl;

void initialMainControl(MainControl *M){
	strcpy(M->OPtype, "\0");
	M->Jump = 0;
	M->Branch = 0;
	M->MemtoReg = 0;
	M->MemRead = 0;
	M->MemWrite = 0;
	M->ALUControl = 0;
	M->ALUSrc = 0;
	M->RegWrite = 0;
	M->RegDst = 0;
	M->Signed = 0;
	M->Shamt = 0;
	M->Jal = 0;
	M->IdRsUse = 0;
	M->IdRtUse = 0;
	M->ExRsUse = 0;
	M->ExRtUse = 0;		
}

void MakeControlSignal(MainControl *M, unsigned int Instruction)
{
	unsigned char opcode = getOpcode( Instruction );
	
	initialMainControl(M);
	
	if(opcode == HALT){
		strcpy(M->OPtype, "HALT");
	}else if( opcode == RTYPEOP){	
		unsigned char funct = getFunct( Instruction );
		switch(funct){
			case ADD:  //$d = $s + $t
				strcpy(M->OPtype, "ADD");
				M->RegWrite = 1;	
				M->ALUControl = 2;
				M->RegDst = 1;
				M->ExRsUse = 1;
				M->ExRtUse = 1;	
				break;
			case SUB:  //$d = $s - $t
				strcpy(M->OPtype, "SUB");
				M->RegWrite = 1;
				M->ALUControl = 6;
				M->RegDst = 1;
				M->ExRsUse = 1;
				M->ExRtUse = 1;		
				break;
			case AND:  //$d = $s & $t
				strcpy(M->OPtype, "AND");
				M->RegWrite = 1;
				M->ALUControl = 0;
				M->RegDst = 1;
				M->ExRsUse = 1;
				M->ExRtUse = 1;		
				break;
			case OR:  //$d = $s | $t
				strcpy(M->OPtype, "OR");
				M->RegWrite = 1;
				M->ALUControl = 1;
				M->RegDst = 1;
				M->ExRsUse = 1;
				M->ExRtUse = 1;		
				break;
			case XOR:  //$d = $s ^ $t
				strcpy(M->OPtype, "XOR");
				M->RegWrite = 1;
				M->ALUControl = 3;
				M->RegDst = 1;
				M->ExRsUse = 1;
				M->ExRtUse = 1;		
				break;
			case NOR:  //$d = ~( $s | $t )
				strcpy(M->OPtype, "NOR");
				M->RegWrite = 1;
				M->ALUControl = 12;
				M->RegDst = 1;
				M->ExRsUse = 1;
				M->ExRtUse = 1;		
				break;
			case NAND:  //$d = ~( $s & $t )
				strcpy(M->OPtype, "NAND");
				M->RegWrite = 1;
				M->ALUControl = 4;
				M->RegDst = 1;
				M->ExRsUse = 1;
				M->ExRtUse = 1;		
				break;
			case SLT:  //$d = ( $s < $t ), signed comparition
				strcpy(M->OPtype, "SLT");
				M->RegWrite = 1;
				M->ALUControl = 7;
				M->RegDst = 1;
				M->ExRsUse = 1;
				M->ExRtUse = 1;		
				break;
			case SLL:  //$d = $t << Imm
				if( getRd(Instruction)==0 && getRt(Instruction)==0 && getShamt(Instruction)==0){
					strcpy(M->OPtype, "NOP");
				}else{
					strcpy(M->OPtype, "SLL");
					M->RegWrite = 1;
					M->ALUControl = 8;
					M->RegDst = 1;
					M->Shamt = 1;
					M->ExRtUse = 1;
				}					
				break;
			case SRL:  //$d = $t >> Imm
				strcpy(M->OPtype, "SRL");
				M->RegWrite = 1;
				M->ALUControl = 9;
				M->RegDst = 1;
				M->Shamt = 1;
				M->ExRtUse = 1;	
				break;
			case SRA:  //$d = $t >> Imm, signed bit shift
				strcpy(M->OPtype, "SRA");
				M->RegWrite = 1;
				M->ALUControl = 10;
				M->RegDst = 1;
				M->Shamt = 1;
				M->ExRtUse = 1;	
				break;
			case JR:  //PC = $s
				strcpy(M->OPtype, "JR");
				M->Jump = 2;
				M->IdRsUse = 1;
				break;
			default:
				strcpy(M->OPtype, "RTypeWrong");	
				break;			
		}		
	}else{
		M->ALUSrc = 1;
		switch(opcode){			
			case ADDI:  //$t = $s + Imm(signed)					
				strcpy(M->OPtype, "ADDI");
				M->RegWrite = 1;
				M->ALUControl = 2;
				M->Signed = 1;
				M->ExRsUse = 1;	
				break;
			case LW:  //$t = 4 bytes from Memory[$s + Imm(signed)] 
				strcpy(M->OPtype, "LW");
				M->RegWrite = 1;
				M->MemtoReg = 1;
				M->ALUControl = 2;
				M->Signed = 1;
				M->ExRsUse = 1;		
				break;
			case LH:  //$t = 2 bytes from Memory[$s + Imm(signed)] 
				strcpy(M->OPtype, "LH");
				M->RegWrite = 1;
				M->MemtoReg = 1;
				M->ALUControl = 2;
				M->Signed = 1;
				M->ExRsUse = 1;			
				break;
			case LHU:  //$t = 4 bytes from Memory[$s + Imm(signed)], unsigned 
				strcpy(M->OPtype, "LHU");
				M->RegWrite = 1;
				M->MemtoReg = 1;
				M->ALUControl = 2;
				M->Signed = 1;
				M->ExRsUse = 1;			
				break;
			case LB:  //$t = Memory[$s + Imm(signed)], signed 
				strcpy(M->OPtype, "LB");
				M->RegWrite = 1;
				M->MemtoReg = 1;
				M->ALUControl = 2;
				M->Signed = 1;	
				M->ExRsUse = 1;		
				break;
			case LBU:  //$t = Memory[$s + Imm(signed)], unsigned 
				strcpy(M->OPtype, "LBU");
				M->RegWrite = 1;
				M->MemtoReg = 1;
				M->ALUControl = 2;
				M->Signed = 1;	
				M->ExRsUse = 1;		
				break;
			case SW:  //4 bytes from Memory[$s + Imm(signed)] = $t 
				strcpy(M->OPtype, "SW");
				M->MemWrite = 1;
				M->ALUControl = 2;
				M->Signed = 1;
				M->ExRsUse = 1;
				M->ExRtUse = 1;		
				break;
			case SH:  //2 bytes from Memory[$s + Imm(signed)] = $t & 0x0000FFFF 
				strcpy(M->OPtype, "SH");	
				M->MemWrite = 1;
				M->ALUControl = 2;
				M->Signed = 1;
				M->ExRsUse = 1;
				M->ExRtUse = 1;		
				break;
			case SB:  //Memory[$s + Imm(signed)] = $t & 0x000000FF 
				strcpy(M->OPtype, "SB");
				M->MemWrite = 1;
				M->ALUControl = 2;
				M->Signed = 1;
				M->ExRsUse = 1;
				M->ExRtUse = 1;			
				break;
			case LUI:  //$t = Imm << 16 
				strcpy(M->OPtype, "LUI");
				M->RegWrite = 1;
				M->ALUControl = 13;		
				break;
			case ANDI:  //$t = $s & Imm(unsigned) 
				strcpy(M->OPtype, "ANDI");
				M->RegWrite = 1;	
				M->ALUControl = 0;
				M->ExRsUse = 1;	
				break;	
			case ORI:  //$t = $s | Imm(unsigned) 
				strcpy(M->OPtype, "ORI");
				M->RegWrite = 1;
				M->ALUControl = 1;
				M->ExRsUse = 1;				
				break;
			case NORI:  //$t = ~($s | Imm(unsigned)) 
				strcpy(M->OPtype, "NORI");	
				M->RegWrite = 1;
				M->ALUControl = 12;
				M->ExRsUse = 1;			
				break;
			case SLTI:  //$t = ($s < Imm(signed) ), signed comparison 
				strcpy(M->OPtype, "SLTI");
				M->RegWrite = 1;	
				M->ALUControl = 7;
				M->Signed = 1;	
				M->ExRsUse = 1;		
				break;
			case BEQ:  //if ($s == $t) go to PC+4+4*Imm(signed) 
				strcpy(M->OPtype, "BEQ");
				M->Branch = 1;				
				M->Signed = 1;
				M->IdRsUse = 1;
				M->IdRtUse = 1;		
				//M->ALUOp = 6;	
				break;	
			case BNE:  //if ($s != $t) go to PC+4+4*Imm(signed) 
				strcpy(M->OPtype, "BNE");
				M->Branch = 2;				
				M->Signed = 1;
				M->IdRsUse = 1;
				M->IdRtUse = 1;		
				//M->ALUOp = 6;		
				break;
			case J:  //PC = (PC+4)[31:28] | 4*Imm(unsigned) 
				strcpy(M->OPtype, "J");
				M->Jump = 1;
				break;
			case JAL:  //$31 = PC + 4; PC = (PC+4)[31:28] | 4*Imm(unsigned) 
				strcpy(M->OPtype, "JAL");
				M->RegWrite = 1;
				M->RegDst = 2;
				M->Jump = 1;
				M->Jal = 1;
				break;
			default:
				strcpy(M->OPtype, "OPwrong");
				break;
		}
	}
}

#endif
