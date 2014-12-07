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

//=====================================================================================================
//									HELPER MACROS
//=====================================================================================================
#define pipetostream(epipe,opcodes,destrs)	opcodes << "tr#:";													\
											destrs	<< "Prd:";													\
											for(vector<traceinstruction>::const_iterator pstg = epipe.begin(); pstg != epipe.end(); pstg++	)	\
											{																	\
												if((*pstg).intOp == BADOpcode)									\
												{																\
													opcodes << "| --- |";									\
													destrs <<  "| --- |";									\
												}																\
												else															\
												{																\
													opcodes << "| 0x" << hex << (*pstg).traceLineNo << " |";	\
													destrs << "| 0x" << hex << (*pstg).m_rd.machineReg << " |";\
												}																\
											}


//======================================================================================================
//								EXECUTION PIPE STAGE
//======================================================================================================

namespace R10k {

class InstPipeStage {
public:

	InstPipeStage(	UserInterface* 	ui,
					ROB* 			pROB)//Initialize blank pipe stages
	{
		_ui = ui;
		_ROB = pROB;

		_FPMpipe.push_back(traceinstruction()); _FPMpipe.push_back(traceinstruction()); _FPMpipe.push_back(traceinstruction());
		_FPApipe.push_back(traceinstruction()); _FPApipe.push_back(traceinstruction()); _FPApipe.push_back(traceinstruction());
		_ALU1pipe.push_back(traceinstruction());
		_ALU2pipe.push_back(traceinstruction());
		_LS1pipe.push_back(traceinstruction()); _LS1pipe.push_back(traceinstruction());
	}

	//Project spec function
	void risingEdge();

	void FPMinPort(traceinstruction FPM)	{_FPMpipe.insert(_FPMpipe.begin(),FPM);	_FPMpipe.pop_back();}
	void FPAinPort(traceinstruction FPA)	{_FPApipe.insert(_FPApipe.begin(),FPA);	_FPApipe.pop_back();}	//As above
	void ALUAinPort(traceinstruction ALUA)	{_ALU1pipe.insert(_ALU1pipe.begin(),ALUA); _ALU1pipe.pop_back();}//as above
	void ALUBinPort(traceinstruction ALUB)	{_ALU2pipe.insert(_ALU2pipe.begin(),ALUB); _ALU2pipe.pop_back();}//as above
	void LSAinPort(traceinstruction LSA)	{_LS1pipe.insert(_LS1pipe.begin(),LSA);	_LS1pipe.pop_back();}	//as above

//TODO: DEBUG AND Make sure these feedback paths work
	bool FPMforwardAvailable(int m_regdependency)
	{
		if((_FPMpipe.at(1).m_rd.machineReg == m_regdependency) || (_FPMpipe.at(2).m_rd.machineReg == m_regdependency))
			return true;
		return false;
	}

	bool FPAforwardAvailable(int m_regdependency)
	{
		if((_FPApipe.at(1).m_rd.machineReg == m_regdependency) || (_FPApipe.at(2).m_rd.machineReg == m_regdependency))
			return true;
		return false;
	}

	//This will just attempt to retire the last element in the vector, which should represent a complete instruction.
	void RetirePipes()
	{
		_ROB->retireEntry(_FPMpipe.back());	//Do not care if it cannot retire, the pipe may contain a bubble
		_ROB->retireEntry(_FPApipe.back());	//Do not care if it cannot retire, the pipe may contain a bubble
		_ROB->retireEntry(_ALU1pipe.back());	//Do not care if it cannot retire, the pipe may contain a bubble
		_ROB->retireEntry(_ALU2pipe.back());	//Do not care if it cannot retire, the pipe may contain a bubble
		_ROB->retireEntry(_LS1pipe.back());	//Do not care if it cannot retire, the pipe may contain a bubble
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
	UserInterface* _ui;
	ROB* _ROB;

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
