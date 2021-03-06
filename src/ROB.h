/*
 * ROB.h
 *
 *  Created on: Nov 23, 2014
 *      Author: king
 *      USED to abstract the ROB buffer from pipeline stages because it should be used by
 *      Instruction decode and commit stages
 */

#ifndef ROB_H_
#define ROB_H_

#include <vector>
#include <iostream>
#include "utils.h"
#include "instructions.h"
#include "userinterface.h"
#include "TraceOutputLogger.h"


using namespace std;

namespace R10k {

//For debug purposes a ROB entry will contain:
/*
 * cycle added: traceinstruction
 * A commit operation will dump that information along with the commit cycle counter
 */

class robEntry: public traceinstruction
{
public:
	bool retired;				//you may only commit retired instructions
	bool didExecute;

	int instantiationcycle;		//this is for debug visibility mainly

	robEntry()														{instantiationcycle = 0; retired = false; didExecute = false;}
	robEntry(traceinstruction* t, int c)	:traceinstruction(*t)	{instantiationcycle = c; retired = false; didExecute = false; return;}

	bool operator==(const robEntry& rhs) const
	{
		return(	(extra == rhs.extra) 	&&
				(intOp == rhs.intOp) 	&&
				(m_rs == rhs.m_rs) 		&&
				(m_rd.Key == rhs.m_rd.Key)&&
				(m_rt == rhs.m_rt)		&&
				(rs == rhs.rs)			&&
				(rd == rhs.rd)			&&
				(rt == rhs.rt)			&&
				(retired == rhs.retired)&&
				(didExecute == rhs.didExecute)	&&
				(instantiationcycle == rhs.instantiationcycle)&&
				(traceLineNo == rhs.traceLineNo)&&
				(strOp.compare(rhs.strOp) == 0)		);
	}

	bool operator!=(const robEntry& rhs) const 						{return !operator==(rhs);}

	virtual ~robEntry(){}
};

class ROB {
public:
	ROB(	UserInterface*			ui,
			TraceOutputLogger* 		logger,
			vector <freeRegList>* 	FreeRegList,
			vector <regmaptable>* 	rmt			)
	{
		head = tail 	= 0;
		_issuePortHead 	= 0;
		_myCycleCounter = 0;
		_ui 			= ui;
		_plogger		= logger;
		_FreeRegList 	= FreeRegList;
		_regMapTable 	= rmt;
		ROBbuffer.push_back(robEntry());		//Initialize first ROB entry for the "full" checking logic
	}

	//Project spec function
	void risingEdge();		//Take in instructions from the Execution pipes here, get rid of old (commit)
	void fallingEdge();		//Make registers available to the scheduler same cycle (retire)
							//They were clocked in from the Execution pipes

	int getHead()							{return head;}
	int getTail()							{return tail;}
	bool isFull();
	void printRobEntry(unsigned int entry)	{if(ROBbuffer.size() > entry){printBufferLine(entry);}}
	void printRob()							{unsigned int i; for(i = 0; i < ROBbuffer.size(); i++){printBufferLine(i);}}
//TODO debug thing called by the scheduler after it is done with the system bus
	void endBusCycle();


//TODO: the instantiation cycle should be fixed up when a clock is added
	void addEntry(traceinstruction value)
	{
		_issuedInsts[_issuePortHead] = value;
		_issuePortHead++;
		if(_issuePortHead == ISSUEWAYCOUNT)
			_issuePortHead = 0;
	};	//Add this trace line into the ROB

	void retireEntry(	int retireport,													//Ex pipes call this during calc.
						traceinstruction retire){_retirePorts[retireport] = retire;}	//To ensure forwarding, it is acted on during fallingEdge()

	//Call this when an instruction has been put down one of the execution pipes.
	void entryExecuted(traceinstruction* inflight);
	bool hasEntryExecuted(traceinstruction checkme);

	bool isDependencyMet(unsigned short machinereg);			//Used by scheduler to check dependencies
																//Must be called after "retireEntry()"
	bool isAddressDependencyMet(int address, int traceNo, int instruction);		//If this address exists in the ROB, belongs to a store and
																//That store did not start going + is older, stall.

	virtual ~ROB();

private:
	unsigned int 		head;
	unsigned int 		tail;
	int					_issuePortHead;
	int					_myCycleCounter;
	vector <freeRegList>*		_FreeRegList;	//The ROB needs this when committing. It will free registers from here
	vector <regmaptable>*		_regMapTable;	//Also needed when committing, to remove
	UserInterface*		_ui;			//Need this to blit out to the user interface
	TraceOutputLogger*	_plogger;

	//used in schedule timing resolution
	traceinstruction _retirePorts[RETIREPORTCOUNT];
	traceinstruction _issuedInsts[ISSUEWAYCOUNT];

	//used by branch resolution when restoring queues
//	traceinstruction _executingPorts[RETIREPORTCOUNT];//TODO rename this to scheduled ports
//Remove this feedback path	vector<traceinstruction>_executionPorts;

	vector<robEntry>ROBbuffer;

	void _issuedInstsToROB();				//Add the _issuedInsts to ROB on the clock risingEdge()
	bool _retireEntry(unsigned int entryindex){if((entryindex < ROBbuffer.size()) && (entryindex <= ROBSize))ROBbuffer[entryindex].retired = true; return true;}

	void _commitTailInstructions(int count); //Will commit this many instructions to memory from ROB


	//Convert the buffer line into a string. Usefull for debug.
	//See: http://stackoverflow.com/questions/3203452/how-to-read-entire-stream-into-a-stdstring
	string bufferLineToString(int entry)
	{
		string robEntryString;
		ostringstream os;
		robEntry line  = ROBbuffer[entry];

		int rt 		= line.rt;		//Fix up swizzled registers during display
		int m_rt 	= line.m_rt;

		if(line.intOp == L)		//Load instruction internally swizzles rt and rd for consistency with the trace format
			{rt = line.rd;	m_rt = line.m_rt;}

		if(((unsigned int)entry == head) && ((unsigned int)entry == tail))
			os << "hd+tl->\t|| #";
		else if((unsigned int)entry == tail)
			os << "tail->\t|| #";
		else if((unsigned int)entry == head)
			os << "head->\t|| #";
		else
			os << "\t|| #";

		os 	<< entry << ":\t|"
			 << " DecCy# "	<< line.instantiationcycle
			 << " rtrd? "	<< line.retired
			 << " Exd? " 	<< line.didExecute
			 << "  | ";
		if(line.intOp != S)
			os << " isa reg " <<hex<<"0x"<< line.rd;
		else
			os << " address " <<hex<<"0x"<< line.extra;

		os	 << " <- isa reg: " <<hex<<"0x"<< line.rs << " "+line.strOp+" isa reg: " <<hex<<"0x"<< rt
			 << "\t|\t"
			 << " m reg "	 <<hex<<"0x"<< line.m_rd.machineReg << " <- m reg: " <<hex<<"0x"<< line.m_rs << " "+line.strOp+" m reg: " <<hex<<"0x"<< m_rt
			 << "\t||";

		robEntryString = os.str();

		return robEntryString;
	}

	void printBufferLine(int entry)	{cerr << bufferLineToString(entry) << endl;}

};

//TODO Function should, swizzle registers and blit ROB to screen
void risingEdge();

} /* namespace R10k */

#endif /* ROB_H_ */























