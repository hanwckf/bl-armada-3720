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

#include "GPPOperations.h"
#include "TimDescriptorParser.h"

CGPPOperation::CGPPOperation() :
		CTimLib() {
	m_OperationId = GPP_NOP;
	m_sOpIdText = "NOP";
	m_Value = 0;
}

CGPPOperation::CGPPOperation(GPP_OPERATION_SPEC_T OpId,
		const string & sOpIdText) :
		m_OperationId(OpId), m_sOpIdText(sOpIdText), CTimLib() {
	m_Value = 0;
}

CGPPOperation::~CGPPOperation(void) {

}

CGPPOperation::CGPPOperation(const CGPPOperation& rhs) :
		CTimLib(rhs) {
	// copy constructor
	m_OperationId = rhs.m_OperationId;
	m_sOpIdText = rhs.m_sOpIdText;
	m_Value = rhs.m_Value;
}

CGPPOperation& CGPPOperation::operator=(const CGPPOperation& rhs) {
	// assignment operator
	if (&rhs != this) {
		CTimLib::operator=(rhs);

		m_OperationId = rhs.m_OperationId;
		m_sOpIdText = rhs.m_sOpIdText;
		m_Value = rhs.m_Value;
	}
	return *this;
}

bool CGPPOperation::SetOperationID(GPP_OPERATION_SPEC_T eOpId) {
	t_GPPOperationListIter Iter =
			CGPPOperations::s_DefinedGPPOperations.begin();
	while (Iter != CGPPOperations::s_DefinedGPPOperations.end()) {
		if ((*Iter)->m_OperationId == eOpId) {
			*this = *(*Iter);
			break;
		}
		Iter++;
	}
	return true;
}

bool CGPPOperation::SetOperationID(string& sOpIdText) {
	t_GPPOperationListIter Iter =
			CGPPOperations::s_DefinedGPPOperations.begin();
	while (Iter != CGPPOperations::s_DefinedGPPOperations.end()) {
		if ((*Iter)->m_sOpIdText == sOpIdText) {
			*this = *(*Iter);
			break;
		}
		Iter++;
	}
	return true;
}

bool CGPPOperation::ToBinary(ofstream& ofs) {
	return ofs.good();
}

t_GPPOperationList CGPPOperations::s_DefinedGPPOperations;
static CGPPOperation NOOP_OP(GPP_NOP, "NOP");
static CGPPOperation GPP_IGNORE_INST_TO_OP(GPP_IGNORE_INST_TO,
		"GPP_IGNORE_INST_TO");

int CGPPOperations::s_GPPOpCount = 0;

const string CGPPOperations::Begin("GPP Operations");
const string CGPPOperations::End("End GPP Operations");
const string CGPPOperations::BlfBegin("GPPOperations");
const string CGPPOperations::BlfEnd("End_GPPOperations");

CGPPOperations::CGPPOperations() :
		CErdBase(GPP_OPERATIONS_ERD, OPERATIONS_MAX) {
	s_GPPOpCount++;

	if (s_GPPOpCount == 1) {
		s_DefinedGPPOperations.clear();
		s_DefinedGPPOperations.push_back(&NOOP_OP);
		s_DefinedGPPOperations.push_back(&GPP_IGNORE_INST_TO_OP);
	}
}

CGPPOperations::~CGPPOperations(void) {
	Reset();

	if (--s_GPPOpCount == 0) {
		if (s_DefinedGPPOperations.size() > 0)
			s_DefinedGPPOperations.clear();
	}
}

// copy constructor
CGPPOperations::CGPPOperations(const CGPPOperations& rhs) :
		CErdBase(rhs) {
	s_GPPOpCount++;

	// need to do a deep copy of lists to avoid dangling references
	CGPPOperations& nc_rhs = const_cast<CGPPOperations&>(rhs);

	t_GPPOperationListIter iter = nc_rhs.m_GPPOperationsList.begin();
	while (iter != nc_rhs.m_GPPOperationsList.end()) {
		CGPPOperation* pOp = new CGPPOperation(*(*iter));
		m_GPPOperationsList.push_back(pOp);
		iter++;
	}
}

// assignment operator
CGPPOperations& CGPPOperations::operator=(const CGPPOperations& rhs) {
	// assignment operator
	if (&rhs != this) {
		Reset();

		CErdBase::operator=(rhs);

		// need to do a deep copy of lists to avoid dangling references
		CGPPOperations& nc_rhs = const_cast<CGPPOperations&>(rhs);
		t_GPPOperationListIter iter = nc_rhs.m_GPPOperationsList.begin();
		while (iter != nc_rhs.m_GPPOperationsList.end()) {
			CGPPOperation* pOp = new CGPPOperation(*(*iter));
			m_GPPOperationsList.push_back(pOp);
			iter++;
		}
	}
	return *this;
}

void CGPPOperations::Reset() {
	if (m_GPPOperationsList.size() > 0) {
		for_each(m_GPPOperationsList.begin(), m_GPPOperationsList.end(),
				[](CGPPOperation*& pOp) {delete pOp;});
		m_GPPOperationsList.clear();
	}
}

unsigned int CGPPOperations::PackageSize() {
	unsigned int iSize = (unsigned int) m_GPPOperationsList.size() * 8;

	return iSize;
}

bool CGPPOperations::Parse(CTimDescriptor& TimDescriptor,
		CTimDescriptorLine*& pLine, bool bIsBlf) {
	m_bChanged = false;

	if (pLine->m_FieldName != (bIsBlf ? SpaceToUnderscore(BlfBegin) : Begin)) {
		stringstream ss;
		ss << endl << "Error: Parsing of '"
				<< (bIsBlf ? SpaceToUnderscore(BlfBegin) : Begin);
		TimDescriptor.ParsingError(ss.str(), true, pLine);
		return false;
	}

	while ((pLine = TimDescriptor.GetNextLineField(pLine))) {
		if (pLine->m_FieldName == (bIsBlf ? SpaceToUnderscore(BlfEnd) : End)) {
			break;
		}

		bool bFound = false;
		t_GPPOperationListIter iter = s_DefinedGPPOperations.begin();
		while (iter != s_DefinedGPPOperations.end()) {
			if (TrimWS(pLine->m_FieldName) == (*iter)->m_sOpIdText) {
				CGPPOperation* pOp = new CGPPOperation(*(*iter));
				pOp->m_Value = Translate(pLine->m_FieldValue);
				m_GPPOperationsList.push_back(pOp);
				bFound = true;
				break;
			}
			iter++;
		}

		if (!bFound) {
			stringstream ss;
			ss << endl << "Error: Parsing of '"
					<< (bIsBlf ? SpaceToUnderscore(BlfBegin) : Begin);
			TimDescriptor.ParsingError(ss.str(), true, pLine);
			break;
		}
	}

	// field not found
	return true;
}

bool CGPPOperations::ToBinary(ofstream& ofs) {
	bool bRet = true;
	return (ofs.good() && bRet);
}

int CGPPOperations::AddPkgStrings(CReservedPackageData* pRPD) {
	t_GPPOperationListIter iter = m_GPPOperationsList.begin();
	while (iter != m_GPPOperationsList.end()) {
		pRPD->AddData(new string(HexFormattedAscii((*iter)->m_OperationId)),
				new string((*iter)->m_sOpIdText));
		pRPD->AddData(new string(HexFormattedAscii((*iter)->m_Value)),
				new string("Value"));
		iter++;
	}

	return PackageSize();
}
