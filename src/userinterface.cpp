#include "userinterface.h"

using namespace std;

//========================================================================================
//						XTERM CONFIG AND INITIALIZATION ROUTINES
//========================================================================================

#define LISTWIDTH 20
#define MAXHEIGHT 50
#define THIRDHEIGHT 17
#define HSPACE 1

//Instruction List dimensions
#define ILLEFT 		0/*LEFT*/
#define ILTOP 		0/*TOP*/
#define ILSCROLLPOS	RIGHT
#define ILHEIGHT	THIRDHEIGHT
#define ILWIDTH		LISTWIDTH

#define ILDIMENSIONS	ILLEFT,		\
						ILTOP,		\
						ILSCROLLPOS,\
						ILHEIGHT,	\
						ILWIDTH

//Trace window dimensions
#define TWLEFT 		0/*LEFT*/
#define TWTOP 		ILHEIGHT
#define TWSCROLLPOS	RIGHT
#define TWHEIGHT	(MAXHEIGHT - TWTOP)
#define TWWIDTH		LISTWIDTH

#define TWDIMENSIONS	TWLEFT,		\
						TWTOP,		\
						TWSCROLLPOS,\
						TWHEIGHT,	\
						TWWIDTH

//Free list dimensions
#define FLLEFT 		(ILLEFT + ILWIDTH + 1)
#define FLTOP 		0/*TOP*/
#define FLSCROLLPOS	RIGHT
#define FLHEIGHT	MAXHEIGHT
#define FLWIDTH		LISTWIDTH

#define FLDIMENSIONS	FLLEFT,		\
						FLTOP,		\
						FLSCROLLPOS,\
						FLHEIGHT,	\
						FLWIDTH

//Register map dimensions
#define RMAPLEFT		(FLLEFT + FLWIDTH + 1)
#define RMAPTOP			0/*TOP*/
#define RMAPSCROLLPOS	RIGHT
#define RMAPHEIGHT	   	FLHEIGHT
#define RMAPWIDTH	   	((2 * FLWIDTH)+(FLWIDTH/2))

#define RMAPDIMENSIONS  RMAPLEFT,		\
						RMAPTOP,		\
						RMAPSCROLLPOS,	\
						RMAPHEIGHT,	   	\
						RMAPWIDTH

//Active list dimensions
#define ACTLISLEFT	   (RMAPLEFT + RMAPWIDTH + 1)
#define ACTLISTTOP		0/*TOP*/
#define ACTLISTSCROLLPOS RIGHT
#define ACTLISTHEIGHT	FLHEIGHT
#define ACTLISTWIDTH	FLWIDTH

#define ACTLISTDIMENSIONS 	ACTLISLEFT,			\
							ACTLISTTOP,			\
							ACTLISTSCROLLPOS,	\
							ACTLISTHEIGHT,		\
							ACTLISTWIDTH


//FP queue dimensions
#define FPQUEUELEFT	   	   (ACTLISLEFT + ACTLISTWIDTH + 1)
#define FPQUEUETOP 			0/*TOP*/
#define FPQUEUESCROLLPOS	RIGHT
#define	FPQUEUEHEIGHT		THIRDHEIGHT
#define FPQUEUEWIDTH		FLWIDTH

#define FPQUEUEDIMENSIONS 	FPQUEUELEFT,		\
							FPQUEUETOP,			\
							FPQUEUESCROLLPOS,	\
							FPQUEUEHEIGHT,		\
							FPQUEUEWIDTH

//Address queue dimensions
#define ADDQUEUELEFT	   (FPQUEUELEFT + FPQUEUEWIDTH + 1)
#define ADDQUEUETOP			0/*TOP*/
#define ADDQUEUESCROLLPOS	RIGHT
#define ADDQUEUEHEIGHT		FPQUEUEHEIGHT
#define ADDQUEUEWIDTH		FLWIDTH

#define ADDQUEUEDIMENSIONS 	ADDQUEUELEFT,		\
							ADDQUEUETOP,		\
							ADDQUEUESCROLLPOS,	\
							ADDQUEUEHEIGHT,		\
							ADDQUEUEWIDTH


