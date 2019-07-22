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
#include <cstdlib>
#include <cstring>

#include "RSAKey.h"

CRSAKey::CRSAKey(void) :
		CKey() {
	m_sEncryptAlgorithmId = "PKCS1_v1_5 (Ippcp)";
	KeySize(1024);
	m_bChanged = false;
}

CRSAKey::~CRSAKey(void) {
	DiscardAll();
}

CRSAKey::CRSAKey(const CRSAKey& rhs) :
		CKey(rhs) {
	// copy constructor
	CRSAKey& nc_rhs = const_cast<CRSAKey&>(rhs);

	m_sPublicKeyExponent = rhs.m_sPublicKeyExponent;
	m_sRsaSystemModulus = rhs.m_sRsaSystemModulus;
	m_sRsaPrivateKey = rhs.m_sRsaPrivateKey;

	// need to do a deep copy of lists to avoid dangling references
	t_stringListIter iterExponent = nc_rhs.m_PublicKeyExponentList.begin();
	while (iterExponent != nc_rhs.m_PublicKeyExponentList.end()) {
		string* pData = new string(*(*iterExponent));
		m_PublicKeyExponentList.push_back(pData);
		iterExponent++;
	}

	t_stringListIter iterModulus = nc_rhs.m_RsaSystemModulusList.begin();
	while (iterModulus != nc_rhs.m_RsaSystemModulusList.end()) {
		string* pData = new string(*(*iterModulus));
		m_RsaSystemModulusList.push_back(pData);
		iterModulus++;
	}

	t_stringListIter iterDS = nc_rhs.m_RsaPrivateKeyList.begin();
	while (iterDS != nc_rhs.m_RsaPrivateKeyList.end()) {
		string* pData = new string(*(*iterDS));
		m_RsaPrivateKeyList.push_back(pData);
		iterDS++;
	}
}

CRSAKey& CRSAKey::operator=(const CRSAKey& rhs) {
	// assignment operator
	if (&rhs != this) {
		CKey::operator=(rhs);

		CRSAKey& nc_rhs = const_cast<CRSAKey&>(rhs);
		CRSAKey::DiscardAll();

		m_sPublicKeyExponent = rhs.m_sPublicKeyExponent;
		m_sRsaSystemModulus = rhs.m_sRsaSystemModulus;
		m_sRsaPrivateKey = rhs.m_sRsaPrivateKey;

		// need to do a deep copy of lists to avoid dangling references
		t_stringListIter iterExponent = nc_rhs.m_PublicKeyExponentList.begin();
		while (iterExponent != nc_rhs.m_PublicKeyExponentList.end()) {
			string* pData = new string(*(*iterExponent));
			m_PublicKeyExponentList.push_back(pData);
			iterExponent++;
		}

		t_stringListIter iterModulus = nc_rhs.m_RsaSystemModulusList.begin();
		while (iterModulus != nc_rhs.m_RsaSystemModulusList.end()) {
			string* pData = new string(*(*iterModulus));
			m_RsaSystemModulusList.push_back(pData);
			iterModulus++;
		}

		// need to do a deep copy of lists to avoid dangling references
		t_stringListIter iterDS = nc_rhs.m_RsaPrivateKeyList.begin();
		while (iterDS != nc_rhs.m_RsaPrivateKeyList.end()) {
			string* pData = new string(*(*iterDS));
			m_RsaPrivateKeyList.push_back(pData);
			iterDS++;
		}
	}
	return *this;
}

CKey* CRSAKey::newCopy() {
	CRSAKey* pNewCopy = new CRSAKey(*this);
	return pNewCopy;
}

void CRSAKey::DiscardAll() {
	// delete all exponent strings
	if (m_PublicKeyExponentList.size() > 0) {
		for_each(m_PublicKeyExponentList.begin(), m_PublicKeyExponentList.end(),
				[](string*& ps) {delete ps;});
		m_PublicKeyExponentList.clear();
	}

	// delete all modulus strings
	if (m_RsaSystemModulusList.size() > 0) {
		for_each(m_RsaSystemModulusList.begin(), m_RsaSystemModulusList.end(),
				[](string*& ps) {delete ps;});
		m_RsaSystemModulusList.clear();
	}

	// delete all RsaPrivateKeyList strings
	if (m_RsaPrivateKeyList.size() > 0) {
		for_each(m_RsaPrivateKeyList.begin(), m_RsaPrivateKeyList.end(),
				[](string*& ps) {delete ps;});
		m_RsaPrivateKeyList.clear();
	}
}

