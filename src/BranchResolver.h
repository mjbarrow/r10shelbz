/*
 * BranchResolver.h
 *
 *  Created on: Dec 16, 2014
 *      Author: king
 */

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "utils.h"			//Needs to be changed to hardware.h or something...
#include "instructions.h"
#include "userinterface.h"
#include "TraceOutputLogger.h"
//#include "InstSched.h"

#ifndef BRANCHRESOLVERZ_H_
#define BRANCHRESOLVERZ_H_



namespace R10k {

typedef struct
{
	long long int address;
	traceinstruction branch;
}instfetchstate;

//to execute branches out of order, the stack should be mapped
class BranchResolver {
public:
	BranchResolver(	vector <freeRegList>* 		FreeRegList,
					vector <regmaptable>*  		r10kRegisterMap,
					vector <ISAreginstance>*  	r10kRegMapDisambiguator,
					vector<FPQueue>*			FPInstructionQueue,
					vector<ALUQueue>*			ALUInstructionQueue,
					vector<LSQueue>*			LSInstructionQueue,
					vector< vector<traceinstruction> >* FPMpipe,
					vector< vector<traceinstruction> >* FPApipe,
					vector< vector<traceinstruction> >* ALU1pipe,
					vector< vector<traceinstruction> >* ALU2pipe,
					vector< vector<traceinstruction> >* LS1pipe				)
	{
		_branchBuffIndex 		= 1;						//Buffers are indexes 1-4, maps are indexes 0.
		_FreeRegList 			= FreeRegList;				//Shared Hardware between Decode ROB
		_r10kRegisterMap 		= r10kRegisterMap;			//Shared hardware
		_r10kRegMapDisambiguator= r10kRegMapDisambiguator;
		_FPInstructionQueue		= FPInstructionQueue;
		_ALUInstructionQueue	= ALUInstructionQueue;
		_LSInstructionQueue		= LSInstructionQueue;
		_FPMpipe				= FPMpipe;
		_FPApipe				= FPApipe;
		_ALU1pipe				= FPApipe;
		_ALU2pipe				= ALU2pipe;
		_LS1pipe				= LS1pipe;

		_instfetchstateidx		= 0;

		//Don't know how to init these with a size..
		_DecodeState.push_back(vector<traceinstruction>());//(BRANCHBUFCOUNT,vector<traceinstruction>());
		_DecodeState.push_back(vector<traceinstruction>());
		_DecodeState.push_back(vector<traceinstruction>());
		_DecodeState.push_back(vector<traceinstruction>());
		_DecodeState.push_back(vector<traceinstruction>());
		//As above
		_branchstackmap.push_back(traceinstruction());	//[BRANCHBUFCOUNT]
		_branchstackmap.push_back(traceinstruction());
		_branchstackmap.push_back(traceinstruction());
		_branchstackmap.push_back(traceinstruction());
		_branchstackmap.push_back(traceinstruction());
	}

	//Always let the simulator work at index 0.
	//This class juggles around copies at indexes 1-4
	void fallingEdge(){}

	//All of this is from the decoder.
	//void BranchResolver::detectBranch(	traceinstruction branch,
	//									vector<traceinstruction> DecodeState,
	//									long long int FetchState				)
	void detectBranch(traceinstruction branch);

	void mispredictRollback(traceinstruction branch);

	//Log the address of a branch
//DEBUG TO USE
	void logBranchAddress(long long int address,traceinstruction branch)
	{
		_FetchState[_instfetchstateidx].address = address;
		_FetchState[_instfetchstateidx].branch = branch;
		_instfetchstateidx++;
		_instfetchstateidx &= 2;
	}

	//Should be called by the Instruction fetch stage
	void instFetchRollbackHelper(){}

	virtual ~BranchResolver(){}
private:
	int _branchBuffIndex;


	vector<traceinstruction> _branchstackmap;

	//GP Register file state
	vector <freeRegList>* 		_FreeRegList;				//Shared Hardware between Decode ROB
	vector <regmaptable>*  		_r10kRegisterMap;			//Shared hardware
	vector <ISAreginstance>*  	_r10kRegMapDisambiguator;	//Shared hardware between ROB Decode //this should wrap around, so the ROB may not be larger than 2^16
	//Scheduler state
	vector<FPQueue>*			_FPInstructionQueue;
	vector<ALUQueue>*			_ALUInstructionQueue;
	vector<LSQueue>*			_LSInstructionQueue;
	//Execution pipe state
	vector< vector<traceinstruction> >* _FPMpipe;
	vector< vector<traceinstruction> >* _FPApipe;
	vector< vector<traceinstruction> >* _ALU1pipe;
	vector< vector<traceinstruction> >* _ALU2pipe;
	vector< vector<traceinstruction> >* _LS1pipe;
	//Decode stage state, used when restoring from a branch
	vector< vector<traceinstruction> >	_DecodeState;
	instfetchstate						_FetchState[BRANCHBUFCOUNT];
	int _instfetchstateidx;
//	int									_lastMispredict
};

} /* namespace R10k */

#endif /* BRANCHRESOLVER_H_ */
