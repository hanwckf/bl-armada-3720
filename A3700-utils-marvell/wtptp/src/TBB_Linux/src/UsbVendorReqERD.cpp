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

#include "UsbVendorReqERD.h"
#include "TimDescriptorParser.h"

const string CUsbVendorReq::Begin("Usb Vendor Request");
const string CUsbVendorReq::End("End Usb Vendor Request");

CUsbVendorReq::CUsbVendorReq() :
		CErdBase(USBVENDORREQ_ERD, USB_VENDOR_REQ_MAX) {
	*m_FieldNames[REQUEST_TYPE] = "REQUEST_TYPE";
	*m_FieldNames[REQUEST] = "REQUEST";
	*m_FieldNames[VALUE] = "VALUE";
	*m_FieldNames[INDEX] = "INDEX";
	*m_FieldNames[LENGTH] = "LENGTH";
	*m_FieldNames[DATA] = "DATA";
}

CUsbVendorReq::~CUsbVendorReq() {
	Reset();
}

// copy constructor
CUsbVendorReq::CUsbVendorReq(const CUsbVendorReq& rhs) :
		CErdBase(rhs) {
	// need to do a deep copy of lists to avoid dangling references
	CUsbVendorReq& nc_rhs = const_cast<CUsbVendorReq&>(rhs);

	t_stringVectorIter iter = nc_rhs.m_DataList.begin();
	while (iter != nc_rhs.m_DataList.end()) {
		string* psData = new string(*(*iter));
		m_DataList.push_back(psData);
		iter++;
	}
}

// assignment operator
CUsbVendorReq& CUsbVendorReq::operator=(const CUsbVendorReq& rhs) {
	// assignment operator
	if (&rhs != this) {
		CErdBase::operator=(rhs);

		Reset();

		// need to do a deep copy of lists to avoid dangling references
		CUsbVendorReq& nc_rhs = const_cast<CUsbVendorReq&>(rhs);
		t_stringVectorIter iter = nc_rhs.m_DataList.begin();
		while (iter != nc_rhs.m_DataList.end()) {
			string* psData = new string(*(*iter));
			m_DataList.push_back(psData);
			iter++;
		}
	}
	return *this;
}

void CUsbVendorReq::Reset() {
	if (m_DataList.size() > 0) {
		for_each(m_DataList.begin(), m_DataList.end(),
				[](string*& ps) {delete ps;});
		m_DataList.clear();
	}

	if (m_DataFieldNames.size() > 0) {
		for_each(m_DataFieldNames.begin(), m_DataFieldNames.end(),
				[](string*& ps) {delete ps;});
		m_DataFieldNames.clear();
	}
}

bool CUsbVendorReq::ToBinary(ofstream& ofs) {
	// validate size
	if (m_FieldValues.size() != m_iMaxFieldNum)
		return false;

	ofs << USBVENDORREQ;
	ofs << PackageSize();
	unsigned int nField = (Translate(*m_FieldValues[VALUE]) & 0xFFFF) << 16;
	nField |= (Translate(*m_FieldValues[REQUEST]) & 0xFF) << 8;
	nField |= (Translate(*m_FieldValues[REQUEST_TYPE]) & 0xFF);
	ofs << nField;

	nField = (Translate(*m_FieldValues[LENGTH]) & 0xFFFF) << 16;
	nField |= (Translate(*m_FieldValues[INDEX]) & 0xFFFF);
	ofs << nField;

	t_stringVectorIter iter = m_DataList.begin();
	while (iter != m_DataList.end())
		ofs << Translate(*(*iter++));

	return ofs.good();
}

int CUsbVendorReq::AddPkgStrings(CReservedPackageData* pRPD) {
	pRPD->PackageIdTag(HexFormattedAscii(USBVENDORREQ));

	unsigned int nField = (Translate(*m_FieldValues[VALUE]) & 0xFFFF) << 16;
	nField |= (Translate(*m_FieldValues[REQUEST]) & 0xFF) << 8;
	nField |= (Translate(*m_FieldValues[REQUEST_TYPE]) & 0xFF);

	pRPD->AddData(new string(HexFormattedAscii(nField)),
			new string("wVALUE, bREQUEST, bREQUEST_TYPE"));

	nField = (Translate(*m_FieldValues[LENGTH]) & 0xFFFF) << 16;
	nField |= (Translate(*m_FieldValues[INDEX]) & 0xFFFF);

	pRPD->AddData(new string(HexFormattedAscii(nField)),
			new string("wLENGTH, wINDEX"));

	t_stringVectorIter iter = m_DataList.begin();
	while (iter != m_DataList.end()) {
		pRPD->AddData(new string(*(*iter)), new string("data"));
		iter++;
	}

	return PackageSize();
}

bool CUsbVendorReq::Parse(CTimDescriptor& TimDescriptor,
		CTimDescriptorLine*& pLine, bool bIsBlf) {

	m_bChanged = false;

	if (pLine->m_FieldName != (bIsBlf ? SpaceToUnderscore(Begin) : Begin)) {
		stringstream ss;
		ss << endl << "Error: Parsing of '"
				<< (bIsBlf ? SpaceToUnderscore(Begin) : Begin);
		TimDescriptor.ParsingError(ss.str(), true, pLine);
		return false;
	}

	string sValue;
	string sData;
	size_t nPos = 0;
	size_t nBeg = 0;
	size_t nEnd = 0;

	while ((pLine = TimDescriptor.GetNextLineField(pLine))) {
		if (pLine->m_FieldName == (bIsBlf ? SpaceToUnderscore(End) : End)) {
			break;
		}

		bool bFound = false;
		for (unsigned int idx = 0; idx < m_FieldNames.size(); idx++) {
			if (pLine->m_FieldName == *m_FieldNames[idx]) {
				if (idx == DATA) {
					*m_FieldValues[idx] = pLine->m_FieldValue;
					sValue = pLine->m_FieldValue;

					nBeg = 0;
					nEnd = 0;
					// parse one or more values per line, separated by white space
					do {
						if (string::npos
								== (nEnd = sValue.find_first_of(": \n\t", nBeg)))
							sData = sValue.substr(nBeg);
						else
							sData = sValue.substr(nBeg, nEnd - nBeg);

						if (sData.length() == 0)
							// skip white space until next data
							nBeg++;
						else {
							m_DataList.push_back(new string(sData));
							nBeg += (nEnd - nBeg) + 1;
						}
					} while (nEnd != string::npos);
				} else {
					*m_FieldValues[idx] = pLine->m_FieldValue;
				}

				bFound = true;
				break;
			}
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

unsigned int CUsbVendorReq::PackageSize() {
	return (unsigned int) (8 + // package tag id + size
			8 + // all fields packed except DATA
			m_DataList.size() * 4);		// all data in data list
}

t_stringVector& CUsbVendorReq::DataFieldNames() {
	if (m_DataFieldNames.size() != m_DataList.size()) {
		if (m_DataFieldNames.size() > 0) {
			for_each(m_DataFieldNames.begin(), m_DataFieldNames.end(),
					[](string*& ps) {delete ps;});
			m_DataFieldNames.clear();
		}

		t_stringVectorIter iter = m_DataList.begin();
		int idx = 0;
		while (iter != m_DataList.end()) {
			stringstream ss;
			ss << "DATA[" << idx << "]";
			m_DataFieldNames.push_back(new string(ss.str()));
			iter++;
			idx++;
		}
	}

	return m_DataFieldNames;
}

