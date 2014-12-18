/*
 * InstFetch.h
 *
 *  Created on: Dec 7, 2014
 *      Author: king
 */

#include <queue>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include "utils.h"
#include "instructions.h"
#include "InstDecode.h"
#include "userinterface.h"
#include "TraceOutputLogger.h"


#ifndef INSTFETCH_H_
#define INSTFETCH_H_

#include "BranchResolver.h"

using namespace std;

namespace R10k {

class InstFetchStage {
public:
	vector<traceinstruction> instructionstream;


	//Requires a fully qualified correct path to a tracefile
	InstFetchStage(	UserInterface* 		ui,
					BranchResolver*		pBrUnit,
					bool*				pstallInput,
					InstDecodeStage* 	decode,
					TraceOutputLogger*	logger,
					const char*			TraceFile
					)
	{
		_tracelinenumber 		= 0;	//for internal book keeping
		_pdecstage 				= decode;
		_ui 					= ui;
		_BrUnit					= pBrUnit;
		_stallInput				= pstallInput;
		_plogger 				= logger;
		_failedBranch 			= traceinstruction();		//used when re-fetching mispredicted
		_failedBranch.extra		= 0;						//By the way Vegita, did I tell you that my
		_failedBranch.traceLineNo = 0x7fffffff;				//power level in this form is over a million?
		_failedPlusOneFilePos 	= 0;						//Branches
		fulltrace.open(TraceFile,std::ifstream::in);
	}

	//Project required functions
	void risingEdge();	//Push all of the instructions
//	void fallingEdge();
	void calc();		//Pull in 4 instructions if you can

	//Logical control lever used to re-fetch instruction stream on failed branch prediction
	//long long int getLastBranchPos(){return _branchAddQ.pop();}						//This is where to resume on missed branch.

	//Should be called on falling edge...
	void reFetchBranchMispredict(long long int filepos, vector<traceinstruction>* oldQout)
	{
		fulltrace.seekg(filepos);				//Roll back myself
		_pdecstage->setDTraceLines(oldQout); 	//Roll back my output to the next stage
	}	//will begin fetching and executing here.

	virtual ~InstFetchStage(){ fulltrace.close();}

private:
	ifstream fulltrace;

	InstDecodeStage* 	_pdecstage;
	UserInterface*		_ui;
	TraceOutputLogger* 	_plogger;
	int 				_tracelinenumber;
	//For failed branch prediction re-play
	queue<long long int> _branchAddQ;			//pop file pos to instruction after earliest branch.
	long long int		_failedPlusOneFilePos;
	traceinstruction	_failedBranch;
	BranchResolver*		_BrUnit;
	bool*				_stallInput;

	string traceInstructionToString(int entry);
};

} /* namespace R10k */

#endif /* INSTFETCH_H_ */
