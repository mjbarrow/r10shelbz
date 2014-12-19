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
/*	vector<robEntry>::iterator i;
	vector<traceinstruction>::iterator loop;
//	int loop = 0;

	//Note all in-flight instructions that have begun to execute
	for(loop = _executionPorts.begin(); loop < _executionPorts.end(); loop++)//(loop = 0; loop < RETIREPORTCOUNT; loop++)
	{
//TODO	CHANGED TO INF PORTS FOR THE SCHED STAGE	if(_executionPorts[loop].intOp != BADOpcode)			//if this is some weird value just dont even bother trying to retire
		if(loop->intOp != BADOpcode)
		{
			for(i = ROBbuffer.begin(); i < ROBbuffer.end(); i++)	//Check every ROB entry for this trace instruction
			{
//TODO				if (i->m_rd.Key == _executionPorts[loop].m_rd.Key)	//If we find the trace instruction at the input port
				if(i->m_rd.Key == loop->m_rd.Key)
				{
					i->didExecute = true;							//Retire that instruction in the ROB
				}
				//Certain instructions (Store and branch) do not have a destination register and so may retire regardless of key
				if(i->intOp & (S | B))
				{
					i->didExecute = true;
				}
			}														//Now the scheduler will know another register is ready
		}
//TODO		_executionPorts[loop] = traceinstruction();	//Nuke the port value so we don't pick up junk
		*loop = traceinstruction();						//Nuke the port value so we don't pick up junk
														//If the execution pipes don't do anything next cycle
	}

	_executionPorts = vector<traceinstruction>();		//Nuke the whole vector. for safty
*/
	_myCycleCounter++;			//This is used so the ROB displays the cycle a trace line entered it.

	//Commit old instructions. They will go away forever to memory.
//	_commitTailInstructions(COMMITPORTCOUNT);	//TODO pipeline diagram tends to show these somewhere.

	//Add the new instructions into the ROB. They will live until pipes commit them
	_issuedInstsToROB();
}

//Perform the retirement of all instructions at the port pipes
//On the clock falling edge, so we ought to update the UI here too
void ROB::fallingEdge()
{
	vector<string> stringROB;
	vector<robEntry>::iterator i;
	int loop;

	//Note all in-flight instructions that have finished executing
	for(loop = 0; loop < RETIREPORTCOUNT; loop++)
	{
		if(_retirePorts[loop].intOp != BADOpcode)			//if this is some weird value just dont even bother trying to retire
		{
			for(i = ROBbuffer.begin(); i < ROBbuffer.end(); i++)	//Check every ROB entry for this trace instruction
			{
				//For store. this instruction has no desination register and may be retired
				//by traceline number. TODO this is debug code
				if(i->traceLineNo == _retirePorts[loop].traceLineNo && (i->intOp != BADOpcode))
				{
					i->retired = true;
				}

			}														//Now the scheduler will know another register is ready
		}
		_retirePorts[loop] = traceinstruction();	//Nuke the port value so we don't pick up junk
													//If the execution pipes don't do anything next cycle
	}
/*
	//Note all new in-flight instructions
	for(loop = 0; loop < RETIREPORTCOUNT; loop++)
	{
		if(_executingPorts[loop].intOp != BADOpcode)			//if this is some weird value just dont even bother trying to retire
		{
			for(i = ROBbuffer.begin(); i < ROBbuffer.end(); i++)	//Check every ROB entry for this trace instruction
			{
				if(i->traceLineNo == _executingPorts[loop].traceLineNo && (i->intOp != BADOpcode))
				{
					i->didExecute = true;
				}

			}														//Now the scheduler will know another register is ready
		}
		_executingPorts[loop] = traceinstruction();	//Nuke the port value so we don't pick up junk
													//If the execution pipes don't do anything next cycle
	}*/
//TODO DEBUG SHOULD COMMIT ON FALLING EDGE AFTER SCHEDULE
	_commitTailInstructions(COMMITPORTCOUNT);	//TODO pipeline diagram tends to show these somewhere.
	//Add the new instructions into the ROB. They will live until pipes commit them

	//Construct the correct type of input for the blit function
	//An array of strings which it will blit out
	for(loop = 0; loop < (int)ROBbuffer.size(); loop++)
	{
		stringROB.push_back(bufferLineToString(loop));
	}
	if(stringROB.size() == 0)
		stringROB.push_back("empty");

	//trigger the blit function so that the screen output is refreshed of the ROB content
	_ui->blitROBList(&stringROB);
}

