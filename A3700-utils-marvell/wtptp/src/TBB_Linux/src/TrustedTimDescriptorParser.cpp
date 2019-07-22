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
#include "TrustedTimDescriptorParser.h"
#include "KeyGenSpec.h"

#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <libgen.h>

CTrustedTimDescriptorParser::CTrustedTimDescriptorParser(
		CCommandLineParser& CommandLineParser) :
		CTimDescriptorParser(CommandLineParser) {
}

CTrustedTimDescriptorParser::~CTrustedTimDescriptorParser(void) {
	Reset();
}

void CTrustedTimDescriptorParser::Reset() {
	CTimDescriptorParser::Reset();
}

bool CTrustedTimDescriptorParser::ParseTrustedDescriptor(
		CCommandLineParser& CommandLineParser, CTimDescriptorLine*& pLine) {
	UINT_T IntParam = 0;
	unsigned int HashSize = 0;
	string tbuf;
	bool bRet = true;

	string lcTimTxtFilePath = CTimLib::ToLower(
			m_TimDescriptor.TimDescriptorFilePath());
	bool bIsBlf = lcTimTxtFilePath.find(".blf")
			== lcTimTxtFilePath.length() - 4;

	if (CommandLineParser.iOption == 1) {
		if (bIsBlf) {
			printf("\nParsing trusted TIM in blf file...\n\n");
			m_TimDescriptor.BlfHeader().ParseBlfHeader(pLine);
		} else {
			printf("\nParsing trusted TIM...\n\n");
		}
	} else {
		if (bIsBlf) {
			printf(
					"\nParsing trusted TIM in blf file without digital signature.\n");
			m_TimDescriptor.BlfHeader().ParseBlfHeader(pLine);
		} else {
			printf("\nParsing trusted TIM without digital signature.\n");
		}
	}

	CCTIM& TimHeader = m_TimDescriptor.getTimHeader();

	if (bIsBlf) {
		pLine = m_TimDescriptor.GetLineField("[TIM_Configuration]", true);
		if (pLine != 0) {
			pPrevLine = pLine;
		}
	}

	pLine = bIsBlf ?
			m_TimDescriptor.GetLineField("Issue_Date", true) :
			m_TimDescriptor.GetNextLineField("Issue Date", pLine, true, 1);
	if (pLine != 0) {
		pPrevLine = pLine;
		TimHeader.setIssueDate(Translate(pLine->m_FieldValue));
	} else {
		pLine = pPrevLine;
		bRet = false;
	}

	pLine = bIsBlf ?
			m_TimDescriptor.GetLineField("OEM_UniqueID", true) :
			m_TimDescriptor.GetNextLineField("OEM UniqueID", pLine, true, 1);
	if (pLine != 0) {
		pPrevLine = pLine;
		TimHeader.setOEMUniqueId(Translate(pLine->m_FieldValue));
	} else {
		pLine = pPrevLine;
		bRet = false;
	}
	if (m_TimDescriptor.getTimHeader().getVersion() >= TIM_3_6_00) {
		pLine = bIsBlf ?
				m_TimDescriptor.GetLineField("Stepping", true) :
				m_TimDescriptor.GetNextLineField("Stepping", pLine, true, 1);
		if (pLine != 0) {
			pPrevLine = pLine;
			TimHeader.setStepping(Translate(pLine->m_FieldValue));
		} else {
			pLine = pPrevLine;
			bRet = false;
		}
	}

	pLine = bIsBlf ?
			m_TimDescriptor.GetLineField("Processor_Type", true) :
			m_TimDescriptor.GetNextLineField("Processor Type", pLine, false, 1);
	if (pLine != 0) {
		// get Processor Type, but if not found reposition to get WTM Save State Flash Signature
		pPrevLine = pLine;
		if (ToUpper(pLine->m_FieldValue) == "UNDEFINED")
			pLine->m_FieldValue = "<undefined>";

		if (!m_TimDescriptor.ProcessorTypeStr(pLine->m_FieldValue)) {
			printf("\nRecognized processor types:\n");
			printf(
					"   PXA168, PXA30x, PXA31x, PXA32x, ARMADA610, PXA91x, PXA92x, PXA93x, PXA94x, PXA95x,\n");
			printf(
					"   ARMADA16x, PXA955, PXA968, PXA1701, PXA978, PXA2128, ARMADA622, PXA1202, PXA1801,\n");
			printf(
					"   88PA62_70, PXA988, PXA1920, PXA2101, PXA192, PXA1928, PXA1986, PXA1802, PXA986,\n");
			printf(
					"    PXA1206, PXA888, PXA1088, PXA1812, PXA1822, PXA1U88, PXA1936, PXA1908, PXA1826, 88PA62_20,\n");
			printf("    88PA62_10, PXA1956, VEGA, BOWIE, ULC2 <undefined>\n");
		}

	} else {
		pLine = pPrevLine;
		//        bRet = false;
	}

	// since processor type may be in two places we need to find "Size.." starting at the beginning again
	if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_6_00) {
		pLine = bIsBlf ?
				m_TimDescriptor.GetLineField("WTM_Save_State_Flash_Signature",
						true) :
				m_TimDescriptor.GetLineField("WTM Save State Flash Signature",
						true);
		if (pLine != 0) {
			pPrevLine = pLine;
			TimHeader.setWtmSaveStateFlashSignature(
					Translate(pLine->m_FieldValue));
		} else
			bRet = false;

		pLine = bIsBlf ?
				m_TimDescriptor.GetLineField(
						"WTM_Save_State_Flash_Entry_Address", true) :
				m_TimDescriptor.GetNextLineField("WTM Save State Entry", pLine,
						true, 1);
		if (pLine != 0) {
			pPrevLine = pLine;
			TimHeader.setWtmSaveStateFlashEntryAddress(
					Translate(pLine->m_FieldValue));
		} else {
			pLine = pPrevLine;
			bRet = false;
		}

		pLine = bIsBlf ?
				m_TimDescriptor.GetLineField(
						"WTM_Save_State_BackUp_Entry_Address", true) :
				m_TimDescriptor.GetNextLineField("WTM Save State Backup Entry",
						pLine, true, 1);
		if (pLine != 0) {
			pPrevLine = pLine;
			TimHeader.setWtmSaveStateBackupEntryAddress(
					Translate(pLine->m_FieldValue));
		} else {
			pLine = pPrevLine;
			bRet = false;
		}

		TimHeader.setWtmSaveStatePatchSignature(0xFFFFFFFF);
		TimHeader.setWtmSaveStatePatchAddress(0xFFFFFFFF);
	}

	pLine = bIsBlf ?
			m_TimDescriptor.GetLineField("Boot_Flash_Signature", true) :
			m_TimDescriptor.GetLineField("Boot Flash Signature", true);
	if (pLine != 0) {
		pPrevLine = pLine;
		TimHeader.setBootRomFlashSignature(Translate(pLine->m_FieldValue));
	} else {
		pLine = pPrevLine;
		bRet = false;
	}

	pLine = bIsBlf ?
			m_TimDescriptor.GetLineField("Number_of_Images", true) :
			m_TimDescriptor.GetNextLineField("Number of Images", pLine, true,
					1);
	if (pLine != 0) {
		pPrevLine = pLine;
		TimHeader.setNumImages(Translate(pLine->m_FieldValue));
	} else {
		pLine = pPrevLine;
		bRet = false;
	}

	//return error if number of images > 20
	if ((int) TimHeader.getNumImages() > MAX_IMAGES) {
		m_TimDescriptor.ParsingError(
				"\n  Error: Maximum allowable number of images in the descriptor file is 10!\n",
				true, pLine);
		bRet = false;
	}

	pLine = bIsBlf ?
			m_TimDescriptor.GetLineField("Number_of_Keys", true) :
			m_TimDescriptor.GetNextLineField("Number of Keys", pLine, true, 1);
	if (pLine != 0) {
		pPrevLine = pLine;
		TimHeader.setNumKeys(Translate(pLine->m_FieldValue));
	} else {
		pLine = pPrevLine;
		bRet = false;
	}

	pLine = bIsBlf ?
			m_TimDescriptor.GetLineField("Size_of_Reserved_in_bytes", false) :
			m_TimDescriptor.GetNextLineField("Size of Reserved in bytes", pLine,
					true, 15);
	if (pLine != 0) {
		TimHeader.setSizeOfReserved(Translate(pLine->m_FieldValue));
	} else {
		pLine = pPrevLine;
		if (!bIsBlf) // ignore in blf because not in original blf syntax
			bRet = false;
	}

	if (CommandLineParser.bTimTxtRelative) {
		if (CommandLineParser.TimTxtFilePath.length() > 0) {
			SetPathAsCWD(CommandLineParser.TimTxtFilePath);
		}
	}

	string NextImageId("");
	stringstream ssImageNum;
	int nImage = 0;

	for (int i = 0; i < (int) TimHeader.getNumImages(); i++) {
		nImage++;
		ssImageNum.str("");
		ssImageNum << nImage << "_";

		CImageDescription* pImageDesc = new CImageDescription(bIsBlf,
				m_TimDescriptor);

		if (ParseImageInfo(*pImageDesc, pLine, ssImageNum.str()) == false) {
			bRet = false;
			delete pImageDesc;
			return false;
		}

		if (i == 0) {
			if ((Translate(pImageDesc->ImageIdTag()) & TYPEMASK)
					!= (TIMIDENTIFIER & TYPEMASK)) {
				m_TimDescriptor.ParsingError(
						"\n  Error: The ImageID value is incorrect for the TIM image!\n",
						true, pLine);
				bRet = false;
			}

			m_sTimBinFilename = pImageDesc->ImageFilePath();
			// added 8/11/2014
			m_sTimBinFilename = CTimLib::MakePathAbsoluteOrRelativeToTIMTxtPath(
					m_sTimBinFilename, CommandLineParser.TimTxtFilePath,
					CommandLineParser.bTimTxtRelative);
			pImageDesc->ImageSize(
					m_TimDescriptor.GetTimImageSize(
							CommandLineParser.bOneNANDPadding,
							CommandLineParser.uiPaddedSize));

			// if ImageSizeToHash is all 1's or larger than image, set to image size
			if ((pImageDesc->ImageSizeToHash() > pImageDesc->ImageSize())
					|| (pImageDesc->ImageSizeToHash() == 0xFFFFFFFF)) {
				pImageDesc->ImageSizeToHash(pImageDesc->ImageSize());
			}

			// if ImageSizeToHash is all 1's or larger than image, set to image size
			if ((pImageDesc->ImageSizeToHash() > pImageDesc->ImageSize())
					|| (pImageDesc->ImageSizeToHash() == 0xFFFFFFFF)) {
				pImageDesc->ImageSizeToHash(pImageDesc->ImageSize());
			}
			if (!ParseImageEncryptSize(pImageDesc, pLine)) {
				bRet = false;
				delete pImageDesc;
				return false;
			}
		} else {
			if (pImageDesc->ImageId() != NextImageId) {
				m_TimDescriptor.ParsingError(
						"\n  Error: The ImageID doesn't match NextImageID from the previous image!\n",
						true, pLine);
				bRet = false;
			}

			if ((pImageDesc->ImageSizeToHash() > pImageDesc->ImageSize())
					|| (pImageDesc->ImageSizeToHash() == 0xFFFFFFFF)) {
				pImageDesc->ImageSizeToHash(pImageDesc->ImageSize());
			}

			// if ImageSizeToHash is all 1's or larger than image, set to image size
			if ((pImageDesc->ImageSizeToHash() > pImageDesc->ImageSize())
					|| (pImageDesc->ImageSizeToHash() == 0xFFFFFFFF)) {
				pImageDesc->ImageSizeToHash(pImageDesc->ImageSize());
			}

			if (!ParseImageEncryptSize(pImageDesc, pLine)) {
				bRet = false;
				delete pImageDesc;
				return false;
			}
		}

		m_TimDescriptor.ImagesList().push_back(pImageDesc);
		NextImageId = pImageDesc->NextImageId();
	}

	if (m_TimDescriptor.ImagesList().size() != (int) TimHeader.getNumImages()) {
		m_TimDescriptor.ParsingError(
				"Number of Images field is not consistent with image structs in TIM text file.\n",
				true);
		bRet = false;
	}

	stringstream ssKeyNum;
	unsigned int nKeyNum = 0;

	for (int i = 0; i < (int) TimHeader.getNumKeys(); i++) {
		if (!bIsBlf) {
			if (!ParseKeyInfoTIM(pLine))
				return false;
		} else {
			nKeyNum++;
			ssKeyNum.str("");
			ssKeyNum << nKeyNum << "_";
			string sKeyNum(ssKeyNum.str());
			if (!ParseKeyInfoBlf(pLine, sKeyNum))
				return false;
		}
	}

	if (!ParseReservedData())
		return false;

	// if reserved data auto migrated then make sure to reset SizeOfReserved
	if (m_TimDescriptor.ReservedDataList().size() == 0)
		TimHeader.setSizeOfReserved(0);

	if (ParseExtendedReservedData(CommandLineParser) == false)
		return false;

	//check to make sure the images do not overlap each other
	if (CheckImageOverlap(m_TimDescriptor.ImagesList()) == false)
		bRet = false;

	if (!CheckPINCount())
		return false;

	if (!CommandLineParser.bUseKeyFile) {
		if (!bIsBlf) {
			if (ParseDsaInfoTIM(CommandLineParser) == false)
				return false;
		} else {
			if (ParseDsaInfoBlf(CommandLineParser) == false)
				return false;

			if (ParseDTIMKeysDataBlf(CommandLineParser) == false)
				return false;
		}
	}

	if (VerifySizeOfTim(CommandLineParser) == false)
		return false;

	if (CommandLineParser.bConcatenate) {
		ifstream ifsImage;
		t_ImagesIter iter = m_TimDescriptor.ImagesList().begin();
		while (iter != m_TimDescriptor.ImagesList().end()) {
			(*iter)->ImageFilePath() =
					CTimLib::MakePathAbsoluteOrRelativeToTIMTxtPath(
							(*iter)->ImageFilePath(),
							CommandLineParser.TimTxtFilePath,
							CommandLineParser.bTimTxtRelative);

			ifsImage.open((*iter)->ImageFilePath().c_str(),
					ios_base::in | ios_base::binary);
			if (ifsImage.bad() || ifsImage.fail()) {
				stringstream ss;
				ss << endl << "  Error: Cannot open image file name <"
						<< (*iter)->ImageFilePath().c_str() << ">" << endl;
				m_TimDescriptor.ParsingError(ss.str(), true);
				bRet = false;
			}
			ifsImage.close();
			iter++;
		}
	}

	m_TimDescriptor.Changed(false);

	if (m_TimDescriptor.ParseErrors() == 0) {
		if (bIsBlf) {
			printf(
					"\n  Success: Trusted Tim Descriptor in blf file parsing has completed successfully!\n");
		} else {
			printf(
					"\n  Success: Trusted Tim Descriptor file parsing has completed successfully!\n");
		}
	} else {
		if (bIsBlf) {
			printf(
					"\n  Failed: Trusted Tim Descriptor in blf file parsing has completed with parsing errors!\n");
		} else {
			printf(
					"\n  Failed: Trusted Tim Descriptor file parsing has completed with parsing errors!\n");
		}
	}

	return bRet;
}

