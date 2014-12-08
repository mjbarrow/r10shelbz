/*
 * TraceOutputLogger.cpp
 *
 *  Created on: Dec 7, 2014
 *      Author: king
 */

#include "TraceOutputLogger.h"

using namespace std;

namespace R10k {

void TraceOutputLogger::fallingEdge()
{
	int i;
	vector<string> blitme;						//Cannot recast vectors. have to copy
	blitme.reserve(_pipelinediagram.size());	//Reserve right amount of memory

	//Add blank to all unmodified inflight
	for(i=0; i <(int)_pipelinediagram.size(); i++)
	{
		if(!_pipelinediagram[i].modified)		//if this trace was not modified
			_pipelinediagram[i] += " - ";	//represent that fact with a bubble

		_pipelinediagram[i].modified = false;	//clear the trace modified flag
												//So we don't forget to add a bubble next time
												//if needed
	}

	blitme.assign(_pipelinediagram.begin(), _pipelinediagram.end());	//Create a string version to blit

	_ui->blitTraceLogWindow(&blitme);
}

} /* namespace R10k */
