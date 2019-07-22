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



#include "TimDescriptor.h"
#include "TimDescriptorLine.h"
#include "ErdBase.h"
#include "Tim.h"
#include "PinERD.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>


// static object to hold all the TIM.txt parsed lines and object references
t_TimDescriptorLines CTimDescriptor::g_TimDescriptorLines;
t_TimDescriptorLines CTimDescriptor::g_SavedTimDescriptorLines;
t_TimDescriptorLines CTimDescriptor::g_TimDescriptorErrors;
t_TimDescriptorLines CTimDescriptor::g_TimDescriptorWarnings;

unsigned int CTimDescriptor::g_uiTimDescriptors = 0;
CTimDescriptorLine* CTimDescriptor::pPrevLine = 0;

CTimDescriptor::CTimDescriptor(void) :
		CTimLib(), m_ExtendedReservedData(m_sProcessorType) {
	g_uiTimDescriptors++;

	m_BlfHeader.m_pTimDescriptor = this;
	Reset();

	m_bChanged = false;
	m_bImagesChanged = false;
	m_bReservedChanged = false;
	m_bKeysChanged = false;
	m_bNotWritten = false;
	m_TimHeader.setIssueDate(HexFormattedTodayDate());
}

CTimDescriptor::~CTimDescriptor(void) {
	DiscardAll();
	if (--g_uiTimDescriptors == 0) {
		DiscardTimDescriptorLines();
		DiscardSavedTimDescriptorLines();
		DiscardTimDescriptorErrors();
		DiscardTimDescriptorWarnings();
	}
}

// copy constructor
CTimDescriptor::CTimDescriptor(const CTimDescriptor& rhs) :
		CTimLib(rhs), m_BlfHeader(rhs.m_BlfHeader), m_ExtendedReservedData(
				rhs.m_sProcessorType) {
	g_uiTimDescriptors++;

	// copy constructor
	CTimDescriptor& nc_rhs = const_cast<CTimDescriptor&>(rhs);

	m_sTimDescriptorFilePath = rhs.m_sTimDescriptorFilePath;
	m_sProcessorType = rhs.m_sProcessorType;
	m_sTimDescriptor = rhs.m_sTimDescriptor;

	memcpy(&m_TimHeader.getTimHeader(),
			&((CTimDescriptor &) rhs).getTimHeader().getTimHeader(),
			m_TimHeader.getSize());

	// need to do a deep copy of the lists to prevent dangling references
	t_ImagesIter iterImage = nc_rhs.m_Images.begin();
	while (iterImage != nc_rhs.m_Images.end()) {
		CImageDescription* pImage = new CImageDescription(*(*iterImage));
		m_Images.push_back(pImage);
		iterImage++;
	}

	t_ReservedDataListIter iterData = nc_rhs.m_ReservedDataList.begin();
	while (iterData != nc_rhs.m_ReservedDataList.end()) {
		CReservedPackageData* pResData = new CReservedPackageData(*(*iterData));
		m_ReservedDataList.push_back(pResData);
		iterData++;
	}

	m_ExtendedReservedData = nc_rhs.m_ExtendedReservedData;

	t_KeyListIter iterKey = nc_rhs.m_KeyList.begin();
	while (iterKey != nc_rhs.m_KeyList.end()) {
		CKey* pKey = (*iterKey)->newCopy();
		m_KeyList.push_back(pKey);
		iterKey++;
	}

	m_DigitalSignature = nc_rhs.m_DigitalSignature;
	m_DTIMKeysDataBlf = nc_rhs.m_DTIMKeysDataBlf;
	m_bChanged = rhs.m_bChanged;
	m_bImagesChanged = rhs.m_bImagesChanged;
	m_bReservedChanged = rhs.m_bReservedChanged;
	m_bKeysChanged = rhs.m_bKeysChanged;
	m_bNotWritten = rhs.m_bNotWritten;
}

