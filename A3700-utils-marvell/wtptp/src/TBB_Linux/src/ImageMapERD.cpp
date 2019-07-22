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

#include "ImageMapERD.h"
#include "TimDescriptorParser.h"

const string CImageMapInfo::Begin("Image Map Info");
const string CImageMapInfo::End("End Image Map Info");

CImageMapInfo::CImageMapInfo(bool bIsBlf) :
		CErdBase(IMAGE_MAP_INFO_ERD, IMAGE_MAP_MAX) {
	m_bIsBlf = bIsBlf;

	*m_FieldNames[IMAGE_ID] = "Image ID";
	*m_FieldNames[IMAGE_TYPE] = "Image Type";
	*m_FieldNames[FLASH_ADDRESS_LO] = "Flash Address Lo";
	*m_FieldNames[FLASH_ADDRESS_HI] = "Flash Address Hi";
	*m_FieldNames[PARTITION_NUM] = "Partition";
}

CImageMapInfo::~CImageMapInfo() {

}

// copy constructor
CImageMapInfo::CImageMapInfo(const CImageMapInfo& rhs) :
		CErdBase(rhs) {
	// copy constructor
	m_eErdType = rhs.m_eErdType;
	m_iMaxFieldNum = rhs.m_iMaxFieldNum;

	m_bIsBlf = rhs.m_bIsBlf;
	m_bChanged = rhs.m_bChanged;

	// need to do a deep copy of lists to avoid dangling references
	for (unsigned int i = 0; i < m_iMaxFieldNum; i++) {
		*m_FieldNames[i] = *rhs.m_FieldNames[i];
		*m_FieldValues[i] = *rhs.m_FieldValues[i];
		*m_FieldComments[i] = *rhs.m_FieldComments[i];
	}
}

// assignment operator
CImageMapInfo& CImageMapInfo::operator=(const CImageMapInfo& rhs) {
	// assignment operator
	if (&rhs != this) {
		CTimLib::operator=(rhs);
		m_bIsBlf = rhs.m_bIsBlf;
		m_eErdType = rhs.m_eErdType;
		m_iMaxFieldNum = rhs.m_iMaxFieldNum;
		m_bChanged = rhs.m_bChanged;

		// need to do a deep copy of lists to avoid dangling references
		for (unsigned int i = 0; i < m_iMaxFieldNum; i++) {
			*m_FieldNames[i] = *rhs.m_FieldNames[i];
			*m_FieldValues[i] = *rhs.m_FieldValues[i];
			*m_FieldComments[i] = *rhs.m_FieldComments[i];
		}

	}
	return *this;
}

bool CImageMapInfo::ToBinary(ofstream& ofs) {
	// validate size
	if (m_FieldValues.size() != m_iMaxFieldNum)
		return false;

	ofs << Translate(*m_FieldValues[IMAGE_ID]);
	ofs << Translate(*m_FieldValues[IMAGE_TYPE]);
	ofs << Translate(*m_FieldValues[FLASH_ADDRESS_LO]);
	ofs << Translate(*m_FieldValues[FLASH_ADDRESS_HI]);
	ofs << Translate(*m_FieldValues[PARTITION_NUM]);

	return ofs.good();
}

int CImageMapInfo::AddPkgStrings(CReservedPackageData* pRPD) {
	pRPD->AddData(new string(*m_FieldValues[IMAGE_ID]), new string("Image ID"));
	pRPD->AddData(new string(*m_FieldValues[IMAGE_TYPE]),
			new string("Type ID"));
	pRPD->AddData(new string(*m_FieldValues[FLASH_ADDRESS_LO]),
			new string("Flash Address Lo"));
	pRPD->AddData(new string(*m_FieldValues[FLASH_ADDRESS_HI]),
			new string("Flash Address Hi"));
	pRPD->AddData(new string(*m_FieldValues[PARTITION_NUM]),
			new string("Partition"));

	return PackageSize();
}