void CRSAKey::ResetExponent() {
	t_stringListIter iterExponent = m_PublicKeyExponentList.begin();
	while (iterExponent != m_PublicKeyExponentList.end()) {
		*(*iterExponent) = "0x00000000";
		iterExponent++;
	}

	m_bChanged = true;
}

void CRSAKey::ResetModulus() {
	t_stringListIter iterModulus = m_RsaSystemModulusList.begin();
	while (iterModulus != m_RsaSystemModulusList.end()) {
		*(*iterModulus) = "0x00000000";
		iterModulus++;
	}

	m_bChanged = true;
}

void CRSAKey::ResetPrivateKey() {
	t_stringListIter iterDS = m_RsaPrivateKeyList.begin();
	while (iterDS != m_RsaPrivateKeyList.end()) {
		*(*iterDS) = "0x00000000";
		iterDS++;
	}

	m_bChanged = true;
}

string& CRSAKey::PublicKeyExponent(bool bIsBlf) {
	// returns a complete list of values, including all trailing 0x00000000
	m_sPublicKeyExponent = "";
	int nPerLine = 0;

	stringstream ssEntryNum;
	int nEntry = 0;

	t_stringListIter iterExponent = m_PublicKeyExponentList.begin();
	while (iterExponent != m_PublicKeyExponentList.end()) {
		if (bIsBlf) {
			nEntry++;
			ssEntryNum.str("");
			ssEntryNum << "#" << nEntry << " = ";

			m_sPublicKeyExponent += ssEntryNum.str();
			m_sPublicKeyExponent += *(*iterExponent);
			m_sPublicKeyExponent += "\n";
		} else {
			m_sPublicKeyExponent += *(*iterExponent);
			nPerLine++;
			if ((nPerLine != 0) && (nPerLine % 4 == 0))
				m_sPublicKeyExponent += "\n";  // 4 is # of words per line
			else
				m_sPublicKeyExponent += " ";
		}
		iterExponent++;
	}
	// handle the case where the final line has less than 4 words
	if (!bIsBlf && (m_PublicKeyExponentList.size() % 4 != 0))
		m_sPublicKeyExponent += "\n";

	return m_sPublicKeyExponent;
}

string& CRSAKey::PublicKeyExponentPacked(bool bIsBlf) {
	// returns a truncated list of values, excluding all trailing 0x00000000
	m_sPublicKeyExponent = "";
	int nPerLine = 0;

	unsigned int uiActiveSize = ActiveSizeOfList(m_PublicKeyExponentList);

	stringstream ssEntryNum;
	int nEntry = 0;

	t_stringListIter iterExponent = m_PublicKeyExponentList.begin();
	while (iterExponent != m_PublicKeyExponentList.end() && uiActiveSize > 0) {
		uiActiveSize -= 32;

		if (bIsBlf) {
			nEntry++;
			ssEntryNum.str("");
			ssEntryNum << "#" << nEntry << " = ";

			m_sPublicKeyExponent += ssEntryNum.str();
			m_sPublicKeyExponent += *(*iterExponent);
			m_sPublicKeyExponent += "\n";
		} else {
			m_sPublicKeyExponent += *(*iterExponent);
			nPerLine++;
			if (((nPerLine != 0) && (nPerLine % 4 == 0)) || uiActiveSize == 0) // 4 is # of words per line
				m_sPublicKeyExponent += "\n";
			else
				m_sPublicKeyExponent += " ";
		}
		iterExponent++;
	}
	// handle the case where the final line has less than 4 words
	if (!bIsBlf && (uiActiveSize % 4 != 0))
		m_sPublicKeyExponent += "\n";

	return m_sPublicKeyExponent;
}

string& CRSAKey::RsaSystemModulus(bool bIsBlf) {
	// returns a complete list of values, including all trailing 0x00000000
	m_sRsaSystemModulus = "";
	int nPerLine = 0;

	stringstream ssEntryNum;
	int nEntry = 0;

	t_stringListIter iterModulus = m_RsaSystemModulusList.begin();
	while (iterModulus != m_RsaSystemModulusList.end()) {
		if (bIsBlf) {
			nEntry++;
			ssEntryNum.str("");
			ssEntryNum << "#" << nEntry << " = ";

			m_sRsaSystemModulus += ssEntryNum.str();
			m_sRsaSystemModulus += *(*iterModulus);
			m_sRsaSystemModulus += "\n";
		} else {
			m_sRsaSystemModulus += *(*iterModulus);
			nPerLine++;
			if ((nPerLine != 0) && (nPerLine % 4 == 0)) // 4 is # of words per line
				m_sRsaSystemModulus += "\n";
			else
				m_sRsaSystemModulus += " ";
		}
		iterModulus++;
	}
	// handle the case where the final line has less than 4 words
	if (!bIsBlf && (m_RsaSystemModulusList.size() % 4 != 0))
		m_sRsaSystemModulus += "\n";

	return m_sRsaSystemModulus;
}

