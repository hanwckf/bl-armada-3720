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



#include "CommandLineParser.h"
#include "TrustedImageBuilder.h"
#include "TrustedTimDescriptorParser.h"
#include "ImageDescription.h"
#include "TimDescriptorParser.h"
#include "Typedef.h"
#include "AutoBindERD.h"
#include "BbmtERD.h"
#include "CoreResetERD.h"
#include "EscapeSeqERD.h"
#include "GpioSetERD.h"
#include "PinERD.h"
#include "ResumeDdrERD.h"
#include "ROMResumeERD.h"
#include "TbrXferERD.h"
#include "UsbERD.h"
#include "UartERD.h"
#include "UsbVendorReqERD.h"
#include "ConsumerID.h"
#include "TzInitialization.h"
#include "KeyGenSpec.h"
#include "GenPurposePatch.h"
#include "GPPOperations.h"
#include "ImageMapERD.h"
#include "EscapeSeqV2ERD.h"
#include "CoreReleaseERD.h"
#include "OnDemandBootERD.h"
#include "CryptoPP_L_interface.h"

#include <sstream>
#include <algorithm>
#include <cstring>
#include <cstdlib>

CTrustedImageBuilder::CTrustedImageBuilder(
		CCommandLineParser& rCommandLineParser,
		CTrustedTimDescriptorParser& rTimDescriptorParser) :
		CImageBuilder(rCommandLineParser, rTimDescriptorParser), TimDescriptorParser(
				rTimDescriptorParser) {
	memset(OrigOtpHashBuffer, 0, sizeof(OrigOtpHashBuffer));
}

CTrustedImageBuilder::~CTrustedImageBuilder(void) {
}

bool CTrustedImageBuilder::BuildDescriptorFile() {
	bool bRet = true;
	char* pReservedBuffer = 0;
	unsigned int TIMHashSize = 0;
	unsigned char TIMHash[MAX_HASH] = { 0 };

	CTimDescriptor& TimDescriptor = TimDescriptorParser.TimDescriptor();

	// if TRUSTED version of TBB is used to build a non-trusted
	// image, then redirect to the non-trusted image builder
	if (!TimDescriptor.getTimHeader().getTrusted())
		return CImageBuilder::BuildDescriptorFile();

	CDigitalSignature& DSig = TimDescriptor.DigitalSignature();
	PLAT_DS Ds = DSig.toPlatDs();

	if (CommandLineParser.iOption == 1) {
		printf("\nProcessing trusted TIM...\n");
	} else {
		printf("\nProcessing trusted TIM without digital signature and\n");
		printf("building TIM hash binary file...\n");
	}

	if (CommandLineParser.bIsTimTxtFile || CommandLineParser.bIsBlfTxtFile) {
		// open new or replace existing tim bin file
		ios_base::openmode mode = ios_base::in | ios_base::out
				| ios_base::binary | ios_base::trunc;
		fstream& fsTimBinFile = OpenTimBinFile(mode); // create new file
		if (fsTimBinFile.bad() || fsTimBinFile.fail())
			return false;

		// set TIMHashSize to default value
		// this value may be changed by the TIM image info
		TIMHashSize = GetTIMHashSize(&Ds);

		if (bRet && !ProcessImages(fsTimBinFile, &TIMHashSize)) {
			printf("  Error: Failed to process TIM images.\n");
			bRet = false;
		}

		if (bRet && ProcessKeyInfo(fsTimBinFile) == FALSE)
			bRet = false;

		if (bRet && !ProcessReservedData(fsTimBinFile)) {
			printf("  Error: Failed to process Reserved Data Area.\n");
			bRet = false;
		}

		unsigned int lLoc = 0;
		// note that the (unsigned int) cast is dangerous for very large files but should not
		// be an issue here
		lLoc = (int) fsTimBinFile.tellg();
		if (bRet && CommandLineParser.bOneNANDPadding) {
			// pad out end of tim image to uiPaddedSize with all 0xFF
			const char PadByte = (char) 0xff;
			printf("\n'%s' size in bytes prior to padding: %u\n",
					TimDescriptorParser.TimDescriptorBinFilePath().c_str(),
					(UINT) sizeof(PLAT_DS) + lLoc);

			int iPadNeeded = CommandLineParser.uiPaddedSize - sizeof(PLAT_DS)
							- lLoc;

			if (iPadNeeded > 0) {
				printf("\n'%s' size in bytes after padding: %u\n",
						TimDescriptorParser.TimDescriptorBinFilePath().c_str(),
						(UINT) CommandLineParser.uiPaddedSize);
			} else {
				printf("\n'%s' padding is not needed. \n",
						TimDescriptorParser.TimDescriptorBinFilePath().c_str());
			}

			while (iPadNeeded-- > 0)
				fsTimBinFile.write(&PadByte, sizeof(char));

			fsTimBinFile.flush();

			// note that the (long) cast is dangerous for very large files but should not
			// be an issue here
			lLoc = (long) fsTimBinFile.tellg();
		} else if (bRet) {
			printf("\n'%s' size in bytes: %lu\n",
					TimDescriptorParser.TimDescriptorBinFilePath().c_str(),
					(sizeof(PLAT_DS) + lLoc));
		}

		if (CommandLineParser.bVerbose
				&& (TimDescriptorParser.TimDescriptor().GetTimImageSize() > 4096
						|| CommandLineParser.uiPaddedSize > 4096)) {
			printf("WARNING: '%s' size is greater than 4Kb.\n",
					TimDescriptorParser.TimDescriptorBinFilePath().c_str());
		}

		if (bRet && ProcessDsaInfo(fsTimBinFile, OrigOtpHashBuffer) == FALSE)
			bRet = false;

		long lFileSize = 0;
		char *TIMBuffer = NULL;

		if (!LoadTimBinFileToTimBuffer(fsTimBinFile, TIMBuffer, &lFileSize))
			return false;

		if (TIMBuffer == NULL) {
			delete[] TIMBuffer;
			return false;
		}

		TIMHashSize = GetTIMHashSize(&Ds);
		if (bRet
				&& GenerateTIMHash(TIMBuffer, lFileSize, TIMHash, TIMHashSize)
		== FALSE)
			bRet = false;

		unsigned int SHAHashSize =
				(Ds.HashAlgorithmID == HASHALGORITHMID_T::SHA512) ? 16 :
						(Ds.HashAlgorithmID == HASHALGORITHMID_T::SHA256) ? 8 : 5;

		// If option 2 is selected the digital signature will be installed later
		if (bRet && CommandLineParser.iOption == 2) {
			DumpTIMHashToBinFile(&Ds, (unsigned char *) &TIMHash,
					(char*) CommandLineParser.HashFilePath.c_str());
		} else if (bRet) {
			if (DSig.DSKey()) {
				if (DSig.DSKey()->EncryptAlgorithmId() == ECDSA_256
						|| DSig.DSKey()->EncryptAlgorithmId() == ECDSA_521) {
					bRet = signMessageUsingCryptoPPECDSA(
							(unsigned char *) TIMHash, TIMHashSize, DSig);
				} else if (DSig.DSKey()->EncryptAlgorithmId() == PKCS_v2_2_Ippcp
						|| DSig.DSKey()->EncryptAlgorithmId()
						== PKCS1_v1_5_Ippcp) {
					bRet = signMessageUsingCryptoPPRSA(
							(unsigned char *) TIMHash, TIMHashSize, DSig);
				} else
					bRet = false;
			}
			if (bRet) {
				CDigitalSignature DSigOrig(DSig);

				if (InstallDigitalSignature(fsTimBinFile) == FALSE)
					bRet = false;
				else {
					if (CommandLineParser.bIsTimVerify == true) {
						bRet = VerifyDS(DSigOrig, fsTimBinFile,
								TimDescriptor.ProcessorTypeStr());
					}
				}
			}
		}

		if (bRet && CommandLineParser.bConcatenate) {
			if (bRet && !ProcessConcatenatedImages(fsTimBinFile)) {
				printf("Failed to process concatenated images.\n");
				bRet = false;
			}
		}

		CloseTimBinFile();
		if (TIMBuffer != NULL)
			delete[] TIMBuffer;
	}

	if (bRet) {
		printf("\n  Success: TIM Processing has completed successfully!\n\n");
	} else {
		string sBadTIMBinName = TimDescriptorParser.TimDescriptorBinFilePath();
		sBadTIMBinName += ".bad";
		ifstream badIfs;
		badIfs.open(sBadTIMBinName.c_str(), ios_base::in);
		if (badIfs.good() && !badIfs.fail()) {
			badIfs.close();
			remove(sBadTIMBinName.c_str());
		}

		printf("\n  Failed: TIM Processing has failed!\n\n");
		if (0
				!= rename(
						TimDescriptorParser.TimDescriptorBinFilePath().c_str(),
						sBadTIMBinName.c_str())) {
			printf("\n  Unable to rename <%s> to <%s> \n",
					TimDescriptorParser.TimDescriptorBinFilePath().c_str(),
					sBadTIMBinName.c_str());
		} else {
			printf(" Renamed incomplete TIM binary file <%s> to <%s> \n",
					TimDescriptorParser.TimDescriptorBinFilePath().c_str(),
					sBadTIMBinName.c_str());
		}
	}

	return bRet;
}

bool CTrustedImageBuilder::BuildDigitalSignature() {
	unsigned char TIMHash[MAX_HASH] = { 0 };
	std::streamoff lFileSize = 0;
	bool IsOk = false;

	printf("\nBuilding Digital Signature...\n");

	// Reading TIM hash from a file

	ifstream ifsTIMHash;
	string sHashFilePath = CommandLineParser.HashFilePath;
	PrependPathIfNone(sHashFilePath);
	ifsTIMHash.open(sHashFilePath.c_str(), ios_base::in | ios_base::binary);
	if (ifsTIMHash.bad() || ifsTIMHash.fail()) {
		printf("\n  Error: Cannot open file <%s> !\n",
				CommandLineParser.HashFilePath.c_str());
		return FALSE;
	}

	ifsTIMHash.seekg(0, ios_base::end);
	ifsTIMHash.clear();

	lFileSize = ifsTIMHash.tellg(); // Get position of file, thus the file size.
	ifsTIMHash.seekg(0, ios_base::beg);  // Set position to SOF

	if (lFileSize > sizeof(TIMHash)) {
		printf(
				"////////////////////////////////////////////////////////////\n");
		printf("  Error: BuildDigitalSignature Error:\n");
		printf("The size of the input hash file is greater than 256 bytes!\n");
		printf(
				"////////////////////////////////////////////////////////////\n");
		ifsTIMHash.close();
		return FALSE;
	}

	memset(TIMHash, 0, sizeof(TIMHash));
	ifsTIMHash.read((char*) TIMHash, lFileSize);
	ifsTIMHash.close();

	CDigitalSignature& DSig =
			TimDescriptorParser.TimDescriptor().DigitalSignature();
	DSig.KeySize(DSig.DSKey()->KeySize());
	unsigned int SHAHashSize =
			(DSig.DSKey()->HashAlgorithmId() == HASHALGORITHMID_T::SHA512) ?
					16 :
					(DSig.DSKey()->HashAlgorithmId() == HASHALGORITHMID_T::SHA256) ?
							8 : 5;

	if (DSig.DSKey()->EncryptAlgorithmId() == ECDSA_256
			|| DSig.DSKey()->EncryptAlgorithmId() == ECDSA_521) {
		if (!signMessageUsingCryptoPPECDSA(TIMHash, SHAHashSize, DSig))
			return FALSE;
	} else if (DSig.DSKey()->EncryptAlgorithmId() == PKCS_v2_2_Ippcp
			|| DSig.DSKey()->EncryptAlgorithmId() == PKCS1_v1_5_Ippcp) {
		if (!signMessageUsingCryptoPPRSA(TIMHash, SHAHashSize, DSig))
			return FALSE;
	} else
		return FALSE;

	// Writing digital signature to a file
	fstream fsDsFile;
	string sDsFilePath = CommandLineParser.DsFilePath;
	PrependPathIfNone(sDsFilePath);
	fsDsFile.open(sDsFilePath.c_str(),
			ios_base::out | ios_base::binary | ios_base::trunc);
	if (fsDsFile.bad() || fsDsFile.fail()) {
		printf("\n  Error: Cannot open TIM Bin file <%s> !\n",
				CommandLineParser.DsFilePath.c_str());
	}

	return InstallDigitalSignature(fsDsFile);

}

bool CTrustedImageBuilder::AddDigitalSignatureToTim() {
	long lFileSize = 0;
	// allocated a buffer big enough for either RSA or ECC ds
	char Ds[((MAXECCKEYSIZEWORDS * 2) + (MAXRSAKEYSIZEWORDS * 3)
			- (MAXECCKEYSIZEWORDS * 4)) * 4] = { 0 };

	// This function is used in option mode 4 where a digital signature is read
	// from a binary file and appended at the end of a TIM binary file.

	printf("\nInstalling Digital Signature...\n");

	// Reading digital signature from a file

	ifstream ifsDsFile;
	string sDsFilePath = CommandLineParser.DsFilePath;
	PrependPathIfNone(sDsFilePath);
	ifsDsFile.open(sDsFilePath.c_str(), ios_base::in | ios_base::binary);
	if (ifsDsFile.bad() || ifsDsFile.fail()) {
		printf(
				"////////////////////////////////////////////////////////////\n");
		printf("  Error: AddDigitalSignatureToTim\n");
		printf("Cannot open file <%s> !\n",
				CommandLineParser.DsFilePath.c_str());
		printf(
				"////////////////////////////////////////////////////////////\n");
		return FALSE;
	}

	ifsDsFile.seekg(0, ios_base::end);  // Set position to EOF
	ifsDsFile.clear();
	// note that the (long) cast is dangerous for very large files but should not
	// be an issue here
	lFileSize = (long) ifsDsFile.tellg(); // Get position of file, thus the file size.
	ifsDsFile.seekg(0, ios_base::beg);  // Set position to SOF

	if (!CommandLineParser.bKeyByteSwap && (lFileSize != MAXRSAKEYSIZEWORDS * 4)
			&& (lFileSize
					!= ((MAXECCKEYSIZEWORDS * 2) + (MAXRSAKEYSIZEWORDS * 3)
							- (MAXECCKEYSIZEWORDS * 4)) * 4)) {
		printf(
				"////////////////////////////////////////////////////////////\n");
		printf("  Error: AddDigitalSignatureToTim\n");
		if (CommandLineParser.bKeyByteSwap) {
			printf(
					"The -a command-line switch is in use for handling externally generated signature bin file.\n");
		} else {
			printf(
					"The -a command-line switch is NOT in use, so signature bin file is assumed to have been generated by TBB.\n");
		}
		printf("The size of the digital signature bin file is %ld!\n",
				lFileSize);
		printf("The size should be %d for RSA Algorithms \n",
				MAXRSAKEYSIZEWORDS * 4);
		printf("or the size should be %d for ECC Algorithms \n",
				((MAXECCKEYSIZEWORDS * 2) + (MAXRSAKEYSIZEWORDS * 3)
						- (MAXECCKEYSIZEWORDS * 4)) * 4);
		printf(
				"////////////////////////////////////////////////////////////\n");
		ifsDsFile.close();
		return FALSE;
	}

	ifsDsFile.read(Ds, lFileSize);
	ifsDsFile.close();

	// feature to support -a command line switch when the ds.bin provided needs to
	// have the byte streams reversed and the Endian order fixed
	if (CommandLineParser.bKeyByteSwap == true) {
		UINT_T SwappedDs[((MAXECCKEYSIZEWORDS * 2) + (MAXRSAKEYSIZEWORDS * 3)
				- (MAXECCKEYSIZEWORDS * 4))] = { 0 };
		int SizeDsInWords = (lFileSize / 4);

		for (int i = 0, x = SizeDsInWords - 1; x >= 0; i++, x--) {
			Endian_Convert((*(unsigned int*) &Ds[i * 4]), &(SwappedDs[x]));
		}
		memcpy(Ds, SwappedDs, lFileSize);
	}

	ifstream ifsTimInFile;
	string sTimInFilePath = CommandLineParser.TimInFilePath;
	PrependPathIfNone(sTimInFilePath);
	ifsTimInFile.open(sTimInFilePath.c_str(), ios_base::in | ios_base::binary);
	if (ifsTimInFile.bad() || ifsTimInFile.fail()) {
		printf("\n  Error: Cannot open TIM In file <%s> !\n",
				CommandLineParser.TimInFilePath.c_str());
	}

	fstream fsTimOutFile;
	string sTimOutFilePath = CommandLineParser.TimOutFilePath;
	PrependPathIfNone(sTimOutFilePath);
	fsTimOutFile.open(sTimOutFilePath.c_str(),
			ios_base::out | ios_base::binary | ios_base::trunc);
	if (fsTimOutFile.bad() || fsTimOutFile.fail()) {
		printf("\n  Error: Cannot open TIM Out file <%s> !\n",
				CommandLineParser.TimOutFilePath.c_str());
	}

	// copy tim in to tim out
	ifsTimInFile.seekg(0, ios::end);
	// note that the (unsigned int) cast is dangerous for very large files but should not
	// be an issue here
	unsigned int size = (unsigned int) ifsTimInFile.tellg();
	ifsTimInFile.seekg(0, ios::beg);

	char* buffer = new char[size + 1];
	memset(buffer, 0, size + 1);
	ifsTimInFile.read(buffer, size);
	ifsTimInFile.close();
	fsTimOutFile.write(buffer, size);
	delete[] buffer;

	if (CommandLineParser.bKeyByteSwap == true) {
		// lFileSize is the length of the dsig
		// iPadSize is the number of '0' bytes of padding
		// assume RSA
		int iPadSize = ((MAXRSAKEYSIZEWORDS) - (lFileSize / 4)) * 4;

		if (iPadSize < 0) {
			// assume ECC
			iPadSize = ((MAXECCKEYSIZEWORDS * 2) - (lFileSize / 4)) * 4;
			iPadSize += ((MAXRSAKEYSIZEWORDS * 3) - (MAXECCKEYSIZEWORDS * 4))
							* 4;
		}

		fsTimOutFile.write(Ds, lFileSize + iPadSize);
	} else {
		// append ds to tim out
		fsTimOutFile.write(Ds, lFileSize);
	}

	fsTimOutFile.close();

	return TRUE;
}

