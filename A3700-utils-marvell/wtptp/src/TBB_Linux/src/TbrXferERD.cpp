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

#include "TbrXferERD.h"
#include "TimDescriptorParser.h"

const string CXfer::Begin("Xfer");
const string CXfer::End("End Xfer");

CXfer::CXfer() :
		CErdBase(XFER_ERD, XFER_MAX) {
	*m_FieldNames[DATA_ID] = "DATA_ID";
	*m_FieldNames[LOCATION] = "LOCATION";
}

CXfer::~CXfer() {
}

// copy constructor
CXfer::CXfer(const CXfer& rhs) :
		CErdBase(rhs) {
	// copy constructor
}

// assignment operator
CXfer& CXfer::operator=(const CXfer& rhs) {
	// assignment operator
	if (&rhs != this) {
		CErdBase::operator=(rhs);
	}
	return *this;
}

bool CXfer::ToBinary(ofstream& ofs) {
	// validate size
	if (m_FieldValues.size() != m_iMaxFieldNum)
		return false;

	ofs << Translate(*m_FieldValues[DATA_ID]);
	ofs << Translate(*m_FieldValues[LOCATION]);

	return ofs.good();
}

int CXfer::AddPkgStrings(CReservedPackageData* pRPD) {
	pRPD->AddData(new string(*m_FieldValues[DATA_ID]), new string("DATA_ID"));
	pRPD->AddData(new string(*m_FieldValues[LOCATION]), new string("LOCATION"));

	return PackageSize();
}

bool CXfer::Parse(CTimDescriptor& TimDescriptor, CTimDescriptorLine*& pLine,
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

const string CTBRXferSet::Begin("TBR Xfer Set");
const string CTBRXferSet::End("End TBR Xfer Set");

CTBRXferSet::CTBRXferSet() :
		CErdBase(TBRXFER_ERD, TBR_XFER_SET_MAX) {
	*m_FieldNames[XFER_LOC] = "XFER_LOC";
	*m_FieldNames[NUM_DATA_PAIRS] = "NUM_DATA_PAIRS";
}

CTBRXferSet::~CTBRXferSet() {
	Reset();
}

// copy constructor
CTBRXferSet::CTBRXferSet(const CTBRXferSet& rhs) :
		CErdBase(rhs) {
	// need to do a deep copy of lists to avoid dangling references
	CTBRXferSet& nc_rhs = const_cast<CTBRXferSet&>(rhs);

	t_XferListIter iter = nc_rhs.Xfers.begin();
	while (iter != nc_rhs.Xfers.end()) {
		CXfer* pXfer = new CXfer(*(*iter));
		Xfers.push_back(pXfer);

		iter++;
	}
}

// assignment operator
CTBRXferSet& CTBRXferSet::operator=(const CTBRXferSet& rhs) {
	// assignment operator
	if (&rhs != this) {
		CErdBase::operator=(rhs);

		Reset();

		// need to do a deep copy of lists to avoid dangling references
		CTBRXferSet& nc_rhs = const_cast<CTBRXferSet&>(rhs);

		t_XferListIter iter = nc_rhs.Xfers.begin();
		while (iter != nc_rhs.Xfers.end()) {
			CXfer* pXfer = new CXfer(*(*iter));
			Xfers.push_back(pXfer);

			iter++;
		}
	}
	return *this;
}

void CTBRXferSet::Reset() {

	if (Xfers.size() > 0) {
		for_each(Xfers.begin(), Xfers.end(), [](CXfer*& ps) {delete ps;});
		Xfers.clear();
	}
}

bool CTBRXferSet::Parse(CTimDescriptor& TimDescriptor,
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

		stringstream ssXferNum;
		int nXfer = Xfers.size() + 1;
		ssXferNum.str("");
		ssXferNum << nXfer << "_";

		CXfer* pXfer = 0;
		if (pLine->m_FieldName
				== (bIsBlf ?
						ssXferNum.str() + SpaceToUnderscore(CXfer::Begin) :
						CXfer::Begin)) {
			pXfer = new CXfer;
			if (pXfer) {
				string sXferNum = ssXferNum.str(); // this syntax if for linux compiler
				if (!pXfer->Parse(TimDescriptor, pLine, bIsBlf, sXferNum)) {
					stringstream ss;
					ss << endl << "Error: Parsing of '"
							<< (bIsBlf ?
									ssXferNum.str()
											+ SpaceToUnderscore(CXfer::Begin) :
									CXfer::Begin);
					TimDescriptor.ParsingError(ss.str(), true, pLine);
					delete pXfer;
					pXfer = 0;
					return false;
				}
				if (pXfer
						&& pLine->m_FieldName
								== (bIsBlf ?
										ssXferNum.str()
												+ SpaceToUnderscore(
														CXfer::End) :
										CXfer::End)) {
					Xfers.push_back(pXfer);
					pXfer = 0;
					continue;
				}
			}
		}

		stringstream ss;
		ss << endl << "Error: Parsing of '"
				<< (bIsBlf ? ssXferNum.str() + SpaceToUnderscore(End) : End);
		TimDescriptor.ParsingError(ss.str(), true, pLine);
		delete pXfer;
		return false;
	}

//    *m_FieldValues[ NUM_DATA_PAIRS ] = HexFormattedAscii( Xfers.size() );
	if (Translate(*m_FieldValues[NUM_DATA_PAIRS]) != Xfers.size()) {
		stringstream ss;
		ss << endl << "Error: Parsing of '"
				<< (bIsBlf ? SpaceToUnderscore(Begin) : Begin)
				<< ", NUM_DATA_PAIRS not equal to actual number of Xfers defined in the TBR Xfer Set";
		TimDescriptor.ParsingError(ss.str(), true, pLine);
		return false;
	}

	return true;
}

bool CTBRXferSet::ToBinary(ofstream& ofs) {
	// validate size
	if (m_FieldValues.size() != m_iMaxFieldNum)
		return false;

	bool bRet = true;
	ofs << TBR_XFER;
	ofs << PackageSize();

	ofs << Translate(*m_FieldNames[XFER_LOC]);
	ofs << Translate(*m_FieldNames[NUM_DATA_PAIRS]);

	t_XferListIter iter = Xfers.begin();
	while (bRet && iter != Xfers.end())
		bRet = (*iter++)->ToBinary(ofs);

	return (ofs.good() && bRet);
}

int CTBRXferSet::AddPkgStrings(CReservedPackageData* pRPD) {
	pRPD->PackageIdTag(HexFormattedAscii(TBR_XFER));
	pRPD->AddData(new string(*m_FieldValues[XFER_LOC]), new string("XFER_LOC"));
	pRPD->AddData(new string(*m_FieldValues[NUM_DATA_PAIRS]),
			new string("NUM_DATA_PAIRS"));

	t_XferListIter iter = Xfers.begin();
	while (iter != Xfers.end())
		(*iter++)->AddPkgStrings(pRPD);

	return PackageSize();
}

unsigned int CTBRXferSet::PackageSize() {
	return (unsigned int) (8 + // package tag id + size
			(m_FieldValues.size() * 4) // all fields
			+ Xfers.size() * 8);		 // all data in data list
}