// assignment operator
CTimDescriptor& CTimDescriptor::operator=(const CTimDescriptor& rhs) {
	// assignment operator
	if (&rhs != this) {
		CTimLib::operator=(rhs);
		CTimDescriptor& nc_rhs = const_cast<CTimDescriptor&>(rhs);

		// delete the existing list and recreate a new one
		DiscardAll();

		m_sTimDescriptorFilePath = rhs.m_sTimDescriptorFilePath;
		m_sProcessorType = rhs.m_sProcessorType;
		m_sTimDescriptor = rhs.m_sTimDescriptor;

		memcpy(&m_TimHeader.getTimHeader(),
				&((CTimDescriptor&) rhs).m_TimHeader.getTimHeader(),
				m_TimHeader.getSize());

		// need to do a deep copy of the lists to prevent dangling references
		t_ImagesIter iterImage = nc_rhs.m_Images.begin();
		while (iterImage != nc_rhs.m_Images.end()) {
			CImageDescription* pImage = new CImageDescription(*(*iterImage));
			m_Images.push_back(pImage);
			iterImage++;
		}

		t_ReservedDataListIter iterData = nc_rhs.m_ReservedDataList.begin();
		while (iterData != nc_rhs.m_ReservedDataList.end()) {
			CReservedPackageData* pResData = new CReservedPackageData(
					*(*iterData));
			m_ReservedDataList.push_back(pResData);
			iterData++;
		}

		m_ExtendedReservedData = nc_rhs.m_ExtendedReservedData;

		t_KeyListIter iterKey = nc_rhs.m_KeyList.begin();
		while (iterKey != nc_rhs.m_KeyList.end()) {
			CKey* pKey = (*iterKey)->newCopy();
			m_KeyList.push_back(pKey);
			iterKey++;
		}

		m_DigitalSignature = nc_rhs.m_DigitalSignature;
		m_DTIMKeysDataBlf = nc_rhs.m_DTIMKeysDataBlf;
		m_bChanged = rhs.m_bChanged;
		m_bImagesChanged = rhs.m_bImagesChanged;
		m_bReservedChanged = rhs.m_bReservedChanged;
		m_bKeysChanged = rhs.m_bKeysChanged;
		m_bNotWritten = rhs.m_bNotWritten;
	}

	return *this;
}

bool CTimDescriptor::IsChanged() {
	return (m_bChanged || m_bImagesChanged || m_bReservedChanged
			|| m_bKeysChanged || m_DigitalSignature.IsChanged()
			|| m_DTIMKeysDataBlf.IsChanged()
			|| m_ExtendedReservedData.IsChanged());
}

void CTimDescriptor::Changed(bool bSet) {
	m_bChanged = bSet;
	m_bImagesChanged = bSet;
	m_bReservedChanged = bSet;
	m_bKeysChanged = bSet;
	m_DigitalSignature.Changed(bSet);
	m_DTIMKeysDataBlf.Changed(bSet);

	m_ExtendedReservedData.Changed(bSet);

	t_ImagesIter iterImage = m_Images.begin();
	while (iterImage != m_Images.end()) {
		(*iterImage)->Changed(bSet);
		iterImage++;
	}

	t_ReservedDataListIter iterData = m_ReservedDataList.begin();
	while (iterData != m_ReservedDataList.end()) {
		(*iterData)->Changed(bSet);
		iterData++;
	}

	t_KeyListIter iterKey = m_KeyList.begin();
	while (iterKey != m_KeyList.end()) {
		(*iterKey)->Changed(bSet);
		iterKey++;
	}

	if (bSet)
		m_bNotWritten = true;
}

void CTimDescriptor::Reset() {
	memset(&m_TimHeader.getTimHeader(), 0, m_TimHeader.getSize()); //sizeof (CTIM));

	m_sTimDescriptorFilePath = "";
	m_sTimDescriptor = "";
	m_sProcessorType = "<undefined>";
	m_TimHeader.setVersion(TIMVersion);
	m_TimHeader.setIssueDate(HexFormattedTodayDate());
	m_TimHeader.setTrusted(0);
	m_TimHeader.setOEMUniqueId(0xFEDCBA98);
	m_TimHeader.setBootRomFlashSignature(0xFFFFFFFF); //i.e., 0x4E414E04; 'NAN'04
	if (TIMVersion < TIM_3_6_00) {
		m_TimHeader.setWtmSaveStateFlashSignature(0xFFFFFFFF); //i.e. 0x4E414E04;  'NAN'04
		m_TimHeader.setWtmSaveStateFlashEntryAddress(0x00000000);
		m_TimHeader.setWtmSaveStateBackupEntryAddress(0x00000000);
	} else {
		m_TimHeader.setStepping(0x00000000);
	}
	m_bNotWritten = true;
	m_bChanged = false;
	m_bImagesChanged = false;
	m_bReservedChanged = false;
	m_bKeysChanged = false;
	pPrevLine = 0;
}

