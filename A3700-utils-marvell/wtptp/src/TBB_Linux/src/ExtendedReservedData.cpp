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

#include "ExtendedReservedData.h"
#include "TimDescriptor.h"
#include "Sdram_Strings.h"
#include "Aspen_Strings.h"
#include "Mmp2_Strings.h"
#include "Mmp3_Strings.h"
#include "Ttc1_Strings.h"

CExtendedReservedData::CExtendedReservedData(const string& sProcessorType) :
		CTimLib() {
	ProcessorTypeStr(sProcessorType);
	m_bChanged = false;

	// initialize common to all processor sdram strings
	SdramStrings SdramStrings(*this);
}

CExtendedReservedData::~CExtendedReservedData(void) {
	Reset();
	DepopulateExtendedReservedDataFields();
}

CExtendedReservedData::CExtendedReservedData(const CExtendedReservedData& rhs) :
		m_sProcessorType(rhs.m_sProcessorType), CTimLib(rhs) {
	// copy constructor
	m_bChanged = rhs.m_bChanged;

	// need to do a deep copy of lists to avoid dangling references
	CExtendedReservedData& nc_rhs = const_cast<CExtendedReservedData&>(rhs);

	CopyFields(ClockEnableFields, nc_rhs.ClockEnableFields);
	CopyFields(DDRGeometryFields, nc_rhs.DDRGeometryFields);
	CopyFields(DDRTimingFields, nc_rhs.DDRTimingFields);
	CopyFields(DDRCustomFields, nc_rhs.DDRCustomFields);
	CopyFields(FrequencyFields, nc_rhs.FrequencyFields);
	CopyFields(VoltagesFields, nc_rhs.VoltagesFields);
	CopyFields(ConfigMemoryControlFields, nc_rhs.ConfigMemoryControlFields);
	CopyFields(TrustZoneFields, nc_rhs.TrustZoneFields);
	CopyFields(TrustZoneRegidFields, nc_rhs.TrustZoneRegidFields);
	CopyFields(OpDivFields, nc_rhs.OpDivFields);
	CopyFields(OpModeFields, nc_rhs.OpModeFields);

	if (nc_rhs.g_ClockEnableFields.size() > 0) {
		for_each(nc_rhs.g_ClockEnableFields.begin(),
				nc_rhs.g_ClockEnableFields.end(),
				[this](string*& ps) {g_ClockEnableFields.push_back(new string(*ps));});
	}

	if (nc_rhs.g_SdramSpecFields.size() > 0) {
		for_each(nc_rhs.g_SdramSpecFields.begin(),
				nc_rhs.g_SdramSpecFields.end(),
				[this](string*& ps) {g_SdramSpecFields.push_back(new string(*ps));});
	}

	if (nc_rhs.g_DDRCustomFields.size() > 0) {
		for_each(nc_rhs.g_DDRCustomFields.begin(),
				nc_rhs.g_DDRCustomFields.end(),
				[this](string*& ps) {g_DDRCustomFields.push_back(new string(*ps));});
	}

	if (nc_rhs.g_FrequencyFields.size() > 0) {
		for_each(nc_rhs.g_FrequencyFields.begin(),
				nc_rhs.g_FrequencyFields.end(),
				[this](string*& ps) {g_FrequencyFields.push_back(new string(*ps));});
	}

	if (nc_rhs.g_VoltagesFields.size() > 0) {
		for_each(nc_rhs.g_VoltagesFields.begin(), nc_rhs.g_VoltagesFields.end(),
				[this](string*& ps) {g_VoltagesFields.push_back(new string(*ps));});
	}

	if (nc_rhs.g_ConfigMemoryControlFields.size() > 0) {
		for_each(nc_rhs.g_ConfigMemoryControlFields.begin(),
				nc_rhs.g_ConfigMemoryControlFields.end(),
				[this](string*& ps) {g_ConfigMemoryControlFields.push_back(new string(*ps));});
	}

	if (nc_rhs.g_TrustZoneFields.size() > 0) {
		for_each(nc_rhs.g_TrustZoneFields.begin(),
				nc_rhs.g_TrustZoneFields.end(),
				[this](string*& ps) {g_TrustZoneFields.push_back(new string(*ps));});
	}

	if (nc_rhs.g_TrustZoneRegidFields.size() > 0) {
		for_each(nc_rhs.g_TrustZoneRegidFields.begin(),
				nc_rhs.g_TrustZoneRegidFields.end(),
				[this](string*& ps) {g_TrustZoneRegidFields.push_back(new string(*ps));});
	}

	if (nc_rhs.g_OpDivFields.size() > 0) {
		for_each(nc_rhs.g_OpDivFields.begin(), nc_rhs.g_OpDivFields.end(),
				[this](string*& ps) {g_OpDivFields.push_back(new string(*ps));});
	}

	if (nc_rhs.g_OpModeFields.size() > 0) {
		for_each(nc_rhs.g_OpModeFields.begin(), nc_rhs.g_OpModeFields.end(),
				[this](string*& ps) {g_OpModeFields.push_back(new string(*ps));});
	}

	if (nc_rhs.ErdVec.size() > 0) {
		for_each(nc_rhs.ErdVec.begin(), nc_rhs.ErdVec.end(),
				[this](CErdBase*& pRhsErd)
				{
					CErdBase* pErd = CErdBase::Create(*pRhsErd);
					if (pErd)
					ErdVec.push_back(pErd);
				});
	}

	if (nc_rhs.m_Consumers.size() > 0) {
		for_each(nc_rhs.m_Consumers.begin(), nc_rhs.m_Consumers.end(),
				[this](CConsumerID*& pRhsConsumer)
				{
					CConsumerID* pConsumer = new CConsumerID(*pRhsConsumer);
					m_Consumers.push_back(pConsumer);
				});
	}
}