bool CTrustedImageBuilder::ProcessKeyInfo(fstream& fsTimBinFile) {
	if ((int) TimDescriptorParser.TimDescriptor().getTimHeader().getNumKeys()
			!= TimDescriptorParser.TimDescriptor().KeyList().size())
		return FALSE;

	t_KeyListIter Iter = TimDescriptorParser.TimDescriptor().KeyList().begin();
	KEY_MOD_3_4_0 KeyInfo_3_4;
	while (Iter != TimDescriptorParser.TimDescriptor().KeyList().end()) {
		memset(&KeyInfo_3_4, 0, sizeof(KEY_MOD_3_4_0));
		KeyInfo_3_4 = (*Iter)->ToKeyInfo_3_4();
		if (GenerateKeyHash(&KeyInfo_3_4) == FALSE) {
			printf("  Error: Generating the Key Hash has failed!\n");
			return FALSE;
		}

		if (TimDescriptorParser.TimDescriptor().getTimHeader().getVersion() < TIM_3_2_00) {
			KEY_MOD_3_1_0 KeyInfo;
			memset(&KeyInfo, 0, sizeof(KEY_MOD_3_1_0));
			KeyInfo.KeyID = KeyInfo_3_4.KeyID;
			KeyInfo.HashAlgorithmID = KeyInfo_3_4.HashAlgorithmID;
			KeyInfo.KeySize = KeyInfo_3_4.KeySize / 8;
			KeyInfo.PublicKeySize = KeyInfo_3_4.PublicKeySize / 8;
			memcpy(&KeyInfo.RSAPublicExponent,
					&KeyInfo_3_4.Rsa.RSAPublicExponent, KeyInfo.PublicKeySize);
			memcpy(&KeyInfo.RSAModulus, &KeyInfo_3_4.Rsa.RSAModulus,
					KeyInfo.KeySize);
			memcpy(&KeyInfo.KeyHash, &KeyInfo_3_4.KeyHash,
					sizeof(KeyInfo.KeyHash));
			fsTimBinFile.write((char*) &KeyInfo, sizeof(KEY_MOD_3_1_0));
		} else if (TimDescriptorParser.TimDescriptor().getTimHeader().getVersion() < TIM_3_3_00) {
			KEY_MOD_3_2_0 KeyInfo;
			memset(&KeyInfo, 0, sizeof(KEY_MOD_3_2_0));
			KeyInfo.KeyID = KeyInfo_3_4.KeyID;
			KeyInfo.HashAlgorithmID = KeyInfo_3_4.HashAlgorithmID;
			KeyInfo.KeySize = KeyInfo_3_4.KeySize / 8;
			KeyInfo.PublicKeySize = KeyInfo_3_4.PublicKeySize / 8;
			memcpy(&KeyInfo.RSAPublicExponent,
					&KeyInfo_3_4.Rsa.RSAPublicExponent, KeyInfo.PublicKeySize);
			memcpy(&KeyInfo.RSAModulus, &KeyInfo_3_4.Rsa.RSAModulus,
					KeyInfo.KeySize);
			memcpy(&KeyInfo.KeyHash, &KeyInfo_3_4.KeyHash,
					sizeof(KeyInfo.KeyHash));
			fsTimBinFile.write((char*) &KeyInfo, sizeof(KEY_MOD_3_2_0));
		} else if (TimDescriptorParser.TimDescriptor().getTimHeader().getVersion() < TIM_3_4_00) {
			KEY_MOD_3_3_0 KeyInfo;
			memset(&KeyInfo, 0, sizeof(KEY_MOD_3_3_0));
			KeyInfo.KeyID = KeyInfo_3_4.KeyID;
			KeyInfo.HashAlgorithmID = KeyInfo_3_4.HashAlgorithmID;
			KeyInfo.KeySize = KeyInfo_3_4.KeySize;
			KeyInfo.PublicKeySize = KeyInfo_3_4.PublicKeySize;
			KeyInfo.EncryptAlgorithmID = KeyInfo_3_4.EncryptAlgorithmID;
			// if encrypted
			if ((KeyInfo_3_4.EncryptAlgorithmID & EncryptedBitMask) > 0) {
				memcpy(&KeyInfo.Rsa.RSAPublicExponent,
						&KeyInfo_3_4.Rsa.RSAPublicExponent,
						KeyInfo.PublicKeySize);
				memcpy(&KeyInfo.Rsa.RSAModulus, &KeyInfo_3_4.Rsa.RSAModulus,
						KeyInfo.KeySize);
				memcpy(&KeyInfo.KeyHash, &KeyInfo_3_4.KeyHash,
						sizeof(KeyInfo.KeyHash));
			} else {
				if (KeyInfo.EncryptAlgorithmID == ECDSA_256
						|| KeyInfo.EncryptAlgorithmID == ECDSA_521) {
					memcpy(&KeyInfo.Ecdsa.PublicKeyCompX,
							&KeyInfo_3_4.Ecdsa.PublicKeyCompX,
							KeyInfo.KeySize / 8);
					memcpy(&KeyInfo.Ecdsa.PublicKeyCompY,
							&KeyInfo_3_4.Ecdsa.PublicKeyCompY,
							KeyInfo.PublicKeySize / 8);
				} else {
					memcpy(&KeyInfo.Rsa.RSAModulus, &KeyInfo_3_4.Rsa.RSAModulus,
							KeyInfo.KeySize / 8);
					memcpy(&KeyInfo.Rsa.RSAPublicExponent,
							&KeyInfo_3_4.Rsa.RSAPublicExponent,
							KeyInfo.PublicKeySize / 8);
				}
				memcpy(&KeyInfo.KeyHash, &KeyInfo_3_4.KeyHash,
						sizeof(KeyInfo.KeyHash));
			}
			fsTimBinFile.write((char*) &KeyInfo, sizeof(KEY_MOD_3_3_0));
		} else if (TimDescriptorParser.TimDescriptor().getTimHeader().getVersion() >= TIM_3_4_00) {
			fsTimBinFile.write((char*) &KeyInfo_3_4, sizeof(KEY_MOD_3_4_0));
		}

		fsTimBinFile.flush();

		Iter++;
	}

	return TRUE;
}

bool CTrustedImageBuilder::ProcessDsaInfo(fstream& fsTimBinFile,
		unsigned int OtpHashBuffer[]) {
	PLAT_DS Temp_Ds =
			TimDescriptorParser.TimDescriptor().DigitalSignature().toPlatDs();

	if (!CalculateOtpHash(&(Temp_Ds), TimDescriptorParser.TimDescriptor(),
			OtpHashBuffer))
		return false;

	if (Temp_Ds.DSAlgorithmID == ECDSA_256
			|| Temp_Ds.DSAlgorithmID == ECDSA_521) {
		fsTimBinFile.write((char*) &Temp_Ds,
				sizeof(PLAT_DS)
				- (sizeof(UINT_T)
						* ((MAXRSAKEYSIZEWORDS * 3)
								- (MAXECCKEYSIZEWORDS * 2))));
	} else if (Temp_Ds.DSAlgorithmID == PKCS1_v1_5_Caddo) {
		// for older tim, keysize is in bytes so convert it
		if (TimDescriptorParser.TimDescriptor().getTimHeader().getVersion() < TIM_3_3_00)
			Temp_Ds.KeySize /= 8;

		std::streamoff lLoc = fsTimBinFile.tellg();
		fsTimBinFile.write((char*) &Temp_Ds,
				(sizeof(PLAT_DS) - (sizeof(UINT_T) * MAXRSAKEYSIZEWORDS)));
	} else {
		// for older tim, keysize is in bytes so convert it
		if (TimDescriptorParser.TimDescriptor().getTimHeader().getVersion() < TIM_3_3_00)
			Temp_Ds.KeySize /= 8;

		fsTimBinFile.write((char*) &Temp_Ds,
				(sizeof(PLAT_DS) - (sizeof(UINT_T) * MAXRSAKEYSIZEWORDS)));
	}

	fsTimBinFile.flush();

	return fsTimBinFile.good();
}

bool CTrustedImageBuilder::CalculateOtpHash(PLAT_DS* pDs,
		CTimDescriptor& TimDescriptor, unsigned int OtpHashBuffer[]) {
	unsigned int KeyBuffer[(MAXRSAKEYSIZEWORDS + 1) * 2] = { 0 };

	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int KeyLength = 0;

	unsigned int SHAHashSize =
			(pDs->HashAlgorithmID == HASHALGORITHMID_T::SHA512) ? 16 :
					(pDs->HashAlgorithmID == HASHALGORITHMID_T::SHA256) ? 8 : 5;
	unsigned int scheme =
			TimDescriptor.getTimHeader().getVersion() >= TIM_3_4_00 ? 1 : 0;

	// PKCS_1.5: The data read from the descriptor file is in LSW, Little Endian format.
	// Endian conversions and word swaps are required for PKCS 1.5 in order to make the
	// data compatible with the Fast signature PI.
	if (pDs->DSAlgorithmID == PKCS1_v1_5_Caddo
			|| pDs->DSAlgorithmID == PKCS1_v1_5_Ippcp) {
		// PKCS_1.5: Store data in a temporary structure just for PKCS. This structure will
		// be written into TIM binary.
		KeyLength = pDs->KeySize / 32;

		if (TimDescriptor.getTimHeader().getVersion() < TIM_3_4_00) {
			if (pDs->DSAlgorithmID == PKCS1_v1_5_Caddo) {
				// PKCS_1.5:Convert Public Exponent, swap LSW to MSW and to change Endian.
				for (i = 0, j = KeyLength - 1; i < KeyLength; i++, j--) {
					KeyBuffer[j] = pDs->Rsa.RSAPublicExponent[i];
					Endian_Convert(*(unsigned int *) &KeyBuffer[j],
							(unsigned int *) &KeyBuffer[j]);

					KeyBuffer[j + KeyLength] = pDs->Rsa.RSAModulus[i];
					Endian_Convert(*(unsigned int *) &KeyBuffer[j + KeyLength],
							(unsigned int *) &KeyBuffer[j + KeyLength]);
				}
			} else {
				for (i = 0; i < (int) KeyLength; i++) {
					KeyBuffer[i] = pDs->Rsa.RSAPublicExponent[i];
					KeyBuffer[i + KeyLength] = pDs->Rsa.RSAModulus[i];
				}
			}
		} else if (TimDescriptor.getTimHeader().getVersion() >= TIM_3_4_00) {
			// >= TIM_3_4_00
			if (pDs->DSAlgorithmID == PKCS1_v1_5_Caddo) {
				// PKCS_1.5:Convert Public Exponent, swap LSW to MSW and to change Endian.
				for (i = 0, j = KeyLength - 1; i < KeyLength; i++, j--) {
					KeyBuffer[j] = pDs->Rsa.RSAPublicExponent[i];
					Endian_Convert(*(unsigned int *) &KeyBuffer[j],
							(unsigned int *) &KeyBuffer[j]);

					KeyBuffer[j + KeyLength] = pDs->Rsa.RSAModulus[i];
					Endian_Convert(*(unsigned int *) &KeyBuffer[j + KeyLength],
							(unsigned int *) &KeyBuffer[j + KeyLength]);
				}
			} else {
				// add scheme id to KeyBuffer if needed
				if (pDs->HashAlgorithmID == HASHALGORITHMID_T::SHA256)
					KeyBuffer[0] =
							(KeyLength == 64) ?
									PKCSv1_SHA256_2048RSA :
									PKCSv1_SHA256_1024RSA;
				else
					// Defaults to 160
					KeyBuffer[0] =
							(KeyLength == 64) ?
									PKCSv1_SHA1_2048RSA : PKCSv1_SHA1_1024RSA;

				for (i = 0; i < KeyLength; i++) {
					KeyBuffer[i + scheme] = pDs->Rsa.RSAModulus[i];
					KeyBuffer[i + KeyLength + scheme] =
							pDs->Rsa.RSAPublicExponent[i];
				}
			}
		}
	}
	else if (pDs->DSAlgorithmID == PKCS_v2_2_Ippcp) {
		KeyLength = pDs->KeySize / 32;
		// add scheme id to KeyBuffer if needed
		if (pDs->HashAlgorithmID == HASHALGORITHMID_T::SHA256)
			KeyBuffer[0] =
					(KeyLength == 64) ?
							PKCSv1_PSS_SHA256_2048RSA :
							PKCSv1_PSS_SHA256_1024RSA;
		else
			// Defaults to 160
			KeyBuffer[0] =
					(KeyLength == 64) ?
							PKCSv1_PSS_SHA1_2048RSA : PKCSv1_PSS_SHA1_1024RSA;

		for (i = 0; i < KeyLength; i++) {
			KeyBuffer[i + scheme] = pDs->Rsa.RSAModulus[i];
			KeyBuffer[i + KeyLength + scheme] = pDs->Rsa.RSAPublicExponent[i];
		}
	}
	else if (pDs->DSAlgorithmID == ECDSA_256
			|| pDs->DSAlgorithmID == ECDSA_521) {
		KeyLength = (pDs->KeySize + 31) / 32;

		// add scheme id to KeyBuffer if needed
		if (TimDescriptor.getTimHeader().getVersion() >= TIM_3_4_00) {
			if (pDs->HashAlgorithmID == HASHALGORITHMID_T::SHA512)
				KeyBuffer[0] =
						(KeyLength == 17) ?
								ECCP521_FIPS_DSA_SHA512 :
								ECCP256_FIPS_DSA_SHA512;
			else if (pDs->HashAlgorithmID == HASHALGORITHMID_T::SHA256)
				KeyBuffer[0] =
						(KeyLength == 17) ?
								ECCP521_FIPS_DSA_SHA256 :
								ECCP256_FIPS_DSA_SHA256;
			else
				// Defaults to 160
				KeyBuffer[0] =
						(KeyLength == 17) ?
								ECCP521_FIPS_DSA_SHA1 : ECCP256_FIPS_DSA_SHA1;
		}

		for (i = 0; i < (int) KeyLength; i++) {
			KeyBuffer[i + scheme] = pDs->Ecdsa.ECDSAPublicKeyCompX[i];
			KeyBuffer[i + KeyLength + scheme] =
					pDs->Ecdsa.ECDSAPublicKeyCompY[i];
		}
	}

	unsigned int OtpHashDataLength =
			CommandLineParser.bOtpHashPadding ?
					(516) : ((KeyLength * 2) + scheme) * 4;

	// Compute SHA hash.

	if (pDs->HashAlgorithmID == HASHALGORITHMID_T::SHA512)
		CryptoPP::SHA512().CalculateDigest((byte *) OtpHashBuffer,
				(unsigned char *) KeyBuffer, OtpHashDataLength);
	else if (pDs->HashAlgorithmID == HASHALGORITHMID_T::SHA256)
		CryptoPP::SHA256().CalculateDigest((byte *) OtpHashBuffer,
				(unsigned char *) KeyBuffer, OtpHashDataLength);
	else if (pDs->HashAlgorithmID == HASHALGORITHMID_T::SHA160)
		CryptoPP::SHA1().CalculateDigest((byte *) OtpHashBuffer,
				(unsigned char *) KeyBuffer, OtpHashDataLength);

	OutputOtpHash(OtpHashBuffer, pDs->HashAlgorithmID);

	//PKCS_1.5:Endian convert the hash and store it in the temp struct.
	if (pDs->DSAlgorithmID == PKCS1_v1_5_Caddo
			&& pDs->HashAlgorithmID != HASHALGORITHMID_T::SHA512) {
		// only room in the pDs->Hash for SHA160 and SHA256
		for (i = 0; i < SHAHashSize; i++) {
			Endian_Convert(OtpHashBuffer[i], &OtpHashBuffer[i]);
			// for CADDO only, we use the DS Hash array
			// all other algorithms have 0's written in the DS Hash array in the TIM.bin
			pDs->Hash[i] = (UINT_T) OtpHashBuffer[i];
		}
	}

	if (pDs->DSAlgorithmID == PKCS1_v1_5_Caddo) {
		// PKCS_1.5:Store Public exponent in temp struct
		// PKCS_1.5:Store Modulus in temp struct
		for (i = 0; i < KeyLength; i++) {
			pDs->Rsa.RSAPublicExponent[i] = (UINT_T) KeyBuffer[i];
			Endian_Convert(*(unsigned int *) &pDs->Rsa.RSAPublicExponent[i],
					(unsigned int *) &pDs->Rsa.RSAPublicExponent[i]);
			pDs->Rsa.RSAModulus[i] = (UINT_T) KeyBuffer[i + KeyLength];
			Endian_Convert(*(unsigned int *) &pDs->Rsa.RSAModulus[i],
					(unsigned int *) &pDs->Rsa.RSAModulus[i]);
		}
	}

	return true;
}