void CTimDescriptor::DiscardAll() {
	t_ImagesIter iterImage = m_Images.begin();
	while (iterImage != m_Images.end()) {
		delete *iterImage;
		iterImage++;
	}
	m_Images.clear();

	t_ReservedDataListIter iterData = m_ReservedDataList.begin();
	while (iterData != m_ReservedDataList.end()) {
		delete *iterData;
		iterData++;
	}
	m_ReservedDataList.clear();

	m_ExtendedReservedData.Reset();

	t_KeyListIter iterKey = m_KeyList.begin();
	while (iterKey != m_KeyList.end()) {
		delete *iterKey;
		iterKey++;
	}
	m_KeyList.clear();
	m_DigitalSignature.DiscardAll();
	m_DTIMKeysDataBlf.DiscardAll();
}

bool CTimDescriptor::AddImage(CImageDescription* pImage) {
	string sValue("<None>");
	if (pImage) {
		CImageDescription* pPrevImage = 0;
		if (!m_Images.empty())
			pPrevImage = m_Images.back();

		if (pImage->ImageId().compare(0, 3, "TIM") == 0) {
			t_ImagesIter iterImage = m_Images.begin();
			while (iterImage != m_Images.end()) {
				// see if TIM* image is already in the list, cannot have 2 with the same id
				if ((*iterImage++)->ImageId() == pImage->ImageId())
					return false;
			}

			// if TIMH is already at the top of the list, then add this TIM* to the bottom
			// if TIMH is not already at the top, then add this TIM* to the top
			iterImage = m_Images.begin();
			if (iterImage != m_Images.end()) {
				if ((*iterImage)->ImageId() == "TIMH") {
					m_Images.push_back(pImage);
					// new image is at end of list so there is no next image yet
					pImage->NextImageId(sValue);
					// prevous image needs to be updated
					if (pPrevImage)
						pPrevImage->NextImageId(pImage->ImageId());
				} else if ((*iterImage)->ImageId().compare(0, 3, "TIM") == 0) {
					// if TIM* is at top of list and new image is TIMH, put TIMH at top
					if (pImage->ImageId() == "TIMH") {
						m_Images.push_front(pImage);
						pImage->NextImageId((*iterImage)->ImageId());
					} else
						// if a TIM* is already in the list, cannot add another
						return false;
				} else {
					// no TIM* at top of list so put TIM* there
					m_Images.push_front(pImage);
					pImage->NextImageId((*iterImage)->ImageId());
				}
			} else {
				// list is empty so add to top
				m_Images.push_front(pImage);
				pImage->NextImageId(sValue);
			}
		} else {
			m_Images.push_back(pImage);
			// new image is at end of list so there is no next image yet
			pImage->NextImageId(sValue);
			// prevous image needs to be updated
			if (pPrevImage)
				pPrevImage->NextImageId(pImage->ImageId());
		}

		m_bImagesChanged = true;
		return true;
	}

	return false;
}

bool CTimDescriptor::DeleteImage(CImageDescription* pImage) {
	bool bRet = false;
	if (pImage) {
		t_ImagesIter iter = m_Images.begin();
		CImageDescription* pPrevImage = 0;
		while (iter != m_Images.end()) {
			if (pImage == *iter) {
				if (pPrevImage != 0)
					pPrevImage->NextImageId(pImage->NextImageId());

				m_Images.remove(*iter);
				delete pImage;
				bRet = true;
				break;
			}
			pPrevImage = *iter;
			iter++;
		}
	}
	m_bImagesChanged = true;
	return bRet;
}

bool CTimDescriptor::UpdateNextImageIds() {
	t_ImagesIter iter = m_Images.begin();
	CImageDescription* pImage = 0;
	CImageDescription* pPrevImage = 0;
	string sValue("<None>");
	while (iter != m_Images.end()) {
		// mark last accessed image with no next image id
		(*iter)->NextImageId(sValue);

		// previous image needs to be updated
		if (pPrevImage) {
			pPrevImage->NextImageId((*iter)->ImageId());
		}
		pPrevImage = (*iter);
		iter++;
	}
	m_bImagesChanged = true;
	return true;
}