//======================================================================================================
//									HELPER METHODS FOR CALC
//======================================================================================================

//don't muck about. Immidieately set the tag for this instruction.
void ROB::entryExecuted(traceinstruction* inflight)
{
	vector<robEntry>::iterator i;

	for(i = ROBbuffer.begin(); i < ROBbuffer.end(); i++)	//Check every ROB entry for this trace instruction
	{
		if(i->traceLineNo == inflight->traceLineNo && (i->intOp != BADOpcode))
		{
			i->didExecute = true; break;
		}
	}														//Now the scheduler will know another register is ready
}

//Only return false if you find this instruction has not executed
//Otherwise, it may have already been retired
bool ROB::hasEntryExecuted(traceinstruction checkme)
{
	vector<robEntry>::iterator i;

	for(i = ROBbuffer.begin(); i < ROBbuffer.end(); i++)	//Check every ROB entry for this trace instruction
	{
		if(	(i->traceLineNo == checkme.traceLineNo)	&&
			(!i->didExecute)									)
			return false;
	}

	return true;
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
			newEntry = robEntry(&_issuedInsts[_issuePortHead],_myCycleCounter);	//Make a new ROB entry

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
	if(machinereg == BADOperand)		//The Load instruction only has one dependency
		return true;					//The other register is bad and does not need to be checked.

	for(	std::vector<robEntry>::iterator entry = ROBbuffer.begin();
			entry != ROBbuffer.end();
			entry++			)
	{
		if(entry->m_rd.machineReg == machinereg)	//This is the ROB entry you are looking for
			if(entry->retired == false)				//The corresponding instruction has not completed
				return false;						//Dependency has therefore been resolved
	}

	//return tr
	return true;
}

//Call this only for a store operation
bool ROB::isAddressDependencyMet(int address, int traceNo, int instruction)
{
	if(instruction == BADOpcode)
		return true;

	for(	std::vector<robEntry>::iterator entry = ROBbuffer.begin();		//We will check all in-flight instructions
			entry != ROBbuffer.end();										//To see if there could be a conflict
			entry++														)
	{

		if( 	((entry->intOp ==  L ) || (entry->intOp == S)) &&	//Shengye assumption. To avoid failiure caused by exceptions
																	//Never execute a memory op unless the previous one completed
				/*(entry->extra == dependent->extra) &&*/				//If it uses the same address, there is still potential
				(entry->traceLineNo < traceNo) &&	//If it is an older instruction than this, there is still potential
				(entry->retired == false)								//We must wait until this Load executes or we cause WAR
			)
			return false;
	}

	return true;
}


//TODO: BUG BUG!!! CAN COMMIT OUT OF ORDER AND DOES NOT SEEM TO ACCOUNT FOR A CIRCULAR BUFFER!

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
			if(		/*(!inflight->didExecute) && Not valid. you do not have to wait for didExecute*/
					(!inflight->retired) &&
					(inflight->m_rt == commitme.m_rd.machineReg) &&
					(inflight->rt != BADOperand)							)	//Discard this check if the rt register is not used
				return;
			//Also cannot commit if an older trace than commitme has not been committed yet
			if(		(inflight->traceLineNo < commitme.traceLineNo ) &&
					(inflight->traceLineNo != BADTraceNo))					//Discard this check if the instruction has no trace line (weird)
				return;
		}

		(*_FreeRegList)[0].push(commitme.m_rd.machineReg);			//Free the machine destination register
		_plogger->logCMTrace(commitme.traceLineNo);					//Add the instruction to the pipeline diagram
																//Get a map iterator for the ISA reg-> machine reg
																//For this trace line
		ISARegToUnmap.ISAReg			= commitme.rd;			//Get the ISA reg for the trace line
		ISARegToUnmap.InstanceCounter 	= commitme.m_rd.ISAInstanceCounter;	//Get the correct instance of this ISA reg
		(*_regMapTable)[0].erase(ISARegToUnmap.Key);					//The <key,value> <{instance,ISAReg},machineReg> can be deleted from our map

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



} /* namespace R10k */
