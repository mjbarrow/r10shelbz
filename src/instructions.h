/*
 * instructions.h
 *
 *  Created on: Nov 16, 2014
 *      Author: king
 */

#include "utils.h"

#ifndef INSTRUCTIONS_H_
#define INSTRUCTIONS_H_

//Trace defines
#define L 0b000001
#define S 0b000010
#define B 0b000100
#define I 0b001000
#define A 0b010000
#define M 0b100000

//Other defines related to class types
#define BADInstKey -1
#define BADOpcode BADInstKey
#define BADOperand BADInstKey

#define needextra(x) (x & 0b000111)

//Instruction enumeration defines
#define ADD 	0b100000
#define ADDI 	0b001000
#define ADDIU	0b001001
#define ADDU	0b100001
#define AND		0b100100
#define ANDI	0b001100
#define BEQ		0b000100
#define BEQL	0b010100
//maybe a weird one. needs REGIMM
#define BGEZ	0b00001
#define BGEZAL	0b10001
#define BGEZALL	0b10011
#define BGEZL	0b00011
#define BGTZ	0b000111
#define BGTZL	0b010111
#define BLEZ	0b000110
#define BLEZL	0b010110
#define BLTZ	0b0000
#define BLTZAL	0b10000
#define BLTZALL 0b10010
#define BLTZL	0b00010
#define BNE		0b000101
#define BNEL	0b010101
#define BREAK	0b001101
#define COP0	0b010000
#define COP1	0b010001
#define COP2	0b010010
#define COP3	0b010011
#define DADD	0b101100
#define DADDI	0b011000
#define DADDIU	0b011001
#define DADDU	0b101101
#define DDIV	0b011110
#define DDIVU	0b011111
#define DIV		0b011010
#define DIVU	0b011011
#define DMULT	0b011100
#define DMULTU	0b011101
#define DSLL	0b111000
#define DSLL32	0b111100
#define DSLLV	0b010100
#define DSRA	0b111011
#define DSRA32	0b111111
#define DSRAV	0b010111
#define DSRL	0b111010
#define DSRL32	0b111110
#define DSRLV	0b010110
//Codes available here: http://math-atlas.sourceforge.net/devel/assembly/mips-iv.pdf


//End instruction enumeration defines


#include<string>

using namespace std;

namespace R10k {

typedef int traceline;

class instruction {
public:
	instruction();
	instruction(string str_instruction);
	string Opcode_name;
	int Opcode_val;
	string Operand1_name;
	int Operand1_val;
	string Operand2_name;
	int Operand2_val;
	string Operand3_name;
	int Operand3_val;

	virtual ~instruction();
};

class traceinstruction{
	public:
	traceinstruction();
	traceinstruction(string traceline, int tracelineNo);
	string strOp;
	traceline traceLineNo;
//Original trace register numbers (ISA Regs)
	int intOp;
	int rs;
	int rt;
	int rd;
	int extra;
//Re-mapped trace variables used by the scheduler and ROB during commit (I guess)
	int m_rs;
	int m_rt;
	RegMapKey m_rd;	//Special. Provides a reverse mapping key that is used to clear map table entries.

	virtual ~traceinstruction();
};

} /* namespace R10k */

#endif /* INSTRUCTIONS_H_ */