CImageDescription* CTimDescriptor::Image(int idx) {
	// increment over the first image which is the TIMx
	CImageDescription* pImage = 0;
	t_ImagesIter iter = m_Images.begin();
	while (idx > 0 && iter != m_Images.end()) {
		idx--;
		iter++;
	}
	if (iter != m_Images.end())
		pImage = *iter;

	return pImage;
}

string& CTimDescriptor::ConvertProcessorTypeStr(string& sProcessor) {
	string sNewProcessor = sProcessor;
	// convert obsolete processor types to current processor types
	if (sProcessor == "PXA688")               // mmp2
		sNewProcessor = "ARMADA610";
	else if (sProcessor == "ARMADA168")       // aspen
		sNewProcessor = "ARMAMA16x";
	else if (sProcessor == "MG1")             // tavor mg1
		sNewProcessor = "PXA955";
	else if (sProcessor == "TAVOR_PV_MG2")    // tavor mg2
		sNewProcessor = "PXA968";
	else if (sProcessor == "CP888")           // eshel
		sNewProcessor = "PXA1701";
	else if (sProcessor == "ESHEL")           // eshel 
		sNewProcessor = "PXA1701";
	else if (sProcessor == "ARMADA620")       // mmp3
		sNewProcessor = "PXA2128";
	else if (sProcessor == "PXA212x")       // mmp3
		sNewProcessor = "PXA2128";
	else if (sProcessor == "ESHEL_LTE")       // eshel lte
		sNewProcessor = "PXA1801";              // eshel2
	else if (sProcessor == "PXA1702")         // eshel lte
		sNewProcessor = "PXA1801";              // eshel2
	else if (sProcessor == "MMP2X")           // mmp2x
		sNewProcessor = "ARMADA622";
	else if (sProcessor == "NEZHA")           // nezha
		sNewProcessor = "PXA1802";
	else if (sProcessor == "ESHEL2")          // eshel2
		sNewProcessor = "PXA1801";
	else if (sProcessor == "PXA928")          // EMEI
		sNewProcessor = "PXA988";
	else if (sProcessor == "PXA1988")			// Qilin
		sNewProcessor = "<undefined>";
	else if (sProcessor == "PXA1926")			// Eden
		sNewProcessor = "PXA1928";
	else if (sProcessor == "ESHEL3")          // eshel3
		sNewProcessor = "PXA1812";
	else if (sProcessor == "PXA1978")         // old ADIR
		sNewProcessor = "PXA1986";              // new ADIR
	else if (sProcessor == "PXA1980")         // old ADIR
		sNewProcessor = "PXA1986";              // new ADIR

	if (sNewProcessor != sProcessor) {
		string sMsg = "Processor type changed from <";
		sMsg += sProcessor;
		sMsg += "> to <";
		sMsg += sNewProcessor + ">\n";
		printf("%s", sMsg.c_str());
		sProcessor = sNewProcessor;
	}

	return sProcessor;
}

bool CTimDescriptor::ProcessorTypeStr(string& sProcessor) {
	ConvertProcessorTypeStr(sProcessor);
	m_sProcessorType = sProcessor;

	return ExtendedReservedData().ProcessorTypeStr(m_sProcessorType);
}

eProcessorType CTimDescriptor::ProcessorType() {
	for (int i = 0; i < PXAMAX_PT; i++) {
		if (gsProcessorType[i] == m_sProcessorType) {
			return (eProcessorType) i;
		}
	}

	return UNDEFINED;
}

int CTimDescriptor::ReservedDataTotalSize() {
	t_ReservedDataListIter iter = m_ReservedDataList.begin();
	int size = 0;
	while (iter != m_ReservedDataList.end())
		size += (*iter++)->Size();
	if (size > 0)
		size += 16;  // add OPTH header and TERM package sizes
	return size;
}

