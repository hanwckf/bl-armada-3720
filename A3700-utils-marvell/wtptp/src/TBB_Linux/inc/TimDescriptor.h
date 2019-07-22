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


#ifndef __TIM_DESCRIPTOR_H__
#define __TIM_DESCRIPTOR_H__

#include "ReservedPackageData.h"
#include "TimLib.h"
#include "ImageDescription.h"
#include "TimDescriptorLine.h"
#include "ExtendedReservedData.h"
#include "BlfHeader.h"
#include "CTIM.h"
#include "Key.h"
#include "DigitalSignature.h"

enum eProcessorType {
	PXA_NONE = -1,        // UNKNOWN
	PXA168 = 0,           // ASPEN A0
	PXA30x = 1,           // MH-L
	PXA31x = 2,           // MH-LV
	PXA32x = 3,           // MH-P
	ARMADA610 = 4,        // MMP2             *
	PXA91x = 5,           // TTC
	PXA92x = 6,           // TAVOR TD         * (920)
	PXA93x = 7,           // TAVOR P/PV
	PXA94x = 8,           // TAVOR PV (RIM ONLY)
	PXA95x = 9,           // TAVOR PV2
	ARMADA16x = 10,       // Aspen family
	reserved1 = 11,
	PXA955 = 12,          // TAVOR MG1
	PXA968 = 13,          // TAVOR MG2        *
	PXA1701 = 14,         // CP888/ESHEL      *
	PXA978 = 15,          // NEVO             *
	PXA2128 = 16,         // MMP3 Family      * (2128)
	reserved2 = 17,
	ARMADA622 = 18,       // MMP2X
	PXA1202 = 19,         // WUKONG           *
	PXA1801 = 20,         // ESHEL2           * 
	P88PA62_70 = 21,      // GRANITE2          *
	PXA988 = 22,          // EMEI             *
	PXA1920 = 23,         // HELAN LTE        *
	PXA2101 = 24,         // BIRCH-40
	PXA192 = 25,          // BIRCH-28
	PXA1928 = 26,         // EDEN             *
	PXA1986 = 27,         // ADIR             *
	PXA1802 = 28,         // NEZHA            *
	PXA986 = 29,          // KUNLUN           *
	PXA1206 = 30,         // WUJIN            *
	PXA888 = 31,          // WUTAI            *
	PXA1088 = 32,         // HELAN            *
	PXA1812 = 33,         // ESHEL3           *
	PXA1822 = 34,			// NEZHA2
	PXA1U88 = 35,			// HELAN2/3
	PXA1936 = 36,
	PXA1908 = 37,         // ULC1
	PXA1826 = 38,         // NEZHA3
	P88PA62_20 = 39,      // GEMSTONE2          *
	P88PA62_10 = 40,      // BASALT2          *
	PXA1956 = 41,			// HELAN4
	VEGA = 42,
	BOWIE = 43,
	ULC2 = 44,
	// put all defined processor type above undefined
	// and adj undefined value to acount for new processors
	UNDEFINED = 45,
	PXAMAX_PT
};

const string gsProcessorType[PXAMAX_PT] = { "PXA168", "PXA30x", "PXA31x",
		"PXA32x", "ARMADA610", "PXA91x", "PXA92x", "PXA93x", "PXA94x", "PXA95x",
		"ARMADA16x", "reserved1", "PXA955", "PXA968", "PXA1701", "PXA978",
		"PXA2128", "reserved2", "ARMADA622", "PXA1202", "PXA1801", "88PA62_70",
		"PXA988", "PXA1920", "PXA2101", "PXA192", "PXA1928", "PXA1986",
		"PXA1802", "PXA986", "PXA1206", "PXA888", "PXA1088", "PXA1812",
		"PXA1822", "PXA1U88", "PXA1936", "PXA1908", "PXA1826", "88PA62_20",
		"88PA62_10", "PXA1956", "VEGA", "BOWIE", "ULC2",

		// put all defined processor type above undefined
		"<undefined>", };

const int TIMVersion = 0x00030600;  // current default

class CTimDescriptor: public CTimLib {
public:
	CTimDescriptor(void);
	virtual ~CTimDescriptor(void);

	// copy constructor
	CTimDescriptor(const CTimDescriptor& rhs);
	// assignment operator
	CTimDescriptor& operator=(const CTimDescriptor& rhs);

	void TimDescriptorFilePath(string& sFilePath) {
		m_sTimDescriptorFilePath = sFilePath;
		m_bNotWritten = m_bChanged = true;
	}
	string& TimDescriptorFilePath() {
		return m_sTimDescriptorFilePath;
	}

	void Version(string& sVersion) {
		m_TimHeader.setVersionStr(sVersion);
		m_bNotWritten = m_bChanged = true;
	}
	void Trusted(bool bSet) {
		m_TimHeader.setTrusted(bSet);
		m_bNotWritten = m_bChanged = true;
	}
	void IssueDate(string& sIssueDate) {
		m_TimHeader.setIssueDateStr(sIssueDate);
		m_bNotWritten = m_bChanged = true;
	}
	void OemUniqueId(string& sOemUniqueId) {
		m_TimHeader.setOEMUniqueIdStr(sOemUniqueId);
		m_bNotWritten = m_bChanged = true;
	}
	void BootRomFlashSignature(string& sBootRomFlashSignature) {
		m_TimHeader.setBootRomFlashSignatureStr(sBootRomFlashSignature);
	}
	void Stepping(string& sStepping) {
		m_TimHeader.setSteppingStr(sStepping);
	}
	void WtmSaveStateFlashSignature(string& sSignature) {
		m_TimHeader.setWtmSaveStateFlashSignatureStr(sSignature);
		m_bNotWritten = m_bChanged = true;
	}
	void WtmSaveStateFlashEntryAddress(string& sAddress) {
		m_TimHeader.setWtmSaveStateFlashEntryAddressStr(sAddress);
		m_bNotWritten = m_bChanged = true;
	}
	void WtmSaveStateBackupEntryAddress(string& sAddress) {
		m_TimHeader.setWtmSaveStateBackupEntryAddressStr(sAddress);
		m_bNotWritten = m_bChanged = true;
	}

