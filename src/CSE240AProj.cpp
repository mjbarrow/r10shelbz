//============================================================================
// Name        : CSE240AProj.cpp
// Author      : MBarrow
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <limits>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "utils.h"			//Needs to be changed to hardware.h or something...
#include "instructions.h"
#include "InstFetch.h"
#include "InstDecode.h"
#include "InstSched.h"
#include "ROB.h"
#include "userinterface.h"
#include "TraceOutputLogger.h"
#include "BranchResolver.h"

using namespace std;
using namespace R10k;

int main() {

	//User interface
	UserInterface UI;	//Creates some terminals and initializes the terminal widgets bla bla

	int simulatorloop = 0;
	int cycle =0;
	int brk = 1;			//Character used to control the program

	TraceOutputLogger	uiPipeLineDiagram(	&UI,
											"outtrace.txt");		//Writes out the pipeline diagram of execution

//DEBUG INSTANTIATE ROB AND FETCH STAGE

	//Most queues and register files are centralized because
	//The branch mispredict backend has to manage a stack of them as per the r10k paper
	//There are 5 copies of everything. Why? a stack of 4 to use and the current working set.

	//GP register files
	vector <freeRegList> 		FreeRegList(BRANCHBUFCOUNT +1,freeRegList());				//Shared Hardware between Decode ROB
	vector <regmaptable>  		r10kRegisterMap(BRANCHBUFCOUNT +1,regmaptable());			//Shared hardware
	vector <ISAreginstance>  	r10kRegMapDisambiguator(BRANCHBUFCOUNT +1,ISAreginstance());	//Shared hardware between ROB Decode //this should wrap around, so the ROB may not be larger than 2^16

	//Scheduler register files
	vector<FPQueue> 	FPInstructionQueue(BRANCHBUFCOUNT +1,FPQueue());		//Execution queue stack
	vector<ALUQueue>	ALUInstructionQueue(BRANCHBUFCOUNT +1,ALUQueue());	//The branch resolution unit.
	vector<LSQueue>		LSInstructionQueue(BRANCHBUFCOUNT +1,LSQueue());		//They are instructions to be executed.

	vector< vector<traceinstruction> > FPMpipe(BRANCHBUFCOUNT +1,vector<traceinstruction>());
	vector< vector<traceinstruction> > FPApipe(BRANCHBUFCOUNT +1,vector<traceinstruction>());
	vector< vector<traceinstruction> > ALU1pipe(BRANCHBUFCOUNT +1,vector<traceinstruction>());
	vector< vector<traceinstruction> > ALU2pipe(BRANCHBUFCOUNT +1,vector<traceinstruction>());
	vector< vector<traceinstruction> > LS1pipe(BRANCHBUFCOUNT +1,vector<traceinstruction>());

	//Control signals (a wire basically)
	bool ExStageStall = false;	//This is global because it is like a global net.

	ROB 				r10kCommit(	&UI,					//Shared User interface
									&uiPipeLineDiagram,
									&FreeRegList,
									&r10kRegisterMap	);	//Shared Hardware

	BranchResolver		r10kBrUnit( &r10kCommit,			//Used to clean up restored queues of instructions
									&FreeRegList,				//Used to manage the branch stack.
									&r10kRegisterMap,
									&r10kRegMapDisambiguator,
									&FPInstructionQueue,
									&ALUInstructionQueue,
									&LSInstructionQueue,
									&FPMpipe,
									&FPApipe,
									&ALU1pipe,
									&ALU2pipe,
									&LS1pipe								);

	InstPipeStage 		r10kExecute(	&UI,
										&uiPipeLineDiagram,
										&r10kCommit,
										&r10kBrUnit,
										&ExStageStall,
										&FPMpipe,
										&FPApipe,
										&ALU1pipe,
										&ALU2pipe,
										&LS1pipe			);

	InstSchedStage 		r10kSchedule(	&UI,
										&uiPipeLineDiagram,
										&FPInstructionQueue,
										&ALUInstructionQueue,
										&LSInstructionQueue,
										&r10kCommit,		//Must resolve met dependencies using the ROB
										&r10kBrUnit,
										&ExStageStall,
										&r10kExecute);		//Scheduler must fill pipes.

	InstDecodeStage 	r10kDecode(&UI,
								&uiPipeLineDiagram,
								&r10kSchedule,			//Decoder must fill the Scheduler queues.
								&r10kCommit,				//Decoder must make new entries in ROB
								&ExStageStall,
								&FreeRegList,
								&r10kRegisterMap,
								&r10kRegMapDisambiguator);	//Needs to be able to add decoded instructions to the r10k ROB

	InstFetchStage		r10kFetch(	&UI,
									&r10kBrUnit,
									&ExStageStall,
									&r10kDecode,
									&uiPipeLineDiagram,
									"intrace.txt");		//Debug file only



	//Redirect system IO to terminal (cout and printf)
	redirectStdinIO(UI);
	//TODO Make use of second debug terminal redirectallIO(UI)

	simulatorloop = 0;
	while(simulatorloop < 15)	//do an extra 30 instructions or so after end
	{
		cerr << "cycle :" << cycle++ << endl;
		//================================================
		//			_|	RISING CLOCK	_|
		//================================================
		r10kFetch.risingEdge();			//IF CK
		r10kDecode.risingEdge();		//ID CK					//Clock in data from SetDtraces and refresh output from last cycle (nothing
		r10kSchedule.risingEdge();		//SCHED
		r10kExecute.risingEdge();		//EX CK
		r10kCommit.risingEdge();		//COMMIT (ROB)
		uiPipeLineDiagram.risingEdge();	//Pipeline diagram also needs to clock
		//================================================
		//			|_	FALLING	CLOCK	|_
		//================================================
		r10kCommit.fallingEdge();		//this ensures all ROB activity is done before  scheduling
		r10kExecute.fallingEdge();		//this will tell F,S and D stages not to clear ports
										//and stall by asserting ExStageStall
		r10kSchedule.fallingEdge();		//cancel any scheduled instructions on mispredict
		//================================================
		//			(A V B)	LOGIC (A V B)
		//================================================
		r10kFetch.calc();		//IF CALC
		r10kDecode.calc();		//ID CALC
		r10kExecute.calc();		//EX CALC
		r10kSchedule.calc();	//SCHED CALC (Note, use registers clocked to ROB on neg edge, see paper)

		//===============================================
		//			+++	USER INTERFACE +++
		//===============================================
		uiPipeLineDiagram.fallingEdge();//Just draw out the present status of the pipeline

		//single step or break simulator flow
		if(	(brk != simulatorloop) ||
			(brk < simulatorloop)		)	//type a to run in auto mode
			scanf ("%d",&brk);				//read in next breakpoint

		if(!r10kFetch.didFetchAll())
			simulatorloop++;
	}

	cerr << "Columbiano yea you know I lu dat" << endl; // prints !!!Hello World!!!
	return 0;
}
