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

#include "OnDemandBootERD.h"
#include "TimDescriptorParser.h"

const string COnDemandBoot::Begin("OnDemand Boot");
const string COnDemandBoot::End("End OnDemand Boot");

COnDemandBoot::COnDemandBoot(bool bIsBlf) :
		CErdBase(ONDEMAND_BOOT_ERD, ONDEMAND_BOOT_MAX) {
	m_bIsBlf = bIsBlf;

	*m_FieldNames[MAGIC_WORD] = "Magic Word";
	*m_FieldNames[MAGIC_WORD_ADDRESS] = "Magic Word Address";
	*m_FieldNames[MAGIC_WORD_PARTITION] = "Magic Word Partition";
	*m_FieldNames[NUM_ODBT_IMAGES] = "Num ODBT Images";
	*m_FieldNames[ODBT_IMAGE_ID] = "ODBT Image ID";
}

COnDemandBoot::~COnDemandBoot() {
	Reset();
}

// copy constructor
COnDemandBoot::COnDemandBoot(const COnDemandBoot& rhs) :
		CErdBase(rhs) {
	// copy constructor
	m_eErdType = rhs.m_eErdType;
	m_iMaxFieldNum = rhs.m_iMaxFieldNum;

	m_bIsBlf = rhs.m_bIsBlf;
	m_bChanged = rhs.m_bChanged;

	COnDemandBoot& nc_rhs = const_cast<COnDemandBoot&>(rhs);

	// need to do a deep copy of lists to avoid dangling references
	for (unsigned int i = 0; i < m_iMaxFieldNum; i++) {
		*m_FieldNames[i] = *rhs.m_FieldNames[i];
		*m_FieldValues[i] = *rhs.m_FieldValues[i];
		*m_FieldComments[i] = *rhs.m_FieldComments[i];
	}

	if (nc_rhs.ODBTImageVec.size() > 0) {
		for_each(nc_rhs.ODBTImageVec.begin(), nc_rhs.ODBTImageVec.end(),
				[this](string*& psRhsImageID)
				{
					string* psImageID = new string(*psRhsImageID);
					ODBTImageVec.push_back(psImageID);
				});
	}
}

// assignment operator
COnDemandBoot& COnDemandBoot::operator=(const COnDemandBoot& rhs) {
	// assignment operator
	if (&rhs != this) {
		CTimLib::operator=(rhs);

		//#if TOOLS_GUI == 1
		Reset();
		//#endif

		m_bIsBlf = rhs.m_bIsBlf;
		m_eErdType = rhs.m_eErdType;
		m_iMaxFieldNum = rhs.m_iMaxFieldNum;
		m_bChanged = rhs.m_bChanged;

		COnDemandBoot& nc_rhs = const_cast<COnDemandBoot&>(rhs);

		// need to do a deep copy of lists to avoid dangling references
		for (unsigned int i = 0; i < m_iMaxFieldNum; i++) {
			*m_FieldNames[i] = *rhs.m_FieldNames[i];
			*m_FieldValues[i] = *rhs.m_FieldValues[i];
			*m_FieldComments[i] = *rhs.m_FieldComments[i];
		}

		if (nc_rhs.ODBTImageVec.size() > 0) {
			for_each(nc_rhs.ODBTImageVec.begin(), nc_rhs.ODBTImageVec.end(),
					[this](string*& psRhsImageID)
					{
						string* psImageID = new string(*psRhsImageID);
						ODBTImageVec.push_back(psImageID);
					});
		}
	}
	return *this;
}

void COnDemandBoot::Reset() {
	if (ODBTImageVec.size() > 0) {
		for_each(ODBTImageVec.begin(), ODBTImageVec.end(),
				[](string*& ps) {delete ps;});
		ODBTImageVec.clear();
	}
}

