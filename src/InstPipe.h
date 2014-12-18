/*
 * InstPipe.h
 *
 *  Created on: Dec 4, 2014
 *      Author: king
 */


#include <vector>
#include "utils.h"
#include "instructions.h"
#include "ROB.h"
#include "userinterface.h"
#include "TraceOutputLogger.h"

#include "BranchResolver.h"

#ifndef INSTPIPE_H_
#define INSTPIPE_H_



//=====================================================================================================
//									HELPER MACROS
//=====================================================================================================
#define pipetostream(epipe,opcodes,destrs)	opcodes << "tr#:";													\
											destrs	<< "Prd:";													\
											for(vector<traceinstruction>::const_iterator pstg = epipe.begin(); pstg != epipe.end(); pstg++	)	\
											{																	\
												if((*pstg).intOp == BADOpcode)									\
												{																\
													opcodes << "| --- |";										\
													destrs <<  "| --- |";										\
												}																\
												else															\
												{																\
													opcodes << "| 0x" << hex << (*pstg).traceLineNo << " |";	\
													if(pstg->intOp & (L | S))									\
														destrs << "| N/A |";									\
													else														\
														destrs << "| 0x" << hex << (*pstg).m_rd.machineReg << " |";	\
												}																\
											}


//======================================================================================================
//								EXECUTION PIPE STAGE
//======================================================================================================

namespace R10k {

class InstPipeStage {
public:

	InstPipeStage(	UserInterface* 		ui,
					TraceOutputLogger*	logger,
					ROB* 				pROB,
					BranchResolver*		pBRUnit,
					vector< vector<traceinstruction> >* FPMpipe,
					vector< vector<traceinstruction> >* FPApipe,
					vector< vector<traceinstruction> >* ALU1pipe,
					vector< vector<traceinstruction> >* ALU2pipe,
					vector< vector<traceinstruction> >* LS1pipe	)//Initialize blank pipe stages
	{
		_ui 		= ui;
		_plogger 	= logger;
		_ROB 		= pROB;
		_pBRUnit	= pBRUnit;

		_FPMpipe 	= FPMpipe;
		_FPApipe 	= FPApipe;
		_ALU1pipe 	= ALU1pipe;
		_ALU2pipe 	= ALU2pipe;
		_LS1pipe 	= LS1pipe;

		_didMispredict = traceinstruction();

		((*_FPMpipe)[0]).push_back(traceinstruction()); ((*_FPMpipe)[0]).push_back(traceinstruction()); ((*_FPMpipe)[0]).push_back(traceinstruction()); ((*_FPMpipe)[0]).push_back(traceinstruction());
		((*_FPApipe)[0]).push_back(traceinstruction()); ((*_FPApipe)[0]).push_back(traceinstruction()); ((*_FPApipe)[0]).push_back(traceinstruction()); ((*_FPApipe)[0]).push_back(traceinstruction());
		((*_ALU1pipe)[0]).push_back(traceinstruction()); ((*_ALU1pipe)[0]).push_back(traceinstruction());
		((*_ALU2pipe)[0]).push_back(traceinstruction()); ((*_ALU2pipe)[0]).push_back(traceinstruction());
		((*_LS1pipe)[0]).push_back(traceinstruction()); ((*_LS1pipe)[0]).push_back(traceinstruction()); ((*_LS1pipe)[0]).push_back(traceinstruction());
	}

	//Project spec function
	void risingEdge();
	void calc();

	void FPMinPort(traceinstruction FPM)	{_DFPM 	= FPM;	}	//Set the port equal to what the Decode stage has passed in during "calc()"
	void FPAinPort(traceinstruction FPA)	{_DFPA 	= FPA;	}	//As above
	void ALUAinPort(traceinstruction ALUA)	{_DALU1 = ALUA;	}	//as above
	void ALUBinPort(traceinstruction ALUB)	{_DALU2 = ALUB;	}	//as above
	void LSAinPort(traceinstruction LSA)	{_DLS1 	= LSA;	}	//as above

	traceinstruction didBranchMispredict()	{return _didMispredict;}	//Used by the falling edge of the scheduler to purge scheduled instructions.

	//These ForwardAV functions are acceptable as being accessible during Calc,
	//because they reflect the state of the pipeline throughout a cycle
	bool FPMforwardAvailable(int m_regdependency)
	{
		if((((*_FPMpipe)[0])[1].m_rd.machineReg == m_regdependency) || (((*_FPMpipe)[0])[2].m_rd.machineReg == m_regdependency) || (((*_FPMpipe)[0])[3].m_rd.machineReg == m_regdependency))
			return true;
		return false;
	}

