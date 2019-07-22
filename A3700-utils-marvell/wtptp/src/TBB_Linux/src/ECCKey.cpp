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


#include "ECCKey.h"

CECCKey::CECCKey() :
		CKey() {
	m_sEncryptAlgorithmId = "ECDSA_256";
	memset(&ECCKeyPrivate, 0, MAXECCKEYSIZEWORDS * 4);
	memset(&ECCKeyPublicX, 0, MAXECCKEYSIZEWORDS * 4);
	memset(&ECCKeyPublicY, 0, MAXECCKEYSIZEWORDS * 4);
	KeySize(256);
	;
	m_bChanged = false;
}

CECCKey::~CECCKey() {
	DiscardAll();
}

CECCKey::CECCKey(const CECCKey& rhs) :
		CKey(rhs) {
	// copy constructor
	CECCKey& nc_rhs = const_cast<CECCKey&>(rhs);

	m_sECDSAPublicKeyCompX = rhs.m_sECDSAPublicKeyCompX;
	m_sECDSAPublicKeyCompY = rhs.m_sECDSAPublicKeyCompY;
	m_sECDSAPrivateKey = rhs.m_sECDSAPrivateKey;

	if (nc_rhs.m_ECDSAPublicKeyCompXList.size() > 0) {
		for_each(nc_rhs.m_ECDSAPublicKeyCompXList.begin(),
				nc_rhs.m_ECDSAPublicKeyCompXList.end(),
				[this](string*& pRhsData)
				{
					string* pData = new string(*pRhsData);
					m_ECDSAPublicKeyCompXList.push_back(pData);
				});
	}

	if (nc_rhs.m_ECDSAPublicKeyCompYList.size() > 0) {
		for_each(nc_rhs.m_ECDSAPublicKeyCompYList.begin(),
				nc_rhs.m_ECDSAPublicKeyCompYList.end(),
				[this](string*& pRhsData)
				{
					string* pData = new string(*pRhsData);
					m_ECDSAPublicKeyCompYList.push_back(pData);
				});
	}

	if (nc_rhs.m_ECDSAPrivateKeyList.size() > 0) {
		for_each(nc_rhs.m_ECDSAPrivateKeyList.begin(),
				nc_rhs.m_ECDSAPrivateKeyList.end(), [this](string*& pRhsData)
				{
					string* pData = new string(*pRhsData);
					m_ECDSAPrivateKeyList.push_back(pData);
				});
	}
}

CECCKey& CECCKey::operator=(const CECCKey& rhs) {
	// assignment operator
	if (&rhs != this) {
		CKey::operator=(rhs);

		CECCKey& nc_rhs = const_cast<CECCKey&>(rhs);
		CECCKey::DiscardAll();

		m_sECDSAPublicKeyCompX = rhs.m_sECDSAPublicKeyCompX;
		m_sECDSAPublicKeyCompY = rhs.m_sECDSAPublicKeyCompY;
		m_sECDSAPrivateKey = rhs.m_sECDSAPrivateKey;

		if (nc_rhs.m_ECDSAPublicKeyCompXList.size() > 0) {
			for_each(nc_rhs.m_ECDSAPublicKeyCompXList.begin(),
					nc_rhs.m_ECDSAPublicKeyCompXList.end(),
					[this](string*& pRhsData)
					{
						string* pData = new string(*pRhsData);
						m_ECDSAPublicKeyCompXList.push_back(pData);
					});
		}

		if (nc_rhs.m_ECDSAPublicKeyCompYList.size() > 0) {
			for_each(nc_rhs.m_ECDSAPublicKeyCompYList.begin(),
					nc_rhs.m_ECDSAPublicKeyCompYList.end(),
					[this](string*& pRhsData)
					{
						string* pData = new string(*pRhsData);
						m_ECDSAPublicKeyCompYList.push_back(pData);
					});
		}

		if (nc_rhs.m_ECDSAPrivateKeyList.size() > 0) {
			for_each(nc_rhs.m_ECDSAPrivateKeyList.begin(),
					nc_rhs.m_ECDSAPrivateKeyList.end(),
					[this](string*& pRhsData)
					{
						string* pData = new string(*pRhsData);
						m_ECDSAPrivateKeyList.push_back(pData);
					});
		}
	}
	return *this;
}