bool CTrustedTimDescriptorParser::CheckPINCount() {
	int PinCount = 0;
	// count any PINs in Reserved Data so we can use the one from PIN.txt
	t_ReservedDataListIter Iter = m_TimDescriptor.ReservedDataList().begin();
	while (Iter != m_TimDescriptor.ReservedDataList().end()) {
		if ((*Iter)->PackageId() == "PINP")
			PinCount++;

		Iter++;
	}

	// count any PINs in Extended Reserved Data so we can use the one from PIN.txt
	t_ErdBaseVectorIter ERDIter =
			m_TimDescriptor.ExtendedReservedData().ErdVec.begin();
	while (ERDIter != m_TimDescriptor.ExtendedReservedData().ErdVec.end()) {
		if ((*ERDIter)->ErdPkgType() == CErdBase::PIN_ERD)
			PinCount++;

		ERDIter++;
	}

	// no more that 1 PIN is allowed in TIM.txt
	if (PinCount > 1) {
		printf(
				"\nError: %d PINs found. Only 1 PIN is allowed.  Check reserved data and Extended Reserved Data areas of <%s> and remove extra PIN(s).\n",
				PinCount, m_sTimFilePath.c_str());

		return false;
	}

	return true;
}

bool CTrustedTimDescriptorParser::ParseKeyInfoTIM(CTimDescriptorLine*& pLine) {
	UINT_T IntParam = 0;
	bool bRet = true;
	string sValue;
	unsigned int i = 0;
	bool IsOk = true;

	string sKeyTag;
	HASHALGORITHMID_T HashAlgoId;
	unsigned int uiKeySize = 0;
	unsigned int uiPublicKeySize = 0;
	CKey* pKey = 0;
	unsigned int EncryptAlgoId = 0;

	// This is a helper function to parse key info data from the descriptor
	// text file supplied by the user.
	t_KeyList& KeyList = m_TimDescriptor.KeyList();

	if ((pLine = m_TimDescriptor.GetNextLineField("Key ID", pLine, true, 1))
			== 0) {
		pLine = pPrevLine;
		bRet = false;
		goto Exit;
	}

	pPrevLine = pLine;
	sKeyTag = HexFormattedAscii(Translate(pLine->m_FieldValue));

	if ((pLine = m_TimDescriptor.GetNextLineField("Hash Algorithm ID", pLine,
			true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
		goto Exit;
	}

	pPrevLine = pLine;
	HashAlgoId = (HASHALGORITHMID_T) Translate(pLine->m_FieldValue);

	if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
		if ((pLine = m_TimDescriptor.GetNextLineField("Modulus Size in bytes",
				pLine, true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
			goto Exit;
		}
		pPrevLine = pLine;
		uiKeySize = Translate(pLine->m_FieldValue) * 8;
	} else {
		if ((pLine = m_TimDescriptor.GetNextLineField("Key Size in bits", pLine,
				true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
			goto Exit;
		}
		pPrevLine = pLine;
		uiKeySize = Translate(pLine->m_FieldValue);
	}

	if (uiKeySize != 1024 && uiKeySize != 256 && uiKeySize != 512
			&& uiKeySize != 2048 && uiKeySize != 521) {
		stringstream ss;
		ss << "  Error: The Key Size is incorrect for Key ID 0x" << hex
				<< Translate(sKeyTag) << endl;
		m_TimDescriptor.ParsingError(ss.str(), true, pLine);
		return false;
	}

	if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
		if ((pLine = m_TimDescriptor.GetNextLineField(
				"Public Key Size in bytes", pLine, true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
			goto Exit;
		}
		pPrevLine = pLine;
		uiPublicKeySize = Translate(pLine->m_FieldValue) * 8;
	} else {
		if ((pLine = m_TimDescriptor.GetNextLineField("Public Key Size in bits",
				pLine, true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
			goto Exit;
		}
		pPrevLine = pLine;
		uiPublicKeySize = Translate(pLine->m_FieldValue);
	}

	if (m_TimDescriptor.getTimHeader().getVersion() >= TIM_3_3_00) {
		if ((pLine = m_TimDescriptor.GetNextLineField("Encrypt Algorithm ID",
				pLine, true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
			goto Exit;
		} else {
			pPrevLine = pLine;
			EncryptAlgoId = Translate(pLine->m_FieldValue);
		}

		if (!pKey) {
			if (EncryptAlgoId == ECDSA_256 || EncryptAlgoId == ECDSA_521)
				pKey = new CECCKey;
			else
				pKey = new CRSAKey;
		}
	} else {
		pKey = new CRSAKey;
	}

	if (!pKey)
		return false;

	pKey->KeyTag(sKeyTag);
	pKey->HashAlgorithmId(HashAlgoId);
	pKey->KeySize(uiKeySize);
	pKey->PublicKeySize(uiPublicKeySize);

	if (m_TimDescriptor.getTimHeader().getVersion() >= TIM_3_3_00) {

		pKey->EncryptAlgorithmId((ENCRYPTALGORITHMID_T) EncryptAlgoId);
		if (pKey->EncryptAlgorithmIdStr() == "") {
			cout << endl << "Error: Unrecognized Encryption Algorithm ID"
					<< endl;
			bRet = false;
			goto Exit;
		}
	}

	if (m_TimDescriptor.getTimHeader().getVersion() >= TIM_3_3_00) {
		if ((pKey->EncryptAlgorithmId() != ECDSA_256)
				&& (pKey->EncryptAlgorithmId() != ECDSA_521)) {
			CRSAKey* pRSAKey = dynamic_cast<CRSAKey*>(pKey);
			bRet = ParseRSAKeyTIM(pRSAKey, pLine);
		} else {
			CECCKey* pECCKey = dynamic_cast<CECCKey*>(pKey);
			bRet = ParseECCKeyTIM(pECCKey, pLine);
		}
	} else {
		if (pKey->EncryptAlgorithmId() == ECDSA_256
				|| pKey->EncryptAlgorithmId() == ECDSA_521) {
			stringstream ss;
			ss << "TIM version "
					<< HexFormattedAscii(
							m_TimDescriptor.getTimHeader().getVersion())
					<< " does not support ECDSA algorithms for Key." << endl;
			m_TimDescriptor.ParsingError(ss.str(), true, pLine);
			bRet = false;
		}

		CRSAKey* pRSAKey = dynamic_cast<CRSAKey*>(pKey);
		bRet = ParseRSAKeyTIM(pRSAKey, pLine);
	}

	Exit: if (bRet == false)
		delete pKey;
	else
		KeyList.push_back(pKey);

	return bRet;
}

bool CTrustedTimDescriptorParser::ParseKeyInfoBlf(CTimDescriptorLine*& pLine,
		string& sNum) {
	UINT_T IntParam = 0;
	bool bRet = true;
	string sValue;
	unsigned int i = 0;
	bool IsOk = true;

	string sKeyId;
	string sHashAlgoId;
	unsigned int uiKeySize = 0;
	unsigned int uiPublicKeySize = 0;
	CKey* pKey = 0;
	unsigned int EncryptAlgoId = 0;

	// This is a helper function to parse key info data from the descriptor
	// text file supplied by the user.

	t_KeyList& KeyList = m_TimDescriptor.KeyList();

	if ((pLine = m_TimDescriptor.GetLineField(sNum + "KEY_Key_ID", true))
			== 0) {
		pLine = pPrevLine;
		bRet = false;
		goto Exit;
	}

	pPrevLine = pLine;
	sKeyId = pLine->m_FieldValue;

	if ((pLine = m_TimDescriptor.GetNextLineField(
			sNum + "KEY_Hash_Algorithm_ID", pLine, true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
		goto Exit;
	}

	pPrevLine = pLine;
	sHashAlgoId = pLine->m_FieldValue;

	if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
		if ((pLine = m_TimDescriptor.GetNextLineField(
				sNum + "KEY_Modulus_Size_in_bytes", pLine, true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
			goto Exit;
		}
		pPrevLine = pLine;
		uiKeySize = Translate(pLine->m_FieldValue) * 8;
	} else {
		if ((pLine = m_TimDescriptor.GetNextLineField(
				sNum + "KEY_Key_Size_in_bits", pLine, true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
			goto Exit;
		}
		pPrevLine = pLine;
		uiKeySize = Translate(pLine->m_FieldValue);
	}

	if (uiKeySize != 1024 && uiKeySize != 256 && uiKeySize != 512
			&& uiKeySize != 2048 && uiKeySize != 521) {
		stringstream ss;
		ss << "  Error: The Key Size is incorrect for Key ID = " << sKeyId
				<< endl;
		m_TimDescriptor.ParsingError(ss.str(), true, pLine);
		return false;
	}

	if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
		if ((pLine = m_TimDescriptor.GetNextLineField(
				sNum + "KEY_Public_Key_Size_in_bytes", pLine, true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
			goto Exit;
		}
		pPrevLine = pLine;
		uiPublicKeySize = Translate(pLine->m_FieldValue) * 8;
	} else {
		//        if ( (pLine = m_TimDescriptor.GetNextLineField( sNum+"KEY_Public_Key_Size_in_bits", pLine, true, 1 )) == 0 )
		// blf actually puts a bits value in a field that indicates bytes
		if ((pLine = m_TimDescriptor.GetNextLineField(
				sNum + "KEY_Public_Key_Size_in_bytes", pLine, true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
			goto Exit;
		}
		pPrevLine = pLine;
		uiPublicKeySize = Translate(pLine->m_FieldValue);
	}

	if (m_TimDescriptor.getTimHeader().getVersion() >= TIM_3_3_00) {
		if ((pLine = m_TimDescriptor.GetNextLineField(
				sNum + "KEY_Encrypt_Algorithm_ID", pLine, true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
			goto Exit;
		} else {
			pPrevLine = pLine;
			// for blf, create a temp key to convert the EncryptAlgorithmId from a string to a enum
			pKey = new CRSAKey;
			pKey->EncryptAlgorithmId(pLine->m_FieldValue);
			EncryptAlgoId = pKey->EncryptAlgorithmId();
			delete pKey;
			pKey = 0;
		}

	}

	if (!pKey) {
		if (EncryptAlgoId == ECDSA_256 || EncryptAlgoId == ECDSA_521)
			pKey = new CECCKey;
		else
			pKey = new CRSAKey;
	} else {
		if (pKey)
			delete pKey;

		pKey = new CRSAKey;
	}

	if (!pKey)
		return false;

	pKey->KeyId(sKeyId);
	pKey->HashAlgorithmId(sHashAlgoId);
	pKey->KeySize(uiKeySize);
	pKey->PublicKeySize(uiPublicKeySize);
	if (m_TimDescriptor.getTimHeader().getVersion() >= TIM_3_3_00) {
		pKey->EncryptAlgorithmId((ENCRYPTALGORITHMID_T) EncryptAlgoId);
		if (pKey->EncryptAlgorithmIdStr() == "") {
			bRet = false;
			goto Exit;
		}
	}

	if (m_TimDescriptor.getTimHeader().getVersion() >= TIM_3_3_00) {
		if ((pKey->EncryptAlgorithmId() != ECDSA_256)
				&& (pKey->EncryptAlgorithmId() != ECDSA_521)) {
			CRSAKey* pRSAKey = dynamic_cast<CRSAKey*>(pKey);
			bRet = ParseRSAKeyBlf(pRSAKey, pLine);
		} else {
			CECCKey* pECCKey = dynamic_cast<CECCKey*>(pKey);
			bRet = ParseECCKeyBlf(pECCKey, pLine);
		}
	} else {
		if (pKey->EncryptAlgorithmId() == ECDSA_256
				|| pKey->EncryptAlgorithmId() == ECDSA_521) {
			stringstream ss;
			ss << "TIM version "
					<< HexFormattedAscii(
							m_TimDescriptor.getTimHeader().getVersion())
					<< " does not support ECDSA algorithms for Key." << endl;
			m_TimDescriptor.ParsingError(ss.str(), true, pLine);
			bRet = false;
		}

		CRSAKey* pRSAKey = dynamic_cast<CRSAKey*>(pKey);
		bRet = ParseRSAKeyBlf(pRSAKey, pLine);
	}

	Exit: if (bRet == false)
		delete pKey;
	else
		KeyList.push_back(pKey);

	return bRet;
}

bool CTrustedTimDescriptorParser::ParseRSAKeyTIM(CRSAKey * pKey,
		CTimDescriptorLine*& pLine) {
	bool IsOk = false;
	bool bRet = true;

	if ((pLine = m_TimDescriptor.GetNextLineField("RSA Public Key Exponent",
			pLine, true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else
		pPrevLine = pLine;

	if (pKey->CKey::KeySize() > 0) {
		IsOk = FillList(pLine, pKey->PublicKeyExponentList(),
				pKey->CKey::PublicKeySize() / 8);
		if (IsOk == false) {
			stringstream ss;
			if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
				ss << "Public Key Size in bytes = "
						<< pKey->CKey::PublicKeySize() / 8 << endl;
			} else {
				ss << "Public Key Size in bits = "
						<< pKey->CKey::PublicKeySize() << endl;
			}
			ss
					<< "  Error: The amount of data read from key file doesn't match what is"
					<< endl;

			if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
				ss << "indicated by 'Public Key Size in bytes' directive!"
						<< endl;
			} else {
				ss << "indicated by 'Public Key Size in bits' directive!"
						<< endl;
			}
			ss << "Error: Public Exponent for Key" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true, pLine);
			bRet = false;
		}
		pPrevLine = pLine;
	}

	if ((pLine = m_TimDescriptor.GetNextLineField("RSA System Modulus", pLine,
			true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else
		pPrevLine = pLine;

	if (pKey->CKey::KeySize() > 0) {
		IsOk = FillList(pLine, pKey->RsaSystemModulusList(),
				pKey->CKey::KeySize() / 8);
		if (IsOk == false) {
			stringstream ss;
			if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
				ss << "Modulus Size in bytes = " << pKey->CKey::KeySize() / 8
						<< endl;
			} else {
				ss << "Key Size in bits = " << pKey->CKey::KeySize() << endl;
			}
			ss
					<< "  Error: The amount of data read from key file doesn't match what is"
					<< endl;

			if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
				ss << "indicated by 'Modulus Size in bytes' directive!" << endl;
			} else {
				ss << "indicated by 'Key Size in bits' directive!" << endl;
			}
			ss << "Error: RsaModulus for Key" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true, pLine);
			bRet = false;
		}
		pPrevLine = pLine;
	}

	return bRet;
}

bool CTrustedTimDescriptorParser::ParseRSAKeyBlf(CRSAKey * pKey,
		CTimDescriptorLine*& pLine) {
	bool IsOk = false;
	bool bRet = true;

	if ((pLine = m_TimDescriptor.GetNextLineField("RSA_Public_Exponent", pLine,
			true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else
		pPrevLine = pLine;

	if (pKey->CKey::KeySize() > 0) {
		IsOk = FillListBlf(pLine, pKey->PublicKeyExponentList(),
				pKey->CKey::PublicKeySize() / 8);
		if (IsOk == false) {
			stringstream ss;
			if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
				ss << "Public Key Size in bytes = "
						<< pKey->CKey::PublicKeySize() / 8 << endl;
			} else {
				ss << "Public Key Size in bits = "
						<< pKey->CKey::PublicKeySize() << endl;
			}
			ss
					<< "  Error: The amount of data read from key file doesn't match what is"
					<< endl;

			if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
				ss << "indicated by 'Public Key Size in bytes' directive!"
						<< endl;
			} else {
				ss << "indicated by 'Public Key Size in bits' directive!"
						<< endl;
			}
			ss << "Error: Public Exponent for Key" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true, pLine);
			bRet = false;
		}
		pPrevLine = pLine;
	}

	if ((pLine = m_TimDescriptor.GetNextLineField("End_RSA_Public_Exponent",
			pLine, true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else
		pPrevLine = pLine;

	if ((pLine = m_TimDescriptor.GetNextLineField("RSA_System_Modulus", pLine,
			true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else
		pPrevLine = pLine;

	if (pKey->CKey::KeySize() > 0) {
		IsOk = FillListBlf(pLine, pKey->RsaSystemModulusList(),
				pKey->CKey::KeySize() / 8);
		if (IsOk == false) {
			stringstream ss;
			if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
				ss << "Modulus Size in bytes = " << pKey->CKey::KeySize() / 8
						<< endl;
			} else {
				ss << "Key Size in bits = " << pKey->CKey::KeySize() << endl;
			}
			ss
					<< "  Error: The amount of data read from key file doesn't match what is"
					<< endl;

			if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
				ss << "indicated by 'Modulus Size in bytes' directive!" << endl;
			} else {
				ss << "indicated by 'Key Size in bits' directive!" << endl;
			}
			ss << "Error: RsaModulus for Key" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true, pLine);
			bRet = false;
		}
		pPrevLine = pLine;
	}

	return bRet;
}

bool CTrustedTimDescriptorParser::ParseECCKeyTIM(CECCKey * pKey,
		CTimDescriptorLine*& pLine) {
	bool bRet = true;
	bool IsOk = false;

	if ((pLine = m_TimDescriptor.GetNextLineField("ECC Public Key Comp X",
			pLine, true, 1)) == 0)
		bRet = false;

	if (pLine) {
		pPrevLine = pLine;
	} else
		pLine = pPrevLine;

	if (pKey->CKey::KeySize() > 0) {
		IsOk = FillList(pLine, pKey->ECDSAPublicKeyCompXList(),
				(((pKey->CKey::KeySize() + 31) / 32) * 4));
		if (IsOk == false) {
			stringstream ss;
			ss << "Key Size in bits = " << pKey->CKey::KeySize() << endl;
			;
			ss
					<< "  Error: The amount of data read from key file doesn't match what is"
					<< endl;
			ss << "indicated by 'Key Size in bits' directive!" << endl;
			ss << "Error: Key file parsing error reading, ECC Public Key Comp X"
					<< endl;
			m_TimDescriptor.ParsingError(ss.str(), true, pLine);
			bRet = false;
		}
		pPrevLine = pLine;
	}

	if ((pLine = m_TimDescriptor.GetNextLineField("ECC Public Key Comp Y",
			pLine, true, 1)) == 0)
		bRet = false;

	if (pLine) {
		pPrevLine = pLine;
	} else
		pLine = pPrevLine;

	if (pKey->CKey::KeySize() > 0) {
		IsOk = FillList(pLine, pKey->ECDSAPublicKeyCompYList(),
				(((pKey->CKey::KeySize() + 31) / 32) * 4));
		if (IsOk == false) {
			stringstream ss;
			ss << "Key Size in bits = " << pKey->CKey::KeySize() << endl;
			ss
					<< "  Error: The amount of data read from key file doesn't match what is"
					<< endl;
			ss << "indicated by 'Key Size in bits' directive!" << endl;
			ss
					<< "Error: Key file parsing error reading, ECC Public Key Comp Y:"
					<< endl;
			m_TimDescriptor.ParsingError(ss.str(), true, pLine);
			bRet = false;
		}
		pPrevLine = pLine;
	}

	return bRet;
}

bool CTrustedTimDescriptorParser::ParseECCKeyBlf(CECCKey * pKey,
		CTimDescriptorLine*& pLine) {
	bool bRet = true;
	bool IsOk = false;

	if ((pLine = m_TimDescriptor.GetNextLineField("ECC_Public_Key_Comp_X",
			pLine, true, 1)) == 0)
		bRet = false;

	if (pLine) {
		pPrevLine = pLine;
	} else
		pLine = pPrevLine;

	if (pKey->CKey::KeySize() > 0) {
		IsOk = FillListBlf(pLine, pKey->ECDSAPublicKeyCompXList(),
				(((pKey->CKey::KeySize() + 31) / 32) * 4));
		if (IsOk == false) {
			stringstream ss;
			ss << "Key Size in bits = " << pKey->CKey::KeySize() << endl;
			;
			ss
					<< "  Error: The amount of data read from key file doesn't match what is"
					<< endl;
			ss << "indicated by 'Key Size in bits' directive!" << endl;
			ss << "Error: Key file parsing error reading, ECC_Public_Key_Comp_X"
					<< endl;
			m_TimDescriptor.ParsingError(ss.str(), true, pLine);
			bRet = false;
		}
		pPrevLine = pLine;
	}

	if ((pLine = m_TimDescriptor.GetNextLineField("ECC_Public_Key_Comp_Y",
			pLine, true, 3)) == 0)
		bRet = false;

	if (pLine) {
		pPrevLine = pLine;
	} else
		pLine = pPrevLine;

	if (pKey->CKey::KeySize() > 0) {
		IsOk = FillListBlf(pLine, pKey->ECDSAPublicKeyCompYList(),
				(((pKey->CKey::KeySize() + 31) / 32) * 4));
		if (IsOk == false) {
			stringstream ss;
			ss << "Key Size in bits = " << pKey->CKey::KeySize() << endl;
			ss
					<< "  Error: The amount of data read from key file doesn't match what is"
					<< endl;
			ss << "indicated by 'Key Size in bits' directive!" << endl;
			ss
					<< "Error: Key file parsing error reading, ECC_Public_Key Comp_Y:"
					<< endl;
			m_TimDescriptor.ParsingError(ss.str(), true, pLine);
			bRet = false;
		}
		pPrevLine = pLine;
	}

	return bRet;
}

bool CTrustedTimDescriptorParser::VerifyNumberOfKeys() {
	unsigned int i = 0, NumberFound = 0;
	string sLine;
	string sField;
	unsigned int iNumKeys = 0;
	bool bRet = true;

	string lcTimTxtFilePath = CTimLib::ToLower(
			m_TimDescriptor.TimDescriptorFilePath());
	bool bIsBlf = lcTimTxtFilePath.find(".blf")
			== lcTimTxtFilePath.length() - 4;

	CTimDescriptorLine* pLine = 0;
	pLine = bIsBlf ?
			m_TimDescriptor.GetLineField("Number_of_Keys", true) :
			m_TimDescriptor.GetLineField("Number of Keys", true);
	if (pLine == 0)
		bRet = false;

	if (bRet) {
		iNumKeys = Translate(pLine->m_FieldValue);

		stringstream ssKeyNum;

		unsigned int nKeyNum = 1;
		ssKeyNum.str("");
		ssKeyNum << nKeyNum << "_";

		// find first Key ID field
		pLine = bIsBlf ?
				m_TimDescriptor.GetLineField(ssKeyNum.str() + "KEY_Key_ID",
						false) :
				m_TimDescriptor.GetLineField("Key ID", false);

		while (pLine != 0) {
			// find additional Key ID fields
			nKeyNum++;
			ssKeyNum.str("");
			ssKeyNum << nKeyNum << "_";

			NumberFound++;
			pLine = bIsBlf ?
					m_TimDescriptor.GetNextLineField(
							ssKeyNum.str() + "KEY_Key_ID", pLine, false, -1) :
					m_TimDescriptor.GetNextLineField("Key ID", pLine, false,
							-1);
		}

		if (NumberFound != iNumKeys) {
			stringstream ss;
			ss
					<< "  Error: The number of keys found does not match the number of keys"
					<< endl;
			ss << "set in Number of Keys: declaration statement!" << endl;
			ss << "Number of Keys: " << iNumKeys << endl;
			ss << "Number of keys found = " << NumberFound << endl;
			m_TimDescriptor.ParsingError(ss.str(), true);
			bRet = false;
		}
	}

	return bRet;
}

bool CTrustedTimDescriptorParser::ParseDsaInfoTIM(
		CCommandLineParser& CommandLineParser) {
	UINT_T IntParam = 0;
	char szTemp[100] = { 0 };
	string sLine;
	string sField;
	bool bFound = false;
	bool bRet = true;
	CTimDescriptorLine* pLine = 0;

	CDigitalSignature& DSig = m_TimDescriptor.DigitalSignature();

	if ((pLine = CTimDescriptor::GetLineField("DSA Algorithm ID", false))
			== 0) {
		stringstream ss;
		ss
				<< "  Warning: DSA Algorithm not found in trusted TIM descriptor text file."
				<< endl;
		m_TimDescriptor.ParsingWarning(ss.str(), true);
		bRet = false;
	}

	ENCRYPTALGORITHMID_T EncryptAlgorithmId = DUMMY_ENALG;
	if (pLine) {
		pPrevLine = pLine;
		EncryptAlgorithmId = (ENCRYPTALGORITHMID_T) Translate(
				pLine->m_FieldValue);
	} else
		pLine = pPrevLine;

	if (EncryptAlgorithmId == ECDSA_256 || EncryptAlgorithmId == ECDSA_521)
		DSig.DSKey(new CECCKey);
	else
		DSig.DSKey(new CRSAKey);

	DSig.DSKey()->EncryptAlgorithmId(EncryptAlgorithmId);
	if (DSig.DSKey()->EncryptAlgorithmIdStr() == "") {
		stringstream ss;
		ss << "Unrecognized Encryption Algorithm ID." << EncryptAlgorithmId
				<< endl;
		m_TimDescriptor.ParsingError(ss.str(), true, pLine);
		bRet = false;
	}

	if ((DSig.DSKey()->EncryptAlgorithmId() == ECDSA_256
			|| DSig.DSKey()->EncryptAlgorithmId() == ECDSA_521)
			&& m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
		stringstream ss;
		ss << "DSAlgorithmID is not support for TIM version." << endl;
		ss << "TIM version: "
				<< HexFormattedAscii(
						m_TimDescriptor.getTimHeader().getVersion())
				<< " does not support ECDSA algorithms." << endl;
		m_TimDescriptor.ParsingError(ss.str(), true, pLine);
		bRet = false;
	}

	if (!bRet)
		return false;

	if ((pLine = m_TimDescriptor.GetNextLineField("Hash Algorithm ID", pLine,
			true, 1)) == 0)
		bRet = false;

	if (pLine) {
		pPrevLine = pLine;
		DSig.DSKey()->HashAlgorithmId(
				(HASHALGORITHMID_T) Translate(pLine->m_FieldValue));
	} else
		pLine = pPrevLine;

	if (DSig.DSKey()->EncryptAlgorithmId() == ECDSA_256
			|| DSig.DSKey()->EncryptAlgorithmId() == ECDSA_521) {
		if (!ParseECDSATIM(CommandLineParser, pLine))
			bRet = false;
	} else {
		if (!ParseRSATIM(CommandLineParser, pLine))
			bRet = false;
	}

	return bRet;
}

bool CTrustedTimDescriptorParser::ParseDsaInfoBlf(
		CCommandLineParser& CommandLineParser) {
	UINT_T IntParam = 0;
	char szTemp[100] = { 0 };
	string sLine;
	string sField;
	bool bFound = false;
	bool bRet = true;
	CTimDescriptorLine* pLine = 0;

	CDigitalSignature& DSig = m_TimDescriptor.DigitalSignature();

	if ((pLine = CTimDescriptor::GetLineField("[Digital_Signature_Data]", false))
			== 0) {
		stringstream ss;
		ss
				<< "  Warning: [Digital_Signature_Data] not found in trusted TIM descriptor in blf text file."
				<< endl;
		m_TimDescriptor.ParsingWarning(ss.str(), true);
		bRet = false;
	}

	if ((pLine = m_TimDescriptor.GetNextLineField("Hash_Algorithm_ID", pLine,
			true, 1)) == 0)
		bRet = false;

	string sHashAlgorithmId = "SHA-160";
	string sEncryptAlgorithmId = "";
	if (pLine) {
		pPrevLine = pLine;
		DSig.DSKey()->HashAlgorithmId(pLine->m_FieldValue);

		sHashAlgorithmId = pLine->m_FieldValue;
	} else
		pLine = pPrevLine;

	if ((pLine = CTimDescriptor::GetLineField("DSA_Algorithm", false)) == 0) {
		stringstream ss;
		ss
				<< "  Warning: DSA Algorithm not found in trusted TIM descriptor text file."
				<< endl;
		m_TimDescriptor.ParsingWarning(ss.str(), true);
		bRet = false;
	}

	if (pLine) {
		pPrevLine = pLine;
		DSig.DSKey()->EncryptAlgorithmId(pLine->m_FieldValue);

		sEncryptAlgorithmId = pLine->m_FieldValue;
	} else
		pLine = pPrevLine;

	if (sEncryptAlgorithmId == "ECDSA_256"
			|| sEncryptAlgorithmId == "ECDSA_521")
		DSig.DSKey(new CECCKey);
	else
		DSig.DSKey(new CRSAKey);

	DSig.DSKey()->EncryptAlgorithmId(sEncryptAlgorithmId);

	DSig.DSKey()->HashAlgorithmId(sHashAlgorithmId);
	if ((DSig.DSKey()->EncryptAlgorithmId() == ECDSA_256
			|| DSig.DSKey()->EncryptAlgorithmId() == ECDSA_521)
			&& m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
		stringstream ss;
		ss << "DSAlgorithmID is not supported for TIM version." << endl;
		ss << "TIM version: "
				<< HexFormattedAscii(
						m_TimDescriptor.getTimHeader().getVersion())
				<< " does not support ECDSA algorithms." << endl;
		m_TimDescriptor.ParsingError(ss.str(), true, pLine);
		bRet = false;
	}

	if (!bRet)
		return false;

	if (DSig.DSKey()->EncryptAlgorithmId() == ECDSA_256
			|| DSig.DSKey()->EncryptAlgorithmId() == ECDSA_521) {
		if (!ParseECDSABlf(DSig, CommandLineParser, pLine))
			bRet = false;
	} else {
		if (!ParseRSABlf(DSig, CommandLineParser, pLine))
			bRet = false;
	}

	if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
		// no Encrypt Algorithm ID in Keys for < TIM_3_3_00
		// so repair keys with the Ds.EncryptAlgorithmId

		t_KeyListIter iterKey = m_TimDescriptor.KeyList().begin();
		while (iterKey != m_TimDescriptor.KeyList().end()) {
			(*iterKey)->EncryptAlgorithmId(DSig.DSKey()->EncryptAlgorithmId(),
					true);
			iterKey++;
		}
	}

	return bRet;
}

bool CTrustedTimDescriptorParser::ParseDTIMKeysDataBlf(
		CCommandLineParser& CommandLineParser) {
	UINT_T IntParam = 0;
	char szTemp[100] = { 0 };
	string sLine;
	string sField;
	bool bFound = false;
	bool bRet = true;
	CTimDescriptorLine* pLine = 0;

	CExtendedReservedData& ERD = m_TimDescriptor.ExtendedReservedData();

	bool bImageMaps = false;
	t_ErdBaseVectorIter ERDIter = ERD.ErdVec.begin();
	while (ERDIter != ERD.ErdVec.end()) {
		if ((*ERDIter)->ErdPkgType() == CErdBase::IMAGE_MAP_INFO_ERD) {
			bImageMaps = true;
			break;
		}
		ERDIter++;
	}

	if (bImageMaps) {
		CDigitalSignature& DTIMSig = m_TimDescriptor.DTIMKeysDataBlf();

		if ((pLine = CTimDescriptor::GetLineField("[DTIM_Keys_Data]", false))
				== 0) {
			stringstream ss;
			ss
					<< "  Warning: [DTIM_Keys_Data] not found in trusted TIM descriptor in blf text file."
					<< endl;
			m_TimDescriptor.ParsingWarning(ss.str(), true);
			bRet = false;
		}

		if ((pLine = m_TimDescriptor.GetNextLineField("Hash_Algorithm_ID",
				pLine, true, 1)) == 0)
			bRet = false;

		if (pLine) {
			pPrevLine = pLine;
			DTIMSig.DSKey()->HashAlgorithmId(pLine->m_FieldValue);

		} else
			pLine = pPrevLine;

		if ((pLine = CTimDescriptor::GetLineField("DSA_Algorithm", false))
				== 0) {
			stringstream ss;
			ss
					<< "  Warning: DSA Algorithm not found in trusted TIM descriptor text file."
					<< endl;
			m_TimDescriptor.ParsingWarning(ss.str(), true);
			bRet = false;
		}

		string sEncryptAlgorithmId = "";
		if (pLine) {
			pPrevLine = pLine;
			sEncryptAlgorithmId = pLine->m_FieldValue;
		} else
			pLine = pPrevLine;

		if (sEncryptAlgorithmId == "ECDSA_256"
				|| sEncryptAlgorithmId == "ECDSA_521")
			DTIMSig.DSKey(new CECCKey);
		else
			DTIMSig.DSKey(new CRSAKey);

		DTIMSig.DSKey()->EncryptAlgorithmId(sEncryptAlgorithmId);

		if ((DTIMSig.DSKey()->EncryptAlgorithmId() == ECDSA_256
				|| DTIMSig.DSKey()->EncryptAlgorithmId() == ECDSA_521)
				&& m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
			stringstream ss;
			ss << "DSAlgorithmID is not supported for TIM version." << endl;
			ss << "TIM version: "
					<< HexFormattedAscii(
							m_TimDescriptor.getTimHeader().getVersion())
					<< " does not support ECDSA algorithms." << endl;
			m_TimDescriptor.ParsingError(ss.str(), true, pLine);
			bRet = false;
		}

		if (!bRet)
			return false;

		if (DTIMSig.DSKey()->EncryptAlgorithmId() == ECDSA_256
				|| DTIMSig.DSKey()->EncryptAlgorithmId() == ECDSA_521) {
			if (!ParseECDSABlf(DTIMSig, CommandLineParser, pLine))
				bRet = false;
		} else {
			if (!ParseRSABlf(DTIMSig, CommandLineParser, pLine))
				bRet = false;
		}

		if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
			// no Encrypt Algorithm ID in Keys for < TIM_3_3_00
			// so repair keys with the Ds.EncryptAlgorithmId

			t_KeyListIter iterKey = m_TimDescriptor.KeyList().begin();
			while (iterKey != m_TimDescriptor.KeyList().end()) {
				(*iterKey)->EncryptAlgorithmId(
						DTIMSig.DSKey()->EncryptAlgorithmId(), true);
				iterKey++;
			}
		}
	}

	return bRet;
}

bool CTrustedTimDescriptorParser::ParseRSATIM(
		CCommandLineParser& CommandLineParser, CTimDescriptorLine*& pLine) {
	CDigitalSignature& DSig = m_TimDescriptor.DigitalSignature();

	bool bRet = true;
	bool IsOk = false;
	unsigned int i = 0;

	if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
		if ((pLine = m_TimDescriptor.GetNextLineField("Modulus Size in bytes",
				pLine, true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
		} else {
			pPrevLine = pLine;
			DSig.KeySize(Translate(pLine->m_FieldValue) * 8);
		}
	} else {
		if ((pLine = m_TimDescriptor.GetNextLineField("Key Size in bits", pLine,
				true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
		} else {
			pPrevLine = pLine;
			DSig.KeySize(Translate(pLine->m_FieldValue));
		}
	}

	if ((pLine = m_TimDescriptor.GetNextLineField("RSA Public Exponent", pLine,
			true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else
		pPrevLine = pLine;

	CRSAKey* pRSAKey = dynamic_cast<CRSAKey*>(DSig.DSKey());

	if (DSig.KeySize() > 0 && pRSAKey) {
		IsOk = FillList(pLine, pRSAKey->PublicKeyExponentList(),
				DSig.KeySize() / 8);
		if (IsOk == false) {
			stringstream ss;
			if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
				ss << "Modulus Size in bytes = " << DSig.KeySize() / 8 << endl;
			} else {
				ss << "Key Size in bits = " << DSig.KeySize() << endl;
			}
			ss
					<< "  Error: The amount of data read from key file doesn't match what is"
					<< endl;
			if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
				ss << "indicated by 'Modulus Size in bytes' directive!" << endl;
			} else {
				ss << "indicated by 'Key Size in bits' directive!" << endl;
			}
			ss << "Error: RSA Public Exponent for DSA Algorithm" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true);
			bRet = false;
		}
		pPrevLine = pLine;

		DSig.IncludeInTim(DSig.KeySize() > 0 ? true : false);
	}

	if ((pLine = m_TimDescriptor.GetNextLineField("RSA System Modulus", pLine,
			true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else
		pPrevLine = pLine;

	if (DSig.KeySize() > 0 && pRSAKey) {
		IsOk = FillList(pLine, pRSAKey->RsaSystemModulusList(),
				DSig.KeySize() / 8);
		if (IsOk == false) {
			stringstream ss;
			if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
				ss << "Modulus Size in bytes = " << DSig.KeySize() / 8 << endl;
				;
			} else {
				ss << "Key Size in bits = " << DSig.KeySize() << endl;
			}

			ss
					<< "  Error: The amount of data read from key file doesn't match what is"
					<< endl;
			if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
				ss << "indicated by 'Modulus Size in bytes' directive!" << endl;
			} else {
				ss << "indicated by 'Key Size in bits' directive!" << endl;
			}
			ss << "Error: RSA System Modulus for DSA Algorithm" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true);
			bRet = false;
		}
		pPrevLine = pLine;
	}

	if (CommandLineParser.iOption == 1) {
		if ((pLine = m_TimDescriptor.GetNextLineField("RSA Private Key", pLine,
				true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
		} else
			pPrevLine = pLine;

		if (DSig.KeySize() > 0 && pRSAKey) {
			IsOk = FillList(pLine, pRSAKey->RsaPrivateKeyList(),
					DSig.KeySize() / 8);
			if (IsOk == false) {
				stringstream ss;
				if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
					ss << "Modulus Size in bytes = " << DSig.KeySize() / 8
							<< endl;
				} else {
					ss << "Key Size in bits = " << DSig.KeySize() << endl;
				}

				ss
						<< "  Error: The amount of data read from key file doesn't match what is"
						<< endl;
				if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
					ss << "indicated by 'Modulus Size in bytes' directive!"
							<< endl;
				} else {
					ss << "indicated by 'Key Size in bits' directive!" << endl;
				}
				ss << "Error: RSA Private Key for DSA Algorithm" << endl;
				m_TimDescriptor.ParsingError(ss.str(), true);
				bRet = false;
			}
			pPrevLine = pLine;
		}
	}

	return bRet;
}

bool CTrustedTimDescriptorParser::ParseRSABlf(CDigitalSignature& DSig,
		CCommandLineParser& CommandLineParser, CTimDescriptorLine*& pLine) {
	bool bRet = true;
	bool IsOk = false;
	unsigned int i = 0;

	if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
		if ((pLine = m_TimDescriptor.GetNextLineField("Modulus_Size_in_bytes",
				pLine, true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
		} else {
			pPrevLine = pLine;
			DSig.KeySize(Translate(pLine->m_FieldValue) * 8);
		}
	} else {
		if ((pLine = m_TimDescriptor.GetNextLineField("Key_Size_in_bits", pLine,
				true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
		} else {
			pPrevLine = pLine;
			DSig.KeySize(Translate(pLine->m_FieldValue));
		}
	}

	if ((pLine = m_TimDescriptor.GetNextLineField("RSA_Public_Exponent", pLine,
			true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else
		pPrevLine = pLine;

	CRSAKey* pRSAKey = dynamic_cast<CRSAKey*>(DSig.DSKey());

	if (DSig.KeySize() > 0 && pRSAKey) {
		IsOk = FillListBlf(pLine, pRSAKey->PublicKeyExponentList(),
				DSig.KeySize() / 8);
		if (IsOk == false) {
			stringstream ss;
			if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
				ss << "Modulus Size in bytes = " << DSig.KeySize() / 8 << endl;
			} else {
				ss << "Key Size in bits = " << DSig.KeySize() << endl;
			}
			ss
					<< "  Error: The amount of data read from key file doesn't match what is"
					<< endl;
			if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
				ss << "indicated by 'Modulus Size in bytes' directive!" << endl;
			} else {
				ss << "indicated by 'Key Size in bits' directive!" << endl;
			}
			ss << "Error: RSA Public Exponent for DSA Algorithm" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true);
			bRet = false;
		}
		pPrevLine = pLine;

		DSig.IncludeInTim(DSig.KeySize() > 0 ? true : false);
	}

	if ((pLine = m_TimDescriptor.GetNextLineField("End_RSA_Public_Exponent",
			pLine, true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else
		pPrevLine = pLine;

	if ((pLine = m_TimDescriptor.GetNextLineField("RSA_System_Modulus", pLine,
			true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else
		pPrevLine = pLine;

	if (DSig.KeySize() > 0 && pRSAKey) {
		IsOk = FillListBlf(pLine, pRSAKey->RsaSystemModulusList(),
				DSig.KeySize() / 8);
		if (IsOk == false) {
			stringstream ss;
			if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
				ss << "Modulus Size in bytes = " << DSig.KeySize() / 8 << endl;
				;
			} else {
				ss << "Key Size in bits = " << DSig.KeySize() << endl;
			}

			ss
					<< "  Error: The amount of data read from key file doesn't match what is"
					<< endl;
			if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
				ss << "indicated by 'Modulus Size in bytes' directive!" << endl;
			} else {
				ss << "indicated by 'Key Size in bits' directive!" << endl;
			}
			ss << "Error: RSA System Modulus for DSA Algorithm" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true);
			bRet = false;
		}
		pPrevLine = pLine;
	}

	if ((pLine = m_TimDescriptor.GetNextLineField("End_RSA_System_Modulus",
			pLine, true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else
		pPrevLine = pLine;

	if (CommandLineParser.iOption == 1) {
		if ((pLine = m_TimDescriptor.GetNextLineField("RSA_Private_Key", pLine,
				true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
		} else
			pPrevLine = pLine;

		if (DSig.KeySize() > 0 && pRSAKey) {
			IsOk = FillListBlf(pLine, pRSAKey->RsaPrivateKeyList(),
					DSig.KeySize() / 8);
			if (IsOk == false) {
				stringstream ss;
				if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
					ss << "Modulus Size in bytes = " << DSig.KeySize() / 8
							<< endl;
				} else {
					ss << "Key Size in bits = " << DSig.KeySize() << endl;
				}

				ss
						<< "  Error: The amount of data read from key file doesn't match what is"
						<< endl;
				if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
					ss << "indicated by 'Modulus Size in bytes' directive!"
							<< endl;
				} else {
					ss << "indicated by 'Key Size in bits' directive!" << endl;
				}
				ss << "Error: RSA Private Key for DSA Algorithm" << endl;
				m_TimDescriptor.ParsingError(ss.str(), true);
				bRet = false;
			}
			pPrevLine = pLine;
		}

		if ((pLine = m_TimDescriptor.GetNextLineField("End_RSA_Private_Key",
				pLine, true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
		} else
			pPrevLine = pLine;
	}

	return bRet;
}

bool CTrustedTimDescriptorParser::ParseECDSATIM(
		CCommandLineParser& CommandLineParser, CTimDescriptorLine*& pLine) {
	bool IsOk = false;
	unsigned int i = 0;
	bool bRet = true;

	CDigitalSignature& DSig = m_TimDescriptor.DigitalSignature();
	CECCKey* pECCKey = dynamic_cast<CECCKey*>(DSig.DSKey());

	if ((pLine = m_TimDescriptor.GetNextLineField("Key Size in bits", pLine,
			true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else {
		pPrevLine = pLine;
		DSig.KeySize(Translate(pLine->m_FieldValue));
	}

	if ((pLine = m_TimDescriptor.GetNextLineField("ECDSA Public Key CompX",
			pLine, true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else {
		pPrevLine = pLine;
	}

	if (bRet && DSig.KeySize() > 0 && pECCKey) {
		IsOk = FillList(pLine, pECCKey->ECDSAPublicKeyCompXList(),
				(((DSig.KeySize() + 31) / 32) * 4));
		if (IsOk == false) {
			stringstream ss;
			ss << endl << "Key Size in bits = " << DSig.KeySize() << endl;
			ss
					<< "  Error: The amount of data read from key file doesn't match what is"
					<< endl;
			ss << "indicated by 'Key Size in bits' directive!" << endl;
			ss << "Error: ECDSA Public Key CompX for DSA Algorithm" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true);
			bRet = false;
		}
		pPrevLine = pLine;

		DSig.IncludeInTim(DSig.KeySize() > 0 ? true : false);
	}

	if ((pLine = m_TimDescriptor.GetNextLineField("ECDSA Public Key CompY",
			pLine, true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else {
		pPrevLine = pLine;
	}

	if (bRet && DSig.KeySize() > 0 && pECCKey) {
		IsOk = FillList(pLine, pECCKey->ECDSAPublicKeyCompYList(),
				(((DSig.KeySize() + 31) / 32) * 4));
		if (IsOk == false) {
			stringstream ss;
			ss << endl << "Key Size in bits = " << DSig.KeySize() << endl;
			ss
					<< "  Error: The amount of data read from key file doesn't match what is"
					<< endl;
			ss << "indicated by 'Key Size in bits' directive!" << endl;
			ss << "Error: ECDSA Public Key CompY for DSA Algorithm" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true);
			bRet = false;
		}
		pPrevLine = pLine;

		if (CommandLineParser.iOption == 1) {
			if ((pLine = m_TimDescriptor.GetNextLineField("ECDSA Private Key",
					pLine, true, 1)) == 0) {
				pLine = pPrevLine;
				bRet = false;
			} else {
				pPrevLine = pLine;
			}
			if (bRet && DSig.KeySize() > 0 && pECCKey) {
				IsOk = FillList(pLine, pECCKey->ECDSAPrivateKeyList(),
						(((DSig.KeySize() + 31) / 32) * 4));
				if (IsOk == false) {
					stringstream ss;
					ss << endl << "Key Size in bits = " << DSig.KeySize()
							<< endl;
					ss
							<< "  Error: The amount of data read from key file doesn't match what is"
							<< endl;
					ss << "indicated by 'Key Size in bits' directive!" << endl;
					ss << "Error: ECDSA Private Key for DSA Algorithm" << endl;
					m_TimDescriptor.ParsingError(ss.str(), true);
					bRet = false;
				}
				pPrevLine = pLine;
			}
		}
	}

	return bRet;
}

bool CTrustedTimDescriptorParser::ParseECDSABlf(CDigitalSignature& DSig,
		CCommandLineParser& CommandLineParser, CTimDescriptorLine*& pLine) {
	bool IsOk = false;
	unsigned int i = 0;
	bool bRet = true;

	CECCKey* pECCKey = dynamic_cast<CECCKey*>(DSig.DSKey());

	if ((pLine = m_TimDescriptor.GetNextLineField("Key_Size_in_bits", pLine,
			true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else {
		pPrevLine = pLine;
		DSig.KeySize(Translate(pLine->m_FieldValue));
	}

	if ((pLine = m_TimDescriptor.GetNextLineField("ECDSA_Public_Key_CompX",
			pLine, true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else {
		pPrevLine = pLine;
	}

	if (bRet && DSig.KeySize() > 0 && pECCKey) {
		IsOk = FillListBlf(pLine, pECCKey->ECDSAPublicKeyCompXList(),
				(((DSig.KeySize() + 31) / 32) * 4));
		if (IsOk == false) {
			stringstream ss;
			ss << endl << "Key Size in bits = " << DSig.KeySize() << endl;
			ss
					<< "  Error: The amount of data read from key file doesn't match what is"
					<< endl;
			ss << "indicated by 'Key Size in bits' directive!" << endl;
			ss << "Error: ECDSA Public Key CompX for DSA Algorithm" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true);
			bRet = false;
		}
		pPrevLine = pLine;

		DSig.IncludeInTim(DSig.KeySize() > 0 ? true : false);
	}

	if ((pLine = m_TimDescriptor.GetNextLineField("End_ECDSA_Public_Key_CompX",
			pLine, true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else {
		pPrevLine = pLine;
	}

	if ((pLine = m_TimDescriptor.GetNextLineField("ECDSA_Public_Key_CompY",
			pLine, true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else {
		pPrevLine = pLine;
	}

	if (bRet && DSig.KeySize() > 0 && pECCKey) {
		IsOk = FillListBlf(pLine, pECCKey->ECDSAPublicKeyCompYList(),
				(((DSig.KeySize() + 31) / 32) * 4));
		if (IsOk == false) {
			stringstream ss;
			ss << endl << "Key Size in bits = " << DSig.KeySize() << endl;
			ss
					<< "  Error: The amount of data read from key file doesn't match what is"
					<< endl;
			ss << "indicated by 'Key Size in bits' directive!" << endl;
			ss << "Error: ECDSA Public Key CompY for DSA Algorithm" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true);
			bRet = false;
		}
		pPrevLine = pLine;
	}

	if ((pLine = m_TimDescriptor.GetNextLineField("End_ECDSA_Public_Key_CompY",
			pLine, true, 1)) == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else {
		pPrevLine = pLine;
	}

	if (CommandLineParser.iOption == 1) {
		if ((pLine = m_TimDescriptor.GetNextLineField("ECDSA_Private_Key",
				pLine, true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
		} else {
			pPrevLine = pLine;
		}

		if (bRet && DSig.KeySize() > 0 && pECCKey) {
			IsOk = FillListBlf(pLine, pECCKey->ECDSAPrivateKeyList(),
					(((DSig.KeySize() + 31) / 32) * 4));
			if (IsOk == false) {
				stringstream ss;
				ss << endl << "Key Size in bits = " << DSig.KeySize() << endl;
				ss
						<< "  Error: The amount of data read from key file doesn't match what is"
						<< endl;
				ss << "indicated by 'Key Size in bits' directive!" << endl;
				ss << "Error: ECDSA Private Key for DSA Algorithm" << endl;
				m_TimDescriptor.ParsingError(ss.str(), true);
				bRet = false;
			}
			pPrevLine = pLine;
		}

		if ((pLine = m_TimDescriptor.GetNextLineField("End_ECDSA_Private_Key",
				pLine, true, 1)) == 0) {
			pLine = pPrevLine;
			bRet = false;
		} else {
			pPrevLine = pLine;
		}
	}

	return bRet;
}

bool CTrustedTimDescriptorParser::ParseKeyFile(
		CCommandLineParser& CommandLineParser, string& sKeyFilePath,
		CKey*& pKey, bool bIgnorePrivateKey/* = false*/) {
	bool IsOk = false;
	UINT_T IntParam = 0;
	char szTemp[100] = { 0 };
	unsigned int i = 0;
	string sValue;
	bool bFound = false;
	bool bRet = true;

	sKeyFilePath = CTimLib::MakePathAbsoluteOrRelativeToTIMTxtPath(sKeyFilePath,
			CommandLineParser.TimTxtFilePath,
			CommandLineParser.bTimTxtRelative);
	printf("\nParsing Key file: <%s>\n", sKeyFilePath.c_str());

	// Reading key from tim descriptor file
	ifstream m_ifsKeyTxtFile;
	m_ifsKeyTxtFile.open(sKeyFilePath.c_str(), ios_base::in);
	if (m_ifsKeyTxtFile.bad() || m_ifsKeyTxtFile.fail())
		return false;

	// reset to beginning of file
	m_ifsKeyTxtFile.seekg(0, ios_base::beg);

	// support new key text file format for key gen and encrypt key

	if ((bRet = (GetSValue(m_ifsKeyTxtFile, "Key ID:", sValue, false)))) {
		// Key ID is the first field in the KeyGenSpecFile
		string sKeyTag = sValue;
		unsigned int iEncryptAlgorithmId = DUMMY_ENALG;

		if ((bRet =
				(GetSValue(m_ifsKeyTxtFile, "Encrypt Algorithm ID:", sValue))))
			iEncryptAlgorithmId = Translate(sValue);

		delete pKey;
		if (iEncryptAlgorithmId == ECDSA_256
				|| iEncryptAlgorithmId == ECDSA_521)
			pKey = new CECCKey;
		else
			pKey = new CRSAKey;

		pKey->KeyTag(sKeyTag);
		pKey->EncryptAlgorithmId((ENCRYPTALGORITHMID_T) iEncryptAlgorithmId);
		if (pKey->EncryptAlgorithmIdStr() == "") {
			cout << endl << "Error: Unrecognized Encryption Algorithm ID"
					<< iEncryptAlgorithmId << endl;
			bRet = false;
		}

		if ((bRet = (GetSValue(m_ifsKeyTxtFile, "Hash Algorithm ID:", sValue))))
			pKey->HashAlgorithmId((HASHALGORITHMID_T) Translate(sValue));

		// get key size and make it consistent with encryption algorithm
		if ((bRet = (GetSValue(m_ifsKeyTxtFile, "Key Size in bits:", sValue)))) {
			if (pKey->EncryptAlgorithmId() == ECDSA_256
					&& Translate(sValue) != 256) {
				printf(
						"Key Size not consistent with Encrypt Algorithm ID. Forced to 256 bits.\n");
				pKey->KeySize(256);
			} else if (pKey->EncryptAlgorithmId() == ECDSA_521
					&& Translate(sValue) != 521) {
				printf(
						"Key Size not consistent with Encrypt Algorithm ID. Forced to 521 bits.\n");
				pKey->KeySize(521);
			} else if (iEncryptAlgorithmId != ECDSA_256
					&& iEncryptAlgorithmId != ECDSA_521
					&& (Translate(sValue) != 1024 && Translate(sValue) != 2048)) {
				printf(
						"Key Size not consistent with Encrypt Algorithm ID. Forced to 1024 bits.\n");
				pKey->KeySize(1024);
			} else {
				// consistent size
				pKey->KeySize(Translate(sValue));
			}

			pKey->PublicKeySize(pKey->KeySize());
		}
	} else {
		// support old key text file format use in TIM < TIM_3_3_00
		m_ifsKeyTxtFile.seekg(0, ios_base::beg);

		unsigned int iEncryptAlgorithmId = DUMMY_ENALG;
		if ((bRet = (GetSValue(m_ifsKeyTxtFile, "DSA Algorithm ID:", sValue))))
			iEncryptAlgorithmId = Translate(sValue);

		delete pKey;
		if (iEncryptAlgorithmId == ECDSA_256
				|| iEncryptAlgorithmId == ECDSA_521)
			pKey = new CECCKey;
		else
			pKey = new CRSAKey;

		pKey->EncryptAlgorithmId((ENCRYPTALGORITHMID_T) iEncryptAlgorithmId);
		if (pKey->EncryptAlgorithmId() == DUMMY_ENALG) {
			bRet = false;
			cout << endl << "Error: Unrecognized Encryption Algorithm ID!"
					<< endl;
			return bRet;
		}

		if ((bRet = (GetSValue(m_ifsKeyTxtFile, "Hash Algorithm ID:", sValue))))
			pKey->HashAlgorithmId((HASHALGORITHMID_T) Translate(sValue));

		if ((bRet = (GetSValue(m_ifsKeyTxtFile, "Key Size in bits:", sValue)))) {
			if (iEncryptAlgorithmId == ECDSA_256 && Translate(sValue) != 256) {
				printf(
						"Key Size not consistent with Encrypt Algorithm ID. Forced to 256 bits.\n");
				pKey->KeySize(256);
			} else if (iEncryptAlgorithmId == ECDSA_521
					&& Translate(sValue) != 521) {
				printf(
						"Key Size not consistent with Encrypt Algorithm ID. Forced to 521 bits.\n");
				pKey->KeySize(521);
			} else if (iEncryptAlgorithmId != ECDSA_256
					&& iEncryptAlgorithmId != ECDSA_521
					&& (Translate(sValue) != 1024 && Translate(sValue) != 2048)) {
				printf(
						"Key Size not consistent with Encrypt Algorithm ID. Forced to 1024 bits.\n");
				pKey->KeySize(1024);
			} else {
				// consistent size
				pKey->KeySize(Translate(sValue));
			}
			pKey->PublicKeySize(pKey->KeySize());
		}
	}

	bool bEC = (pKey->EncryptAlgorithmId() == ECDSA_256
			|| pKey->EncryptAlgorithmId() == ECDSA_521);
	CECCKey* pECCKey = dynamic_cast<CECCKey*>(pKey);
	CRSAKey* pRSAKey = dynamic_cast<CRSAKey*>(pKey);

	if (bEC)
		//if ( bVer0303 && bEC )
		bRet = GetSValue(m_ifsKeyTxtFile, "ECDSA Public Key CompX:", sValue);
	else
		bRet = GetSValue(m_ifsKeyTxtFile, "RSA Public Exponent:", sValue);

	if (bRet && pKey->KeySize() > 0) {
		if (bEC)
			IsOk = FillList(m_ifsKeyTxtFile, pECCKey->ECDSAPublicKeyCompXList(),
					(((pKey->KeySize() + 31) / 32) * 4));
		else
			IsOk = FillList(m_ifsKeyTxtFile, pRSAKey->PublicKeyExponentList(),
					pKey->KeySize() / 8);

		if (IsOk == false) {
			stringstream ss;
			ss << "Key Size in bits = " << pKey->KeySize() << endl;
			ss
					<< "  Error: The amount of data read from key file doesn't match what is"
					<< endl;
			ss << "indicated by 'Key Size in bits' directive!" << endl;
			ss << "Error: ECDSA Public Key CompX for DSA Algorithm" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true);
			bRet = false;
		}
	}

	if (bRet && bEC)
		//if ( bRet && bVer0303 && bEC )
		bRet = GetSValue(m_ifsKeyTxtFile, "ECDSA Public Key CompY:", sValue);
	else
		bRet = GetSValue(m_ifsKeyTxtFile, "RSA System Modulus:", sValue);

	if (bRet && pKey->KeySize() > 0) {
		if (bEC)
			IsOk = FillList(m_ifsKeyTxtFile, pECCKey->ECDSAPublicKeyCompYList(),
					(((pKey->KeySize() + 31) / 32) * 4));
		else
			IsOk = FillList(m_ifsKeyTxtFile, pRSAKey->RsaSystemModulusList(),
					pKey->KeySize() / 8);

		if (IsOk == false) {
			stringstream ss;
			if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00)
				ss << "Modulus Size in bytes = " << (pKey->KeySize() + 7) / 8
						<< endl;
			else
				ss << "Key Size in bits = " << pKey->KeySize() << endl;

			ss
					<< "  Error: The amount of data read from key file doesn't match what is"
					<< endl;
			if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
				ss << "indicated by 'Modulus Size in bytes' directive!" << endl;
				ss << "Error: RSA System Modulus for DSA Algorithm" << endl;
			} else {
				ss << "indicated by 'Key Size in bits' directive!" << endl;
				ss << "Error: RSA or ECDSA Public Key CompY for DSA Algorithm"
						<< endl;
			}
			m_TimDescriptor.ParsingError(ss.str(), true);
			bRet = false;
		}

		//i = 0;

		if (bRet && !bIgnorePrivateKey) {
			if (bEC)
				//if ( bVer0303 && bEC )
				bRet = GetSValue(m_ifsKeyTxtFile, "ECDSA Private Key:", sValue);
			else
				bRet = GetSValue(m_ifsKeyTxtFile, "RSA Private Key:", sValue);

			if (!bRet) {
				printf(
						"\nWarning: No Private Key in Key File, using all 0's for Private Key.\n");
			} else {
				if (pKey->KeySize() > 0) {
					if (bEC)
						IsOk = FillList(m_ifsKeyTxtFile,
								pECCKey->ECDSAPrivateKeyList(),
								(((pKey->KeySize() + 31) / 32) * 4));
					else
						IsOk = FillList(m_ifsKeyTxtFile,
								pRSAKey->RsaPrivateKeyList(),
								pKey->KeySize() / 8);

					if (IsOk == false) {
						stringstream ss;
						if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00)
							ss << "Modulus Size in bytes = "
									<< (pKey->KeySize() + 7) / 8 << endl;
						else
							ss << "Key Size in bits = " << pKey->KeySize()
									<< endl;

						ss
								<< "  Error: The amount of data read from key file doesn't match what is"
								<< endl;
						if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
							ss
									<< "indicated by 'Modulus Size in bytes' directive!"
									<< endl;
							ss << "Error: RSA Private Key for DSA Algorithm"
									<< endl;
						} else {
							ss << "indicated by 'Key Size in bits' directive!"
									<< endl;
							ss
									<< "Error: RSA or ECDSA Private Key for DSA Algorithm"
									<< endl;
						}
						m_TimDescriptor.ParsingError(ss.str(), true);
						bRet = false;
					}
				}
			}
		}
	}

	if (bRet) {
		printf("\nParsing Key file successful.\n");
	} else {
		printf("\nParsing Key file failed.\n");
	}

	m_ifsKeyTxtFile.close();
	return bRet;
}

bool CTrustedTimDescriptorParser::ParseBinaryKeyFile(string& sKeyBinaryFilePath,
		CCommandLineParser& CommandLineParser, CKey*& pKey) {
	if (sKeyBinaryFilePath.empty())
		return false;

	sKeyBinaryFilePath = CTimLib::MakePathAbsoluteOrRelativeToTIMTxtPath(
			sKeyBinaryFilePath, CommandLineParser.TimTxtFilePath,
			CommandLineParser.bTimTxtRelative);

	ifstream ifsKeyBinaryFile;
	ifsKeyBinaryFile.open(sKeyBinaryFilePath.c_str(),
			ios_base::in | ios::binary);
	if (ifsKeyBinaryFile.fail() || ifsKeyBinaryFile.bad()) {
		printf("\n  Error: Cannot open file name <%s> !",
				sKeyBinaryFilePath.c_str());
		return false;
	}

	// get the file length for allocating a buffer to hold the file
	ifsKeyBinaryFile.seekg(0, ios_base::end);
	ifsKeyBinaryFile.clear();
	unsigned int ifsPos = (unsigned int) ifsKeyBinaryFile.tellg();
	ifsKeyBinaryFile.seekg(0, ios_base::beg);
	if (ifsPos <= 0) {
		printf("\n  Error: Binary Key File is empty: <%s>!",
				sKeyBinaryFilePath.c_str());
		return false;
	}

	// alloc a data array to load the file
	char* FileData = new char[ifsPos];
	memset(FileData, 0, ifsPos);
	ifsKeyBinaryFile.read(FileData, ifsPos);
	if (ifsKeyBinaryFile.fail()) {
		printf("\n  Error: Failed to read all of file: <%s>!",
				sKeyBinaryFilePath.c_str());
		return false;
	}

	unsigned int iValuePos = 0;
	string sKeyId = HexAsciiToText(
			HexFormattedAscii(*(unsigned int*) &FileData[iValuePos]));
	iValuePos += 4;
	HASHALGORITHMID_T iHashAlgorithmID =
			(HASHALGORITHMID_T) *(unsigned int*) &FileData[iValuePos];
	iValuePos += 4;
	unsigned int iEncryptAlgorithmID = *(unsigned int*) &FileData[iValuePos];
	iValuePos += 4;

	delete pKey;
	bool bEncrypted = (iEncryptAlgorithmID & EncryptedBitMask) > 0;
	if (iEncryptAlgorithmID == ECDSA_256 || iEncryptAlgorithmID == ECDSA_521)
		pKey = new CECCKey;
	else
		pKey = new CRSAKey;

	pKey->KeyId(sKeyId);
	pKey->HashAlgorithmId(iHashAlgorithmID);
	pKey->EncryptAlgorithmId((ENCRYPTALGORITHMID_T) iEncryptAlgorithmID);

	pKey->KeySize((*(unsigned int*) &FileData[iValuePos]));
	iValuePos += 4;

	if (pKey->EncryptAlgorithmId() == ECDSA_256
			|| pKey->EncryptAlgorithmId() == ECDSA_521) {
		CECCKey* pECCKey = dynamic_cast<CECCKey*>(pKey);

		t_stringListIter iterPubExp =
				pECCKey->ECDSAPublicKeyCompXList().begin();
		while (iterPubExp != pECCKey->ECDSAPublicKeyCompXList().end()) {
			*(*iterPubExp) = HexFormattedAscii(
					*(unsigned int*) &FileData[iValuePos]);
			iValuePos += 4;
			iterPubExp++;
		}

		// discard unused bytes from file because MAXECCKEYSIZEWORDS were written
		// though not all bytes were actually used and should be 0's
		size_t discard = MAXECCKEYSIZEWORDS
				- pECCKey->ECDSAPublicKeyCompXList().size();
		while (discard-- > 0) {
			unsigned int discardedValue = *(unsigned int*) &FileData[iValuePos];
			iValuePos += 4;
		}

		iterPubExp = pECCKey->ECDSAPublicKeyCompYList().begin();
		while (iterPubExp != pECCKey->ECDSAPublicKeyCompYList().end()) {
			*(*iterPubExp) = HexFormattedAscii(
					*(unsigned int*) &FileData[iValuePos]);
			iValuePos += 4;
			iterPubExp++;
		}

		// discard unused bytes from file because MAXECCKEYSIZEWORDS were written
		// though not all bytes were actually used and should be 0's
		discard = MAXECCKEYSIZEWORDS
				- pECCKey->ECDSAPublicKeyCompYList().size();
		while (discard-- > 0) {
			unsigned int discardedValue = *(unsigned int*) &FileData[iValuePos];
			iValuePos += 4;
		}

		t_stringListIter iterPrivExp = pECCKey->ECDSAPrivateKeyList().begin();
		while (iterPrivExp != pECCKey->ECDSAPrivateKeyList().end()) {
			*(*iterPrivExp) = HexFormattedAscii(
					*(unsigned int*) &FileData[iValuePos]);
			iValuePos += 4;
			iterPrivExp++;
		}

		// discard unused bytes from file because MAXECCKEYSIZEWORDS were written
		// though not all bytes were actually used and should be 0's
		discard = MAXECCKEYSIZEWORDS - pECCKey->ECDSAPrivateKeyList().size();
		while (discard-- > 0) {
			unsigned int discardedValue = *(unsigned int*) &FileData[iValuePos];
			iValuePos += 4;
		}
	} else {
		CRSAKey* pRSAKey = dynamic_cast<CRSAKey*>(pKey);

		t_stringListIter iterPubExp = pRSAKey->PublicKeyExponentList().begin();
		while (iterPubExp != pRSAKey->PublicKeyExponentList().end()) {
			*(*iterPubExp) = HexFormattedAscii(
					*(unsigned int*) &FileData[iValuePos]);
			iValuePos += 4;
			iterPubExp++;
		}

		// discard unused bytes from file because MAXRSAKEYSIZEWORDS were written
		// though not all bytes were actually used and should be 0's
		size_t discard = MAXRSAKEYSIZEWORDS
				- pRSAKey->PublicKeyExponentList().size();
		while (discard-- > 0) {
			unsigned int discardedValue = *(unsigned int*) &FileData[iValuePos];
			iValuePos += 4;
		}

		t_stringListIter iterSysMod = pRSAKey->RsaSystemModulusList().begin();
		while (iterSysMod != pRSAKey->RsaSystemModulusList().end()) {
			*(*iterSysMod) = HexFormattedAscii(
					*(unsigned int*) &FileData[iValuePos]);
			iValuePos += 4;
			iterSysMod++;
		}

		// discard unused bytes from file because MAXRSAKEYSIZEWORDS were written
		// though not all bytes were actually used and should be 0's
		discard = MAXRSAKEYSIZEWORDS - pRSAKey->RsaSystemModulusList().size();
		while (discard-- > 0) {
			unsigned int discardedValue = *(unsigned int*) &FileData[iValuePos];
			iValuePos += 4;
		}

		t_stringListIter iterPriv = pRSAKey->RsaPrivateKeyList().begin();
		while (iterPriv != pRSAKey->RsaPrivateKeyList().end()) {
			*(*iterPriv) = HexFormattedAscii(
					*(unsigned int*) &FileData[iValuePos]);
			iValuePos += 4;
			iterPriv++;
		}

		// discard unused bytes from file because MAXRSAKEYSIZEWORDS were written
		// though not all bytes were actually used and should be 0's
		discard = MAXRSAKEYSIZEWORDS - pRSAKey->RsaPrivateKeyList().size();
		while (discard-- > 0) {
			unsigned int discardedValue = *(unsigned int*) &FileData[iValuePos];
			iValuePos += 4;
		}
	}

	delete[] FileData;

	return true;
}

