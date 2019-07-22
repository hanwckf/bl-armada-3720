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


#include "GenPurposePatch.h"
#include "TimDescriptorParser.h"

const string CGenPurposePatch::Begin("GPP");
const string CGenPurposePatch::End("End GPP");

CGenPurposePatch::CGenPurposePatch() :
		CErdBase(GPP_ERD, GPP_FIELDS_MAX) {
	*m_FieldNames[GPP_PID] = "GPP_PID";
	m_sGPP_PID = "";
}

CGenPurposePatch::~CGenPurposePatch(void) {

}

// copy constructor
CGenPurposePatch::CGenPurposePatch(const CGenPurposePatch& rhs) :
		CErdBase(rhs) {
	m_sGPP_PID = rhs.m_sGPP_PID;

	m_GPPOperations = rhs.m_GPPOperations;
	m_GPPInstructions = rhs.m_GPPInstructions;
}

// assignment operator
CGenPurposePatch& CGenPurposePatch::operator=(const CGenPurposePatch& rhs) {
	// assignment operator
	if (&rhs != this) {
		CErdBase::operator=(rhs);
		m_sGPP_PID = rhs.m_sGPP_PID;
		m_GPPOperations = rhs.m_GPPOperations;
		m_GPPInstructions = rhs.m_GPPInstructions;
	}
	return *this;
}

unsigned int CGenPurposePatch::PackageSize() {
	// 8(WRAH) + 8(NumOps & NumInst) + instructions actual size + ops actual size
	return 8 + 8 + m_GPPInstructions.PackageSize()
			+ m_GPPOperations.PackageSize();
}

bool CGenPurposePatch::Parse(CTimDescriptor& TimDescriptor,
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
		CErdBase* pErd = 0;

		if (pLine->m_FieldName == (bIsBlf ? SpaceToUnderscore(End) : End)) {
			break;
		}

		bool bFound = false;
		for (unsigned int idx = 0; idx < m_FieldNames.size(); idx++) {
			if (TrimWS(pLine->m_FieldName) == *m_FieldNames[idx]) {
				if (idx == GPP_PID)
					m_sGPP_PID = TrimWS(pLine->m_FieldValue);
				else
					*m_FieldValues[idx] = TrimWS(pLine->m_FieldValue);

				bFound = true;
				break;
			}
		}

		if (bFound)
			continue;

		if (pLine->m_FieldName.find(
				bIsBlf ?
						SpaceToUnderscore(CGPPOperations::BlfBegin) :
						CGPPOperations::Begin) != string::npos) {
			pErd = &m_GPPOperations;
			ParseERDPackage(TimDescriptor, pLine, pErd,
					(bIsBlf ?
							SpaceToUnderscore(CGPPOperations::BlfBegin) :
							CGPPOperations::Begin),
					(bIsBlf ?
							SpaceToUnderscore(CGPPOperations::BlfEnd) :
							CGPPOperations::End), bIsBlf);

			bFound = true;
			continue;
		} else if (pLine->m_FieldName.find(
				bIsBlf ?
						SpaceToUnderscore(CInstructions::Begin) :
						CInstructions::Begin) != string::npos) {
			pErd = &m_GPPInstructions;
			ParseERDPackage(TimDescriptor, pLine, pErd,
					(bIsBlf ?
							SpaceToUnderscore(CInstructions::Begin) :
							CInstructions::Begin),
					(bIsBlf ?
							SpaceToUnderscore(CInstructions::End) :
							CInstructions::End), bIsBlf);

			bFound = true;
			continue;
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

bool CGenPurposePatch::ParseERDPackage(CTimDescriptor& TimDescriptor,
		CTimDescriptorLine*& pLine, CErdBase*& pErd, const string& sBegin,
		const string& sEnd, bool bIsBlf) {

	if ((!pLine) || !pErd->Parse(TimDescriptor, pLine, bIsBlf)
			|| (pLine->m_FieldName.find(bIsBlf ? SpaceToUnderscore(sEnd) : sEnd)
					== string::npos)) {
		stringstream ss;
		ss << endl << "Error: Parsing of '"
				<< (bIsBlf ? SpaceToUnderscore(sBegin) : sBegin);
		TimDescriptor.ParsingError(ss.str(), true, pLine);
		return false;
	}
	return true;
}

bool CGenPurposePatch::ToBinary(ofstream& ofs) {
	bool bRet = true;
	return (ofs.good() && bRet);
}

int CGenPurposePatch::AddPkgStrings(CReservedPackageData* pRPD) {
	string sData;
	sData = TextToHexFormattedAscii(m_sGPP_PID);
	pRPD->PackageIdTag(sData);

	string* pData = new string;
	*pData = HexFormattedAscii(m_GPPOperations.NumOps());
	pRPD->AddData(pData, new string("NumOps"));

	pData = new string;
	*pData = HexFormattedAscii(m_GPPInstructions.NumInst());
	pRPD->AddData(pData, new string("NumInst"));

	m_GPPOperations.AddPkgStrings(pRPD);
	m_GPPInstructions.AddPkgStrings(pRPD);

	return PackageSize();
}
