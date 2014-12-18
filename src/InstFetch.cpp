/*
 * InstFetch.cpp
 *
 *  Created on: Dec 7, 2014
 *      Author: king
 */

#include "InstFetch.h"

namespace R10k {


void InstFetchStage::risingEdge()
{
	vector<string> stringInstList;
	unsigned int i;
	//Construct the correct type of input for the blit function
	//An array of strings which it will blit out
	for(i = 0; i < instructionstream.size(); i++)
	{
		stringInstList.push_back(traceInstructionToString(i));
	}
	if(stringInstList.size() == 0)
		stringInstList.push_back("empty");

	//trigger the blit function so that the screen output is refreshed of the ROB content
	_ui->blitInstructionList(&stringInstList);//blitROBList(&stringROB);

	//Clear all of the instruction stream for next time
	instructionstream = vector<traceinstruction>();	//CLEAN UP THE INSTRUCTION STREAM
}

//Branch mispredict rollback
void InstFetchStage::fallingEdge()
{

}

void InstFetchStage::calc()
{
	int loop = INSTDECODEPERCYCLE;
	std::string traceline;
	traceinstruction currentInstruction;

//take in feedback from the decode stage or whatever to limit the number of
//Instructions to pull in
	loop -= _pdecstage->getLinesAccepted();

	//Resolve any mispredicted branches
	if(		(loop < INSTDECODEPERCYCLE) && 			//possible to issue an instruction
			(_failedBranch.extra !=0)		)		//Should re-execute a mispredicted branch
	{
		instructionstream.push_back(currentInstruction);	//Present branch for execution
		_plogger->logIFTrace(_tracelinenumber);				//Log the instruction fetch for the pipeline diagram

		loop++;
		_tracelinenumber++;

		_failedBranch.extra = 0;	//Never re-do this instruction again
	}

	//Normal instruction decode flow
	while (loop < INSTDECODEPERCYCLE)
	{
		if(!getline(fulltrace, traceline))
			break;

		currentInstruction = traceinstruction(traceline,_tracelinenumber);
		//Check if this is a failed branch. if so, we will need to keep a file pointer
		//and fixed up branch for when the simulator rolls back.
		if(		(currentInstruction.intOp == B) && 			//This is a branch
				(currentInstruction.extra != 0) && 			//It will mispredict
				(currentInstruction.traceLineNo < _failedBranch.traceLineNo)		)	//We have no earlier mispredict logged
		{
		//	_failedBranch 			= currentInstruction;
		//	_failedBranch.extra 	= 0;					//It will be right next time for sure
		//	_failedPlusOneFilePos 	= fulltrace.tellg();	//File position of next instruction
	//		_branchAddQ.push(fulltrace.tellg());			//File position of next instruction from branch.

//SHOULD USE ANOTHER FUNCTION
			_BrUnit->logBranchAddress(fulltrace.tellg(),currentInstruction);
//			_BrUnit->detectBranch(	currentInstruction,
//									instructionstream,	//Useless, these are old instructions!
//									fulltrace.tellg()		);	//Take a system snapshot
														//This saves a copy of:
														//reg map
														//other stuff
	//TODO this will loose registers, because present in-flight instructions will be lost and won't
	//Unmap!
		}

		instructionstream.push_back(currentInstruction);
		_plogger->logIFTrace(_tracelinenumber);				//Log the instruction fetch for the pipeline diagram

		loop++;
		_tracelinenumber++;
	}
	_pdecstage->setDTraceLines(&instructionstream);

}

//==================================================================================================
//										UTILITY FUNCTIONS
//==================================================================================================
//Convert the buffer line into a string. Usefull for debug.
//See: http://stackoverflow.com/questions/3203452/how-to-read-entire-stream-into-a-stdstring
string InstFetchStage::traceInstructionToString(int entry)
{
	string instructionString;
	ostringstream os;
	traceinstruction instruction = instructionstream[entry];


	os 		<< "|tr#:" << instruction.traceLineNo
			<<hex<<"| 0x"<< instruction.rd << " <- " <<hex<<"0x"<< instruction.rs << " "+instruction.strOp+" isa reg: " <<hex<<"0x"<< instruction.rt
			<< "|";

	instructionString = os.str();

	return instructionString;
}


} /* namespace R10k */