CKey* CECCKey::newCopy() {
	CECCKey* pNewCopy = new CECCKey(*this);
	return pNewCopy;
}

void CECCKey::DiscardAll() {
	if (m_ECDSAPublicKeyCompXList.size() > 0) {
		for_each(m_ECDSAPublicKeyCompXList.begin(),
				m_ECDSAPublicKeyCompXList.end(), [](string*& ps) {delete ps;});
		m_ECDSAPublicKeyCompXList.clear();
	}

	if (m_ECDSAPublicKeyCompYList.size() > 0) {
		for_each(m_ECDSAPublicKeyCompYList.begin(),
				m_ECDSAPublicKeyCompYList.end(), [](string*& ps) {delete ps;});
		m_ECDSAPublicKeyCompYList.clear();
	}

	if (m_ECDSAPrivateKeyList.size() > 0) {
		for_each(m_ECDSAPrivateKeyList.begin(), m_ECDSAPrivateKeyList.end(),
				[](string*& ps) {delete ps;});
		m_ECDSAPrivateKeyList.clear();
	}
}

void CECCKey::ResetPrivateKey() {
	if (m_ECDSAPrivateKeyList.size() > 0) {
		for_each(m_ECDSAPrivateKeyList.begin(), m_ECDSAPrivateKeyList.end(),
				[](string*& pData) {*pData = "0x00000000";});
	}

	m_bChanged = true;
}

void CECCKey::ResetCompX() {
	if (m_ECDSAPublicKeyCompXList.size() > 0) {
		for_each(m_ECDSAPublicKeyCompXList.begin(),
				m_ECDSAPublicKeyCompXList.end(),
				[](string*& pData) {*pData = "0x00000000";});
	}

	m_bChanged = true;
}

void CECCKey::ResetCompY() {
	if (m_ECDSAPublicKeyCompYList.size() > 0) {
		for_each(m_ECDSAPublicKeyCompYList.begin(),
				m_ECDSAPublicKeyCompYList.end(),
				[](string*& pData) {*pData = "0x00000000";});
	}

	m_bChanged = true;
}

string& CECCKey::ECDSAPublicKeyCompX(bool bIsBlf) {
	// returns a complete list of values, including all trailing 0x00000000
	m_sECDSAPublicKeyCompX = "";
	int nPerLine = 0;

	stringstream ssEntryNum;
	int nEntry = 0;

	t_stringListIter iterCompX = m_ECDSAPublicKeyCompXList.begin();
	while (iterCompX != m_ECDSAPublicKeyCompXList.end()) {
		if (bIsBlf) {
			nEntry++;
			ssEntryNum.str("");
			ssEntryNum << "#" << nEntry << " = ";

			m_sECDSAPublicKeyCompX += ssEntryNum.str();
			m_sECDSAPublicKeyCompX += *(*iterCompX);
			m_sECDSAPublicKeyCompX += "\n";
		} else {
			m_sECDSAPublicKeyCompX += *(*iterCompX);
			nPerLine++;
			if ((nPerLine != 0) && (nPerLine % 4 == 0)) // 4 is # of words per line
				m_sECDSAPublicKeyCompX += "\n";
			else
				m_sECDSAPublicKeyCompX += " ";
		}
		iterCompX++;
	}

	// handle the case where the final line has less than 4 words
	if (!bIsBlf && (m_ECDSAPublicKeyCompXList.size() % 4 != 0))
		m_sECDSAPublicKeyCompX += "\n";

	return m_sECDSAPublicKeyCompX;
}