bool CTrustedImageBuilder::GenerateKeyHash(KEY_MOD_3_4_0* key) {
	unsigned int i = 0, j = 0;
	unsigned int KeyLength = key->KeySize / 32;
	unsigned int SHAHashSize =
			(key->HashAlgorithmID == HASHALGORITHMID_T::SHA512) ? 16 :
					(key->HashAlgorithmID == HASHALGORITHMID_T::SHA256) ? 8 : 5;

	//Max 2048 bit key * 2  (for exponent and modulus)
	// +32 is for schemeID in TIM_3_4_00
	unsigned int KeyData[((2048 * 2) + 32) / sizeof(unsigned int)] = { 0 };
	int scheme =
			TimDescriptorParser.TimDescriptor().getTimHeader().getVersion() >= TIM_3_4_00 ?
					1 : 0;

	int EncryptAlgo = key->EncryptAlgorithmID & ~EncryptedBitMask;
	if (TimDescriptorParser.TimDescriptor().getTimHeader().getVersion() < TIM_3_3_00) {
		CDigitalSignature Ds =
				TimDescriptorParser.TimDescriptor().DigitalSignature();
		if (Ds.DSKey()) {
			// old code depended on DS algorithm because key did not have an EncryptAlgorithmID
			key->EncryptAlgorithmID = Ds.DSKey()->EncryptAlgorithmId();
		}
	}

	// PKCS_1.5: The data read from the descriptor file is in LSW, Little Endian format.
	// Endian conversions and word swaps are required for PKCS 1.5 in order to make the
	// data compatible with the Fast signature PI.
	if (EncryptAlgo == PKCS1_v1_5_Caddo || EncryptAlgo == PKCS1_v1_5_Ippcp) {
		// PKCS_1.5: Store data in a temporary structure just for PKCS. This structure will
		// be written into TIM binary.
		KeyLength = key->KeySize / 32;
		if (TimDescriptorParser.TimDescriptor().getTimHeader().getVersion() < TIM_3_4_00) {
			if (key->EncryptAlgorithmID == PKCS1_v1_5_Caddo) {
				// Fill in the Exponent
				// Fill in the modulus
				for (i = 0; i < KeyLength; i++) {
					Endian_Convert(key->Rsa.RSAPublicExponent[i], &KeyData[i]);
					Endian_Convert(key->Rsa.RSAModulus[i],
							&KeyData[KeyLength + i]);
				}
			} else {
				// Fill in the Exponent
				// Fill in the modulus
				for (i = 0; i < KeyLength; i++) {
					KeyData[i] = key->Rsa.RSAPublicExponent[i];
					KeyData[KeyLength + i] = key->Rsa.RSAModulus[i];
				}
			}
		} else if (TimDescriptorParser.TimDescriptor().getTimHeader().getVersion() >= TIM_3_4_00) {
			if (EncryptAlgo == PKCS1_v1_5_Caddo) {
				// Fill in the Exponent
				// Fill in the modulus
				for (i = 0; i < KeyLength; i++) {
					Endian_Convert(key->Rsa.RSAPublicExponent[i], &KeyData[i]);
					Endian_Convert(key->Rsa.RSAModulus[i],
							&KeyData[KeyLength + i]);
				}
			} else {

				// >= TIM_3_4_00
				// add scheme id to KeyBuffer if needed
				if (key->HashAlgorithmID == HASHALGORITHMID_T::SHA256)
					KeyData[0] =
							(KeyLength == 64) ?
									PKCSv1_SHA256_2048RSA :
									PKCSv1_SHA256_1024RSA;
				else
					// Defaults to 160
					KeyData[0] =
							(KeyLength == 64) ?
									PKCSv1_SHA1_2048RSA : PKCSv1_SHA1_1024RSA;

				// Fill in the modulus
				// Fill in the Exponent
				// Note: modulus before exponent in post 3_4
				for (i = 0; i < KeyLength; i++) {
					KeyData[i + scheme] = key->Rsa.RSAModulus[i];
					KeyData[KeyLength + i + scheme] =
							key->Rsa.RSAPublicExponent[i];
				}
			}
		}
	} else if (EncryptAlgo == PKCS_v2_2_Ippcp) {
		if (key->HashAlgorithmID == HASHALGORITHMID_T::SHA256)
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
		for (i = 0; i < KeyLength; i++) {
			KeyData[i + scheme] = key->Rsa.RSAModulus[i];
			KeyData[KeyLength + i + scheme] = key->Rsa.RSAPublicExponent[i];
		}
	} else if (EncryptAlgo == ECDSA_256 || EncryptAlgo == ECDSA_521) {
		KeyLength = (key->KeySize + 31) / 32;

		// add scheme id to KeyBuffer if needed
		if (TimDescriptorParser.TimDescriptor().getTimHeader().getVersion() >= TIM_3_4_00) {
			if (key->HashAlgorithmID == HASHALGORITHMID_T::SHA256)
				KeyData[0] =
						(KeyLength == 17) ?
								ECCP521_FIPS_DSA_SHA256 :
								ECCP256_FIPS_DSA_SHA256;
			else
				// Defaults to 160
				KeyData[0] =
						(KeyLength == 17) ?
								ECCP521_FIPS_DSA_SHA1 : ECCP256_FIPS_DSA_SHA1;
		}

		for (i = 0; i < (int) KeyLength; i++) {
			KeyData[i + scheme] = key->Ecdsa.PublicKeyCompX[i];
			KeyData[i + KeyLength + scheme] = key->Ecdsa.PublicKeyCompY[i];
		}
	}

	if (key->HashAlgorithmID == HASHALGORITHMID_T::SHA512)
		CryptoPP::SHA512().CalculateDigest((unsigned char *) key->KeyHash,
				(unsigned char *) KeyData, ((KeyLength * 2) + scheme) * 4);
	else if (key->HashAlgorithmID == HASHALGORITHMID_T::SHA256)
		CryptoPP::SHA256().CalculateDigest((unsigned char *) key->KeyHash,
				(unsigned char *) KeyData, ((KeyLength * 2) + scheme) * 4);
	else
		CryptoPP::SHA1().CalculateDigest((unsigned char *) key->KeyHash,
				(unsigned char *) KeyData, ((KeyLength * 2) + scheme) * 4);

	// Do an endian conversion 1 word (4 bytes) at a time
	if (EncryptAlgo == PKCS1_v1_5_Caddo) {
		for (i = 0; i < SHAHashSize; i++)
			Endian_Convert(*(UINT_T *) (&key->KeyHash[i]),
					(UINT_T *) (&key->KeyHash[i]));
	}

	if (CommandLineParser.bVerbose) {
		string sKeyName = HexAsciiToText(HexFormattedAscii(key->KeyID));
		string sKeyFileName = sKeyName + "_KeyHash.txt";

		ofstream ofsKeyHash;
		stringstream ss;
		string value;
		string sKeyHashFilePath = sKeyFileName;
		PrependPathIfNone(sKeyHashFilePath);
		ofsKeyHash.open(sKeyHashFilePath.c_str(),
				ios_base::out | ios_base::trunc);
		for (i = 0; i < SHAHashSize; i++) {
			ss << HexFormattedAscii(key->KeyHash[i]) << endl;
		}
		ofsKeyHash << ss.str();
		ofsKeyHash.flush();
		ofsKeyHash.close();
	}

	return TRUE;
}

int CTrustedImageBuilder::GetTIMHashSize(PLAT_DS* pDs) {
	int iTotalHashSize = TimDescriptorParser.TimDescriptor().GetTimImageSize(
			CommandLineParser.bOneNANDPadding, CommandLineParser.uiPaddedSize);

	// when determining size of a TIM, handle the DS part
	if (pDs != 0) {
		// eliminate the modulus when calculating the tim hash
		int iExpHashSize = 0;
		if (pDs->DSAlgorithmID == ECDSA_256 || pDs->DSAlgorithmID == ECDSA_521)
			iExpHashSize += (sizeof(UINT_T)
					* ((MAXRSAKEYSIZEWORDS * 3) - (MAXECCKEYSIZEWORDS * 2)));
		else
			iExpHashSize += (sizeof(UINT_T) * (MAXRSAKEYSIZEWORDS));

		iTotalHashSize -= iExpHashSize;
	}

	return iTotalHashSize;
}

bool CTrustedImageBuilder::GenerateTIMHash(char *TIMBuffer, long lFileSize,
		unsigned char *pTIMHash, unsigned int TimSizeToHash) {
	if (!TimDescriptorParser.TimDescriptor().getTimHeader().getTrusted())
		return CImageBuilder::GenerateTIMHash(TIMBuffer, lFileSize, pTIMHash,
				TimSizeToHash);

	bool EnableEndianConvert = false;

	// A hash of the TIM minus the digital signature is generated by
	// this function.

	// determine if hash size is greater than the complete file size and adjust TimSizeToHash
	if ((long) TimSizeToHash > lFileSize)
		TimSizeToHash = lFileSize;

	PLAT_DS Ds =
			TimDescriptorParser.TimDescriptor().DigitalSignature().toPlatDs();
	//get the max size of the TIM to be hashed
	unsigned long TimSizeWithNoDS = GetTIMHashSize(&Ds);

	// determine if hash size is greater than the complete file size and adjust TimSizeToHash
	if (TimSizeToHash > TimSizeWithNoDS)
		TimSizeToHash = TimSizeWithNoDS;

	if (TimSizeToHash > 0) {

		unsigned char TIMHash[MAX_HASH] = { 0 };

		if (Ds.DSAlgorithmID == PKCS1_v1_5_Caddo)
			EnableEndianConvert = true;

		//endian convert TIM to big endian for IPP reasons(LSW->MSW)
		if (EnableEndianConvert) {
			unsigned int pad = sizeof(UINT_T) - (lFileSize % sizeof(UINT_T));
			for (unsigned int c = 0; c < TimSizeToHash; c += 4) {
				Endian_Convert(*(unsigned int *) &TIMBuffer[c],
						(unsigned int *) &TIMBuffer[c]);
				// if the last word does not use all bytes, need to shift bytes accordingly
				if (c + 4 > TimSizeToHash) {
					*(unsigned int*) &TIMBuffer[c] >>= pad * 8; // shift right all align data
				}
			}
		}

		memset(TIMHash, 0, sizeof(TIMHash)); //clear the hash buffers

		unsigned int SHAHashSize =
				(Ds.HashAlgorithmID == HASHALGORITHMID_T::SHA512) ? 16 :
						(Ds.HashAlgorithmID == HASHALGORITHMID_T::SHA256) ? 8 : 5;

		//Calculate the SHA hash of the TIM file
		if (Ds.HashAlgorithmID == HASHALGORITHMID_T::SHA512)
			CryptoPP::SHA512().CalculateDigest(TIMHash,
					(unsigned char *) TIMBuffer, TimSizeToHash);
		else if (Ds.HashAlgorithmID == HASHALGORITHMID_T::SHA256)
			CryptoPP::SHA256().CalculateDigest(TIMHash,
					(unsigned char *) TIMBuffer, TimSizeToHash);
		else if (Ds.HashAlgorithmID == HASHALGORITHMID_T::SHA160)
			CryptoPP::SHA1().CalculateDigest(TIMHash,
					(unsigned char*) TIMBuffer, TimSizeToHash);
		else {
			printf(
					"  Error: GenerateTIMHash() failed because Ds.HashAlgorithm for TIM Hash is not recognized.\n\n");
			return FALSE;
		}

		memcpy(pTIMHash, TIMHash, sizeof(TIMHash));
	}

	return TRUE;
}

