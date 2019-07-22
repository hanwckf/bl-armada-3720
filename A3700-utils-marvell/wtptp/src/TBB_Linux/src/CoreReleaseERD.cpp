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

#include "CoreReleaseERD.h"
#include "TimDescriptorParser.h"

const string CCorePair::Begin("Core Pair");
const string CCorePair::End("End Core Pair");

CCorePair::CCorePair() :
		CErdBase(CORE_PAIR_ERD, CORE_PAIR_MAX)

{
	*m_FieldNames[CORE_ID_V2_FIELD] = "Core ID";
	*m_FieldNames[VECTOR_ADDRESS_FIELD] = "Vector Address";
}

CCorePair::~CCorePair(void) {

}

CCorePair::CCorePair(const CCorePair& rhs) :
		CErdBase(rhs) {

}

CCorePair& CCorePair::operator=(const CCorePair& rhs) {
	// assignment operator
	if (&rhs != this) {
		CErdBase::operator=(rhs);
	}
	return *this;
}

bool CCorePair::ToBinary(ofstream& ofs) {
	// validate size
	if (m_FieldValues.size() != m_iMaxFieldNum)
		return false;

	ofs << Translate(*m_FieldValues[CORE_ID_V2_FIELD]);
	ofs << Translate(*m_FieldValues[VECTOR_ADDRESS_FIELD]);

	return ofs.good();
}

int CCorePair::AddPkgStrings(CReservedPackageData* pRPD) {
	pRPD->AddData(new string(*m_FieldValues[CORE_ID_V2_FIELD]),
			new string("Core ID"));
	pRPD->AddData(new string(*m_FieldValues[VECTOR_ADDRESS_FIELD]),
			new string("Vector Address"));

	return PackageSize();
}

bool CCorePair::Parse(CTimDescriptor& TimDescriptor, CTimDescriptorLine*& pLine,
		bool bIsBlf, string& sNum) {
	m_bChanged = false;

	while ((pLine = TimDescriptor.GetNextLineField(pLine))) {
		if (pLine->m_FieldName
				== (bIsBlf ?
						sNum + SpaceToUnderscore("End " + PackageName()) :
						("End " + PackageName()))) {
			return true;
		}

		bool bFound = false;
		for (unsigned int idx = 0; idx < m_FieldNames.size(); idx++) {
			if (pLine->m_FieldName
					== (bIsBlf ?
							sNum + SpaceToUnderscore(*m_FieldNames[idx]) :
							*m_FieldNames[idx])) {
				*m_FieldValues[idx] = pLine->m_FieldValue;
				bFound = true;
				break;
			}
		}

		if (!bFound) {
			stringstream ss;
			ss << endl << "Error: Parsing of '"
					<< (bIsBlf ?
							sNum + SpaceToUnderscore(PackageName()) :
							PackageName());
			TimDescriptor.ParsingError(ss.str(), true, pLine);
			break;
		}
	}

	// field not found  
	return true;
}

const string CCoreRelease::Begin("Core Release");
const string CCoreRelease::End("End Core Release");

CCoreRelease::CCoreRelease() :
		CErdBase(CORE_RELEASE_ERD, CORE_RELEASE_MAX) {
	*m_FieldNames[NUM_PAIRS_FIELD] = "NUM_PAIRS";
}

CCoreRelease::~CCoreRelease() {
	Reset();
}

// copy constructor
CCoreRelease::CCoreRelease(const CCoreRelease& rhs) :
		CErdBase(rhs) {
	// need to do a deep copy of lists to avoid dangling references
	CCoreRelease& nc_rhs = const_cast<CCoreRelease&>(rhs);
	if (nc_rhs.Pairs.size() > 0) {
		for_each(nc_rhs.Pairs.begin(), nc_rhs.Pairs.end(),
				[this](CCorePair*& pRhsCore)
				{
					CCorePair* pCore = new CCorePair(*pRhsCore);
					Pairs.push_back(pCore);
				});
	}
}

// assignment operator
CCoreRelease& CCoreRelease::operator=(const CCoreRelease& rhs) {
	// assignment operator
	if (&rhs != this) {
		CErdBase::operator=(rhs);

		Reset();

		// need to do a deep copy of lists to avoid dangling references
		CCoreRelease& nc_rhs = const_cast<CCoreRelease&>(rhs);
		if (nc_rhs.Pairs.size() > 0) {
			for_each(nc_rhs.Pairs.begin(), nc_rhs.Pairs.end(),
					[this](CCorePair*& pRhsCore)
					{
						CCorePair* pCore = new CCorePair(*pRhsCore);
						Pairs.push_back(pCore);
					});
		}
	}
	return *this;
}

