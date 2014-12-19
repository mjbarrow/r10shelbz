#ifndef _USER_INTERFACE_H
#define _USER_INTERFACE_H


#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <curses.h>
#include <sys/ioctl.h>
#include "/usr/include/cdk/cdk.h"

#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include "InterfaceLayout.h"	//all offsets of the UI

using namespace std;

//==================================================================================================
//										HELPER MACROS
//==================================================================================================


#define redirectAllIO(UIinstance) 	std::ofstream out((UIinstance.getstdout_term()).c_str());	\
									std::cout.rdbuf(out.rdbuf());							\
									std::ifstream in((UIinstance.getstdout_term()).c_str());	\
									std::cin.rdbuf(in.rdbuf());							\
									std::ofstream err((UIinstance. getstderr_term()).c_str());	\
									std::cerr.rdbuf(err.rdbuf());							\
									std::cout << "INTERFACE STREAM READY!" << std::endl;	\
									std::cerr << "ERROR STREAM READY!" << std::endl;		\

#define redirectStdinIO(UIinstance) std::ofstream out((UIinstance.getstdout_term()).c_str());	\
									std::cout.rdbuf(out.rdbuf());							\
									std::cin.rdbuf(out.rdbuf());							\
									/*std::cout << "INTERFACE STREAM READY!" << std::endl;*/\
									std::cerr << "ERROR STREAM READY!" << std::endl;		\

//====================================================================================================
//									UI WIDGET TYPEDEFS
//====================================================================================================

typedef struct _UIWIDGETS
{
//INSTRUCTION FETCH STUFF
	CDKSCROLL *instructionlist;
	CDKSCROLL *tracewindow;
//COMMIT LIKE STUFF
	CDKSCROLL *ROBList;
//SCHEDULE STUFF
	CDKSCROLL *freelist;
	//RETIRED WIDGET CDKSCROLL *activelist;
	CDKSCROLL *regmaptable;	//http://www.fifi.org/doc/libcdk-dev/examples/swindow_ex.c
//DECODE STUFF
	CDKSCROLL *fpQueue;
	CDKSCROLL *addressQueue;
	CDKSCROLL *integerQueue;
 //PIPES
	CDKSCROLL *fpMPipe;
	CDKSCROLL *fpAPipe;
	CDKSCROLL *ALUAPipe;
	CDKSCROLL *ALUBPipe;
	CDKSCROLL *LSPipe;
//PIPETRACE WIDGIT
	CDKSCROLL *tracelogwindow;

	   _UIWIDGETS()
	   {
		   instructionlist=(CDKSCROLL *)NULL;
		   tracewindow	= (CDKSCROLL *)NULL;
		   ROBList		= (CDKSCROLL *)NULL;
		   freelist		= (CDKSCROLL *)NULL;
		   //RETIRED WIDGET activelist	= (CDKSCROLL *)NULL;
		   regmaptable	= (CDKSCROLL *)NULL;	//http://www.fifi.org/doc/libcdk-dev/examples/swindow_ex.c
		   fpQueue		= (CDKSCROLL *)NULL;
		   addressQueue	= (CDKSCROLL *)NULL;
		   integerQueue	= (CDKSCROLL *)NULL;
		   fpMPipe		= (CDKSCROLL *)NULL;
		   fpAPipe		= (CDKSCROLL *)NULL;
		   ALUAPipe		= (CDKSCROLL *)NULL;
		   ALUBPipe		= (CDKSCROLL *)NULL;
		   LSPipe		= (CDKSCROLL *)NULL;
		   tracelogwindow=(CDKSCROLL *)NULL;
	   }
}uiWidgets, *puiWidgets;


typedef struct _UIWIDGETSITEMS
{
	const char** 	instructionlistItems;
	const char** 	tracewindowItems;
	//ROB STUFF
	const char**	ROBListItems;
	//SCHEDULE STUFF
	const char** 	freelistItems;
	//RETIRED WIDGET const char** 	activelistItems;
	const char**	regmaptableItems;
	//DECODE STUFF
	const char**	fpQueueItems;
	const char**	addressQueueItems;
	const char**	integerQueueItems;
	//PIPES
	const char**	fpMPipeItems;
	const char**	fpAPipeItems;
	const char**	ALUAPipeItems;
	const char**	ALUBPipeItems;
	const char**	LSPipeItems;
	//PIPE TRACE WIDGIT
	const char**	traceLogWindowItems;

	_UIWIDGETSITEMS()
	{
		instructionlistItems	= NULL;
		tracewindowItems 		= NULL;
		ROBListItems			= NULL;
		freelistItems			= NULL;
		//RETIRED WIDGET activelistItems			= NULL;
		regmaptableItems		= NULL;
		fpQueueItems			= NULL;
		addressQueueItems		= NULL;
		integerQueueItems		= NULL;
		fpMPipeItems			= NULL;
		fpAPipeItems			= NULL;
		ALUAPipeItems			= NULL;
		ALUBPipeItems			= NULL;
		LSPipeItems				= NULL;
		traceLogWindowItems		= NULL;
	}
}uiWidgetsItems, *puiWidgetsItems;

