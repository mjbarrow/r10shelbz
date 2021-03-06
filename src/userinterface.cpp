#include "userinterface.h"

using namespace std;

//========================================================================================
//						XTERM CONFIG AND INITIALIZATION ROUTINES
//========================================================================================


//=========================================================================================
//								HELPER FUNCTIONS
//=========================================================================================
#define		LoadEmptyItem(widget)																	\
									{	const char* e[1] = {(const char*)"empty"};					\
									setCDKScrollItems (	_interfaceWidgets.widget,					\
														(char**)e,									\
														1,											\
														true								);	}
//1 = ROBListItems
//2 = _InterfaceWidgetItems.ROBListItems
//3 = _InterfaceWidgets.ROBList
#define		ReloadItems(newContent,widgetItems,widgetItemsCount,widget)					\
			int 						loop = 0;										\
			const char*					charROBListItems[newContent->size()];			\
			vector<string>::iterator 	listit;											\
			for(listit =newContent->begin(); listit != newContent->end(); listit++)		\
			{																			\
				charROBListItems[loop] = convert(*listit);								\
				loop++;																	\
			}																			\
			/*library bug. Remove items  if list shrank*/								\
			while(_interfaceWidgetItemCount.widgetItemsCount > loop)					\
				{																		\
					_interfaceWidgetItemCount.widgetItemsCount--;						\
					deleteCDKScrollItem(_interfaceWidgets.widget,						\
										_interfaceWidgetItemCount.widgetItemsCount);	\
				}																		\
				_interfaceWidgetItemCount.widgetItemsCount = loop;						\
				/*re-set list items to new list*/										\
			_interfaceWidgetItems.widgetItems = charROBListItems;						\
			if(_interfaceWidgetItems.widgetItems == NULL)								\
				LoadEmptyItem(widget);													\
			setCDKScrollItems (	_interfaceWidgets.widget,								\
								(char**)_interfaceWidgetItems.widgetItems,				\
								loop,													\
								true								);


#ifdef HAVE_XCURSES
char *XCursesProgramName="scroll_ex";
#endif

using namespace std;

const char *convert(const std::string & s);		//Some junk helper function used by blitters.

//Create a new terminal
bool createNewTerm(string* OUT_termname, int* OUT_pt, bool maximized)
{
	 int pt = posix_openpt(O_RDWR);
	  if (pt == -1)
	  {
	    std::cerr << "Could not open pseudo terminal.\n";
	    return false;
	  }
	  char* ptname = ptsname(pt);
	  if (!ptname)
	  {
	    std::cerr << "Could not get pseudo terminal device name.\n";
	    close(pt);
	    return false;
	  }

	  if (unlockpt(pt) == -1)
	  {
	    std::cerr << "Could not get pseudo terminal device name.\n";
	    close(pt);
	    return false;
	  }

	  //return an integer = to the terminal
	  std::ostringstream oss;
	  if(maximized)
		  oss << "xterm -maximized -S" << (strrchr(ptname, '/')+1) << "/" << pt << " &";
	  else
		  oss << "xterm -S" << (strrchr(ptname, '/')+1) << "/" << pt << " &";
	  system(oss.str().c_str());

	  //Set up output variables. Use these file pointers to close the terminals.
	  *OUT_pt = pt;									//Return a pt
	  *OUT_termname = ptname;

	  return true;
}

//Check the terminal was successfully created
bool validateNewTerm(int xterm_fd, int pt)
{
	  char c;
	  do read(xterm_fd, &c, 1); while (c!='\n');

	  if (dup2(pt, 1) <0)
	  {
	    std::cerr << "Could not redirect standard output.\n";
	    close(pt);
	    return false;
	  }
	  if (dup2(pt, 2) <0)
	  {
	    std::cerr << "Could not redirect standard error output.\n";
	    close(pt);
	    return false;
	  }
	  return true;
}

