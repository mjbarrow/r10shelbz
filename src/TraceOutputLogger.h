/*
 * TraceOutputLogger.h
 *
 *  Created on: Dec 7, 2014
 *      Author: king
 */

#ifndef TRACEOUTPUTLOGGER_H_
#define TRACEOUTPUTLOGGER_H_

#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include "utils.h"
#include "instructions.h"
#include "userinterface.h"

using namespace std;

typedef std::map<int,int>::iterator logindexmapitr;
class pipetrace: public string
{
public:
	bool modified;

	pipetrace(){modified = false;}
	virtual~pipetrace(){}
};

//=====================================================================================================
//									HELPER MACROS
//=====================================================================================================

#define stageToLogStrings(pstage,tline)					\
{														\
	int i;												\
														\
	logindexmapitr index =_outlogindexmap.find(tline);	\
	if(index != _outlogindexmap.end())					\
	{													\
		_pipelinediagram[index->second] += pstage;		\
		_pipelinediagram[index->second].modified = true;\
	}													\
	else												\
	{													\
		ostringstream ss;								\
		pipetrace newtrace;								\
		newtrace.modified = true;						\
		newtrace += " ";								\
		ss << tline;									\
		newtrace += ss.str();							\
		newtrace += " ";								\
														\
		for(i = 0; i < _cycle; i++)						\
		{												\
			newtrace += " - ";							\
		}												\
		newtrace += pstage;								\
/*a mapping between this trace and this outlog entry*/	\
		_outlogindexmap[traceline] = _pipelinediagram.size();\
		_pipelinediagram.push_back(newtrace);			\
	}													\
}

//=====================================================================================================
//								Class methods
//=====================================================================================================


namespace R10k {

class TraceOutputLogger {
public:
	TraceOutputLogger(	UserInterface* ui,
						const char* LogFile)
	{
		_ui = ui;
		_cycle = 0;
		_pipetrace.open(LogFile,std::ifstream::out);
	}

	virtual ~TraceOutputLogger(){}

	void logIFTrace(int traceline){stageToLogStrings(" F ",traceline);}

	void logIDTrace(int traceline){stageToLogStrings(" D ",traceline);}		//Log a D in the pipeline race for this pipeline trace

	void logISTrace(int traceline){stageToLogStrings(" S ",traceline);}

	void logEXTrace(int traceline){stageToLogStrings(" E ",traceline);}

	void logMPTrace(int traceline){stageToLogStrings(" M ",traceline);}		//log a mispredict.

//	void logRTTrace(int traceline);

	void logCMTrace(int traceline)
	{
		logindexmapitr print;
		stageToLogStrings(" C ",traceline)
		//TODO LOG TO FILE
		print =_outlogindexmap.find(traceline);
				/*if(index != _outlogindexmap.end())					\
				{													\
					_pipelinediagram[index->second] += pstage;		\
				stageToLogStrings(" C ",traceline)*/
		//TODO DELETE MAPPINGS
		//TODO DELETE STRING
	}	//DELETE MAPPINGS HERE

	void risingEdge(){_cycle++;}//Increment cycle counter

	//Used to write out the current pipeline status
	//Has to be done after the falling edge call of the ROB, because this reflects the commit and retire stages of instructions.
	void fallingEdge();

private:
	int _cycle;
	map<int,int>		_outlogindexmap;
	vector<pipetrace> 	_pipelinediagram;

//	vector<string> _outlog;				//TODO not being used yet
	UserInterface* _ui;

	//string*		_outfile;				//To do with outputting a pipeline trace
	ifstream	_pipetrace;

};

} /* namespace R10k */

#endif /* TRACEOUTPUTLOGGER_H_ */
