
#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <string>
#include <queue>
#include <map>
#include "instructions.h"
#include "stdio.h"	//for SHOW_DEFINE. useful enough that I'll do this.

using namespace std;

//Misc useful stuff http://stackoverflow.com/questions/1164652/printing-name-and-value-of-a-define
#define STR(x)   #x
#define SHOW_DEFINE_AND_VAL(x) printf("%s=%s\n", #x, STR(x))
#define SHOW_DEFINE(x) printf("%s",#x)

//TODO: move this to a machine parameterization file (see: http://en.wikipedia.org/wiki/R10000)
#define ISARegCount 	32
#define renameRegCount 	64
#define ROBSize			48

#define MAXDECODEDINSTPERCYCLE 4

#define FPQPORTCOUNT	4
#define ALUQPORTCOUNT	4
#define ADDQPORTCOUNT	4

//Instruction fetch ish
#define INSTDECODEPERCYCLE 4

//ROB ish
#define COMMITPORTCOUNT	4

#define RETIREPORTCOUNT	5
#define ISSUEWAYCOUNT	4

#define FPMRETIREPORT	0
#define FPARETIREPORT	1
#define ALUARETIREPORT	2
#define ALUBRETIREPORT	3
#define LSARETIREPORT	4

//instruction reg types represent the 4 possible uses of a machine register in an instruction
#define badregtype -1
#define instD 	1
#define instOp1 2
#define instOp2 3
#define instOp3	4
//end inst reg type

//To do with branch predict resolution
#define BRANCHBUFCOUNT	4

//To do with queue size
#define SCHEDQUEUESIZE	16

//Set Behaviour of pipe stages
//NOT DEBUGED #define ARBITRATE_BY_OLDEST
#define ARBITRATE_BY_QUEUE_POSITION


namespace R10k
{

//Active list (these are machine registers being used in the pipeline stages)
//|inst # 	| Inst reg type | renameReg		#| Architectural Reg # 	|
//===================================================================
//| 0		| instD			| 1				|		1				|
//-------------------------------------------------------------------
//| .		|	.			|	.			|		.				|
//-------------------------------------------------------------------
//|	n		| instOp3		|	32			|		n				|
//-------------------------------------------------------------------
typedef struct _ActiveReg {
	int instructionNumber;
	int instructionregtype;
	int renameRegNo;
	int physicalregNo;

	_ActiveReg()
	{
		instructionNumber 	= -1;
		instructionregtype  = badregtype;
		renameRegNo 		= -1;
		physicalregNo 		= -1;
	}
}ActiveReg, *pActveReg;

//freeRegister list, has extra method to print its content.
class freeRegList: public std::queue<int>
{
public:
	freeRegList(){}

	//heavy weight!
	void print()
	{
		std::queue<int> printme = *this;
		int i = 0;
		int value;
		cerr << "[ ";
		while(!printme.empty())
		{
			value = printme.front();
			printme.pop();
			std::cerr << "< " << i << "," << value << "> ," << std::endl;
			i++;
		}
		cerr <<"]" << std::endl;
	}

	virtual~freeRegList(){}
};


//Hardware to do with the register maps

//Register Map table typedefs

	//It can be that an ISA register appears twice or more in the RegMapTable
	//This situation occurs when several instructions have the same target register
	//To deal with this, the key to the RegMapTable will be {ISAReg,InstanceCounter}
	//For book keeping. InstanceCounters are prioritized from earliest instance to latest instance.

typedef union
{
	int Key;	//Because maps need operators for non standard types.
	struct
	{
		short ISAReg, InstanceCounter;
	};
	struct
	{
		short machineReg, ISAInstanceCounter;
	};
}RegMapKey;

typedef map<int, int>::const_iterator regmaptableIterator;	//Used when printing out the regmaptable

class regmaptable: public map<int,int>
{
public:
	regmaptable(){}

	void print()
	{
		RegMapKey key;
		cerr << "[" << endl;
		for(regmaptableIterator it = this->begin(); it != this->end(); it++)
		{
			key.Key = it->first;
			cerr << "<key: { ISA_reg: " << key.ISAReg  << ", instance: " << key.ISAInstanceCounter << "}, mach_reg: " << it->second <<">," << endl;
		}
		cerr << "]" << endl;
	}

	virtual~regmaptable(){}
};
//typedef map<int,int> regmaptable;


typedef pair<int, int> regmappair;

//Level 2 of the RegMapTable index. a seperate map to get the
//Latest instance of an ISA reg that has been mapped
typedef map<int, int> ISAreginstance;
typedef pair<int, int>ISAreginstancepair;

//Pulled out of instructions.h

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


//Pulled out of InstSched.h

class FPQueueEntry: public traceinstruction
{
public:
	bool m_rt_rdy;
	bool m_rs_rdy;
	bool add_rdy;		//Only used by the store instruction for mem disambiguation

	FPQueueEntry(){m_rt_rdy = m_rs_rdy = add_rdy = false;}
	FPQueueEntry(traceinstruction* t) : traceinstruction(*t) {m_rt_rdy = m_rs_rdy = add_rdy = false;}

	virtual~FPQueueEntry(){}

};
typedef FPQueueEntry ALUQueueEntry;
typedef FPQueueEntry LSQueueEntry;

typedef vector<FPQueueEntry>::iterator FPQueueEntryiterator;	//Used when printing out the regmaptable
typedef FPQueueEntryiterator ALUQueueEntryiterator;
typedef FPQueueEntryiterator LSQueueEntryiterator;

/*class FPQueue: public vector<FPQueueEntry>
{
public:
	FPQueue(){}
*/
/*	void print()
	{
		int i = 0;
		cerr << "[" << endl;
		for(FPQueueEntryiterator it = this->begin(); it != this->end(); it++)
		{
//			PrintQueueEntry(i,it);
			i++;
		}
		cerr << "]" << endl;
	}*/
/*
	virtual~FPQueue(){}
};*/

typedef vector<FPQueueEntry> FPQueue;
typedef FPQueue ALUQueue;
typedef FPQueue LSQueue;

//End pulled out of InstSched.h


//End hardware to do with the register maps

//End TODO

//Stuff used throughout the R10K pipeline

	class R10kregister{
	public:
		R10kregister(){name = "-"; value = 0;}
		R10kregister(string Name){name = Name; value = 0;}
		R10kregister(string Name, int Value){name = Name; value = Value;}

		string name;
		int value;
		virtual ~R10kregister(){}	//Boilerplate
	};

}

//Logic building blocks for 32bit clocked registers
namespace logic
{
/*
class flipflop {
public:
	flipflop(){_D = 0; _Q = 0;}
	void rst(){_D = _Q = 0;}
	void setD(int D){_D = D;}
	int getQ(){return _Q;}
	int getD(){return _D;}

	void toggleclk(){_Q = _D;}

	virtual ~flipflop(){}
protected:
	int _D;
	int _Q;
};


class latch: flipflop{
public:
	latch(){_L = 0; _transp = true;}
	void transparent(bool transparent){_transp = transparent; if(_transp)_Q = _D;}
	void setD(int D){if(_transp){_D = D; _Q = _D;} else {_D = D;}}
private:
	bool _L;
	bool _transp;
protected:
	bool getL(){return _L;}
};
*/
}

#endif