unsigned int CTimDescriptor::GetTimImageSize(bool bOneNANDPadding /* = false */,
		unsigned int uiPaddedSize /* = 0 */) {
	unsigned int iTimImageSize = 0;

	iTimImageSize = m_TimHeader.getSize();

	if (m_TimHeader.getVersion() < TIM_3_2_00)
		iTimImageSize +=
				(sizeof(IMAGE_INFO_3_1_0) * m_TimHeader.getNumImages());
	else if (m_TimHeader.getVersion() < TIM_3_3_00)
		iTimImageSize +=
				(sizeof(IMAGE_INFO_3_2_0) * m_TimHeader.getNumImages());
	else if (m_TimHeader.getVersion() == TIM_3_4_00)
		iTimImageSize +=
				(sizeof(IMAGE_INFO_3_4_0) * m_TimHeader.getNumImages());
	else if (m_TimHeader.getVersion() >= TIM_3_5_00)
		iTimImageSize +=
				(sizeof(IMAGE_INFO_3_5_0) * m_TimHeader.getNumImages());

	iTimImageSize += m_TimHeader.getSizeOfReserved();

	if (m_TimHeader.getTrusted()) {
		if (m_TimHeader.getVersion() < TIM_3_2_00)
			iTimImageSize += (sizeof(KEY_MOD_3_1_0) * m_TimHeader.getNumKeys());
		else if (m_TimHeader.getVersion() < TIM_3_3_00)
			iTimImageSize += (sizeof(KEY_MOD_3_2_0) * m_TimHeader.getNumKeys());
		else if (m_TimHeader.getVersion() < TIM_3_4_00)
			iTimImageSize += (sizeof(KEY_MOD_3_3_0) * m_TimHeader.getNumKeys());
		else
			iTimImageSize += (sizeof(KEY_MOD_3_4_0) * m_TimHeader.getNumKeys());

		iTimImageSize += sizeof(PLAT_DS);
	}

	if (bOneNANDPadding) {
		if (iTimImageSize < uiPaddedSize)
			iTimImageSize = uiPaddedSize;
	}

	return iTimImageSize;
}

bool CTimDescriptor::InjectPin(string& sPinFilePath) {
	bool bRet = true;

	ifstream ifsPinTxtFile;
	ifsPinTxtFile.open(sPinFilePath.c_str(), ios_base::in);
	if (ifsPinTxtFile.bad() || ifsPinTxtFile.fail()) {
		printf("\n  Error: Cannot open file name <%s> !\n",
				sPinFilePath.c_str());
		bRet = false;
	}

	string sLine;
	string PinA;
	string PinB;

	if (GetNextLine(ifsPinTxtFile, sLine)) {
		PinA = HexFormattedAscii(Translate(sLine));
		if (GetNextLine(ifsPinTxtFile, sLine)) {
			PinB = HexFormattedAscii(Translate(sLine));
		} else
			bRet = false;
	} else
		bRet = false;

	int PinCount = 0;
	if (bRet) {
		// remove any PINs in Reserved Data so we can use the one from PIN.txt
		t_ReservedDataListIter Iter = m_ReservedDataList.begin();
		while (Iter != m_ReservedDataList.end()) {
			if ((*Iter)->PackageId() == "PINP") {
				m_TimHeader.setSizeOfReserved(
						m_TimHeader.getSizeOfReserved() - (*Iter)->Size());

				CImageDescription* pImage = Image(0);
				if (pImage != 0) {
					if (pImage->ImageSizeToHash()
							> (m_TimHeader.getSize()
									+ m_TimHeader.getSizeOfReserved())) {
						pImage->ImageSizeToHash(
								pImage->ImageSizeToHash() - (*Iter)->Size());
					}

					pImage->ImageSize(pImage->ImageSize() - (*Iter)->Size());
				}
				m_ReservedDataList.remove(*Iter);

				// a PINP is reing removed, restart the search for any more
				Iter = m_ReservedDataList.begin();
				PinCount++;
				continue;
			}
			Iter++;
		}

		// remove any PINs in Extended Reserved Data so we can use the one from PIN.txt
		t_ErdBaseVectorIter ERDIter = m_ExtendedReservedData.ErdVec.begin();
		while (ERDIter != m_ExtendedReservedData.ErdVec.end()) {
			if ((*ERDIter)->ErdPkgType() == CErdBase::PIN_ERD) {
				t_ErdBaseVectorIter remIter = ERDIter;
				ERDIter++;
				m_TimHeader.setSizeOfReserved(
						m_TimHeader.getSizeOfReserved()
								- (*remIter)->PackageSize());

				CImageDescription* pImage = Image(0);
				if (pImage) {
					if (pImage->ImageSizeToHash()
							> (m_TimHeader.getSize()
									+ m_TimHeader.getSizeOfReserved())) {
						pImage->ImageSizeToHash(
								pImage->ImageSizeToHash()
										- (*remIter)->PackageSize());
					}

					pImage->ImageSize(
							pImage->ImageSize() - (*remIter)->PackageSize());
				}
				m_ExtendedReservedData.ErdVec.remove(*remIter);
				PinCount++;
				continue;
			}
			ERDIter++;
		}

		// if more than 1 PIN in reserved and Extended Reserved, this fix TIM.txt first
		if (PinCount != 0) {
			printf(
					"\nWarning: PIN in <%s> overrides a PIN in reserved data in <%s>.\n",
					sPinFilePath.c_str(), m_sTimDescriptorFilePath.c_str());
		}
	}

	// inject the PIN from PIN.txt into the reserved data area
	if (bRet) {
		CPin Pin;
		*Pin.m_FieldValues[CPin::PINA_FIELD] = PinA;
		*Pin.m_FieldValues[CPin::PINB_FIELD] = PinB;
		CReservedPackageData* pRPD = new CReservedPackageData;
		Pin.AddPkgStrings(pRPD);
		m_ReservedDataList.push_back(pRPD);
		m_TimHeader.setSizeOfReserved(
				m_TimHeader.getSizeOfReserved() + pRPD->Size());

		CImageDescription* pImage = Image(0);
		if (pImage) {
			if (pImage->ImageSizeToHash()
					> (m_TimHeader.getSize() + m_TimHeader.getSizeOfReserved())) {
				pImage->ImageSizeToHash(
						pImage->ImageSizeToHash() + pRPD->Size());
			}

			pImage->ImageSize(pImage->ImageSize() + pRPD->Size());
		}
	}

	return bRet;
}

