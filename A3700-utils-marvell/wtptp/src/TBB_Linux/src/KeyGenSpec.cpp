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


#include "KeyGenSpec.h"

CKeyGenSpec::CKeyGenSpec(void) :
		CTimLib() {
	m_bChanged = false;
	m_sKeyGenSpecFilename = "";
	m_sSeed = "No Seed";
	m_sKeyId = "JTAG";
	m_sKeyTag = "0x4A544147"; //JTAG
	m_sEncryptAlgorithmId = "PKCS1_v1_5 (Ippcp)";
	m_sHashAlgorithmId = "SHA-160";
	m_uiKeySize = 1024;
	m_sOutputBinaryFilename = "";
	m_sOutputTextFilename = "";
	m_sOutputHashBinaryFilename = "";
}

CKeyGenSpec::~CKeyGenSpec(void) {
}

CKeyGenSpec::CKeyGenSpec(const CKeyGenSpec& rhs) :
		CTimLib(rhs) {
	// copy constructor
	CKeyGenSpec& nc_rhs = const_cast<CKeyGenSpec&>(rhs);

	m_sKeyGenSpecFilename = rhs.m_sKeyGenSpecFilename;
	m_sSeed = rhs.m_sSeed;
	m_sKeyId = rhs.m_sKeyId;
	m_sKeyTag = rhs.m_sKeyTag;
	m_sEncryptAlgorithmId = rhs.m_sEncryptAlgorithmId;
	m_sHashAlgorithmId = rhs.m_sHashAlgorithmId;
	m_uiKeySize = rhs.m_uiKeySize;
	m_sOutputBinaryFilename = rhs.m_sOutputBinaryFilename;
	m_sOutputTextFilename = rhs.m_sOutputTextFilename;

	m_bChanged = rhs.m_bChanged;
}

CKeyGenSpec& CKeyGenSpec::operator=(const CKeyGenSpec& rhs) {
	// assignment operator
	if (&rhs != this) {
		CTimLib::operator=(rhs);

		CKeyGenSpec& nc_rhs = const_cast<CKeyGenSpec&>(rhs);

		m_sKeyGenSpecFilename = rhs.m_sKeyGenSpecFilename;
		m_sSeed = rhs.m_sSeed;
		m_sKeyId = rhs.m_sKeyId;
		m_sKeyTag = rhs.m_sKeyTag;
		m_sEncryptAlgorithmId = rhs.m_sEncryptAlgorithmId;
		m_sHashAlgorithmId = rhs.m_sHashAlgorithmId;
		m_uiKeySize = rhs.m_uiKeySize;
		m_sOutputBinaryFilename = rhs.m_sOutputBinaryFilename;
		m_sOutputTextFilename = rhs.m_sOutputTextFilename;

		m_bChanged = rhs.m_bChanged;
	}
	return *this;
}

void CKeyGenSpec::KeyGenSpecFilename(string& sKeyGenSpecFilename) {
	m_sKeyGenSpecFilename = sKeyGenSpecFilename;
}

void CKeyGenSpec::Seed(string& sSeed) {
	m_sSeed = sSeed;
	unsigned int Hash[5] = { 0 };
	CryptoPP::SHA1().CalculateDigest((byte *) &Hash,
			(unsigned char *) m_sSeed.c_str(), m_sSeed.size());
	// reduce 5 int hash to 32 bits
	unsigned int seed = (Hash[0] & Hash[1]) | (Hash[2] & ~Hash[3]) | Hash[4];
	srand(seed);
	int ri = rand();
}

void CKeyGenSpec::KeyId(string& sKeyId) {
	sKeyId.resize(4);
	m_sKeyId = ToUpper(sKeyId, true);
	m_sKeyTag = TextToHexFormattedAscii(m_sKeyId);
	m_bChanged = true;
}