	CImageDescription* Image(int idx);
	bool AddImage(CImageDescription* pImage);
	bool DeleteImage(CImageDescription* pImage);
	size_t ImagesCount() {
		return m_Images.size();
	}

	bool KeysIsChanged() {
		return m_bKeysChanged;
	}
	void KeysChanged(bool bSet) {
		m_bKeysChanged = bSet;
	}

	CCTIM& getTimHeader() {
		return m_TimHeader;
	}

	bool ProcessorTypeStr(string& sProcessor);
	string& ProcessorTypeStr() {
		return m_sProcessorType;
	}
	string& ConvertProcessorTypeStr(string& sProcessor);

	t_ImagesList& ImagesList() {
		return m_Images;
	}
	bool UpdateNextImageIds();

	t_ReservedDataList& ReservedDataList() {
		return m_ReservedDataList;
	}
	int ReservedDataTotalSize();

	CExtendedReservedData& ExtendedReservedData() {
		return m_ExtendedReservedData;
	}

	void TimDescriptorViewText(stringstream& ss);
	void BlfViewText(stringstream& ss);
	CBlfHeader& BlfHeader() {
		return m_BlfHeader;
	}

	t_KeyList& KeyList() {return m_KeyList;}
	CDigitalSignature& DigitalSignature() {return m_DigitalSignature;}
	CDigitalSignature& DTIMKeysDataBlf() {return m_DTIMKeysDataBlf;}

	static CTimDescriptorLine* GetLineField(string sFieldName,
			bool bNotFoundMsg = true, void* pObject = 0);
	static CTimDescriptorLine* GetNextLineField(CTimDescriptorLine* pPrev,
			bool bNotFoundMsg = true);
	static CTimDescriptorLine* GetNextLineField(string sFieldName,
			CTimDescriptorLine* pPrev, bool bNotFoundMsg = true,
			int WithinNLines = -1);
	static CTimDescriptorLine* IsFieldPresent(string sFieldName,
			CTimDescriptorLine* pPrev, int WithinNLines);
	static t_TimDescriptorLines g_TimDescriptorLines;
	static t_TimDescriptorLines& TimDescriptorLines() {
		return g_TimDescriptorLines;
	}
	static void DiscardTimDescriptorLines();

	static void SaveTimDescriptorLines();
	static void RestoreTimDescriptorLines();

	static int ParseErrors();
	static t_TimDescriptorLines g_TimDescriptorErrors;
	static void DiscardTimDescriptorErrors();

	static int ParseWarnings();
	static t_TimDescriptorLines g_TimDescriptorWarnings;
	static void DiscardTimDescriptorWarnings();

	static void ParsingError(string sErr, bool bShow = false,
			CTimDescriptorLine* pInLine = 0);
	static void ParsingWarning(string sWarn, bool bShow = false,
			CTimDescriptorLine* pInLine = 0);

	static CTimDescriptorLine* pPrevLine;
	static unsigned int g_uiTimDescriptors;

protected:
	static t_TimDescriptorLines g_SavedTimDescriptorLines;
	static t_TimDescriptorLines& SavedTimDescriptorLines() {
		return g_SavedTimDescriptorLines;
	}
	static void DiscardSavedTimDescriptorLines();

public:
	bool TimParsingError();

	bool IsChanged();
	void Changed(bool bSet);
	bool IsNotWritten() {
		return m_bNotWritten;
	}
	;
	void NotWritten(bool bSet) {
		m_bNotWritten = bSet;
	}

	bool ImagesIsChanged() {
		return m_bImagesChanged;
	}
	void ImagesChanged(bool bSet) {
		m_bImagesChanged = bSet;
	}

	bool ReservedIsChanged() {
		return m_bReservedChanged;
	}
	void ReservedChanged(bool bSet) {
		m_bReservedChanged = bSet;
	}

	bool InjectPin(string& sPinFilePath);

	void Reset();
	void DiscardAll();

	unsigned int GetTimImageSize(bool bOneNANDPadding = false,
			unsigned int uiPaddedSize = 0);
	eProcessorType ProcessorType();

private:
	CCTIM m_TimHeader;
	CBlfHeader m_BlfHeader;

	string m_sTimDescriptorFilePath;
	string m_sTimDescriptor;
	string m_sProcessorType;

	t_ImagesList m_Images;
	t_ReservedDataList m_ReservedDataList;

	CExtendedReservedData m_ExtendedReservedData;

	CDigitalSignature m_DigitalSignature;
	CDigitalSignature m_DTIMKeysDataBlf;
	t_KeyList m_KeyList;

	bool m_bChanged;
	bool m_bImagesChanged;
	bool m_bReservedChanged;
	bool m_bKeysChanged;
	bool m_bNotWritten;
};

#endif //__TIM_DESCRIPTOR_H__