void CTimDescriptor::SaveTimDescriptorLines() {
	DiscardSavedTimDescriptorLines();

	t_TimDescriptorLinesIter Iter = g_TimDescriptorLines.begin();
	while (Iter != g_TimDescriptorLines.end()) {
		g_SavedTimDescriptorLines.push_back(new CTimDescriptorLine(*(*Iter)));
		Iter++;
	}
	return;
}

int CTimDescriptor::ParseErrors() {
	int iErrorCount = 0;
	t_TimDescriptorLinesIter Iter = g_TimDescriptorErrors.begin();
	while (Iter != g_TimDescriptorErrors.end()) {
		if ((*Iter)->ParsingError())
			iErrorCount++;
		Iter++;
	}
	return iErrorCount;
}

int CTimDescriptor::ParseWarnings() {
	int iWarningCount = 0;
	t_TimDescriptorLinesIter Iter = g_TimDescriptorWarnings.begin();
	while (Iter != g_TimDescriptorWarnings.end()) {
		if ((*Iter)->ParsingWarning())
			iWarningCount++;
		Iter++;
	}
	return iWarningCount;
}

void CTimDescriptor::RestoreTimDescriptorLines() {
	// restore only if we previously saved a tim
	if (g_SavedTimDescriptorLines.size() > 0) {
		DiscardTimDescriptorLines();

		t_TimDescriptorLinesIter Iter = g_SavedTimDescriptorLines.begin();
		while (Iter != g_SavedTimDescriptorLines.end()) {
			g_TimDescriptorLines.push_back(new CTimDescriptorLine(*(*Iter)));
			Iter++;
		}
	}
	return;
}

void CTimDescriptor::DiscardTimDescriptorLines() {
	if (g_TimDescriptorLines.size() > 0) {
		for_each(g_TimDescriptorLines.begin(), g_TimDescriptorLines.end(),
				[](CTimDescriptorLine*& ps) {delete ps;});
		g_TimDescriptorLines.clear();
	}
}

void CTimDescriptor::DiscardSavedTimDescriptorLines() {
	if (g_SavedTimDescriptorLines.size() > 0) {
		for_each(g_SavedTimDescriptorLines.begin(),
				g_SavedTimDescriptorLines.end(),
				[](CTimDescriptorLine*& ps) {delete ps;});
		g_SavedTimDescriptorLines.clear();
	}
}

void CTimDescriptor::DiscardTimDescriptorErrors() {
	if (g_TimDescriptorErrors.size() > 0) {
		for_each(g_TimDescriptorErrors.begin(), g_TimDescriptorErrors.end(),
				[](CTimDescriptorLine*& ps) {delete ps;});
		g_TimDescriptorErrors.clear();
	}
}