bool CTrustedImageBuilder::DumpTIMHashToBinFile(PLAT_DS *pDs, unsigned char *pTIMHash,
		char *HashFileName) {
	ofstream ofsTIMHash;
	unsigned char TIMHash[MAX_HASH] = { 0 };

	memcpy(TIMHash, pTIMHash, (sizeof(unsigned char) * pDs->KeySize / 8));

	string sTIMHashFilePath = HashFileName;
	PrependPathIfNone(sTIMHashFilePath);
	ofsTIMHash.open(sTIMHashFilePath.c_str(),
			ios_base::out | ios_base::trunc | ios_base::binary);
	if (ofsTIMHash.bad() || ofsTIMHash.fail()) {
		printf("\n  Error: fopen failed to open file <%s>\n",
				sTIMHashFilePath.c_str());
		return FALSE;
	}

	ofsTIMHash.write((char*) TIMHash, (sizeof(unsigned char) * pDs->KeySize / 8));
	ofsTIMHash.close();

	return TRUE;
}

bool CTrustedImageBuilder::InstallDigitalSignature(fstream& fsFile) {
	PLAT_DS Ds =
			TimDescriptorParser.TimDescriptor().DigitalSignature().toPlatDs();

	if (!fsFile.is_open() || fsFile.bad() || fsFile.fail()) {
		printf(
				"////////////////////////////////////////////////////////////\n");
		printf("  Error: InstallDigitalSignature Error:\n");
		//		printf ("Cannot open file name %s !\n",fsFile.);
		printf(
				"////////////////////////////////////////////////////////////\n");
	} else {
		fsFile.seekg(0, ios_base::end);
		std::streamoff pos = fsFile.tellg();
		fsFile.clear();

		if (Ds.DSAlgorithmID == ECDSA_256 || Ds.DSAlgorithmID == ECDSA_521) {
			fsFile.write((char*) &Ds.Ecdsa.ECDSADigS_R,
					sizeof(Ds.Ecdsa.ECDSADigS_R) + sizeof(Ds.Ecdsa.ECDSADigS_S)
					+ sizeof(Ds.Ecdsa.Reserved));
		} else
			fsFile.write((char*) Ds.Rsa.RSADigS,
					sizeof(unsigned int) * MAXRSAKEYSIZEWORDS);
	}

	fsFile.flush();

	return fsFile.good();
}

bool CTrustedImageBuilder::OutputOtpHash(unsigned int OtpHashBuffer[],
		HASHALGORITHMID_T HashAlgorithmID) {
	string sHashValue;
	ofstream ofsOtpHash;
	unsigned int SHAHashSize =
			(HashAlgorithmID == HASHALGORITHMID_T::SHA512) ? 16 :
					(HashAlgorithmID == HASHALGORITHMID_T::SHA256) ? 8 : 5;

	if (CommandLineParser.bVerbose) {
		printf("\nOTP Hash\n");
		for (unsigned int i = 0; i < SHAHashSize; i++) {
			printf("0x%08X\n", OtpHashBuffer[i]);
		}

		string sOtpHashFilePath = "OtpHash.txt";
		PrependPathIfNone(sOtpHashFilePath);
		ofsOtpHash.open(sOtpHashFilePath.c_str(),
				ios_base::out | ios_base::trunc);
		if (ofsOtpHash.bad() || ofsOtpHash.fail()) {
			printf("\n  Error: fopen failed to open OtpHash.txt\n");
			return FALSE;
		}

		for (unsigned int i = 0; i < SHAHashSize; i++) {
			sHashValue = HexFormattedAscii(OtpHashBuffer[i]);
			sHashValue += "\n";
			ofsOtpHash.write(sHashValue.c_str(), (int) sHashValue.length());
		}

		ofsOtpHash.close();
	}

	return TRUE;
}

bool CTrustedImageBuilder::VerifyDS(CDigitalSignature& DSigOrig,
		fstream& fsTimBinFile, string& sProcessorType) {
	// algorithm:
	// read the TIM file contents
	// locate the TIM image structure
	// calculate the TIM image hash
	// compare the TIM image hash to the hash structure in the TIM binary
	long lFileSize = 0;
	char *TIMBuffer = NULL;
	bool isVerified;

	string lcTimTxtFilePath = CTimLib::ToLower(
			TimDescriptor.TimDescriptorFilePath());
	bool bIsBlf = lcTimTxtFilePath.find(".blf")
					== lcTimTxtFilePath.length() - 4;

	// no processor type in bin so use the one passed in
	TimDescriptor.ProcessorTypeStr(sProcessorType);

	if (!LoadTimBinFileToTimBuffer(fsTimBinFile, TIMBuffer, &lFileSize))
		return false;

	if (TIMBuffer == NULL) {
		delete[] TIMBuffer;
		return false;
	}

	printf("\nVerifying Digital Signature\n\n");

	unsigned int iPos = 0;
	if (!ParseTimHeader(iPos, TIMBuffer)) {
		printf(
				"////////////////////////////////////////////////////////////\n");
		printf("  Error: VerifyDS() couldn't parse NTIM binary file.\n");
		printf(
				"////////////////////////////////////////////////////////////\n");
		if (TIMBuffer != NULL)
			delete[] TIMBuffer;
		return false;
	}

	CImageDescription TimImage(bIsBlf, TimDescriptor);
	ParseImages(iPos, TIMBuffer, TimImage);

	if (TimDescriptor.getTimHeader().getNumKeys() > 0)
		ParseKeys(iPos, TIMBuffer);

	if (TimDescriptor.getTimHeader().getSizeOfReserved() > 0)
		ParseReservedData(iPos, TIMBuffer);

	SkipAllOnesPadding(iPos, TIMBuffer);

	if (!ParseDigitalSignature(iPos, TIMBuffer))
		return false;

	PLAT_DS BinDS = TimDescriptor.DigitalSignature().toPlatDs();
	unsigned int SHAHashSize =
			(BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA512) ? 16 :
					(BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA256) ? 8 : 5;

	if (BinDS.DSAlgorithmID == PKCS1_v1_5_Caddo) {
		// need to MSW->LSW and endian convert the DS key components before calculating the otphash
		unsigned int KeyLength = (BinDS.KeySize + 31) / 32;
		unsigned int KeyBuffer[MAXRSAKEYSIZEWORDS + 1] = { 0 };

		// PKCS_1.5:Convert Public Exponent, the modulus from LSW to MSW and to Big Endian.
		for (unsigned int i = 0, j = KeyLength - 1; i < KeyLength; i++, j--)
			KeyBuffer[j] = BinDS.Rsa.RSAPublicExponent[i];
		for (unsigned int i = 0; i < KeyLength; i++)
			BinDS.Rsa.RSAPublicExponent[i] = KeyBuffer[i];

		for (unsigned int i = 0, j = KeyLength - 1; i < KeyLength; i++, j--)
			KeyBuffer[j] = BinDS.Rsa.RSAModulus[i];
		for (unsigned int i = 0; i < KeyLength; i++)
			BinDS.Rsa.RSAModulus[i] = KeyBuffer[i];
	}

	unsigned int OtpHash[16] = { 0 };
	if (!CalculateOtpHash(&BinDS, TimDescriptor, OtpHash)) {
		printf(
				"////////////////////////////////////////////////////////////\n");
		printf("  Error: VerifyDS() failed to calculate OtpHash.\n");
		printf(
				"////////////////////////////////////////////////////////////\n");
		if (TIMBuffer != NULL)
			delete[] TIMBuffer;
		return false;
	}

	if (0 != memcmp(OrigOtpHashBuffer, OtpHash, sizeof(OrigOtpHashBuffer))) {
		//sizeof(BinDS.Hash) is used instead of sizeof(OtpHash) since Sizeof(OtpHash) = 16 and sizeof(BinDS.Hash) = 8; 
		//The DigitalSignature hash size is of 8 bytes. The hash size of message with SHA512 is 16 
		//which cannot be accomodated into the 8 bytes space in the Digital Signature structure. 
		//Hence only the first 8 bytes of SHA512 hash is used to verify with the hash in the 8 bytes space of Digital Signature
		printf(
				"////////////////////////////////////////////////////////////\n");
		printf(
				"  Error: VerifyDS() did not match OtpHash in DS with calculated OtpHash.\n");
		printf(
				"////////////////////////////////////////////////////////////\n");
		if (TIMBuffer != NULL)
			delete[] TIMBuffer;
		return false;
	}

	unsigned char CalcTIMHash[MAX_HASH] = { 0 };
	if (!GenerateTIMHash(TIMBuffer, lFileSize, CalcTIMHash,
			(unsigned int)GetTIMHashSize(&BinDS))) //TimImage.ImageSizeToHash()))
	{
		printf(
				"////////////////////////////////////////////////////////////\n");
		printf("  Error: GenerateTIMHash failed in VerifyDS()!\n");
		printf(
				"////////////////////////////////////////////////////////////\n");
		if (TIMBuffer != NULL)
			delete[] TIMBuffer;
		return false;
	}

	if (BinDS.DSAlgorithmID == ECDSA_256 || BinDS.DSAlgorithmID == ECDSA_521) {
		if (!verifyMessageUsingCryptoPPECDSA((unsigned char *)TIMBuffer,
				(unsigned int)GetTIMHashSize(&BinDS), DSigOrig))
			return false;
	} else if (BinDS.DSAlgorithmID == PKCS1_v1_5_Ippcp
			|| BinDS.DSAlgorithmID == PKCS_v2_2_Ippcp) {
		if (!verifyMessageUsingCryptoPPRSA((unsigned char *) TIMBuffer,
				(unsigned int) GetTIMHashSize(&BinDS), DSigOrig))
			return false;
	} else
		return false;

	printf("\nVerification of Digital Signature successful!\n\n");

	if (TIMBuffer != NULL)
		delete[] TIMBuffer;

	return true;
}

void CTrustedImageBuilder::ParseKeys(unsigned int& iPos, char* TimBuffer) {
	CKey* pKey = 0;
	if (TimDescriptor.getTimHeader().getTrusted()) {
		for (unsigned int i = 0; i < TimDescriptor.getTimHeader().getNumKeys(); i++) {
			if (TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
				pKey = new CRSAKey;
				pKey->KeyTag(
						HexFormattedAscii(*(unsigned int*) &TimBuffer[iPos]));
				iPos += 4;

				unsigned int HashAlgorithmID = *(unsigned int*) &TimBuffer[iPos];
				iPos += 4;
				pKey->HashAlgorithmId((HASHALGORITHMID_T) HashAlgorithmID);

				// convert bytes to bits when getting sizes
				pKey->KeySize(8 * (*(unsigned int*) &TimBuffer[iPos]));
				iPos += 4;
				pKey->PublicKeySize(8 * (*(unsigned int*) &TimBuffer[iPos]));
				iPos += 4;

				CRSAKey* pRSAKey = dynamic_cast<CRSAKey*>(pKey);
				if (pRSAKey) {
					ToListFromArray((unsigned int*) &TimBuffer[iPos],
							pRSAKey->PublicKeyExponentList(),
							pRSAKey->PublicKeyExponentList().size());
					// discard used and unused bytes from file because MAXRSAKEYSIZEWORDS were written
					// though not all bytes were actually used and should be 0's
					size_t discard = 4 * MAXRSAKEYSIZEWORDS;
					iPos += discard;

					ToListFromArray((unsigned int*) &TimBuffer[iPos],
							pRSAKey->RsaSystemModulusList(),
							pRSAKey->RsaSystemModulusList().size());
					// discard used and unused bytes from file because MAXRSAKEYSIZEWORDS were written
					// though not all bytes were actually used and should be 0's
					discard = 4 * MAXRSAKEYSIZEWORDS;
					iPos += discard;
				}

				UINT_T KeyHash[8] = { 0 }; // hash in binary is skipped over and discarded
				for (unsigned int j = 0; j < 8; j++) {
					KeyHash[j] = *(unsigned int*) &TimBuffer[iPos];
					iPos += 4;
				}

				TimDescriptor.KeyList().push_back(pKey);
			} else if (TimDescriptor.getTimHeader().getVersion() >= TIM_3_3_00) {
				string sKeyTag = HexFormattedAscii(
						*(unsigned int*) &TimBuffer[iPos]);
				iPos += 4;
				unsigned int HashAlgorithmID = *(unsigned int*) &TimBuffer[iPos];
				iPos += 4;
				unsigned int KeySize = (*(unsigned int*) &TimBuffer[iPos]);
				iPos += 4;
				unsigned int PublicKeySize = (*(unsigned int*) &TimBuffer[iPos]);
				iPos += 4;
				unsigned int EncryptAlgorithmID =
						*(unsigned int*) &TimBuffer[iPos];
				iPos += 4;

				// use EncryptAlgorithmID and bEncrypted to determine what kind of Key is needed
				if (EncryptAlgorithmID == ECDSA_256
						|| EncryptAlgorithmID == ECDSA_521)
					pKey = new CECCKey;
				else
					pKey = new CRSAKey;

				pKey->KeyTag(sKeyTag);
				pKey->HashAlgorithmId((HASHALGORITHMID_T) HashAlgorithmID);
				pKey->EncryptAlgorithmId(
						(ENCRYPTALGORITHMID_T) EncryptAlgorithmID);
				pKey->KeySize(KeySize);
				pKey->PublicKeySize(PublicKeySize);
				EncryptAlgorithmID &= ~EncryptedBitMask;
				if (EncryptAlgorithmID == ECDSA_256
						|| EncryptAlgorithmID == ECDSA_521) {

					CECCKey* pECCKey = dynamic_cast<CECCKey*>(pKey);
					if (pECCKey) {
						ToListFromArray((unsigned int*) &TimBuffer[iPos],
								pECCKey->ECDSAPublicKeyCompXList(),
								pECCKey->ECDSAPublicKeyCompXList().size());
						// discard used and unused bytes from file because MAXECCKEYSIZEWORDS were written
						// though not all bytes were actually used and should be 0's
						size_t discard = 4 * MAXECCKEYSIZEWORDS;
						iPos += discard;

						ToListFromArray((unsigned int*) &TimBuffer[iPos],
								pECCKey->ECDSAPublicKeyCompYList(),
								pECCKey->ECDSAPublicKeyCompYList().size());
						// discard used and unused bytes from file because MAXECCKEYSIZEWORDS were written
						// though not all bytes were actually used and should be 0's
						discard = 4 * MAXECCKEYSIZEWORDS;
						iPos += discard;

						// discard the reserved part of the key structure
						discard = 4
								* ((2 * MAXRSAKEYSIZEWORDS)
										- (2 * MAXECCKEYSIZEWORDS));
						iPos += discard;
					}

					TimDescriptor.KeyList().push_back(pKey);
				} else {
					CRSAKey* pRSAKey = dynamic_cast<CRSAKey*>(pKey);
					if (pRSAKey) {
						ToListFromArray((unsigned int*) &TimBuffer[iPos],
								pRSAKey->PublicKeyExponentList(),
								pRSAKey->PublicKeyExponentList().size());
						// discard used and unused bytes from file because MAXRSAKEYSIZEWORDS were written
						// though not all bytes were actually used and should be 0's
						size_t discard = 4 * MAXRSAKEYSIZEWORDS;
						iPos += discard;

						ToListFromArray((unsigned int*) &TimBuffer[iPos],
								pRSAKey->RsaSystemModulusList(),
								pRSAKey->RsaSystemModulusList().size());
						// discard used and unused bytes from file because MAXRSAKEYSIZEWORDS were written
						// though not all bytes were actually used and should be 0's
						discard = 4 * MAXRSAKEYSIZEWORDS;
						iPos += discard;
					}

					TimDescriptor.KeyList().push_back(pKey);
				}

				if (TimDescriptor.getTimHeader().getVersion() == TIM_3_3_00) {
					UINT_T KeyHash[8] = { 0 }; // hash in binary is skipped over and discarded
					for (unsigned int j = 0; j < 8; j++) {
						KeyHash[j] = *(unsigned int*) &TimBuffer[iPos];
						iPos += 4;
					}
				} else {
					UINT_T KeyHash[16] = { 0 }; // hash in binary is skipped over and discarded
					for (unsigned int j = 0; j < 16; j++) {
						KeyHash[j] = *(unsigned int*) &TimBuffer[iPos];
						iPos += 4;
					}
				}
			}
		}
	}
}