string& CRSAKey::RsaSystemModulusPacked(bool bIsBlf) {
	// returns a truncated list of values, excluding all trailing 0x00000000
	m_sRsaSystemModulus = "";
	int nPerLine = 0;

	unsigned int uiActiveSize = ActiveSizeOfList(m_RsaSystemModulusList);

	stringstream ssEntryNum;
	int nEntry = 0;

	t_stringListIter iterModulus = m_RsaSystemModulusList.begin();
	while (iterModulus != m_RsaSystemModulusList.end() && uiActiveSize > 0) {
		uiActiveSize -= 32;

		if (bIsBlf) {
			nEntry++;
			ssEntryNum.str("");
			ssEntryNum << "#" << nEntry << " = ";

			m_sRsaSystemModulus += ssEntryNum.str();
			m_sRsaSystemModulus += *(*iterModulus);
			m_sRsaSystemModulus += "\n";
		} else {
			m_sRsaSystemModulus += *(*iterModulus);
			nPerLine++;
			if (((nPerLine != 0) && (nPerLine % 4 == 0)) || uiActiveSize == 0) // 4 is # of words per line
				m_sRsaSystemModulus += "\n";
			else
				m_sRsaSystemModulus += " ";
		}
		iterModulus++;
	}
	// handle the case where the final line has less than 4 words
	if (!bIsBlf && (uiActiveSize % 4 != 0))
		m_sRsaSystemModulus += "\n";

	return m_sRsaSystemModulus;
}

string& CRSAKey::RsaPrivateKey(bool bIsBlf) {
	m_sRsaPrivateKey = "";
	int nPerLine = 0;

	stringstream ssEntryNum;
	int nEntry = 0;

	t_stringListIter iterDS = m_RsaPrivateKeyList.begin();
	while (iterDS != m_RsaPrivateKeyList.end()) {
		if (bIsBlf) {
			nEntry++;
			ssEntryNum.str("");
			ssEntryNum << "#" << nEntry << " = ";

			m_sRsaPrivateKey += ssEntryNum.str();
			m_sRsaPrivateKey += *(*iterDS);
			m_sRsaPrivateKey += "\n";
		} else {
			m_sRsaPrivateKey += *(*iterDS);
			nPerLine++;
			if ((nPerLine != 0) && (nPerLine % 4 == 0)) // 4 is # of words per line
				m_sRsaPrivateKey += "\n";
			else
				m_sRsaPrivateKey += " ";
		}
		iterDS++;
	}
	// handle the case where the final line has less than 4 words
	if (!bIsBlf && (m_RsaPrivateKeyList.size() % 4 != 0))
		m_sRsaPrivateKey += "\n";

	return m_sRsaPrivateKey;
}

string& CRSAKey::RsaPrivateKeyPacked(bool bIsBlf) {
	m_sRsaPrivateKey = "";
	int nPerLine = 0;

	unsigned int uiActiveSize = ActiveSizeOfList(m_RsaPrivateKeyList);

	stringstream ssEntryNum;
	int nEntry = 0;

	t_stringListIter iterDS = m_RsaPrivateKeyList.begin();
	while (iterDS != m_RsaPrivateKeyList.end() && uiActiveSize > 0) {
		uiActiveSize -= 32;

		if (bIsBlf) {
			nEntry++;
			ssEntryNum.str("");
			ssEntryNum << "#" << nEntry << " = ";

			m_sRsaPrivateKey += ssEntryNum.str();
			m_sRsaPrivateKey += *(*iterDS);
			m_sRsaPrivateKey += "\n";
		} else {
			m_sRsaPrivateKey += *(*iterDS);
			if (++nPerLine % 4 == 0 || uiActiveSize == 0) // 4 is # of words per line
					{
				m_sRsaPrivateKey += "\n";
			} else
				m_sRsaPrivateKey += " ";
		}
		iterDS++;
	}
	// handle the case where the final line has less than 4 words
	if (!bIsBlf && (uiActiveSize % 4 != 0))
		m_sRsaPrivateKey += "\n";

	return m_sRsaPrivateKey;
}