//Returns the terminal number of stdio
int initializeXterm(	string* stdout_term,
						string* stderr_term,
						int* outfd,
						int* errfd)
{
	int pt_stdout;
//NOT USING RIGHT NOW	int pt_stderr;
	int intstdout;
	string ptname_stdout;
	string ptname_stderr;

	//create xterm windows
	createNewTerm(&ptname_stdout, &pt_stdout, true);		//True means maximized
	*outfd = open(ptname_stdout.c_str(),O_RDWR);
//NOT USING RIGHT NOW	createNewTerm(&ptname_stderr, &pt_stderr, false);		//True means default size
//NOt USING RIGHT NOW	*errfd = open(ptname_stderr.c_str(),O_RDWR);

	cout << "Output will be redirected to new terminals... " << ptname_stdout << endl;

	//Set terminal string pointers so that the main function can re-direct cout and cerr to the new terminals
	*stdout_term = ptname_stdout;
//NOT USING RIGHT NOW	*stderr_term = ptname_stderr;

	//Set the stdout and stdin macros so that printf and scanf will read from the new full screen terminal
	stdout = fopen(stdout_term->c_str(),"a");							//http://stackoverflow.com/questions/10947335/c-modifying-printf-to-output-to-a-file
	stdin = fopen(stdout_term->c_str(),"r");

	//Return stdout term number
	//get the first number (term number)
   size_t pos = 0;
   std::string token;
   while ((pos = stdout_term->find("/")) != std::string::npos)
       stdout_term->erase(0, pos + 1);
   istringstream ss(*stdout_term);
   ss >> intstdout;
   *stdout_term = ptname_stdout;	//Restore stdout.

	return intstdout;
}

void destroyXterm(	CDKSCREEN* cdkscreen,		//Must persist the lifetime of
					WINDOW*	cursesWin,			//The application
					puiWidgets widgets,
					int* pt_stdout,
					int* pt_stderr)
{
	//Destroy fetch
	destroyCDKScroll(widgets->tracewindow);
	destroyCDKScroll(widgets->instructionlist);
	destroyCDKScroll (widgets->ROBList);
	destroyCDKScroll (widgets->freelist);
	/*destroyCDKScroll (widgets->activelist);	RETIRED WIDGET*/
	destroyCDKScroll(widgets->regmaptable);
	destroyCDKScroll(widgets->fpQueue);
	destroyCDKScroll(widgets->addressQueue);
	destroyCDKScroll(widgets->integerQueue);
	 //DESTROY PIPES
	destroyCDKScroll(widgets->fpMPipe);
	destroyCDKScroll(widgets->fpAPipe);
	destroyCDKScroll(widgets->ALUAPipe);
	destroyCDKScroll(widgets->ALUBPipe);
	destroyCDKScroll(widgets->LSPipe);
	//Destroy tracelogwidow
	destroyCDKScroll(widgets->tracelogwindow);

	  destroyCDKScreen (cdkscreen);
	  delwin (cursesWin);
	  endCDK();
	  close(*pt_stdout);
	  close(*pt_stderr);
}

//========================================================================================
//						END XTERM CONFIG AND INITIALIZATION ROUTINES
//========================================================================================

//========================================================================================
//						INTERFACE INITIALIZATION ROUTINES
//========================================================================================

/*
 * This program demonstrates the Cdk scrolling list widget.
 * see: http://www.fifi.org/doc/libcdk-dev/examples/scroll_ex.c
 */
