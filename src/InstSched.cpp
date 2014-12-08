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

	//DO register swizzle By reading in the input ports to this stage
	//Input ports are set using "pushFPInstruction()" by a connected driving stage "calc()" method.

	//Check all input ports and if any port has an instruction, add it to the
	//FP instruction queue to be executed when dependencies are resolved
	for(i = 0; i < FPQPORTCOUNT; i++)
	{
		if(_DFPQueue[_DFPQidx].intOp != BADOpcode)
		{
			_FPInstructionQueue.push_back(_DFPQueue[_DFPQidx]);		//Add something to the execution pipes if this port has data
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
			_ALUInstructionQueue.push_back(_DALUQueue[_DALUQidx]);	//Add something to the execution pipes if this port has data
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
			_LSInstructionQueue.push_back(_DLSQueue[_DLSQidx]);	//Add something to the execution pipes if this port has data
			_plogger->logISTrace(_DLSQueue[_DLSQidx].traceLineNo);//And add to the pipeline diagram
		}

		_DLSQueue[_DLSQidx] = LSQueueEntry();	//Clear the port in case the connected stage does not set it next cycle
		_DLSQidx++;

		if(_DLSQidx == ALUQPORTCOUNT)	//Index has to wrap around so we end this risingEdge having
			_DLSQidx = 0;	//Not affected the position of the input port index
	}

	//Construct the correct type of input for the blit function
	//An array of strings which it will blit out
	for(i = 0; i < _FPInstructionQueue.size(); i++)
	{
		stringFPInsQueue.push_back(FPQueueEntryToString(i));
	}
	if(stringFPInsQueue.size() == 0)
		stringFPInsQueue.push_back("empty");

	for(i = 0; i < _ALUInstructionQueue.size(); i++)
	{
		stringALUInsQueue.push_back(ALUQueueEntryToString(i));
	}
	if(stringALUInsQueue.size() == 0)
		stringALUInsQueue.push_back("empty");

	for(i = 0; i < _LSInstructionQueue.size(); i++)
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

//=====================================================================================================
//						PROMOTE FROM ALL QUEUES TO THE PIPES (SCHEDULE EXECUTION)
//=====================================================================================================


void InstSchedStage::calc()//promoQueueToPipe()
{
	//First resolve all dependencies
	_resolveSchedDependencies();
	//Now schedule from all available instructions (without pending dependencies)

	//prepare the _scheduledInstructions (without pending dependencies), which will be fed into the pipe.
	_arbitrateQueues();

//NOW CHUCK _scheduledInstructions at the pipe
	if(_scheduledInstructions.scheduledFPM != _FPInstructionQueue.end())
	{
		_pPipes->FPMinPort(*_scheduledInstructions.scheduledFPM);
		_FPInstructionQueue.erase(_scheduledInstructions.scheduledFPM);
	}
	else
		{_pPipes->FPMinPort(traceinstruction());}
	if(_scheduledInstructions.scheduledFPA != _FPInstructionQueue.end())
	{
		_pPipes->FPAinPort(*_scheduledInstructions.scheduledFPA);
		_FPInstructionQueue.erase(_scheduledInstructions.scheduledFPA);
	}
	else
		{_pPipes->FPAinPort(traceinstruction());}
	if(_scheduledInstructions.scheduledALU1 != _ALUInstructionQueue.end())
	{
		_pPipes->ALUAinPort(*_scheduledInstructions.scheduledALU1);
		_ALUInstructionQueue.erase(_scheduledInstructions.scheduledALU1);
	}
	else
		{_pPipes->ALUAinPort(traceinstruction());}
	if(_scheduledInstructions.scheduledALU2 != _ALUInstructionQueue.end())
	{
		_pPipes->ALUBinPort(*_scheduledInstructions.scheduledALU2);
		_ALUInstructionQueue.erase(_scheduledInstructions.scheduledALU2);
	}
	else
		{_pPipes->ALUBinPort(traceinstruction());}
	if(_scheduledInstructions.scheduledLS1 != _LSInstructionQueue.end())
	{
		_pPipes->LSAinPort(*_scheduledInstructions.scheduledLS1);
		_LSInstructionQueue.erase(_scheduledInstructions.scheduledLS1);
	}
	else
		{_pPipes->LSAinPort(traceinstruction());}

	//Clear all scheduled iterators.
	_scheduledInstructions.scheduledFPM = 	_FPInstructionQueue.end();
	_scheduledInstructions.scheduledFPA = 	_FPInstructionQueue.end();
	_scheduledInstructions.scheduledALU1 = 	_ALUInstructionQueue.end();
	_scheduledInstructions.scheduledALU2 = 	_ALUInstructionQueue.end();
	_scheduledInstructions.scheduledLS1 = 	_LSInstructionQueue.end();

}