string& CECCKey::ECDSAPublicKeyCompXPacked(bool bIsBlf) {
	// returns a truncated list of values, excluding all trailing 0x00000000
	m_sECDSAPublicKeyCompX = "";
	int nPerLine = 0;

	unsigned int uiActiveSize = ActiveSizeOfList(m_ECDSAPublicKeyCompXList);

	stringstream ssEntryNum;
	int nEntry = 0;

	t_stringListIter iterCompX = m_ECDSAPublicKeyCompXList.begin();
	while (iterCompX != m_ECDSAPublicKeyCompXList.end()) {
		uiActiveSize -= 32;

		if (bIsBlf) {
			nEntry++;
			ssEntryNum.str("");
			ssEntryNum << "#" << nEntry << " = ";

			m_sECDSAPublicKeyCompX += ssEntryNum.str();
			m_sECDSAPublicKeyCompX += *(*iterCompX);
			m_sECDSAPublicKeyCompX += "\n";
		} else {
			m_sECDSAPublicKeyCompX += *(*iterCompX);
			nPerLine++;
			if (((nPerLine != 0) && (nPerLine % 4 == 0)) || uiActiveSize == 0) // 4 is # of words per line
				m_sECDSAPublicKeyCompX += "\n";
			else
				m_sECDSAPublicKeyCompX += " ";
		}
		iterCompX++;
	}
	// handle the case where the final line has less than 4 words
	if (!bIsBlf && (uiActiveSize % 4 != 0))
		m_sECDSAPublicKeyCompX += "\n";

	return m_sECDSAPublicKeyCompX;
}

string& CECCKey::ECDSAPublicKeyCompY(bool bIsBlf) {
	// returns a complete list of values, including all trailing 0x00000000
	m_sECDSAPublicKeyCompY = "";
	int nPerLine = 0;

	stringstream ssEntryNum;
	int nEntry = 0;

	t_stringListIter iterCompY = m_ECDSAPublicKeyCompYList.begin();
	while (iterCompY != m_ECDSAPublicKeyCompYList.end()) {
		if (bIsBlf) {
			nEntry++;
			ssEntryNum.str("");
			ssEntryNum << "#" << nEntry << " = ";

			m_sECDSAPublicKeyCompY += ssEntryNum.str();
			m_sECDSAPublicKeyCompY += *(*iterCompY);
			m_sECDSAPublicKeyCompY += "\n";
		} else {
			m_sECDSAPublicKeyCompY += *(*iterCompY);
			nPerLine++;
			if ((nPerLine != 0) && (nPerLine % 4 == 0)) // 4 is # of words per line
				m_sECDSAPublicKeyCompY += "\n";
			else
				m_sECDSAPublicKeyCompY += " ";
		}
		iterCompY++;
	}

	// handle the case where the final line has less than 4 words
	if (!bIsBlf && (m_ECDSAPublicKeyCompYList.size() % 4 != 0))
		m_sECDSAPublicKeyCompY += "\n";

	return m_sECDSAPublicKeyCompY;
}

string& CECCKey::ECDSAPublicKeyCompYPacked(bool bIsBlf) {
	// returns a truncated list of values, excluding all trailing 0x00000000
	m_sECDSAPublicKeyCompY = "";
	int nPerLine = 0;

	unsigned int uiActiveSize = ActiveSizeOfList(m_ECDSAPublicKeyCompYList);

	stringstream ssEntryNum;
	int nEntry = 0;

	t_stringListIter iterCompY = m_ECDSAPublicKeyCompYList.begin();
	while (iterCompY != m_ECDSAPublicKeyCompYList.end()) {
		uiActiveSize -= 32;

		if (bIsBlf) {
			nEntry++;
			ssEntryNum.str("");
			ssEntryNum << "#" << nEntry << " = ";

			m_sECDSAPublicKeyCompY += ssEntryNum.str();
			m_sECDSAPublicKeyCompY += *(*iterCompY);
			m_sECDSAPublicKeyCompY += "\n";
		} else {
			m_sECDSAPublicKeyCompY += *(*iterCompY);
			nPerLine++;
			if (((nPerLine != 0) && (nPerLine % 4 == 0)) || uiActiveSize == 0) // 4 is # of words per line
				m_sECDSAPublicKeyCompY += "\n";
			else
				m_sECDSAPublicKeyCompY += " ";
		}

		iterCompY++;
	}

	// handle the case where the final line has less than 4 words
	if (!bIsBlf && (uiActiveSize % 4 != 0))
		m_sECDSAPublicKeyCompY += "\n";

	return m_sECDSAPublicKeyCompY;
}