bool initui (	CDKSCREEN* cdkscreen,		//Must persist the lifetime of
				WINDOW*	cursesWin,			//The application
				puiWidgets widgets,			//All of the widgets to position ready for use
				string* stdout_term,		//Required to redirect cout.
				string* stderr_term,
				int* 	pt_stdout,
				int* 	pt_stderr		)
{
	 //LIBRARY STUFF
	struct winsize w;
	int wintermNo;
	//Widgit init stuff
	char* on	= (char*)"Booting...";
	char* in 	= (char*)"Ready:";
	char* pinit[2] 	= {on,in};


   /* Set up CDK. */
   wintermNo = initializeXterm(stdout_term, stderr_term, pt_stdout, pt_stderr);  //http://stackoverflow.com/questions/9996730/unix-c-open-new-terminal-and-redirect-output-to-it

   sleep(0);	//bug on getting xterm size

   ioctl(wintermNo, TIOCGWINSZ, &w);			//TODO 5 is a debug setting! Get the screen size
   cursesWin = initscr();
   endwin();
   refresh();
   clear();


   cerr << "terminal lines :" << w.ws_row << endl;
   cerr << "terminal cols : " << w.ws_col << endl;
#ifdef DEBUG
//Problems...   if(w.ws_col == 0)
	   w.ws_col = 317;
//Problems...   if(w.ws_row == 0)
	   w.ws_row = 87;
#endif
   resizeterm( w.ws_row, w.ws_col);
   cdkscreen = initCDKScreen (cursesWin);

   /* Set up CDK Colors. */
   initCDKColor();

 //Instruction fetch stage stuff
   /* Create the scrolling list. */
      widgets->instructionlist = newCDKScroll (	cdkscreen,
   		   	   	   	   	   	   	   	   ILDIMENSIONS,
									   (char*)"<C></5>Instruction list",
   									   pinit,
   									   1,
   									   NUMBERS, A_REVERSE, TRUE, FALSE);

      /* Create the scrolling window. */
      widgets->tracewindow = newCDKScroll (	cdkscreen,
   		   	   	   	   	   	   	   	   	   TWDIMENSIONS,
										   (char*)"<C></5>Trace window", //title
										   (char**)pinit,
	   									   1,
   										   NUMBERS, A_REVERSE, TRUE, FALSE);

 //Instruction decode stage stuff
   /* Create the scrolling list. */
   widgets->freelist = newCDKScroll (	cdkscreen,
		   	   	   	   	   	   	   	   FLDIMENSIONS,
									   (char*)"<C></5>free list",
   									   pinit,
   									   1,
									   NUMBERS, A_REVERSE, TRUE, FALSE);

   /* Create the scrolling window. */
   widgets->regmaptable = newCDKScroll (	cdkscreen,
		   	   	   	   	   	   	   	   	   RMAPDIMENSIONS,
										   (char*)"<C></5>Register map table", //title
	   									   pinit,
	   									   1,
										   NUMBERS, A_REVERSE, TRUE, FALSE);


   /*widgets->activelist = newCDKScroll (cdkscreen,				RETIRED WIDGET
		   	   	   	   	   	   	   	   	   ACTLISTDIMENSIONS,
										   "<C></5>active list",
	   									   pinit,
	   									   1,
										   NUMBERS, A_REVERSE, TRUE, FALSE);*/

   widgets->fpQueue = newCDKScroll (	cdkscreen,
		   	   	   	   	   	   	   	   	   FPQUEUEDIMENSIONS,
										   (char*)"<C></5>FP queue", //title
	   									   pinit,
	   									   1,
										   NUMBERS, A_REVERSE, TRUE, FALSE);

   widgets->addressQueue = newCDKScroll (cdkscreen,
		   	   	   	   	   	   	   	   	   ADDQUEUEDIMENSIONS,
										   (char*)"<C></5>address queue", //title
	   									   pinit,
	   									   1,
										   NUMBERS, A_REVERSE, TRUE, FALSE);

   widgets->integerQueue = newCDKScroll (cdkscreen,
		   	   	   	   	   	   	   	   	   IQUEUEDIMENSIONS,
										   (char*)"<C></5>integer queue", //title
	   									   pinit,
	   									   1,
										   NUMBERS, A_REVERSE, TRUE, FALSE);
//CREATE PIPES

   widgets->fpMPipe = newCDKScroll (cdkscreen,
		   	   	   	   	   	   	   	   FPMPIPEDIMENSIONS,
									   (char*)"<C></5>M Pipe", //title
   									   pinit,
   									   1,
									   NUMBERS, A_REVERSE, TRUE, FALSE);

   widgets->fpAPipe = newCDKScroll (cdkscreen,
		   	   	   	   	   	   	   	   FPAPIPEDIMENSIONS,
									   (char*)"<C></5>A Pipe", //title
   									   pinit,
   									   1,
									   NUMBERS, A_REVERSE, TRUE, FALSE);

   widgets->ALUAPipe = newCDKScroll (cdkscreen,
		   	   	   	   	   	   	   	   ALUAPIPEDIMENSIONS,
									   (char*)"<C></5>ALU A Pipe", //title
   									   pinit,
   									   1,
									   NUMBERS, A_REVERSE, TRUE, FALSE);

   widgets->ALUBPipe  = newCDKScroll (cdkscreen,
	   	   	   	   	   	   	   	   	   ALUBPIPEDIMENSIONS,
									   (char*)"<C></5>ALU B Pipe", //title
   									   pinit,
   									   1,
									   NUMBERS, A_REVERSE, TRUE, FALSE);

   widgets->LSPipe  = newCDKScroll (cdkscreen,
	   	   	   	   	   	   	   	   	   LSAPIPEDIMENSIONS,
									   (char*)"<C></5>LS Pipe", //title
   									   pinit,
   									   1,
									   NUMBERS, A_REVERSE, TRUE, FALSE);



//COMMIT-ESQUE STUFF
   widgets->ROBList = newCDKScroll (cdkscreen,
		   	   	   	   	   	   	   	   ROBLISTDIMENSIONS,
									   (char*)"<C></5>ROB list",	//Title
   									   pinit,
   									   2,
									   NUMBERS, A_REVERSE, TRUE, FALSE);	//[show nums,?,?,?]


   //TRACE (PIPELINE DIAGRAM)
   widgets->tracelogwindow = newCDKScroll(	cdkscreen,
										   TRACEOUTLOGDIMENSIONS,
		   	   	   	   	   	   	   	   	   (char*)"<C></5>Execution Pipeline Diagram",
										   pinit,
			   	   	   	   	   	   	   	   2,
			   	   	   	   	   	   	   	   NUMBERS, A_REVERSE, TRUE, FALSE);	//[show nums,?,?,?]

   /* Is the scrolling list NULL? */
   //TODO check all widgets are not null
   if (	widgets->tracelogwindow   == (CDKSCROLL *)NULL)
   {
      /* Exit CDK. */
      destroyCDKScreen (cdkscreen);
      endCDK();

      /* Print out a message and exit. */
      printf ("Oops. Could not make scrolling list. Is the window too small?\n");
      exit (1);
   }




/*
 addCDKSwindow (regmaptable, "<C></11>TOP: This is the first line.", BOTTOM);
 addCDKSwindow (regmaptable, "<C>Sleeping for 1 second.", BOTTOM);
 sleep (1);

 addCDKSwindow (regmaptable, "<L></11>1: This is another line.", BOTTOM);
 addCDKSwindow (regmaptable, "<C>Sleeping for 1 second.", BOTTOM);
 sleep (1);

 addCDKSwindow (regmaptable, "<C></11>2: This is another line.", BOTTOM);
 addCDKSwindow (regmaptable, "<C>Sleeping for 1 second.", BOTTOM);
 sleep (1);

 addCDKSwindow (regmaptable, "<R></11>3: This is another line.", BOTTOM);
 addCDKSwindow (regmaptable, "<C>Sleeping for 1 second.", BOTTOM);
 sleep (1);

 addCDKSwindow (regmaptable, "<C></11>4: This is another line.", BOTTOM);
 addCDKSwindow (regmaptable, "<C>Sleeping for 1 second.", BOTTOM);
 sleep (1);

 addCDKSwindow (regmaptable, "<L></11>5: This is another line.", BOTTOM);
 addCDKSwindow (regmaptable, "<C>Sleeping for 1 second.", BOTTOM);
 sleep (1);

 addCDKSwindow (regmaptable, "<C></11>6: This is another line.", BOTTOM);
 addCDKSwindow (regmaptable, "<C>Sleeping for 1 second.", BOTTOM);
 sleep (1);

 addCDKSwindow (regmaptable, "<C>Done. You can now play.", BOTTOM);

 addCDKSwindow (regmaptable, "<C>This is being added to the top.", TOP);*/

/*
   // Activate the scrolling list.
   selection = activateCDKScroll (ROBList, (chtype *)NULL);

   // Determine how the widget was exited.
   if (ROBList->exitType == vESCAPE_HIT)
   {
      mesg[0] = "<C>You hit escape. No file selected.";
      mesg[1] = "",
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, mesg, 3);
   }
   else if (ROBList->exitType == vNORMAL)
   {
      mesg[0] = "<C>You selected the following file";
      sprintf (temp, "<C>%s", item[selection]);
      mesg[1] = copyChar (temp);
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, mesg, 3);
      freeChar (mesg[1]);
   }
*/
   /* Clean up. */

   return true;
}