//==============================================================================================
//								FIX DEPENDENCIES FOR ALL QUEUE TYPES
//==============================================================================================


//Try to schedule an FP instruction
void InstSchedStage::_readyFPQueue()
{
	for(	FPQueueEntryiterator qitr = _FPInstructionQueue.begin();
			qitr != _FPInstructionQueue.end();
			qitr++																)
	{
//TODO: ADD IN CALLS TO PIPE FORWARDING STAGES AND DEBUG SIMPLE!!!!
		if(! (*qitr).m_rs_rdy)						//Attempt to meet dependency for rs operand
		{
			//Search ROB for m_rs
			if(_ROB->isDependencyMet((*qitr).m_rs))
				(*qitr).m_rs_rdy = true;
		}
		if(! (*qitr).m_rt_rdy)
		{
			//Search ROB for m_rt
			if(_ROB->isDependencyMet((*qitr).m_rt))
				(*qitr).m_rt_rdy = true;
		}
	}
}

void InstSchedStage::_readyALUQueue()
{
	for(	ALUQueueEntryiterator qitr = _ALUInstructionQueue.begin();
			qitr != _ALUInstructionQueue.end();
			qitr++																)
	{
		if(! (*qitr).m_rs_rdy)						//Attempt to meet dependency for rs operand
		{
			//Search ROB for m_rs
			if(_ROB->isDependencyMet((*qitr).m_rs))
				(*qitr).m_rs_rdy = true;
		}
		if(! (*qitr).m_rt_rdy)
		{
			//Search ROB for m_rt
			if(_ROB->isDependencyMet((*qitr).m_rt))
				(*qitr).m_rt_rdy = true;
		}
	}
}

void InstSchedStage::_readyLSQueue()
{
	for(	LSQueueEntryiterator qitr = _LSInstructionQueue.begin();
			qitr != _LSInstructionQueue.end();
			qitr++																)
	{
		if(! (*qitr).m_rs_rdy)						//Attempt to meet dependency for rs operand
		{
			//Search ROB for m_rs
			if(_ROB->isDependencyMet((*qitr).m_rs))
				(*qitr).m_rs_rdy = true;
		}
		if(! (*qitr).m_rt_rdy)
		{
			//Search ROB for m_rt
			if(_ROB->isDependencyMet((*qitr).m_rt))
				(*qitr).m_rt_rdy = true;
		}
	}
}

//=====================================================================================================
//						ADD A NEW INSTRUCTION TO ONE OF THE THREE QUEUE TYPES
//=====================================================================================================