string& CECCKey::ECDSAPrivateKey(bool bIsBlf) {
	m_sECDSAPrivateKey = "";
	int nPerLine = 0;

	stringstream ssEntryNum;
	int nEntry = 0;

	t_stringListIter iterECDSA = m_ECDSAPrivateKeyList.begin();
	while (iterECDSA != m_ECDSAPrivateKeyList.end()) {
		if (bIsBlf) {
			nEntry++;
			ssEntryNum.str("");
			ssEntryNum << "#" << nEntry << " = ";

			m_sECDSAPrivateKey += ssEntryNum.str();
			m_sECDSAPrivateKey += *(*iterECDSA);
			m_sECDSAPrivateKey += "\n";
		} else {
			m_sECDSAPrivateKey += *(*iterECDSA);
			nPerLine++;
			if ((nPerLine != 0) && (nPerLine % 4 == 0)) // 4 is # of words per line
				m_sECDSAPrivateKey += "\n";
			else
				m_sECDSAPrivateKey += " ";
		}
		iterECDSA++;
	}
	// handle the case where the final line has less than 4 words
	if (!bIsBlf && (m_ECDSAPrivateKeyList.size() % 4 != 0))
		m_sECDSAPrivateKey += "\n";

	return m_sECDSAPrivateKey;
}

string& CECCKey::ECDSAPrivateKeyPacked(bool bIsBlf) {
	m_sECDSAPrivateKey = "";
	int nPerLine = 0;

	unsigned int uiActiveSize = ActiveSizeOfList(m_ECDSAPrivateKeyList);

	stringstream ssEntryNum;
	int nEntry = 0;

	t_stringListIter iterECDSA = m_ECDSAPrivateKeyList.begin();
	while (iterECDSA != m_ECDSAPrivateKeyList.end() && uiActiveSize > 0) {
		uiActiveSize -= 32;

		if (bIsBlf) {
			nEntry++;
			ssEntryNum.str("");
			ssEntryNum << "#" << nEntry << " = ";

			m_sECDSAPrivateKey += ssEntryNum.str();
			m_sECDSAPrivateKey += *(*iterECDSA);
			m_sECDSAPrivateKey += "\n";
		} else {
			m_sECDSAPrivateKey += *(*iterECDSA);
			nPerLine++;
			if (((nPerLine != 0) && (nPerLine % 4 == 0)) || uiActiveSize == 0) // 4 is # of words per line
					{
				m_sECDSAPrivateKey += "\n";
			} else
				m_sECDSAPrivateKey += " ";
		}
		iterECDSA++;
	}
	// handle the case where the final line has less than 4 words
	if (!bIsBlf && (uiActiveSize % 4 != 0))
		m_sECDSAPrivateKey += "\n";

	return m_sECDSAPrivateKey;
}

void CECCKey::PublicKeySize(unsigned int uiPublicKeySize) {
	// changing the size results in expansion or reduction in exponent and modulus
	ResizeList(m_ECDSAPublicKeyCompXList, ((uiPublicKeySize + 31) / 32),
			m_bChanged);
	ResizeList(m_ECDSAPublicKeyCompYList, ((uiPublicKeySize + 31) / 32),
			m_bChanged);

	m_uiPublicKeySize = uiPublicKeySize;
}

void CECCKey::KeySize(unsigned int uiKeySize) {
	ResizeList(m_ECDSAPublicKeyCompXList, ((uiKeySize + 31) / 32), m_bChanged);
	ResizeList(m_ECDSAPublicKeyCompYList, ((uiKeySize + 31) / 32), m_bChanged);
	ResizeList(m_ECDSAPrivateKeyList, ((uiKeySize + 31) / 32), m_bChanged);

	m_uiKeySize = uiKeySize;
	PublicKeySize(m_uiKeySize);
}