//========================================================================================
//									CLASS METHODS
//========================================================================================
namespace R10k
{

void UserInterface::drawALL()
{
	 //===========================================================================================
	 //									DRAW ALL WIDGETS
	 //===========================================================================================
	   /* Draw the scrolling window. */
	//DRAW FETCH
		drawCDKScroll (_interfaceWidgets.instructionlist, true);
		drawCDKScroll (_interfaceWidgets.tracewindow, true);
	//DRAW DECODE
	   	drawCDKScroll (_interfaceWidgets.regmaptable, true);//(swindow, swindow->box); where box is bool true for box show.
		drawCDKScroll (_interfaceWidgets.freelist, 1);
		/*drawCDKScroll (_interfaceWidgets.activelist, 1);	RETIRED WIDGET*/
	//DRAW QUEUES
		drawCDKScroll (_interfaceWidgets.fpQueue, true);
		drawCDKScroll (_interfaceWidgets.addressQueue, true);
		drawCDKScroll (_interfaceWidgets.integerQueue, true);
	//DRAW PIPES
		drawCDKScroll (_interfaceWidgets.fpMPipe,1);
		drawCDKScroll (_interfaceWidgets.fpAPipe,1);
		drawCDKScroll (_interfaceWidgets.ALUAPipe,1);
		drawCDKScroll (_interfaceWidgets.ALUBPipe,1);
		drawCDKScroll (_interfaceWidgets.LSPipe,1);
	//DRAW COMMIT / ROB
		drawCDKScroll (_interfaceWidgets.ROBList, 1);
	//DRAW TRACELOG
		drawCDKScroll (_interfaceWidgets.tracelogwindow, true);
}

void UserInterface::blitInstructionList(vector<string>* InstructionListItems)
{
	ReloadItems(							//A helper macro to implement the body of UserInterface::blitROBList()
				InstructionListItems,		//1 In vector:				FreeListItems
				instructionlistItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.freelistItems
				instructionlistItemsCount,	//Needed for UI fix up (remove items over the new list length)
				instructionlist		);		//3 Widget to blit to:		_interfaceWidgets.freelist

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.instructionlist,		//Draw this window
					true								);	//Draw with a box around it
#endif
}

