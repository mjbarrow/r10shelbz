//============================================================================
// Name        : CSE240AProj.cpp
// Author      : MBarrow
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

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

	BranchResolver		r10kBrUnit( &FreeRegList,				//Used to manage the branch stack.
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

	ROB 				r10kCommit(	&UI,					//Shared User interface
									&uiPipeLineDiagram,
									&FreeRegList,
									&r10kRegisterMap	);	//Shared Hardware

	InstPipeStage 		r10kExecute(	&UI,
										&uiPipeLineDiagram,
										&r10kCommit,
										&r10kBrUnit,
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
										&r10kExecute);		//Scheduler must fill pipes.

	InstDecodeStage 	r10kDecode(&UI,
								&uiPipeLineDiagram,
								&r10kSchedule,			//Decoder must fill the Scheduler queues.
								&r10kCommit,				//Decoder must make new entries in ROB
							//	&r10kBrUnit,
								&FreeRegList,
								&r10kRegisterMap,
								&r10kRegMapDisambiguator);	//Needs to be able to add decoded instructions to the r10k ROB

	InstFetchStage		r10kFetch(	&UI,
									&r10kBrUnit,
									&r10kDecode,
									&uiPipeLineDiagram,
									"intrace.txt");		//Debug file only



	//Redirect system IO to terminal (cout and printf)
	redirectStdinIO(UI);
	//TODO Make use of second debug terminal redirectallIO(UI)

	simulatorloop = 0;

	while(simulatorloop++ < 30)
	{
		cerr << "cycle :" << simulatorloop << endl;
	//CLOCK
		//IF CK
		r10kFetch.risingEdge();
		//ID CK
		r10kDecode.risingEdge();							//Clock in data from SetDtraces and refresh output from last cycle (nothing
		//SCHED
		r10kSchedule.risingEdge();
		//EX CK
		r10kExecute.risingEdge();
		//COMMIT (ROB)
		r10kCommit.risingEdge();
		//Pipeline diagram also needs to clock
		uiPipeLineDiagram.risingEdge();
//FALLING EDGE COMPLETES BEFORE CALC
		r10kCommit.fallingEdge();		//this ensures all ROB activity is done before  scheduling
		r10kBrUnit.fallingEdge();
		r10kSchedule.fallingEdge();		//cancel any scheduled instructions on mispredict

	//END CLOCK
	//CALC
		//IF CALC
		r10kFetch.calc();
		//ID CALC
		r10kDecode.calc();

		//EX CALC
		r10kExecute.calc();
		//SCHED CALC
		r10kSchedule.calc();	//see r10k paper, do scheduling based on registers just clocked in to Activelist/ROB
		//EX CALC
//Moved for Branch rollback		r10kExecute.calc();
		//COMMIT CALC (ROB).
		uiPipeLineDiagram.fallingEdge();//Just draw out the present status of the pipeline

	//END CALC
	}

	cerr << "Columbiano yea you know I lu dat" << endl; // prints !!!Hello World!!!
	return 0;
}
