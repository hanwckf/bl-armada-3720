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

#include "CTIM.h"
#include "TimLib.h"
#include <time.h>

CCTIM::CCTIM() {
}

CCTIM::~CCTIM() {
}

int CCTIM::getSize() {
	if (m_TimHeader.VersionFlash3_5.VersionBind.Version >= TIM_3_6_00)
		return sizeof(VERSION_I_3_6) + sizeof(FLASH_I_3_6) + sizeof(UINT_T) * 3;
	return sizeof(VERSION_I) + sizeof(FLASH_I) + sizeof(UINT_T) * 3;

}

void CCTIM::setVersionStr(string& sVersion) {
	m_TimHeader.VersionFlash3_5.VersionBind.Version = CTimLib::Translate(
			sVersion);
}

string& CCTIM::getVersionStr() {
	return CTimLib::HexFormattedAscii(
			m_TimHeader.VersionFlash3_5.VersionBind.Version);
}

void CCTIM::setTrusted(UINT_T bSet) {
	m_TimHeader.VersionFlash3_5.VersionBind.Trusted = (int) bSet;
}
UINT_T& CCTIM::getTrusted() {
	return m_TimHeader.VersionFlash3_5.VersionBind.Trusted;
}

void CCTIM::setTrustedStr(string& sTrusted) {
	m_TimHeader.VersionFlash3_5.VersionBind.Trusted = CTimLib::Translate(
			sTrusted);
}
string& CCTIM::getTrustedStr() {
	return CTimLib::HexFormattedAscii(
			m_TimHeader.VersionFlash3_5.VersionBind.Trusted);
}

void CCTIM::setIssueDateStr(string& sIssueDate) {
	m_TimHeader.VersionFlash3_5.VersionBind.IssueDate = CTimLib::Translate(
			sIssueDate);
}
string& CCTIM::getIssueDateStr() {
	return CTimLib::HexFormattedAscii(
			m_TimHeader.VersionFlash3_5.VersionBind.IssueDate);
}

void CCTIM::setOEMUniqueIdStr(string& sOemUniqueId) {
	m_TimHeader.VersionFlash3_5.VersionBind.OEMUniqueID = CTimLib::Translate(
			sOemUniqueId);
}
string& CCTIM::getOEMUniqueIdStr() {
	return CTimLib::HexFormattedAscii(
			m_TimHeader.VersionFlash3_5.VersionBind.OEMUniqueID);
}

void CCTIM::setBootRomFlashSignatureStr(string& sBootRomFlashSignature) {
	m_TimHeader.VersionFlash3_5.FlashInfo.BootFlashSign = CTimLib::Translate(
			sBootRomFlashSignature);
}
string& CCTIM::getBootRomFlashSignatureStr() {
	return CTimLib::HexFormattedAscii(
			m_TimHeader.VersionFlash3_5.FlashInfo.BootFlashSign);
}

void CCTIM::setSteppingStr(string& sStepping) {
	m_TimHeader.VersionFlash3_6.VersionBind.Stepping = CTimLib::Translate(
			sStepping);
}
string& CCTIM::getSteppingStr() {
	return CTimLib::HexFormattedAscii(
			m_TimHeader.VersionFlash3_6.VersionBind.Stepping);
}

void CCTIM::setWtmSaveStateFlashSignatureStr(string& sSignature) {
	m_TimHeader.VersionFlash3_5.FlashInfo.WTMFlashSign = CTimLib::Translate(
			sSignature);
}
string& CCTIM::getWtmSaveStateFlashSignatureStr() {
	return CTimLib::HexFormattedAscii(
			m_TimHeader.VersionFlash3_5.FlashInfo.WTMFlashSign);
}

void CCTIM::setWtmSaveStateFlashEntryAddressStr(string& sAddress) {
	m_TimHeader.VersionFlash3_5.FlashInfo.WTMEntryAddr = CTimLib::Translate(
			sAddress);
}
string& CCTIM::getWtmSaveStateFlashEntryAddressStr() {
	return CTimLib::HexFormattedAscii(
			m_TimHeader.VersionFlash3_5.FlashInfo.WTMEntryAddr);
}

void CCTIM::setWtmSaveStateBackupEntryAddressStr(string& sAddress) {
	m_TimHeader.VersionFlash3_5.FlashInfo.WTMEntryAddrBack = CTimLib::Translate(
			sAddress);
}
string& CCTIM::getWtmSaveStateBackupEntryAddressStr() {
	return CTimLib::HexFormattedAscii(
			m_TimHeader.VersionFlash3_5.FlashInfo.WTMEntryAddrBack);
}

void CCTIM::setWtmSaveStatePatchSignatureStr(string& sAddress) {
	m_TimHeader.VersionFlash3_5.FlashInfo.WTMPatchSign = CTimLib::Translate(
			sAddress);
}
string& CCTIM::getWtmSaveStatePatchSignatureStr() {
	return CTimLib::HexFormattedAscii(
			m_TimHeader.VersionFlash3_5.FlashInfo.WTMPatchSign);
}