void UserInterface::blitTraceWindow(vector<string>* TraceWindowListItems)
{
	ReloadItems(						//A helper macro to implement the body of UserInterface::blitROBList()
				TraceWindowListItems,	//1 In vector:				FreeListItems
				tracewindowItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.freelistItems
				tracewindowItemsCount,	//Needed for UI fix up (remove items over the new list length)
				tracewindow		);		//3 Widget to blit to:		_interfaceWidgets.freelist

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.tracewindow,		//Draw this window
					true						);	//Draw with a box around it
#endif
}

void UserInterface::blitROBList(vector<string>* ROBListItems)
{
	int 						loop = 0;
	const char* 				charROBListItems[ROBListItems->size()];
	vector<string>::iterator 	listit;

	for(listit =ROBListItems->begin(); listit != ROBListItems->end(); listit++)	//Iterate over each string
	{
		charROBListItems[loop] = convert(*listit);
		loop++;
	}

	_interfaceWidgetItems.ROBListItems = charROBListItems;		//Set the items list.
	if(_interfaceWidgetItems.ROBListItems == NULL)
		return;

	setCDKScrollItems (	_interfaceWidgets.ROBList,				//Nuke the old content and refresh
						(char**)_interfaceWidgetItems.ROBListItems,
						ROBListItems->size(),
						true								);

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.ROBList,		//Draw this window
					true						);	//Draw with a box around it
