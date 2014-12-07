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

using namespace std;
using namespace R10k;

int main() {

	//User interface
	UserInterface UI;	//Creates some terminals and initializes the terminal widgets bla bla

	int simulatorloop = 0;

//DEBUG INSTANTIATE ROB AND FETCH STAGE
	freeRegList 		FreeRegList;				//Shared Hardware between Decode ROB
	regmaptable 		r10kRegisterMap;			//Shared hardware
	ISAreginstance 		r10kRegMapDisambiguator;	//Shared hardware between ROB Decode //this should wrap around, so the ROB may not be larger than 2^16


	ROB 				r10kROB(&UI,					//Shared User interface
								&FreeRegList,
								&r10kRegisterMap	);	//Shared Hardware

	InstPipeStage 		r10kExecutionPipes(	&UI,
											&r10kROB);

	InstSchedStage 		r10kScheduler(	&UI,
										&r10kROB,					//Must resolve met dependencies using the ROB
										&r10kExecutionPipes);		//Scheduler must fill pipes.

	InstDecodeStage 	debugme(&UI,
								&r10kScheduler,			//Decoder must fill the Scheduler queues.
								&r10kROB,				//Decoder must make new entries in ROB
								&FreeRegList,
								&r10kRegisterMap,
								&r10kRegMapDisambiguator);	//Needs to be able to add decoded instructions to the r10k ROB

	InstFetchStage		r10kFetch(	&UI,
									&debugme,
									"intrace.txt");		//Debug file only

	//Redirect system IO to terminal (cout and printf)
	redirectStdinIO(UI);
	//TODO Make use of second debug terminal redirectallIO(UI)

	simulatorloop = 14;

	while(simulatorloop--)
	{
		cerr << "cycle :" << simulatorloop << endl;

		//IF CK
		r10kFetch.risingEdge();
		//ID CK
		debugme.risingEdge();							//Clock in data from SetDtraces and refresh output from last cycle (nothing
		//EX CK
		r10kExecutionPipes.risingEdge();
		//ROB CK (COMMIT ?)
		r10kROB.risingEdge();//TODO only allow the pipes calc to have effect at this point


		r10kScheduler.risingEdge();
		//r10kScheduler.printInstructionQueues();
		//END ALL CK
	//IF CALC
		r10kFetch.calc();
	//ID CALC
		debugme.calc();								//Do calculation on data just clocked in
	//SCHED CALC
		r10kROB.fallingEdge();//this ensures all ROB activity is done before  scheduling
		r10kScheduler.calc();	//see r10k paper
		//r10kScheduler.printInstructionQueues();
	//EX CALC
		r10kExecutionPipes.calc();

	}

	cerr << "Columbiano yea you know I lu dat" << endl; // prints !!!Hello World!!!
	return 0;
}