bool CECCKey::FromKeyInfo_3_4(KEY_MOD_3_4_0& stKey) {
	string sKeyId = HexAsciiToText(HexFormattedAscii(stKey.KeyID));
	KeyId(sKeyId);
	EncryptAlgorithmId((ENCRYPTALGORITHMID_T) stKey.EncryptAlgorithmID);
	if (m_sEncryptAlgorithmId == "") {
		cout << "\nError: Unrecognized Encryption Algorithm ID: "
				<< stKey.EncryptAlgorithmID;
		return false;
	}

	HashAlgorithmId((HASHALGORITHMID_T) stKey.HashAlgorithmID);
	KeySize(stKey.KeySize);

	if (!ToListFromArray(stKey.Ecdsa.PublicKeyCompX, m_ECDSAPublicKeyCompXList,
			((m_uiKeySize + 31) / 32)))
		return false;

	if (!ToListFromArray(stKey.Ecdsa.PublicKeyCompY, m_ECDSAPublicKeyCompYList,
			((m_uiKeySize + 31) / 32)))
		return false;

	return true;
}

KEY_MOD_3_4_0 CECCKey::ToKeyInfo_3_4() {
	KEY_MOD_3_4_0 stKey;
	memset(&stKey, 0, sizeof(stKey));

	stKey.KeyID = Translate(m_sKeyTag);
	// the encrypted flag is added to the EncryptAlgorithmId at the end of this function
	stKey.EncryptAlgorithmID = EncryptAlgorithmId();
	stKey.HashAlgorithmID = HashAlgorithmId();
	stKey.KeySize = m_uiKeySize;
	stKey.PublicKeySize = m_uiPublicKeySize;

	unsigned int i = 0;

	ToArrayFromList(m_ECDSAPublicKeyCompXList, stKey.Ecdsa.PublicKeyCompX,
			(m_uiKeySize + 31) / 32);
	ToArrayFromList(m_ECDSAPublicKeyCompYList, stKey.Ecdsa.PublicKeyCompY,
			(m_uiKeySize + 31) / 32);

	if (Encrypted())
		stKey.EncryptAlgorithmID =
				(ENCRYPTALGORITHMID_T) (stKey.EncryptAlgorithmID
						| EncryptedBitMask);

	return stKey;
}

bool CECCKey::FromPlatDs(PLAT_DS& stDs) {
	EncryptAlgorithmId((ENCRYPTALGORITHMID_T) stDs.DSAlgorithmID);
	if (m_sEncryptAlgorithmId == "") {
		cout << "\nError: Unrecognized Encryption Algorithm ID: "
				<< stDs.DSAlgorithmID;
		return false;
	}
	KeySize(stDs.KeySize);
	HashAlgorithmId((HASHALGORITHMID_T) stDs.HashAlgorithmID);

	if (!ToListFromArray(stDs.Ecdsa.ECDSAPublicKeyCompX,
			m_ECDSAPublicKeyCompXList, ((m_uiKeySize + 31) / 32)))
		return false;

	if (!ToListFromArray(stDs.Ecdsa.ECDSAPublicKeyCompY,
			m_ECDSAPublicKeyCompYList, ((m_uiKeySize + 31) / 32)))
		return false;

	return true;
}

PLAT_DS CECCKey::toPlatDs() {
	PLAT_DS stDs;
	memset(&stDs, 0, sizeof(stDs));

	stDs.KeySize = m_uiKeySize;
	stDs.DSAlgorithmID = EncryptAlgorithmId();
	stDs.HashAlgorithmID = HashAlgorithmId();

	ToArrayFromList(m_ECDSAPublicKeyCompXList, stDs.Ecdsa.ECDSAPublicKeyCompX,
			(m_uiKeySize + 31) / 32);

	ToArrayFromList(m_ECDSAPublicKeyCompYList, stDs.Ecdsa.ECDSAPublicKeyCompY,
			(m_uiKeySize + 31) / 32);

	return stDs;
}