	bool FPAforwardAvailable(int m_regdependency)
	{
		if((((*_FPApipe)[0])[1].m_rd.machineReg == m_regdependency) || (((*_FPApipe)[0])[2].m_rd.machineReg == m_regdependency) || (((*_FPApipe)[0])[3].m_rd.machineReg == m_regdependency))
			return true;
		return false;
	}

	bool ALU1forwardAvailable(int m_regdependency)
	{
		if((((*_ALU1pipe)[0])[1].m_rd.machineReg == m_regdependency))
			return true;
		return false;
	}

	bool ALU2forwardAvailable(int m_regdependency)
	{
		if((((*_ALU2pipe)[0])[1].m_rd.machineReg == m_regdependency))
			return true;
		return false;
	}

	bool LS1RegforwardAvailable(int m_regdependency)
	{
		if((((*_LS1pipe)[0])[2].m_rd.machineReg == m_regdependency))
			return true;
		return false;
	}

	//Just swizzle all the met dependencies
	bool LS1AddforwardAvailable(int address)
	{
	//For load and store
		//Resolve address dependency
		if(((*_LS1pipe)[0])[2].extra == address)
			return true;
		return false;
	}



	void print()
	{
		vector<traceinstruction>::const_iterator pstg;

		cerr << "_____________________________________________________" << endl;
		cerr << "FP Mul Pipe:\t||";
		for(	pstg = ((*_FPMpipe)[0]).begin(); pstg != ((*_FPMpipe)[0]).end(); pstg++	)
		{
			cerr << (*pstg).traceLineNo << "\t|";
		}
		cerr << "|" << endl;

		cerr << "_____________________________________________________"<< endl;
		cerr << "FP Add Pipe:\t||";
		for(	pstg = ((*_FPApipe)[0]).begin(); pstg != ((*_FPApipe)[0]).end(); pstg++	)
		{
			cerr << (*pstg).traceLineNo << "\t|";
		}
		cerr << "|" << endl;

		cerr << "_____________________________________________________"<< endl;
		cerr << "ALU 1 Pipe:\t||";
		for(	pstg = ((*_ALU1pipe)[0]).begin(); pstg != ((*_ALU1pipe)[0]).end(); pstg++	)
		{
			cerr << (*pstg).traceLineNo << "\t|";
		}
		cerr << "|" << endl;

		cerr << "_____________________________________________________"<< endl;
		cerr << "ALU 2 Pipe:\t||";
		for(	pstg = ((*_ALU2pipe)[0]).begin(); pstg != ((*_ALU2pipe)[0]).end(); pstg++	)
		{
			cerr << (*pstg).traceLineNo << "\t|";
		}
		cerr << "|" << endl;

		cerr << "_____________________________________________________"<< endl;
		cerr << "LS 1 Pipe:\t||";
		for(	pstg = ((*_LS1pipe)[0]).begin(); pstg != ((*_LS1pipe)[0]).end(); pstg++	)
		{
			cerr << (*pstg).traceLineNo << "\t|";
		}
		cerr << "|" << endl;
	}

	virtual ~InstPipeStage(){}

private:
	UserInterface* 		_ui;
	TraceOutputLogger* 	_plogger;
	ROB* 				_ROB;
	BranchResolver*		_pBRUnit;	//Call this when a branch is mispredicted

	//D ports that can be hammered by the logic of the scheduler
	traceinstruction _DFPM;
	traceinstruction _DFPA;
	traceinstruction _DALU1;
	traceinstruction _DALU2;
	traceinstruction _DLS1;

	//Actual pipes, on clock the first entry of each gets
	vector< vector<traceinstruction> >* _FPMpipe;
	vector< vector<traceinstruction> >* _FPApipe;
	vector< vector<traceinstruction> >* _ALU1pipe;
	vector< vector<traceinstruction> >* _ALU2pipe;
	vector< vector<traceinstruction> >* _LS1pipe;


	//flag that is set if a mispredict happened in the pipeline
	traceinstruction _didMispredict;

	void FPMPipeToString(vector<string>*	stringFPMpipe	);
	void FPAPipeToString(vector<string>*	stringFPApipe	);
	void ALU1PipeToString(vector<string>*	stringALU1pipe	);
	void ALU2PipeToString(vector<string>*	stringALU2pipe	);
	void LS1PipeToString(vector<string>*	stringLSpipe	);

//TODO flush out all instructions from pipe younger than mispredicted
//Uses _didMispredict. so that had better be set up right.
	void _mispredicthandler();
};

} /* namespace R10k */

#endif /* INSTPIPE_H_ */