void CTimDescriptor::DiscardTimDescriptorWarnings() {
	if (g_TimDescriptorWarnings.size() > 0) {
		for_each(g_TimDescriptorWarnings.begin(), g_TimDescriptorWarnings.end(),
				[](CTimDescriptorLine*& ps) {delete ps;});
		g_TimDescriptorWarnings.clear();
	}
}

CTimDescriptorLine* CTimDescriptor::IsFieldPresent(string sFieldName,
		CTimDescriptorLine* pPrev, int WithinNLines) {
	if (g_TimDescriptorLines.size() > 0) {
		t_TimDescriptorLinesIter Iter = g_TimDescriptorLines.begin();
		int nLines = 0;
		while (Iter != g_TimDescriptorLines.end()) {
			if (WithinNLines != -1 && nLines >= WithinNLines)
				break;

			// find the starting point to search
			if ((*Iter++) == pPrev || pPrev == 0) {
				// now find the next field with sFieldName
				while (Iter != g_TimDescriptorLines.end()) {
					if (WithinNLines != -1 && nLines >= WithinNLines)
						break;

					if ((*Iter)->m_FieldName == sFieldName) {
						pPrevLine = (*Iter);
						return (*Iter);
					}

					// found a field following a list of hex value fields, as after a key
					if (sFieldName.empty() && !((*Iter)->m_FieldName.empty())) {
						pPrevLine = (*Iter);
						return (*Iter);
					}

					nLines++;
					Iter++;
				}
			}
		}
	}
	return NULL;
}

CTimDescriptorLine* CTimDescriptor::GetNextLineField(string sFieldName,
		CTimDescriptorLine* pPrev, bool bNotFoundMsg, int WithinNLines) {
	if (g_TimDescriptorLines.size() > 0) {
		t_TimDescriptorLinesIter Iter = g_TimDescriptorLines.begin();
		int nLines = 0;
		while (Iter != g_TimDescriptorLines.end()) {
			if (WithinNLines != -1 && nLines >= WithinNLines)
				break;

			// find the starting point to search
			if ((*Iter++) == pPrev || pPrev == 0) {
				// now find the next field with sFieldName
				while (Iter != g_TimDescriptorLines.end()) {
					if (WithinNLines != -1 && nLines >= WithinNLines)
						break;

					if ((*Iter)->m_FieldName == sFieldName) {
						pPrevLine = (*Iter);
						return (*Iter);
					}

					// found a field following a list of hex value fields, as after a key
					if (sFieldName.empty() && !((*Iter)->m_FieldName.empty())) {
						pPrevLine = (*Iter);
						return (*Iter);
					}

					nLines++;
					Iter++;
				}
			}
		}

		if (bNotFoundMsg) {
			stringstream ss;
			ss << endl
					<< "\n  Error: TIM Descriptor file parsing failed to find field <";
			ss << sFieldName.c_str() << ">";

			if (pPrevLine == 0)
				ss << "unknown>";
			ParsingError(ss.str(), true, pPrevLine);
		}
	}
	return 0;
}

CTimDescriptorLine* CTimDescriptor::GetNextLineField(CTimDescriptorLine* pPrev,
		bool bNotFoundMsg) {
	if (g_TimDescriptorLines.size() > 0) {
		t_TimDescriptorLinesIter Iter = g_TimDescriptorLines.begin();
		while (Iter != g_TimDescriptorLines.end()) {
			// find the starting point to search
			if ((*Iter++) == pPrev || pPrev == 0) {
				if (Iter != g_TimDescriptorLines.end()) {
					pPrevLine = (*Iter);
					return (*Iter);
				}
			}
		}

		if (bNotFoundMsg) {
			stringstream ss;
			ss << endl
					<< "\n  Error: TIM Descriptor file parsing failed to find field <";
			if (pPrevLine != 0)
				ss << pPrevLine->m_FieldName.c_str() << ">";
			else
				ss << "unknown>";
			ParsingError(ss.str(), true, pPrevLine);
		}
	}
	return 0;
}

