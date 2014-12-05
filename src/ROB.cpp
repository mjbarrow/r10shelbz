/*
 * ROB.cpp
 *
 *  Created on: Nov 23, 2014
 *      Author: king
 */

#include "ROB.h"

namespace R10k {

ROB::~ROB() {
	// TODO Auto-generated destructor stub
}

//TODO: this needs to be fixed up with clock because it has no
//Clock cycle value in the newEntry
void ROB::addEntry(traceinstruction value)
{
	robEntry newEntry(&value);

	if(ROBbuffer.size() <= head)
	{
		ROBbuffer.push_back(newEntry);
		head++;
		if(head >= ROBSize)		//Make the ROB circular, so you could have overflow problems
			head = 0;
	}
	else
	{
		ROBbuffer[head] = newEntry;
		head++;
		if(head >= ROBSize)		//Make the ROB circular, so you could have overflow problems
			head = 0;
	}
}

//TODO: this should be synchronised with clock. It is not

//The dependency is always met unless this register is the target of an in-flight instruction
bool ROB::isDependencyMet(unsigned short machinereg)
{

	for(	std::vector<robEntry>::iterator entry = ROBbuffer.begin();
			entry != ROBbuffer.end();
			entry++														)
	{
		if((*entry).m_rd.machineReg == machinereg)
			if(! (*entry).retired)
				return false;
//TODO: Need some kind of mechanism for handling in flight forwarded instructions
	}

	return true;
}

//TODO: this should be synchronised with clock. It is not

void ROB::commitTailInstructions(int count)
{
	RegMapKey ISARegToUnmap;				//Use to disambiguate and clear register maps.
	robEntry commitme;						//another working variable
	vector<robEntry>::iterator inflight;	//Use to check no in flight instructions depend on the instruction we
											//Want to commit

	if(count > ROBSize) return;	//this is just nonsense.

	//Add the destination machine registers back into the free list
	while(count)
	{
		//Commit the earliest instructions in the ROB
		commitme = ROBbuffer[tail];
		//we may only commit retired instructions
		if(!commitme.retired)
			return;
		//We may only commit instructions which have no non-retired instructions that depend on it.
		for(inflight = ROBbuffer.begin(); inflight < ROBbuffer.end(); inflight++)
		{
			//If this "inflight" instruction is not retired and it requires use of the commit candidate dest reg
			//We cannot commit yet
			if((!(*inflight).retired) && ((*inflight).m_rs == commitme.m_rd.machineReg))
				return;
			//If this "inflight" instruction is not retired and it requires use of the commit candidate dest reg
			//We cannot commit yet
			if((!(*inflight).retired) && ((*inflight).m_rt == commitme.m_rd.machineReg))
				return;
		}

		_FreeRegList->push(commitme.m_rd.machineReg);			//Free the machine destination register
																//Get a map iterator for the ISA reg-> machine reg
																//For this trace line
		ISARegToUnmap.ISAReg			= commitme.rd;			//Get the ISA reg for the trace line
		ISARegToUnmap.InstanceCounter 	= commitme.m_rd.ISAInstanceCounter;	//Get the correct instance of this ISA reg
		_regMapTable->erase(ISARegToUnmap.Key);					//The <key,value> <{instance,ISAReg},machineReg> can be deleted from our map

		ROBbuffer[tail] = robEntry();				//Blank out the old content, its junk.
		tail++;
		if(tail == ROBSize)										//Corner case, you committed up to the end of the ROB
			tail = 0;
		count--;
	}

}

bool ROB::isFull()
{
	robEntry invalid;
	if(tail == head)						//Would adding something to the ROB overwrite what is at the tail?
		if(ROBbuffer[head] != invalid)		//if the entry at the tail just junk, we couldn't possibly be full (true in the initial corner case)
			return true;

	return false;
}

bool ROB::retireEntry(traceinstruction retire)
{
	vector<robEntry>::iterator i;
	if(retire.intOp == BADOpcode)			//if this is some weird value just dont even bother trying to retire
		return false;

	for(i = ROBbuffer.begin(); i < ROBbuffer.end(); i++)
	{
		if ((*i).m_rd.Key == retire.m_rd.Key)
			{(*i).retired = true; return true;}				//Cool! was able to retire an instruction!
	}
	return false;
}

} /* namespace R10k */
