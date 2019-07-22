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


#include "BlfHeader.h"
#include "TimDescriptor.h"

// static object to hold all the *.blf only parsed lines and object references
//t_TimDescriptorLines CBlfHeader::g_BlfHeaderLines;
CTimDescriptorLine* CBlfHeader::pPrevLine = 0;

CBlfHeader::CBlfHeader() :
		CTimLib() {
	Reset();
}

CBlfHeader::~CBlfHeader(void) {

}

CBlfHeader::CBlfHeader(const CBlfHeader& rhs) :
		CTimLib(rhs) {
	// need to do a deep copy of lists to avoid dangling references
	CBlfHeader& nc_rhs = const_cast<CBlfHeader&>(rhs);

	m_sBlfVersion = rhs.m_sBlfVersion;

	// [UE_Options]
	m_uiUEBootOption = rhs.m_uiUEBootOption;

	// [Flash_Properties]
	m_uiMaxUploadSplitSize = rhs.m_uiMaxUploadSplitSize;
	m_uiMaxFBFSplitSize = rhs.m_uiMaxFBFSplitSize;
	m_sFlashFamily = rhs.m_sFlashFamily;
	m_uiSpareAreaSize = rhs.m_uiSpareAreaSize;
	m_uiDataAreaSize = rhs.m_uiDataAreaSize;
	m_uiFBFSectorSize = rhs.m_uiFBFSectorSize;

	// [Flash_Options]
	m_uiProductionMode = rhs.m_uiProductionMode;
	m_uiSkipBlocksNumber = rhs.m_uiSkipBlocksNumber;
	m_bEraseAllFlash = rhs.m_bEraseAllFlash;
	m_bResetBBT = rhs.m_bResetBBT;
	m_pTimDescriptor = rhs.m_pTimDescriptor;
	m_bChanged = rhs.m_bChanged;
}

CBlfHeader& CBlfHeader::operator=(const CBlfHeader& rhs) {
	// assignment operator
	if (&rhs != this) {
		CTimLib::operator=(rhs);

		// need to do a deep copy of lists to avoid dangling references
		CBlfHeader& nc_rhs = const_cast<CBlfHeader&>(rhs);

		m_sBlfVersion = rhs.m_sBlfVersion;

		// [UE_Options]
		m_uiUEBootOption = rhs.m_uiUEBootOption;

		// [Flash_Properties]
		m_uiMaxUploadSplitSize = rhs.m_uiMaxUploadSplitSize;
		m_uiMaxFBFSplitSize = rhs.m_uiMaxFBFSplitSize;
		m_sFlashFamily = rhs.m_sFlashFamily;
		m_uiSpareAreaSize = rhs.m_uiSpareAreaSize;
		m_uiDataAreaSize = rhs.m_uiDataAreaSize;
		m_uiFBFSectorSize = rhs.m_uiFBFSectorSize;

		// [Flash_Options]
		m_uiProductionMode = rhs.m_uiProductionMode;
		m_uiSkipBlocksNumber = rhs.m_uiSkipBlocksNumber;
		m_bEraseAllFlash = rhs.m_bEraseAllFlash;
		m_bResetBBT = rhs.m_bResetBBT;

		m_bChanged = rhs.m_bChanged;
	}
	return *this;
}

void CBlfHeader::Reset() {
	m_sBlfVersion = "V2.1.0";

	// [UE_Options]
	m_uiUEBootOption = 0;

	// [Flash_Properties]
	m_uiMaxUploadSplitSize = 0;
	m_uiMaxFBFSplitSize = 0;
	m_sFlashFamily = "eMMC";
	m_uiSpareAreaSize = 0;
	m_uiDataAreaSize = 0;
	m_uiFBFSectorSize = 4096;

	// [Flash_Options]
	m_uiProductionMode = 0;
	m_uiSkipBlocksNumber = 0;
	m_bEraseAllFlash = false;
	m_bResetBBT = false;

	m_bChanged = false;
}