bool CImageMapInfo::Parse(CTimDescriptor& TimDescriptor,
		CTimDescriptorLine*& pLine, bool bIsBlf, string& sNum) {
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

const string CImageMaps::Begin("Image Maps");
const string CImageMaps::End("End Image Maps");

CImageMaps::CImageMaps(bool bIsBlf) :
		CErdBase(IMAGE_MAPS_ERD, IMAGE_MAPS_MAX) {
	m_bIsBlf = bIsBlf;
	*m_FieldNames[NUM_MAPS] = "NUM_MAPS";
}

CImageMaps::~CImageMaps() {
	Reset();
}

// copy constructor
CImageMaps::CImageMaps(const CImageMaps& rhs) :
		CErdBase(rhs) {
	// need to do a deep copy of lists to avoid dangling references
	CImageMaps& nc_rhs = const_cast<CImageMaps&>(rhs);
	m_bIsBlf = rhs.m_bIsBlf;
	t_ImageMapListIter iter = nc_rhs.ImageMapList.begin();
	while (iter != nc_rhs.ImageMapList.end()) {
		CImageMapInfo* pImageMap = new CImageMapInfo(*(*iter));
		ImageMapList.push_back(pImageMap);
		iter++;
	}
}

// assignment operator
CImageMaps& CImageMaps::operator=(const CImageMaps& rhs) {
	// assignment operator
	if (&rhs != this) {
		CErdBase::operator=(rhs);

		Reset();

		m_bIsBlf = rhs.m_bIsBlf;

		// need to do a deep copy of lists to avoid dangling references
		CImageMaps& nc_rhs = const_cast<CImageMaps&>(rhs);
		t_ImageMapListIter iter = nc_rhs.ImageMapList.begin();
		while (iter != nc_rhs.ImageMapList.end()) {
			CImageMapInfo* pImageMap = new CImageMapInfo(*(*iter));
			ImageMapList.push_back(pImageMap);
			iter++;
		}
	}
	return *this;
}

void CImageMaps::Reset() {
	if (ImageMapList.size() > 0) {
		for_each(ImageMapList.begin(), ImageMapList.end(),
				[](CImageMapInfo*& pIMI) {delete pIMI;});
		ImageMapList.clear();
	}
}

bool CImageMaps::Parse(CTimDescriptor& TimDescriptor,
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

		stringstream ssImageMapNum;
		int nImageMap = ImageMapList.size() + 1;
		ssImageMapNum.str("");
		ssImageMapNum << nImageMap << "_";

		CImageMapInfo* pImageMap = 0;
		if (pLine->m_FieldName
				== (bIsBlf ?
						ssImageMapNum.str()
								+ SpaceToUnderscore(CImageMapInfo::Begin) :
						CImageMapInfo::Begin)) {
			pImageMap = new CImageMapInfo(bIsBlf);
			string sImageMapNum = ssImageMapNum.str(); // this syntax if for linux compiler
			if (pImageMap
					&& !pImageMap->Parse(TimDescriptor, pLine, bIsBlf,
							sImageMapNum)) {
				stringstream ss;
				ss << endl << "Error: Parsing of '"
						<< (bIsBlf ?
								ssImageMapNum.str()
										+ SpaceToUnderscore(
												CImageMapInfo::Begin) :
								CImageMapInfo::Begin);
				TimDescriptor.ParsingError(ss.str(), true, pLine);
				delete pImageMap;
				return false;
			}

			if (pImageMap
					&& pLine->m_FieldName
							== (bIsBlf ?
									ssImageMapNum.str()
											+ SpaceToUnderscore(
													CImageMapInfo::End) :
									CImageMapInfo::End)) {
				ImageMapList.push_back(pImageMap);
				pImageMap = 0;
				continue;
			}
		}

		stringstream ss;
		ss << endl << "Error: Parsing of '"
				<< (bIsBlf ? ssImageMapNum.str() + SpaceToUnderscore(End) : End);
		TimDescriptor.ParsingError(ss.str(), true, pLine);
		delete pImageMap;
		return false;
	}

	if (Translate(*m_FieldValues[NUM_MAPS]) != ImageMapList.size()) {
		stringstream ss;
		ss << endl
				<< "\nError: Parsing of Image Maps, NUM_MAPS not equal to actual number of Image Maps defined in the ImageMaps";
		TimDescriptor.ParsingError(ss.str(), true, pLine);
		return false;
	}

	return true;
}

bool CImageMaps::ToBinary(ofstream& ofs) {
	// validate size
	if (m_FieldValues.size() != m_iMaxFieldNum)
		return false;

	bool bRet = true;
	ofs << IMAPID;
	ofs << PackageSize();
	ofs << Translate(*m_FieldValues[NUM_MAPS]);
	t_ImageMapListIter iter = ImageMapList.begin();
	while (bRet && iter != ImageMapList.end())
		bRet = (*iter++)->ToBinary(ofs);

	return (ofs.good() && bRet);
}

int CImageMaps::AddPkgStrings(CReservedPackageData* pRPD) {
	pRPD->PackageIdTag(HexFormattedAscii(IMAPID));
	pRPD->AddData(new string(*m_FieldValues[NUM_MAPS]), new string("NUM_MAPS"));

	t_ImageMapListIter iter = ImageMapList.begin();
	while (iter != ImageMapList.end())
		(*iter++)->AddPkgStrings(pRPD);

	return PackageSize();
}
