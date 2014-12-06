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

using namespace logic;


namespace R10k {

class InstDecodeStage {
public:

	/*Initialize FreeRegList where all renameReg's are free*/
	InstDecodeStage(	UserInterface* 	ui,
						InstSchedStage* pScheduler,
						ROB* 			pROB,
						freeRegList* 	pFreeRegList,
						regmaptable* 	pRegMapTable,
						ISAreginstance* pMaxISAregmap)
	{
		int i;
		_ui 				= ui;
		_pScheduler 		= pScheduler;
		_pROB 				= pROB;
		_FreeRegList 		= pFreeRegList;
		_RegMapTable 		= pRegMapTable;
		_ISAInstanceRegmap 	= pMaxISAregmap;		//this should wrap around, so the ROB may not be larger than 2^16

		for(i = 0; i< renameRegCount; i++){_FreeRegList->push(i);}						//Initialize free reg list
		for(i = 0; i< ISARegCount; i++){_ISAInstanceRegmap->insert(ISAreginstancepair(i,0));} //Initialize reginstance to 0 for all ISAReg's
	}

//TODO:	void toggleclk();
	//TODO swizzle registers
	//Blit the appropriate widget
	void risingEdge();

	//RETURNS FALSE IF WE RUN OUT OF ACTIVE LIST ENTRIES. WILL NOT HAVE MAPPED traceline ON FALSE RETURN
	bool Decode(traceinstruction traceline);	//return success if ok, false if something went wrong

	//Actual machine registers. you use one of these when doing work (we do not do work)
	ActiveReg ActiveList[renameRegCount];

	//key is: {ISAReg,instance of ISAReg}
	//Value is: machine reg index (use it to access ActiveList)

	virtual ~InstDecodeStage(){}

private:
	string regmapEntryToString(regmappair entry);
	//Use this to update the UI
	UserInterface* 	_ui;

	//Need to connect to some external hardware. Namely, an ROB buffer
	ROB* 			_pROB;
	//Also need to connect to the Scheduler because according to R10K paper Decoder fills its
	//Instruction queues
	InstSchedStage* _pScheduler;

	//machine registers in the ActiveList that are available to be assigned to ISA regs for working.
	freeRegList* 	_FreeRegList;

	//Instruction decode register renaming structures
	regmaptable* 	_RegMapTable;

	//BastardChild
	ISAreginstance* _ISAInstanceRegmap;

private:
	//The thing is clocked so have some "wire representation of a trace that gets set whenever"
	//traceinstruction D_traceline;


};

} /* namespace R10k */

#endif /* INSTDECODE_H_ */