bool CTrustedImageBuilder::ParseReservedData(unsigned int& iPos,
		char* TimBuffer) {
	string lcTimTxtFilePath = CTimLib::ToLower(
			TimDescriptor.TimDescriptorFilePath());
	bool bIsBlf = lcTimTxtFilePath.find(".blf")
					== lcTimTxtFilePath.length() - 4;

	unsigned int ResAreaTag = *(unsigned int*) &TimBuffer[iPos];
	iPos += 4;
	if (ResAreaTag == WTPRESERVEDAREAID) {
		CExtendedReservedData& Ext = TimDescriptor.ExtendedReservedData();

		unsigned int uiNumPackages = *(unsigned int*) &TimBuffer[iPos];
		iPos += 4;
		unsigned int j = 0;
		unsigned fieldValue = 0;

		eProcessorType PT = TimDescriptor.ProcessorType();

		for (unsigned int i = 0; i < uiNumPackages; i++) {
			unsigned int uiPackageId = *(unsigned int*) &TimBuffer[iPos];
			iPos += 4;
			unsigned int uiPackageSize = *(unsigned int*) &TimBuffer[iPos];
			iPos += 4;

			// check if package size is extreme, indicating a parsing error or incorrect binary format
			if (uiPackageSize > 4096 * 4)
				return false;

			if (uiPackageId == TERMINATORID) {
				// done processing reserved data
				goto Term;
				break;
			}

			if (PT == PXA30x || PT == PXA31x || PT == PXA32x) {
				// make it an oem custom package instead
				CReservedPackageData* pCustom = new CReservedPackageData;
				pCustom->PackageIdTag(HexFormattedAscii(uiPackageId));
				pCustom->PackageId(
						HexAsciiToText(HexFormattedAscii(uiPackageId)));

				if (uiPackageSize > 0) {
					for (j = 0; j < (uiPackageSize - 8) / 4; j++) {
						string* pData = new string(
								HexFormattedAscii(
										*(unsigned int*) &TimBuffer[iPos]));
						iPos += 4;
						string* pComment = new string("");
						pCustom->AddData(pData, pComment);
					}
				}
				TimDescriptor.ReservedDataList().push_back(pCustom);
				continue;
			}

			if (PT == UNDEFINED || PT == PXA168 || PT == PXA91x || PT == PXA92x
					|| PT == PXA93x || PT == PXA94x || PT == PXA95x
					|| PT == ARMADA16x || PT == ARMADA610 || PT == ARMADA622
					|| PT == PXA2128) {
				switch (uiPackageId) {
				case CLKEID:
					ParseERDFields((uiPackageSize - 8) / (4 * 2), iPos,
							TimBuffer, Ext,
							&CExtendedReservedData::AddClockEnableField);
					continue;
					break;

				case DDRGID:
					ParseERDFields((uiPackageSize - 8) / (4 * 2), iPos,
							TimBuffer, Ext,
							&CExtendedReservedData::AddDDRGeometryField);
					continue;
					break;

				case DDRTID:
					ParseERDFields((uiPackageSize - 8) / (4 * 2), iPos,
							TimBuffer, Ext,
							&CExtendedReservedData::AddDDRTimingField);
					continue;
					break;

				case DDRCID:
					ParseERDFields((uiPackageSize - 8) / (4 * 2), iPos,
							TimBuffer, Ext,
							&CExtendedReservedData::AddDDRCustomField);
					continue;
					break;

				case FREQID:
					ParseERDFields((uiPackageSize - 8) / (4 * 2), iPos,
							TimBuffer, Ext,
							&CExtendedReservedData::AddFrequencyField);
					continue;
					break;

				case VOLTID:
					ParseERDFields((uiPackageSize - 8) / (4 * 2), iPos,
							TimBuffer, Ext,
							&CExtendedReservedData::AddVoltagesField);
					continue;
					break;

				case CMCCID:
					ParseERDFields((uiPackageSize - 8) / (4 * 2), iPos,
							TimBuffer, Ext,
							&CExtendedReservedData::AddConfigMemoryControlField);
					continue;
					break;
				}
			}

			if (PT == UNDEFINED || PT == ARMADA610 || PT == ARMADA622
					|| PT == PXA2128) {
				switch (uiPackageId) {
				case TZID:
					ParseERDFields((uiPackageSize - 8) / (4 * 2), iPos,
							TimBuffer, Ext,
							&CExtendedReservedData::AddTrustZoneField);
					continue;
					break;

				case TZON:
					ParseERDFields((uiPackageSize - 8) / (4 * 2), iPos,
							TimBuffer, Ext,
							&CExtendedReservedData::AddTrustZoneRegidField);
					continue;
					break;
				}
			}

			if (PT == UNDEFINED || PT == PXA168 || PT == ARMADA16x) {
				switch (uiPackageId) {
				case OPDIVID:
					ParseERDFields((uiPackageSize - 8) / (4 * 2), iPos,
							TimBuffer, Ext,
							&CExtendedReservedData::AddOpDivField);
					continue;
					break;

				case OPMODEID:
					ParseERDFields((uiPackageSize - 8) / (4 * 2), iPos,
							TimBuffer, Ext,
							&CExtendedReservedData::AddOpModeField);
					continue;
					break;
				}
			}

			if (PT == UNDEFINED || PT == ARMADA610 || PT == ARMADA622
					|| PT == PXA2128 || PT == PXA988 || PT == P88PA62_70
					|| PT == P88PA62_20 || PT == P88PA62_10 || PT == PXA1928
					|| PT == PXA1920 || PT == PXA2101 || PT == PXA192
					|| PT == PXA1986 || PT == PXA1U88 || PT == PXA1826
					|| PT == PXA1956 || PT == VEGA || PT == BOWIE
					|| PT == ULC2) {
				switch (uiPackageId) {
				case PINID: {
					CPin* pPin = new CPin;
					ParseERDBaseFields(pPin, (uiPackageSize - 8) / 4, iPos,
							TimBuffer);
					Ext.ErdVec.push_back(pPin);
					continue;
				}
				break;
				}
			}

			if (PT == UNDEFINED || PT == ARMADA610 || PT == ARMADA622
					|| PT == PXA2128 || PT == PXA988 || PT == PXA1701
					|| PT == PXA978 || PT == PXA1801 || PT == PXA1928
					|| PT == P88PA62_70 || PT == P88PA62_20 || PT == P88PA62_10
					|| PT == PXA988 || PT == PXA1920 || PT == PXA1986
					|| PT == PXA1U88 || PT == PXA1826 || PT == PXA1956
					|| PT == VEGA || PT == BOWIE || PT == ULC2) {
				switch (uiPackageId) {
				case IMAPID: {
					CImageMaps* pImageMaps = new CImageMaps;
					*(pImageMaps->FieldValues()[CImageMaps::NUM_MAPS]) =
							HexFormattedAscii(
									*(unsigned int*) &TimBuffer[iPos]);
					iPos += 4;
					if (uiPackageSize > 0) {
						if ((uiPackageSize - 8 - 4) / (4 * 5)
								== Translate(
										*(pImageMaps->FieldValues()[CImageMaps::NUM_MAPS]))) {
							for (j = 0; j < (uiPackageSize - 8 - 4) / (4 * 5);
									j++) {
								CImageMapInfo* pImageMap = new CImageMapInfo(
										bIsBlf);
								*(pImageMap->FieldValues()[CImageMapInfo::IMAGE_ID]) =
										HexFormattedAscii(
												*(unsigned int*) &TimBuffer[iPos]);
								iPos += 4;
								*(pImageMap->FieldValues()[CImageMapInfo::IMAGE_TYPE]) =
										HexFormattedAscii(
												*(unsigned int*) &TimBuffer[iPos]);
								iPos += 4;
								*(pImageMap->FieldValues()[CImageMapInfo::FLASH_ADDRESS_LO]) =
										HexFormattedAscii(
												*(unsigned int*) &TimBuffer[iPos]);
								iPos += 4;
								*(pImageMap->FieldValues()[CImageMapInfo::FLASH_ADDRESS_HI]) =
										HexFormattedAscii(
												*(unsigned int*) &TimBuffer[iPos]);
								iPos += 4;
								*(pImageMap->FieldValues()[CImageMapInfo::PARTITION_NUM]) =
										HexFormattedAscii(
												*(unsigned int*) &TimBuffer[iPos]);
								iPos += 4;

								pImageMaps->ImageMaps().push_back(pImageMap);
							}
						}
					}
					Ext.ErdVec.push_back(pImageMaps);
					continue;
				}
				break;
				}
			}

			if (PT == UNDEFINED) {
				switch (uiPackageId) {
				case ONDEMANDBOOTTID: {
					COnDemandBoot* pOnDemandBoot = new COnDemandBoot;
					*(pOnDemandBoot->FieldValues()[COnDemandBoot::MAGIC_WORD]) =
							HexFormattedAscii(
									*(unsigned int*) &TimBuffer[iPos]);
					iPos += 4;
					*(pOnDemandBoot->FieldValues()[COnDemandBoot::MAGIC_WORD_ADDRESS]) =
							HexFormattedAscii(
									*(unsigned int*) &TimBuffer[iPos]);
					iPos += 4;
					*(pOnDemandBoot->FieldValues()[COnDemandBoot::MAGIC_WORD_PARTITION]) =
							HexFormattedAscii(
									*(unsigned int*) &TimBuffer[iPos]);
					iPos += 4;
					*(pOnDemandBoot->FieldValues()[COnDemandBoot::NUM_ODBT_IMAGES]) =
							HexFormattedAscii(
									*(unsigned int*) &TimBuffer[iPos]);
					iPos += 4;
					if ((uiPackageSize - 8 - 4) / (4 * 2)
							== Translate(
									*(pOnDemandBoot->FieldValues()[COnDemandBoot::NUM_ODBT_IMAGES]))) {
						if (uiPackageSize > 0) {
							for (j = 0; j < (uiPackageSize - 8 - 4) / (4 * 2);
									j++) {
								string* pODBTImageID =
										new string(
												HexFormattedAscii(
														*(unsigned int*) &TimBuffer[iPos]));
								iPos += 4;
								pOnDemandBoot->ODBTImageVec.push_back(
										pODBTImageID);
							}
						}
					}
					Ext.ErdVec.push_back(pOnDemandBoot);
					continue;
				}
				break;
				}
			}

			switch (uiPackageId) {
			case GPIOID: {
				CGpioSet* pGpioSet = new CGpioSet;
				*(pGpioSet->FieldValues()[CGpioSet::NUM_GPIOS]) =
						HexFormattedAscii(*(unsigned int*) &TimBuffer[iPos]);
				iPos += 4;
				if ((uiPackageSize - 8 - 4) / (4 * 2)
						== Translate(
								*(pGpioSet->FieldValues()[CGpioSet::NUM_GPIOS]))) {
					if (uiPackageSize > 0) {
						for (j = 0; j < (uiPackageSize - 8 - 4) / (4 * 2);
								j++) {
							CGpio* pGpio = new CGpio;
							*(pGpio->FieldValues()[CGpio::ADDRESS]) =
									HexFormattedAscii(
											*(unsigned int*) &TimBuffer[iPos]);
							iPos += 4;
							*(pGpio->FieldValues()[CGpio::VALUE]) =
									HexFormattedAscii(
											*(unsigned int*) &TimBuffer[iPos]);
							iPos += 4;

							pGpioSet->GpiosList().push_back(pGpio);
						}
					}
				}
				Ext.ErdVec.push_back(pGpioSet);
				continue;
			}
			break;

			case USBVENDORREQ: {
				CUsbVendorReq* pUsbVendorReq = new CUsbVendorReq;
				ParseERDBaseFields(pUsbVendorReq, (uiPackageSize - 8) / 4, iPos,
						TimBuffer);
				Ext.ErdVec.push_back(pUsbVendorReq);
				continue;
			}
			break;

			case USBID: {
				CUsb* pUsb = new CUsb;
				ParseERDBaseFields(pUsb, (uiPackageSize - 8) / 4, iPos,
						TimBuffer);
				Ext.ErdVec.push_back(pUsb);
				continue;
			}
			break;

			case UARTID: {
				CUart* pUart = new CUart;
				ParseERDBaseFields(pUart, (uiPackageSize - 8) / 4, iPos,
						TimBuffer);
				Ext.ErdVec.push_back(pUart);
				continue;
			}
			break;

			// reserved data packages
			case AUTOBIND: {
				CAutoBind* pAutoBind = new CAutoBind;
				ParseERDBaseFields(pAutoBind, (uiPackageSize - 8) / 4, iPos,
						TimBuffer);
				Ext.ErdVec.push_back(pAutoBind);
				continue;
			}
			break;

			case RESUMEBLID: {
				CResumeDdr* pResumeDdr = new CResumeDdr;
				ParseERDBaseFields(pResumeDdr, (uiPackageSize - 8) / 4, iPos,
						TimBuffer);
				Ext.ErdVec.push_back(pResumeDdr);
				continue;
			}
			break;

			case ROMRESUMEID: {
				CROMResume* pROMResume = new CROMResume;
				ParseERDBaseFields(pROMResume, (uiPackageSize - 8) / 4, iPos,
						TimBuffer);
				Ext.ErdVec.push_back(pROMResume);
				continue;
			}
			break;

			case TBR_XFER: {
				CTBRXferSet* pTBRXferSet = new CTBRXferSet;
				*(pTBRXferSet->FieldValues()[CTBRXferSet::XFER_LOC]) =
						HexFormattedAscii(*(unsigned int*) &TimBuffer[iPos]);
				iPos += 4;
				*(pTBRXferSet->FieldValues()[CTBRXferSet::NUM_DATA_PAIRS]) =
						HexFormattedAscii(*(unsigned int*) &TimBuffer[iPos]);
				iPos += 4;
				if ((uiPackageSize - 8 - 8) / (4 * 2)
						== Translate(
								*(pTBRXferSet->FieldValues()[CTBRXferSet::NUM_DATA_PAIRS]))) {
					if (uiPackageSize > 0) {
						for (j = 0; j < (uiPackageSize - 8 - 8) / (4 * 2);
								j++) {
							CXfer* pXfer = new CXfer;
							*(pXfer->FieldValues()[CXfer::DATA_ID]) =
									HexFormattedAscii(
											*(unsigned int*) &TimBuffer[iPos]);
							iPos += 4;
							*(pXfer->FieldValues()[CXfer::LOCATION]) =
									HexFormattedAscii(
											*(unsigned int*) &TimBuffer[iPos]);
							iPos += 4;

							pTBRXferSet->XfersList().push_back(pXfer);
						}
					}
				}
				Ext.ErdVec.push_back(pTBRXferSet);
				continue;
			}
			break;

			case ESCAPESEQID: {
				CEscapeSeq* pEscapeSeq = new CEscapeSeq;
				ParseERDBaseFields(pEscapeSeq, (uiPackageSize - 8) / 4, iPos,
						TimBuffer);
				Ext.ErdVec.push_back(pEscapeSeq);
				continue;
			}
			break;

			case ESCSEQID_V2: {
				CEscapeSeqV2* pEscapeSeqV2 = new CEscapeSeqV2;
				*(pEscapeSeqV2->FieldValues()[CEscapeSeqV2::GPIO_NUM_FIELD]) =
						HexFormattedAscii(*(unsigned int*) &TimBuffer[iPos]);
				iPos += 4;
				*(pEscapeSeqV2->FieldValues()[CEscapeSeqV2::GPIO_TRIGGER_VALUE_FIELD]) =
						HexFormattedAscii(*(unsigned int*) &TimBuffer[iPos]);
				iPos += 4;
				*(pEscapeSeqV2->FieldValues()[CEscapeSeqV2::NUM_MFPR_PAIRS]) =
						HexFormattedAscii(*(unsigned int*) &TimBuffer[iPos]);
				iPos += 4;
				if ((uiPackageSize - 8 - 12) / (4 * 2)
						== Translate(
								*(pEscapeSeqV2->FieldValues()[CEscapeSeqV2::NUM_MFPR_PAIRS]))) {
					if (uiPackageSize > 0) {
						for (j = 0; j < (uiPackageSize - 8 - 12) / (4 * 2);
								j++) {
							CMFPRPair* pMFPRPair = new CMFPRPair;
							*(pMFPRPair->FieldValues()[CMFPRPair::MFPR_ADDRESS_FIELD]) =
									HexFormattedAscii(
											*(unsigned int*) &TimBuffer[iPos]);
							iPos += 4;
							*(pMFPRPair->FieldValues()[CMFPRPair::MFPR_VALUE_FIELD]) =
									HexFormattedAscii(
											*(unsigned int*) &TimBuffer[iPos]);
							iPos += 4;

							pEscapeSeqV2->MFPRPairs().push_back(pMFPRPair);
						}
					}
				}
				Ext.ErdVec.push_back(pEscapeSeqV2);
				continue;
			}
			break;

			case COREID: {
				CCoreReset* pCoreReset = new CCoreReset;
				ParseERDBaseFields(pCoreReset, (uiPackageSize - 8) / 4, iPos,
						TimBuffer);
				Ext.ErdVec.push_back(pCoreReset);
				continue;
			}
			break;

			case COREID_V2: {
				CCoreRelease* pCoreRelease = new CCoreRelease;
				*(pCoreRelease->FieldValues()[CCoreRelease::NUM_PAIRS_FIELD]) =
						HexFormattedAscii(*(unsigned int*) &TimBuffer[iPos]);
				iPos += 4;
				if ((uiPackageSize - 8 - 4) / (4 * 2)
						== Translate(
								*(pCoreRelease->FieldValues()[CCoreRelease::NUM_PAIRS_FIELD]))) {
					if (uiPackageSize > 0) {
						for (j = 0; j < (uiPackageSize - 8 - 4) / (4 * 2);
								j++) {
							CCorePair* pCorePair = new CCorePair;
							*(pCorePair->FieldValues()[CCorePair::CORE_ID_V2_FIELD]) =
									HexFormattedAscii(
											*(unsigned int*) &TimBuffer[iPos]);
							iPos += 4;
							*(pCorePair->FieldValues()[CCorePair::VECTOR_ADDRESS_FIELD]) =
									HexFormattedAscii(
											*(unsigned int*) &TimBuffer[iPos]);
							iPos += 4;

							pCoreRelease->CorePairsList().push_back(pCorePair);
						}
					}
				}
				Ext.ErdVec.push_back(pCoreRelease);
				continue;
			}
			break;

			case BBMTID: {
				CBbmt* pBBMT = new CBbmt;
				ParseERDBaseFields(pBBMT, (uiPackageSize - 8) / 4, iPos,
						TimBuffer);
				Ext.ErdVec.push_back(pBBMT);
				continue;
			}
			break;

			// use default reserved data for these and treat as a custom package
			case USB_DEVICE_DESCRIPTOR:
			case USB_CONFIG_DESCRIPTOR:
			case USB_INTERFACE_DESCRIPTOR:
			case USB_LANGUAGE_STRING_DESCRIPTOR:
			case USB_MANUFACTURER_STRING_DESCRIPTOR:
			case USB_PRODUCT_STRING_DESCRIPTOR:
			case USB_SERIAL_STRING_DESCRIPTOR:
			case USB_INTERFACE_STRING_DESCRIPTOR:
			case USB_DEFAULT_STRING_DESCRIPTOR:
			case USB_ENDPOINT_DESCRIPTOR:
			case OEMCUSTOMID:
			case NOMONITORID:
			case DDRID:
			case FLASHGEOPKGID:
			default: {
				if (PT != UNDEFINED) {
					// make it an oem custom package instead
					CReservedPackageData* pCustom = new CReservedPackageData;
					pCustom->PackageId(
							HexAsciiToText(HexFormattedAscii(uiPackageId)));
					pCustom->PackageIdTag(HexFormattedAscii(uiPackageId));

					if (uiPackageSize > 0) {
						for (j = 0; j < (uiPackageSize - 8) / 4; j++) {
							string* pData = new string(
									HexFormattedAscii(
											*(unsigned int*) &TimBuffer[iPos]));
							iPos += 4;
							string* pComment = new string("");
							pCustom->AddData(pData, pComment);
						}
					}
					TimDescriptor.ReservedDataList().push_back(pCustom);
					continue;
				}
			}
			break;

			}

			if (PT == UNDEFINED || PT == ARMADA610 || PT == ARMADA622
					|| PT == PXA2128 || PT == PXA988 || PT == PXA1701
					|| PT == PXA1202 || PT == PXA978 || PT == PXA1801
					|| PT == PXA92x || PT == PXA1986) {
				if ((uiPackageId == CIDPID)) {
					unsigned int nConsumers = TimBuffer[iPos];
					iPos += 4;
					for (unsigned int k = 0; k < nConsumers; k++) {
						CConsumerID* pCidp = new CConsumerID;
						*(pCidp->FieldValues()[CConsumerID::CID]) =
								HexFormattedAscii(
										*(unsigned int*) &TimBuffer[iPos]);
						iPos += 4;

						unsigned int nPids = TimBuffer[iPos];
						iPos += 4;
						for (unsigned int n = 0; n < nPids; n++) {
							string* sPid = new string(
									HexFormattedAscii(
											*(unsigned int*) &TimBuffer[iPos]));
							iPos += 4;
							pCidp->m_PIDs.push_back(sPid);
						}
						Ext.m_Consumers.push_back(pCidp);
					}
					continue;
				}

				// check for DDRn
				if ((uiPackageId & 0xffffff00) >> 8 == DDRTYPE) {
					stringstream ss;
					ss << (char) (uiPackageId & 0x000000ff);
					if (IsAlphaNumeric(ss.str())) {
						// process DDRn packages apart from all others that are DDR[alpha] packages
						CDDRInitialization* pDDRn = new CDDRInitialization;
						*(pDDRn->FieldValues()[CDDRInitialization::DDR_PID]) =
								HexFormattedAscii(uiPackageId);
						pDDRn->m_sDdrPID =
								HexAsciiToText(
										*pDDRn->FieldValues()[CDDRInitialization::DDR_PID]);

						unsigned int iOperations =
								*(unsigned int*) &TimBuffer[iPos];
						iPos += 4;
						unsigned int iInstructions =
								*(unsigned int*) &TimBuffer[iPos];
						iPos += 4;

						for (j = 0; j < iOperations; j++) {
							CDDROperation* pOperation = new CDDROperation;
							pOperation->SetOperationID(
									(DDR_OPERATION_SPEC_T) (*(unsigned int*) &TimBuffer[iPos]));
							iPos += 4;
							pOperation->m_Value =
									*(unsigned int*) &TimBuffer[iPos];
							iPos += 4;
							pDDRn->m_DdrOperations.m_DdrOperationsList.push_back(
									pOperation);
						}

						for (j = 0; j < iInstructions; j++) {
							CInstruction* pInstruction = new CInstruction;
							pInstruction->SetInstructionType(
									(INSTRUCTION_OP_CODE_SPEC_T) (*(unsigned int*) &TimBuffer[iPos]));
							iPos += 4;
							for (unsigned int k = 0;
									k < pInstruction->m_NumParamsUsed; k++) {
								pInstruction->m_ParamValues[k] =
										*(unsigned int*) &TimBuffer[iPos];
								iPos += 4;
							}
							pDDRn->m_DdrInstructions.m_InstructionsList.push_back(
									pInstruction);
						}

						Ext.ErdVec.push_back(pDDRn);
						continue;
					}
				}

				// check for TZRn
				if ((uiPackageId & 0xffffff00) >> 8 == TZRTYPE) {
					stringstream ss;
					ss << (char) (uiPackageId & 0x000000ff);
					if (IsAlphaNumeric(ss.str())) {
						CTzInitialization* pTzInit = new CTzInitialization;
						*(pTzInit->FieldValues()[CTzInitialization::TZ_PID]) =
								HexFormattedAscii(uiPackageId);
						pTzInit->m_sTzPID =
								HexAsciiToText(
										*pTzInit->FieldValues()[CTzInitialization::TZ_PID]);

						unsigned int iOperations =
								*(unsigned int*) &TimBuffer[iPos];
						iPos += 4;
						unsigned int iInstructions =
								*(unsigned int*) &TimBuffer[iPos];
						iPos += 4;

						for (j = 0; j < iOperations; j++) {
							CTzOperation* pOperation = new CTzOperation;
							pOperation->SetOperationID(
									(TZ_OPERATION_SPEC_T) (*(unsigned int*) &TimBuffer[iPos]));
							iPos += 4;
							pOperation->m_Value =
									*(unsigned int*) &TimBuffer[iPos];
							iPos += 4;
							pTzInit->m_TzOperations.m_TzOperationsList.push_back(
									pOperation);
						}

						for (j = 0; j < iInstructions; j++) {
							CInstruction* pInstruction = new CInstruction;
							pInstruction->SetInstructionType(
									(INSTRUCTION_OP_CODE_SPEC_T) (*(unsigned int*) &TimBuffer[iPos]));
							iPos += 4;
							for (unsigned int k = 0;
									k < pInstruction->m_NumParamsUsed; k++) {
								pInstruction->m_ParamValues[k] =
										*(unsigned int*) &TimBuffer[iPos];
								iPos += 4;
							}
							pTzInit->m_TzInstructions.m_InstructionsList.push_back(
									pInstruction);
						}

						Ext.ErdVec.push_back(pTzInit);
						continue;
					}
				}

				// check for GPPn
				if ((uiPackageId & 0xffffff00) >> 8 == GPPTYPE) {
					stringstream ss;
					ss << (char) (uiPackageId & 0x000000ff);
					if (IsAlphaNumeric(ss.str())) {
						CGenPurposePatch* pGPP = new CGenPurposePatch;
						*(pGPP->FieldValues()[CGenPurposePatch::GPP_PID]) =
								HexFormattedAscii(uiPackageId);
						pGPP->m_sGPP_PID =
								HexAsciiToText(
										*pGPP->FieldValues()[CGenPurposePatch::GPP_PID]);

						unsigned int iOperations =
								*(unsigned int*) &TimBuffer[iPos];
						iPos += 4;
						unsigned int iInstructions =
								*(unsigned int*) &TimBuffer[iPos];
						iPos += 4;

						for (j = 0; j < iOperations; j++) {
							CGPPOperation* pOperation = new CGPPOperation;
							pOperation->SetOperationID(
									(GPP_OPERATION_SPEC_T) (*(unsigned int*) &TimBuffer[iPos]));
							iPos += 4;
							pOperation->m_Value =
									*(unsigned int*) &TimBuffer[iPos];
							iPos += 4;
							pGPP->m_GPPOperations.m_GPPOperationsList.push_back(
									pOperation);
						}

						for (j = 0; j < iInstructions; j++) {
							CInstruction* pInstruction = new CInstruction;
							pInstruction->SetInstructionType(
									(INSTRUCTION_OP_CODE_SPEC_T) (*(unsigned int*) &TimBuffer[iPos]));
							iPos += 4;
							for (unsigned int k = 0;
									k < pInstruction->m_NumParamsUsed; k++) {
								pInstruction->m_ParamValues[k] =
										*(unsigned int*) &TimBuffer[iPos];
								iPos += 4;
							}
							pGPP->m_GPPInstructions.m_InstructionsList.push_back(
									pInstruction);
						}

						Ext.ErdVec.push_back(pGPP);
						continue;
					}
				}
			}

			// last chance to deal with the package
			if (PT == UNDEFINED) {
				// make it an oem custom package instead
				CReservedPackageData* pCustom = new CReservedPackageData;
				pCustom->PackageId(
						HexAsciiToText(HexFormattedAscii(uiPackageId)));
				pCustom->PackageIdTag(HexFormattedAscii(uiPackageId));

				if (uiPackageSize > 0) {
					for (j = 0; j < (uiPackageSize - 8) / 4; j++) {
						string* pData = new string(
								HexFormattedAscii(
										*(unsigned int*) &TimBuffer[iPos]));
						iPos += 4;
						string* pComment = new string("");
						pCustom->AddData(pData, pComment);
					}
				}
				TimDescriptor.ReservedDataList().push_back(pCustom);
				continue;
			}
		}
	}

	Term:

	// update tim header SizeOfReservedData based on actual size
	// some reserved data may have been parsed as ExtendedReservedData
	// and if there was any all ones padding, the padding is not included in TIM.txt
	// but will be included in the TIM.bin only if the TBB -O switch is used
	TimDescriptor.getTimHeader().setSizeOfReserved(
			TimDescriptor.ReservedDataTotalSize());

	return true;
}

