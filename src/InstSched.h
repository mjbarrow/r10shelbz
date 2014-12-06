/*
 * InstSched.h
 *
 *  Created on: Dec 3, 2014
 *      Author: king
 */

#ifndef INSTSCHED_H_
#define INSTSCHED_H_

#include <iostream>
#include <queue>
#include <algorithm>
#include "utils.h"
#include "instructions.h"
#include "InstPipe.h"
#include "ROB.h"			//This needs to do something to the ROB? Actually I dont think so...

#define PrintQueueEntry(i,x) 				cerr << "<|| #" << i;	\
											cerr << ":\t|";			\
											cerr << "tr# " << (*x).traceLineNo;	\
											cerr << "\t| ";						\
											if(((*x).m_rs_rdy && (*x).m_rt_rdy))\
												cerr << "READY! \t\t|";			\
											else if((!(*x).m_rs_rdy) && (*x).m_rt_rdy)	\
												cerr << "Wait: m_rs\t\t|";			  	\
											else if((!(*x).m_rt_rdy) && (*x).m_rs_rdy)	\
												cerr << "Wait: m_rt\t\t|";				\
											else if(((*x).m_rs_rdy || (*x).m_rt_rdy))	\
												cerr << "Wait: m_rs+m_rt\t| ";			\
											cerr << "rd: " <<hex<<"0x"<< (*x).rd << " <- rs:" <<hex<<"0x"<< (*x).rs << " "+(*x).strOp+" rt: " <<hex<<"0x"<< (*x).rt;\
											cerr << "\t|";			\
											cerr <<	"m_rd: " <<hex<<"0x"<< (*x).m_rd.machineReg << " <- m_rs:" <<hex<<"0x"<< (*x).m_rs << " "+(*x).strOp+" m_rt: " <<hex<<"0x"<< (*x).m_rt;\
											cerr << "\t||>," << endl;


using namespace std;

namespace R10k {

class FPQueueEntry: public traceinstruction
{
public:
	bool m_rt_rdy;
	bool m_rs_rdy;

	FPQueueEntry(){m_rt_rdy = m_rs_rdy = false;}
	FPQueueEntry(traceinstruction* t) : traceinstruction(*t) {m_rt_rdy = m_rs_rdy = false;}

	virtual~FPQueueEntry(){}

};
typedef FPQueueEntry ALUQueueEntry;
typedef FPQueueEntry LSQueueEntry;

typedef vector<FPQueueEntry>::iterator FPQueueEntryiterator;	//Used when printing out the regmaptable
typedef FPQueueEntryiterator ALUQueueEntryiterator;
typedef FPQueueEntryiterator LSQueueEntryiterator;

class FPQueue: public vector<FPQueueEntry>
{
public:
	FPQueue(){}

	void print()
	{
		int i = 0;
		cerr << "[" << endl;
		for(FPQueueEntryiterator it = this->begin(); it != this->end(); it++)
		{
			PrintQueueEntry(i,it);
			i++;
		}
		cerr << "]" << endl;
	}

	virtual~FPQueue(){}
};

typedef FPQueue ALUQueue;
typedef FPQueue LSQueue;

//Arbitration struct. Corresponds to all entry ports of the Instruction pipe
class arbitrationStruct
{
public:
	arbitrationStruct(){}
	virtual~arbitrationStruct(){}

	FPQueueEntryiterator 	scheduledFPM;
	FPQueueEntryiterator 	scheduledFPA;
	ALUQueueEntryiterator 	scheduledALU1;
	ALUQueueEntryiterator 	scheduledALU2;
	LSQueueEntryiterator 	scheduledLS1;
//Branch does not go into the pipes, it is just verified	int oldestBtrace;
};

class InstSchedStage {
public:
	InstSchedStage(	ROB* 			ROB,
					InstPipeStage* 	pPipes)
	{
		_ROB = ROB;
		_pPipes = pPipes;
		_scheduledInstructions.scheduledFPM = 	_FPInstructionQueue.end();
		_scheduledInstructions.scheduledFPA = 	_FPInstructionQueue.end();
		_scheduledInstructions.scheduledALU1 = 	_ALUInstructionQueue.end();
		_scheduledInstructions.scheduledALU2 = 	_ALUInstructionQueue.end();
		_scheduledInstructions.scheduledLS1 = 	_LSInstructionQueue.end();
	}