void CCoreRelease::Reset() {
	if (Pairs.size() > 0) {
		for_each(Pairs.begin(), Pairs.end(),
				[](CCorePair*& pPair) {delete pPair;});
		Pairs.clear();
	}
}

bool CCoreRelease::Parse(CTimDescriptor& TimDescriptor,
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
		for (unsigned int idx = 0; idx < m_FieldNames.size(); idx++) {
			if (pLine->m_FieldName == *m_FieldNames[idx]) {
				*m_FieldValues[idx] = pLine->m_FieldValue;
				bFound = true;
				break;
			}
		}

		if (bFound)
			continue;

		stringstream ssCorePairNum;
		int nCorePairs = Pairs.size() + 1;
		ssCorePairNum.str("");
		ssCorePairNum << nCorePairs << "_";

		CCorePair* pCorePair = 0;
		if (pLine->m_FieldName
				== (bIsBlf ?
						ssCorePairNum.str()
								+ SpaceToUnderscore(CCorePair::Begin) :
						CCorePair::Begin)) {
			pCorePair = new CCorePair;
			string sCorePairNum = ssCorePairNum.str(); // this syntax if for linux compiler
			if (pCorePair
					&& !pCorePair->Parse(TimDescriptor, pLine, bIsBlf,
							sCorePairNum)) {
				stringstream ss;
				ss << endl << "Error: Parsing of '"
						<< (bIsBlf ?
								ssCorePairNum.str()
										+ SpaceToUnderscore(CCorePair::Begin) :
								CCorePair::Begin);
				TimDescriptor.ParsingError(ss.str(), true, pLine);
				delete pCorePair;
				return false;
			}

			if (pCorePair
					&& pLine->m_FieldName
							== (bIsBlf ?
									ssCorePairNum.str()
											+ SpaceToUnderscore(
													CCorePair::End) :
									CCorePair::End)) {
				Pairs.push_back(pCorePair);
				continue;
			}
		}

		stringstream ss;
		ss << endl << "Error: Parsing of '"
				<< (bIsBlf ?
						ssCorePairNum.str()
								+ SpaceToUnderscore(CCorePair::End) :
						CCorePair::End);
		TimDescriptor.ParsingError(ss.str(), true, pLine);
		delete pCorePair;
		return false;
	}

	if (Translate(*m_FieldValues[NUM_PAIRS_FIELD]) != Pairs.size()) {
		stringstream ss;
		ss << endl
				<< "\nError: Parsing of CorePairs, NUM_PAIRS not equal to actual number of Core Pairs defined in the Core Release";
		TimDescriptor.ParsingError(ss.str(), true, pLine);
		return false;
	}

	return true;
}

bool CCoreRelease::ToBinary(ofstream& ofs) {
	// validate size
	if (m_FieldValues.size() != m_iMaxFieldNum)
		return false;

	bool bRet = true;
	ofs << COREID_V2;
	ofs << PackageSize();

	ofs << Translate(*m_FieldNames[NUM_PAIRS_FIELD]);

	for_each(Pairs.begin(), Pairs.end(),
			[this, &ofs, &bRet](CCorePair*& pRhsCore)
			{
				if ( bRet )
				bRet = pRhsCore->ToBinary(ofs);
			});

	return (ofs.good() && bRet);
}

int CCoreRelease::AddPkgStrings(CReservedPackageData* pRPD) {
	pRPD->PackageIdTag(HexFormattedAscii(COREID_V2));
	pRPD->AddData(new string(*m_FieldValues[NUM_PAIRS_FIELD]),
			new string("NUM_PAIRS"));

	for_each(Pairs.begin(), Pairs.end(),
			[&pRPD](CCorePair*& pPair) {pPair->AddPkgStrings(pRPD);});

	return PackageSize();
}

unsigned int CCoreRelease::PackageSize() {
	return (unsigned int) (8 + // package tag id + size
			(m_FieldValues.size() * 4) // all fields
			+ (Pairs.size() * (CCorePair::CORE_PAIR_MAX * 4)));	// all data in data list
}