CExtendedReservedData& CExtendedReservedData::operator=(
		const CExtendedReservedData& rhs) {
	// assignment operator
	if (&rhs != this) {
		CTimLib::operator=(rhs);

		// delete the existing list and recreate a new one
		Reset();
		DepopulateExtendedReservedDataFields();

		m_bChanged = rhs.m_bChanged;

		m_sProcessorType = rhs.m_sProcessorType;

		// need to do a deep copy of lists to avoid dangling references
		CExtendedReservedData& nc_rhs = const_cast<CExtendedReservedData&>(rhs);

		CopyFields(ClockEnableFields, nc_rhs.ClockEnableFields);
		CopyFields(DDRGeometryFields, nc_rhs.DDRGeometryFields);
		CopyFields(DDRTimingFields, nc_rhs.DDRTimingFields);
		CopyFields(DDRCustomFields, nc_rhs.DDRCustomFields);
		CopyFields(FrequencyFields, nc_rhs.FrequencyFields);
		CopyFields(VoltagesFields, nc_rhs.VoltagesFields);
		CopyFields(ConfigMemoryControlFields, nc_rhs.ConfigMemoryControlFields);
		CopyFields(TrustZoneFields, nc_rhs.TrustZoneFields);
		CopyFields(TrustZoneRegidFields, nc_rhs.TrustZoneRegidFields);
		CopyFields(OpDivFields, nc_rhs.OpDivFields);
		CopyFields(OpModeFields, nc_rhs.OpModeFields);

		if (nc_rhs.g_ClockEnableFields.size() > 0) {
			for_each(nc_rhs.g_ClockEnableFields.begin(),
					nc_rhs.g_ClockEnableFields.end(),
					[this](string*& ps) {g_ClockEnableFields.push_back(new string(*ps));});
		}

		if (nc_rhs.g_SdramSpecFields.size() > 0) {
			for_each(nc_rhs.g_SdramSpecFields.begin(),
					nc_rhs.g_SdramSpecFields.end(),
					[this](string*& ps) {g_SdramSpecFields.push_back(new string(*ps));});
		}

		if (nc_rhs.g_DDRCustomFields.size() > 0) {
			for_each(nc_rhs.g_DDRCustomFields.begin(),
					nc_rhs.g_DDRCustomFields.end(),
					[this](string*& ps) {g_DDRCustomFields.push_back(new string(*ps));});
		}

		if (nc_rhs.g_FrequencyFields.size() > 0) {
			for_each(nc_rhs.g_FrequencyFields.begin(),
					nc_rhs.g_FrequencyFields.end(),
					[this](string*& ps) {g_FrequencyFields.push_back(new string(*ps));});
		}

		if (nc_rhs.g_VoltagesFields.size() > 0) {
			for_each(nc_rhs.g_VoltagesFields.begin(),
					nc_rhs.g_VoltagesFields.end(),
					[this](string*& ps) {g_VoltagesFields.push_back(new string(*ps));});
		}

		if (nc_rhs.g_ConfigMemoryControlFields.size() > 0) {
			for_each(nc_rhs.g_ConfigMemoryControlFields.begin(),
					nc_rhs.g_ConfigMemoryControlFields.end(),
					[this](string*& ps) {g_ConfigMemoryControlFields.push_back(new string(*ps));});
		}

		if (nc_rhs.g_TrustZoneFields.size() > 0) {
			for_each(nc_rhs.g_TrustZoneFields.begin(),
					nc_rhs.g_TrustZoneFields.end(),
					[this](string*& ps) {g_TrustZoneFields.push_back(new string(*ps));});
		}

		if (nc_rhs.g_TrustZoneRegidFields.size() > 0) {
			for_each(nc_rhs.g_TrustZoneRegidFields.begin(),
					nc_rhs.g_TrustZoneRegidFields.end(),
					[this](string*& ps) {g_TrustZoneRegidFields.push_back(new string(*ps));});
		}

		if (nc_rhs.g_OpDivFields.size() > 0) {
			for_each(nc_rhs.g_OpDivFields.begin(), nc_rhs.g_OpDivFields.end(),
					[this](string*& ps) {g_OpDivFields.push_back(new string(*ps));});
		}

		if (nc_rhs.g_OpModeFields.size() > 0) {
			for_each(nc_rhs.g_OpModeFields.begin(), nc_rhs.g_OpModeFields.end(),
					[this](string*& ps) {g_OpModeFields.push_back(new string(*ps));});
		}

		if (nc_rhs.ErdVec.size() > 0) {
			for_each(nc_rhs.ErdVec.begin(), nc_rhs.ErdVec.end(),
					[this](CErdBase*& pRhsErd)
					{
						CErdBase* pErd = CErdBase::Create(*pRhsErd);
						if (pErd)
						ErdVec.push_back(pErd);
					});
		}

		if (nc_rhs.m_Consumers.size() > 0) {
			for_each(nc_rhs.m_Consumers.begin(), nc_rhs.m_Consumers.end(),
					[this](CConsumerID*& pRhsConsumer)
					{
						CConsumerID* pConsumer = new CConsumerID(*pRhsConsumer);
						m_Consumers.push_back(pConsumer);
					});
		}
	}
	return *this;
}

