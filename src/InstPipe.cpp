/*
 * InstPipe.cpp
 *
 *  Created on: Dec 4, 2014
 *      Author: king
 */

#include "InstPipe.h"

namespace R10k
{

//====================================================================================
//						REQUIRED PROJECT FUNCTIONS
//====================================================================================
//CALLED AFTER RISING EDGE,
//SO THE INSTRUCTIONS SHOULD GO TO COMMIT WHEN
//THEY CLOCK OUT (PIPELINE LENGTH + 1)
void InstPipeStage::calc()				//This will just attempt to retire the last element in the vector, which should
{										//represent a complete instruction.
//MOVED ALL PIPE CRAP FROM RISING EDGE TO CALC. SEEMS RIGHT PLACE TO DO IT

	vector<string> stringFPMpipe;
	vector<string> stringFPApipe;
	vector<string> stringALU1pipe;
	vector<string> stringALU2pipe;
	vector<string> stringLS1pipe;

	//Clear mispredict flag if set
	_didMispredict = traceinstruction();

/*
	//Detect branch mispredict here
	((*_ALU1pipe)[0]).insert(((*_ALU1pipe)[0]).begin(),_DALU1);
	((*_ALU1pipe)[0]).pop_back();	//as above
	if(_DALU1.intOp != BADOpcode)
	{
		if((_DALU1.intOp == B) && (_DALU1.extra == 1))
		{
			//the pipeline is already stalled, by falling edge perform system rewind
			//log the mispredict
			_didMispredict = _DALU1;	//Will allow the scheduler to know sthg was wrong on falling edge
			_plogger->logMPTrace(_DALU1.traceLineNo);
			//start restoration
			_pBRUnit->mispredictRollback(_DALU1);
//TODO: DUMP PIPES!
			_mispredicthandler();
		}
		else
			_plogger->logEXTrace(_DALU1.traceLineNo);
	}

	//Only do this if we are not now stalled
	if(_DALU1.extra != 1)

	//Detect branch mispredict here
	((*_ALU2pipe)[0]).insert(((*_ALU2pipe)[0]).begin(),_DALU2);
	((*_ALU2pipe)[0]).pop_back();	//as above
	if(_DALU2.intOp != BADOpcode)
	{
		if((_DALU2.intOp == B) && (_DALU2.extra == 1))
		{
			//the pipeline is already stalled, by falling edge perform system rewind
			//log the mispredict
			_didMispredict = _DALU1;	//Will allow the scheduler to know sthg was wrong on falling edge
			_plogger->logMPTrace(_DALU2.traceLineNo);
			//start restoration
			_pBRUnit->mispredictRollback(_DALU2);
//TODO: DUMP PIPES!
			_mispredicthandler();
		}
		else
			_plogger->logEXTrace(_DALU2.traceLineNo);
	}
*/
	//Stalled flow
	if(*_stallPrevStages)
	{
		//Should log a stall on ALU1 and roll back
		if((_DALU1.intOp == B) && (_DALU1.extra == 1))
		{
			//the pipeline is already stalled, by falling edge perform system rewind
			 _DALU1.extra = 0;	//prevent inf loop
			//note this instruction as executing in the ROB
			_ROB->entryExecuted(&_DALU1);
			 //log the mispredict
			_didMispredict = _DALU1;	//Will allow the scheduler to know sthg was wrong on falling edge
			_plogger->logMPTrace(_DALU1.traceLineNo);
			//start restoration
			_pBRUnit->mispredictRollback(_DALU1);
			//dump pipe new instructions
			_mispredicthandler();

			_DFPM 	= traceinstruction();	//Reset to a default value in case the port is not touched next cycle.
			_DFPA 	= traceinstruction();
			_DLS1 	= traceinstruction();
//			_DALU1 	= traceinstruction();
			_DALU2 	= traceinstruction();
		}

		//Should log a stall on ALU2 and roll back
		if((_DALU2.intOp == B) && (_DALU2.extra == 1))
		{
			//the pipeline is already stalled, by falling edge perform system rewind
			_DALU2.extra = 0;	//Prevent inf loop
			//note this instruction as executing in the ROB
			_ROB->entryExecuted(&_DALU2);
			//log the mispredict
			_didMispredict = _DALU1;	//Will allow the scheduler to know sthg was wrong on falling edge
			_plogger->logMPTrace(_DALU2.traceLineNo);
			//start restoration
			_pBRUnit->mispredictRollback(_DALU2);
			//dump pipe new instructions
			_mispredicthandler();

			_DFPM 	= traceinstruction();	//Reset to a default value in case the port is not touched next cycle.
			_DFPA 	= traceinstruction();
			_DLS1 	= traceinstruction();
			_DALU1 	= traceinstruction();
//			_DALU2 	= traceinstruction();
		}
	}

	//Normal flow
	if(!*_stallPrevStages)
	{
		((*_ALU1pipe)[0]).insert(((*_ALU1pipe)[0]).begin(),_DALU1);
		((*_ALU1pipe)[0]).pop_back();	//as above
		if(_DALU1.intOp != BADOpcode)
		{
			_plogger->logEXTrace(_DALU1.traceLineNo);
			//note this instruction as executing in the ROB
			_ROB->entryExecuted(&_DALU1);
		}
		((*_ALU2pipe)[0]).insert(((*_ALU2pipe)[0]).begin(),_DALU2);
		((*_ALU2pipe)[0]).pop_back();	//as above
		if(_DALU2.intOp != BADOpcode)
		{
			_plogger->logEXTrace(_DALU2.traceLineNo);
			//note this instruction as executing in the ROB
			_ROB->entryExecuted(&_DALU2);
		}
		//Do register swizzle. Changes of the ports can't affect the pipes with this approach.
		((*_FPMpipe)[0]).insert(((*_FPMpipe)[0]).begin(),_DFPM);	//Replace stage 1 with the input port value
		((*_FPMpipe)[0]).pop_back();						//Remove oldest value. This makes rendering very easy to do
		if(_DFPM.intOp != BADOpcode)				//Add this instruction to the pipeline diagram (if it is one)
		{
			_plogger->logEXTrace(_DFPM.traceLineNo);//There was an instruction, add it to the pipeline diagram
			//note this instruction as executing in the ROB
			_ROB->entryExecuted(&_DFPM);
		}
		((*_FPApipe)[0]).insert(((*_FPApipe)[0]).begin(),_DFPA);
		((*_FPApipe)[0]).pop_back();	//As above
		if(_DFPA.intOp != BADOpcode)
		{
			_plogger->logEXTrace(_DFPA.traceLineNo);
			_ROB->entryExecuted(&_DFPA);
		}
		((*_LS1pipe)[0]).insert(((*_LS1pipe)[0]).begin(),_DLS1);
		if(_DLS1.intOp != BADOpcode)
		{
			_plogger->logEXTrace(_DLS1.traceLineNo);
			_ROB->entryExecuted(&_DLS1);
		}
		((*_LS1pipe)[0]).pop_back();	//as above

		//Clear all input ports only if no mispredict

		_DFPM 	= traceinstruction();	//Reset to a default value in case the port is not touched next cycle.
		_DFPA 	= traceinstruction();
		_DLS1 	= traceinstruction();
		_DALU1 	= traceinstruction();
		_DALU2 	= traceinstruction();
	}

	//Construct the correct type of input for the blit function
	//An array of strings which it will blit out
	FPMPipeToString(&stringFPMpipe);
	FPAPipeToString(&stringFPApipe);
	ALU1PipeToString(&stringALU1pipe);
	ALU2PipeToString(&stringALU2pipe);
	LS1PipeToString(&stringLS1pipe);

	//trigger the blit function so that the screen output is refreshed of the queues content
	//Blit all queues
	_ui->blitFPMPipe(	&stringFPMpipe	);
	_ui->blitFPApipe(	&stringFPApipe	);
	_ui->blitALUAPipe(	&stringALU1pipe	);
	_ui->blitALUBPipe(	&stringALU2pipe	);
	_ui->blitLSPipe(	&stringLS1pipe	);


//END RE-PLACED FROM RISING EDGE

	//Note: /*.back()*/ does not work.
	//This lets the commit stage commit the instruction at the end of the pipe
	_ROB->retireEntry(FPMRETIREPORT,((*_FPMpipe)[0])[2]);	//Do not care if it cannot retire, the pipe may contain a bubble
	_ROB->retireEntry(FPARETIREPORT,((*_FPApipe)[0])[2]);	//Do not care if it cannot retire, the pipe may contain a bubble
	_ROB->retireEntry(ALUARETIREPORT,((*_ALU1pipe)[0])[0]);	//Do not care if it cannot retire, the pipe may contain a bubble
	_ROB->retireEntry(ALUBRETIREPORT,((*_ALU2pipe)[0])[0]);	//Do not care if it cannot retire, the pipe may contain a bubble
	_ROB->retireEntry(LSARETIREPORT,((*_LS1pipe)[0])[1]);	//Do not care if it cannot retire, the pipe may contain a bubble
}

void InstPipeStage::risingEdge()
{
/*
	vector<string> stringFPMpipe;
	vector<string> stringFPApipe;
	vector<string> stringALU1pipe;
	vector<string> stringALU2pipe;
	vector<string> stringLS1pipe;

	//Clear mispredict flag if set
	_didMispredict = traceinstruction();
	//Clear mispredict signal wire (goes to other units) if set
	*_stallPrevStages = false;

	//Detect branch mispredict here
	((*_ALU1pipe)[0]).insert(((*_ALU1pipe)[0]).begin(),_DALU1);
	((*_ALU1pipe)[0]).pop_back();	//as above
	if(_DALU1.intOp != BADOpcode)
	{
		if((_DALU1.intOp == B) && (_DALU1.extra == 1))
		{
			//the pipeline is already stalled, by falling edge perform system rewind
			//log the mispredict
			_didMispredict = _DALU1;	//Will allow the scheduler to know sthg was wrong on falling edge
			_plogger->logMPTrace(_DALU1.traceLineNo);
			//start restoration
			_pBRUnit->mispredictRollback(_DALU1);
//TODO: DUMP PIPES!
			_mispredicthandler();
		}
		else
			_plogger->logEXTrace(_DALU1.traceLineNo);
	}
	//Detect branch mispredict here
	((*_ALU2pipe)[0]).insert(((*_ALU2pipe)[0]).begin(),_DALU2);
	((*_ALU2pipe)[0]).pop_back();	//as above
	if(_DALU2.intOp != BADOpcode)
	{
		if((_DALU2.intOp == B) && (_DALU2.extra == 1))
		{
			//the pipeline is already stalled, by falling edge perform system rewind
			//log the mispredict
			_didMispredict = _DALU1;	//Will allow the scheduler to know sthg was wrong on falling edge
			_plogger->logMPTrace(_DALU2.traceLineNo);
			//start restoration
			_pBRUnit->mispredictRollback(_DALU2);
//TODO: DUMP PIPES!
			_mispredicthandler();
		}
		else
			_plogger->logEXTrace(_DALU2.traceLineNo);
	}

	//Do register swizzle. Changes of the ports can't affect the pipes with this approach.
	((*_FPMpipe)[0]).insert(((*_FPMpipe)[0]).begin(),_DFPM);	//Replace stage 1 with the input port value
	((*_FPMpipe)[0]).pop_back();						//Remove oldest value. This makes rendering very easy to do
	if(_DFPM.intOp != BADOpcode)				//Add this instruction to the pipeline diagram (if it is one)
		_plogger->logEXTrace(_DFPM.traceLineNo);//There was an instruction, add it to the pipeline diagram
	((*_FPApipe)[0]).insert(((*_FPApipe)[0]).begin(),_DFPA);
	((*_FPApipe)[0]).pop_back();	//As above
	if(_DFPA.intOp != BADOpcode)
		_plogger->logEXTrace(_DFPA.traceLineNo);
	((*_LS1pipe)[0]).insert(((*_LS1pipe)[0]).begin(),_DLS1);
	if(_DLS1.intOp != BADOpcode)
		_plogger->logEXTrace(_DLS1.traceLineNo);
	((*_LS1pipe)[0]).pop_back();	//as above


	//Clear all input ports only if no mispredict
	if(! *_stallPrevStages)
	{
		_DFPM 	= traceinstruction();	//Reset to a default value in case the port is not touched next cycle.
		_DFPA 	= traceinstruction();
		_DLS1 	= traceinstruction();
		_DALU1 	= traceinstruction();
		_DALU2 	= traceinstruction();
	}

	//Construct the correct type of input for the blit function
	//An array of strings which it will blit out
	FPMPipeToString(&stringFPMpipe);
	FPAPipeToString(&stringFPApipe);
	ALU1PipeToString(&stringALU1pipe);
	ALU2PipeToString(&stringALU2pipe);
	LS1PipeToString(&stringLS1pipe);

	//trigger the blit function so that the screen output is refreshed of the queues content
	//Blit all queues
	_ui->blitFPMPipe(	&stringFPMpipe	);
	_ui->blitFPApipe(	&stringFPApipe	);
	_ui->blitALUAPipe(	&stringALU1pipe	);
	_ui->blitALUBPipe(	&stringALU2pipe	);
	_ui->blitLSPipe(	&stringLS1pipe	);
*/
	//Clear input ports if we did not stall the rest of the pipe

}

//TODO move to rising edge
void InstPipeStage::fallingEdge()	//This will set the stall signal true if a mispredict is found
{									//Doing this allows earlier pipe stages to react on their calc edges.
	//The scheduler will ensure that no two branch instructions enter
	//the pipeline at once, so it is safe to stall on either integer pipe

	if((_DALU2.intOp == B) && (_DALU2.extra == 0))
		{*_stallPrevStages = false;}	//Clear any prev stall
	if((_DALU1.intOp == B) && (_DALU1.extra == 0))
		{*_stallPrevStages = false;}	//Clear any prev stall

	if((_DALU2.intOp == B) && (_DALU2.extra == 1))
		{*_stallPrevStages = true;}	//No inf loop on stall
	if((_DALU1.intOp == B) && (_DALU1.extra == 1))
		{*_stallPrevStages = true;}	//No inf loop on stall

}

//=========================================================================================
//						HELPER FNS
//=========================================================================================
void InstPipeStage::_mispredicthandler()
{
	vector<traceinstruction>::iterator pstg;

	//clean FPM pipe
	for(	pstg = ((*_FPMpipe)[0]).begin(); pstg != ((*_FPMpipe)[0]).end(); pstg++	)
	{
		if(pstg->traceLineNo > _didMispredict.traceLineNo)
			*pstg = traceinstruction();//Kill!
	}
	//Clean FPA pipe
	for(	pstg = ((*_FPApipe)[0]).begin(); pstg != ((*_FPApipe)[0]).end(); pstg++	)
	{
		if(pstg->traceLineNo > _didMispredict.traceLineNo)
			*pstg = traceinstruction();//Crush!
	}
	//Clean ALU1 pipe
	for(	pstg = ((*_ALU1pipe)[0]).begin(); pstg != ((*_ALU1pipe)[0]).end(); pstg++	)
	{
		if(pstg->traceLineNo > _didMispredict.traceLineNo)
			*pstg = traceinstruction();//Destroy!
	}
	//Clean ALU2 pipe
	for(	pstg = ((*_ALU2pipe)[0]).begin(); pstg != ((*_ALU2pipe)[0]).end(); pstg++	)
	{
		if(pstg->traceLineNo > _didMispredict.traceLineNo)
			*pstg = traceinstruction();//You Died!
	}
	//Clean LS1 pipe
	for(	pstg = ((*_LS1pipe)[0]).begin(); pstg != ((*_LS1pipe)[0]).end(); pstg++	)
	{
		if(pstg->traceLineNo > _didMispredict.traceLineNo)
			*pstg = traceinstruction();//Ah ha ha ha!
	}
}

//==========================================================================================
//						UTIL FNS
//==========================================================================================

void InstPipeStage::FPMPipeToString(vector<string>*	stringFPMpipe	)
{
	vector<traceinstruction>::const_iterator pstg;
	ostringstream opos;	//For the op code
	ostringstream rdos; //For the dest reg

	opos << "tr#:";
	rdos << "Prd:";
	for(	pstg = ((*_FPMpipe)[0]).begin(); pstg != ((*_FPMpipe)[0]).end(); pstg++	)
	{
		if((*pstg).intOp == BADOpcode)
		{
			opos << "| --- |";
			rdos << "| --- |";
		}
		else
		{
			opos << "| 0x" << hex << (*pstg).traceLineNo << " |";
			rdos << "| 0x" << hex << (*pstg).m_rd.machineReg << " |";
		}
	}

	stringFPMpipe->push_back("st#:| -1- || -2- || -3- |");
	stringFPMpipe->push_back(opos.str());
	stringFPMpipe->push_back(rdos.str());
}

void InstPipeStage::FPAPipeToString(vector<string>*	stringFPApipe	)
{
	ostringstream opos;	//For the op code
	ostringstream rdos; //For the dest reg

	pipetostream(					//This macro implements the body of FPMPipeToString(vector<string>*	stringFPMpipe	)
					((*_FPApipe)[0]),		//Output pipe to get data from
					opos,			//string converted opcodes go here
					rdos		);	//string converted dest regs go here

	stringFPApipe->push_back("st#:| -1- | -2- | -3- |");	//Let the user see what stage has what in it
	stringFPApipe->push_back(opos.str());							//Show the user the trace line
	stringFPApipe->push_back(rdos.str());							//Show the user destination register
}

void InstPipeStage::ALU1PipeToString(vector<string>*	stringALU1pipe	)
{
	ostringstream opos;	//For the op code
	ostringstream rdos; //For the dest reg

	pipetostream(					//This macro implements the body of FPMPipeToString(vector<string>*	stringFPMpipe	)
					((*_ALU1pipe)[0]),		//Output pipe to get data from
					opos,			//string converted opcodes go here
					rdos		);	//string converted dest regs go here

	stringALU1pipe->push_back("st#:| -1- |");	//Let the user see what stage has what in it
	stringALU1pipe->push_back(opos.str());							//Show the user the trace line
	stringALU1pipe->push_back(rdos.str());							//Show the user destination register
}

void InstPipeStage::ALU2PipeToString(vector<string>*	stringALU2pipe	)
{
	ostringstream opos;	//For the op code
	ostringstream rdos; //For the dest reg

	pipetostream(					//This macro implements the body of FPMPipeToString(vector<string>*	stringFPMpipe	)
					((*_ALU2pipe)[0]),		//Output pipe to get data from
					opos,			//string converted opcodes go here
					rdos		);	//string converted dest regs go here

	stringALU2pipe->push_back("st#:| -1- |");	//Let the user see what stage has what in it
	stringALU2pipe->push_back(opos.str());							//Show the user the trace line
	stringALU2pipe->push_back(rdos.str());							//Show the user destination register
}

void InstPipeStage::LS1PipeToString(vector<string>*	stringLSpipe	)
{
	ostringstream opos;	//For the op code
	ostringstream rdos; //For the dest reg

	pipetostream(					//This macro implements the body of FPMPipeToString(vector<string>*	stringFPMpipe	)
					((*_LS1pipe)[0]),		//Output pipe to get data from
					opos,			//string converted opcodes go here
					rdos		);	//string converted dest regs go here

	stringLSpipe->push_back("st#:| -1- | -2- |");						//Let the user see what stage has what in it
	stringLSpipe->push_back(opos.str());							//Show the user the trace line
	stringLSpipe->push_back(rdos.str());							//Show the user destination register
}

} /* namespace R10k */
