/*
 * InstDecode.cpp
 *
 *  Created on: Nov 20, 2014
 *      Author: king
 */

#include "utils.h"
#include "InstDecode.h"
#include <list>
#ifdef DEBUG
#include <iostream>
#endif

using namespace std;

namespace R10k {

//===================================================================================================================
//							SYNCHRONOUS FUNCTIONS
//==================================================================================================================

//TODO swizzle registers
//Blit the appropriate widget
void InstDecodeStage::risingEdge()
{
	vector<string> stringRegMap;
	std::map<int,int>::iterator rmapit;
	regmappair a;

	//Swizzle the D and Q registers
	int i = 0;

	while(i < MAXDECODEDINSTPERCYCLE)
	{
		//Arranget the DBuffer into the Q buffer in a traceline indexed fashion.
		_QTraceLines[i] =  _DTraceLines[_tracebufhead];
		_DTraceLines[_tracebufhead] = traceinstruction();			//Blank this and never work with it again.
		_tracebufhead++;
		if(_tracebufhead == MAXDECODEDINSTPERCYCLE)
			_tracebufhead = 0;
		//Add the Clocked in instructions to the pipeline diagram
		if(_QTraceLines[i].intOp != BADOpcode)
			_plogger->logIDTrace(_QTraceLines[i].traceLineNo);
		i++;
	}

	//Construct the correct type of input for the blit function
	//An array of strings which it will blit out
	for(rmapit = _RegMapTable->begin(); rmapit != _RegMapTable->end(); rmapit++)
	{
		stringRegMap.push_back(regmapEntryToString(*rmapit));
	}
	if(stringRegMap.size() == 0)
		stringRegMap.push_back("empty");

	//trigger the blit function so that the screen output is refreshed of the ROB content
	_ui->blitRegMapTable(&stringRegMap);
}

//===================================================================================================================
//							LOGIC FUNCTIONS
//===================================================================================================================


//RETURNS FALSE IF WE RUN OUT OF ACTIVE LIST ENTRIES. WILL NOT HAVE MAPPED traceline ON FALSE RETURN
bool InstDecodeStage::Decode(traceinstruction traceline)	//return success if ok, false if something went wrong
{
	int ActiveRegIndex = 0;
	RegMapKey ISARegToFind;
	ISARegToFind.Key = 0;

	//Cannot progress if we don't have any free machine registers.
	if(_FreeRegList->size() < 3)
	{
//TODO: BUG BUG! CERTAIN INSTRUCTIONS ONLY NEED ONE REGISTER!!
//TODO: Stall housekeeping, other housekeeping
//NO! STALL STUFF ETC SHOULD BE DONE IN CLOCK????
		return false;					//Cannot schedule an instruction, not enough FreeRegisters
	}

	//Cannot progress if the ROB is full.
	if(_pROB->isFull())
		return false;					//Cannot schedule an instruction, the ROB is full

	//We don't care about the ISA register mapping of the output register except for hazards
	//So, we will take a free reg from the FreeRegList to write output to first...
	ActiveRegIndex = _FreeRegList->front();	//Get an index to the oldest free reg from the FreeRegList
	_FreeRegList->pop();						//Remove the oldest free reg from the FreeRegList

	//Find the ISA register mapping for the instruction output register if it exists in the
	//RegMapTable
	ISARegToFind.ISAReg = traceline.rd;
	if(_RegMapTable->find(ISARegToFind.Key) != _RegMapTable->end())
	{
		//This ISA register has already been mapped!
		ISARegToFind.InstanceCounter = (*_ISAInstanceRegmap)[ISARegToFind.ISAReg];
		ISARegToFind.InstanceCounter++;
		(*_ISAInstanceRegmap)[ISARegToFind.ISAReg] = ISARegToFind.InstanceCounter;

		(*_RegMapTable)[ISARegToFind.Key] 	= ActiveRegIndex;
		traceline.m_rd.machineReg 			= ActiveRegIndex;				//Modify the input trace destination to write to the correct Active Register
		traceline.m_rd.ISAInstanceCounter 	= ISARegToFind.InstanceCounter;	//Keep a copy of the Key, you can use this in the ROB to remove this mapping
																			//From the register map

		//ISA register maps to an active register. oh crap, we cannot schedule it
		//And will have to stall
//TODO: stall mechanism
//NO! SHOULD BE DONE IN CLOCK
	}
	else
	{
		//Reserve a register normally in the RegMapTable
		(*_RegMapTable)[ISARegToFind.Key] = ActiveRegIndex;
		(*_ISAInstanceRegmap)[ISARegToFind.ISAReg] = 0;	//Always start counting at 0 in computer science ;)

		traceline.m_rd.machineReg 			= ActiveRegIndex;	//Modify the input trace destination to write to the correct Active Register
		traceline.m_rd.ISAInstanceCounter 	= 0;				//This was the 0'th time that this ISAReg
	}


//TODO: Should this be in the decode or execute stage? I believe we should stall in the
//Decode stage.
//TODO:
	//Determine if we will create a hazard by issuing this instruction
	//What type of inst is that instruction? how many cycles before it is safe to write this instruction?

	//Find the ISA register mapping for the rs operand in the RegMapTable
	ISARegToFind.Key = 0;	//clear.
	ISARegToFind.ISAReg = traceline.rs;
	if(_RegMapTable->find(ISARegToFind.Key) != _RegMapTable->end())
	{

		//That means that the operand can be found using the machine register that corresponds
		//To this ISA register. This instruction must wait for that register to be
		//Written to

		//It must get the latest instance of that ISA register though!
		ISARegToFind.InstanceCounter = (*_ISAInstanceRegmap)[ISARegToFind.ISAReg];

		traceline.m_rs = (*_RegMapTable)[ISARegToFind.Key];
		//rs operand is converted OK!
	}
	else
	{
		//if it is not in the RegMapTable???
		//You would copy from the real reg file into the active list.
		//That would not happen until later, but you would reserve one entry now (do that)

		//Remove an entry from the active list
		ActiveRegIndex = _FreeRegList->front();	//Get an index to the oldest free reg from the FreeRegList
		_FreeRegList->pop();						//Remove the oldest free reg from the FreeRegList

		//Reserve a register normally in the RegMapTable
		(*_RegMapTable)[ISARegToFind.Key] = ActiveRegIndex;
		(*_ISAInstanceRegmap)[ISARegToFind.ISAReg] = 0;				//If someone wants to write to this ISA reg, they better wait.

		traceline.m_rs = ActiveRegIndex;	//Modify the input trace destination to write to the correct Active Register
	}
	//Translate the operand register from ISA to Register in the ActiveList


	//Find the ISA register mapping for the rt operand in the RegMapTable
	ISARegToFind.Key = 0; //clear
	ISARegToFind.ISAReg = traceline.rt;
	if(_RegMapTable->find(ISARegToFind.Key) != _RegMapTable->end())
	{

		//That means that the operand can be found using the machine register that corresponds
		//To this ISA register. This instruction must wait for that register to be
		//Written to

		//It must get the latest instance of that ISA register though!
		ISARegToFind.InstanceCounter = (*_ISAInstanceRegmap)[ISARegToFind.ISAReg];

		traceline.m_rt = (*_RegMapTable)[ISARegToFind.Key];
		//rt operand is converted OK!
	}
	else
	{
		//if it is not in the RegMapTable???
		//You would copy from the real reg file into the active list.
		//That would not happen until later, but you would reserve one entry now (do that)

		//Remove an entry from the active list
		ActiveRegIndex = _FreeRegList->front();	//Get an index to the oldest free reg from the FreeRegList
		_FreeRegList->pop();						//Remove the oldest free reg from the FreeRegList

		//Reserve a register normally in the RegMapTable
		(*_RegMapTable)[ISARegToFind.Key] = ActiveRegIndex;
		(*_ISAInstanceRegmap)[ISARegToFind.ISAReg] = 0;				//If someone wants to write to this ISA reg, they better wait.

		traceline.m_rt = ActiveRegIndex;	//Modify the input trace destination to write to the correct Active Register
	}

	//Should now add the instruction to the Instruction queue and ROB.
	//This is in accordance with the R10k paper
//TODO: check and note why you did this

	if(traceline.intOp & (A | M))					//Put floating point ops on FP queue
	{
		if(!_pScheduler->isFPQueueFull())			//We can only add 3 instructions to the Scheduler per cycle. So, don't add more!
		{
			if(!_pScheduler->pushFPInstruction(traceline))
				return false;						//Something is horribly wrong with the Scheduler stage. We are dead.
			_pROB->addEntry(traceline);				//I do think this should be done here, but it is not indicated in the R10k paper
		}
		else
			return false;							//Something is wrong, perhaps this member was called in error?
	}
	if(traceline.intOp & (I | B))					//Put branches and integer ops on integer queue
	{
		if(!_pScheduler->isALUQueueFull())
		{
			if(! _pScheduler->pushALUInstruction(traceline))
				return false;						//Something is horribly wrong with the Scheduler stage. We are dead.
			_pROB->addEntry(traceline);
		}
		else
			return false;							//Something is wrong, perhaps this Decode stage should not have been called?
	}
	if(traceline.intOp & (L | S))					//Put memory ops on the Memory Queue
	{
		if(!_pScheduler->isLSQueueFull())
		{
			if(! _pScheduler->pushLSInstruction(traceline))
				return false;
			_pROB->addEntry(traceline);
		}
		else
			return false;							//Oh dear, something is logically wrong. We should never get here!
	}

	return true;
}

//=============================================================================================================================
//								UI RELATED FUNCTIONS
//=============================================================================================================================


string InstDecodeStage::regmapEntryToString(regmappair entry)
{
	string regmapEntryString;
	ostringstream os;
	RegMapKey entryKey;

	entryKey.Key = entry.first;	//Get the key from the key value pair. This has special mapping we need to descramble.

	os << "| cpy#: 0x" <<hex << entryKey.ISAInstanceCounter <<" | ISA: 0x" << hex << entryKey.ISAReg << " -> PHY: 0x" <<hex << entry.second;

	regmapEntryString = os.str();	//Assign the returned string this formatted output

	return regmapEntryString;		//Return a string formatted regmap table entry
}

}