bool CTrustedImageBuilder::ParseDigitalSignature(unsigned int& iPos,
		char* TimBuffer) {
	size_t discard = 0;

	if (TimDescriptor.getTimHeader().getTrusted()) {
		CDigitalSignature& DSig = TimDescriptor.DigitalSignature();
		DSig.IncludeInTim(true);

		unsigned int iDSAlgorithmID = *(unsigned int*) &TimBuffer[iPos];
		iPos += 4;
		if (iDSAlgorithmID == ECDSA_256 || iDSAlgorithmID == ECDSA_521)
			DSig.DSKey(new CECCKey);
		else
			DSig.DSKey(new CRSAKey);

		DSig.DSKey()->EncryptAlgorithmId((ENCRYPTALGORITHMID_T) iDSAlgorithmID);

		unsigned int HashAlgorithmID = *(unsigned int*) &TimBuffer[iPos];
		iPos += 4;
		DSig.DSKey()->HashAlgorithmId((HASHALGORITHMID_T) HashAlgorithmID);

		if ((int) TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
			// size is in bytes prior to version 3.3
			DSig.KeySize((*(unsigned int*) &TimBuffer[iPos]) * 8);
			iPos += 4;
		} else {
			// size is in bits after version 3.3
			DSig.KeySize((*(unsigned int*) &TimBuffer[iPos]));
			iPos += 4;
		}

		// OtpHash in binary is extracted to a separate array
		for (unsigned int i = 0; i < 8; i++) {
			// discarding unused OtpHash in DS because hash is not stored in TIM.bin
			//*(unsigned int*)&TimBuffer[ iPos ];
			iPos += 4;
		}

		if (TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
			if (DSig.DSKey()) {
				if (DSig.DSKey()->EncryptAlgorithmId() == ECDSA_256
						|| DSig.DSKey()->EncryptAlgorithmId() == ECDSA_521)
					return false;

				CRSAKey* pRSAKey = dynamic_cast<CRSAKey*>(DSig.DSKey());
				ToListFromArray((unsigned int*) &TimBuffer[iPos],
						pRSAKey->PublicKeyExponentList(),
						pRSAKey->PublicKeyExponentList().size());
				// discard used and unused bytes from file because MAXRSAKEYSIZEWORDS were written
				// though not all bytes were actually used and should be 0's
				discard = 4 * MAXRSAKEYSIZEWORDS;
				iPos += discard;

				ToListFromArray((unsigned int*) &TimBuffer[iPos],
						pRSAKey->RsaSystemModulusList(),
						pRSAKey->RsaSystemModulusList().size());

				// discard used and unused bytes from file because MAXRSAKEYSIZEWORDS were written
				// though not all bytes were actually used and should be 0's
				discard = 4 * MAXRSAKEYSIZEWORDS;
				iPos += discard;
			}

			ToListFromArray((unsigned int*) &TimBuffer[iPos],
					DSig.RsaDigSList(), DSig.RsaDigSList().size());
			// discard used and unused bytes from file because MAXRSAKEYSIZEWORDS were written
			// though not all bytes were actually used and should be 0's
			discard = 4 * MAXRSAKEYSIZEWORDS;
			iPos += discard;
		} else if (TimDescriptor.getTimHeader().getVersion() >= TIM_3_3_00) {
			if (DSig.DSKey()->EncryptAlgorithmId() == ECDSA_256
					|| DSig.DSKey()->EncryptAlgorithmId() == ECDSA_521) {
				CECCKey* pECCKey = dynamic_cast<CECCKey*>(DSig.DSKey());
				ToListFromArray((unsigned int*) &TimBuffer[iPos],
						pECCKey->ECDSAPublicKeyCompXList(),
						pECCKey->ECDSAPublicKeyCompXList().size());
				// discard used and unused bytes from file because MAXECCKEYSIZEWORDS were written
				// though not all bytes were actually used and should be 0's
				discard = 4 * MAXECCKEYSIZEWORDS;
				iPos += discard;

				ToListFromArray((unsigned int*) &TimBuffer[iPos],
						pECCKey->ECDSAPublicKeyCompYList(),
						pECCKey->ECDSAPublicKeyCompYList().size());
				// discard used and unused bytes from file because MAXECCKEYSIZEWORDS were written
				// though not all bytes were actually used and should be 0's
				discard = 4 * MAXECCKEYSIZEWORDS;
				iPos += discard;

				ToListFromArray((unsigned int*) &TimBuffer[iPos],
						DSig.ECDigS_RList(), DSig.ECDigS_RList().size());
				// discard used and unused bytes from file because MAXECCKEYSIZEWORDS were written
				// though not all bytes were actually used and should be 0's
				discard = 4 * MAXECCKEYSIZEWORDS;
				iPos += discard;

				ToListFromArray((unsigned int*) &TimBuffer[iPos],
						DSig.ECDigS_SList(), DSig.ECDigS_SList().size());
				// discard used and unused bytes from file because MAXRSAKEYSIZEWORDS were written
				// though not all bytes were actually used and should MAXECCKEYSIZEWORDS 0's
				discard = 4 * MAXECCKEYSIZEWORDS;
				iPos += discard;

				// discard the reserved part of the key structure
				discard = 4
						* ((3 * MAXRSAKEYSIZEWORDS) - (4 * MAXECCKEYSIZEWORDS));
				iPos += discard;
			} else {
				CRSAKey* pRSAKey = dynamic_cast<CRSAKey*>(DSig.DSKey());
				ToListFromArray((unsigned int*) &TimBuffer[iPos],
						pRSAKey->PublicKeyExponentList(),
						pRSAKey->PublicKeyExponentList().size());
				// discard used and unused bytes from file because MAXRSAKEYSIZEWORDS were written
				// though not all bytes were actually used and should be 0's
				discard = 4 * MAXRSAKEYSIZEWORDS;
				iPos += discard;

				ToListFromArray((unsigned int*) &TimBuffer[iPos],
						pRSAKey->RsaSystemModulusList(),
						pRSAKey->RsaSystemModulusList().size());
				// discard use dand unused bytes from file because MAXRSAKEYSIZEWORDS were written
				// though not all bytes were actually used and should be 0's
				discard = 4 * MAXRSAKEYSIZEWORDS;
				iPos += discard;

				ToListFromArray((unsigned int*) &TimBuffer[iPos],
						DSig.RsaDigSList(), DSig.RsaDigSList().size());
				// discard used and unused bytes from file because MAXRSAKEYSIZEWORDS were written
				// though not all bytes were actually used and should be 0's
				discard = 4 * MAXRSAKEYSIZEWORDS;
				iPos += discard;
			}
		}
	}
	return true;
}