bool COnDemandBoot::ToBinary(ofstream& ofs) {
	// validate size
	if (m_FieldValues.size() != m_iMaxFieldNum)
		return false;

	ofs << ONDEMANDBOOTTID;
	ofs << PackageSize();

	ofs << Translate(*m_FieldValues[MAGIC_WORD]);
	ofs << Translate(*m_FieldValues[MAGIC_WORD_ADDRESS]);
	ofs << Translate(*m_FieldValues[MAGIC_WORD_PARTITION]);

	*m_FieldValues[NUM_ODBT_IMAGES] = HexFormattedAscii(ODBTImageVec.size());
	ofs << Translate(*m_FieldValues[NUM_ODBT_IMAGES]);

	if (ODBTImageVec.size() > 0) {
		for_each(ODBTImageVec.begin(), ODBTImageVec.end(),
				[&ofs, this](string*& psRhsImageID) {ofs << Translate(*psRhsImageID);});
	}

	return ofs.good();
}

int COnDemandBoot::AddPkgStrings(CReservedPackageData* pRPD) {
	pRPD->PackageIdTag(HexFormattedAscii(ONDEMANDBOOTTID));
	pRPD->AddData(new string(*m_FieldValues[MAGIC_WORD]),
			new string("Magic Word"));
	pRPD->AddData(new string(*m_FieldValues[MAGIC_WORD_ADDRESS]),
			new string("Magic Word Address"));
	pRPD->AddData(new string(*m_FieldValues[MAGIC_WORD_PARTITION]),
			new string("Magic Word Partition"));

	*m_FieldValues[NUM_ODBT_IMAGES] = HexFormattedAscii(ODBTImageVec.size());
	pRPD->AddData(new string(*m_FieldValues[NUM_ODBT_IMAGES]),
			new string("Num ODBT Images"));

	t_stringVectorIter iter = ODBTImageVec.begin();
	if (ODBTImageVec.size() > 0) {
		string* pODBTImage = 0;
		for_each(ODBTImageVec.begin(), ODBTImageVec.end(),
				[&pRPD, &pODBTImage, this](string*& psRhsImageID)
				{
					pODBTImage = new string;
					*pODBTImage = TextToHexFormattedAscii(*psRhsImageID);
					pRPD->AddData(pODBTImage, new string(*psRhsImageID + ("  ODBT Image ID")));
				});
	}

	m_bChanged = true;

	return PackageSize();
}

bool COnDemandBoot::Parse(CTimDescriptor& TimDescriptor,
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
//            if ( TrimWS( pLine->m_FieldName ) != *m_FieldNames[ ODBT_IMAGE_ID ] )
			if (TrimWS(pLine->m_FieldName)
					!= (bIsBlf ?
							SpaceToUnderscore(*m_FieldNames[ODBT_IMAGE_ID]) :
							*m_FieldNames[ODBT_IMAGE_ID])) {
				if (TrimWS(pLine->m_FieldName)
						== (bIsBlf ?
								SpaceToUnderscore(*m_FieldNames[idx]) :
								*m_FieldNames[idx])) {
					*m_FieldValues[idx] = TrimWS(pLine->m_FieldValue);
					bFound = true;
					break;
				}
			} else {
				string* pODBTImage = new string(
						HexAsciiToText(TrimWS(pLine->m_FieldValue)));
				ODBTImageVec.push_back(pODBTImage);
				bFound = true;
				break;
			}
		}

		if (!bFound) {
			stringstream ss;
			ss << endl << "Error: Parsing of '"
					<< (bIsBlf ? SpaceToUnderscore(Begin) : Begin);
			TimDescriptor.ParsingError(ss.str(), true, pLine);
		}
	}

	if (Translate(*m_FieldValues[NUM_ODBT_IMAGES]) != ODBTImageVec.size()) {
		stringstream ss;
		ss << endl
				<< "\nError: Parsing of OnDemandBoot ERD, NUM_ODBT_IMAGES not equal to actual number of ODBT Image IDs defined in the ODBTImageList";
		TimDescriptor.ParsingError(ss.str(), true, pLine);
		return false;
	}

	return true;
}

unsigned int COnDemandBoot::PackageSize() {
	// 8 (WRAH) + 12 (fields) + 4(num ODBTImage) + actual size of ODBTImageVec
	unsigned int iSize = 8 + 12 + 4 + ((unsigned int) ODBTImageVec.size() * 4);
	return iSize;
}