CTimDescriptorLine* CTimDescriptor::GetLineField(string sFieldName,
		bool bNotFoundMsg, void* pObject) {
	if (g_TimDescriptorLines.size() > 0) {
		// first try based on having a field name or not
		t_TimDescriptorLinesIter Iter = g_TimDescriptorLines.begin();
		while (Iter != g_TimDescriptorLines.end()) {
			// match fieldname (or lack of fieldname) plus object pointer
			if (((*Iter)->m_FieldName == sFieldName)
					&& (pObject != 0 && (*Iter)->IsObject(pObject))) {
				pPrevLine = (*Iter);
				return (*Iter);
			}

			// no fieldname but use object pointer
			if (sFieldName.length() == 0
					&& (pObject != 0 && (*Iter)->IsObject(pObject))) {
				pPrevLine = (*Iter);
				return (*Iter);
			}

			Iter++;
		}

		// second try based on having a object or not
		Iter = g_TimDescriptorLines.begin();
		while (Iter != g_TimDescriptorLines.end()) {
			// match based solely on provided object pointer
			if ((pObject != 0 && (*Iter)->IsObject(pObject))) {
				pPrevLine = (*Iter);
				return (*Iter);
			}

			// match fieldname only, if object pointer not provided
			if (pObject == 0 && (*Iter)->m_FieldName == sFieldName) {
				pPrevLine = (*Iter);
				return (*Iter);
			}

			Iter++;
		}

		if (bNotFoundMsg) {
			stringstream ss;
			ss << endl
					<< "\n  Error: TIM Descriptor file parsing failed to find field <";
			ss << sFieldName.c_str() << ">";

			if (pPrevLine == 0)
				ss << "(unknown)";
			ParsingError(ss.str(), true, pPrevLine);
		}
	}
	return 0;
}

bool CTimDescriptor::TimParsingError() {
	if (g_TimDescriptorLines.size() > 0) {
		t_TimDescriptorLinesIter Iter = g_TimDescriptorLines.begin();
		while (Iter != g_TimDescriptorLines.end()) {
			// if any line has an error return true
			if ((*Iter)->ParsingError())
				return true;

			Iter++;
		}
	}
	return false;
}

void CTimDescriptor::ParsingError(string sErr, bool bShow,
		CTimDescriptorLine* pInLine) {
	static CTimDescriptorLine* pLastErrorLine = 0;

	if (pInLine != 0)
		pPrevLine = pInLine;

	if (pLastErrorLine != 0) {
		// prevent multiple errors resulting from 1 error leading to a sequence of errors
		if (pLastErrorLine == pInLine && g_TimDescriptorErrors.size() > 0)
			return;
	}
	pLastErrorLine = pInLine;

	if (pInLine) {
		stringstream ss;
		if (pInLine->m_FieldName.size() > 0)
			ss << sErr << ", failed near line: '" << pInLine->m_FieldName << "'"
					<< endl;
		else if (pInLine->m_FieldValue.size() > 0)
			ss << sErr << ", failed near line: '" << pInLine->m_FieldValue
					<< "'" << endl;
		else
			ss << sErr << endl;

		sErr = ss.str();
	}

	if (bShow) {
		printf("%s", sErr.c_str());
	}

	CTimDescriptorLine* pLine = 0;
	if (pInLine)
		pLine = new CTimDescriptorLine(*pInLine);
	else if (pPrevLine)
		pLine = new CTimDescriptorLine(*pPrevLine);
	else
		pLine = new CTimDescriptorLine;

	pLine->ParsingError(TrimWS(sErr));
	g_TimDescriptorErrors.push_back(pLine);
}

void CTimDescriptor::ParsingWarning(string sWarn, bool bShow,
		CTimDescriptorLine* pInLine) {
	static CTimDescriptorLine* pLastWarningLine = 0;

	if (pInLine != 0)
		pPrevLine = pInLine;

	if (pLastWarningLine != 0) {
		// prevent multiple errors resulting from 1 error leading to a sequence of errors
		if (pLastWarningLine == pInLine && g_TimDescriptorWarnings.size() > 0)
			return;
	}
	pLastWarningLine = pInLine;

	if (bShow) {
		printf("%s", sWarn.c_str());
	}

	CTimDescriptorLine* pLine = 0;
	if (pInLine)
		pLine = new CTimDescriptorLine(*pInLine);
	else if (pPrevLine)
		pLine = new CTimDescriptorLine(*pPrevLine);
	else
		pLine = new CTimDescriptorLine;

	pLine->ParsingWarning(CTimLib::TrimWS(sWarn));
	g_TimDescriptorWarnings.push_back(pLine);
}
