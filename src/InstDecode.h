/*
 * InstDecode.h
 *
 *  Created on: Nov 20, 2014
 *      Author: king
 */

#ifndef INSTDECODE_H_
#define INSTDECODE_H_

#include "utils.h"
#include "instructions.h"
#include "ROB.h"
#include "InstSched.h"
#include <queue>
#include <map>
#include "userinterface.h"
#include "TraceOutputLogger.h"
//#include "BranchResolver.h"

using namespace logic;


namespace R10k {

class InstDecodeStage {
public:

	/*Initialize FreeRegList where all renameReg's are free*/
	InstDecodeStage(	UserInterface* 			ui,
						TraceOutputLogger* 		logger,
						InstSchedStage* 		pScheduler,
						ROB* 					pROB,
						bool*					stallInput,
						vector<freeRegList>* 	pFreeRegList,
						vector<regmaptable>* 	pRegMapTable,
						vector<ISAreginstance>* pMaxISAregmap)
	{
		int i;
		_ui 				= ui;
		_plogger			= logger;
		_pScheduler 		= pScheduler;
		_pROB 				= pROB;
		_FreeRegList 		= pFreeRegList;
		_RegMapTable 		= pRegMapTable;
		_ISAInstanceRegmap 	= pMaxISAregmap;		//this should wrap around, so the ROB may not be larger than 2^16
		_stallInput			= stallInput;			//Signal that will stall this stage if needed

		for(i = 0; i< renameRegCount; i++){(*_FreeRegList)[0].push(i);}						//Initialize free reg list
		for(i = 0; i< ISARegCount; i++){(*_ISAInstanceRegmap)[0].insert(ISAreginstancepair(i,0));} //Initialize reginstance to 0 for all ISAReg's

		for(i = 0; i<4; i++)
		{
			_DTraceLines.push_back(traceinstruction());
			_QTraceLines.push_back(traceinstruction());
		}			//Always have 4 trace instructions available

		//house keeping logic init
		_traceLinesAccepted = MAXDECODEDINSTPERCYCLE;
		_tracebufhead = 0;
	}

	void risingEdge();
	void calc();

	//The IO function of this stage. set the D input and read the feedback output.
	//You can do this as many times as you want you wont change what does on with the calc()
	int setDTraceLines(vector<traceinstruction>* DTraceLines)
	{
		int i = 0;

		//Ugh. Corner case. Always accept Max if you accepted max last time
		if((MAXDECODEDINSTPERCYCLE - _traceLinesAccepted) == 0)
			_traceLinesAccepted = 0;

		//Accept the amount of instruction slots available in the instruction buffer
		while((i < (MAXDECODEDINSTPERCYCLE - _traceLinesAccepted)) && ((unsigned int)i < DTraceLines->size()) )
		{
			//Remove blank instruction slots.
			_DTraceLines[_tracebufhead] = (*DTraceLines)[i];
			_tracebufhead++;
			if(_tracebufhead == MAXDECODEDINSTPERCYCLE)
				_tracebufhead = 0;
			i++;
		}
		_traceLinesAccepted = i;

		return _traceLinesAccepted;	//Provide feedback to the instruction buffer so it doesn't overload us and make us drop instructions
	}

	int getLinesAccepted(){return _traceLinesAccepted;}	//Feedback that the Instruction fetch uses so as not to over stuff this stage.

	//Actual machine registers. you use one of these when doing work (we do not do work)
	ActiveReg ActiveList[renameRegCount];

	//key is: {ISAReg,instance of ISAReg}
	//Value is: machine reg index (use it to access ActiveList)

	virtual ~InstDecodeStage(){}

private:
	//RETURNS FALSE IF WE RUN OUT OF ACTIVE LIST ENTRIES. WILL NOT HAVE MAPPED traceline ON FALSE RETURN
	bool Decode(traceinstruction traceline);	//return success if ok, false if something went wrong

	string regmapEntryToString(regmappair entry);
	//Use this to update the UI
	UserInterface* 		_ui;
	TraceOutputLogger* 	_plogger;

	//Need to connect to some external hardware. Namely, an ROB buffer
	ROB* 				_pROB;
	//Also need to connect to the Scheduler because according to R10K paper Decoder fills its
	//Instruction queues
	InstSchedStage* 	_pScheduler;

	//Check this when doing anything. if it is true, you must stall because of a mispredict
	bool*				_stallInput;

	//machine registers in the ActiveList that are available to be assigned to ISA regs for working.
	vector<freeRegList>* _FreeRegList;

	//Instruction decode register renaming structures
	vector<regmaptable>* 	_RegMapTable;

	//BastardChild
	vector<ISAreginstance>* _ISAInstanceRegmap;

	//D and Q variable
	//Clock related stuff D tracelines (these are the input side of the register)
	vector<traceinstruction> _DTraceLines;

	//The 4 tracelines to work on (these are the output side of the stage's register, which we work on)
	vector<traceinstruction> _QTraceLines;

	int 			_traceLinesAccepted;	//Number of tracelines accepted last cycle
	int				_tracebufhead;

};

} /* namespace R10k */

#endif /* INSTDECODE_H_ */
