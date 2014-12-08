/*
 * InstFetch.h
 *
 *  Created on: Dec 7, 2014
 *      Author: king
 */

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

using namespace std;

namespace R10k {

class InstFetchStage {
public:
	vector<traceinstruction> instructionstream;


	//Requires a fully qualified correct path to a tracefile
	InstFetchStage(	UserInterface* 		ui,
					InstDecodeStage* 	decode,
					TraceOutputLogger*	logger,
					const char*			TraceFile
					)
	{
		_tracelinenumber = 0;	//for internal book keeping
		_pdecstage = decode;
		_ui = ui;
		_plogger = logger;
		fulltrace.open(TraceFile,std::ifstream::in);
	}

	//Project required functions
	void risingEdge();	//Push all of the instructions
	void calc();		//Pull in 4 instructions if you can

	virtual ~InstFetchStage(){ fulltrace.close();}

private:
	ifstream fulltrace;

	InstDecodeStage* 	_pdecstage;
	UserInterface*		_ui;
	TraceOutputLogger* 	_plogger;
	int 				_tracelinenumber;

	string traceInstructionToString(int entry);
};

} /* namespace R10k */

#endif /* INSTFETCH_H_ */