void CExtendedReservedData::CopyFields(t_PairList& Fields,
		t_PairList& nc_rhsFields) {
	if (nc_rhsFields.size() > 0) {
		t_PairListIter iter = nc_rhsFields.begin();
		while (iter != nc_rhsFields.end()) {
			pair<unsigned int, unsigned int>* pPair = new pair<unsigned int,
					unsigned int>((*iter)->first, (*iter)->second);
			Fields.push_back(pPair);
			iter++;
		}
	}
}

void CExtendedReservedData::Reset(void) {
	RemoveFieldRefs(ClockEnableFields);
	RemoveFieldRefs(DDRGeometryFields);
	RemoveFieldRefs(DDRTimingFields);
	RemoveFieldRefs(DDRCustomFields);
	RemoveFieldRefs(FrequencyFields);
	RemoveFieldRefs(VoltagesFields);
	RemoveFieldRefs(ConfigMemoryControlFields);
	RemoveFieldRefs(TrustZoneFields);
	RemoveFieldRefs(TrustZoneRegidFields);
	RemoveFieldRefs(OpDivFields);
	RemoveFieldRefs(OpModeFields);

	// delete all package data strings
	if (ErdVec.size() > 0) {
		for_each(ErdVec.begin(), ErdVec.end(), [](CErdBase*& pBase)
		{
			delete pBase;
		});
	}
	ErdVec.clear();

	if (m_Consumers.size() > 0) {
		for_each(m_Consumers.begin(), m_Consumers.end(),
				[](CConsumerID*& pConsumer)
				{
					delete pConsumer;
				});
	}
	m_Consumers.clear();

	m_bChanged = false;
}

void CExtendedReservedData::RemoveFieldRefs(t_PairList& Fields) {
	if (Fields.size() > 0) {
		for_each(Fields.begin(), Fields.end(),
				[](pair<unsigned int, unsigned int>*& pField)
				{
					delete pField;
				});
	}
	Fields.clear();
}

