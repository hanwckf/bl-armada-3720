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

#include "TzOperations.h"
#include "TimDescriptorParser.h"

CTzOperation::CTzOperation() :
		CTimLib() {
	m_OperationId = TZ_NOP;
	m_sOpIdText = "NOP";
	m_Value = 0;
}

CTzOperation::CTzOperation(TZ_OPERATION_SPEC_T OpId, const string & sOpIdText) :
		m_OperationId(OpId), m_sOpIdText(sOpIdText), CTimLib() {
	m_Value = 0;
}

CTzOperation::~CTzOperation(void) {

}

CTzOperation::CTzOperation(const CTzOperation& rhs) :
		CTimLib(rhs) {
	// copy constructor
	m_OperationId = rhs.m_OperationId;
	m_sOpIdText = rhs.m_sOpIdText;
	m_Value = rhs.m_Value;

}

CTzOperation& CTzOperation::operator=(const CTzOperation& rhs) {
	// assignment operator
	if (&rhs != this) {
		CTimLib::operator=(rhs);

		m_OperationId = rhs.m_OperationId;
		m_sOpIdText = rhs.m_sOpIdText;
		m_Value = rhs.m_Value;

	}
	return *this;
}

bool CTzOperation::SetOperationID(TZ_OPERATION_SPEC_T eOpId) {
	t_TzOperationListIter Iter = CTzOperations::s_DefinedTzOperations.begin();
	while (Iter != CTzOperations::s_DefinedTzOperations.end()) {
		if ((*Iter)->m_OperationId == eOpId) {
			*this = *(*Iter);
			break;
		}
		Iter++;
	}
	return true;
}

bool CTzOperation::SetOperationID(string& sOpIdText) {
	t_TzOperationListIter Iter = CTzOperations::s_DefinedTzOperations.begin();
	while (Iter != CTzOperations::s_DefinedTzOperations.end()) {
		if ((*Iter)->m_sOpIdText == sOpIdText) {
			*this = *(*Iter);
			break;
		}
		Iter++;
	}
	return true;
}

bool CTzOperation::ToBinary(ofstream& ofs) {
	return ofs.good();
}

t_TzOperationList CTzOperations::s_DefinedTzOperations;
static CTzOperation NOOP_OP(TZ_NOP, "NOP");
static CTzOperation TZ_CONFIG_ENABLE_OP(TZ_CONFIG_ENABLE, "TZ_CONFIG_ENABLE");
static CTzOperation TZ_IGNORE_INST_TO_OP(TZ_IGNORE_INST_TO,
		"TZ_IGNORE_INST_TO");

int CTzOperations::s_TzOpCount = 0;

const string CTzOperations::Begin("TZ Operations");
const string CTzOperations::End("End TZ Operations");

CTzOperations::CTzOperations() :
		CErdBase(TZ_OPERATIONS_ERD, OPERATIONS_MAX) {
	s_TzOpCount++;

	if (s_TzOpCount == 1) {
		s_DefinedTzOperations.clear();
		s_DefinedTzOperations.push_back(&NOOP_OP);
		s_DefinedTzOperations.push_back(&TZ_CONFIG_ENABLE_OP);
		s_DefinedTzOperations.push_back(&TZ_IGNORE_INST_TO_OP);
	}
}

CTzOperations::~CTzOperations(void) {
	Reset();
	if (--s_TzOpCount == 0) {
		if (s_DefinedTzOperations.size() > 0)
			s_DefinedTzOperations.clear();
	}
}

// copy constructor
CTzOperations::CTzOperations(const CTzOperations& rhs) :
		CErdBase(rhs) {
	s_TzOpCount++;

	// need to do a deep copy of lists to avoid dangling references
	CTzOperations& nc_rhs = const_cast<CTzOperations&>(rhs);

	t_TzOperationListIter iter = nc_rhs.m_TzOperationsList.begin();
	while (iter != nc_rhs.m_TzOperationsList.end()) {
		CTzOperation* pOp = new CTzOperation(*(*iter));
		m_TzOperationsList.push_back(pOp);
		iter++;
	}
}

// assignment operator
CTzOperations& CTzOperations::operator=(const CTzOperations& rhs) {
	// assignment operator
	if (&rhs != this) {
		Reset();

		CErdBase::operator=(rhs);

		// need to do a deep copy of lists to avoid dangling references
		CTzOperations& nc_rhs = const_cast<CTzOperations&>(rhs);

		t_TzOperationListIter iter = nc_rhs.m_TzOperationsList.begin();
		while (iter != nc_rhs.m_TzOperationsList.end()) {
			CTzOperation* pOp = new CTzOperation(*(*iter));
			m_TzOperationsList.push_back(pOp);
			iter++;
		}
	}
	return *this;
}

void CTzOperations::Reset() {

	if (m_TzOperationsList.size() > 0) {
		for_each(m_TzOperationsList.begin(), m_TzOperationsList.end(),
				[](CTzOperation*& ps) {delete ps;});
		m_TzOperationsList.clear();
	}
}

unsigned int CTzOperations::PackageSize() {
	unsigned int iSize = (unsigned int) m_TzOperationsList.size() * 8;

	return iSize;
}

bool CTzOperations::Parse(CTimDescriptor& TimDescriptor,
		CTimDescriptorLine*& pLine, bool bIsBlf) {
	m_bChanged = false;

	if (pLine->m_FieldName != (bIsBlf ? SpaceToUnderscore(Begin) : Begin)) {
		stringstream ss;
		ss << endl << "Error: Parsing of '"
				<< (bIsBlf ? SpaceToUnderscore(Begin) : Begin);
		TimDescriptor.ParsingError(ss.str(), true, pLine);
		return false;
	}

	while ((pLine = TimDescriptor.GetNextLineField(pLine))) {
		if (pLine->m_FieldName == (bIsBlf ? SpaceToUnderscore(End) : End)) {
			break;
		}

		bool bFound = false;
		t_TzOperationListIter iter = s_DefinedTzOperations.begin();
		while (iter != s_DefinedTzOperations.end()) {
			if (TrimWS(pLine->m_FieldName) == (*iter)->m_sOpIdText) {
				CTzOperation* pOp = new CTzOperation(*(*iter));
				pOp->m_Value = Translate(pLine->m_FieldValue);
				m_TzOperationsList.push_back(pOp);

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
			break;
		}
	}

	// field not found
	return true;
}

bool CTzOperations::ToBinary(ofstream& ofs) {
	bool bRet = true;
	return (ofs.good() && bRet);
}

int CTzOperations::AddPkgStrings(CReservedPackageData* pRPD) {
	t_TzOperationListIter iter = m_TzOperationsList.begin();
	while (iter != m_TzOperationsList.end()) {
		pRPD->AddData(new string(HexFormattedAscii((*iter)->m_OperationId)),
				new string((*iter)->m_sOpIdText));
		pRPD->AddData(new string(HexFormattedAscii((*iter)->m_Value)),
				new string("Value"));
		iter++;
	}

	return PackageSize();
}

