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
//							Project required FUNCTIONS
//==================================================================================================================

//TODO swizzle registers
//Blit the appropriate widget
void InstDecodeStage::risingEdge()
{
	vector<string> stringRegMap;
	std::map<int,int>::iterator rmapit;
	regmappair a;

	if(*_stallInput)		//Do nothing if this stage should stall
		return;				//due to branch mispredict

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
		//Back up the register files if need be
		i++;
	}
	//Construct the correct type of input for the blit function
	//An array of strings which it will blit out
	for(rmapit = (*_RegMapTable)[0].begin(); rmapit != (*_RegMapTable)[0].end(); rmapit++)
	{
		stringRegMap.push_back(regmapEntryToString(*rmapit));
	}
	if(stringRegMap.size() == 0)
		stringRegMap.push_back("empty");

	//trigger the blit function so that the screen output is refreshed of the ROB content
	_ui->blitRegMapTable(&stringRegMap);
}

void InstDecodeStage::calc()
{
	if(*_stallInput)		//Do nothing if this stage should stall
		return;				//due to branch mispredict

	_traceLinesAccepted = 0;

	//BEHAVIOUR. PUSH Into THE QUEUES IN ORDER. IF
	//INSTRUCTION CANNOT BE PUSHED, STOP TRYING TO PUSH ANY SUBSEQUENT INSTRUCTION INTO QUEUES.

	//The instruction Decode width is 4 Try to decode
	while(_traceLinesAccepted < MAXDECODEDINSTPERCYCLE)
	{
		if(_QTraceLines[_traceLinesAccepted].intOp != BADOpcode)	//Dont do work on naff tracelines
			if(!Decode(_QTraceLines[_traceLinesAccepted]))			//Try to decode this trace line
				break;												//If we cannot, just give up.
		_traceLinesAccepted++;										//Track how many tracelines we were able
	}																//To decode
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
	if(	(traceline.intOp & ( I| A |M)) &&						//Return if you don't have 3
		(_FreeRegList->size() < 3)	)
	{
		return false;					//Cannot schedule an instruction, not enough FreeRegisters
	}
	if(((traceline.intOp) & (L | S | B )) &&	//These instructions only require 2  FreeRegisters max
		(_FreeRegList->size() < 2)	)
	{
		return false;					//Cannot schedule an instruction, not enough FreeRegisters
	}

	//Cannot progress if the ROB is full.
	if(_pROB->isFull())
		return false;					//Cannot schedule an instruction, the ROB is full

	//Find the ISA register mapping for the rs operand in the RegMapTable
	ISARegToFind.Key = 0;	//clear.
	ISARegToFind.ISAReg = traceline.rs;
	if((*_RegMapTable)[0].find(ISARegToFind.Key) != (*_RegMapTable)[0].end())
	{

		//That means that the operand can be found using the machine register that corresponds
		//To this ISA register. This instruction must wait for that register to be
		//Written to

		//It must get the latest instance of that ISA register though!
		ISARegToFind.InstanceCounter = ((*_ISAInstanceRegmap)[0])[ISARegToFind.ISAReg];

		traceline.m_rs = ((*_RegMapTable)[0])[ISARegToFind.Key];
		//rs operand is converted OK!
	}
	else
	{
		//if it is not in the RegMapTable???
		//You would copy from the real reg file into the active list.
		//That would not happen until later, but you would reserve one entry now (do that)

		//Remove an entry from the active list
		ActiveRegIndex = (*_FreeRegList)[0].front();	//Get an index to the oldest free reg from the FreeRegList
		(*_FreeRegList)[0].pop();						//Remove the oldest free reg from the FreeRegList

		//Reserve a register normally in the RegMapTable
		((*_RegMapTable)[0])[ISARegToFind.Key] = ActiveRegIndex;
		((*_ISAInstanceRegmap)[0])[ISARegToFind.ISAReg] = 0;				//If someone wants to write to this ISA reg, they better wait.

		traceline.m_rs = ActiveRegIndex;	//Modify the input trace destination to write to the correct Active Register
	}
	//Translate the operand register from ISA to Register in the ActiveList


	//Find the ISA register mapping for the rt operand in the RegMapTable
	ISARegToFind.Key = 0; //clear
	ISARegToFind.ISAReg = traceline.rt;
	if(traceline.rt != BADOperand)	//Certain classes of instruction (Load) do not have an rt register. rt mapping may be ignored for them
	{
		if((*_RegMapTable)[0].find(ISARegToFind.Key) != (*_RegMapTable)[0].end())
		{

			//That means that the operand can be found using the machine register that corresponds
			//To this ISA register. This instruction must wait for that register to be
			//Written to

			//It must get the latest instance of that ISA register though!
			ISARegToFind.InstanceCounter = ((*_ISAInstanceRegmap)[0])[ISARegToFind.ISAReg];

			traceline.m_rt = ((*_RegMapTable)[0])[ISARegToFind.Key];
			//rt operand is converted OK!
		}
		else
		{
			//if it is not in the RegMapTable???
			//You would copy from the real reg file into the active list.
			//That would not happen until later, but you would reserve one entry now (do that)

			//Remove an entry from the active list
			ActiveRegIndex = (*_FreeRegList)[0].front();	//Get an index to the oldest free reg from the FreeRegList
			(*_FreeRegList)[0].pop();						//Remove the oldest free reg from the FreeRegList

			//Reserve a register normally in the RegMapTable
			((*_RegMapTable)[0])[ISARegToFind.Key] = ActiveRegIndex;
			((*_ISAInstanceRegmap)[0])[ISARegToFind.ISAReg] = 0;				//If someone wants to write to this ISA reg, they better wait.

			traceline.m_rt = ActiveRegIndex;	//Modify the input trace destination to write to the correct Active Register
		}
	}
//DEBUG MOVED rd ASSIGN HERE TO AVOID CHICKEN AND EGG WHERE RD <- Rx

	//Dont map the ISA reg if it is BAD (some instructions do not modify any ISA registers
	if(ISARegToFind.ISAReg != BADOperand)
	{
		//We don't care about the ISA register mapping of the output register except for hazards
		//So, we will take a free reg from the FreeRegList to write output to first...
		ActiveRegIndex = (*_FreeRegList)[0].front();	//Get an index to the oldest free reg from the FreeRegList
		(*_FreeRegList)[0].pop();						//Remove the oldest free reg from the FreeRegList

		//Find the ISA register mapping for the instruction output register if it exists in the
		//RegMapTable
		ISARegToFind.ISAReg = traceline.rd;

		//Begin, rd exists so trace output register map is required
		if((*_RegMapTable)[0].find(ISARegToFind.Key) != (*_RegMapTable)[0].end())
		{
			//This ISA register has already been mapped!
			ISARegToFind.InstanceCounter = ((*_ISAInstanceRegmap)[0])[ISARegToFind.ISAReg];
			ISARegToFind.InstanceCounter++;
			((*_ISAInstanceRegmap)[0])[ISARegToFind.ISAReg] = ISARegToFind.InstanceCounter;

			((*_RegMapTable)[0])[ISARegToFind.Key] 	= ActiveRegIndex;
			traceline.m_rd.machineReg 			= ActiveRegIndex;				//Modify the input trace destination to write to the correct Active Register
			traceline.m_rd.ISAInstanceCounter 	= ISARegToFind.InstanceCounter;	//Keep a copy of the Key, you can use this in the ROB to remove this mapping
																				//From the register map

			//ISA register maps to an active register. oh crap, we cannot schedule it
			//And will have to stall
		}
		else
		{
			//Reserve a register normally in the RegMapTable
			((*_RegMapTable)[0])[ISARegToFind.Key] = ActiveRegIndex;
			((*_ISAInstanceRegmap)[0])[ISARegToFind.ISAReg] = 0;	//Always start counting at 0 in computer science ;)

			traceline.m_rd.machineReg 			= ActiveRegIndex;	//Modify the input trace destination to write to the correct Active Register
			traceline.m_rd.ISAInstanceCounter 	= 0;				//This was the 0'th time that this ISAReg
		}	//End, rd exists so trace output register map is required
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
//TODO Support branch resolution
	if(traceline.intOp == B)
		cerr << "ERROR! CANNOT HANDLE BRANCH INSTRUCTION!";

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