int CExtendedReservedData::Combine(t_ReservedDataList& ReservedDataList) {
	int nBytesAdded = 0;
	// add ClockEnable package
	if (ClockEnableFields.size() > 0)
		nBytesAdded += AddPkg(sCLKE, ClockEnableFields, ReservedDataList);

	if (DDRGeometryFields.size() > 0)
		nBytesAdded += AddPkg(sDDRG, DDRGeometryFields, ReservedDataList);

	if (DDRTimingFields.size() > 0)
		nBytesAdded += AddPkg(sDDRT, DDRTimingFields, ReservedDataList);

	if (DDRCustomFields.size() > 0)
		nBytesAdded += AddPkg(sDDRC, DDRCustomFields, ReservedDataList);

	if (FrequencyFields.size() > 0)
		nBytesAdded += AddPkg(sFREQ, FrequencyFields, ReservedDataList);

	if (VoltagesFields.size() > 0)
		nBytesAdded += AddPkg(sVOLT, VoltagesFields, ReservedDataList);

	if (ConfigMemoryControlFields.size() > 0)
		nBytesAdded += AddPkg(sCMCC, ConfigMemoryControlFields,
				ReservedDataList);

	if (TrustZoneFields.size() > 0)
		nBytesAdded += AddPkg(sTZID, TrustZoneFields, ReservedDataList);

	if (TrustZoneRegidFields.size() > 0)
		nBytesAdded += AddPkg(sTZON, TrustZoneRegidFields, ReservedDataList);

	if (OpDivFields.size() > 0)
		nBytesAdded += AddPkg(sOPDV, OpDivFields, ReservedDataList);

	if (OpModeFields.size() > 0)
		nBytesAdded += AddPkg(sMODE, OpModeFields, ReservedDataList);

	if (m_Consumers.size() > 0) {
		CReservedPackageData* pRPD = new CReservedPackageData;
		pRPD->PackageIdTag(HexFormattedAscii(CIDPID));

		nBytesAdded += 8; // WRAH; 

		string* pData = new string;
		*pData = HexFormattedAscii((unsigned int) m_Consumers.size());
		pRPD->AddData(pData, new string("num CIDs"));

		nBytesAdded += 4; // num CIDs 

		t_ConsumerIDVecIter ConsumerIter = m_Consumers.begin();
		while (ConsumerIter != m_Consumers.end()) {
			nBytesAdded += (*ConsumerIter)->AddPkgStrings(pRPD);
			ConsumerIter++;
		}

		ReservedDataList.push_back(pRPD);
	}

	t_ErdBaseVectorIter ErdIter = ErdVec.begin();
	while (ErdIter != ErdVec.end()) {

		{
			CReservedPackageData* pRPD = new CReservedPackageData;
			nBytesAdded += (*ErdIter)->AddPkgStrings(pRPD);
			ReservedDataList.push_back(pRPD);
			ErdIter++;
		}
	}

	return nBytesAdded;
}

int CExtendedReservedData::AddPkg(const string& sSectionTag, t_PairList& Fields,
		t_ReservedDataList& ReservedDataList) {
	int nBytesAdded = 0;
	CReservedPackageData* pPkg = new CReservedPackageData;
	pPkg->PackageIdTag(sSectionTag);
	t_PairListIter iter = Fields.begin();
	while (iter != Fields.end()) {
		pPkg->PackageDataList().push_back(
				new string(HexFormattedAscii((*iter)->first)));
		pPkg->PackageDataList().push_back(
				new string(HexFormattedAscii((*iter)->second)));
		iter++;
	}
	nBytesAdded += pPkg->Size();
	ReservedDataList.push_back(pPkg);

	m_bChanged = true;

	return nBytesAdded;
}

void CExtendedReservedData::DepopulateExtendedReservedDataFields() {
	Depopulate(g_ClockEnableFields);
	Depopulate(g_FrequencyFields);
	Depopulate(g_SdramSpecFields);
	Depopulate(g_DDRCustomFields);
	Depopulate(g_VoltagesFields);
	Depopulate(g_ConfigMemoryControlFields);
	Depopulate(g_TrustZoneFields);
	Depopulate(g_TrustZoneRegidFields);
	Depopulate(g_OpDivFields);
	Depopulate(g_OpModeFields);
}

void CExtendedReservedData::Depopulate(t_stringVector& Fields) {
	if (Fields.size() > 0) {
		for_each(Fields.begin(), Fields.end(), [](string*& ps) {delete ps;});
		Fields.clear();
	}
}

