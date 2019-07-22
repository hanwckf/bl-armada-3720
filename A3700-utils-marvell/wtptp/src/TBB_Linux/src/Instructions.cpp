/*******************************************************************************
Copyright (C) 2017 Marvell International Ltd.

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the three
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation, either version 2 of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************************
Marvell GNU General Public License FreeRTOS Exception

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the Lesser
General Public License Version 2.1 plus the following FreeRTOS exception.
An independent module is a module which is not derived from or based on
FreeRTOS.
Clause 1:
Linking FreeRTOS statically or dynamically with other modules is making a
combined work based on FreeRTOS. Thus, the terms and conditions of the GNU
General Public License cover the whole combination.
As a special exception, the copyright holder of FreeRTOS gives you permission
to link FreeRTOS with independent modules that communicate with FreeRTOS solely
through the FreeRTOS API interface, regardless of the license terms of these
independent modules, and to copy and distribute the resulting combined work
under terms of your choice, provided that:
1. Every copy of the combined work is accompanied by a written statement that
details to the recipient the version of FreeRTOS used and an offer by yourself
to provide the FreeRTOS source code (including any modifications you may have
made) should the recipient request it.
2. The combined work is not itself an RTOS, scheduler, kernel or related
product.
3. The independent modules add significant and primary functionality to
FreeRTOS and do not merely extend the existing functionality already present in
FreeRTOS.
Clause 2:
FreeRTOS may not be used for any competitive or comparative purpose, including
the publication of any form of run time or compile time metric, without the
express permission of Real Time Engineers Ltd. (this is the norm within the
industry and is intended to ensure information accuracy).

********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.

	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.

	* Neither the name of Marvell nor the names of its contributors may be
	  used to endorse or promote products derived from this software without
	  specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include <algorithm>

#include "Instructions.h"
#include "TimDescriptorParser.h"

CInstruction::CInstruction() :
		CTimLib() {
	// empty nop instruction
	m_InstructionOpCode = INSTR_NOP;
	m_InstructionText = "NOP";
	m_NumParamsUsed = 0;
	for (int i = 0; i < MAX_INST_PARAMS; i++)
		m_ParamValues[i] = 0;
}

CInstruction::CInstruction(INSTRUCTION_OP_CODE_SPEC_T eOpCode,
		const string sInstructionName, unsigned int uiNumParamsUsed,
		const string sParam0Name, const string sParam1Name,
		const string sParam2Name, const string sParam3Name,
		const string sParam4Name) :
		m_InstructionOpCode(eOpCode), m_InstructionText(sInstructionName), m_NumParamsUsed(
				uiNumParamsUsed), CTimLib() {
	m_ParamNames[0] = sParam0Name;
	m_ParamNames[1] = sParam1Name;
	m_ParamNames[2] = sParam2Name;
	m_ParamNames[3] = sParam3Name;
	m_ParamNames[4] = sParam4Name;
	m_ParamValues[0] = 0;
	m_ParamValues[1] = 0;
	m_ParamValues[2] = 0;
	m_ParamValues[3] = 0;
	m_ParamValues[4] = 0;
}

CInstruction::~CInstruction(void) {

}

CInstruction::CInstruction(const CInstruction& rhs) :
		CTimLib(rhs) {
	// copy constructor
	CInstruction& nc_rhs = const_cast<CInstruction&>(rhs);

	m_InstructionOpCode = rhs.m_InstructionOpCode;
	m_InstructionText = rhs.m_InstructionText;
	m_NumParamsUsed = rhs.m_NumParamsUsed;
	for (int i = 0; i < MAX_INST_PARAMS; i++) {
		m_ParamNames[i] = rhs.m_ParamNames[i];
		m_ParamValues[i] = rhs.m_ParamValues[i];
	}
}

CInstruction& CInstruction::operator=(const CInstruction& rhs) {
	// assignment operator
	if (&rhs != this) {
		CTimLib::operator=(rhs);

		CInstruction& nc_rhs = const_cast<CInstruction&>(rhs);

		m_InstructionOpCode = rhs.m_InstructionOpCode;
		m_InstructionText = rhs.m_InstructionText;
		m_NumParamsUsed = rhs.m_NumParamsUsed;
		for (int i = 0; i < MAX_INST_PARAMS; i++) {
			m_ParamNames[i] = rhs.m_ParamNames[i];
			m_ParamValues[i] = rhs.m_ParamValues[i];
		}
	}
	return *this;
}

bool CInstruction::SetInstructionType(INSTRUCTION_OP_CODE_SPEC_T eOpCode) {
	t_InstructionListIter Iter = CInstructions::s_InstFmt.begin();
	while (Iter != CInstructions::s_InstFmt.end()) {
		if ((*Iter)->m_InstructionOpCode == eOpCode) {
			*this = *(*Iter);
			break;
		}
		Iter++;
	}
	return true;
}

bool CInstruction::SetInstructionType(string& sInstructionText) {
	t_InstructionListIter Iter = CInstructions::s_InstFmt.begin();
	while (Iter != CInstructions::s_InstFmt.end()) {
		if ((*Iter)->m_InstructionText == sInstructionText) {
			*this = *(*Iter);
			break;
		}
		Iter++;
	}
	return true;
}

bool CInstruction::ToBinary(ofstream& ofs) {
	return ofs.good();
}

unsigned int CInstruction::Size() {
	if (m_InstructionOpCode == INSTR_PP_TABLEHEADER)
		return 0; // nothing added to TIM.bin
	else if (m_InstructionOpCode == INSTR_PP_WRITE)
		return ((2 * 4) + 4) * 3; // size of 3 INSTR_WRITE's added to TIM.bin
	else
		// all other instructions based on params used
		return (m_NumParamsUsed * 4) + 4;
}

string CInstruction::IntFieldValueToText(int iParamNum) {
	stringstream ss;
	char szValue[255] = { 0 };
	switch (m_InstructionOpCode) {
	case INSTR_LABEL:
	case INSTR_BRANCH:
		if (iParamNum == 0) {
			//char szLabel[5]={0};
			//memcpy(szLabel, (char*)&m_ParamValues[0], sizeof(int));
			//ss << szLabel;
			ss << HexFormattedAscii(m_ParamValues[iParamNum]);
		}
		break;

		// all instructions with scratch memory id in parameter 1
	case INSTR_TEST_SM_IF_ZERO_AND_SET:
	case INSTR_TEST_SM_IF_NOT_ZERO_AND_SET:
		if (iParamNum == 0 || iParamNum == 2) {
			ss << "SM" << m_ParamValues[iParamNum];
		}
		break;

	case INSTR_MOV_SM_SM:
	case INSTR_OR_SM_SM:
	case INSTR_AND_SM_SM:
	case INSTR_ADD_SM_SM:
	case INSTR_SUB_SM_SM:
	case INSTR_LOAD_SM_FROM_ADDR_IN_SM:
	case INSTR_STORE_SM_TO_ADDR_IN_SM:
		if (iParamNum == 0 || iParamNum == 1) {
			ss << "SM" << m_ParamValues[iParamNum];
		}
		break;

	case INSTR_LOAD_SM_ADDR:
	case INSTR_LOAD_SM_VAL:
	case INSTR_STORE_SM_ADDR:
	case INSTR_RSHIFT_SM_VAL:
	case INSTR_LSHIFT_SM_VAL:
	case INSTR_AND_SM_VAL:
	case INSTR_OR_SM_VAL:
	case INSTR_ADD_SM_VAL:
	case INSTR_SUB_SM_VAL:
		if (iParamNum == 0) {
			ss << "SM" << m_ParamValues[iParamNum];
		}
		break;

	case INSTR_TEST_SM_AND_BRANCH:
		if (iParamNum == 0) {
			ss << "SM" << m_ParamValues[iParamNum];
		}
		// no break
	case INSTR_TEST_ADDR_AND_BRANCH:
		if (iParamNum == 3) {
			ss << IntOperatorToText(m_ParamValues[iParamNum]);
		}
		if (iParamNum == 4) {
			//char szLabel[5]={0};
			//memcpy(szLabel, (char*)&m_ParamValues[ iParamNum ], sizeof(int));
			//ss << szLabel;
			ss << HexFormattedAscii(m_ParamValues[iParamNum]);
		}
		break;
	}

	// default is ""
	return ss.str();
}

string CInstruction::IntOperatorToText(int iOperator) {
	switch (iOperator) {
	case INSTR_TEST_FOR_EQ:
		return "==";
	case INSTR_TEST_FOR_NE:
		return "!=";
	case INSTR_TEST_FOR_LT:
		return "<";
	case INSTR_TEST_FOR_LTE:
		return "<=";
	case INSTR_TEST_FOR_GT:
		return ">";
	case INSTR_TEST_FOR_GTE:
		return ">=";
	case INSTR_ILLEGAL_OP:
	default:
		return "ILLEGAL_OP";
	}
	//default
	return "ILLEGAL_OP";
}

int CInstruction::TextOperatorToInt(string& sTextOperator) {
	if ("ILLEGAL_OP" == sTextOperator)
		return INSTR_ILLEGAL_OP;
	if ("==" == sTextOperator)
		return INSTR_TEST_FOR_EQ;
	if ("!=" == sTextOperator)
		return INSTR_TEST_FOR_NE;
	if ("<" == sTextOperator)
		return INSTR_TEST_FOR_LT;
	if ("<=" == sTextOperator)
		return INSTR_TEST_FOR_LTE;
	if (">" == sTextOperator)
		return INSTR_TEST_FOR_GT;
	if (">=" == sTextOperator)
		return INSTR_TEST_FOR_GTE;

	//default
	return INSTR_ILLEGAL_OP;
}

int CInstruction::TextFieldValueToInt(int iParamNum, string& sTextField) {
	switch (m_InstructionOpCode) {
	case INSTR_LABEL:
	case INSTR_BRANCH:
		if (iParamNum == 0) {
			if (IsAlphaNumeric(sTextField)) {
				char szText[40] = { 0 };
				int len = min(4, (int) sTextField.length());
				memcpy(szText, sTextField.substr(0, len).c_str(), len);
				return *((int*) szText);
			}
		}

		if (iParamNum == 1) {
			return Translate(sTextField);
		}
		break;

	case INSTR_WRITE:
	case INSTR_READ:
	case INSTR_AND_VAL:
	case INSTR_OR_VAL:
		if (iParamNum == 0 || iParamNum == 1) {
			return Translate(sTextField);
		}
		break;

	case INSTR_DELAY:
		if (iParamNum == 0) {
			return Translate(sTextField);
		}
		break;

	case INSTR_WAIT_FOR_BIT_SET:
	case INSTR_WAIT_FOR_BIT_CLEAR:
	case INSTR_SET_BITFIELD:
		if (iParamNum == 0 || iParamNum == 1 || iParamNum == 2) {
			return Translate(sTextField);
		}
		break;

	case INSTR_WAIT_FOR_BIT_PATTERN:
		if (iParamNum == 0 || iParamNum == 1 || iParamNum == 2
				|| iParamNum == 3) {
			return Translate(sTextField);
		}
		break;

	case INSTR_TEST_IF_ZERO_AND_SET:
	case INSTR_TEST_IF_NOT_ZERO_AND_SET:
		if (iParamNum == 0 || iParamNum == 1 || iParamNum == 2 || iParamNum == 3
				|| iParamNum == 4) {
			return Translate(sTextField);
		}
		break;

		// all instructions with scratch memory id in parameter 1
	case INSTR_TEST_SM_IF_ZERO_AND_SET:
	case INSTR_TEST_SM_IF_NOT_ZERO_AND_SET:
		if (iParamNum == 0 || iParamNum == 2) {
			if (sTextField.compare(0, 2, "SM") == 0) {
				string subs = sTextField.substr(2);
				return Translate(subs);
			}
		}
		if (iParamNum == 1 || iParamNum == 3 || iParamNum == 4) {
			return Translate(sTextField);
		}
		break;

	case INSTR_MOV_SM_SM:
	case INSTR_OR_SM_SM:
	case INSTR_AND_SM_SM:
	case INSTR_ADD_SM_SM:
	case INSTR_SUB_SM_SM:
	case INSTR_LOAD_SM_FROM_ADDR_IN_SM:
	case INSTR_STORE_SM_TO_ADDR_IN_SM:
		if (iParamNum == 0 || iParamNum == 1) {
			if (sTextField.compare(0, 2, "SM") == 0) {
				string subs = sTextField.substr(2);
				return Translate(subs);
			}
		}
		break;

	case INSTR_LOAD_SM_ADDR:
	case INSTR_LOAD_SM_VAL:
	case INSTR_STORE_SM_ADDR:
	case INSTR_RSHIFT_SM_VAL:
	case INSTR_LSHIFT_SM_VAL:
	case INSTR_AND_SM_VAL:
	case INSTR_OR_SM_VAL:
	case INSTR_ADD_SM_VAL:
	case INSTR_SUB_SM_VAL:
		if (iParamNum == 0) {
			if (sTextField.compare(0, 2, "SM") == 0) {
				string subs = sTextField.substr(2);
				return Translate(subs);
			}
		}

		if (iParamNum == 1) {
			return Translate(sTextField);
		}
		break;

		// all instructions with an operator in parameter 4
	case INSTR_TEST_SM_AND_BRANCH:
		if (iParamNum == 0) {
			if (sTextField.compare(0, 2, "SM") == 0) {
				string subs = sTextField.substr(2);
				return Translate(subs);
			}
		}
		if (iParamNum == 1 || iParamNum == 2) {
			return Translate(sTextField);
		}
		if (iParamNum == 3) {
			string sOp = sTextField.substr(0,
					sTextField.find_first_of(" \t\r\n"));
			return TextOperatorToInt(sOp);
		}
		if (iParamNum == 4) {
			if (IsAlphaNumeric(sTextField)) {
				char szText[4] = { 0 };
				int len = min(4, (int) sTextField.length());
				memcpy(szText, sTextField.substr(0, len).c_str(), len);
				return *((int*) szText);
			}
		}
		break;

	case INSTR_TEST_ADDR_AND_BRANCH:
		if (iParamNum == 0 || iParamNum == 1 || iParamNum == 2) {
			return Translate(sTextField);
		}

		if (iParamNum == 3) {
			string sOp = sTextField.substr(0,
					sTextField.find_first_of(" \t\r\n"));
			return TextOperatorToInt(sOp);
		}

		if (iParamNum == 4) {
			if (IsAlphaNumeric(sTextField)) {
				char szText[4] = { 0 };
				int len = min(4, (int) sTextField.length());
				memcpy(szText, sTextField.substr(0, len).c_str(), len);
				return *((int*) szText);
			}
		}
		break;
	}
	// default is 0
	return 0;
}

// static members initialization
t_InstructionList CInstructions::s_InstFmt;

CInstruction NOOP_INST(INSTR_NOP, "NOP", 0, "", "", "", "", "");
CInstruction WRITE_INST(INSTR_WRITE, "WRITE", 2, "Address", "Value", "", "",
		"");
CInstruction READ_INST(INSTR_READ, "READ", 2, "Address", "NumberOfReads", "",
		"", "");
CInstruction DELAY_INST(INSTR_DELAY, "DELAY", 1, "Value", "", "", "", "");
CInstruction WAIT_FOR_BIT_SET_INST(INSTR_WAIT_FOR_BIT_SET, "WAIT_FOR_BIT_SET",
		3, "Address", "Mask", "TimeOutValue", "", "");
CInstruction WAIT_FOR_BIT_CLEAR_INST(INSTR_WAIT_FOR_BIT_CLEAR,
		"WAIT_FOR_BIT_CLEAR", 3, "Address", "Mask", "TimeOutValue", "", "");
CInstruction AND_VAL_INST(INSTR_AND_VAL, "AND_VAL", 2, "Address", "Value", "",
		"", "");
CInstruction OR_VAL_INST(INSTR_OR_VAL, "OR_VAL", 2, "Address", "Value", "", "",
		"");
// new DDR script instructions
CInstruction SET_BITFIELD_INST(INSTR_SET_BITFIELD, "SET_BITFIELD", 3, "Address",
		"Mask", "Value", "", "");
CInstruction WAIT_FOR_BIT_PATTERN_INST(INSTR_WAIT_FOR_BIT_PATTERN,
		"WAIT_FOR_BIT_PATTERN", 4, "Address", "Mask", "Value", "TimeOutValue",
		"");
CInstruction TEST_IF_ZERO_AND_SET_INST(INSTR_TEST_IF_ZERO_AND_SET,
		"TEST_IF_ZERO_AND_SET", 5, "TestAddress", "TestMask", "SetAddress",
		"SetMask", "SetValue");
CInstruction TEST_IF_NOT_ZERO_AND_SET_INST(INSTR_TEST_IF_NOT_ZERO_AND_SET,
		"TEST_IF_NOT_ZERO_AND_SET", 5, "TestAddress", "TestMask", "SetAddress",
		"SetMask", "SetValue");
CInstruction LOAD_SM_ADDR_INST(INSTR_LOAD_SM_ADDR, "LOAD_SM_ADDR", 2,
		"ScratchMemoryID", "Address", "", "", "");
CInstruction LOAD_SM_VAL_INST(INSTR_LOAD_SM_VAL, "LOAD_SM_VAL", 2,
		"ScratchMemoryID", "Value", "", "", "");
CInstruction STORE_SM_ADDR_INST(INSTR_STORE_SM_ADDR, "STORE_SM_ADDR", 2,
		"ScratchMemoryID", "Address", "", "", "");
CInstruction MOV_SM_SM_INST(INSTR_MOV_SM_SM, "MOV_SM_SM", 2,
		"DestScratchMemoryID", "SrcScratchMemoryID", "", "", "");
CInstruction RSHIFT_SM_VAL_INST(INSTR_RSHIFT_SM_VAL, "RSHIFT_SM_VAL", 2,
		"ScratchMemoryID", "Value", "", "", "");
CInstruction LSHIFT_SM_VAL_INST(INSTR_LSHIFT_SM_VAL, "LSHIFT_SM_VAL", 2,
		"ScratchMemoryID", "Value", "", "", "");
CInstruction AND_SM_VAL_INST(INSTR_AND_SM_VAL, "AND_SM_VAL", 2,
		"ScratchMemoryID", "Value", "", "", "");
CInstruction OR_SM_VAL_INST(INSTR_OR_SM_VAL, "OR_SM_VAL", 2, "ScratchMemoryID",
		"Value", "", "", "");
CInstruction OR_SM_SM_INST(INSTR_OR_SM_SM, "OR_SM_SM", 2, "DestScratchMemoryID",
		"SrcScratchMemoryID", "", "", "");
CInstruction AND_SM_SM_INST(INSTR_AND_SM_SM, "AND_SM_SM", 2,
		"DestScratchMemoryID", "SrcScratchMemoryID", "", "", "");
CInstruction TEST_SM_IF_ZERO_AND_SET_INST(INSTR_TEST_SM_IF_ZERO_AND_SET,
		"TEST_SM_IF_ZERO_AND_SET", 5, "TestScratchMemoryID", "TestMask",
		"SetScratchMemoryID", "SetMask", "SetValue");
CInstruction TEST_SM_IF_NOT_ZERO_AND_SET_INST(INSTR_TEST_SM_IF_NOT_ZERO_AND_SET,
		"TEST_SM_IF_NOT_ZERO_AND_SET", 5, "TestScratchMemoryID", "TestMask",
		"SetScratchMemoryID", "SetMask", "SetValue");
CInstruction LABEL_INST(INSTR_LABEL, "LABEL", 1, "Label", "", "", "", "");
CInstruction TEST_ADDR_AND_BRANCH_INST(INSTR_TEST_ADDR_AND_BRANCH,
		"TEST_ADDR_AND_BRANCH", 5, "TestAddress", "TestMask", "TestValue",
		"Operation", "BranchLabel");
CInstruction TEST_SM_AND_BRANCH_INST(INSTR_TEST_SM_AND_BRANCH,
		"TEST_SM_AND_BRANCH", 5, "ScratchMemoryID", "Mask", "Value",
		"Operation", "BranchLabel");
CInstruction BRANCH_INST(INSTR_BRANCH, "BRANCH", 1, "Label", "", "", "", "");
CInstruction END_INST(INSTR_END, "END", 0, "", "", "", "", "");
CInstruction ADD_SM_VAL_INST(INSTR_ADD_SM_VAL, "ADD_SM_VAL", 2,
		"ScratchMemoryID", "Value", "", "", "");
CInstruction ADD_SM_SM_INST(INSTR_ADD_SM_SM, "ADD_SM_SM", 2,
		"DestScratchMemoryID", "SrcScratchMemoryID", "", "", "");
CInstruction SUB_SM_VAL_INST(INSTR_SUB_SM_VAL, "SUB_SM_VAL", 2,
		"ScratchMemoryID", "Value", "", "", "");
CInstruction SUB_SM_SM_INST(INSTR_SUB_SM_SM, "SUB_SM_SM", 2,
		"DestScratchMemoryID", "SrcScratchMemoryID", "", "", "");
// Load/Store Indirect
CInstruction LOAD_SM_FROM_ADDR_IN_SM(INSTR_LOAD_SM_FROM_ADDR_IN_SM,
		"LOAD_SM_FROM_ADDR_IN_SM", 2, "DestScratchMemoryID",
		"IndirectSrcScratchMemoryID", "", "", "");
CInstruction STORE_SM_TO_ADDR_IN_SM(INSTR_STORE_SM_TO_ADDR_IN_SM,
		"STORE_SM_TO_ADDR_IN_SM", 2, "DestScratchMemoryID",
		"IndirectSrcScratchMemoryID", "", "", "");
// pseudo instructions
// These instructions use pseudo op codes that are used by tools for syntax shortcuts.  The
// bootrom does not recognize the pseudo op code.  When TBB is writing the TIM.bin, the pseudo
// op codes are converted to one or more instructions using the instructions above.
CInstruction PP_TABLEHEADER_INST(INSTR_PP_TABLEHEADER, "PP_TABLEHEADER", 3,
		"WRITE Address 1", "WRITE Address 2", "WRITE Address 3", "", "");
CInstruction PP_WRITE_INST(INSTR_PP_WRITE, "PP_WRITE", 3, "WRITE Value 1",
		"WRITE Value 2", "WRITE Value 3", "", "");

//SVC Instructions
//CInstruction DEBUG_STOP(INSTR_DEBUG_STOP, "DEBUG_STOP", 0, "", "", "", "", "");
//CInstruction CLEAR_BITFIELD(INSTR_CLEAR_BITFIELD, "CLEAR_BITFIELD", 3, "Address", "Mask", "Value", "", "");
//CInstruction SVC_MEMCPY(INSTR_SVC_MEMCPY, "SVC_MEMCPY", 3, "SrcAddress", "DstAddress", "Size", "", "");
//CInstruction SVC_UART_PUTCHAR(INSTR_SVC_UART_PUTCHAR, "SVC_UART_PUTCHAR", 1, "Value", "", "", "", "");
//CInstruction SVC_UART_INIT(INSTR_SVC_UART_INIT, "SVC_UART_INIT", 0, "", "", "", "", "");

int CInstructions::s_InstCount = 0;

const string CInstructions::Begin("Instructions");
const string CInstructions::End("End Instructions");

CInstructions::CInstructions() :
		CErdBase(INSTRUCTIONS_ERD, INSTRUCTION_MAX) {
	s_InstCount++;

	if (s_InstCount == 1) {
		s_InstFmt.clear();
		s_InstFmt.push_back(&NOOP_INST);
		s_InstFmt.push_back(&WRITE_INST);
		s_InstFmt.push_back(&READ_INST);
		s_InstFmt.push_back(&DELAY_INST);
		s_InstFmt.push_back(&WAIT_FOR_BIT_SET_INST);
		s_InstFmt.push_back(&WAIT_FOR_BIT_CLEAR_INST);
		s_InstFmt.push_back(&AND_VAL_INST);
		s_InstFmt.push_back(&OR_VAL_INST);
		// new DDR script instructions
		s_InstFmt.push_back(&SET_BITFIELD_INST);
		s_InstFmt.push_back(&WAIT_FOR_BIT_PATTERN_INST);
		s_InstFmt.push_back(&TEST_IF_ZERO_AND_SET_INST);
		s_InstFmt.push_back(&TEST_IF_NOT_ZERO_AND_SET_INST);
		s_InstFmt.push_back(&LOAD_SM_ADDR_INST);
		s_InstFmt.push_back(&LOAD_SM_VAL_INST);
		s_InstFmt.push_back(&STORE_SM_ADDR_INST);
		s_InstFmt.push_back(&MOV_SM_SM_INST);
		s_InstFmt.push_back(&RSHIFT_SM_VAL_INST);
		s_InstFmt.push_back(&LSHIFT_SM_VAL_INST);
		s_InstFmt.push_back(&AND_SM_VAL_INST);
		s_InstFmt.push_back(&OR_SM_VAL_INST);
		s_InstFmt.push_back(&OR_SM_SM_INST);
		s_InstFmt.push_back(&AND_SM_SM_INST);
		s_InstFmt.push_back(&TEST_SM_IF_ZERO_AND_SET_INST);
		s_InstFmt.push_back(&TEST_SM_IF_NOT_ZERO_AND_SET_INST);
		// conditional execution branch and subroutine call instructions
		s_InstFmt.push_back(&LABEL_INST);
		s_InstFmt.push_back(&TEST_ADDR_AND_BRANCH_INST);
		s_InstFmt.push_back(&TEST_SM_AND_BRANCH_INST);
		s_InstFmt.push_back(&BRANCH_INST);
		// Interpreter Directives
		s_InstFmt.push_back(&END_INST);
		//Add/Subtract
		s_InstFmt.push_back(&ADD_SM_VAL_INST);
		s_InstFmt.push_back(&ADD_SM_SM_INST);
		s_InstFmt.push_back(&SUB_SM_VAL_INST);
		s_InstFmt.push_back(&SUB_SM_SM_INST);
		// Load/Store Indirect
		s_InstFmt.push_back(&LOAD_SM_FROM_ADDR_IN_SM);
		s_InstFmt.push_back(&STORE_SM_TO_ADDR_IN_SM);

		// pseudo instructions
		s_InstFmt.push_back(&PP_TABLEHEADER_INST);
		s_InstFmt.push_back(&PP_WRITE_INST);

	}
}

CInstructions::~CInstructions(void) {
	Reset();

	// delete static members when last CInstructions object is deleted
	if (--s_InstCount == 0) {
		if (s_InstFmt.size() > 0)
			s_InstFmt.clear();
	}
}

// copy constructor
CInstructions::CInstructions(const CInstructions& rhs) :
		CErdBase(rhs) {
	s_InstCount++;

	// need to do a deep copy of lists to avoid dangling references
	CInstructions& nc_rhs = const_cast<CInstructions&>(rhs);
	t_InstructionListIter iter = nc_rhs.m_InstructionsList.begin();
	while (iter != nc_rhs.m_InstructionsList.end()) {
		CInstruction* pInst = new CInstruction(*(*iter));
		m_InstructionsList.push_back(pInst);
		iter++;
	}

	t_stringListIter iterLab = nc_rhs.m_Labels.begin();
	while (iterLab != nc_rhs.m_Labels.end()) {
		string* pLabel = new string(*(*iterLab));
		m_Labels.push_back(pLabel);

		iterLab++;
	}
}

// assignment operator
CInstructions& CInstructions::operator=(const CInstructions& rhs) {
	// assignment operator
	if (&rhs != this) {
		CErdBase::operator=(rhs);
		Reset();

		// need to do a deep copy of lists to avoid dangling references
		CInstructions& nc_rhs = const_cast<CInstructions&>(rhs);
		t_InstructionListIter iter = nc_rhs.m_InstructionsList.begin();
		while (iter != nc_rhs.m_InstructionsList.end()) {
			CInstruction* pInst = new CInstruction(*(*iter));
			m_InstructionsList.push_back(pInst);
			iter++;
		}

		t_stringListIter iterLab = nc_rhs.m_Labels.begin();
		while (iterLab != nc_rhs.m_Labels.end()) {
			string* pLabel = new string(*(*iterLab));
			m_Labels.push_back(pLabel);

			iterLab++;
		}
	}

	return *this;
}

void CInstructions::Reset() {
	if (m_InstructionsList.size() > 0) {
		for_each(m_InstructionsList.begin(), m_InstructionsList.end(),
				[](CInstruction*& ps) {delete ps;});
		m_InstructionsList.clear();
	}

	DeleteLabels();
}

void CInstructions::DeleteLabels() {
	if (m_Labels.size() > 0) {
		for_each(m_Labels.begin(), m_Labels.end(),
				[](string*& ps) {delete ps;});
		m_Labels.clear();
	}
}

unsigned int CInstructions::PackageSize() {
	unsigned int iSize = 0;

	t_InstructionListIter iter = m_InstructionsList.begin();
	while (iter != m_InstructionsList.end())
		iSize += (*iter++)->Size();

	return iSize;
}

unsigned int CInstructions::NumInst() {
	unsigned int uiNum = 0;
	t_InstructionListIter iter = m_InstructionsList.begin();
	while (iter != m_InstructionsList.end()) {
		if (INSTR_PP_TABLEHEADER == (*iter)->m_InstructionOpCode) {
			iter++;
			continue;
		} else if (INSTR_PP_WRITE == (*iter)->m_InstructionOpCode) {
			uiNum += 3;
		} else {
			uiNum++;
		}
		iter++;
	}
	return uiNum;
}

bool CInstructions::Parse(CTimDescriptor& TimDescriptor,
		CTimDescriptorLine*& pLine, bool bIsBlf) {
	ParseLabels(TimDescriptor, pLine, bIsBlf);
	m_bChanged = false;

	while ((pLine = TimDescriptor.GetNextLineField(pLine))) {
		if (pLine->m_FieldName == (bIsBlf ? SpaceToUnderscore(End) : End)) {
			break;
		} else if (pLine->m_FieldName.find("End") == 0) {
			// found an "End" statment but not the one we are looking for!
			stringstream ss;
			ss << endl << "Error: Parsing of '"
					<< (bIsBlf ? SpaceToUnderscore(Begin) : Begin);
			ss << ".  Found unexpected <" << pLine->m_FieldName << ">";
			TimDescriptor.ParsingError(ss.str(), true, pLine);
			return false;
		}

		bool bFound = false;
		t_InstructionListIter iter = s_InstFmt.begin();
		while (iter != s_InstFmt.end()) {
			if (TrimWS(pLine->m_FieldName) == (*iter)->m_InstructionText) {
				CInstruction* pInstr = new CInstruction(*(*iter));
				string sValues = TrimWS(pLine->m_FieldValue);

				for (unsigned int i = 0; i < (*iter)->m_NumParamsUsed; i++) {
					if (bIsBlf) {
						size_t nPos = string::npos;
						// remove delimiters in an instruction line in a blf
						while ((nPos = sValues.find_first_of("<,>"))
								!= string::npos) {
							sValues.replace(nPos, 1, 1, ' ');
						}
					}

					if (IsHexNumeric(sValues) || IsNumeric(sValues)) {
						pInstr->m_ParamValues[i] = Translate(sValues);
					} else {
						string sAlphaField = sValues.substr(0,
								sValues.find_first_of(" \t\r\n"));
						pInstr->m_ParamValues[i] = pInstr->TextFieldValueToInt(
								i, sAlphaField);
					}

					// remove value that is now stored in m_ParamValues to access next value
					sValues = sValues.substr(
							sValues.find_first_of(" \t\r\n") + 1);
					sValues = TrimLeadingWS(sValues);
				}
				m_InstructionsList.push_back(pInstr);
				bFound = true;
				break;
			}
			iter++;
		}

		if (!bFound) {
			stringstream ss;
			ss << endl << "Error: Parsing of '"
					<< (bIsBlf ? SpaceToUnderscore(Begin) : Begin);
			TimDescriptor.ParsingError(ss.str(), true, pLine);
		}
	}

	// field not found
	return true;
}

bool CInstructions::ParseLabels(CTimDescriptor& TimDescriptor,
		CTimDescriptorLine*& pInstructionsLine, bool bIsBlf) {
	printf("\nParsing Instruction Labels....\n");

	// this function preparses the labels in this instructions object so they will be available
	// for any label references or to list in GUI
	CTimDescriptorLine* pLine = pInstructionsLine;
	DeleteLabels();

	while ((pLine = TimDescriptor.GetNextLineField(pLine))) {
		if (pLine->m_FieldName == (bIsBlf ? SpaceToUnderscore(End) : End)) {
			break;
		} else if (pLine->m_FieldName.find("End") == 0) {
			// found an "End" statment but not the one we are looking for!
			stringstream ss;
			ss << endl << "Error: Parsing of '"
					<< (bIsBlf ? SpaceToUnderscore(Begin) : Begin);
			ss << ".  Found unexpected <" << pLine->m_FieldName << ">";
			TimDescriptor.ParsingError(ss.str(), true, pLine);
			return false;
		}

		t_InstructionListIter iter = s_InstFmt.begin();
		while (iter != s_InstFmt.end()) {
			if (TrimWS(pLine->m_FieldName) == (*iter)->m_InstructionText) {
				CInstruction Instr = *(*iter);
				string sValues = TrimWS(pLine->m_FieldValue);

				if (Instr.m_InstructionOpCode == INSTR_LABEL
						|| Instr.m_InstructionOpCode == INSTR_BRANCH) {
					// get the parameters for the instruction
					for (unsigned int i = 0; i < Instr.m_NumParamsUsed; i++) {
						if (IsHexNumeric(sValues) || IsNumeric(sValues)) {
							Instr.m_ParamValues[i] = Translate(sValues);
						} else {
							string sAlphaField = sValues.substr(0,
									sValues.find_first_of(" \t\r\n"));
							Instr.m_ParamValues[i] = Instr.TextFieldValueToInt(
									i, sAlphaField);
						}

						// remove value that is now stored in m_ParamValues to access next value
						sValues = sValues.substr(
								sValues.find_first_of(" \t\r\n") + 1);
						sValues = TrimLeadingWS(sValues);
					}

					string sLabel(Instr.IntFieldValueToText(0));
					AttachLabel(sLabel);
					break;
				}
			}
			iter++;
		}
	}

	printf("\nFinished Parsing Instruction Labels....\n");

	// field not found
	return true;
}

bool CInstructions::ToBinary(ofstream& ofs) {
	bool bRet = true;
	return (ofs.good() && bRet);
}

int CInstructions::AddPkgStrings(CReservedPackageData* pRPD) {
	t_InstructionListIter iter = m_InstructionsList.begin();
	CInstruction* pPP_TableHeader = 0;

	while (iter != m_InstructionsList.end()) {
		if ((*iter)->m_InstructionOpCode == INSTR_PP_TABLEHEADER) {
			// do not add to reserved data
			delete pPP_TableHeader;
			pPP_TableHeader = new CInstruction(*(*iter));
			iter++;
			continue;
		}

		if (((*iter)->m_InstructionOpCode == INSTR_PP_WRITE)
				&& (pPP_TableHeader != 0)) {
			CInstruction Write(WRITE_INST);
			// WRITE 1
			Write.m_ParamValues[0] = pPP_TableHeader->m_ParamValues[0];
			Write.m_ParamValues[1] = (*iter)->m_ParamValues[0];

			pRPD->AddData(
					new string(HexFormattedAscii(Write.m_InstructionOpCode)),
					new string(Write.m_InstructionText));

			for (unsigned int j = 0; j < Write.m_NumParamsUsed; j++) {
				pRPD->AddData(
						new string(HexFormattedAscii(Write.m_ParamValues[j])),
						new string(Write.m_ParamNames[j]));
			}

			// WRITE 2
			Write.m_ParamValues[0] = pPP_TableHeader->m_ParamValues[1];
			Write.m_ParamValues[1] = (*iter)->m_ParamValues[1];

			pRPD->AddData(
					new string(HexFormattedAscii(Write.m_InstructionOpCode)),
					new string(Write.m_InstructionText));

			for (unsigned int j = 0; j < Write.m_NumParamsUsed; j++) {
				pRPD->AddData(
						new string(HexFormattedAscii(Write.m_ParamValues[j])),
						new string(Write.m_ParamNames[j]));
			}

			// WRITE 3
			Write.m_ParamValues[0] = pPP_TableHeader->m_ParamValues[2];
			Write.m_ParamValues[1] = (*iter)->m_ParamValues[2];

			pRPD->AddData(
					new string(HexFormattedAscii(Write.m_InstructionOpCode)),
					new string(Write.m_InstructionText));

			for (unsigned int j = 0; j < Write.m_NumParamsUsed; j++) {
				pRPD->AddData(
						new string(HexFormattedAscii(Write.m_ParamValues[j])),
						new string(Write.m_ParamNames[j]));
			}

			iter++;
			continue;
		}

		pRPD->AddData(
				new string(HexFormattedAscii((*iter)->m_InstructionOpCode)),
				new string((*iter)->m_InstructionText));
		for (unsigned int j = 0; j < (*iter)->m_NumParamsUsed; j++) {
			pRPD->AddData(
					new string(HexFormattedAscii((*iter)->m_ParamValues[j])),
					new string((*iter)->m_ParamNames[j]));
		}
		iter++;
	}

	delete pPP_TableHeader;

	return PackageSize();
}

string* CInstructions::AttachLabel(string& sLabel) {
	string* pLabel = 0;
	if (sLabel != "") {
		t_stringListIter iter = m_Labels.begin();
		while (iter != m_Labels.end()) {
			if (*(*iter) == sLabel)
				return *iter;
			iter++;
		}

		// no string pointer yet, so make new string
		pLabel = new string(sLabel);
		m_Labels.push_back(pLabel);
	}
	return pLabel;
}