void CRSAKey::PublicKeySize(unsigned int uiPublicKeySize) {
	// changing the size results in expansion or reduction in exponent and modulus
	ResizeList(m_PublicKeyExponentList, ((uiPublicKeySize + 31) / 32),
			m_bChanged);

	m_uiPublicKeySize = uiPublicKeySize;
}

void CRSAKey::KeySize(unsigned int uiKeySize) {
	// changing the size results in expansion or reduction in exponent and modulus
	ResizeList(m_PublicKeyExponentList, ((uiKeySize + 31) / 32), m_bChanged);
	ResizeList(m_RsaSystemModulusList, ((uiKeySize + 31) / 32), m_bChanged);
	ResizeList(m_RsaPrivateKeyList, ((uiKeySize + 31) / 32), m_bChanged);

	m_uiKeySize = uiKeySize;
	PublicKeySize(m_uiKeySize);
}

bool CRSAKey::FromKeyInfo_3_4(KEY_MOD_3_4_0& stKey) {
	string sKeyId = HexAsciiToText(HexFormattedAscii(stKey.KeyID));
	KeyId(sKeyId);
	EncryptAlgorithmId((ENCRYPTALGORITHMID_T) stKey.EncryptAlgorithmID);
	if (m_sEncryptAlgorithmId == "") {
		cout << endl << "Error: Unrecognized Encryption Algorithm ID" << endl;
		return false;
	}

	HashAlgorithmId((HASHALGORITHMID_T) stKey.HashAlgorithmID);
	KeySize(stKey.KeySize);

	if (!ToListFromArray(stKey.Rsa.RSAModulus, m_RsaSystemModulusList,
			((m_uiKeySize + 31) / 32)))
		return false;

	if (!ToListFromArray(stKey.Rsa.RSAPublicExponent, m_PublicKeyExponentList,
			((m_uiKeySize + 31) / 32)))
		return false;

	return true;
}

KEY_MOD_3_4_0 CRSAKey::ToKeyInfo_3_4() {
	KEY_MOD_3_4_0 stKey;
	memset(&stKey, 0, sizeof(stKey));

	stKey.KeyID = Translate(m_sKeyTag);
	// the encrypted flag is added to the EncryptAlgorithmId at the end of this function
	stKey.EncryptAlgorithmID = EncryptAlgorithmId();
	stKey.HashAlgorithmID = HashAlgorithmId();
	stKey.KeySize = m_uiKeySize;
	stKey.PublicKeySize = m_uiPublicKeySize;

	unsigned int i = 0;

	ToArrayFromList(m_RsaSystemModulusList,
			stKey.EncryptedRsa.EncryptedHashRSAModulus,
			(m_uiKeySize + 31) / 32);
	ToArrayFromList(m_PublicKeyExponentList,
			stKey.EncryptedRsa.EncryptedHashRSAPublicExponent,
			(m_uiKeySize + 31) / 32);

	if (Encrypted())
		stKey.EncryptAlgorithmID =
				(ENCRYPTALGORITHMID_T) (stKey.EncryptAlgorithmID
						| EncryptedBitMask);

	return stKey;
}

bool CRSAKey::FromPlatDs(PLAT_DS& stDs) {
	EncryptAlgorithmId((ENCRYPTALGORITHMID_T) stDs.DSAlgorithmID);
	if (m_sEncryptAlgorithmId == "") {
		cout << endl << "Error: Unrecognized Encryption Algorithm ID" << endl;
		return false;
	}

	KeySize(stDs.KeySize);
	HashAlgorithmId((HASHALGORITHMID_T) stDs.HashAlgorithmID);

	if (!ToListFromArray(stDs.Rsa.RSAModulus, m_RsaSystemModulusList,
			((m_uiKeySize + 31) / 32)))
		return false;

	if (!ToListFromArray(stDs.Rsa.RSAPublicExponent, m_PublicKeyExponentList,
			((m_uiKeySize + 31) / 32)))
		return false;

	return true;
}

