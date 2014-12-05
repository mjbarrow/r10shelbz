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


using namespace std;
using namespace R10k;

int main() {

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


	ROB 				r10kROB(&FreeRegList,
								&r10kRegisterMap/*,
								&r10kRegMapDisambiguator*/);		//Shared Hardware

	InstPipeStage 		r10kExecutionPipes(&r10kROB);

	InstSchedStage 		r10kScheduler(	&r10kROB,					//Must resolve met dependencies using the ROB
										&r10kExecutionPipes);		//Scheduler must fill pipes.

	InstDecodeStage 	debugme(&r10kScheduler,			//Decoder must fill the Scheduler queues.
								&r10kROB,				//Decoder must make new entries in ROB
								&FreeRegList,
								&r10kRegisterMap,
								&r10kRegMapDisambiguator);	//Needs to be able to add decoded instructions to the r10k ROB


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
//		r10kROB.printRobEntry(tracelinenumber);						//This should show the newly added trace line in the ROB

		//SCHEDULE ANY INSTRUCTIONS POSSIBLE (SHOULD TAKE INPUT FROM RETIREMENT BUFFER)
		//debugme.Schedule()
//}LOGIC
		//Clock
		//debugme.Clk()

//DEBUG SCHEDULER WITH INFLIGHT (EARLY DEBUG)
/*		if(r10kScheduler.promoQueueToPipe())
			cout << "successful queue promotion" << endl;
		else
			cout << "queue promotion failed" << endl;
*/
		tracelinenumber++;
//TO DO: all emulation :(
	}

	cout << "ROB after decoding all instructions" << endl;
	r10kROB.printRob();

//DEBUG, CHECK THE PROMOTION OF THE SCHEDULER WORKED
	cout << "Scheduler Queues before promoting any instructions to pipe:" << endl;
	r10kScheduler.printInstructionQueues();

	if(r10kScheduler.promoQueueToPipe())
		cout << "successful queue promotion" << endl;

	cout << "Scheduler Queues after promoting to Pipe (NO retired): " << endl;
	r10kScheduler.printInstructionQueues();
	cout << "Pipes after promotion:" << endl;
	cout << "----------------------------CK 1----------------------------" << endl;
	r10kExecutionPipes.RetirePipes();
	r10kExecutionPipes.print();						//RETIRES TO ROB


/*	debug = 0;
	while(debug < 3)
	{
		r10kROB.retireEntry(debug);
		cout << "Retired trace line :" << debug << endl;
		debug++;
	}

	cout << "ROB after retiring 3 instructions" << endl;*/

//DEBUG THE INSTRUCTION PIPELINE
	cout << "Manualy stuffing Pipes" << endl;
	cout << "----------------------------CK 2----------------------------" << endl;
	r10kExecutionPipes.FPMinPort(traceinstruction());
	r10kExecutionPipes.FPAinPort(traceinstruction());
	r10kExecutionPipes.ALUAinPort(traceinstruction());
	r10kExecutionPipes.ALUBinPort(traceinstruction());
	r10kExecutionPipes.LSAinPort(traceinstruction());
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	r10kExecutionPipes.print();
	cout << "----------------------------CK 3----------------------------" << endl;
	r10kExecutionPipes.FPMinPort(traceinstruction());
	r10kExecutionPipes.FPAinPort(traceinstruction());
	r10kExecutionPipes.ALUAinPort(traceinstruction());
	r10kExecutionPipes.ALUBinPort(traceinstruction());
	r10kExecutionPipes.LSAinPort(traceinstruction());
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	r10kExecutionPipes.print();
	cout << "----------------------------CK 4----------------------------" << endl;
	r10kExecutionPipes.FPMinPort(traceinstruction());
	r10kExecutionPipes.FPAinPort(traceinstruction());
	r10kExecutionPipes.ALUAinPort(traceinstruction());
	r10kExecutionPipes.ALUBinPort(traceinstruction());
	r10kExecutionPipes.LSAinPort(traceinstruction());
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	r10kExecutionPipes.print();
	cout << "ROB after pipe stuffing" << endl;
	r10kROB.printRob();
	//Now Commit the ROB entries and free the architectural registers
	cout << endl << endl;
	cout << "================================================================"<< endl << endl;

//Check the SCHEDULER will be able to promote all instructions with dependencies
	cout << "try to schedule again, now that dependencies have been met" << endl;

	if(r10kScheduler.promoQueueToPipe())
		cout << "Scheduler: promoting instruction queues" << endl;
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	cout << "Pipes after promotion" << endl;
	r10kScheduler.printInstructionQueues();
	cout << endl << endl;