void CKeyGenSpec::KeyTag(string& sKeyTag) {
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

void CKeyGenSpec::HashAlgorithmId(HASHALGORITHMID_T Id) {
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
		Id = HASHALGORITHMID_T::SHA160;
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

void CKeyGenSpec::EncryptAlgorithmId(ENCRYPTALGORITHMID_T Id) {
	switch (Id) {
	case PKCS1_v1_5_Caddo:
		m_sEncryptAlgorithmId = "PKCS1_v1_5 (Caddo)";
		break;

	case PKCS1_v1_5_Ippcp:
		m_sEncryptAlgorithmId = "PKCS1_v1_5 (Ippcp)";
		break;

	case PKCS_v2_2_Ippcp:
		m_sEncryptAlgorithmId = "PKCS_v2_2 (Ippcp)";
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
		break;
	}
}

void CKeyGenSpec::KeySize(unsigned int uiKeySize) {
	m_uiKeySize = uiKeySize;
}

bool CKeyGenSpec::isValidKeySize(ENCRYPTALGORITHMID_T Id) {
	switch (Id) {
	case ECDSA_256:
		if (m_uiKeySize == 256)
			return true;
		return false;
	case ECDSA_521:
		if (m_uiKeySize == 521)
			return true;
		return false;
	default:
		if (m_uiKeySize == 1024 || m_uiKeySize == 2048)
			return true;
		return false;
	}
}

bool CKeyGenSpec::IsChanged() {
	return m_bChanged;
}

ENCRYPTALGORITHMID_T CKeyGenSpec::EncryptAlgorithmId() {
	ENCRYPTALGORITHMID_T eEncryptAlgoId = DUMMY_ENALG;
	if (EncryptAlgorithmIdStr() == "PKCS1_v1_5 (Caddo)"
			|| EncryptAlgorithmIdStr() == "PKCS1_v1_5_Caddo")
		eEncryptAlgoId = PKCS1_v1_5_Caddo;
	else if (EncryptAlgorithmIdStr() == "PKCS1_v1_5 (Ippcp)"
			|| EncryptAlgorithmIdStr() == "PKCS1_v1_5_Ippcp")
		eEncryptAlgoId = PKCS1_v1_5_Ippcp;
	else if (EncryptAlgorithmIdStr() == "PKCS_v2_2 (Ippcp)"
			|| EncryptAlgorithmIdStr() == "PKCS_v2_2_Ippcp")
		eEncryptAlgoId = PKCS_v2_2_Ippcp;
	else if (EncryptAlgorithmIdStr() == "ECDSA_256")
		eEncryptAlgoId = ECDSA_256;
	else if (EncryptAlgorithmIdStr() == "ECDSA_521")
		eEncryptAlgoId = ECDSA_521;

	return eEncryptAlgoId;
}

HASHALGORITHMID_T CKeyGenSpec::HashAlgorithmId() {
	HASHALGORITHMID_T eHashAlgoId = HASHALGORITHMID_T::SHA160;
	if (HashAlgorithmIdStr() == "SHA-160")
		eHashAlgoId = HASHALGORITHMID_T::SHA160;
	else if (HashAlgorithmIdStr() == "SHA-256")
		eHashAlgoId = HASHALGORITHMID_T::SHA256;
	else if (HashAlgorithmIdStr() == "SHA-512")
		eHashAlgoId = HASHALGORITHMID_T::SHA512;

	return eHashAlgoId;

}

void CKeyGenSpec::OutputBinaryFilename(string& sOutputBinaryFilename) {
	m_sOutputBinaryFilename = sOutputBinaryFilename;
}

void CKeyGenSpec::OutputTextFilename(string& sOutputTextFilename) {
	m_sOutputTextFilename = sOutputTextFilename;
}

void CKeyGenSpec::OutputHashBinaryFilename(string& sOutputHashBinaryFilename) {
	m_sOutputHashBinaryFilename = sOutputHashBinaryFilename;
}

bool CKeyGenSpec::GenerateKey(CKey& Key) {
	Key.KeyId(KeyId());
	Key.EncryptAlgorithmId(EncryptAlgorithmId());
	if (Key.EncryptAlgorithmIdStr() == "") {
		printf("\nERROR: ECCKeyGen failed. Invalid EncryptAlgorithm ID!\n");
		return false;
	}
	Key.KeySize(KeySize());
	Key.HashAlgorithmId(HashAlgorithmId());
	int iRetry = 0;
	if (EncryptAlgorithmId() == ECDSA_256
			|| EncryptAlgorithmId() == ECDSA_521) {
		ECC_KEY_TYPE ECCtype = KeySize() == 256 ? ECC256 : ECC521;
		CECCKey& EccKey = dynamic_cast<CECCKey&>(Key);

		for (iRetry = 0; iRetry < 20; iRetry++) {
			if (!EccKey.ECCKeyGen(ECCtype)) {
				if (iRetry < 19) {
					printf(
							"\nRetry: ECCKeyGen - generated key was invalid! Generating new key.\n");
					continue;
				}
				printf(
						"\nERROR: ECCKeyGen failed to generate a valid key after 20 attempts!\n");
				return false;
			}
		}

		ToListFromArray(EccKey.ECCKeyPrivate, EccKey.ECDSAPrivateKeyList(),
				(KeySize() + 31) / 32);
		ToListFromArray(EccKey.ECCKeyPublicX, EccKey.ECDSAPublicKeyCompXList(),
				(KeySize() + 31) / 32);
		ToListFromArray(EccKey.ECCKeyPublicY, EccKey.ECDSAPublicKeyCompYList(),
				(KeySize() + 31) / 32);
	} else {
		int rval = 0;
		unsigned int *N = 0;
		unsigned int *E = 0;
		unsigned int *D = 0;
		unsigned int *P = 0;
		unsigned int *Q = 0;
		int szN = 0;
		int szE = 0;
		int szD = 0;
		int szP = 0;
		int szQ = 0;

		CRSAKey& RSAKey = dynamic_cast<CRSAKey&>(Key);
		RSA_KEY_TYPE RSAtype = KeySize() == 1024 ? RSA1024 : RSA2048;
		for (iRetry = 0; iRetry < 20; iRetry++) {
			delete[] N;
			delete[] E;
			delete[] D;
			delete[] P;
			delete[] Q;
			N = 0;
			E = 0;
			D = 0;
			P = 0;
			Q = 0;

			szN = 0;
			szE = 0;
			szD = 0;
			szP = 0;
			szQ = 0;
			rval = 0;

			rval = RSAKey.RSAKeyGen(RSAtype, N, &szN, E, &szE, D, &szD, P, &szP,
					Q, &szQ);
			if (rval == 1) {
				bool bHighMSBEq1 = ((char) (N[(szN) - 1] >> 24) & 0x80) > 0; // make sure high bit of MSB is a 1
				if (!bHighMSBEq1) {
					printf(
							"\nRetry: ippsRSAKeyGen - generated key modulus MSB high bit was not 1! Generating new key.\n");
					continue;
				}
			} else {
				if (iRetry < 19) {

					printf(
							"\nRetry: ippsRSAKeyGen - generated key was invalid! Generating new key.\n");
					continue;
				}
				printf(
						"\nERROR: ippsRSAKeyGen failed to generate a valid key after 20 attempts!\n");
				return false;
			}
			break;
		}

		ToListFromArray(N, RSAKey.RsaSystemModulusList(), szN);
		ToListFromArray(E, RSAKey.PublicKeyExponentList(), szE);
		ToListFromArray(D, RSAKey.RsaPrivateKeyList(), szD);

		delete[] N;
		delete[] E;
		delete[] D;
		delete[] P;
		delete[] Q;
		N = 0;
		E = 0;
		D = 0;
		P = 0;
		Q = 0;
	}

	printf("\nKey Generated!");

	return true;
}

bool CKeyGenSpec::GenerateHash(string& sOutputHashBinaryFilename, CKey& Key) {
	KEY_MOD_3_4_0 KeyInfo;
	memset(&KeyInfo, 0, sizeof(KEY_MOD_3_4_0));
	KeyInfo = Key.ToKeyInfo_3_4();
	unsigned int KeyLength = KeyInfo.KeySize / 32;
	unsigned int SHAHashSize =
			(KeyInfo.HashAlgorithmID == HASHALGORITHMID_T::SHA512) ? 16 :
			(KeyInfo.HashAlgorithmID == HASHALGORITHMID_T::SHA256) ? 8 : 5;
	unsigned int KeyData[((2048 * 2) + 32) / sizeof(unsigned int)] = { 0 };
	int EncryptAlgo = KeyInfo.EncryptAlgorithmID & ~EncryptedBitMask;
	if (EncryptAlgo == PKCS1_v1_5_Caddo || EncryptAlgo == PKCS1_v1_5_Ippcp) {
		// PKCS_1.5: Store data in a temporary structure just for PKCS. This structure will  
		// be written into TIM binary.
		KeyLength = KeyInfo.KeySize / 32;
		if (EncryptAlgo == PKCS1_v1_5_Caddo) {
			// Fill in the Exponent
			// Fill in the modulus
			for (int i = 0; i < (int) KeyLength; i++) {
				Endian_Convert(KeyInfo.Rsa.RSAPublicExponent[i], &KeyData[i]);
				Endian_Convert(KeyInfo.Rsa.RSAModulus[i],
						&KeyData[KeyLength + i]);
			}
		} else {
			if (KeyInfo.HashAlgorithmID == HASHALGORITHMID_T::SHA256)
				KeyData[0] =
						(KeyLength == 64) ?
								PKCSv1_SHA256_2048RSA : PKCSv1_SHA256_1024RSA;
			else
				// Defaults to 160
				KeyData[0] =
						(KeyLength == 64) ?
								PKCSv1_SHA1_2048RSA : PKCSv1_SHA1_1024RSA;

			// Fill in the modulus
			// Fill in the Exponent
			// Note: modulus before exponent in post 3_4
			for (int i = 0; i < (int) KeyLength; i++) {
				KeyData[i + 1] = KeyInfo.Rsa.RSAModulus[i];
				KeyData[KeyLength + i + 1] = KeyInfo.Rsa.RSAPublicExponent[i];
			}
		}

	} else if (EncryptAlgo == PKCS_v2_2_Ippcp) {
		if (KeyInfo.HashAlgorithmID == HASHALGORITHMID_T::SHA256)
			KeyData[0] =
					(KeyLength == 64) ?
							PKCSv1_PSS_SHA256_2048RSA :
							PKCSv1_PSS_SHA256_1024RSA;
		else
			// Defaults to 160
			KeyData[0] =
					(KeyLength == 64) ?
							PKCSv1_PSS_SHA1_2048RSA : PKCSv1_PSS_SHA1_1024RSA;
		// Fill in the modulus
		// Fill in the Exponent
		// Note: modulus before exponent in post 3_4
		for (int i = 0; i < (int) KeyLength; i++) {
			KeyData[i + 1] = KeyInfo.Rsa.RSAModulus[i];
			KeyData[KeyLength + i + 1] = KeyInfo.Rsa.RSAPublicExponent[i];
		}
	}

	else if (EncryptAlgo == ECDSA_256 || EncryptAlgo == ECDSA_521) {
		KeyLength = (KeyInfo.KeySize + 31) / 32;
		if (KeyInfo.HashAlgorithmID == HASHALGORITHMID_T::SHA256)
			KeyData[0] =
					(KeyLength == 17) ?
							ECCP521_FIPS_DSA_SHA256 : ECCP256_FIPS_DSA_SHA256;
		else
			// Defaults to 160
			KeyData[0] =
					(KeyLength == 17) ?
							ECCP521_FIPS_DSA_SHA1 : ECCP256_FIPS_DSA_SHA1;
		for (int i = 0; i < (int) KeyLength; i++) {
			KeyData[i + 1] = KeyInfo.Ecdsa.PublicKeyCompX[i];
			KeyData[i + KeyLength + 1] = KeyInfo.Ecdsa.PublicKeyCompY[i];
		}
	}

	switch (KeyInfo.HashAlgorithmID) {
	case HASHALGORITHMID_T::SHA512:
		CryptoPP::SHA512().CalculateDigest((byte *) KeyInfo.KeyHash,
				(unsigned char *) KeyData, ((KeyLength * 2) + 1) * 4);
		break;
	case HASHALGORITHMID_T::SHA256:
		CryptoPP::SHA256().CalculateDigest((byte *) KeyInfo.KeyHash,
				(unsigned char *) KeyData, ((KeyLength * 2) + 1) * 4);
		break;
	default:
		CryptoPP::SHA1().CalculateDigest((byte *) KeyInfo.KeyHash,
				(unsigned char *) KeyData, ((KeyLength * 2) + 1) * 4);
	}
	// Do an endian conversion 1 word (4 bytes) at a time
	if (EncryptAlgo == PKCS1_v1_5_Caddo) {
		for (int i = 0; i < (int) SHAHashSize; i++)
			Endian_Convert(*(UINT_T *) (&KeyInfo.KeyHash[i]),
					(UINT_T *) (&KeyInfo.KeyHash[i]));
	}
	ofstream ofsKeyHashBinaryFile;
	ofsKeyHashBinaryFile.open(sOutputHashBinaryFilename.c_str(),
			ios_base::out | ios_base::trunc | ios_base::binary);
	if (ofsKeyHashBinaryFile.bad() || ofsKeyHashBinaryFile.fail()) {
		printf("\n  Error: fopen failed to open Key Binary File <%s>",
				sOutputHashBinaryFilename.c_str());
	}
	int size = sizeof(KeyInfo.KeyHash);
	ofsKeyHashBinaryFile.write((char*) KeyInfo.KeyHash,
			sizeof(KeyInfo.KeyHash));

	ofsKeyHashBinaryFile.flush();
	ofsKeyHashBinaryFile.close();

	printf("\n Generated KeyHashBinaryFile <%s>.",
			sOutputHashBinaryFilename.c_str());
	return true;
}

bool CKeyGenSpec::OutputKeyText(string& sOutputTextFilename, CKey& Key) {
	if (sOutputTextFilename.size() > 0) {
		ofstream ofsKeyTextFile;
		ofsKeyTextFile.open(sOutputTextFilename.c_str(),
				ios_base::out | ios_base::trunc);
		if (ofsKeyTextFile.bad() || ofsKeyTextFile.fail()) {
			printf("\n  Error: fopen failed to open KeyTextFile.txt <%s>",
					sOutputTextFilename.c_str());
		}

		stringstream ss;

		ss << "Key ID: " << Key.KeyId() << endl;
		ss << "Encrypt Algorithm ID: " << Key.EncryptAlgorithmId() << endl;
		ss << "Hash Algorithm ID: " << Key.HashAlgorithmId() << endl;
		ss << "Key Size in bits: " << Key.KeySize() << endl;

		if (EncryptAlgorithmId() == ECDSA_256
				|| EncryptAlgorithmId() == ECDSA_521) {
			CECCKey& EccKey = dynamic_cast<CECCKey&>(Key);
			ss << "ECDSA Public Key CompX: " << endl;
			t_stringListIter iter = EccKey.ECDSAPublicKeyCompXList().begin();
			int nPerLine = 0;
			while (iter != EccKey.ECDSAPublicKeyCompXList().end()) {
				if (++nPerLine % 4 == 0) // 4 is # of words per line
					ss << *(*iter) << endl;
				else
					ss << *(*iter) << " ";

				iter++;
			}
			if (EncryptAlgorithmId() == ECDSA_521)
				ss << endl;

			ss << "ECDSA Public Key CompY: " << endl;
			iter = EccKey.ECDSAPublicKeyCompYList().begin();
			nPerLine = 0;
			while (iter != EccKey.ECDSAPublicKeyCompYList().end()) {
				if (++nPerLine % 4 == 0) // 4 is # of words per line
					ss << *(*iter) << endl;
				else
					ss << *(*iter) << " ";

				iter++;
			}
			if (EncryptAlgorithmId() == ECDSA_521)
				ss << endl;

			ss << "ECDSA Private Key: " << endl;
			iter = EccKey.ECDSAPrivateKeyList().begin();
			nPerLine = 0;
			while (iter != EccKey.ECDSAPrivateKeyList().end()) {
				if (++nPerLine % 4 == 0) // 4 is # of words per line
					ss << *(*iter) << endl;
				else
					ss << *(*iter) << " ";

				iter++;
			}
			if (EncryptAlgorithmId() == ECDSA_521)
				ss << endl;

		} else {
			CRSAKey& RSAKey = dynamic_cast<CRSAKey&>(Key);
			ss << "RSA Public Exponent: " << endl;
			t_stringListIter iter = RSAKey.PublicKeyExponentList().begin();
			int nPerLine = 0;
			while (iter != RSAKey.PublicKeyExponentList().end()) {
				if (++nPerLine % 4 == 0) // 4 is # of words per line
					ss << *(*iter) << endl;
				else
					ss << *(*iter) << " ";

				iter++;
			}

			ss << "RSA System Modulus: " << endl;
			iter = RSAKey.RsaSystemModulusList().begin();
			nPerLine = 0;
			while (iter != RSAKey.RsaSystemModulusList().end()) {
				if (++nPerLine % 4 == 0) // 4 is # of words per line
					ss << *(*iter) << endl;
				else
					ss << *(*iter) << " ";

				iter++;
			}

			ss << "RSA Private Key: " << endl;
			iter = RSAKey.RsaPrivateKeyList().begin();
			nPerLine = 0;
			while (iter != RSAKey.RsaPrivateKeyList().end()) {
				if (++nPerLine % 4 == 0) // 4 is # of words per line
					ss << *(*iter) << endl;
				else
					ss << *(*iter) << " ";

				iter++;

			}
		}
		ofsKeyTextFile.write(ss.str().c_str(), ss.str().size());
		ofsKeyTextFile.flush();
		ofsKeyTextFile.close();

		printf("\n Generated KeyTextFile <%s>.", sOutputTextFilename.c_str());
	}

	return true;
}

bool CKeyGenSpec::OutputKeyBinary(string& sOutputBinaryFilename, CKey& Key) {
	if (sOutputBinaryFilename.size() > 0) {
		GENERATED_KEY KeyBin = { 0 };
		KeyBin.KeyID = Translate(TextToHexFormattedAscii(Key.KeyId()));
		KeyBin.HashAlgorithmID = Key.HashAlgorithmId();
		KeyBin.EncryptAlgorithmID = Key.EncryptAlgorithmId();
		KeyBin.KeySize = Key.KeySize();
		if (Key.EncryptAlgorithmId() == ECDSA_256
				|| Key.EncryptAlgorithmId() == ECDSA_521) {
			CECCKey& EccKey = dynamic_cast<CECCKey&>(Key);
			ToArrayFromList(EccKey.ECDSAPublicKeyCompXList(),
					KeyBin.Ecdsa.ECDSAPublicKeyCompX,
					EccKey.ECDSAPublicKeyCompXList().size());
			ToArrayFromList(EccKey.ECDSAPublicKeyCompYList(),
					KeyBin.Ecdsa.ECDSAPublicKeyCompY,
					EccKey.ECDSAPublicKeyCompYList().size());
			ToArrayFromList(EccKey.ECDSAPrivateKeyList(),
					KeyBin.Ecdsa.ECDSAPrivateKey,
					EccKey.ECDSAPrivateKeyList().size());
		} else {
			CRSAKey& RSAKey = dynamic_cast<CRSAKey&>(Key);
			ToArrayFromList(RSAKey.PublicKeyExponentList(),
					KeyBin.Rsa.RSAPublicExponent,
					RSAKey.PublicKeyExponentList().size());
			ToArrayFromList(RSAKey.RsaSystemModulusList(),
					KeyBin.Rsa.RSAModulus,
					RSAKey.RsaSystemModulusList().size());
			ToArrayFromList(RSAKey.RsaPrivateKeyList(),
					KeyBin.Rsa.RSAPrivateKey,
					RSAKey.RsaPrivateKeyList().size());
		}
		ofstream ofsKeyBinaryFile;
		ofsKeyBinaryFile.open(sOutputBinaryFilename.c_str(),
				ios_base::out | ios_base::trunc | ios_base::binary);
		if (ofsKeyBinaryFile.bad() || ofsKeyBinaryFile.fail()) {
			printf("\n  Error: fopen failed to open Key Binary File <%s>",
					sOutputBinaryFilename.c_str());
		}

		ofsKeyBinaryFile.write((char*) &KeyBin, sizeof(KeyBin));

		ofsKeyBinaryFile.flush();
		ofsKeyBinaryFile.close();

		printf("\n Generated KeyBinaryFile <%s>.",
				sOutputBinaryFilename.c_str());
	}

	return true;
}

bool CKeyGenSpec::ParseKeyGenSpecFile() {
	string sParam;
	unsigned int IntParam = 0;
	bool bRet = true;

	printf("\nParsing KeyGenSpecFile: <%s>\n", m_sKeyGenSpecFilename.c_str());

	// Reading private key from tim descriptor file
	ifstream m_ifsKeyGenSpecTxtFile;
	m_ifsKeyGenSpecTxtFile.open(m_sKeyGenSpecFilename.c_str(), ios_base::in);
	if (m_ifsKeyGenSpecTxtFile.bad() || m_ifsKeyGenSpecTxtFile.fail())
		return false;

	if (GetSValue(m_ifsKeyGenSpecTxtFile, "Seed:", sParam)) {
		sParam = TrimQuotes(sParam);
		Seed(sParam);
	} else
		bRet = false;

	if (GetSValue(m_ifsKeyGenSpecTxtFile, "Key ID:", sParam)) {
		sParam = TrimQuotes(sParam);
		KeyId(sParam);
	} else
		bRet = false;

	if (GetDWord(m_ifsKeyGenSpecTxtFile, "Encrypt Algorithm ID:", &IntParam)) {
		EncryptAlgorithmId((ENCRYPTALGORITHMID_T) IntParam);
		if (m_sEncryptAlgorithmId.empty()) {
			cout << endl << "Error: Unrecognized Encrypt Algorithm ID:"
					<< IntParam << endl;
			bRet = false;
		}
	} else
		bRet = false;

	if (GetDWord(m_ifsKeyGenSpecTxtFile, "Hash Algorithm ID:", &IntParam))
		HashAlgorithmId((HASHALGORITHMID_T) IntParam);
	else
		bRet = false;

	if (GetDWord(m_ifsKeyGenSpecTxtFile, "Key Size:", &IntParam)) {
		KeySize(IntParam);
	} else
		bRet = false;
	bRet = isValidKeySize(EncryptAlgorithmId());
	if (!bRet) {
		printf(
				"////////////////////////////////////////////////////////////\n");
		printf("  Error: Declaration statement is missing or corrupted!\n");
		printf("  Key Size:%d\n", IntParam);
		printf(
				"////////////////////////////////////////////////////////////\n");
	}

	if (GetSValue(m_ifsKeyGenSpecTxtFile, "Output Binary Filename:", sParam)) {
		sParam = TrimQuotes(sParam);
		OutputBinaryFilename(sParam);
	} else
		bRet = false;

	if (GetSValue(m_ifsKeyGenSpecTxtFile, "Output Text Filename:", sParam)) {
		sParam = TrimQuotes(sParam);
		OutputTextFilename(sParam);
	} else
		bRet = false;

	if (GetSValue(m_ifsKeyGenSpecTxtFile, "Output Hash Filename:", sParam)) {
		sParam = TrimQuotes(sParam);
		OutputHashBinaryFilename(sParam);
	}

	m_ifsKeyGenSpecTxtFile.close();

	return bRet;
}