#endif
//DOUBT I DOUBT THAT THE LIST ITEMS WILL STAY IN SCOPE OUTSIDE OF THIS FUNCTION

}

void UserInterface::UserInterface::blitFreeList(vector<string>* FreeListItems)
{
	ReloadItems(						//A helper macro to implement the body of UserInterface::blitROBList()
				FreeListItems,			//1 In vector:				FreeListItems
				freelistItems,			//2	WidgetList to refresh:	_interfaceWidgetItems.freelistItems
				freelistItemsCount,		//Needed for UI fix up (remove items over the new list length)
				freelist		);		//3 Widget to blit to:		_interfaceWidgets.freelist

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.freelist,		//Draw this window
					true						);	//Draw with a box around it
#endif
}

/*void UserInterface::blitActiveList(		vector<string>* ActiveListItems)
{
	ReloadItems(						//A helper macro to implement the body of UserInterface::blitROBList()
				ActiveListItems,		//1 In vector:				FreeListItems
				activelistItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.activelistItems
				activelistItemsCount,
				activelist		);		//3 Widget to blit to:		_interfaceWidgets.activelist

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.activelist,	//Draw this window
					true						);	//Draw with a box around it
#endif
}													RETIRED WIDGET*/

void UserInterface::blitRegMapTable(vector<string>* RegMapListItems)	//working OK Dec 6 2k14.
{
	ReloadItems(						//A helper macro to implement the body of UserInterface::blitROBList()
				RegMapListItems,		//1 In vector:				RegMapListItems
				regmaptableItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.regmaptableItems
				regmaptableItemsCount,
				regmaptable		);		//3 Widget to blit to:		_interfaceWidgets.regmaptable

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.regmaptable,	//Draw this window
					true						);	//Draw with a box around it
#endif
}

void UserInterface::blitFPQueueList(		vector<string>* FPQueueListItems)	//working OK Dec 6 2k14
{
	ReloadItems(					//A helper macro to implement the body of UserInterface::blitROBList()
				FPQueueListItems,	//1 In vector:				FPQueueListItems
				fpQueueItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.fpQueueItems
				fpQueueItemsCount,
				fpQueue		);		//3 Widget to blit to:		_interfaceWidgets.fpQueue

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.fpQueue,		//Draw this window
					true						);	//Draw with a box around it
#endif
}

void UserInterface::blitAddressQueueList(	vector<string>* AddressQueueListItems)	//OK Dec 6
{
	ReloadItems(						//A helper macro to implement the body of UserInterface::blitROBList()
				AddressQueueListItems,	//1 In vector:				AddressQueueListItems
				addressQueueItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.addressQueueItems
				addressQueueItemsCount,
				addressQueue		);	//3 Widget to blit to:		_interfaceWidgets.addressQueue

				/*library bug. Remove items  if list shrank*/
				while(_interfaceWidgetItemCount.addressQueueItemsCount > loop)
					{
						_interfaceWidgetItemCount.addressQueueItemsCount--;
						deleteCDKScrollItem(_interfaceWidgets.addressQueue,
											_interfaceWidgetItemCount.addressQueueItemsCount);
					}
					_interfaceWidgetItemCount.addressQueueItemsCount = loop;
					/*re-set list items to new list*/
				_interfaceWidgetItems.addressQueueItems = charROBListItems;
				if(_interfaceWidgetItems.addressQueueItems == NULL)
					return;
				setCDKScrollItems (	_interfaceWidgets.addressQueue,
									(char**)_interfaceWidgetItems.addressQueueItems,
									loop,
									true								);



//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.addressQueue,		//Draw this window
					true						);	//Draw with a box around it
