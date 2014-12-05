/*
 * InstSched.cpp
 *
 *  Created on: Dec 3, 2014
 *      Author: king
 */

#include "InstSched.h"

namespace R10k {

InstSchedStage::~InstSchedStage() {
	// TODO Auto-generated destructor stub
}

//=====================================================================================================
//						PROMOTE FROM ALL QUEUES TO THE PIPES (SCHEDULE EXECUTION)
//=====================================================================================================


bool InstSchedStage::promoQueueToPipe()
{
	//First resolve all dependencies
	_resolveSchedDependencies();
	//Now schedule from all available instructions (without pending dependencies)

	//prepare the _scheduledInstructions (without pending dependencies), which will be fed into the pipe.
	_arbitrateQueues();

//TODO DEBUG DEBUG DEBUG DELETE ANYTHING YOU PUT IN THE PIPE
//FROM YOUR  QUEUE
//TODO DEBUG DEBUG DEBUG DELETE ANYTHING YOU PUT IN THE PIPE
//FROM YOUR  QUEUE
//TODO DEBUG DEBUG DEBUG DELETE ANYTHING YOU PUT IN THE PIPE
//FROM YOUR  QUEUE

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


	return true;
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

	_FPInstructionQueue.push_back(_i);

	return true;	//Success
}

bool InstSchedStage::pushALUInstruction(traceinstruction i)
{
	ALUQueueEntry _i(&i);

	if(_ALUInstructionQueue.size() >= 16)
		return false;

	_ALUInstructionQueue.push_back(_i);

	return true;	//Success
}

bool InstSchedStage::pushLSInstruction(traceinstruction i)
{
	LSQueueEntry _i(&i);

	if(_LSInstructionQueue.size() >= 16)
		return false;

	_LSInstructionQueue.push_back(_i);

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