PLAT_DS CRSAKey::toPlatDs() {
	PLAT_DS stDs;
	memset(&stDs, 0, sizeof(stDs));

	stDs.KeySize = m_uiKeySize;
	stDs.DSAlgorithmID = EncryptAlgorithmId();
	stDs.HashAlgorithmID = HashAlgorithmId();

	ToArrayFromList(m_RsaSystemModulusList, stDs.Rsa.RSAModulus,
			(m_uiKeySize + 31) / 32);

	ToArrayFromList(m_PublicKeyExponentList, stDs.Rsa.RSAPublicExponent,
			(m_uiKeySize + 31) / 32);

	return stDs;
}

bool CRSAKey::PrivateFromArray(UINT* pPrivateAry, unsigned int size) {
	if (!ToListFromArray(pPrivateAry, m_RsaPrivateKeyList, size))
		return false;

	return true;
}

bool CRSAKey::PrivateToArray(UINT* pPrivateAry, unsigned int size) {
	if (!ToArrayFromList(m_RsaPrivateKeyList, pPrivateAry, size))
		return false;

	return true;
}

bool CRSAKey::PubCompDFromArray(UINT* pCompDAry, unsigned int size) {
	if (!ToListFromArray(pCompDAry, m_RsaSystemModulusList, size))
		return false;

	return true;
}

bool CRSAKey::PubCompDToArray(UINT* pCompDAry, unsigned int size) {
	if (!ToArrayFromList(m_RsaSystemModulusList, pCompDAry, size))
		return false;

	return true;
}

bool CRSAKey::PubCompEFromArray(UINT* pCompEAry, unsigned int size) {
	if (!ToListFromArray(pCompEAry, m_PublicKeyExponentList, size))
		return false;

	return true;
}

bool CRSAKey::PubCompEToArray(UINT* pCompEAry, unsigned int size) {
	if (!ToArrayFromList(m_PublicKeyExponentList, pCompEAry, size))
		return false;

	return true;
}

int CRSAKey::RSAKeyGen(RSA_KEY_TYPE RSAtype, unsigned int*& pBufN, int *szN,
		unsigned int*& pBufE, int *szE,
		unsigned int*& pBufD, int *szD, unsigned int*& pBufP, int *szP, unsigned int*& pBufQ,
		int *szQ) {
	int size;
	unsigned int publicKeyArray[MAXRSAKEYSIZEWORDS];
	unsigned int privateKeyArray[MAXRSAKEYSIZEWORDS];
	unsigned int modulusArray[MAXRSAKEYSIZEWORDS];

	CryptoPP::AutoSeededRandomPool rng;
	CryptoPP::RSA::PrivateKey privateKey;
	CryptoPP::Integer pubModulus, pubExponent, privExponent;
	switch (RSAtype) {
	case RSA_KEY_TYPE::RSA1024:
		size = 1024;
		break;
	case RSA_KEY_TYPE::RSA2048:
		size = 2048;
		break;
	default:
		return false;
	}

	privateKey.GenerateRandomWithKeySize(rng, size);
	if (!privateKey.Validate(rng, 3))
		return false;

	unsigned int *publicKeyArraySwapped = new unsigned int[size / 32];
	unsigned int *privateKeyArraySwapped = new unsigned int[size / 32];
	unsigned int *modulusArraySwapped = new unsigned int[size / 32];

	pubExponent = privateKey.GetPublicExponent();
	privExponent = privateKey.GetPrivateExponent();
	pubModulus = privateKey.GetModulus();

	pubExponent.Encode((byte *) publicKeyArray, MAXRSAKEYSIZEWORDS * 4,
			CryptoPP::Integer::UNSIGNED);
	privExponent.Encode((byte *) privateKeyArray, MAXRSAKEYSIZEWORDS * 4,
			CryptoPP::Integer::UNSIGNED);
	pubModulus.Encode((byte *) modulusArray, MAXRSAKEYSIZEWORDS * 4,
			CryptoPP::Integer::UNSIGNED);

	for (int i = 0; i < size / 32; ++i) {
		publicKeyArraySwapped[i] = __builtin_bswap32(
				publicKeyArray[MAXRSAKEYSIZEWORDS - i - 1]);
		privateKeyArraySwapped[i] = __builtin_bswap32(
				privateKeyArray[MAXRSAKEYSIZEWORDS - i - 1]);
		modulusArraySwapped[i] = __builtin_bswap32(
				modulusArray[MAXRSAKEYSIZEWORDS - i - 1]);
	}

	pBufN = modulusArraySwapped;
	pBufE = publicKeyArraySwapped;
	pBufD = privateKeyArraySwapped;

	*szD = size / 32;
	*szE = size / 32;
	*szN = size / 32;

	return true;
}

