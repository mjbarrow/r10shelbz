/*
 * InterfaceLayout.h
 *
 *  Created on: Dec 7, 2014
 *      Author: king
 */

#ifndef INTERFACELAYOUT_H_
#define INTERFACELAYOUT_H_



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

/* WIDGET RETIRED
//Active list dimensions
#define ACTLISLEFT	   (RMAPLEFT + RMAPWIDTH + 1)
#define ACTLISTTOP		0
#define ACTLISTSCROLLPOS RIGHT
#define ACTLISTHEIGHT	FLHEIGHT
#define ACTLISTWIDTH	FLWIDTH

#define ACTLISTDIMENSIONS 	ACTLISLEFT,			\
							ACTLISTTOP,			\
							ACTLISTSCROLLPOS,	\
							ACTLISTHEIGHT,		\
							ACTLISTWIDTH
*/

//FP queue dimensions
#define FPQUEUELEFT	   	   (RMAPLEFT + RMAPWIDTH + 1)/*(ACTLISLEFT + ACTLISTWIDTH + 1)*/
#define FPQUEUETOP 			0
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
#define FPMPIPELEFT			(RMAPLEFT + RMAPWIDTH + 1)/*(ACTLISLEFT + ACTLISTWIDTH + 1)*/
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
#define LSAPIPEWIDTH		(FLWIDTH * 2)

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

//Trace out logger dimensions
#define	TRACEOUTLOGLEFT			ROBPIPELEFT
#define TRACEOUTLOGTOP 			(ROBPIPETOP + ROBPIPEHEIGHT)
#define TRACEOUTLOGSCROLLPOS 	RIGHT
#define TRACEOUTLOGHEIGHT 		ROBPIPEHEIGHT
#define TRACEOUTLOGWIDTH 		ROBPIPEWIDTH

#define TRACEOUTLOGDIMENSIONS	TRACEOUTLOGLEFT,		\
								TRACEOUTLOGTOP,			\
								TRACEOUTLOGSCROLLPOS,	\
								TRACEOUTLOGHEIGHT,		\
								TRACEOUTLOGWIDTH

#endif /* INTERFACELAYOUT_H_ */
