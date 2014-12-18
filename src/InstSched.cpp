/*
 * InstSched.cpp
 *
 *  Created on: Dec 3, 2014
 *      Author: king
 */

#include "InstSched.h"

namespace R10k {

//====================================================================================================
//								PROJECT SPEC FUNCTIONS
//====================================================================================================
//Project spec function
void InstSchedStage::risingEdge()
{
	vector<string> stringFPInsQueue;
	vector<string> stringALUInsQueue;
	vector<string> stringLSInsQueue;
	unsigned int i;

	if(*_pstallInput)		//If the pipe has stalled
		return;				//Do nothing

	//DO register swizzle By reading in the input ports to this stage
	//Input ports are set using "pushFPInstruction()" by a connected driving stage "calc()" method.

	//Construct the correct type of input for the blit function
	//An array of strings which it will blit out
	for(i = 0; i < ((*_FPInstructionQueue)[0]).size(); i++)
	{
		stringFPInsQueue.push_back(FPQueueEntryToString(i));
	}
	if(stringFPInsQueue.size() == 0)
		stringFPInsQueue.push_back("empty");

	for(i = 0; i < ((*_ALUInstructionQueue)[0]).size(); i++)
	{
		stringALUInsQueue.push_back(ALUQueueEntryToString(i));
	}
	if(stringALUInsQueue.size() == 0)
		stringALUInsQueue.push_back("empty");

	for(i = 0; i < ((*_LSInstructionQueue)[0]).size(); i++)
	{
		stringLSInsQueue.push_back(LSQueueEntryToString(i));
	}
	if(stringLSInsQueue.size() == 0)
		stringLSInsQueue.push_back("empty");

	//trigger the blit function so that the screen output is refreshed of the queues content
	//Blit all queues
	_ui->blitFPQueueList(		&stringFPInsQueue	);
	_ui->blitIntegerQueueList(	&stringALUInsQueue	);
	_ui->blitAddressQueueList(	&stringLSInsQueue	);

}

//Check if the execution pipe found a mispredict, if so purge the output
void InstSchedStage::fallingEdge()
{
	unsigned int i;

	if(*_pstallInput)		//If the pipe has stalled
		return;				//Do nothing

/*
	traceinstruction LastBr = _pPipes->didBranchMispredict();

	//Start purge pipe logic
	if(LastBr.intOp != BADOpcode)
	{
		//Purge output pipes
		//Check all input ports and if any port has an instruction, add it to the
		//FP instruction queue to be executed when dependencies are resolved
		for(i = 0; i < FPQPORTCOUNT; i++)
		{
			if(	(_DFPQueue[_DFPQidx].intOp != BADOpcode) &&					//There was a misspredicted branch
				(_DFPQueue[_DFPQidx].traceLineNo > LastBr.traceLineNo)	)	//This instruction is younger than that branch
			{
				(_DFPQueue[_DFPQidx] = FPQueueEntry());	//Cancel schedule of this instruction
			}
			_DFPQidx++;
			if(_DFPQidx == FPQPORTCOUNT)	//Index has to wrap around so we end this risingEdge having
				_DFPQidx = 0;			//Not affected the position of the input port index
		}

		for(i = 0; i < ALUQPORTCOUNT; i++)
		{
			if(	(_DALUQueue[_DALUQidx].intOp != BADOpcode) &&				//There was a mispredict
				(_DALUQueue[_DALUQidx].traceLineNo > LastBr.traceLineNo))	//This scheduled instruction is younger than that branch
			{
				(_DALUQueue[_DALUQidx] = ALUQueueEntry());	//Cancel schedule of this instruction
			}
			_DALUQidx++;

			if(_DALUQidx == ALUQPORTCOUNT)	//Index has to wrap around so we end this risingEdge having
				_DALUQidx = 0;				//Not affected the position of the input port index
		}

		for(i = 0; i < ADDQPORTCOUNT; i++)
		{
			if(	(_DLSQueue[_DLSQidx].intOp != BADOpcode)	&&
				(_DLSQueue[_DLSQidx].traceLineNo > LastBr.traceLineNo))
			{
				_DLSQueue[_DLSQidx] = ALUQueueEntry();	//Add something to the execution pipes if this port has data
			}

			_DLSQidx++;

			if(_DLSQidx == ALUQPORTCOUNT)	//Index has to wrap around so we end this risingEdge having
				_DLSQidx = 0;	//Not affected the position of the input port index
		}
	}
	//End purge pipe logic
*/
	//Begin Enqueue
	//Check all input ports and if any port has an instruction, add it to the
	//FP instruction queue to be executed when dependencies are resolved
	for(i = 0; i < FPQPORTCOUNT; i++)
	{
		if(_DFPQueue[_DFPQidx].intOp != BADOpcode)
		{
			((*_FPInstructionQueue)[0]).push_back(_DFPQueue[_DFPQidx]);		//Add something to the execution pipes if this port has data
			_plogger->logISTrace(_DFPQueue[_DFPQidx].traceLineNo); 	//Add scheduled instruction to the pipeline diagram
		}

		_DFPQueue[_DFPQidx] = FPQueueEntry();	//Clear the port in case the connected stage does not set it next cycle
		_DFPQidx++;

		if(_DFPQidx == FPQPORTCOUNT)	//Index has to wrap around so we end this risingEdge having
			_DFPQidx = 0;	//Not affected the position of the input port index
	}

	for(i = 0; i < ALUQPORTCOUNT; i++)
	{
		if(_DALUQueue[_DALUQidx].intOp != BADOpcode)
		{
			((*_ALUInstructionQueue)[0]).push_back(_DALUQueue[_DALUQidx]);	//Add something to the execution pipes if this port has data
			_plogger->logISTrace(_DALUQueue[_DALUQidx].traceLineNo);//Add to the pipeline diagram too.
		}

		_DALUQueue[_DALUQidx] = ALUQueueEntry();	//Clear the port in case the connected stage does not set it next cycle
		_DALUQidx++;

		if(_DALUQidx == ALUQPORTCOUNT)	//Index has to wrap around so we end this risingEdge having
			_DALUQidx = 0;	//Not affected the position of the input port index
	}

	for(i = 0; i < ADDQPORTCOUNT; i++)
	{
		if(_DLSQueue[_DLSQidx].intOp != BADOpcode)
		{
			((*_LSInstructionQueue)[0]).push_back(_DLSQueue[_DLSQidx]);	//Add something to the execution pipes if this port has data
			_plogger->logISTrace(_DLSQueue[_DLSQidx].traceLineNo);//And add to the pipeline diagram
		}

		_DLSQueue[_DLSQidx] = LSQueueEntry();	//Clear the port in case the connected stage does not set it next cycle
		_DLSQidx++;

		if(_DLSQidx == ALUQPORTCOUNT)	//Index has to wrap around so we end this risingEdge having
			_DLSQidx = 0;	//Not affected the position of the input port index
	}
	//End enqueue
}

//=====================================================================================================
//						PROMOTE FROM ALL QUEUES TO THE PIPES (SCHEDULE EXECUTION)
//=====================================================================================================


void InstSchedStage::calc()//promoQueueToPipe()
{
	if(*_pstallInput)		//If the pipe has stalled
		return;				//Do nothing

	//First resolve all dependencies
	_resolveSchedDependencies();

	//Now schedule from all available instructions (without pending dependencies)

	//prepare the _scheduledInstructions (without pending dependencies), which will be fed into the pipe.
	_arbitrateQueues();

//NOW CHUCK _scheduledInstructions at the pipe
	if(_scheduledInstructions.scheduledFPM <SCHEDQUEUESIZE)//!= _FPInstructionQueue.end())
	{
		_pPipes->FPMinPort(((*_FPInstructionQueue)[0])[_scheduledInstructions.scheduledFPM]);
		((*_FPInstructionQueue)[0]).erase(((*_FPInstructionQueue)[0]).begin() + _scheduledInstructions.scheduledFPM);
	}
	else
		{_pPipes->FPMinPort(traceinstruction());}
	if(_scheduledInstructions.scheduledFPA <SCHEDQUEUESIZE)//!= _FPInstructionQueue.end())
	{
		_pPipes->FPAinPort(((*_FPInstructionQueue)[0])[_scheduledInstructions.scheduledFPA]);
		((*_FPInstructionQueue)[0]).erase(((*_FPInstructionQueue)[0]).begin() + _scheduledInstructions.scheduledFPA);
	}
	else
		{_pPipes->FPAinPort(traceinstruction());}
	if(_scheduledInstructions.scheduledALU1 <SCHEDQUEUESIZE)//!= _ALUInstructionQueue.end())
	{
		_pPipes->ALUAinPort(((*_ALUInstructionQueue)[0])[_scheduledInstructions.scheduledALU1]);
		((*_ALUInstructionQueue)[0]).erase(((*_ALUInstructionQueue)[0]).begin() +  _scheduledInstructions.scheduledALU1);
	}
	else
		{_pPipes->ALUAinPort(traceinstruction());}
	if(_scheduledInstructions.scheduledALU2 <SCHEDQUEUESIZE)//!= _ALUInstructionQueue.end())
	{
		_pPipes->ALUBinPort(((*_ALUInstructionQueue)[0])[_scheduledInstructions.scheduledALU2]);
		((*_ALUInstructionQueue)[0]).erase(((*_ALUInstructionQueue)[0]).begin() +  _scheduledInstructions.scheduledALU2);
	}
	else
		{_pPipes->ALUBinPort(traceinstruction());}
	if(_scheduledInstructions.scheduledLS1 <SCHEDQUEUESIZE)//!= _LSInstructionQueue.end())
	{
		_pPipes->LSAinPort(((*_LSInstructionQueue)[0])[_scheduledInstructions.scheduledLS1]);
		((*_LSInstructionQueue)[0]).erase(((*_LSInstructionQueue)[0]).begin() +  _scheduledInstructions.scheduledLS1);
	}
	else
		{_pPipes->LSAinPort(traceinstruction());}

	//Clear all scheduled iterators.
	_scheduledInstructions.scheduledFPM = 	SCHEDQUEUESIZE;
	_scheduledInstructions.scheduledFPA = 	SCHEDQUEUESIZE;//_FPInstructionQueue.end();
	_scheduledInstructions.scheduledALU1 = 	SCHEDQUEUESIZE;//_ALUInstructionQueue.end();
	_scheduledInstructions.scheduledALU2 = 	SCHEDQUEUESIZE;//_ALUInstructionQueue.end();
	_scheduledInstructions.scheduledLS1 = 	SCHEDQUEUESIZE;//_LSInstructionQueue.end();

}

//==============================================================================================
//								FIX DEPENDENCIES FOR ALL QUEUE TYPES
//==============================================================================================


//Try to schedule an FP instruction
void InstSchedStage::_readyFPQueue()
{
	for(	FPQueueEntryiterator qitr = ((*_FPInstructionQueue)[0]).begin();
			qitr != ((*_FPInstructionQueue)[0]).end();
			qitr++																)
	{
//TODO: ADD IN CALLS TO PIPE FORWARDING STAGES AND DEBUG SIMPLE!!!!
		if(! (*qitr).m_rs_rdy)						//Attempt to meet dependency for rs operand
		{
			//Search ROB for m_rs
			if(_ROB->isDependencyMet((*qitr).m_rs))					//Check ROB
				(*qitr).m_rs_rdy = true;
			else if(_pPipes->FPAforwardAvailable((*qitr).m_rs))		//Check FP
				(*qitr).m_rs_rdy = true;
			else if(_pPipes->FPMforwardAvailable((*qitr).m_rs))
				(*qitr).m_rs_rdy = true;
			else if(_pPipes->ALU1forwardAvailable(qitr->m_rs))		//Check ALU
				qitr->m_rs_rdy = true;
			else if(_pPipes->ALU2forwardAvailable(qitr->m_rs))
				qitr->m_rs_rdy = true;
			else if(_pPipes->LS1RegforwardAvailable(qitr->m_rs))	//Check LS
				qitr->m_rs_rdy = true;
		}
		if(! (*qitr).m_rt_rdy)
		{
			//Search ROB for m_rt
			if(_ROB->isDependencyMet((*qitr).m_rt))					//Check ROB
				(*qitr).m_rt_rdy = true;
			else if(_pPipes->FPAforwardAvailable((*qitr).m_rt))		//Check FP
				(*qitr).m_rt_rdy = true;
			else if(_pPipes->FPMforwardAvailable((*qitr).m_rt))
				(*qitr).m_rt_rdy = true;
			else if(_pPipes->ALU1forwardAvailable(qitr->m_rt))		//Check ALU
				qitr->m_rt_rdy = true;
			else if(_pPipes->ALU2forwardAvailable(qitr->m_rt))
				qitr->m_rt_rdy = true;
			else if(_pPipes->LS1RegforwardAvailable(qitr->m_rt))	//Check LS
				qitr->m_rt_rdy = true;
		}
	}
}

void InstSchedStage::_readyALUQueue()
{
	for(	ALUQueueEntryiterator qitr = ((*_ALUInstructionQueue)[0]).begin();
			qitr != ((*_ALUInstructionQueue)[0]).end();
			qitr++																)
	{
		if(! (*qitr).m_rs_rdy)						//Attempt to meet dependency for rs operand
		{
			//Save state for branch mispredict recovery
			if(qitr->intOp == B)
				_pBrUnit->detectBranch(*qitr);	//Do this here because we need all younger instructions re-done.

			//Search ROB for m_rs
			if(_ROB->isDependencyMet(qitr->m_rs))					//Check ROB
				qitr->m_rs_rdy = true;
			else if(_pPipes->ALU1forwardAvailable(qitr->m_rs))		//Check ALU
				qitr->m_rs_rdy = true;
			else if(_pPipes->ALU2forwardAvailable(qitr->m_rs))
				qitr->m_rs_rdy = true;
			else if(_pPipes->FPAforwardAvailable((*qitr).m_rs))		//Check FP
				(*qitr).m_rs_rdy = true;
			else if(_pPipes->FPMforwardAvailable((*qitr).m_rs))
				(*qitr).m_rs_rdy = true;
			else if(_pPipes->LS1RegforwardAvailable(qitr->m_rs))	//Check LS
				qitr->m_rs_rdy = true;
		}
		if(! (*qitr).m_rt_rdy)
		{
			//Search ROB for m_rt
			if(_ROB->isDependencyMet((*qitr).m_rt))
				qitr->m_rt_rdy = true;
			else if(_pPipes->ALU1forwardAvailable(qitr->m_rt))
				qitr->m_rt_rdy = true;
			else if(_pPipes->ALU2forwardAvailable(qitr->m_rt))
				qitr->m_rt_rdy = true;
			else if(_pPipes->FPAforwardAvailable((*qitr).m_rt))		//Check FP
				(*qitr).m_rt_rdy = true;
			else if(_pPipes->FPMforwardAvailable((*qitr).m_rt))
				(*qitr).m_rt_rdy = true;
			else if(_pPipes->LS1RegforwardAvailable(qitr->m_rt))	//Check LS
				qitr->m_rt_rdy = true;
		}
	}
}



void InstSchedStage::_readyLSQueue()	//Must perform Memory disambiguation for Store instructions.
{
	for(	LSQueueEntryiterator qitr = ((*_LSInstructionQueue)[0]).begin();
			qitr != ((*_LSInstructionQueue)[0]).end();
			qitr++																)
	{
		if(! (*qitr).m_rs_rdy)						//Attempt to meet dependency for rs operand
		{
			//Search ROB for m_rs
			if(_ROB->isDependencyMet(qitr->m_rs))	//Memory disambiguation version
				{qitr->m_rs_rdy = true;}
			else if(_pPipes->ALU1forwardAvailable(qitr->m_rs))		//Check ALUS
				qitr->m_rs_rdy = true;
			else if(_pPipes->ALU2forwardAvailable(qitr->m_rs))
				qitr->m_rs_rdy = true;
			else if(_pPipes->FPAforwardAvailable(qitr->m_rs))		//Check floats
				qitr->m_rs_rdy = true;
			else if(_pPipes->FPMforwardAvailable((*qitr).m_rs))
				qitr->m_rs_rdy = true;
			else if(_pPipes->LS1RegforwardAvailable(qitr->m_rs))	//Check LS
				qitr->m_rs_rdy = true;
		}
		if(! (*qitr).m_rt_rdy)
		{
			//Search ROB for m_rt
			if(_ROB->isDependencyMet(qitr->m_rt))	//Memory disambguation version
				{qitr->m_rt_rdy = true;}
			else if(_pPipes->ALU1forwardAvailable(qitr->m_rt))
				qitr->m_rt_rdy = true;
			else if(_pPipes->ALU2forwardAvailable(qitr->m_rt))
				qitr->m_rt_rdy = true;
			else if(_pPipes->FPAforwardAvailable(qitr->m_rt))
				qitr->m_rt_rdy = true;
			else if(_pPipes->FPMforwardAvailable(qitr->m_rt))
				qitr->m_rt_rdy = true;
			else if(_pPipes->LS1RegforwardAvailable(qitr->m_rt))
				qitr->m_rt_rdy = true;
		}
		if((! qitr->add_rdy) && (qitr->intOp == S || qitr->intOp == L))	//Doing Shengye's logic. Memory system stalls everything
		{
			//Search ROB for address dependency (Avoid WAR)
//TODO ROB VERSION ALWAYS FAILS
			if(_ROB->isAddressDependencyMet(qitr->extra, qitr->traceLineNo, qitr->intOp))
				qitr->add_rdy = true;
			if(_pPipes->LS1AddforwardAvailable(qitr->extra))
				qitr->add_rdy = true;
		}
//		if(qitr->intOp == L)
//			qitr->add_rdy = true;					//Load instructions do not have to wait for anything other than registers
	}
}

//=====================================================================================================
//						ADD A NEW INSTRUCTION TO ONE OF THE THREE QUEUE TYPES
//=====================================================================================================

bool InstSchedStage::pushFPInstruction(traceinstruction i)
{
	FPQueueEntry _i(&i);

	if(((*_FPInstructionQueue)[0]).size() >= SCHEDQUEUESIZE)
		return false;

	_DFPQueue[_DFPQidx] = _i;	//There are two entries here. just fill which ever you feel like
	_DFPQidx++;
	if(_DFPQidx == FPQPORTCOUNT)
		_DFPQidx = 0;

	return true;	//Success
}

bool InstSchedStage::pushALUInstruction(traceinstruction i)
{
	ALUQueueEntry _i(&i);

	if(((*_ALUInstructionQueue)[0]).size() >= SCHEDQUEUESIZE)
		return false;

	_DALUQueue[_DALUQidx] = _i;
	_DALUQidx++;
	if(_DALUQidx == ALUQPORTCOUNT)
		_DALUQidx = 0;

	return true;	//Success
}

bool InstSchedStage::pushLSInstruction(traceinstruction i)
{
	LSQueueEntry _i(&i);

	if(((*_LSInstructionQueue)[0]).size() >= SCHEDQUEUESIZE)
		return false;

	_DLSQueue[_DLSQidx] = _i;					//Easier than the others :D
	_DLSQidx++;
	if(_DLSQidx == ADDQPORTCOUNT)
		_DLSQidx = 0;

	return true;	//Success
}

//=============================================================================================
//							ARBITRATE READY INSTRUCTIONS FROM ALL QUEUES
//=============================================================================================

//Prepare _scheduledInstructions for this cycle by calling the arbitration logic for each
//Instruction queue.
void InstSchedStage::_arbitrateQueues()
{
	_getFirstFPMulandAdd(	&_scheduledInstructions.scheduledFPM,
							&_scheduledInstructions.scheduledFPA	);
	_getFirsttwoALU(		&_scheduledInstructions.scheduledALU1,
							&_scheduledInstructions.scheduledALU2	);
	_getFirstLS(			&_scheduledInstructions.scheduledLS1	);
}

//R10K arbitrates by the head of the queue, not the age of instruction. _getOldestF
void InstSchedStage::_getFirstFPMulandAdd(int* Mulidx, int* Addidx)//FPQueueEntryiterator* Mulidx, FPQueueEntryiterator* Addidx)
{
#ifdef ARBITRATE_BY_QUEUE_POSITION
	bool firstMulFound = false;
	bool firstAddFound = false;
	int loop = 0;
	FPQueueEntryiterator qitr = ((*_FPInstructionQueue)[0]).begin();

	*Mulidx = SCHEDQUEUESIZE;//_FPInstructionQueue.end();
	*Addidx = SCHEDQUEUESIZE;//_FPInstructionQueue.end();

	//Acts as a FIFO in the respect that the first entry of each type is always chosen first
	for(loop = 0; loop < SCHEDQUEUESIZE; loop++)
	{
		if(qitr == ((*_FPInstructionQueue)[0]).end())
			break;

		//First multiply logic
		if(	(!firstMulFound) 	&&		//Do we need the first multiply?
			(qitr->intOp == M)&& 		//is this actually a multiply?
			(*qitr).m_rs_rdy 	&& 		//Is this operand ready?
			(*qitr).m_rt_rdy		)	//Is the other operand ready?
		{
			*Mulidx = loop; firstMulFound = true;	//If all above are true, thanks very much this is the first to schedule :)
		}
		//First add logic
		if(	(!firstAddFound)	&&		//Do we need the first add?
			((*qitr).intOp == A)&& 		//Is this actually an add?
			(*qitr).m_rs_rdy 	&&
			(*qitr).m_rt_rdy		)
		{
			*Addidx = loop; firstAddFound = true;	//If all above are true, thanks very much this is the first to schedule :)
		}
		qitr++;
	}
	return;
#endif

}

void InstSchedStage::_getFirsttwoALU(int* ALU1idx, int* ALU2idx)//ALUQueueEntryiterator* ALU1idx, ALUQueueEntryiterator* ALU2idx)
{
#ifdef ARBITRATE_BY_QUEUE_POSITION
	bool firstALUFound = false;
	bool secondALUFound = false;
	int loop;
	int branchloop;
	ALUQueueEntryiterator qitr = ((*_ALUInstructionQueue)[0]).begin();
	ALUQueueEntryiterator chkitr;

	*ALU1idx = SCHEDQUEUESIZE;
	*ALU2idx = SCHEDQUEUESIZE;

	//Acts as a FIFO in the respect that the first entry of each type is always chosen first															)
	for(loop = 0; loop < SCHEDQUEUESIZE; loop++)
	{
		//An exception condition. Somehow the instruction queue got too small
		if(qitr == ((*_ALUInstructionQueue)[0]).end())
			{cerr << "DANGER! Unexpected term\n"; break;}

		if(	(!firstALUFound) &&
			(*qitr).m_rs_rdy &&
			(*qitr).m_rt_rdy	)
		{
			//by defualt we will keep this branch instruction for execution
			*ALU1idx = loop;
			firstALUFound = true;

			if(qitr->intOp == B)	//Branch only case (ensure oldest branch)
			{
				//Be sure this is the earliest branch (strict in order)
				chkitr = ((*_ALUInstructionQueue)[0]).begin();
				for(	branchloop = 0;		//Make sure this is oldest
						branchloop < SCHEDQUEUESIZE; 		//ready branch
						branchloop++										)
				{
					//An exception condition. Somehow the instruction got too small
					if(chkitr == ((*_ALUInstructionQueue)[0]).end())
						{cerr << "DANGER! Unexpected term\n"; break;}

					if(qitr->traceLineNo > chkitr->traceLineNo)
						{*ALU1idx = loop; firstALUFound = true;}
					chkitr++;
				}
			}
		}
		//First add logic
		if(	(loop != *ALU1idx ) &&
			(!secondALUFound)&&
			(*qitr).m_rs_rdy &&
			(*qitr).m_rt_rdy	)
		{
			//Hold back parallel branches
			if(	(qitr->intOp == B) 						&& 				//If this is a branch
				(((*_ALUInstructionQueue)[0])[*ALU1idx].intOp != B) )	//and it is the first branch
			{
				//Be sure this is the earliest branch (strict in order)
				chkitr = ((*_ALUInstructionQueue)[0]).begin();
				for(	branchloop = 0;		//Make sure this is oldest
						branchloop < SCHEDQUEUESIZE; 		//ready branch
						branchloop++										)
				{
					//An exception condition. Somehow the instruction got too small
					if(chkitr == ((*_ALUInstructionQueue)[0]).end())
						{cerr << "DANGER! Unexpected term\n"; break;}

					if(qitr->traceLineNo > chkitr->traceLineNo)
						{*ALU2idx = loop; secondALUFound = true;}
					chkitr++;
				}
			}
			//Don't want the second operation to be assigned if the above test didn't work out.
			if(	(qitr->intOp != B) 	)
				{*ALU2idx = loop; secondALUFound = true;}	//may only take instruction if not
		}													//A branch.
		qitr++;
	}
	return;
#endif

}

//LS ALWAYS RETURNS THE OLDEST TRACE
void InstSchedStage::_getFirstLS(int* LSidx)//LSQueueEntryiterator* LSidx)
{
	int loop;
	*LSidx = SCHEDQUEUESIZE;
	traceline oldest = 0x7fffffff;
	LSQueueEntryiterator qitr = ((*_LSInstructionQueue)[0]).begin();

//	for(	LSQueueEntryiterator qitr = _LSInstructionQueue.begin();
//			qitr != _LSInstructionQueue.end();
//			qitr++																)
	for(loop = 0; loop < SCHEDQUEUESIZE; loop++)
	{
		if(qitr == ((*_LSInstructionQueue)[0]).end())
			break;
//BUG BUG BECAUSE S DOES NOT HAVE A	rt
		if(	(qitr->traceLineNo < oldest)	&&
			qitr->m_rs_rdy					&&
			qitr->m_rt_rdy					&&
			qitr->add_rdy						)
		{
			//*LSidx = qitr;
			*LSidx = loop;
			oldest = (*qitr).traceLineNo;
		}
		qitr++;
	}
	return;
}





} /* namespace R10k */