void CCTIM::setWtmSaveStatePatchAddressStr(string& sAddress) {
	m_TimHeader.VersionFlash3_5.FlashInfo.WTMPatchAddr = CTimLib::Translate(
			sAddress);
}
string& CCTIM::getWtmSaveStatePatchAddressStr() {
	return CTimLib::HexFormattedAscii(
			m_TimHeader.VersionFlash3_5.FlashInfo.WTMPatchAddr);
}
void CCTIM::setVersion(UINT_T Version) {
	if (Version < TIM_3_6_00) {
		m_TimHeader.VersionFlash3_5.VersionBind.Version = Version;
		m_TimHeader.VersionFlash3_5.FlashInfo.WTMEntryAddr = 0xFFFFFFFF;
		m_TimHeader.VersionFlash3_5.FlashInfo.WTMEntryAddrBack = 0xFFFFFFFF;
		m_TimHeader.VersionFlash3_5.FlashInfo.WTMFlashSign = 0xFFFFFFFF;
		m_TimHeader.VersionFlash3_5.FlashInfo.WTMPatchAddr = 0xFFFFFFFF;
		m_TimHeader.VersionFlash3_5.FlashInfo.WTMPatchSign = 0xFFFFFFFF;
	} else {
		m_TimHeader.VersionFlash3_6.VersionBind.Version = Version;
		m_TimHeader.VersionFlash3_6.FlashInfo.Reserved1 = 0xFFFFFFFF;
		m_TimHeader.VersionFlash3_6.FlashInfo.Reserved2 = 0xFFFFFFFF;
		m_TimHeader.VersionFlash3_6.FlashInfo.Reserved3 = 0xFFFFFFFF;
		m_TimHeader.VersionFlash3_6.FlashInfo.Reserved4 = 0xFFFFFFFF;
	}

}

UINT_T& CCTIM::getVersion() {
	return m_TimHeader.VersionFlash3_5.VersionBind.Version;
}

void CCTIM::setIssueDate(UINT_T sIssueDate) {
	m_TimHeader.VersionFlash3_5.VersionBind.IssueDate = sIssueDate;
}
UINT_T& CCTIM::getIssueDate() {
	return m_TimHeader.VersionFlash3_5.VersionBind.IssueDate;
}

void CCTIM::setOEMUniqueId(UINT_T sOemUniqueId) {
	m_TimHeader.VersionFlash3_5.VersionBind.OEMUniqueID = sOemUniqueId;
}
UINT_T& CCTIM::getOEMUniqueId() {
	return m_TimHeader.VersionFlash3_5.VersionBind.OEMUniqueID;
}

void CCTIM::setBootRomFlashSignature(UINT_T sBootRomFlashSignature) {
	m_TimHeader.VersionFlash3_5.FlashInfo.BootFlashSign =
			sBootRomFlashSignature;
}
UINT_T& CCTIM::getBootRomFlashSignature() {
	return m_TimHeader.VersionFlash3_5.FlashInfo.BootFlashSign;
}

void CCTIM::setStepping(UINT_T sStepping) {
	m_TimHeader.VersionFlash3_6.VersionBind.Stepping = sStepping;
}
UINT_T& CCTIM::getStepping() {
	return m_TimHeader.VersionFlash3_6.VersionBind.Stepping;
}

void CCTIM::setWtmSaveStateFlashSignature(UINT_T sSignature) {
	m_TimHeader.VersionFlash3_5.FlashInfo.WTMFlashSign = sSignature;
}
UINT_T& CCTIM::getWtmSaveStateFlashSignature() {
	return m_TimHeader.VersionFlash3_5.FlashInfo.WTMFlashSign;
}

void CCTIM::setWtmSaveStateFlashEntryAddress(UINT_T sAddress) {
	m_TimHeader.VersionFlash3_5.FlashInfo.WTMEntryAddr = sAddress;
}
UINT_T& CCTIM::getWtmSaveStateFlashEntryAddress() {
	return m_TimHeader.VersionFlash3_5.FlashInfo.WTMEntryAddr;
}

void CCTIM::setWtmSaveStateBackupEntryAddress(UINT_T sAddress) {
	m_TimHeader.VersionFlash3_5.FlashInfo.WTMEntryAddrBack = sAddress;
}
UINT_T& CCTIM::getWtmSaveStateBackupEntryAddress() {
	return m_TimHeader.VersionFlash3_5.FlashInfo.WTMEntryAddrBack;
}

void CCTIM::setWtmSaveStatePatchSignature(UINT_T sAddress) {
	m_TimHeader.VersionFlash3_5.FlashInfo.WTMPatchSign = sAddress;
}
UINT_T& CCTIM::getWtmSaveStatePatchSignature() {
	return m_TimHeader.VersionFlash3_5.FlashInfo.WTMPatchSign;
}

void CCTIM::setWtmSaveStatePatchAddress(UINT_T sAddress) {
	m_TimHeader.VersionFlash3_5.FlashInfo.WTMPatchAddr = sAddress;
}
UINT_T& CCTIM::getWtmSaveStatePatchAddress() {
	return m_TimHeader.VersionFlash3_5.FlashInfo.WTMPatchAddr;
}

void CCTIM::setNumImages(UINT_T numImages) {
	m_TimHeader.NumImages = numImages;
}
UINT_T& CCTIM::getNumImages() {
	return m_TimHeader.NumImages;
}

void CCTIM::setNumKeys(UINT_T numKeys) {
	m_TimHeader.NumKeys = numKeys;
}
UINT_T& CCTIM::getNumKeys() {
	return m_TimHeader.NumKeys;
}

void CCTIM::setSizeOfReserved(UINT_T sizeOfReserved) {
	m_TimHeader.SizeOfReserved = sizeOfReserved;
}
UINT_T& CCTIM::getSizeOfReserved() {
	return m_TimHeader.SizeOfReserved;
}

void CCTIM::setTimHeader(CTIM& TimHeader) {
	memcpy((void *) &this->m_TimHeader, (void *) &TimHeader, getSize());
}

void CCTIM::setIdentifier(UINT_T Identifier) {
	m_TimHeader.VersionFlash3_5.VersionBind.Identifier = Identifier;
}
UINT_T& CCTIM::getIdentifier() {
	return m_TimHeader.VersionFlash3_5.VersionBind.Identifier;
}