void CTrustedImageBuilder::SkipAllOnesPadding(unsigned int& iPos,
		char* TimBuffer) {
	while (*(unsigned int*) &TimBuffer[iPos] == 0xffffffff) {
		iPos += 4;
	}
}

void CTrustedImageBuilder::ParseERDFields(unsigned int uiLoop,
		unsigned int& iPos, char* TimBuffer, CExtendedReservedData& Ext,
		void (CExtendedReservedData::*AddField)(
				std::pair<unsigned int, unsigned int>*&)) {
	for (unsigned int j = 0; j < uiLoop; j++) {
		unsigned int iField = *(unsigned int*) &TimBuffer[iPos];
		iPos += 4;
		unsigned int iFieldValue = *(unsigned int*) &TimBuffer[iPos];
		iPos += 4;
		std::pair<unsigned int, unsigned int>* pPair = new std::pair<
				unsigned int, unsigned int>(iField, iFieldValue);
		(Ext.*AddField)(pPair);
	}
}

void CTrustedImageBuilder::ParseERDBaseFields(CErdBase* pERD,
		unsigned int uiLoop, unsigned int& iPos, char* TimBuffer) {
	for (unsigned int j = 0; j < uiLoop; j++) {
		*(pERD->FieldValues()[j]) = HexFormattedAscii(
				*(unsigned int*) &TimBuffer[iPos]);
		iPos += 4;
	}
}

bool CTrustedImageBuilder::GenerateKey(CCommandLineParser& CommandLineParser,
		CKeyGenSpec& KeyGenSpec) {
	CKey* pKey = 0;
	bool bRet = true;
	if (KeyGenSpec.EncryptAlgorithmId() == ECDSA_256
			|| KeyGenSpec.EncryptAlgorithmId() == ECDSA_521)
		pKey = new CECCKey;
	else
		pKey = new CRSAKey;

	if (!KeyGenSpec.GenerateKey(*pKey))
		bRet = false;

	if (!KeyGenSpec.OutputKeyText(KeyGenSpec.OutputTextFilename(), *pKey))
		bRet = false;

	if (!KeyGenSpec.OutputKeyBinary(KeyGenSpec.OutputBinaryFilename(), *pKey))
		bRet = false;

	if (!KeyGenSpec.GenerateHash(KeyGenSpec.OutputHashBinaryFilename(), *pKey))
		bRet = false;

	delete pKey;

	return bRet;
}

bool CTrustedImageBuilder::CalcFileHash(string& sFileName, unsigned char* HashAry,
		HASHALGORITHMID_T algo) {
	ifstream ifsFile;
	ifsFile.open(sFileName.c_str(), ios_base::in | ios_base::binary);
	if (ifsFile.fail() || ifsFile.bad()) {
		printf("\n  Error: Cannot open file name <%s> !", sFileName.c_str());
		return false;
	}

	// get the file length for allocating a buffer to hold the file
	ifsFile.seekg(0, ios_base::end);
	ifsFile.clear();
	unsigned int ifsPos = (long) ifsFile.tellg();
	ifsFile.seekg(0, ios_base::beg);
	if (ifsPos <= 0) {
		printf("\n  Error: File is empty! <%s>", sFileName.c_str());
		return false;
	}

	// alloc a data array to load the file
	char* FileData = new char[ifsPos];
	memset(FileData, 0, ifsPos);

	ifsFile.read(FileData, ifsPos);
	if (ifsFile.fail()) {
		printf("\n  Error: Failed to read all of file! <%s>",
				sFileName.c_str());
		delete[] FileData;
		return false;
	}

	// Calculate the SHA hash of the TIM file
	if (algo == HASHALGORITHMID_T::SHA512)
		CryptoPP::SHA512().CalculateDigest(HashAry, (unsigned char *) FileData,
				ifsPos);
	else if (algo == HASHALGORITHMID_T::SHA256)
		CryptoPP::SHA256().CalculateDigest(HashAry, (unsigned char *) FileData,
				ifsPos);
	else if (algo == HASHALGORITHMID_T::SHA160)
		CryptoPP::SHA1().CalculateDigest(HashAry, (unsigned char *) FileData,
				ifsPos);

	delete[] FileData;

	return true;
}

bool CTrustedImageBuilder::signMessageUsingCryptoPPRSA(unsigned char * msg,
		UINT_T msgSize, CDigitalSignature &CDigS) {
	UINT_T* RSA_D = NULL;	//Private Exponent
	UINT_T* RSA_E = NULL;	//Public Exponent
	UINT_T* RSA_N = NULL;	//Public Modulus
	byte* sign = NULL;
	bool retVal = true;
	int KeySizeInWords = CDigS.DSKey()->KeySize() / 32;

	RSA_D = new UINT_T[KeySizeInWords];
	RSA_E = new UINT_T[KeySizeInWords];
	RSA_N = new UINT_T[KeySizeInWords];
	sign = new byte[KeySizeInWords * 4];
	std::memset(RSA_D, 0, KeySizeInWords * 4);
	std::memset(RSA_E, 0, KeySizeInWords * 4);
	std::memset(RSA_N, 0, KeySizeInWords * 4);
	std::memset(sign, 0, KeySizeInWords * 4);

	CDigS.DSKey()->PubCompEToArray(RSA_E, KeySizeInWords); //Exponent is one word long
	CDigS.DSKey()->PubCompDToArray(RSA_N, KeySizeInWords); //KeySize is in Bits
	CDigS.DSKey()->PrivateToArray(RSA_D, KeySizeInWords);

	CryptoPP::AutoSeededRandomPool rng;
	UINT_T RsaPrivateKeyReversed[MAXRSAKEYSIZEWORDS] = { 0 };
	UINT_T RsaModulusReversed[MAXRSAKEYSIZEWORDS] = { 0 };
	UINT_T RsaPublicKeyReversed[MAXRSAKEYSIZEWORDS] = { 0 };
	PLAT_DS BinDS = CDigS.toPlatDs();
	for (int i = 0; i < KeySizeInWords; ++i) {
		Endian_Convert(RSA_D[(BinDS.KeySize / 32) - 1 - i], &RsaPrivateKeyReversed[i]);
		Endian_Convert(RSA_N[(BinDS.KeySize / 32) - 1 - i], &RsaModulusReversed[i]);
		Endian_Convert(RSA_E[(BinDS.KeySize / 32) - 1 - i], &RsaPublicKeyReversed[i]);

	}

	CryptoPP::Integer n((const byte *) RsaModulusReversed,
			(int) (BinDS.KeySize / 8));
	CryptoPP::Integer d((const byte *) RsaPrivateKeyReversed,
			(int) (BinDS.KeySize / 8));
	CryptoPP::Integer e((const byte *) RsaPublicKeyReversed,
			(int) (BinDS.KeySize / 8));

	CryptoPP::RSA::PrivateKey privateKey;
	privateKey.Initialize(n, e, d);
	retVal = privateKey.Validate(rng, 3);

	if (retVal && BinDS.DSAlgorithmID == PKCS1_v1_5_Ippcp) {
		if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA512) {
			CryptoPP::RSASS_Hash<CryptoPP::PKCS1v15_Hash, CryptoPP::SHA512>::SignerHash signer(
					privateKey);
			signer.SignHash(rng, (const byte*) msg, sign);
		} else if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA256) {
			CryptoPP::RSASS_Hash<CryptoPP::PKCS1v15_Hash, CryptoPP::SHA256>::SignerHash signer(
					privateKey);
			signer.SignHash(rng, (const byte*) msg, sign);

		} else if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA160) {
			CryptoPP::RSASS_Hash<CryptoPP::PKCS1v15_Hash, CryptoPP::SHA1>::SignerHash signer(
					privateKey);
			signer.SignHash(rng, (const byte*) msg, sign);
		} else
			retVal = false;

	} else if (retVal && BinDS.DSAlgorithmID == PKCS_v2_2_Ippcp) {
		if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA512) {
			CryptoPP::RSASS_Hash<CryptoPP::PSS_Hash, CryptoPP::SHA512>::SignerHash signer(
					privateKey);
			signer.SignHash(rng, (const byte*) msg, sign);
		} else if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA256) {
			byte hash[64] = { 0 };
			CryptoPP::RSASS_Hash<CryptoPP::PSS_Hash, CryptoPP::SHA256>::SignerHash signer(
					privateKey);
			signer.SignHash(rng, (const byte*) msg, sign);
		} else if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA160) {
			CryptoPP::RSASS_Hash<CryptoPP::PSS_Hash, CryptoPP::SHA1>::SignerHash signer(
					privateKey);
			signer.SignHash(rng, (const byte*) msg, sign);
		} else
			retVal = false;
	} else
		retVal = false;

	if (retVal) {
		for (UINT i = 0; i < BinDS.KeySize / 16; ++i) {
			byte temp = sign[i];
			sign[i] = sign[BinDS.KeySize / 8 - 1 - i];
			sign[BinDS.KeySize / 8 - 1 - i] = temp;
		}
		CDigS.DigSFromArray((UINT *) sign, CDigS.KeySize() / 32);
	}

	delete RSA_D;	//Private Exponent
	delete RSA_E;	//Public Exponent
	delete RSA_N;	//Public Modulus
	delete sign;

	return retVal;
}

