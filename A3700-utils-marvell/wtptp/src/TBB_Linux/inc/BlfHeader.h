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

#ifndef __BLF_HEADER_H__
#define __BLF_HEADER_H__

#include "TimLib.h"
#include "TimDescriptorLine.h"

#include <string>
#include <list>
#include <iterator>
using namespace std;

// predeclarations
class CTimDescriptor;

class CBlfHeader: public CTimLib {
public:
	CBlfHeader();
	virtual ~CBlfHeader(void);

	// copy constructor
	CBlfHeader(const CBlfHeader& rhs);
	// assignment operator
	CBlfHeader& operator=(const CBlfHeader& rhs);

	void BlfVersion(string& sVersion) {
		m_sBlfVersion = sVersion;
		m_bChanged = true;
	}
	string& BlfVersion() {
		return m_sBlfVersion;
	}

	void UEBootOption(unsigned int uiUEBootOption) {
		m_uiUEBootOption = uiUEBootOption;
		m_bChanged = true;
	}
	unsigned int& UEBootOption() {
		return m_uiUEBootOption;
	}

	void MaxUploadSplitSize(unsigned int uiMaxUploadSplitSize) {
		m_uiMaxUploadSplitSize = uiMaxUploadSplitSize;
		m_bChanged = true;
	}
	unsigned int& MaxUploadSplitSize() {
		return m_uiMaxUploadSplitSize;
	}

	void MaxFBFSplitSize(unsigned int uiMaxFBFSplitSize) {
		m_uiMaxFBFSplitSize = uiMaxFBFSplitSize;
		m_bChanged = true;
	}
	unsigned int& MaxFBFSplitSize() {
		return m_uiMaxFBFSplitSize;
	}

	void FlashFamily(string& sFlashFamily) {
		m_sFlashFamily = sFlashFamily;
		m_bChanged = true;
	}
	string& FlashFamily() {
		return m_sFlashFamily;
	}

	void SpareAreaSize(unsigned int uiSpareAreaSize) {
		m_uiSpareAreaSize = uiSpareAreaSize;
		m_bChanged = true;
	}
	unsigned int& SpareAreaSize() {
		return m_uiSpareAreaSize;
	}

	void DataAreaSize(unsigned int uiDataAreaSize) {
		m_uiDataAreaSize = uiDataAreaSize;
		m_bChanged = true;
	}
	unsigned int& DataAreaSize() {
		return m_uiDataAreaSize;
	}

	void FBFSectorSize(unsigned int uiFBFSectorSize) {
		m_uiFBFSectorSize = uiFBFSectorSize;
		m_bChanged = true;
	}
	unsigned int& FBFSectorSize() {
		return m_uiFBFSectorSize;
	}

	void ProductionMode(unsigned int uiProductionMode) {
		m_uiProductionMode = uiProductionMode;
		m_bChanged = true;
	}
	unsigned int& ProductionMode() {
		return m_uiProductionMode;
	}

	void SkipBlocksNumber(unsigned int uiSkipBlocksNumber) {
		m_uiSkipBlocksNumber = uiSkipBlocksNumber;
		m_bChanged = true;
	}
	unsigned int& SkipBlocksNumber() {
		return m_uiSkipBlocksNumber;
	}

	void EraseAllFlash(bool bSet) {
		m_bEraseAllFlash = bSet;
		m_bChanged = true;
	}
	bool& EraseAllFlash() {
		return m_bEraseAllFlash;
	}

	void ResetBBT(bool bSet) {
		m_bResetBBT = bSet;
		m_bChanged = true;
	}
	bool& ResetBBT() {
		return m_bResetBBT;
	}

	bool ParseBlfHeader(CTimDescriptorLine*& pLine);

	void Reset();

	bool IsChanged() {
		return m_bChanged;
	}
	void Changed(bool bSet) {
		m_bChanged = bSet;
	}

	static CTimDescriptorLine* pPrevLine;

	CTimDescriptor* m_pTimDescriptor;

protected:

	// [BLF_Version]
	string m_sBlfVersion;

	// [UE_Options]
	unsigned int m_uiUEBootOption;

	// [Flash_Properties]
	unsigned int m_uiMaxUploadSplitSize;
	unsigned int m_uiMaxFBFSplitSize;
	string m_sFlashFamily;
	unsigned int m_uiSpareAreaSize;
	unsigned int m_uiDataAreaSize;
	unsigned int m_uiFBFSectorSize;

	// [Flash_Options]
	unsigned int m_uiProductionMode;
	unsigned int m_uiSkipBlocksNumber;
	bool m_bEraseAllFlash;
	bool m_bResetBBT;

	bool m_bChanged;
};

#endif //__BLF_HEADER_H__
