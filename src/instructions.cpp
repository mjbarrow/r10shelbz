/*
 * instructions.cpp
 *
 *  Created on: Nov 16, 2014
 *      Author: king
 */

#include "utils.h"
#include "instructions.h"
#include <iostream>
#include <sstream>	//atoi type thing

#define opcodetoint(x) if (istrequal(Opcode,x)) return(opcodestrtoint(x));

namespace R10k {

//Utility functions
bool istrequal(const string& a, const string& b);

int opcodestrtoint(string operand);

bool istrequal(const string& a, const string& b) //see: http://stackoverflow.com/questions/11635/case-insensitive-string-comparison-in-c
{
    unsigned int sz = a.size();
    if (b.size() != sz)
        return false;
    for (unsigned int i = 0; i < sz; ++i)
        if (tolower(a[i]) != tolower(b[i]))
            return false;
    return true;
}

int opcodestrtoint(string operand)
{
	if (istrequal(operand,"ADD")) return 0b100000;
	if (istrequal(operand,"ADDI")) return 0b001000;
	return -1;
}

int decode_opcode(string Opcode)
{
	int decodedOpcode = BADOpcode; //invalid by default.

	//Go through all R10K operands..
	//this takes care of case sensitivity. Don't worry abt it in the src.
	opcodetoint("ADD"); //Macro case insensitively compares Opcode with ADD and calls opcodestrtoint if true.

	return decodedOpcode;
}

//End utility functions

instruction::instruction() {
	Operand3_val = Operand2_val = Operand1_val = BADOperand;
	Opcode_val = BADOpcode;
	Operand3_name = Operand2_name = Operand1_name = "BAD";
}

instruction::instruction(string str_instruction)
{
	size_t pos = 0;
	std::string delimiter = " ";
	std::string unchked_asm;

	//init to bad instruction
	 Operand1_val = Operand2_val = Operand3_val  = BADOperand;
	 Opcode_val = BADOpcode;
	 Opcode_name = Operand1_name = Operand2_name = Operand3_name = "BAD";

	//Get Opcode
	if ((pos = str_instruction.find(delimiter)) != std::string::npos)
	{
//TO DO: Parse Opcode
		unchked_asm = str_instruction.substr(0, pos);
	    Opcode_val = decode_opcode(unchked_asm);
	    if(Opcode_val != -1) Opcode_name = unchked_asm;	//Keep the opcode if we decode ok.
	    std::cerr << Opcode_name << std::endl;

	    //Adjust input for next part of instruction
	    str_instruction.erase(0, pos + delimiter.length());
	}
	else
	{
		//Should have an opcode. :( Fail.
		std::cerr << "bad opcode" << std::endl;
		return;
	}
	//Get Operand 1
	//Operands are ',' delimited
	delimiter = ",";
	if ((pos = str_instruction.find(delimiter)) != std::string::npos)
	{
//TO DO: Parse Opcode
	    Operand1_name = str_instruction.substr(0, pos);
	    std::cerr << Operand1_name << std::endl;

	    //Adjust input for next part of instruction
	    str_instruction.erase(0, pos + delimiter.length());
	}
	else
	{
		//Should have an opcode. :( Fail.
		std::cerr << "bad operand 1" << std::endl;
		return;
	}
	//Get Operand 2
	if ((pos = str_instruction.find(delimiter)) != std::string::npos)
	{
//TO DO: Parse Opcode
	    Operand2_name = str_instruction.substr(0, pos);
	    std::cerr << Operand2_name << std::endl;

	    //Adjust input for next part of instruction
	    str_instruction.erase(0, pos + delimiter.length());
	}
	else
	{
		//Should have an opcode. :( Fail.
		std::cerr << "bad operand 2" << std::endl;
		return;
	}
	//Operand 3 should be all that is left in the string
//TO DO: Parse Opcode
	Operand3_name = str_instruction.substr(0, pos);
	std::cerr << Operand3_name << std::endl;

	//Adjust input for next part of instruction
	str_instruction.erase(0, pos + delimiter.length());

}

instruction::~instruction() {
	// TODO Auto-generated destructor stub
}

//<op> <rs> <rt> <rd> <extra>
traceinstruction::traceinstruction(string traceline, int tracelineNo)
{
	size_t pos = 0;
	std::string delimiter = " ";
	std::string unchked_asm;
	std::stringstream ss;

	strOp = "BAD";
	traceLineNo = intOp = m_rs = rs = m_rt = rt = m_rd.Key = rd = extra = -1;

	traceLineNo = tracelineNo;

	//Get Opcode
	if ((pos = traceline.find(delimiter)) != std::string::npos)
	{
		unchked_asm = traceline.substr(0, pos);
		//Go through the 5 or so assembly instructions
		if(istrequal("L",unchked_asm))
			{strOp = unchked_asm; intOp = L;}
		if(istrequal("S",unchked_asm))
			{strOp = unchked_asm; intOp = S;}
		if(istrequal("I",unchked_asm))
			{strOp = unchked_asm; intOp = I;}
		if(istrequal("B",unchked_asm))
			{strOp = unchked_asm; intOp = B;}
		if(istrequal("A",unchked_asm))
			{strOp = unchked_asm; intOp = A;}
		if(istrequal("M",unchked_asm))
			{strOp = unchked_asm; intOp = M;}
	    //Adjust input for next part of instruction
	    traceline.erase(0, pos + delimiter.length());
	}
	else
	{
		//Should have an opcode. :( Fail.
		std::cerr << "bad opcode" << std::endl;
		return;
	}
	//Get Operand 1
	if ((pos = traceline.find(delimiter)) != std::string::npos)
	{
		//Parse assuming no weird input
		unchked_asm = traceline.substr(0, pos);
		ss << std::hex << unchked_asm;
		ss >> rs;
		ss.str( std::string() );
		ss.clear();
	    //Adjust input for next part of instruction
	    traceline.erase(0, pos + delimiter.length());
	}
	else
	{
		//Should have an opcode. :( Fail.
		std::cerr << "bad operand 1" << std::endl;
		//return;
	}
	//Get rt
	if ((pos = traceline.find(delimiter)) != std::string::npos)
	{
		//Parse assuming no weird input
		unchked_asm = traceline.substr(0, pos);
		ss << std::hex << unchked_asm;
		ss >> rt;
		ss.str( std::string() );
		ss.clear();
	    //Adjust input for next part of instruction
	    traceline.erase(0, pos + delimiter.length());
	}
	else
	{
		//Should have an opcode. :( Fail.
		std::cerr << "bad operand 2" << std::endl;
		//return;
	}

	if(needextra(intOp))
	{
		//Get rd
		if ((pos = traceline.find(delimiter)) != std::string::npos)
		{
			//Parse assuming no weird input
			unchked_asm = traceline.substr(0, pos);
			ss << std::hex << unchked_asm;
			ss >> rd;
			ss.str( std::string() );
			ss.clear();
		    //Adjust input for next part of instruction
		    traceline.erase(0, pos + delimiter.length());
		}
		else
		{
			//Should have an opcode. :( Fail.
			std::cerr << "bad rd" << std::endl;
			//return;
		}
		//Get extra if needed
		ss << std::hex << traceline;
		ss >> extra;
	}
	else
	{
		if ((pos = traceline.find(delimiter)) != std::string::npos)
		{
			//Parse assuming messed up input
			unchked_asm = traceline.substr(0, pos);
			ss << std::hex << unchked_asm;
			ss >> rd;
		}
		else
		{
			//Get rd assuming good input
			ss << std::hex << traceline;
			ss >> rd;
		}
	}
	//Swizzle registers for consistency regarding dependency checks and memory disambigation
	if(intOp == L)						//Load instructions require reg swizzling
		{rd = rt; rt = BADOperand;}
	if((intOp == S) || (intOp == B))	//Stores and branches do not write to any register
		rd = BADOperand;

}

traceinstruction::traceinstruction()
{
	strOp = "BAD";
	traceLineNo = intOp = m_rs = rs = m_rt = rt = m_rd.Key = rd = extra = -1;
}

traceinstruction::~traceinstruction(){}

} /* namespace R10k */