bool InstSchedStage::pushFPInstruction(traceinstruction i)
{
	FPQueueEntry _i(&i);

	if(_FPInstructionQueue.size() >= 16)
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

	if(_ALUInstructionQueue.size() >= 16)
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

	if(_LSInstructionQueue.size() >= 16)
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
void InstSchedStage::_getFirstFPMulandAdd(FPQueueEntryiterator* Mulidx, FPQueueEntryiterator* Addidx)
{
#ifdef ARBITRATE_BY_QUEUE_POSITION
	bool firstMulFound = false;
	bool firstAddFound = false;

	*Mulidx = _FPInstructionQueue.end();
	*Addidx = _FPInstructionQueue.end();

	//Acts as a FIFO in the respect that the first entry of each type is always chosen first
	for(	FPQueueEntryiterator qitr = _FPInstructionQueue.begin();
			qitr != _FPInstructionQueue.end();
			qitr++																)
	{
		//First multiply logic
		if(	(!firstMulFound) 	&&		//Do we need the first multiply?
			((*qitr).intOp == M)&& 		//is this actually a multiply?
			(*qitr).m_rs_rdy 	&& 		//Is this operand ready?
			(*qitr).m_rt_rdy		)	//Is the other operand ready?
		{
			*Mulidx = qitr; firstMulFound = true;	//If all above are true, thanks very much this is the first to schedule :)
		}
		//First add logic
		if(	(!firstAddFound)	&&		//Do we need the first add?
			((*qitr).intOp == A)&& 		//Is this actually an add?
			(*qitr).m_rs_rdy 	&&
			(*qitr).m_rt_rdy		)
		{
			*Addidx = qitr; firstAddFound = true;	//If all above are true, thanks very much this is the first to schedule :)
		}
	}
	return;
#endif

#ifdef ARBITRATE_OLDEST
//BUGGY DOES NOT CHECK IF THE THINGS ARE READY!
	FPQueueEntryiterator pOldMul = _FPInstructionQueue.end();
	FPQueueEntryiterator pOldAdd = _FPInstructionQueue.end();
	traceline Mulcandidate = 0x7fffffff;
	traceline Addcandidate = Mulcandidate;

	for(	FPQueueEntryiterator qitr = _FPInstructionQueue.begin();
			qitr != _FPInstructionQueue.end();
			qitr++																)
	{
		//Oldest multiply logic
		if((*qitr).intOp == M)
		{
			if((*qitr).traceLineNo < Mulcandidate)
				{pOldMul = qitr; Mulcandidate = (*qitr).traceLineNo;}
		}
		//Oldest add logic
		if((*qitr).intOp == A)
		{
			if((*qitr).traceLineNo < Addcandidate)
				{pOldAdd = qitr; Addcandidate = (*qitr).traceLineNo;}
		}
	}

	//Set the oldest instances for the arbitrator to deal with
	*oldestMul = pOldMul;
	*oldestAdd = pOldAdd;
	return;
#endif



}

void InstSchedStage::_getFirsttwoALU(ALUQueueEntryiterator* ALU1idx, ALUQueueEntryiterator* ALU2idx)
{
#ifdef ARBITRATE_BY_QUEUE_POSITION
	bool firstALUFound = false;
	bool secondALUFound = false;

	*ALU1idx = _ALUInstructionQueue.end();
	*ALU2idx = _ALUInstructionQueue.end();

	//Acts as a FIFO in the respect that the first entry of each type is always chosen first
	for(	ALUQueueEntryiterator qitr = _ALUInstructionQueue.begin();
			qitr != _ALUInstructionQueue.end();
			qitr++																)
	{
//TODO: This will not account for instructions that should not be here. That is to say, a branch in here will F things up.
		if(	(!firstALUFound) &&
			(*qitr).m_rs_rdy &&
			(*qitr).m_rt_rdy	)
		{
			*ALU1idx = qitr; firstALUFound = true;
		}
		//First add logic
		if(	(!secondALUFound)&&
			(*qitr).m_rs_rdy &&
			(*qitr).m_rt_rdy	)
		{
			*ALU2idx = qitr; secondALUFound = true;
		}
	}
	return;
#endif

#ifdef ARBITRATE_OLDEST
//BUGGY DOES NOT CHECK IF THE THINGS ARE READY!
	ALUQueueEntryiterator pOldALU1 = _ALUInstructionQueue.end();
	ALUQueueEntryiterator pOldALU2 = _ALUInstructionQueue.end();
	traceline oldest1 = 0x7fffffff;
	traceline oldest2 = oldest1 = 1;

	for(	ALUQueueEntryiterator qitr = _ALUInstructionQueue.begin();
			qitr != _ALUInstructionQueue.end();
			qitr++																)
	{
		if((*qitr).traceLineNo < oldest1)
			{pOldALU1 = qitr; oldest1 = (*qitr).traceLineNo;}
		else if((*qitr).traceLineNo < oldest2)
			{pOldALU2 = qitr; oldest2 = (*qitr).traceLineNo;}
	}

	//set the oldest instances for the arbiter to deal with
	*oldALU1 = pOldALU1;
	*oldALU2 = pOldALU2;
#endif
}

//LS ALWAYS RETURNS THE OLDEST TRACE
void InstSchedStage::_getFirstLS(LSQueueEntryiterator* LSidx)
{
	*LSidx = _LSInstructionQueue.end();
	traceline oldest = 0x7fffffff;

	for(	LSQueueEntryiterator qitr = _LSInstructionQueue.begin();
			qitr != _LSInstructionQueue.end();
			qitr++																)
	{
		if(	(*qitr).traceLineNo < oldest&&
			(*qitr).m_rs_rdy			&&
			(*qitr).m_rt_rdy				)
		{
			*LSidx = qitr; oldest = (*qitr).traceLineNo;
		}
	}
	return;
}





} /* namespace R10k */