bool CBlfHeader::ParseBlfHeader(CTimDescriptorLine*& pLine) {
	bool bRet = true;
	pLine = m_pTimDescriptor->GetLineField("[BLF_Version]", true);
	if (pLine != 0) {
		pPrevLine = pLine;
	} else {
		pLine = pPrevLine;
	}

	pLine = m_pTimDescriptor->GetLineField("Blf_Version_Number", true);
	if (pLine != 0) {
		pPrevLine = pLine;
		m_sBlfVersion = pLine->m_FieldValue;
	} else {
		pLine = pPrevLine;
	}

	pLine = m_pTimDescriptor->GetLineField("[UE_Options]", true);
	if (pLine != 0) {
		pPrevLine = pLine;
	} else {
		pLine = pPrevLine;
	}

	pLine = m_pTimDescriptor->GetLineField("UE_Boot_Option", true);
	if (pLine != 0) {
		pPrevLine = pLine;
		m_uiUEBootOption = Translate(pLine->m_FieldValue);
	} else {
		pLine = pPrevLine;
	}

	pLine = m_pTimDescriptor->GetLineField("[Flash_Properties]", true);
	if (pLine != 0) {
		pPrevLine = pLine;
	} else {
		pLine = pPrevLine;
	}

	pLine = m_pTimDescriptor->GetLineField("Max_Upload_Split_Size", true);
	if (pLine != 0) {
		pPrevLine = pLine;
		m_uiMaxUploadSplitSize = Translate(pLine->m_FieldValue);
	} else {
		pLine = pPrevLine;
	}

	pLine = m_pTimDescriptor->GetLineField("Max_FBF_Split_Size", true);
	if (pLine != 0) {
		pPrevLine = pLine;
		m_uiMaxFBFSplitSize = Translate(pLine->m_FieldValue);
	} else {
		pLine = pPrevLine;
	}

	pLine = m_pTimDescriptor->GetLineField("Flash_Family", true);
	if (pLine != 0) {
		pPrevLine = pLine;
		m_sFlashFamily = pLine->m_FieldValue;
	} else {
		pLine = pPrevLine;
	}

	pLine = m_pTimDescriptor->GetLineField("Spare_Area_Size", true);
	if (pLine != 0) {
		pPrevLine = pLine;
		m_uiSpareAreaSize = Translate(pLine->m_FieldValue);
	} else {
		pLine = pPrevLine;
	}

	pLine = m_pTimDescriptor->GetLineField("Data_Area_Size", true);
	if (pLine != 0) {
		pPrevLine = pLine;
		m_uiDataAreaSize = Translate(pLine->m_FieldValue);
	} else {
		pLine = pPrevLine;
	}

	pLine = m_pTimDescriptor->GetLineField("FBF_Sector_Size", true);
	if (pLine != 0) {
		pPrevLine = pLine;
		m_uiFBFSectorSize = Translate(pLine->m_FieldValue);
	} else {
		pLine = pPrevLine;
	}

	pLine = m_pTimDescriptor->GetLineField("[Flash_Options]", true);
	if (pLine != 0) {
		pPrevLine = pLine;
	} else {
		pLine = pPrevLine;
	}

	pLine = m_pTimDescriptor->GetLineField("ProductionMode", true);
	if (pLine != 0) {
		pPrevLine = pLine;
		m_uiProductionMode = Translate(pLine->m_FieldValue);
	} else {
		pLine = pPrevLine;
	}

	pLine = m_pTimDescriptor->GetLineField("Skip_Blocks_Number", true);
	if (pLine != 0) {
		pPrevLine = pLine;
		m_uiSkipBlocksNumber = Translate(pLine->m_FieldValue);
	} else {
		pLine = pPrevLine;
	}

	pLine = m_pTimDescriptor->GetLineField("Erase_All_Flash", true);
	if (pLine != 0) {
		pPrevLine = pLine;
		m_bEraseAllFlash = (Translate(pLine->m_FieldValue) == 0 ? false : true);
	} else {
		pLine = pPrevLine;
	}

	pLine = m_pTimDescriptor->GetLineField("Reset_BBT", true);
	if (pLine != 0) {
		pPrevLine = pLine;
		m_bResetBBT = (Translate(pLine->m_FieldValue) == 0 ? false : true);
	} else {
		pLine = pPrevLine;
	}

	return bRet;
}
