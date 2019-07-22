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

#ifndef __KEY_GEN_SPEC_H__
#define __KEY_GEN_SPEC_H__
#include "TimLib.h"
#include "RSAKey.h"
#include "ECCKey.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <list>
#include <iterator>
using namespace std;

typedef list<string*> t_stringList;
typedef list<string*>::iterator t_stringListIter;

typedef struct {
	UINT_T KeyID;			// Associate an ID with this key
							// this value is the ASCII encoded hex value of the Key 
							// TAG <DSIG, JTAG, PTAC, TCAK, any 4-char>
	UINT_T HashAlgorithmID;	// 20=SHA160, 32=SHA256, 64=SHA512
	UINT_T EncryptAlgorithmID;// 0=Marvell_DS, 1=PKCS_1_5(caddo), (removed)2=PKCS_2_1(caddo)
							  // 3=PKCS_1_5(ippcp), (removed)4=PKCS_2_1(ippcp), 5=ECC_256, 6=ECC_521,7=PKCS_2_2(ippcp)
	UINT_T KeySize;	          	// Specified in bits
	union {
		struct {
			UINT_T RSAPublicExponent[MAXRSAKEYSIZEWORDS];
			UINT_T RSAModulus[MAXRSAKEYSIZEWORDS];         // Up to 2K bits
			UINT_T RSAPrivateKey[MAXRSAKEYSIZEWORDS];
		} Rsa;

		struct {
			UINT_T ECDSAPublicKeyCompX[MAXECCKEYSIZEWORDS];
			UINT_T ECDSAPublicKeyCompY[MAXECCKEYSIZEWORDS];
			UINT_T ECDSAPrivateKey[MAXECCKEYSIZEWORDS];
			// Pad this struct so it remains consistent with RSA struct
			UINT_T Reserved[(MAXRSAKEYSIZEWORDS * 3) - (MAXECCKEYSIZEWORDS * 3)];
		} Ecdsa;
	};
} GENERATED_KEY;

class CKeyGenSpec: public CTimLib {
public:
	CKeyGenSpec(void);
	virtual ~CKeyGenSpec(void);

	// copy constructor
	CKeyGenSpec(const CKeyGenSpec& rhs);
	// assignment operator
	CKeyGenSpec& operator=(const CKeyGenSpec& rhs);

	bool CheckKeySize(unsigned int keySize);
public:
	string& KeyGenSpecFilename() {
		return m_sKeyGenSpecFilename;
	}
	void KeyGenSpecFilename(string& sKeyGenSpecFilename);

	string& Seed() {
		return m_sSeed;
	}
	void Seed(string& sSeed);

	string& KeyId() {
		return m_sKeyId;
	}
	void KeyId(string& sKeyId);

	string& KeyTag() {
		return m_sKeyTag;
	}
	void KeyTag(string& sKeyTag);

	string& HashAlgorithmIdStr() {
		return m_sHashAlgorithmId;
	}
	void HashAlgorithmId(string& sHashAlgorithmId) {
		m_sHashAlgorithmId = sHashAlgorithmId;
		m_bChanged = true;
	}
	void HashAlgorithmId(HASHALGORITHMID_T Id);
	HASHALGORITHMID_T HashAlgorithmId();

	string& EncryptAlgorithmIdStr() {
		return m_sEncryptAlgorithmId;
	}
	void EncryptAlgorithmId(string& sEncryptAlgorithmId) {
		m_sEncryptAlgorithmId = sEncryptAlgorithmId;
		m_bChanged = true;
	}
	void EncryptAlgorithmId(ENCRYPTALGORITHMID_T Id);
	ENCRYPTALGORITHMID_T EncryptAlgorithmId();

	unsigned int& KeySize() {
		return m_uiKeySize;
	}
	virtual void KeySize(unsigned int uiKeySize);

	bool isValidKeySize(ENCRYPTALGORITHMID_T Id);

	string& OutputBinaryFilename() {
		return m_sOutputBinaryFilename;
	}
	void OutputBinaryFilename(string& sOutputBinaryFilename);

	string& OutputTextFilename() {
		return m_sOutputTextFilename;
	}
	void OutputTextFilename(string& sOutputTextFilename);

	string& OutputHashBinaryFilename() {
		return m_sOutputHashBinaryFilename;
	}
	void OutputHashBinaryFilename(string& sOutputHashBinaryFilename);

	bool GenerateKey(CKey& Key);
	bool OutputKeyText(string& sOutputTextFilename, CKey& Key);
	bool OutputKeyBinary(string& sOutputBinaryFilename, CKey& Key);
	bool GenerateHash(string& sOutputHashBinaryFilename, CKey& Key);

	bool ParseKeyGenSpecFile();

	bool IsChanged();
	void Changed(bool bSet) {
		m_bChanged = bSet;
	}

protected:
	bool m_bChanged;
	string m_sKeyGenSpecFilename;
	string m_sSeed;
	string m_sKeyId;
	string m_sKeyTag;
	string m_sEncryptAlgorithmId;
	string m_sHashAlgorithmId;
	unsigned int m_uiKeySize;
	string m_sOutputBinaryFilename;
	string m_sOutputTextFilename;
	string m_sOutputHashBinaryFilename;
};

#endif //__KEY_GEN_SPEC_H__
