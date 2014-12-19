/*
 * InstFetch.cpp
 *
 *  Created on: Dec 7, 2014
 *      Author: king
 */

#include "InstFetch.h"

namespace R10k {


void InstFetchStage::risingEdge()
{
	vector<string> stringInstList;
	std::ostringstream oss;
	unsigned int i;

	//Do nothing if stalling
	//On Branch Mispredict
	if(*_stallInput)
		return;

	oss << "<C></4>cyc: " << _cycle;	//Display cycle somewhere
	stringInstList.push_back(oss.str());

	//Construct the correct type of input for the blit function
	//An array of strings which it will blit out
	for(i = 0; i < instructionstream.size(); i++)
	{
		stringInstList.push_back(traceInstructionToString(i));
	}
	if(stringInstList.size() == 0)
		stringInstList.push_back("empty");

	//trigger the blit function so that the screen output is refreshed of the ROB content
	_ui->blitInstructionList(&stringInstList);

	//Clear all of the instruction stream for next time
	instructionstream = vector<traceinstruction>();	//CLEAN UP THE INSTRUCTION STREAM
}

void InstFetchStage::calc()
{
	int loop = INSTDECODEPERCYCLE;
	std::string traceline;
	traceinstruction currentInstruction;

	//Do nothing if stalling on branch mispredict
	if(*_stallInput)
		return;

//take in feedback from the decode stage or whatever to limit the number of
//Instructions to pull in
	loop -= _pdecstage->getLinesAccepted();

	//Normal instruction decode flow
	while (loop < INSTDECODEPERCYCLE)
	{
		if(!getline(fulltrace, traceline))
			{_fetchEnd = true; break;}

		currentInstruction = traceinstruction(traceline,_tracelinenumber);

		instructionstream.push_back(currentInstruction);
		_plogger->logIFTrace(_tracelinenumber);				//Log the instruction fetch for the pipeline diagram

		loop++;
		_tracelinenumber++;
	}
	_pdecstage->setDTraceLines(&instructionstream);

}

//==================================================================================================
//										UTILITY FUNCTIONS
//==================================================================================================
//Convert the buffer line into a string. Usefull for debug.
//See: http://stackoverflow.com/questions/3203452/how-to-read-entire-stream-into-a-stdstring
string InstFetchStage::traceInstructionToString(int entry)
{
	string instructionString;
	ostringstream os;
	traceinstruction instruction = instructionstream[entry];


	os 		<< "|tr#:" << instruction.traceLineNo
			<<hex<<"| 0x"<< instruction.rd << " <- " <<hex<<"0x"<< instruction.rs << " "+instruction.strOp+" isa reg: " <<hex<<"0x"<< instruction.rt
			<< "|";

	instructionString = os.str();

	return instructionString;
}


} /* namespace R10k */