//integer queue dimensions
#define IQUEUELEFT	   	   (ADDQUEUELEFT + ADDQUEUEWIDTH + 1)
#define IQUEUETOP			0/*TOP*/
#define IQUEUESCROLLPOS		RIGHT
#define IQUEUEHEIGHT		FPQUEUEHEIGHT
#define IQUEUEWIDTH			FLWIDTH

#define IQUEUEDIMENSIONS	IQUEUELEFT,			\
							IQUEUETOP,			\
							IQUEUESCROLLPOS,	\
							IQUEUEHEIGHT,		\
							IQUEUEWIDTH

//M pipe dimensions
#define FPMPIPELEFT			(ACTLISLEFT + ACTLISTWIDTH + 1)
#define	FPMPIPETOP			(FPQUEUETOP + FPQUEUEHEIGHT)
#define FPMPIPESCROLLPOS	RIGHT
#define FPMPIPEHEIGHT		6
#define FPMPIPEWIDTH		(3*FLWIDTH)

#define FPMPIPEDIMENSIONS	FPMPIPELEFT,		\
							FPMPIPETOP,			\
							FPMPIPESCROLLPOS,	\
							FPMPIPEHEIGHT,		\
							FPMPIPEWIDTH

//A pipe dimensions
#define FPAPIPELEFT  		FPMPIPELEFT
#define FPAPIPETOP			(FPMPIPETOP + FPMPIPEHEIGHT)
#define FPAPIPESCROLLPOS	RIGHT
#define FPAPIPEHEIGHT		FPMPIPEHEIGHT
#define FPAPIPEWIDTH		FPMPIPEWIDTH

#define FPAPIPEDIMENSIONS	FPAPIPELEFT,		\
							FPAPIPETOP,			\
							FPAPIPESCROLLPOS,	\
							FPAPIPEHEIGHT,		\
							FPAPIPEWIDTH

//ALU A pipe dimensions
#define ALUAPIPELEFT		FPMPIPELEFT
#define ALUAPIPETOP			(FPAPIPETOP + FPAPIPEHEIGHT)
#define ALUAPIPESCROLLPOS	RIGHT
#define ALUAPIPEHEIGHT		FPAPIPEHEIGHT
#define ALUAPIPEWIDTH		FLWIDTH

#define ALUAPIPEDIMENSIONS	ALUAPIPELEFT,		\
							ALUAPIPETOP,		\
							ALUAPIPESCROLLPOS,	\
							ALUAPIPEHEIGHT,		\
							ALUAPIPEWIDTH

//ALU B pipe dimensions
#define ALUBPIPELEFT		FPMPIPELEFT
#define ALUBPIPETOP			(ALUAPIPETOP + ALUAPIPEHEIGHT)
#define ALUBPIPESCROLLPOS	RIGHT
#define ALUBPIPEHEIGHT		FPAPIPEHEIGHT
#define ALUBPIPEWIDTH		FLWIDTH

#define ALUBPIPEDIMENSIONS	ALUBPIPELEFT,		\
							ALUBPIPETOP,		\
							ALUBPIPESCROLLPOS,	\
							ALUBPIPEHEIGHT,		\
							ALUBPIPEWIDTH

//LS Pipe dimensions
#define LSAPIPELEFT			FPMPIPELEFT
#define LSAPIPETOP			(ALUBPIPETOP + ALUBPIPEHEIGHT)
#define LSAPIPESCROLLPOS	RIGHT
#define LSAPIPEHEIGHT		FPAPIPEHEIGHT
#define LSAPIPEWIDTH		FLWIDTH

#define LSAPIPEDIMENSIONS	LSAPIPELEFT,		\
							LSAPIPETOP,			\
							LSAPIPESCROLLPOS,	\
							LSAPIPEHEIGHT,		\
							LSAPIPEWIDTH

