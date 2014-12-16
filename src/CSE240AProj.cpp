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

using namespace std;
using namespace R10k;

int main() {

	//User interface
	UserInterface UI;	//Creates some terminals and initializes the terminal widgets bla bla

	int simulatorloop = 0;

	TraceOutputLogger	uiPipeLineDiagram(	&UI,
											"outtrace.txt");		//Writes out the pipeline diagram of execution

//DEBUG INSTANTIATE ROB AND FETCH STAGE
	freeRegList 		FreeRegList;				//Shared Hardware between Decode ROB
	regmaptable 		r10kRegisterMap;			//Shared hardware
	ISAreginstance 		r10kRegMapDisambiguator;	//Shared hardware between ROB Decode //this should wrap around, so the ROB may not be larger than 2^16


	ROB 				r10kCommit(	&UI,					//Shared User interface
									&uiPipeLineDiagram,
									&FreeRegList,
									&r10kRegisterMap	);	//Shared Hardware

	InstPipeStage 		r10kExecute(	&UI,
										&uiPipeLineDiagram,
										&r10kCommit);

	InstSchedStage 		r10kSchedule(	&UI,
										&uiPipeLineDiagram,
										&r10kCommit,					//Must resolve met dependencies using the ROB
										&r10kExecute);		//Scheduler must fill pipes.

	InstDecodeStage 	r10kDecode(&UI,
								&uiPipeLineDiagram,
								&r10kSchedule,			//Decoder must fill the Scheduler queues.
								&r10kCommit,				//Decoder must make new entries in ROB
								&FreeRegList,
								&r10kRegisterMap,
								&r10kRegMapDisambiguator);	//Needs to be able to add decoded instructions to the r10k ROB

	InstFetchStage		r10kFetch(	&UI,
									&r10kDecode,
									&uiPipeLineDiagram,
									"intrace.txt");		//Debug file only

	//Redirect system IO to terminal (cout and printf)
	redirectStdinIO(UI);
	//TODO Make use of second debug terminal redirectallIO(UI)

	simulatorloop = 0;

	while(simulatorloop++ < 20)
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
//		uiPipeLineDiagram.fallingEdge();//Just draw out the present status of the pipeline

	//END CLOCK
	//CALC
		//IF CALC
		r10kFetch.calc();
		//ID CALC
		r10kDecode.calc();
		//SCHED CALC
		r10kSchedule.calc();	//see r10k paper, do scheduling based on registers just clocked in to Activelist/ROB
		//EX CALC
		r10kExecute.calc();
		//COMMIT CALC (ROB).
//		r10kCommit.fallingEdge();		//this ensures all ROB activity is done before  scheduling
		uiPipeLineDiagram.fallingEdge();//Just draw out the present status of the pipeline

	//END CALC
	}

	cerr << "Columbiano yea you know I lu dat" << endl; // prints !!!Hello World!!!
	return 0;
}