bool CTrustedImageBuilder::verifyMessageUsingCryptoPPRSA(unsigned char * msg,
		UINT_T msgSize, CDigitalSignature &CDigS) {
	UINT_T* RSA_D = NULL;	//Private Exponent
	UINT_T* RSA_E = NULL;	//Public Exponent
	UINT_T* RSA_N = NULL;	//Public Modulus
	UINT_T* sign = NULL;
	UINT_T* signReversed = NULL;
	bool retVal = true;
	int KeySizeInWords = CDigS.DSKey()->KeySize() / 32;
	CTimLib timLib;

	RSA_D = new UINT_T[KeySizeInWords];
	RSA_E = new UINT_T[KeySizeInWords];
	RSA_N = new UINT_T[KeySizeInWords];
	sign = new UINT_T[KeySizeInWords * 4];
	signReversed = new UINT_T[KeySizeInWords * 4];

	std::memset(RSA_D, 0, KeySizeInWords * 4);
	std::memset(RSA_E, 0, KeySizeInWords * 4);
	std::memset(RSA_N, 0, KeySizeInWords * 4);
	std::memset(sign, 0, KeySizeInWords * 4);
	std::memset(signReversed, 0, KeySizeInWords * 4);

	CDigS.DSKey()->PubCompEToArray(RSA_E, KeySizeInWords); //Exponent is one word long
	CDigS.DSKey()->PubCompDToArray(RSA_N, KeySizeInWords); //KeySize is in Bits
	CDigS.DSKey()->PrivateToArray(RSA_D, KeySizeInWords);
	CDigS.DigSToArray(signReversed, (CDigS.KeySize() + 31) / 32);

	CryptoPP::AutoSeededRandomPool rng;
	UINT_T RsaPrivateKeyReversed[MAXRSAKEYSIZEWORDS] = { 0 };
	UINT_T RsaModulusReversed[MAXRSAKEYSIZEWORDS] = { 0 };
	UINT_T RsaPublicKeyReversed[MAXRSAKEYSIZEWORDS] = { 0 };
	PLAT_DS BinDS = CDigS.toPlatDs();
	for (int i = 0; i < KeySizeInWords; ++i) {
		timLib.Endian_Convert(RSA_D[(BinDS.KeySize / 32) - 1 - i], &RsaPrivateKeyReversed[i]);
		timLib.Endian_Convert(RSA_N[(BinDS.KeySize / 32) - 1 - i], &RsaModulusReversed[i]);
		timLib.Endian_Convert(RSA_E[(BinDS.KeySize / 32) - 1 - i], &RsaPublicKeyReversed[i]);
		timLib.Endian_Convert(signReversed[(BinDS.KeySize / 32) - 1 - i], &sign[i]);
	}

	CryptoPP::Integer n((const byte *) RsaModulusReversed,
			(int) (BinDS.KeySize / 8));
	CryptoPP::Integer d((const byte *) RsaPrivateKeyReversed,
			(int) (BinDS.KeySize / 8));
	CryptoPP::Integer e((const byte *) RsaPublicKeyReversed,
			(int) (BinDS.KeySize / 8));

	CryptoPP::RSA::PublicKey publicKey;
	publicKey.Initialize(n, e);
	retVal = publicKey.Validate(rng, 3);

	if (retVal && BinDS.DSAlgorithmID == PKCS1_v1_5_Ippcp) {
		if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA512) {
			CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA512>::Verifier verifier(
					publicKey);
			retVal = verifier.VerifyMessage((const byte*) msg, msgSize,
					(byte *) sign, (BinDS.KeySize / 8));
		} else if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA256) {
			CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA256>::Verifier verifier(
					publicKey);
			retVal = verifier.VerifyMessage((const byte*) msg, msgSize,
					(byte *) sign, (BinDS.KeySize / 8));
		} else if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA160) {
			CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA1>::Verifier verifier(
					publicKey);
			retVal = verifier.VerifyMessage((const byte*) msg, msgSize,
					(byte *) sign, (BinDS.KeySize / 8));
		} else
			retVal = false;

	} else if (retVal && BinDS.DSAlgorithmID == PKCS_v2_2_Ippcp) {
		if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA512) {
			CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA512>::Verifier verifier(
					publicKey);
			retVal = verifier.VerifyMessage((const byte*) msg, msgSize,
					(byte *) sign, (BinDS.KeySize / 8));
		} else if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA256) {
			CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA256>::Verifier verifier(
					publicKey);
			retVal = verifier.VerifyMessage((const byte*) msg, msgSize,
					(byte *) sign, (BinDS.KeySize / 8));
		} else if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA160) {
			CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA1>::Verifier verifier(
					publicKey);
			retVal = verifier.VerifyMessage((const byte*) msg, msgSize,
					(byte *) sign, (BinDS.KeySize / 8));
		} else
			retVal = false;
	} else
		retVal = false;

	delete RSA_D;	//Private Exponent
	delete RSA_E;	//Public Exponent
	delete RSA_N;	//Public Modulus
	delete sign;
	delete signReversed;

	return retVal;
}

bool CTrustedImageBuilder::signMessageUsingCryptoPPECDSA(unsigned char * msg,
		UINT_T msgSize, CDigitalSignature &CDigS) {
	UINT_T* ECDSA_Public = NULL;
	UINT_T* ECDSA_Private = NULL;
	byte sign[4096];
	byte signReversed[4096];
	bool retVal = true;
	int eccp_field_size_bits = CDigS.KeySize();
	int BNsize32 = (eccp_field_size_bits + 31) / 32;
	int siglen;

	ECDSA_Public = new UINT_T[2 * MAXECCKEYSIZEWORDS];
	ECDSA_Private = new UINT_T[2 * MAXECCKEYSIZEWORDS];

	std::memset(ECDSA_Public, 0, 8 * MAXECCKEYSIZEWORDS);
	std::memset(ECDSA_Private, 0, 8 * MAXECCKEYSIZEWORDS);
	std::memset(sign, 0, 4096);
	std::memset(signReversed, 0, 4096);

	CDigS.DSKey()->PubCompDToArray(&ECDSA_Public[0], BNsize32);	//MAXECCKEYSIZEWORDS );
	CDigS.DSKey()->PubCompEToArray(&ECDSA_Public[BNsize32], BNsize32);//MAXECCKEYSIZEWORDS );
	CDigS.DSKey()->PrivateToArray(ECDSA_Private, BNsize32);

	CryptoPP::AutoSeededRandomPool rng;
	PLAT_DS BinDS = CDigS.toPlatDs();
	memset(BinDS.Ecdsa.ECDSADigS_R, 0, sizeof(BinDS.Ecdsa.ECDSADigS_R));
	memset(BinDS.Ecdsa.ECDSADigS_S, 0, sizeof(BinDS.Ecdsa.ECDSADigS_S));

	UINT_T ECDSA_Public_Reversed[2 * MAXECCKEYSIZEWORDS] = { 0 };
	UINT_T ECDSA_Private_Reversed[2 * MAXECCKEYSIZEWORDS] = { 0 };
	for (int i = 0; i < BNsize32; ++i) {
		Endian_Convert (ECDSA_Public[BNsize32 - 1 - i],		&ECDSA_Public_Reversed[i]);
		Endian_Convert (ECDSA_Public[2 * BNsize32 - 1 - i], &ECDSA_Public_Reversed[i + BNsize32]);
		Endian_Convert (ECDSA_Private[BNsize32 - 1 - i], 	&ECDSA_Private_Reversed[i]);
	}

	CryptoPP::Integer x((const byte *) ECDSA_Private_Reversed,
			(int) (BNsize32 * 4));       // private exponent

	if (BinDS.DSAlgorithmID == ENCRYPTALGORITHMID_T::ECDSA_521) {
		if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA512) {
			CryptoPP::ECDSA_Hash<CryptoPP::ECP, CryptoPP::SHA512>::PrivateKey privateKey;
			privateKey.Initialize(CryptoPP::ASN1::secp521r1(), x);
			retVal = privateKey.Validate(rng, 3);
			if (retVal == false)
				return false;

			CryptoPP::ECDSA_Hash<CryptoPP::ECP, CryptoPP::SHA512>::SignerHash signer(
					privateKey);
			siglen = signer.SignHash(rng, (const byte*) msg, sign);
		} else if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA256) {
			CryptoPP::ECDSA_Hash<CryptoPP::ECP, CryptoPP::SHA256>::PrivateKey privateKey;
			privateKey.Initialize(CryptoPP::ASN1::secp521r1(), x);
			retVal = privateKey.Validate(rng, 3);
			if (retVal == false)
				return false;

			CryptoPP::ECDSA_Hash<CryptoPP::ECP, CryptoPP::SHA256>::SignerHash signer(
					privateKey);
			siglen = signer.SignHash(rng, (const byte*) msg, sign);
		} else if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA160) {
			CryptoPP::ECDSA_Hash<CryptoPP::ECP, CryptoPP::SHA1>::PrivateKey privateKey;
			privateKey.Initialize(CryptoPP::ASN1::secp521r1(), x);
			retVal = privateKey.Validate(rng, 3);
			if (retVal == false)
				return false;

			CryptoPP::ECDSA_Hash<CryptoPP::ECP, CryptoPP::SHA1>::SignerHash signer(
					privateKey);
			siglen = signer.SignHash(rng, (const byte*) msg, sign);
		} else
			retVal = false;
	} else if (BinDS.DSAlgorithmID == ENCRYPTALGORITHMID_T::ECDSA_256) {
		if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA512) {
			CryptoPP::ECDSA_Hash<CryptoPP::ECP, CryptoPP::SHA512>::PrivateKey privateKey;
			privateKey.Initialize(CryptoPP::ASN1::secp256r1(), x);
			retVal = privateKey.Validate(rng, 3);
			if (retVal == false)
				return false;

			CryptoPP::ECDSA_Hash<CryptoPP::ECP, CryptoPP::SHA512>::SignerHash signer(
					privateKey);
			siglen = signer.SignHash(rng, (const byte*) msg, sign);
		} else if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA256) {
			CryptoPP::ECDSA_Hash<CryptoPP::ECP, CryptoPP::SHA256>::PrivateKey privateKey;
			privateKey.Initialize(CryptoPP::ASN1::secp256r1(), x);
			retVal = privateKey.Validate(rng, 3);
			if (retVal == false)
				return false;

			CryptoPP::ECDSA_Hash<CryptoPP::ECP, CryptoPP::SHA256>::SignerHash signer(
					privateKey);
			siglen = signer.SignHash(rng, (const byte*) msg, sign);
		} else if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA160) {
			CryptoPP::ECDSA_Hash<CryptoPP::ECP, CryptoPP::SHA1>::PrivateKey privateKey;
			privateKey.Initialize(CryptoPP::ASN1::secp256r1(), x);
			retVal = privateKey.Validate(rng, 3);
			if (retVal == false)
				return false;

			CryptoPP::ECDSA_Hash<CryptoPP::ECP, CryptoPP::SHA1>::SignerHash signer(
					privateKey);
			siglen = signer.SignHash(rng, (const byte*) msg, sign);
		} else
			retVal = false;
	}
	int keySize = (CDigS.KeySize() + 31) / 32;
	if (retVal) {
		for (UINT i = 0; i < siglen / 2; ++i) {
			signReversed[i] = sign[siglen / 2 - 1 - i];
			signReversed[keySize * 4 + i] = sign[siglen - 1 - i];
		}
		CDigS.DigSFromArray((UINT *) signReversed, keySize);
	}

	delete ECDSA_Public;	//Private Exponent
	delete ECDSA_Private;	//Public Exponent

	return retVal;
}

bool CTrustedImageBuilder::verifyMessageUsingCryptoPPECDSA(unsigned char * msg,
		UINT_T msgSize, CDigitalSignature &CDigS) {
	UINT_T* ECDSA_Public = NULL;
	UINT signReversed[1024];
	UINT sign[1024];
	PLAT_DS BinDS = CDigS.toPlatDs();
	bool retVal = true;
	int eccp_field_size_bits = CDigS.KeySize();
	int BNsize32 = (eccp_field_size_bits + 31) / 32;
	CTimLib timLib;

	ECDSA_Public = new UINT_T[2 * MAXECCKEYSIZEWORDS];

	std::memset(ECDSA_Public, 0, 8 * MAXECCKEYSIZEWORDS);
	std::memset(sign, 0, 4096);

	CDigS.DigSToArray(signReversed, (CDigS.KeySize() + 31) / 32);
	CDigS.DSKey()->PubCompDToArray(&ECDSA_Public[0], BNsize32);	//MAXECCKEYSIZEWORDS );
	CDigS.DSKey()->PubCompEToArray(&ECDSA_Public[BNsize32], BNsize32);//MAXECCKEYSIZEWORDS );

	CryptoPP::AutoSeededRandomPool rng;
	UINT_T ECDSA_Public_Reversed[2 * MAXECCKEYSIZEWORDS] = { 0 };
	UINT_T ECDSA_Private_Reversed[2 * MAXECCKEYSIZEWORDS] = { 0 };
	for (int i = 0; i < BNsize32; ++i) {
		timLib.Endian_Convert (ECDSA_Public[BNsize32 - 1 - i],		&ECDSA_Public_Reversed[i]);
		timLib.Endian_Convert (ECDSA_Public[2 * BNsize32 - 1 - i], &ECDSA_Public_Reversed[i + BNsize32]);
		timLib.Endian_Convert (signReversed[BNsize32 - 1 - i],		&sign[i]);
		timLib.Endian_Convert (signReversed[2 * BNsize32 - 1 - i], &sign[i + BNsize32]);
	}

	CryptoPP::Integer px((const byte *) ECDSA_Public_Reversed, (int) (BNsize32 * 4));
	CryptoPP::Integer py((const byte *) &ECDSA_Public_Reversed[BNsize32], (int) (BNsize32 * 4));
	CryptoPP::ECP::Point q;

	q.identity = false;
	q.x = px;
	q.y = py;

	int keyLengthInBytes = (CDigS.KeySize() + 7) / 8;
	byte CPPSign[1024];
	byte *sigBytes = (byte *) sign;
	std::memset(CPPSign, 0, 1024);
	for (int i = 0; i < keyLengthInBytes; ++i) {
		CPPSign[keyLengthInBytes - i - 1] = sigBytes[BNsize32 * 4 - i - 1];
		CPPSign[2 * keyLengthInBytes - i - 1] = sigBytes[BNsize32 * 8 - i - 1];
	}

	if (BinDS.DSAlgorithmID == ENCRYPTALGORITHMID_T::ECDSA_521) {
		if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA512) {
			CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA512>::PublicKey publicKey;
			publicKey.Initialize(CryptoPP::ASN1::secp521r1(), q);
			retVal = publicKey.Validate(rng, 3);
			if (retVal == false)
				return retVal;
			CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA512>::Verifier verifier(
					publicKey);
			retVal = verifier.VerifyMessage((const byte*) msg, msgSize,
					(const byte*) CPPSign, 2 * keyLengthInBytes);
		} else if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA256) {
			CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey publicKey;
			publicKey.Initialize(CryptoPP::ASN1::secp521r1(), q);
			retVal = publicKey.Validate(rng, 3);
			if (retVal == false)
				return retVal;
			CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::Verifier verifier(
					publicKey);
			retVal = verifier.VerifyMessage((const byte*) msg, msgSize,
					(const byte*) CPPSign, 2 * keyLengthInBytes);
		} else if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA160) {
			CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA1>::PublicKey publicKey;
			publicKey.Initialize(CryptoPP::ASN1::secp521r1(), q);
			retVal = publicKey.Validate(rng, 3);
			if (retVal == false)
				return retVal;
			CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA1>::Verifier verifier(
					publicKey);
			retVal = verifier.VerifyMessage((const byte*) msg, msgSize,
					(const byte*) CPPSign, 2 * keyLengthInBytes);
		} else
			retVal = false;
	} else if (BinDS.DSAlgorithmID == ENCRYPTALGORITHMID_T::ECDSA_256) {
		if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA512) {
			CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA512>::PublicKey publicKey;
			publicKey.Initialize(CryptoPP::ASN1::secp256r1(), q);
			retVal = publicKey.Validate(rng, 3);
			if (retVal == false)
				return retVal;
			CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA512>::Verifier verifier(
					publicKey);
			retVal = verifier.VerifyMessage((const byte*) msg, msgSize,
					(const byte*) CPPSign, 2 * keyLengthInBytes);
		} else if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA256) {
			CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey publicKey;
			publicKey.Initialize(CryptoPP::ASN1::secp256r1(), q);
			retVal = publicKey.Validate(rng, 3);
			if (retVal == false)
				return retVal;
			CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::Verifier verifier(
					publicKey);
			retVal = verifier.VerifyMessage((const byte*) msg, msgSize,
					(const byte*) CPPSign, 2 * keyLengthInBytes);
		} else if (BinDS.HashAlgorithmID == HASHALGORITHMID_T::SHA160) {
			CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA1>::PublicKey publicKey;
			publicKey.Initialize(CryptoPP::ASN1::secp256r1(), q);
			retVal = publicKey.Validate(rng, 3);
			if (retVal == false)
				return retVal;
			CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA1>::Verifier verifier(
					publicKey);
			retVal = verifier.VerifyMessage((const byte*) msg, msgSize,
					(const byte*) CPPSign, 2 * keyLengthInBytes);
		} else
			retVal = false;
	}
	if (retVal)
		cout << "Verified signature on message" << endl;
	else
		cerr << "Failed to verify signature on message" << endl;

	delete ECDSA_Public;
	return retVal;

}

