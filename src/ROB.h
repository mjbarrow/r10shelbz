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

#include "utils.h"
#include "instructions.h"
#include <vector>
#include <iostream>


using namespace std;

namespace R10k {

//For debug purposes a ROB entry will contain:
/*
 * cycle added: traceinstruction
 * A commit operation will dump that information along with the commit cycle counter
 */
/*
typedef struct _ROBENTRY {
	int 				instantiationcycle;
	traceinstruction 	value;
}robEntry, *probEntry;*/

class robEntry: public traceinstruction
{
public:
	bool retired;				//you may only commit retired instructions

	int instantiationcycle;		//this is for debug visibility mainly

	robEntry()														{instantiationcycle = 0; retired = false;}
	robEntry(traceinstruction* t)			:traceinstruction(*t)	{instantiationcycle = 0; retired = false; return;}
	robEntry(traceinstruction* t, int c)	:traceinstruction(*t)	{instantiationcycle = c; retired = false; return;}

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
				(instantiationcycle == rhs.instantiationcycle)&&
				(traceLineNo == rhs.traceLineNo)&&
				(strOp.compare(rhs.strOp) == 0)		);
	}

	bool operator!=(const robEntry& rhs) const 						{return !operator==(rhs);}

	virtual ~robEntry(){}
};

class ROB {
public:
	ROB(	freeRegList* FreeRegList,
			regmaptable* rmt/*,
			ISAreginstance* miri*/)
	{
		head = tail 	= 0;
		_FreeRegList 	= FreeRegList;
		_regMapTable 	= rmt;
		ROBbuffer.push_back(robEntry());		//Initialize first ROB entry for the "full" checking logic
//		ROBbuffer.push_back(robEntry());		//Initial mapping for mr_1
//		ROBbuffer.push_back(robEntry());		//initial mapping for mr_2
//		ROBbuffer[1].m_rd.ISAReg = 1;			//Initial val for ROB entry 1 (lets the first instruction be scheduled
//		ROBbuffer[1].m_rd.ISAReg = 2;			//Initial val for ROB entry 2 (lets the second instruction )
	}

	int getHead()							{return head;}
	int getTail()							{return tail;}
	bool isFull();
	void printRobEntry(unsigned int entry)	{if(ROBbuffer.size() > entry){printBufferLine(entry);}}
	void printRob()							{unsigned int i; for(i = 0; i < ROBbuffer.size(); i++){printBufferLine(i);}}

//TODO: the instantiation cycle should be fixed up when a clock is added
	void addEntry(traceinstruction value);	//Add this trace line into the ROB
//FOR DEBUG ONLY
	bool retireEntry(unsigned int entryindex){if((entryindex < ROBbuffer.size()) && (entryindex <= ROBSize))ROBbuffer[entryindex].retired = true; return true;}
	bool retireEntry(traceinstruction retire);	//Long version, used by scheduler
	void commitTailInstructions(int count); //Will commit this many instructions to memory from ROB

	bool isDependencyMet(unsigned short machinereg);

	virtual ~ROB();

private:
	unsigned int 		head;
	unsigned int 		tail;
	freeRegList* 		_FreeRegList;	//The ROB needs this when committing. It will free registers from here
	regmaptable* 		_regMapTable;	//Also needed when committing, to remove
//DO NOT TOUCH.	ISAreginstance* 	_ISAregmap;	//Invalid machine to architecture register maps

	std::vector<robEntry>ROBbuffer;

	void printBufferLine(int entry)
	{
		robEntry line  = ROBbuffer[entry];
		if(((unsigned int)entry == head) && ((unsigned int)entry == tail))
			cout << "hd+tl->\t|| #";
		else if((unsigned int)entry == tail)
			cout << "tail->\t|| #";
		else if((unsigned int)entry == head)
			cout << "head->\t|| #";
		else
			cout << "\t|| #";

		cout << entry << ":\t|"
			 << " DecCy# "	<< line.instantiationcycle
			 << " rtrd? "	<< line.retired
			 << "  | "
			 << " isa reg "	 <<hex<<"0x"<< line.rd << " <- isa reg: " <<hex<<"0x"<< line.rs << " "+line.strOp+" isa reg: " <<hex<<"0x"<< line.rt
			 << "\t|\t"
			 << " m reg "	 <<hex<<"0x"<< line.m_rd.machineReg << " <- m reg: " <<hex<<"0x"<< line.m_rs << " "+line.strOp+" m reg: " <<hex<<"0x"<< line.m_rt
			 << "\t||" << endl;
	}
};

} /* namespace R10k */

#endif /* ROB_H_ */























