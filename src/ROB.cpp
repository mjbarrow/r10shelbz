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

//TODO swizzle registers
//Blit the appropriate widget
void ROB::risingEdge()
{
	vector<string> stringROB;
	unsigned int i;

	//Commit old instructions. They will go away forever to memory.
	_commitTailInstructions(COMMITPORTCOUNT);	//TODO pipeline diagram tends to show these somewhere.
	//Add the new instructions into the ROB. They will live until pipes commit them
	_issuedInstsToROB();

	//Construct the correct type of input for the blit function
	//An array of strings which it will blit out
	for(i = 0; i < ROBbuffer.size(); i++)
	{
		stringROB.push_back(bufferLineToString(i));
	}
	if(stringROB.size() == 0)
		stringROB.push_back("empty");

	//trigger the blit function so that the screen output is refreshed of the ROB content
	_ui->blitROBList(&stringROB);
}

//TODO: this needs to be fixed up with clock because it has no
//Clock cycle value in the newEntry
void ROB::_issuedInstsToROB()//addEntry(traceinstruction value)
{
	robEntry newEntry;
//	robEntry newEntry(&value);
	int i = 0;
	while(i < ISSUEWAYCOUNT)
	{
		//Add all of the outstanding instructions to the ROB...

		//Create a new ROB entry
		if(_issuedInsts[_issuePortHead].intOp != BADOpcode)
		{
			//If something exists at this port,
			newEntry = robEntry(&_issuedInsts[_issuePortHead]);	//Make a new ROB entry

			if((ROBbuffer.size() <= head) && (head < ROBSize))	//Ugh, circular buffer house keeping...
			{
				ROBbuffer.push_back(newEntry);					//Add new ROB entry to ROB
				head++;											//Be sure not to overwrite it next time!
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

			//Nuke port value for next time...
			_issuedInsts[_issuePortHead] = traceinstruction();
		}

		//Index house keeping for the loop...
		_issuePortHead++;
		if(_issuePortHead == ISSUEWAYCOUNT)
			_issuePortHead = 0;
		i++;
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

void ROB::_commitTailInstructions(int count)
{
	RegMapKey ISARegToUnmap;				//Use to disambiguate and clear register maps.
	robEntry commitme;						//another working variable
	vector<robEntry>::iterator inflight;	//Use to check no in flight instructions depend on the instruction we
											//Want to commit

	if(count > ROBSize) return;	//this is just nonsense.

	//Add the destination machine registers back into the free list
	while(	count &&
			(tail < ROBbuffer.size())	)	//You may not commit more instructions than exist in the ROB presently. (it is a vector and grows)
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

//Perform the retirement of all instructions at the port pipes
//On the clock falling edge
void ROB::fallingEdge()//_retireEntry(traceinstruction retire)
{
	vector<robEntry>::iterator i;
	int loop;

	for(loop = 0; loop < RETIREPORTCOUNT; loop++)
	{
		if(_retirePorts[loop].intOp != BADOpcode)			//if this is some weird value just dont even bother trying to retire
		{
			for(i = ROBbuffer.begin(); i < ROBbuffer.end(); i++)	//Check every ROB entry for this trace instruction
			{
				if ((*i).m_rd.Key == _retirePorts[loop].m_rd.Key)	//If we find the trace instruction at the input port
					(*i).retired = true;							//Retire that instruction in the ROB
			}														//Now the scheduler will know another register is ready
		}
		_retirePorts[loop] = traceinstruction();	//Nuke the port value so we don't pick up junk
													//If the execution pipes don't do anything next cycle
	}
		//return false;
}

} /* namespace R10k */
