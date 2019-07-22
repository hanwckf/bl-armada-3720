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


#include "Key.h"

CKey::CKey(void) :
		CTimLib() {
	m_uiPublicKeySize = 0;
	m_uiKeySize = 0;
	m_bChanged = false;
	m_sEncryptAlgorithmId = "Marvell";
	m_sHashAlgorithmId = "SHA-160";
	m_bEncrypted = false;
}

CKey::~CKey(void) {
	CKey::DiscardAll();
}

CKey::CKey(const CKey& rhs) :
		CTimLib(rhs) {
	// copy constructor
	CKey& nc_rhs = const_cast<CKey&>(rhs);

	m_sKeyId = rhs.m_sKeyId;
	m_sKeyTag = rhs.m_sKeyTag;
	m_sHashAlgorithmId = rhs.m_sHashAlgorithmId;
	m_sEncryptAlgorithmId = rhs.m_sEncryptAlgorithmId;
	m_uiPublicKeySize = rhs.m_uiPublicKeySize;
	m_uiKeySize = rhs.m_uiKeySize;

	m_bEncrypted = rhs.m_bEncrypted;
	m_bChanged = rhs.m_bChanged;

}

CKey& CKey::operator=(const CKey& rhs) {
	// assignment operator
	if (&rhs != this) {
		CTimLib::operator=(rhs);

		CKey& nc_rhs = const_cast<CKey&>(rhs);
		CKey::DiscardAll();

		m_sKeyId = rhs.m_sKeyId;
		m_sKeyTag = rhs.m_sKeyTag;
		m_sHashAlgorithmId = rhs.m_sHashAlgorithmId;
		m_sEncryptAlgorithmId = rhs.m_sEncryptAlgorithmId;
		m_uiPublicKeySize = rhs.m_uiPublicKeySize;
		m_uiKeySize = rhs.m_uiKeySize;

		m_bEncrypted = rhs.m_bEncrypted;
		m_bChanged = rhs.m_bChanged;

	}
	return *this;
}

void CKey::DiscardAll() {

}

void CKey::KeyId(string& sKeyId) {
	sKeyId.resize(4);
	m_sKeyId = ToUpper(sKeyId, true);
	m_sKeyTag = TextToHexFormattedAscii(m_sKeyId);
	m_bChanged = true;
}

void CKey::KeyTag(string& sKeyTag) {
	m_sKeyTag = sKeyTag;
	UINT_T KeyID = Translate(sKeyTag);
	switch (KeyID) {
	case JTAGIDENTIFIER:
		m_sKeyId = "JTAG";
		break;
	case PATCHIDENTIFIER:
		m_sKeyId = "PATC";
		break;
	case TCAIDENTIFIER:
		m_sKeyId = "TCAK";
		break;
	case DSIGIDENTIFIER:
		m_sKeyId = "DSIG";
		break;
	case ENCKIDENTIFIER:
		m_sKeyId = "ENCK";
		break;
	case CSK0ID:
		m_sKeyId = "CSK0";
		break;
	case CSK1ID:
		m_sKeyId = "CSK1";
		break;
	case CSK2ID:
		m_sKeyId = "CSK2";
		break;
	case CSK3ID:
		m_sKeyId = "CSK3";
		break;
	case CSK4ID:
		m_sKeyId = "CSK4";
		break;
	case CSK5ID:
		m_sKeyId = "CSK5";
		break;
	case CSK6ID:
		m_sKeyId = "CSK6";
		break;
	case CSK7ID:
		m_sKeyId = "CSK7";
		break;
	case CSK8ID:
		m_sKeyId = "CSK8";
		break;
	case CSK9ID:
		m_sKeyId = "CSK9";
		break;
	case CSKAID:
		m_sKeyId = "CSKA";
		break;
	case CSKBID:
		m_sKeyId = "CSKB";
		break;
	case CSKCID:
		m_sKeyId = "CSKC";
		break;
	case CSKDID:
		m_sKeyId = "CSKD";
		break;
	case CSKEID:
		m_sKeyId = "CSKE";
		break;
	case CSKFID:
		m_sKeyId = "CSKF";
		break;
	default: {
		string sCvt = HexAsciiToText(m_sKeyTag);
		if (sCvt != m_sKeyTag)
			m_sKeyId = HexAsciiToText(m_sKeyTag); // custom
		else
			m_sKeyId = m_sKeyTag;
	}
	}

	m_bChanged = true;
}