//ROB list dimensions
#define	ROBPIPELEFT			(IQUEUELEFT + IQUEUEWIDTH + 1)
#define ROBPIPETOP 			0/*TOP*/
#define ROBPIPESCROLLPOS 	RIGHT
#define ROBPIPEHEIGHT 		(FPAPIPETOP + FPAPIPEHEIGHT)
#define ROBPIPEWIDTH 		(FLWIDTH * 6)

#define ROBLISTDIMENSIONS	ROBPIPELEFT,		\
							ROBPIPETOP,			\
							ROBPIPESCROLLPOS,	\
							ROBPIPEHEIGHT,		\
							ROBPIPEWIDTH

//=========================================================================================
//								HELPER FUNCTIONS
//=========================================================================================
//1 = ROBListItems
//2 = _InterfaceWidgetItems.ROBListItems
//3 = _InterfaceWidgets.ROBList
#define		ReloadItems(newContent,widgetItems,widget)									\
			int 						loop = 0;										\
			const char* 				charROBListItems[newContent->size()];			\
			vector<string>::iterator 	listit;											\
			for(listit =newContent->begin(); listit != newContent->end(); listit++)		\
			{																			\
				charROBListItems[loop] = convert(*listit);								\
				loop++;																	\
			}																			\
			_interfaceWidgetItems.widgetItems = charROBListItems;						\
			if(_interfaceWidgetItems.widgetItems == NULL)								\
				return;																	\
			setCDKScrollItems (	_interfaceWidgets.widget,								\
								(char**)_interfaceWidgetItems.widgetItems,				\
								newContent->size(),										\
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
	   destroyCDKScroll (widgets->ROBList);
	   destroyCDKScroll (widgets->freelist);
	   destroyCDKScroll (widgets->activelist);
	   destroyCDKSwindow(widgets->regmaptable);
	   destroyCDKSwindow(widgets->fpQueue);
	   destroyCDKSwindow(widgets->addressQueue);
	   destroyCDKSwindow(widgets->integerQueue);
	 //DESTROY PIPES
	   destroyCDKSwindow(widgets->fpMPipe);
	   destroyCDKSwindow(widgets->fpAPipe);
	   destroyCDKSwindow(widgets->ALUAPipe);
	   destroyCDKSwindow(widgets->ALUBPipe);
	   destroyCDKSwindow(widgets->LSPipe);

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
//	  int selection, count;//junk
	//Widgit init stuff
	string init 	= "Ready:";
	char* pinit[1] 	= {(char*)init.c_str()};
//	int initcount 	= 1;


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
   if(w.ws_col == 0)
	   w.ws_col = 317;
   if(w.ws_row == 0)
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
   									   (char**)pinit,
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


   widgets->activelist = newCDKScroll (cdkscreen,
		   	   	   	   	   	   	   	   	   ACTLISTDIMENSIONS,
										   (char*)"<C></5>active list",
	   									   pinit,
	   									   1,
										   NUMBERS, A_REVERSE, TRUE, FALSE);

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
   									   1,
									   NUMBERS, A_REVERSE, TRUE, FALSE);	//[show nums,?,?,?]


   /* Is the scrolling list NULL? */
   //TODO check all widgets are not null
   if (	widgets->ROBList   == (CDKSCROLL *)NULL)
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
		drawCDKScroll (_interfaceWidgets.activelist, 1);
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
				freelist		);		//3 Widget to blit to:		_interfaceWidgets.freelist

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.freelist,		//Draw this window
					true						);	//Draw with a box around it
#endif
}

//TODO ALL BELOW ARE BAD

void UserInterface::blitActiveList(		vector<string>* ActiveListItems)
{
	ReloadItems(						//A helper macro to implement the body of UserInterface::blitROBList()
				ActiveListItems,		//1 In vector:				FreeListItems
				activelistItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.activelistItems
				activelist		);		//3 Widget to blit to:		_interfaceWidgets.activelist

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.activelist,	//Draw this window
					true						);	//Draw with a box around it
#endif
}

