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

#include "ErdBase.h"
#include "TimDescriptorParser.h"

#include "AutoBindERD.h"
#include "CoreResetERD.h"
#include "EscapeSeqERD.h"
#include "GpioSetERD.h"
#include "ResumeDdrERD.h"
#include "ROMResumeERD.h"
#include "TbrXferERD.h"
#include "UartERD.h"
#include "UsbERD.h"
#include "UsbVendorReqERD.h"
#include "PinERD.h"
#include "ConsumerID.h"
#include "TzInitialization.h"
#include "TzOperations.h"
#include "BbmtERD.h"
#include "GenPurposePatch.h"
#include "GPPOperations.h"
#include "ImageMapERD.h"
#include "CoreReleaseERD.h"
#include "EscapeSeqV2ERD.h"
#include "OnDemandBootERD.h"
#include "DDRTypeERD.h"
#include "FlashGeometryReqERD.h"

CErdBase::CErdBase(ERD_PKG_TYPE ErdType, int iMaxFieldNum) :
		m_eErdType(ErdType), m_iMaxFieldNum(iMaxFieldNum), CTimLib() {
	m_bChanged = false;
	Init();
}

CErdBase::~CErdBase(void) {
	Reset();
}

// copy constructor
CErdBase::CErdBase(const CErdBase& rhs) :
		CTimLib(rhs) {
	m_eErdType = rhs.m_eErdType;
	m_iMaxFieldNum = rhs.m_iMaxFieldNum;

	Init();

	m_bChanged = rhs.m_bChanged;

	// need to do a deep copy of lists to avoid dangling references
	for (unsigned int i = 0; i < m_iMaxFieldNum; i++) {
		*m_FieldNames[i] = *rhs.m_FieldNames[i];
		*m_FieldValues[i] = *rhs.m_FieldValues[i];
		*m_FieldComments[i] = *rhs.m_FieldComments[i];
	}
}