	//The Scheduler class should check this before trying to call pushInstruction
	bool isFPQueueFull()						{if(_FPInstructionQueue.size() 	>= 16)return true; return false;}
	bool isALUQueueFull()						{if(_ALUInstructionQueue.size() >= 16)return true; return false;}
	bool isLSQueueFull()						{if(_LSInstructionQueue.size() 	>= 16)return true; return false;}
//DEBUG! MOVE THESE TO PRIVATE?!
	//Can fail, The sheduler ought to check or it could drop an instruction.
	bool pushFPInstruction(traceinstruction i);
	bool pushALUInstruction(traceinstruction i);
	bool pushLSInstruction(traceinstruction i);

//Called externally
	bool promoQueueToPipe();

	void printInstructionQueues()
	{
		cerr << "\t\t\t\t\t= Floating Point Queue status: =" << endl;
		_FPInstructionQueue.print();
		cerr << "\t\t\t\t\t================================" << endl;
		cerr << "\t\t\t\t\t\t= ALU Queue status: =" << endl;
		_ALUInstructionQueue.print();
		cerr << "\t\t\t\t\t\t=====================" << endl;
		cerr << "\t\t\t\t\t\t= LS Queue status: =" << endl;
		_LSInstructionQueue.print();
		cerr << "\t\t\t\t\t\t====================" << endl;
	}

	void printArbitrationStruct()
	{
		cerr << "\t\t\t\t\t= Arbitration struct status: =" << endl;

		if(_scheduledInstructions.scheduledFPM != _FPInstructionQueue.end())
			{cerr << "scheduled FPM:\t,"; PrintQueueEntry(0,_scheduledInstructions.scheduledFPM);}
		else
			{cerr << "scheduled FPM:\t," << 0  << "<|| NULL ||>" << endl;}
		if(_scheduledInstructions.scheduledFPA != _FPInstructionQueue.end())
			{cerr << "scheduled FPA:\t,"; PrintQueueEntry(1,_scheduledInstructions.scheduledFPA);}
		else
			{cerr << "scheduled FP2:\t," << 1  << "<|| NULL ||>" << endl;}
		if(_scheduledInstructions.scheduledALU1 != _ALUInstructionQueue.end())
			{cerr << "scheduled ALU1:\t,"; PrintQueueEntry(2,_scheduledInstructions.scheduledALU1);}
		else
			{cerr << "scheduled ALU1:\t," << 2  << "<|| NULL ||>" << endl;}
		if(_scheduledInstructions.scheduledALU2 != _ALUInstructionQueue.end())
			{cerr << "scheduled ALU2:\t,"; PrintQueueEntry(3,_scheduledInstructions.scheduledALU2);}
		else
			{cerr << "scheduled ALU2:\t," << 3  << "<|| NULL ||>" << endl;}
		if(_scheduledInstructions.scheduledLS1 != _LSInstructionQueue.end())
			{cerr << "scheduled LS1:\t,"; PrintQueueEntry(4,_scheduledInstructions.scheduledLS1);}
		else
			{cerr << "scheduled LS1:\t," << 4  << "<|| NULL ||>" << endl;}
		cerr << "\t\t\t\t\t================================" << endl;
	}

	virtual ~InstSchedStage();

private:
	ROB* 					_ROB;
	InstPipeStage*			_pPipes;

	arbitrationStruct		_scheduledInstructions;
	FPQueue 				_FPInstructionQueue;
	ALUQueue				_ALUInstructionQueue;
	LSQueue					_LSInstructionQueue;
	//The Execution unit

	//Fix Dependencies 		(combinational logic)
	void _readyFPQueue();
	void _readyALUQueue();
	void _readyLSQueue();
	void _resolveSchedDependencies()			{_readyFPQueue();_readyALUQueue();_readyLSQueue();}
	//Schedule instructions	(combinational logic)
	void _getFirstFPMulandAdd(FPQueueEntryiterator* oldestMul, FPQueueEntryiterator* oldestAdd);
	void _getFirsttwoALU(FPQueueEntryiterator* oldALU1, FPQueueEntryiterator* oldALU2);
	void _getFirstLS(FPQueueEntryiterator* oldestLS);
	void _arbitrateQueues();
};

} /* namespace R10k */

#endif /* INSTSCHED_H_ */
