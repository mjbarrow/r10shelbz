/*
 * BranchResolver.cpp
 *
 *  Created on: Dec 16, 2014
 *      Author: king
 */

#include "BranchResolver.h"

#define cleantracevector(vecptr,copy)																\
			for(vector<traceinstruction>::iterator pstg = ((*vecptr)[copy]).begin(); pstg != ((*vecptr)[copy]).end(); pstg++	)	\
			{																					\
				if(pstg->traceLineNo == branch.traceLineNo)										\
				pstg->extra = 0;																\
			}

#define cleantraceQueuevector(vecptr,copy)																\
			for(vector<FPQueueEntry>::iterator pstg = ((*vecptr)[copy]).begin(); pstg != ((*vecptr)[copy]).end(); pstg++	)	\
			{																					\
				if(pstg->traceLineNo == branch.traceLineNo)										\
				pstg->extra = 0;																\
			}

namespace R10k {

//Saves system state. Called from instruction fetch
//void BranchResolver::detectBranch(	traceinstruction branch,
//									vector<traceinstruction> DecodeState,
//									long long int FetchState				)
void BranchResolver::detectBranch(traceinstruction branch)
{
//DEBUG BELOW A CRASHA?? NO
//	DecodeState.push_back(branch);	//Have the full instruction trace upto this point
									//logged

	//Now, save the machine state

	//Circular buffer, so has an edge case. ugh
	if(_branchBuffIndex == BRANCHBUFCOUNT)
		_branchBuffIndex = 1;

	(*_FreeRegList)[_branchBuffIndex] 				= (*_FreeRegList)[0];
	(*_r10kRegisterMap)[_branchBuffIndex] 			= (*_r10kRegisterMap)[0];
	(*_r10kRegMapDisambiguator)[_branchBuffIndex]	= (*_r10kRegMapDisambiguator)[0];
	(*_FPInstructionQueue)[_branchBuffIndex]		= (*_FPInstructionQueue)[0];
	(*_ALUInstructionQueue)[_branchBuffIndex]		= (*_ALUInstructionQueue)[0];
	(*_LSInstructionQueue)[_branchBuffIndex]		= (*_LSInstructionQueue)[0];
	(*_FPMpipe)[_branchBuffIndex]					= (*_FPMpipe)[0];
	(*_FPApipe)[_branchBuffIndex]					= (*_FPApipe)[0];
	(*_ALU1pipe)[_branchBuffIndex]					= (*_FPApipe)[0];
	(*_ALU2pipe)[_branchBuffIndex]					= (*_ALU2pipe)[0];
	(*_LS1pipe)[_branchBuffIndex]					= (*_LS1pipe)[0];

	_branchstackmap[_branchBuffIndex] = branch;		//This will be handy for control C

	//Clean out the copied register files from this not-taken branch
	//Fix up any lurker instances of branch to not have mispredict flag set
	cleantraceQueuevector(	_FPInstructionQueue,		//fixup the mispredict from here
							_branchBuffIndex		);	//Remove from this instance on the stack
	cleantraceQueuevector(	_ALUInstructionQueue,
							_branchBuffIndex		);
	cleantraceQueuevector(	_LSInstructionQueue,
							_branchBuffIndex		);
	cleantracevector(		_FPMpipe,				//clean FPM pipe
							_branchBuffIndex		);
	cleantracevector(		_FPApipe,
							_branchBuffIndex		);
	cleantracevector(		_ALU1pipe,
							_branchBuffIndex		);
	cleantracevector(		_ALU2pipe,
							_branchBuffIndex		);
	cleantracevector(		_LS1pipe,
							_branchBuffIndex		);

	_branchBuffIndex++;


//MOVE	_DecodeState[_branchBuffIndex] = DecodeState;	//When restoring the system state
//TO EX STAGE	_FetchState[_branchBuffIndex] = FetchState;	//This is where to resume on missed branch.
}

//Called from pipe when a mispredicted branch is found
void BranchResolver::mispredictRollback(traceinstruction branch)
{
	vector<FPQueue>::iterator it;	//use to remove inflight instructions
	int loop;

	for(loop=1; loop< BRANCHBUFCOUNT; loop++)
	{
		if(branch.traceLineNo == _branchstackmap[loop].traceLineNo)
			{
											//Pop the branch buffer
				_branchBuffIndex = loop;	//Found you! You little mispredicting devil!
				break;						//stop looking
			}
	}
	if(loop >= BRANCHBUFCOUNT && (branch.traceLineNo != _branchstackmap[BRANCHBUFCOUNT -1].traceLineNo))
		{cerr << "DANGER: UNABLE TO RESTORE SYSTEM AFTER MISPREDICT" << endl; return;}

	//Restore system state
//DONT ROLL BACK MAPS. INFLIGHT INSTRUCTIONS WILL TAKE CARE OF THEM	(*_FreeRegList)[0] 				= (*_FreeRegList)[_branchBuffIndex];
//	(*_r10kRegisterMap)[0] 			= (*_r10kRegisterMap)[_branchBuffIndex];
//	(*_r10kRegMapDisambiguator)[0]	= (*_r10kRegMapDisambiguator)[_branchBuffIndex];
	(*_FPInstructionQueue)[0]		= (*_FPInstructionQueue)[_branchBuffIndex];
	(*_ALUInstructionQueue)[0]		= (*_ALUInstructionQueue)[_branchBuffIndex];
	(*_LSInstructionQueue)[0]		= (*_LSInstructionQueue)[_branchBuffIndex];
//DONT ROLL BACK PIPES, LET THEM DO THEIR OWN CARETAKING (*_FPMpipe)[0]					= (*_FPMpipe)[_branchBuffIndex];
//DONT ROLL BACK PIPES, LET THEM DO THEIR OWN CARETAKING (*_FPMpipe)[0]		//(*_FPApipe)[0]					= (*_FPApipe)[_branchBuffIndex];
//DONT ROLL BACK PIPES, LET THEM DO THEIR OWN CARETAKING (*_FPMpipe)[0]		//(*_ALU1pipe)[0]					= (*_FPApipe)[_branchBuffIndex];
//DONT ROLL BACK PIPES, LET THEM DO THEIR OWN CARETAKING (*_FPMpipe)[0]		//(*_ALU2pipe)[0]					= (*_ALU2pipe)[_branchBuffIndex];
//DONT ROLL BACK PIPES, LET THEM DO THEIR OWN CARETAKING (*_FPMpipe)[0]		//(*_LS1pipe)[0]					= (*_LS1pipe)[_branchBuffIndex];
	//ROLL BACK Fetcher and decoder!

	//clean instruction queues of executed instructions
	for(	vector<FPQueueEntry>::iterator pstg = ((*_FPInstructionQueue)[0]).begin(); pstg != ((*_FPInstructionQueue)[0]).end(); pstg++										)
	{
		if(_pROB->hasEntryExecuted(*pstg))
			((*_FPInstructionQueue)[0]).erase(pstg);
	}

	for(	vector<FPQueueEntry>::iterator pstg = ((*_ALUInstructionQueue)[0]).begin(); pstg != ((*_ALUInstructionQueue)[0]).end(); pstg++										)
	{
		if(_pROB->hasEntryExecuted(*pstg))
			((*_ALUInstructionQueue)[0]).erase(pstg);
	}

	for(	vector<FPQueueEntry>::iterator pstg = ((*_LSInstructionQueue)[0]).begin(); pstg != ((*_LSInstructionQueue)[0]).end(); pstg++										)
	{
		if(_pROB->hasEntryExecuted(*pstg))
			((*_LSInstructionQueue)[0]).erase(pstg);
	}
}

/*
 * NEVER REFETCH! TRACE IS IN ORDER EXECUTED!
//Instruction fetch rollback helper (re-set file pointer in trace file to re-fetch instructions
//Or, go back in time
bool instFetchRollbackHelper()
{

	return false;
}
*/

} /* namespace R10k */