// assignment operator
CErdBase& CErdBase::operator=(const CErdBase& rhs) {
	// assignment operator
	if (&rhs != this) {
		CTimLib::operator=(rhs);
		m_eErdType = rhs.m_eErdType;
		m_iMaxFieldNum = rhs.m_iMaxFieldNum;

		Init();

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

void CErdBase::Reset() {

	if (m_FieldNames.size() > 0) {
		for_each(m_FieldNames.begin(), m_FieldNames.end(),
				[](string*& ps) {delete ps;});
		m_FieldNames.clear();
	}

	if (m_FieldValues.size() > 0) {
		for_each(m_FieldValues.begin(), m_FieldValues.end(),
				[](string*& ps) {delete ps;});
		m_FieldValues.clear();
	}

	if (m_FieldComments.size() > 0) {
		for_each(m_FieldComments.begin(), m_FieldComments.end(),
				[](string*& ps) {delete ps;});
		m_FieldComments.clear();
	}
}

void CErdBase::Init() {
	Reset();

	m_FieldNames.resize(m_iMaxFieldNum);
	m_FieldValues.resize(m_iMaxFieldNum);
	m_FieldComments.resize(m_iMaxFieldNum);

	for (unsigned int i = 0; i < m_iMaxFieldNum; i++) {
		m_FieldNames[i] = new string("");
		m_FieldValues[i] = new string("0");
		m_FieldComments[i] = new string("");
	}
}

CErdBase* CErdBase::Create(ERD_PKG_TYPE ErdPkgType) {
	CErdBase* pErdBase = 0;

	switch (ErdPkgType) {
	case GPIO_ERD:
		pErdBase = new CGpio;
		break;

	case XFER_ERD:
		pErdBase = new CXfer;
		break;

	case CORE_PAIR_ERD:
		pErdBase = new CCorePair;
		break;

	case MFPR_PAIR_ERD:
		pErdBase = new CMFPRPair;
		break;

	default:
		pErdBase = 0;
		break;
	}

	return pErdBase;
}

CErdBase* CErdBase::Create(CErdBase& src) {
	CErdBase* pErdBase = 0;

	switch (src.ErdPkgType()) {
	case AUTOBIND_ERD:
		pErdBase = new CAutoBind(reinterpret_cast<CAutoBind&>(src));
		break;
	case ESCAPESEQ_ERD:
		pErdBase = new CEscapeSeq(reinterpret_cast<CEscapeSeq&>(src));
		break;
	case GPIOSET_ERD:
		pErdBase = new CGpioSet(reinterpret_cast<CGpioSet&>(src));
		break;
	case GPIO_ERD:
		pErdBase = new CGpio(reinterpret_cast<CGpio&>(src));
		break;
	case RESUME_DDR_ERD:
		pErdBase = new CResumeDdr(reinterpret_cast<CResumeDdr&>(src));
		break;
	case ROM_RESUME_ERD:
		pErdBase = new CROMResume(reinterpret_cast<CROMResume&>(src));
		break;
	case TBRXFER_ERD:
		pErdBase = new CTBRXferSet(reinterpret_cast<CTBRXferSet&>(src));
		break;
	case XFER_ERD:
		pErdBase = new CXfer(reinterpret_cast<CXfer&>(src));
		break;
	case UART_ERD:
		pErdBase = new CUart(reinterpret_cast<CUart&>(src));
		break;
	case USB_ERD:
		pErdBase = new CUsb(reinterpret_cast<CUsb&>(src));
		break;
	case USBVENDORREQ_ERD:
		pErdBase = new CUsbVendorReq(reinterpret_cast<CUsbVendorReq&>(src));
		break;
	case CONSUMER_ID_ERD:
		pErdBase = new CConsumerID(reinterpret_cast<CConsumerID&>(src));
		break;
	case DDR_INITIALIZATION_ERD:
		pErdBase = new CDDRInitialization(
				reinterpret_cast<CDDRInitialization&>(src));
		break;
	case INSTRUCTIONS_ERD:
		pErdBase = new CInstructions(reinterpret_cast<CInstructions&>(src));
		break;
	case DDR_OPERATIONS_ERD:
		pErdBase = new CDDROperations(reinterpret_cast<CDDROperations&>(src));
		break;
	case CORE_ID_ERD:
		pErdBase = new CCoreReset(reinterpret_cast<CCoreReset&>(src));
		break;
	case TZ_INITIALIZATION_ERD:
		pErdBase = new CTzInitialization(
				reinterpret_cast<CTzInitialization&>(src));
		break;
	case TZ_OPERATIONS_ERD:
		pErdBase = new CTzOperations(reinterpret_cast<CTzOperations&>(src));
		break;
	case BBMT_ERD:
		pErdBase = new CBbmt(reinterpret_cast<CBbmt&>(src));
		break;
	case GPP_ERD:
		pErdBase = new CGenPurposePatch(
				reinterpret_cast<CGenPurposePatch&>(src));
		break;
	case GPP_OPERATIONS_ERD:
		pErdBase = new CGPPOperations(reinterpret_cast<CGPPOperations&>(src));
		break;
	case PIN_ERD:
		pErdBase = new CPin(reinterpret_cast<CPin&>(src));
		break;
	case IMAGE_MAPS_ERD:
		pErdBase = new CImageMaps(reinterpret_cast<CImageMaps&>(src));
		break;
	case IMAGE_MAP_INFO_ERD:
		pErdBase = new CImageMapInfo(reinterpret_cast<CImageMapInfo&>(src));
		;
		break;
	case CORE_RELEASE_ERD:
		pErdBase = new CCoreRelease(reinterpret_cast<CCoreRelease&>(src));
		;
		break;
	case CORE_PAIR_ERD:
		pErdBase = new CCorePair(reinterpret_cast<CCorePair&>(src));
		;
		break;
	case ESC_SEQ_V2_ERD:
		pErdBase = new CEscapeSeqV2(reinterpret_cast<CEscapeSeqV2&>(src));
		break;
	case MFPR_PAIR_ERD:
		pErdBase = new CMFPRPair(reinterpret_cast<CMFPRPair&>(src));
		break;
	case ONDEMAND_BOOT_ERD:
		pErdBase = new COnDemandBoot(reinterpret_cast<COnDemandBoot&>(src));
		break;
	case DDR_TYPE_ERD:
		pErdBase = new CDDRType(reinterpret_cast<CDDRType&>(src));
		break;
	case FLASH_GEOMETRY_REQ_ERD:
		pErdBase = new CFlashGeometryReq(
				reinterpret_cast<CFlashGeometryReq&>(src));
		break;
	default:
		pErdBase = 0;
	}

	return pErdBase;
}

CErdBase* CErdBase::Create(string& sPackageName, bool bIsBlf) {
	CErdBase* pErdBase = 0;

	if (sPackageName == "AutoBind")
		pErdBase = new CAutoBind;
	else if (sPackageName == "Escape Sequence")
		pErdBase = new CEscapeSeq;
	else if (sPackageName == "GpioSet")
		pErdBase = new CGpioSet;
	else if (sPackageName == "Gpio")
		pErdBase = new CGpio;
	else if (sPackageName == "Resume DDR")
		pErdBase = new CResumeDdr;
	else if (sPackageName == "ROM Resume")
		pErdBase = new CROMResume;
	else if (sPackageName == "TBR Xfer Set")
		pErdBase = new CTBRXferSet;
	else if (sPackageName == "Xfer")
		pErdBase = new CXfer;
	else if (sPackageName == "Uart")
		pErdBase = new CUart;
	else if (sPackageName == "Usb")
		pErdBase = new CUsb;
	else if (sPackageName == "Usb Vendor Request")
		pErdBase = new CUsbVendorReq;
	else if (sPackageName == "Consumer ID")
		pErdBase = new CConsumerID;
	else if (sPackageName == "DDR Initialization")
		pErdBase = new CDDRInitialization;
	else if (sPackageName == "Instruction")
		pErdBase = new CInstructions;
	else if (sPackageName == "Operations")    // DDR Operations
		pErdBase = new CDDROperations;
	else if (sPackageName == "Core Reset")
		pErdBase = new CCoreReset;
	else if (sPackageName == "TZ Initialization")
		pErdBase = new CTzInitialization;
	else if (sPackageName == "TZ Operations")
		pErdBase = new CTzOperations;
	else if (sPackageName == "BBMT")
		pErdBase = new CBbmt;
	else if (sPackageName == "GPP")
		pErdBase = new CGenPurposePatch;
	else if (sPackageName == "GPP Operations")
		pErdBase = new CGPPOperations;
	else if (sPackageName == "Pin")
		pErdBase = new CPin;
	else if (sPackageName == "Image Maps")
		pErdBase = new CImageMaps(bIsBlf);
	else if (sPackageName == "Image Map Info")
		pErdBase = new CImageMapInfo(bIsBlf);
	else if (sPackageName == "Core Pair")
		pErdBase = new CCorePair;
	else if (sPackageName == "Core Release")
		pErdBase = new CCoreRelease;
	else if (sPackageName == "Escape Seq V2")
		pErdBase = new CEscapeSeqV2;
	else if (sPackageName == "MFPR Pair")
		pErdBase = new CMFPRPair;
	else if (sPackageName == "OnDemand Boot")
		pErdBase = new COnDemandBoot;
	else if (sPackageName == "DTYP")
		pErdBase = new CDDRType;
	else if (sPackageName == "Flash Geometry Package")
		pErdBase = new CFlashGeometryReq;
	return pErdBase;
}

bool CErdBase::Parse(CTimDescriptor& TimDescriptor, CTimDescriptorLine*& pLine,
		bool bIsBlf) {
	m_bChanged = false;

	while ((pLine = TimDescriptor.GetNextLineField(pLine))) {
		if (pLine->m_FieldName
				== (bIsBlf ?
						SpaceToUnderscore("End " + PackageName()) :
						("End " + PackageName()))) {
			return true;
		}

		bool bFound = false;
		for (unsigned int idx = 0; idx < m_FieldNames.size(); idx++) {
			if (pLine->m_FieldName
					== (bIsBlf ?
							SpaceToUnderscore(*m_FieldNames[idx]) :
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
							SpaceToUnderscore(PackageName()) : PackageName());
			TimDescriptor.ParsingError(ss.str(), true, pLine);
			break;
		}
	}

	if (pLine == 0)
		return false;

	// field not found  
	return true;
}