#endif
}

void UserInterface::blitIntegerQueueList(	vector<string>* IntegerQueueListItems)	//OK
{
	ReloadItems(						//A helper macro to implement the body of UserInterface::blitROBList()
				IntegerQueueListItems,	//1 In vector:				IntegerQueueListItems
				integerQueueItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.integerQueueItems
				integerQueueItemsCount,
				integerQueue		);	//3 Widget to blit to:		_interfaceWidgets.integerQueue

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.integerQueue,		//Draw this window
					true						);	//Draw with a box around it
#endif
}

void UserInterface::blitFPMPipe(			vector<string>* FPMPipeListItems)
{
	ReloadItems(					//A helper macro to implement the body of UserInterface::blitROBList()
				FPMPipeListItems,	//1 In vector:				FreeListItems
				fpMPipeItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.fpMPipe
				fpMPipeItemsCount,
				fpMPipe		);		//3 Widget to blit to:		_interfaceWidgets.fpMPipe

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.fpMPipe,		//Draw this window
					true						);	//Draw with a box around it
#endif
}

void UserInterface::blitFPApipe(			vector<string>* FPAPipeListItems)		//OK
{
	ReloadItems(					//A helper macro to implement the body of UserInterface::blitROBList()
				FPAPipeListItems,	//1 In vector:				FreeListItems
				fpAPipeItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.fpAPipetItems
				fpAPipeItemsCount,
				fpAPipe		);		//3 Widget to blit to:		_interfaceWidgets.fpAPipe

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.fpAPipe,		//Draw this window
					true						);	//Draw with a box around it
#endif
}

void UserInterface::blitALUAPipe(			vector<string>* ALUAPipeListItems)	//OK
{
	ReloadItems(					//A helper macro to implement the body of UserInterface::blitROBList()
				ALUAPipeListItems,	//1 In vector:				FreeListItems
				ALUAPipeItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.ALUAPipeItems
				ALUAPipeItemsCount,
				ALUAPipe		);	//3 Widget to blit to:		_interfaceWidgets.ALUAPipe

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.ALUAPipe,		//Draw this window
					true						);	//Draw with a box around it
#endif
}

void UserInterface::blitALUBPipe(			vector<string>* ALUBPipeListItems)	//OK
{
	ReloadItems(						//A helper macro to implement the body of UserInterface::blitROBList()
				ALUBPipeListItems,		//1 In vector:				ALUBPipeListItems
				ALUBPipeItems,			//2	WidgetList to refresh:	_interfaceWidgetItems.ALUBPipe
				ALUBPipeItemsCount,
				ALUBPipe		);		//3 Widget to blit to:		_interfaceWidgets.ALUBPipe

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.ALUBPipe,		//Draw this window
					true						);	//Draw with a box around it
#endif
}

void UserInterface::blitLSPipe(			vector<string>* LSPipeListItems)	//OK
{
	ReloadItems(					//A helper macro to implement the body of UserInterface::blitROBList()
				LSPipeListItems,	//1 In vector:				LSPipeListItems
				LSPipeItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.LSPipeItems
				LSPipeItemsCount,
				LSPipe		);		//3 Widget to blit to:		_interfaceWidgets.LSPipe

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.LSPipe,		//Draw this window
					true						);	//Draw with a box around it
#endif
}

void UserInterface::blitTraceLogWindow(	vector<string>* TraceLogListItems)
{
	ReloadItems(							//A helper macro to implement the body of UserInterface::blitROBList()
				TraceLogListItems,			//1 In vector:				TraceLogListItems
				traceLogWindowItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.traceLogWindowItems
				traceLogWindowItemsCount,
				tracelogwindow		);		//3 Widget to blit to:		_interfaceWidgets.tracelogwindow

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.tracelogwindow,		//Draw this window
					true						);	//Draw with a box around it
#endif
}


}

//==================================================================================================
//								MISC JUNK
//==================================================================================================
const char *convert(const std::string & s)
{
   return s.c_str();
}
