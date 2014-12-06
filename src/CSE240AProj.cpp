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
#include "InstDecode.h"
#include "InstSched.h"
#include "ROB.h"
#include "userinterface.h"


using namespace std;
using namespace R10k;

int main() {

	//User interface
	UserInterface UI;	//Creates some terminals and initializes the terminal widgets bla bla

//Debug file only
	std::vector<traceinstruction> instructionstream;
	std::string traceline;
	std::ifstream fulltrace("intrace.txt");
	int tracelinenumber = 0;
	int debug = 0;

//DEBUG INSTANTIATE ROB AND FETCH STAGE
	freeRegList 		FreeRegList;				//Shared Hardware between Decode ROB
	regmaptable 		r10kRegisterMap;			//Shared hardware
	ISAreginstance 		r10kRegMapDisambiguator;	//Shared hardware between ROB Decode //this should wrap around, so the ROB may not be larger than 2^16


	ROB 				r10kROB(&UI,					//Shared User interface
								&FreeRegList,
								&r10kRegisterMap	);	//Shared Hardware

	InstPipeStage 		r10kExecutionPipes(&r10kROB);

	InstSchedStage 		r10kScheduler(	&r10kROB,					//Must resolve met dependencies using the ROB
										&r10kExecutionPipes);		//Scheduler must fill pipes.

	InstDecodeStage 	debugme(&UI,
								&r10kScheduler,			//Decoder must fill the Scheduler queues.
								&r10kROB,				//Decoder must make new entries in ROB
								&FreeRegList,
								&r10kRegisterMap,
								&r10kRegMapDisambiguator);	//Needs to be able to add decoded instructions to the r10k ROB

	//Redirect system IO to terminal (cout and printf)
	redirectStdinIO(UI);
	//TODO Make use of second debug terminal redirectallIO(UI)

	while (std::getline(fulltrace, traceline))
	{
		//fetch next instruction to instruction stream
/*IGNORE FOR NOW		instructionstream.push_back(traceinstruction(traceline, tracelinenumber));
		if(instructionstream.size() > 6)	//Have 6 instructions in "cache" try and execute them
		{

		}*/
//{LOGIC
		//DECODE AND REMAP REGISTERS
		debugme.Decode(traceinstruction(traceline,tracelinenumber));
//DEBUG ONLY SHOULD BE SYNCED WITH ALL OTHER SYNC LOGIC
		//test blitting
		debugme.risingEdge();
//END DEBUG ONLY TO BE SYNCED WITH ALL OTHER SYNC LOGIC

//		r10kROB.printRobEntry(tracelinenumber);						//This should show the newly added trace line in the ROB

		//SCHEDULE ANY INSTRUCTIONS POSSIBLE (SHOULD TAKE INPUT FROM RETIREMENT BUFFER)
		//debugme.Schedule()
//}LOGIC
		//Clock
		//debugme.Clk()

//DEBUG SCHEDULER WITH INFLIGHT (EARLY DEBUG)
/*		if(r10kScheduler.promoQueueToPipe())
			cerr << "successful queue promotion" << endl;
		else
			cerr << "queue promotion failed" << endl;
*/
		tracelinenumber++;
//TO DO: all emulation :(
	}

	cerr << "ROB after decoding all instructions" << endl;
	r10kROB.printRob();
//DEBUG ONLY THIS SHOULD BE CALLED EVERY TIME THAT THE ROB COMBINATIONAL LOGIC FIRES
	//test the screen blitting of the UI
	r10kROB.risingEdge();
//END DEBUG ONLY

//DEBUG, CHECK THE PROMOTION OF THE SCHEDULER WORKED
	cerr << "Scheduler Queues before promoting any instructions to pipe:" << endl;
	r10kScheduler.printInstructionQueues();

	if(r10kScheduler.promoQueueToPipe())
		cerr << "successful queue promotion" << endl;

	cerr << "Scheduler Queues after promoting to Pipe (NO retired): " << endl;
	r10kScheduler.printInstructionQueues();
	cerr << "Pipes after promotion:" << endl;
	cerr << "----------------------------CK 1----------------------------" << endl;
	r10kExecutionPipes.RetirePipes();
	r10kExecutionPipes.print();						//RETIRES TO ROB


/*	debug = 0;
	while(debug < 3)
	{
		r10kROB.retireEntry(debug);
		cerr << "Retired trace line :" << debug << endl;
		debug++;
	}

	cerr << "ROB after retiring 3 instructions" << endl;*/

//DEBUG THE INSTRUCTION PIPELINE
	cerr << "Manualy stuffing Pipes" << endl;
	cerr << "----------------------------CK 2----------------------------" << endl;
	r10kExecutionPipes.FPMinPort(traceinstruction());
	r10kExecutionPipes.FPAinPort(traceinstruction());
	r10kExecutionPipes.ALUAinPort(traceinstruction());
	r10kExecutionPipes.ALUBinPort(traceinstruction());
	r10kExecutionPipes.LSAinPort(traceinstruction());
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	r10kExecutionPipes.print();
	cerr << "----------------------------CK 3----------------------------" << endl;
	r10kExecutionPipes.FPMinPort(traceinstruction());
	r10kExecutionPipes.FPAinPort(traceinstruction());
	r10kExecutionPipes.ALUAinPort(traceinstruction());
	r10kExecutionPipes.ALUBinPort(traceinstruction());
	r10kExecutionPipes.LSAinPort(traceinstruction());
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	r10kExecutionPipes.print();
	cerr << "----------------------------CK 4----------------------------" << endl;
	r10kExecutionPipes.FPMinPort(traceinstruction());
	r10kExecutionPipes.FPAinPort(traceinstruction());
	r10kExecutionPipes.ALUAinPort(traceinstruction());
	r10kExecutionPipes.ALUBinPort(traceinstruction());
	r10kExecutionPipes.LSAinPort(traceinstruction());
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	r10kExecutionPipes.print();
	cerr << "ROB after pipe stuffing" << endl;
	r10kROB.printRob();
	//Now Commit the ROB entries and free the architectural registers
	cerr << endl << endl;
	cerr << "================================================================"<< endl << endl;

//Check the SCHEDULER will be able to promote all instructions with dependencies
	cerr << "try to schedule again, now that dependencies have been met" << endl;

	if(r10kScheduler.promoQueueToPipe())
		cerr << "Scheduler: promoting instruction queues" << endl;
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	cerr << "Pipes after promotion" << endl;
	r10kScheduler.printInstructionQueues();
	cerr << endl << endl;
//USELESS	cerr << "Printing scheduled instructions" << endl;
//USELESS	r10kScheduler.printArbitrationStruct();
	cerr << "EX Pipes after promotion:" << endl;
	cerr << "----------------------------CK 1----------------------------" << endl;
	r10kExecutionPipes.print();

	cerr << "Manualy stuffing Pipes" << endl;
	cerr << "----------------------------CK 2----------------------------" << endl;
	r10kExecutionPipes.FPMinPort(traceinstruction());
	r10kExecutionPipes.FPAinPort(traceinstruction());
	r10kExecutionPipes.ALUAinPort(traceinstruction());
	r10kExecutionPipes.ALUBinPort(traceinstruction());
	r10kExecutionPipes.LSAinPort(traceinstruction());
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	r10kExecutionPipes.print();
	cerr << "----------------------------CK 3----------------------------" << endl;
	r10kExecutionPipes.FPMinPort(traceinstruction());
	r10kExecutionPipes.FPAinPort(traceinstruction());
	r10kExecutionPipes.ALUAinPort(traceinstruction());
	r10kExecutionPipes.ALUBinPort(traceinstruction());
	r10kExecutionPipes.LSAinPort(traceinstruction());
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	r10kExecutionPipes.print();
	cerr << "----------------------------CK 4----------------------------" << endl;
	r10kExecutionPipes.FPMinPort(traceinstruction());
	r10kExecutionPipes.FPAinPort(traceinstruction());
	r10kExecutionPipes.ALUAinPort(traceinstruction());
	r10kExecutionPipes.ALUBinPort(traceinstruction());
	r10kExecutionPipes.LSAinPort(traceinstruction());
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	r10kExecutionPipes.print();
	cerr << "ROB after pipe stuffing" << endl;
	r10kROB.printRob();
//DEBUG TO CHECK COMMIT WORKS
	r10kROB.commitTailInstructions(4);

	cerr << "ROB after attempting to commit 4 instructions" << endl;
	r10kROB.printRob();

	cerr << "With trace 1, this will not commit 4 because the last instruction has a dependence on the third" << endl;

	cerr << "================================================================================================" <<endl;

	cerr << "try to schedule LS stage again, now that dependencies have been met" << endl;

	if(r10kScheduler.promoQueueToPipe())
		cerr << "Scheduler: promoting instruction queues" << endl;
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	cerr << "Pipes after promotion" << endl;
	r10kScheduler.printInstructionQueues();
	cerr << endl << endl;
	cerr << "EX Pipes after promotion:" << endl;
	cerr << "----------------------------CK 1----------------------------" << endl;
	r10kExecutionPipes.print();

	cerr << "Manualy stuffing Pipes" << endl;
	cerr << "----------------------------CK 2----------------------------" << endl;
	r10kExecutionPipes.FPMinPort(traceinstruction());
	r10kExecutionPipes.FPAinPort(traceinstruction());
	r10kExecutionPipes.ALUAinPort(traceinstruction());
	r10kExecutionPipes.ALUBinPort(traceinstruction());
	r10kExecutionPipes.LSAinPort(traceinstruction());
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	r10kExecutionPipes.print();
	cerr << "----------------------------CK 3----------------------------" << endl;
	r10kExecutionPipes.FPMinPort(traceinstruction());
	r10kExecutionPipes.FPAinPort(traceinstruction());
	r10kExecutionPipes.ALUAinPort(traceinstruction());
	r10kExecutionPipes.ALUBinPort(traceinstruction());
	r10kExecutionPipes.LSAinPort(traceinstruction());
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	r10kExecutionPipes.print();
	cerr << "----------------------------CK 4----------------------------" << endl;
	r10kExecutionPipes.FPMinPort(traceinstruction());
	r10kExecutionPipes.FPAinPort(traceinstruction());
	r10kExecutionPipes.ALUAinPort(traceinstruction());
	r10kExecutionPipes.ALUBinPort(traceinstruction());
	r10kExecutionPipes.LSAinPort(traceinstruction());
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	r10kExecutionPipes.print();
	cerr << "ROB after pipe stuffing" << endl;
	r10kROB.printRob();
//DEBUG TO CHECK COMMIT WORKS
	r10kROB.commitTailInstructions(4);

	cerr << "ROB after final commit" << endl;
	r10kROB.commitTailInstructions(4);


	cerr << endl;
//DEBUG ONLY, SEE WHAT registers are free
/*	cerr << "r10k reg map" << endl;
	r10kRegisterMap.print();
	cerr << "r10k free reg list" << endl;
	FreeRegList.print();*/

	debug = 0;
/*	while(tracelinenumber--)
	{
//TODO: ALL OF THIS LOGIC MUST BE FULLY DEBUGGED!!!!

		//No instruction will successfully commit unless it is retired
		r10kROB.retireEntry(debug);
		//Commit the first tail instruction
		r10kROB.commitTailInstructions(1);
		//verify that the tail instruction was committed
		r10kROB.printRobEntry(debug);
		debug++;
	}*/


	cerr << "Columbiano yea you know I lu dat" << endl; // prints !!!Hello World!!!
	return 0;
}
