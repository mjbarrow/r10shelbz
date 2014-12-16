/*
 * InstPipe.h
 *
 *  Created on: Dec 4, 2014
 *      Author: king
 */

#ifndef INSTPIPE_H_
#define INSTPIPE_H_

#include <vector>
#include "utils.h"
#include "instructions.h"
#include "ROB.h"
#include "userinterface.h"
#include "TraceOutputLogger.h"

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
					ROB* 				pROB	)//Initialize blank pipe stages
	{
		_ui 		= ui;
		_plogger 	= logger;
		_ROB 		= pROB;

		_FPMpipe.push_back(traceinstruction()); _FPMpipe.push_back(traceinstruction()); _FPMpipe.push_back(traceinstruction()); _FPMpipe.push_back(traceinstruction());
		_FPApipe.push_back(traceinstruction()); _FPApipe.push_back(traceinstruction()); _FPApipe.push_back(traceinstruction()); _FPApipe.push_back(traceinstruction());
		_ALU1pipe.push_back(traceinstruction()); _ALU1pipe.push_back(traceinstruction());
		_ALU2pipe.push_back(traceinstruction()); _ALU2pipe.push_back(traceinstruction());
		_LS1pipe.push_back(traceinstruction()); _LS1pipe.push_back(traceinstruction()); _LS1pipe.push_back(traceinstruction());
	}

	//Project spec function
	void risingEdge();
	void calc();

	void FPMinPort(traceinstruction FPM)	{_DFPM 	= FPM;	}	//Set the port equal to what the Decode stage has passed in during "calc()"
	void FPAinPort(traceinstruction FPA)	{_DFPA 	= FPA;	}	//As above
	void ALUAinPort(traceinstruction ALUA)	{_DALU1 = ALUA;	}	//as above
	void ALUBinPort(traceinstruction ALUB)	{_DALU2 = ALUB;	}	//as above
	void LSAinPort(traceinstruction LSA)	{_DLS1 	= LSA;	}	//as above

//TODO: DEBUG AND Make sure these feedback paths work
	//These ForwardAV functions are acceptable as being accessible during Calc,
	//because they reflect the state of the pipeline throughout a cycle
	bool FPMforwardAvailable(int m_regdependency)
	{
		if((_FPMpipe[1].m_rd.machineReg == m_regdependency) || (_FPMpipe[2].m_rd.machineReg == m_regdependency) || (_FPMpipe[3].m_rd.machineReg == m_regdependency))
			return true;
		return false;
	}

	bool FPAforwardAvailable(int m_regdependency)
	{
		if((_FPApipe[1].m_rd.machineReg == m_regdependency) || (_FPApipe[2].m_rd.machineReg == m_regdependency) || (_FPApipe[3].m_rd.machineReg == m_regdependency))
			return true;
		return false;
	}

	bool ALU1forwardAvailable(int m_regdependency)
	{
		if((_ALU1pipe[1].m_rd.machineReg == m_regdependency))
			return true;
		return false;
	}

	bool ALU2forwardAvailable(int m_regdependency)
	{
		if((_ALU2pipe[1].m_rd.machineReg == m_regdependency))
			return true;
		return false;
	}

	bool LS1RegforwardAvailable(int m_regdependency)
	{
		if((_LS1pipe[2].m_rd.machineReg == m_regdependency))
			return true;
		return false;
	}

	//Just swizzle all the met dependencies
	bool LS1AddforwardAvailable(int address)
	{
	//For load and store
		//Resolve address dependency
		if(_LS1pipe[2].extra == address)
			return true;
		return false;
	}



	void print()
	{
		vector<traceinstruction>::const_iterator pstg;

		cerr << "_____________________________________________________" << endl;
		cerr << "FP Mul Pipe:\t||";
		for(	pstg = _FPMpipe.begin(); pstg != _FPMpipe.end(); pstg++	)
		{
			cerr << (*pstg).traceLineNo << "\t|";
		}
		cerr << "|" << endl;

		cerr << "_____________________________________________________"<< endl;
		cerr << "FP Add Pipe:\t||";
		for(	pstg = _FPApipe.begin(); pstg != _FPApipe.end(); pstg++	)
		{
			cerr << (*pstg).traceLineNo << "\t|";
		}
		cerr << "|" << endl;

		cerr << "_____________________________________________________"<< endl;
		cerr << "ALU 1 Pipe:\t||";
		for(	pstg = _ALU1pipe.begin(); pstg != _ALU1pipe.end(); pstg++	)
		{
			cerr << (*pstg).traceLineNo << "\t|";
		}
		cerr << "|" << endl;

		cerr << "_____________________________________________________"<< endl;
		cerr << "ALU 2 Pipe:\t||";
		for(	pstg = _ALU2pipe.begin(); pstg != _ALU2pipe.end(); pstg++	)
		{
			cerr << (*pstg).traceLineNo << "\t|";
		}
		cerr << "|" << endl;

		cerr << "_____________________________________________________"<< endl;
		cerr << "LS 1 Pipe:\t||";
		for(	pstg = _LS1pipe.begin(); pstg != _LS1pipe.end(); pstg++	)
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

	//D ports that can be hammered by the logic of the scheduler
	traceinstruction _DFPM;
	traceinstruction _DFPA;
	traceinstruction _DALU1;
	traceinstruction _DALU2;
	traceinstruction _DLS1;

	//Actual pipes, on clock the first entry of each gets
	vector<traceinstruction> _FPMpipe;
	vector<traceinstruction> _FPApipe;
	vector<traceinstruction> _ALU1pipe;
	vector<traceinstruction> _ALU2pipe;
	vector<traceinstruction> _LS1pipe;

	void FPMPipeToString(vector<string>*	stringFPMpipe	);
	void FPAPipeToString(vector<string>*	stringFPApipe	);
	void ALU1PipeToString(vector<string>*	stringALU1pipe	);
	void ALU2PipeToString(vector<string>*	stringALU2pipe	);
	void LS1PipeToString(vector<string>*	stringLSpipe	);

};

} /* namespace R10k */

#endif /* INSTPIPE_H_ */
