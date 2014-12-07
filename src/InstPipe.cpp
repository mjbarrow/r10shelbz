/*
 * InstPipe.cpp
 *
 *  Created on: Dec 4, 2014
 *      Author: king
 */

#include "InstPipe.h"

namespace R10k
{


void InstPipeStage::risingEdge()
{
	vector<string> stringFPMpipe;
	vector<string> stringFPApipe;
	vector<string> stringALU1pipe;
	vector<string> stringALU2pipe;
	vector<string> stringLS1pipe;

	//Do register swizzle. Changes of the ports can't affect the pipes with this approach.
	_FPMpipe.insert(_FPMpipe.begin(),_DFPM);	//Replace stage 1 with the input port value
	_FPMpipe.pop_back();						//Remove oldest value. This makes rendering very easy to do
	_DFPM = traceinstruction();					//Reset to a default value in case the port is not touched next cycle.
	_FPApipe.insert(_FPApipe.begin(),_DFPA);
	_FPApipe.pop_back();	//As above
	_DFPA = traceinstruction();
	_ALU1pipe.insert(_ALU1pipe.begin(),_DALU1);
	_ALU1pipe.pop_back();	//as above
	_DALU1 = traceinstruction();
	_ALU2pipe.insert(_ALU2pipe.begin(),_DALU2);
	_ALU2pipe.pop_back();	//as above
	_DALU2 = traceinstruction();
	_LS1pipe.insert(_LS1pipe.begin(),_DLS1);
	_DLS1 = traceinstruction();
	_LS1pipe.pop_back();	//as above

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
}

void InstPipeStage::FPMPipeToString(vector<string>*	stringFPMpipe	)
{
	vector<traceinstruction>::const_iterator pstg;
	ostringstream opos;	//For the op code
	ostringstream rdos; //For the dest reg

	opos << "tr#:";
	rdos << "Prd:";
	for(	pstg = _FPMpipe.begin(); pstg != _FPMpipe.end(); pstg++	)
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
					_FPApipe,		//Output pipe to get data from
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
					_ALU1pipe,		//Output pipe to get data from
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
					_ALU2pipe,		//Output pipe to get data from
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
					_LS1pipe,		//Output pipe to get data from
					opos,			//string converted opcodes go here
					rdos		);	//string converted dest regs go here

	stringLSpipe->push_back("st#:| -1- | -2- |");						//Let the user see what stage has what in it
	stringLSpipe->push_back(opos.str());							//Show the user the trace line
	stringLSpipe->push_back(rdos.str());							//Show the user destination register
}

} /* namespace R10k */