bool CExtendedReservedData::ProcessorSpecificFields(const string& sProcessor) {
	DepopulateExtendedReservedDataFields();

	string sUCProcessor = ToUpper(sProcessor);

	if (sProcessor.size() > 0) {
		if (sUCProcessor == ToUpper(gsProcessorType[PXA30x]))
			return true;
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA31x]))
			return true;
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA32x]))
			return true;
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA168]))
			//initialize ExtReservedData with Aspen specific strings;
			AspenStrings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[ARMADA610]))
			//initialize ExtReservedData with MMP2 specific strings;
			MMP2Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA91x]))
			//initialize ExtReservedData with TTC1 specific strings;
			TTC1Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA92x]))
			//initialize ExtReservedData with TTC1 specific strings;
			TTC1Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA93x]))
			//initialize ExtReservedData with TTC1 specific strings;
			TTC1Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA94x]))
			//initialize ExtReservedData with TTC1 specific strings;
			TTC1Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA95x]))
			//initialize ExtReservedData with TTC1 specific strings;
			TTC1Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[ARMADA16x]))
			//initialize ExtReservedData with Aspen specific strings;
			AspenStrings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA955]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA968]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA1701]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA978]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA2128]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[ARMADA622]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA1202]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA1801]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[P88PA62_70]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA988]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA1920]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA2101]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA192]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA1928]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA1986]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA1802]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA986]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA1206]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA888]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA1088]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA1812]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA1822]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA1U88]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA1936]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA1908]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA1826]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[P88PA62_20]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[P88PA62_10]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[PXA1956]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[VEGA]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[BOWIE]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[ULC2]))
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		else if (sUCProcessor == ToUpper(gsProcessorType[UNDEFINED])) {
			//initialize ExtReservedData with MMP3 specific strings;
			MMP3Strings CustomFields(*this);
		} else {
			//initialize ExtReservedData with MMP3 specific strings;
			printf(
					"Error: Processor Type not recognized...Parsing continues using <undefined> Processor Type.\n");
			return false;
		}

	}
	// initialize common to all processor sdram strings
	SdramStrings SdramStrings(*this);

	return true;
}

bool CExtendedReservedData::IsChanged() {
	t_ErdBaseVectorIter ErdIter = ErdVec.begin();
	while (ErdIter != ErdVec.end()) {
		if ((*ErdIter)->IsChanged())
			return true;
		ErdIter++;
	}

	t_ConsumerIDVecIter ConsumerIter = m_Consumers.begin();
	while (ConsumerIter != m_Consumers.end()) {
		if ((*ConsumerIter)->IsChanged())
			return true;
		ConsumerIter++;
	}

	return m_bChanged;
}

int CExtendedReservedData::Size() {
	int iSize = (int) (
	// size indicate number of pairs in list
	// *8 for number of bytes in field id & field value pair
	// + 8 for package tag & package size
			(ClockEnableFields.size() > 0 ?
					(ClockEnableFields.size() * 8) + 8 : 0)
					+ (DDRGeometryFields.size() > 0 ?
							(DDRGeometryFields.size() * 8) + 8 : 0)
					+ (DDRTimingFields.size() > 0 ?
							(DDRTimingFields.size() * 8) + 8 : 0)
					+ (DDRCustomFields.size() > 0 ?
							(DDRCustomFields.size() * 8) + 8 : 0)
					+ (FrequencyFields.size() > 0 ?
							(FrequencyFields.size() * 8) + 8 : 0)
					+ (VoltagesFields.size() > 0 ?
							(VoltagesFields.size() * 8) + 8 : 0)
					+ (ConfigMemoryControlFields.size() > 0 ?
							(ConfigMemoryControlFields.size() * 8) + 8 : 0)
					+ (TrustZoneFields.size() > 0 ?
							(TrustZoneFields.size() * 8) + 8 : 0)
					+ (TrustZoneRegidFields.size() > 0 ?
							(TrustZoneRegidFields.size() * 8) + 8 : 0)
					+ (OpDivFields.size() > 0 ? (OpDivFields.size() * 8) + 8 : 0)
					+ (OpModeFields.size() > 0 ?
							(OpModeFields.size() * 8) + 8 : 0));

	t_ErdBaseVectorIter ErdIter = ErdVec.begin();
	while (ErdIter != ErdVec.end()) {
		iSize += (*ErdIter)->PackageSize();
		ErdIter++;
	}

	if (m_Consumers.size() > 0) {
		iSize += 12; // 8(WRAH) + 4 (num CIDs)
		t_ConsumerIDVecIter ConsumerIter = m_Consumers.begin();
		while (ConsumerIter != m_Consumers.end()) {
			iSize += (*ConsumerIter)->PackageSize();
			ConsumerIter++;
		}
	}

	return iSize;
}