typedef struct _UIWIDGETSIZES
{
	//INSTRUCTION FETCH STUFF
	int instructionlistItemsCount;
	int tracewindowItemsCount;
	//ROB STUFF
	int	ROBListItemsCount;
	//SCHEDULE STUFF
	int freelistItemsCount;
	//RETIRED WIDGET int activelistItemsCount;
	int	regmaptableItemsCount;
	//DECODE STUFF
	int	fpQueueItemsCount;
	int	addressQueueItemsCount;
	int	integerQueueItemsCount;
	//PIPES
	int	fpMPipeItemsCount;
	int	fpAPipeItemsCount;
	int	ALUAPipeItemsCount;
	int	ALUBPipeItemsCount;
	int	LSPipeItemsCount;
	//TRACE LOG WINDOW (PIPELINE DIAGRAM)
	int traceLogWindowItemsCount;

	_UIWIDGETSIZES()
	{
		instructionlistItemsCount=0;
		tracewindowItemsCount	= 0;
		ROBListItemsCount		= 0;
		freelistItemsCount		= 0;
		//RETIRED WIDGET activelistItemsCount	= 0;
		regmaptableItemsCount	= 0;
		fpQueueItemsCount		= 0;
		addressQueueItemsCount	= 0;
		integerQueueItemsCount	= 0;
		fpMPipeItemsCount		= 0;
		fpAPipeItemsCount		= 0;
		ALUAPipeItemsCount		= 0;
		ALUBPipeItemsCount		= 0;
		LSPipeItemsCount		= 0;
		traceLogWindowItemsCount= 0;
	}
}uiWidgetsItemCount, *puiWidgetsItemCount;

//===============================================================================================================================================
//							UI HELPER FUNCTIONS
//===============================================================================================================================================

//XTERM init and destructor functions
bool initui (	CDKSCREEN* 	cdkscreen,
				WINDOW*		cursesWin,
				puiWidgets 	widgets,
				string* 	stdout_term,		//Required to redirect cout.
				string* 	stderr_term,
				int* 		pt_stdout,
				int* 		pt_stderr		);	//Required to close the output terminals

void destroyXterm(	CDKSCREEN* 	cdkscreen,		//Must persist the lifetime of
					WINDOW*		cursesWin,		//The application
					puiWidgets 	widgets,
					int* 		pt_stdout,
					int* 		pt_stderr	);

//==============================================================================================================
//								UI INTERFACE CLASS
//==============================================================================================================

namespace R10k
{

class UserInterface
{
public:
	UserInterface()
	{
		_cdkscreen	= (CDKSCREEN *)NULL;
		_cursesWin	= (WINDOW *)NULL;

		initui (	_cdkscreen,
					_cursesWin,
					&_interfaceWidgets,
					&_stdout_term,	//Required to redirect cout.
					&_stderr_term,
					&_pt_stdout,
					&_pt_stderr		);//Required to close the output terminals
		sleep(1);					//Otherwies buggy terminal behavior
		drawALL();

	}

	string getstdout_term(){return _stdout_term;}
	string getstderr_term(){return _stderr_term;}

	//Functions to refresh the Output widget of each displayed r10k device
	void blitInstructionList(	vector<string>* InstructionListItems);
	void blitTraceWindow(		vector<string>* TraceWindowListItems);		//Incoming trace (actual trace)
	void blitROBList(			vector<string>* ROBListItems);
	void blitFreeList(			vector<string>* FreeListItems);
	/*void blitActiveList(		vector<string>* ActiveListItems); RETIRED WIDGET*/
	void blitRegMapTable(		vector<string>* RegMapListItems);
	void blitFPQueueList(		vector<string>* FPQueueListItems);
	void blitAddressQueueList(	vector<string>* AddressQueueListItems);
	void blitIntegerQueueList(	vector<string>* IntegerQueueListItems);
	void blitFPMPipe(			vector<string>* FPMPipeListItems);
	void blitFPApipe(			vector<string>* FPAPipeListItems);
	void blitALUAPipe(			vector<string>* ALUAPipeListItems);
	void blitALUBPipe(			vector<string>* ALUBPipeListItems);
	void blitLSPipe(			vector<string>* LSPipeListItems);
	void blitTraceLogWindow(	vector<string>* TraceLogListItems);	//Outgoing trace (pipeline diagram)

	virtual ~UserInterface(){ destroyXterm(_cdkscreen, _cursesWin,&_interfaceWidgets, &_pt_stdout, &_pt_stderr);}


private:
	string _stdout_term;
	string _stderr_term;
	int 	_pt_stdout;
	int 	_pt_stderr;

//Widgets
	CDKSCREEN* 			_cdkscreen		= (CDKSCREEN *)NULL;
	WINDOW*				_cursesWin		= (WINDOW *)NULL;
	uiWidgets			_interfaceWidgets;
	uiWidgetsItems 		_interfaceWidgetItems;
	uiWidgetsItemCount 	_interfaceWidgetItemCount;

	void drawALL();
};



}


#endif