void CKey::HashAlgorithmId(HASHALGORITHMID_T Id) {
	switch ((int) Id) {
	case 2:
	case 32:
	case 256:
		Id = HASHALGORITHMID_T::SHA256;
		break;

	case 64:
	case 512:
		Id = HASHALGORITHMID_T::SHA512;
		break;

	case 1:
	case 160:
	case 20:
		Id = HASHALGORITHMID_T::SHA160;
		break;

	default:
		printf("\nUnrecognized Hash Algorithm Id. Defaulting to SHA-160.\n");
		Id = SHA160;
		break;
	}

	switch (Id) {
	case HASHALGORITHMID_T::SHA256:
		m_sHashAlgorithmId = "SHA-256";
		break;

	case HASHALGORITHMID_T::SHA512:
		m_sHashAlgorithmId = "SHA-512";
		break;

	case HASHALGORITHMID_T::SHA160:
	default:
		m_sHashAlgorithmId = "SHA-160";
		break;
	}
}

void CKey::EncryptAlgorithmId(ENCRYPTALGORITHMID_T Id, bool bIsBlf) {
	m_bEncrypted = false;
	if ((Id & EncryptedBitMask) > 0) {
		m_bEncrypted = true;
		Id = (ENCRYPTALGORITHMID_T) (Id & ~EncryptedBitMask);
	}

	switch (Id) {
	case PKCS1_v1_5_Caddo:
		m_sEncryptAlgorithmId =
				bIsBlf ? "PKCS1_v1_5_Caddo" : "PKCS1_v1_5 (Caddo)";
		break;

	case PKCS1_v1_5_Ippcp:
		m_sEncryptAlgorithmId =
				bIsBlf ? "PKCS1_v1_5_Ippcp" : "PKCS1_v1_5 (Ippcp)";
		break;

	case PKCS_v2_2_Ippcp:
		m_sEncryptAlgorithmId =
				bIsBlf ? "PKCS_v2_2_Ippcp" : "PKCS_v2_2 (Ippcp)";
		break;

	case ECDSA_256:
		m_sEncryptAlgorithmId = "ECDSA_256";
		break;

	case ECDSA_521:
		m_sEncryptAlgorithmId = "ECDSA_521";
		break;

	case Marvell_DS:
		m_sEncryptAlgorithmId = "Marvell";
		break;

	default:
		m_sEncryptAlgorithmId = "";
		printf("\nError: Unrecognized Encryption Algorithm ID.\n");
		break;
	}
}

string& CKey::EncryptAlgorithmIdStr(bool bIsBlf) {
	// make sure m_sEncryptAlgorithmId is updated in case bIsBlf has changed
	EncryptAlgorithmId(EncryptAlgorithmId(), bIsBlf);
	// return the correct string
	return m_sEncryptAlgorithmId;
}

unsigned int CKey::ActiveSizeOfList(t_stringList& List) {
	// returns a size excluding all trailing 0x00000000 values

	// get bit size of entire list
	unsigned int uiSize = ((int) List.size()) * 32;
	string sItem;

	if (uiSize > 0) {
		t_stringListIter iter = List.end();
		while (--iter != List.begin()) {
			sItem = *(*iter);
			if (ToUpper(sItem, true) == ToUpper("0x00000000"))
				uiSize -= 32;
			else
				break;
		}
	}
	return uiSize;
}

bool CKey::IsChanged() {
	return m_bChanged;
}

ENCRYPTALGORITHMID_T CKey::EncryptAlgorithmId() {
	ENCRYPTALGORITHMID_T eEncryptAlgoId = DUMMY_ENALG;
	if (m_sEncryptAlgorithmId == "PKCS1_v1_5 (Caddo)"
			|| m_sEncryptAlgorithmId == "PKCS1_v1_5_Caddo")
		eEncryptAlgoId = PKCS1_v1_5_Caddo;
	else if (m_sEncryptAlgorithmId == "PKCS1_v1_5 (Ippcp)"
			|| m_sEncryptAlgorithmId == "PKCS1_v1_5_Ippcp")
		eEncryptAlgoId = PKCS1_v1_5_Ippcp;
	else if (m_sEncryptAlgorithmId == "PKCS_v2_2 (Ippcp)"
			|| m_sEncryptAlgorithmId == "PKCS_v2_2_Ippcp")
		eEncryptAlgoId = PKCS_v2_2_Ippcp;
	else if (m_sEncryptAlgorithmId == "ECDSA_256")
		eEncryptAlgoId = ECDSA_256;
	else if (m_sEncryptAlgorithmId == "ECDSA_521")
		eEncryptAlgoId = ECDSA_521;

	return eEncryptAlgoId;
}

HASHALGORITHMID_T CKey::HashAlgorithmId() {
	HASHALGORITHMID_T eHashAlgoId = HASHALGORITHMID_T::SHA160;
	if (HashAlgorithmIdStr() == "SHA-160")
		eHashAlgoId = HASHALGORITHMID_T::SHA160;
	else if (HashAlgorithmIdStr() == "SHA-256")
		eHashAlgoId = HASHALGORITHMID_T::SHA256;
	else if (HashAlgorithmIdStr() == "SHA-512")
		eHashAlgoId = HASHALGORITHMID_T::SHA512;

	return eHashAlgoId;
}