bool CECCKey::PrivateFromArray(UINT* pPrivateAry, unsigned int size) {
	if (!ToListFromArray(pPrivateAry, m_ECDSAPrivateKeyList, size))
		return false;

	return true;
}

bool CECCKey::PrivateToArray(UINT* pPrivateAry, unsigned int size) {
	if (!ToArrayFromList(m_ECDSAPrivateKeyList, pPrivateAry, size))
		return false;

	return true;
}

bool CECCKey::PubCompDFromArray(UINT* pCompDAry, unsigned int size) {
	if (!ToListFromArray(pCompDAry, m_ECDSAPublicKeyCompXList, size))
		return false;

	return true;
}

bool CECCKey::PubCompDToArray(UINT* pCompDAry, unsigned int size) {
	if (!ToArrayFromList(m_ECDSAPublicKeyCompXList, pCompDAry, size))
		return false;

	return true;
}

bool CECCKey::PubCompEFromArray(UINT* pCompEAry, unsigned int size) {
	if (!ToListFromArray(pCompEAry, m_ECDSAPublicKeyCompYList, size))
		return false;

	return true;
}

bool CECCKey::PubCompEToArray(UINT* pCompEAry, unsigned int size) {
	if (!ToArrayFromList(m_ECDSAPublicKeyCompYList, pCompEAry, size))
		return false;

	return true;
}

bool CECCKey::ECCKeyGen(ECC_KEY_TYPE ECCtype) {
	int size, sizeInWords;
	unsigned int publicKeyXArray[MAXECCKEYSIZEWORDS];
	unsigned int publicKeyYArray[MAXECCKEYSIZEWORDS];
	unsigned int privateKeyArray[MAXECCKEYSIZEWORDS];

	CryptoPP::AutoSeededRandomPool rng;
	CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA1>::PrivateKey privateKey;
	CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA1>::PublicKey publicKey;
	CryptoPP::Integer pubX, pubY, priv;
	CryptoPP::ECP::Point publicPoint;

	switch (ECCtype) {
	case ECC_KEY_TYPE::ECC256:
		size = 256;
		privateKey.Initialize(rng, CryptoPP::ASN1::secp256r1());
		break;
	case ECC_KEY_TYPE::ECC521:
		size = 521;
		privateKey.Initialize(rng, CryptoPP::ASN1::secp521r1());
		break;
	default:
		return false;
	}

	if (!privateKey.Validate(rng, 3))
		return false;

	privateKey.MakePublicKey(publicKey);
	if (!publicKey.Validate(rng, 3))
		return false;

	sizeInWords = (size + 31) / 32;
	unsigned int *publicKeyXArraySwapped = new unsigned int[sizeInWords];
	unsigned int *publicKeyYArraySwapped = new unsigned int[sizeInWords];
	unsigned int *privateKeyArraySwapped = new unsigned int[sizeInWords];

	priv = privateKey.GetPrivateExponent();
	publicPoint = publicKey.GetPublicElement();
	pubX = publicPoint.x;
	pubY = publicPoint.y;

	pubX.Encode((byte *) publicKeyXArray, MAXECCKEYSIZEWORDS * 4,
			CryptoPP::Integer::UNSIGNED);
	pubY.Encode((byte *) publicKeyYArray, MAXECCKEYSIZEWORDS * 4,
			CryptoPP::Integer::UNSIGNED);
	priv.Encode((byte *) privateKeyArray, MAXECCKEYSIZEWORDS * 4,
			CryptoPP::Integer::UNSIGNED);

	for (int i = 0; i < sizeInWords; ++i) {
		ECCKeyPublicX[i] = __builtin_bswap32(
				publicKeyXArray[MAXECCKEYSIZEWORDS - i - 1]);
		ECCKeyPublicY[i] = __builtin_bswap32(
				publicKeyYArray[MAXECCKEYSIZEWORDS - i - 1]);
		ECCKeyPrivate[i] = __builtin_bswap32(
				privateKeyArray[MAXECCKEYSIZEWORDS - i - 1]);
	}

	return true;
}