//USELESS	cout << "Printing scheduled instructions" << endl;
//USELESS	r10kScheduler.printArbitrationStruct();
	cout << "EX Pipes after promotion:" << endl;
	cout << "----------------------------CK 1----------------------------" << endl;
	r10kExecutionPipes.print();

	cout << "Manualy stuffing Pipes" << endl;
	cout << "----------------------------CK 2----------------------------" << endl;
	r10kExecutionPipes.FPMinPort(traceinstruction());
	r10kExecutionPipes.FPAinPort(traceinstruction());
	r10kExecutionPipes.ALUAinPort(traceinstruction());
	r10kExecutionPipes.ALUBinPort(traceinstruction());
	r10kExecutionPipes.LSAinPort(traceinstruction());
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	r10kExecutionPipes.print();
	cout << "----------------------------CK 3----------------------------" << endl;
	r10kExecutionPipes.FPMinPort(traceinstruction());
	r10kExecutionPipes.FPAinPort(traceinstruction());
	r10kExecutionPipes.ALUAinPort(traceinstruction());
	r10kExecutionPipes.ALUBinPort(traceinstruction());
	r10kExecutionPipes.LSAinPort(traceinstruction());
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	r10kExecutionPipes.print();
	cout << "----------------------------CK 4----------------------------" << endl;
	r10kExecutionPipes.FPMinPort(traceinstruction());
	r10kExecutionPipes.FPAinPort(traceinstruction());
	r10kExecutionPipes.ALUAinPort(traceinstruction());
	r10kExecutionPipes.ALUBinPort(traceinstruction());
	r10kExecutionPipes.LSAinPort(traceinstruction());
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	r10kExecutionPipes.print();
	cout << "ROB after pipe stuffing" << endl;
	r10kROB.printRob();
//DEBUG TO CHECK COMMIT WORKS
	r10kROB.commitTailInstructions(4);

	cout << "ROB after attempting to commit 4 instructions" << endl;
	r10kROB.printRob();

	cout << "With trace 1, this will not commit 4 because the last instruction has a dependence on the third" << endl;

	cout << "================================================================================================" <<endl;

	cout << "try to schedule LS stage again, now that dependencies have been met" << endl;

	if(r10kScheduler.promoQueueToPipe())
		cout << "Scheduler: promoting instruction queues" << endl;
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	cout << "Pipes after promotion" << endl;
	r10kScheduler.printInstructionQueues();
	cout << endl << endl;
	cout << "EX Pipes after promotion:" << endl;
	cout << "----------------------------CK 1----------------------------" << endl;
	r10kExecutionPipes.print();

	cout << "Manualy stuffing Pipes" << endl;
	cout << "----------------------------CK 2----------------------------" << endl;
	r10kExecutionPipes.FPMinPort(traceinstruction());
	r10kExecutionPipes.FPAinPort(traceinstruction());
	r10kExecutionPipes.ALUAinPort(traceinstruction());
	r10kExecutionPipes.ALUBinPort(traceinstruction());
	r10kExecutionPipes.LSAinPort(traceinstruction());
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	r10kExecutionPipes.print();
	cout << "----------------------------CK 3----------------------------" << endl;
	r10kExecutionPipes.FPMinPort(traceinstruction());
	r10kExecutionPipes.FPAinPort(traceinstruction());
	r10kExecutionPipes.ALUAinPort(traceinstruction());
	r10kExecutionPipes.ALUBinPort(traceinstruction());
	r10kExecutionPipes.LSAinPort(traceinstruction());
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	r10kExecutionPipes.print();
	cout << "----------------------------CK 4----------------------------" << endl;
	r10kExecutionPipes.FPMinPort(traceinstruction());
	r10kExecutionPipes.FPAinPort(traceinstruction());
	r10kExecutionPipes.ALUAinPort(traceinstruction());
	r10kExecutionPipes.ALUBinPort(traceinstruction());
	r10kExecutionPipes.LSAinPort(traceinstruction());
	r10kExecutionPipes.RetirePipes();					//RETIRES TO ROB
	r10kExecutionPipes.print();
	cout << "ROB after pipe stuffing" << endl;
	r10kROB.printRob();
//DEBUG TO CHECK COMMIT WORKS
	r10kROB.commitTailInstructions(4);

	cout << "ROB after final commit" << endl;
	r10kROB.commitTailInstructions(4);


	cout << endl;
//DEBUG ONLY, SEE WHAT registers are free
/*	cout << "r10k reg map" << endl;
	r10kRegisterMap.print();
	cout << "r10k free reg list" << endl;
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


	cout << "Columbiano yea you know I lu dat" << endl; // prints !!!Hello World!!!
	return 0;
}