void UserInterface::blitRegMapTable(vector<string>* RegMapListItems)
{
	ReloadItems(						//A helper macro to implement the body of UserInterface::blitROBList()
				RegMapListItems,		//1 In vector:				RegMapListItems
				regmaptableItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.regmaptableItems
				regmaptable		);		//3 Widget to blit to:		_interfaceWidgets.regmaptable

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.regmaptable,	//Draw this window
					true						);	//Draw with a box around it
#endif
}

void UserInterface::blitFPQueueList(		vector<string>* FPQueueListItems)
{
	ReloadItems(					//A helper macro to implement the body of UserInterface::blitROBList()
				FPQueueListItems,	//1 In vector:				FPQueueListItems
				fpQueueItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.fpQueueItems
				fpQueue		);		//3 Widget to blit to:		_interfaceWidgets.fpQueue

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.fpQueue,		//Draw this window
					true						);	//Draw with a box around it
#endif
}

void UserInterface::blitAddressQueueList(	vector<string>* AddressQueueListItems)
{
	ReloadItems(						//A helper macro to implement the body of UserInterface::blitROBList()
				AddressQueueListItems,	//1 In vector:				AddressQueueListItems
				addressQueueItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.addressQueueItems
				addressQueue		);	//3 Widget to blit to:		_interfaceWidgets.addressQueue

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.addressQueue,		//Draw this window
					true						);	//Draw with a box around it
#endif
}

void UserInterface::blitIntegerQueueList(	vector<string>* IntegerQueueListItems)
{
	ReloadItems(						//A helper macro to implement the body of UserInterface::blitROBList()
				IntegerQueueListItems,	//1 In vector:				IntegerQueueListItems
				integerQueueItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.integerQueueItems
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
				fpMPipe		);		//3 Widget to blit to:		_interfaceWidgets.fpMPipe

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.fpMPipe,		//Draw this window
					true						);	//Draw with a box around it
#endif
}

void UserInterface::blitFPApipe(			vector<string>* FPAPipeListItems)
{
	ReloadItems(					//A helper macro to implement the body of UserInterface::blitROBList()
				FPAPipeListItems,	//1 In vector:				FreeListItems
				fpAPipeItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.fpAPipetItems
				fpAPipe		);		//3 Widget to blit to:		_interfaceWidgets.fpAPipe

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.fpAPipe,		//Draw this window
					true						);	//Draw with a box around it
#endif
}

void UserInterface::blitALUAPipe(			vector<string>* ALUAPipeListItems)
{
	ReloadItems(					//A helper macro to implement the body of UserInterface::blitROBList()
				ALUAPipeListItems,	//1 In vector:				FreeListItems
				ALUAPipeItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.ALUAPipeItems
				ALUAPipe		);	//3 Widget to blit to:		_interfaceWidgets.ALUAPipe

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.ALUAPipe,		//Draw this window
					true						);	//Draw with a box around it
#endif
}

void UserInterface::blitALUBPipe(			vector<string>* ALUBPipeListItems)
{
	ReloadItems(						//A helper macro to implement the body of UserInterface::blitROBList()
				ALUBPipeListItems,		//1 In vector:				ALUBPipeListItems
				ALUBPipeItems,			//2	WidgetList to refresh:	_interfaceWidgetItems.ALUBPipe
				ALUBPipe		);		//3 Widget to blit to:		_interfaceWidgets.ALUBPipe

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.ALUBPipe,		//Draw this window
					true						);	//Draw with a box around it
#endif
}

void UserInterface::blitLSPipe(			vector<string>* LSPipeListItems)
{
	ReloadItems(					//A helper macro to implement the body of UserInterface::blitROBList()
				LSPipeListItems,	//1 In vector:				LSPipeListItems
				LSPipeItems,		//2	WidgetList to refresh:	_interfaceWidgetItems.LSPipeItems
				LSPipe		);		//3 Widget to blit to:		_interfaceWidgets.LSPipe

//TODO blit all on clock
#ifdef DEBUG
	drawCDKScroll (_interfaceWidgets.LSPipe,		//Draw this window
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
