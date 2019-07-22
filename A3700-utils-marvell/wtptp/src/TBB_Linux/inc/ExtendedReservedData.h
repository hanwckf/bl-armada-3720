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


#ifndef __EXTENDED_RESERVED_DATA_H__
#define __EXTENDED_RESERVED_DATA_H__

#include "ReservedPackageData.h"
#include "ErdBase.h"

#include "DDRInitialization.h"
#include "ConsumerID.h"
#include "TzInitialization.h"
#include "Instructions.h"
#include "DDROperations.h"
#include "TzOperations.h"
#include "GenPurposePatch.h"

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <list>
#include <vector>
using namespace std;

typedef vector<string*> t_stringVector;
typedef vector<string*>::iterator t_stringVectorIter;

typedef list<pair<unsigned int, unsigned int>*> t_PairList;
typedef list<pair<unsigned int, unsigned int>*>::iterator t_PairListIter;

// Block labels to be recognized:
// Descriptor file     binary file
const string sExtendedReservedData = "Extended Reserved Data";

// Clock Enable  => (0x434c4b45) (CLKE),
const string sClockEnable = "Clock Enable";
const string sCLKE = "0x434c4b45";
// DDR Geometry  => (0x44445247) (DDRG),
const string sDDRGeometry = "DDR Geometry";
const string sDDRG = "0x44445247";
// DDR Timing    => (0x44445254) (DDRT),
const string sDDRTiming = "DDR Timing";
const string sDDRT = "0x44445254";
// DDR Custom    => (0x44445243) (DDRC),
const string sDDRCustom = "DDR Custom";
const string sDDRC = "0x44445243";
// Frequency     => (0x46524551) (FREQ),
const string sFrequency = "Frequency";
const string sFREQ = "0x46524551";
// Voltages      => (0x564f4c54) (VOLT)
const string sVoltages = "Voltages";
const string sVOLT = "0x564f4c54";
// "Configure Memory Control  => (0x434d4343) (CCMC)
const string sConfigMemoryControl = "Configure Memory Control";
const string sCMCC = "0x434d4343";
// "Trust Zone => (0x545A4944) (TZID)
const string sTrustZone = "Trust Zone";
const string sTZID = "0x545A4944";
// "Trust Zone Regid=> (0x545A4F4E) (TZON)
const string sTrustZoneRegid = "Trust Zone Regid";
const string sTZON = "0x545A4F4E";
// "OP Div ==> (0x4F504456) (OPDV)
const string sOpDiv = "OpDiv";
const string sOPDV = "0x4F504456";
// "OP Mode ==> (0x4D4F4445) (MODE)
const string sOpMode = "OpMode";
const string sMODE = "0x4D4F4445";

class CTimDescriptor;

class CExtendedReservedData: public CTimLib {
public:

	t_stringVector g_ClockEnableFields;	// CLKE ids: See *_CLOCK_ID_T for a list of ids.
	t_stringVector g_DDRCustomFields;// DDRC ids: See *_MCU_REGID_T for a list of ids.
	t_stringVector g_FrequencyFields;// FREQ ids: See *_FREQ_ID_T for a list of ids.

	t_stringVector g_SdramSpecFields;// DDRG, DDRT ids: See SDRAM_SPEC_T for a list of ids.  
	t_stringVector g_VoltagesFields;	// TBD

	t_stringVector g_ConfigMemoryControlFields; // CCMC ids
	t_stringVector g_TrustZoneFields; // TZID ids
	t_stringVector g_TrustZoneRegidFields; // TZON ids

	t_stringVector g_OpDivFields;  // Opdv ids
	t_stringVector g_OpModeFields; // Mode ids

	void DepopulateExtendedReservedDataFields();
	void Depopulate(t_stringVector& Fields);

public:
	CExtendedReservedData(const string& sProcessorType);
	virtual ~CExtendedReservedData(void);

	CExtendedReservedData(const CExtendedReservedData& rhs);
	CExtendedReservedData& operator=(const CExtendedReservedData& rhs);

	void Reset(void);

	bool IsChanged();
	void Changed(bool bSet) {
		m_bChanged = bSet;
	}

	void AddClockEnableField(pair<unsigned int, unsigned int>*& field) {
		ClockEnableFields.push_back(field);
		Changed(true);
	}

	void AddDDRGeometryField(pair<unsigned int, unsigned int>*& field) {
		DDRGeometryFields.push_back(field);
		Changed(true);
	}

	void AddDDRTimingField(pair<unsigned int, unsigned int>*& field) {
		DDRTimingFields.push_back(field);
		Changed(true);
	}

	void AddDDRCustomField(pair<unsigned int, unsigned int>*& field) {
		DDRCustomFields.push_back(field);
		Changed(true);
	}

	void AddFrequencyField(pair<unsigned int, unsigned int>*& field) {
		FrequencyFields.push_back(field);
		Changed(true);
	}

	void AddVoltagesField(pair<unsigned int, unsigned int>*& field) {
		VoltagesFields.push_back(field);
		Changed(true);
	}

	void AddConfigMemoryControlField(pair<unsigned int, unsigned int>*& field) {
		ConfigMemoryControlFields.push_back(field);
		Changed(true);
	}

	void AddTrustZoneField(pair<unsigned int, unsigned int>*& field) {
		TrustZoneFields.push_back(field);
		Changed(true);
	}

	void AddTrustZoneRegidField(pair<unsigned int, unsigned int>*& field) {
		TrustZoneRegidFields.push_back(field);
		Changed(true);
	}

	void AddOpDivField(pair<unsigned int, unsigned int>*& field) {
		OpDivFields.push_back(field);
		Changed(true);
	}

	void AddOpModeField(pair<unsigned int, unsigned int>*& field) {
		OpModeFields.push_back(field);
		Changed(true);
	}

	int Combine(t_ReservedDataList& ReservedDataList);

	int Size();

	int AddPkg(const string& sSectionTag, t_PairList& Fields,
			t_ReservedDataList& ReservedDataList);

	bool ProcessorTypeStr(const string& sProcessorType) {
		m_sProcessorType = sProcessorType;
		return ProcessorSpecificFields(sProcessorType);
	}
	string& ProcessorTypeStr() {
		return m_sProcessorType;
	}

	t_PairList ClockEnableFields;
	t_PairList DDRGeometryFields;
	t_PairList DDRTimingFields;
	t_PairList DDRCustomFields;
	t_PairList FrequencyFields;
	t_PairList VoltagesFields;
	t_PairList ConfigMemoryControlFields;
	t_PairList TrustZoneFields;
	t_PairList TrustZoneRegidFields;
	t_PairList OpDivFields;
	t_PairList OpModeFields;

	t_ErdBaseVector ErdVec;

	t_ConsumerIDVec m_Consumers;

private:
	bool ProcessorSpecificFields(const string& sProcessor);
	void ERDPackageText(CTimDescriptor& TimDescriptor, stringstream& ss,
			t_PairList& Fields, const string& sPkgName,
			t_stringVector& g_Fields, bool bIsBlf);
	void CopyFields(t_PairList& Fields, t_PairList& nc_rhsFields);
	void RemoveFieldRefs(t_PairList& Fields);

	string m_sProcessorType;
	bool m_bChanged;
};

#endif //__EXTENDED_RESERVED_DATA_H__
