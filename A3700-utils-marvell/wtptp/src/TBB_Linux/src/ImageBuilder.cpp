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

#include "ImageBuilder.h"
#include "ImageDescription.h"
#include "Typedef.h"
#include <cstring>
#include <cstdlib>

CImageBuilder::CImageBuilder(CCommandLineParser& rCommandLineParser,
		CTimDescriptorParser& rTimDescriptorParser) :
		CommandLineParser(rCommandLineParser), TimDescriptorParser(
				rTimDescriptorParser) {

}

CImageBuilder::~CImageBuilder(void) {

}

fstream& CImageBuilder::OpenTimBinFile(ios_base::openmode mode) {
	CloseTimBinFile();

	m_fsTimBinFile.open(TimDescriptorParser.TimDescriptorBinFilePath().c_str(),
			mode);
	if (m_fsTimBinFile.bad() || m_fsTimBinFile.fail()) {
		printf("\n  Error: Cannot open TIM Bin file name <%s> !\n",
				TimDescriptorParser.TimDescriptorBinFilePath().c_str());
	}
	return m_fsTimBinFile;
}

void CImageBuilder::CloseTimBinFile() {
	if (m_fsTimBinFile.is_open())
		m_fsTimBinFile.close();
	m_fsTimBinFile.clear();
}

bool CImageBuilder::LoadTimBinFileToTimBuffer(fstream& fsTimBinFile,
		char*& pTIMBuffer, long* plFileSize) {
	if (pTIMBuffer != 0) {
		delete[] pTIMBuffer;
		pTIMBuffer = 0;
	}
	*plFileSize = 0;

	fsTimBinFile.seekg(0, ios_base::end);  // Set position to EOF
	fsTimBinFile.clear();

	// determine the size of the pTIMBuffer
	// note that the (long) cast is dangerous for very large files but should not be an issue here
	*plFileSize = (long) fsTimBinFile.tellg();

	// allocate a buffer large enough to include the file data + padding and is a size that is word aligned
	unsigned int pad = sizeof(UINT_T) - (*plFileSize % sizeof(UINT_T));
	int buffersize = *plFileSize;
	// if pad is sizeof(UINT_T), then no pad needed
	if (pad == sizeof(UINT_T))
		buffersize += pad;

	pTIMBuffer = new char[buffersize];
	if (0 == pTIMBuffer) {
		printf(
				"////////////////////////////////////////////////////////////\n");
		printf(
				"  Error: Memory allocation for TimBuffer failed in LoadTimBinFileToTimBuffer()!\n");
		printf(
				"////////////////////////////////////////////////////////////\n");
		return false;
	}

	// pad tim if necessary
	if (CommandLineParser.bOneNANDPadding)
		memset(pTIMBuffer, 0xff, buffersize);
	else
		memset(pTIMBuffer, 0x0, buffersize);

	// allocate a TIMBuffer large enough to hold the TIM binary file
	// read in TIM file to pTIMBuffer
	fsTimBinFile.seekg(0, ios_base::beg);  // Set position SOF
	fsTimBinFile.read(pTIMBuffer, *plFileSize);
	if (fsTimBinFile.bad() || fsTimBinFile.fail()) {
		printf(
				"////////////////////////////////////////////////////////////\n");
		printf(
				"  Error: LoadTimBinFileToTimBuffer() couldn't read all of the %ld bytes\n",
				*plFileSize);
		printf("from the TIM binary image!\n");
		printf(
				"////////////////////////////////////////////////////////////\n");
		if (pTIMBuffer != 0) {
			delete[] pTIMBuffer;
			pTIMBuffer = 0;
		}
		return true;
	}

	return true;
}

bool CImageBuilder::BuildDescriptorFile() {
	bool bRet = true;
	unsigned int TIMHashSize = 0;
	char *pData = NULL;
	int numberOfLinesRead = 0;
	int sizeOfReservedDataFile = 0;
	unsigned char TIMHash[MAX_HASH] = { 0 };

	printf("\nProcessing non trusted TIM...\n\n");

	if (CommandLineParser.bIsTimTxtFile || CommandLineParser.bIsBlfTxtFile) {
		// open new or replace existing tim bin file
		ios_base::openmode mode = ios_base::in | ios_base::out
				| ios_base::binary | ios_base::trunc;
		fstream& fsTimBinFile = OpenTimBinFile(mode); // create new file
		if (fsTimBinFile.bad() || fsTimBinFile.fail())
			return false;

		TIMHashSize = GetTIMHashSize();

		if (bRet && !ProcessImages(fsTimBinFile, &TIMHashSize)) {
			printf("  Error: Failed to process TIM images.\n");
			bRet = false;
		}

		if (bRet && !ProcessReservedData(fsTimBinFile)) {
			printf("  Error: Failed to process Reserved Data Area.\n");
			bRet = false;
		}

		if (bRet && CommandLineParser.bOneNANDPadding) {
			printf("\n'%s' size in bytes prior to padding: %u\n",
					TimDescriptorParser.TimDescriptorBinFilePath().c_str(),
					TimDescriptorParser.TimDescriptor().GetTimImageSize());

			// pad out end of tim image to uiPaddedSize with all 0xFF
			const char PadByte = (char) 0xff;
			int iPadNeeded = CommandLineParser.uiPaddedSize
					- TimDescriptorParser.TimDescriptor().GetTimImageSize();

			if (iPadNeeded > 0) {
				printf("\n'%s' size in bytes after padding: %u\n",
						TimDescriptorParser.TimDescriptorBinFilePath().c_str(),
						CommandLineParser.uiPaddedSize);
			} else {
				printf("\n'%s' padding is not needed. \n",
						TimDescriptorParser.TimDescriptorBinFilePath().c_str());
			}

			while (iPadNeeded-- > 0)
				fsTimBinFile.write(&PadByte, sizeof(char));
		} else if (bRet) {
			printf("\n'%s' size in bytes: %u\n",
					TimDescriptorParser.TimDescriptorBinFilePath().c_str(),
					TimDescriptorParser.TimDescriptor().GetTimImageSize());
		}

		if (CommandLineParser.bVerbose
				&& (TimDescriptorParser.TimDescriptor().GetTimImageSize() > 4096
						|| CommandLineParser.uiPaddedSize > 4096)) {
			printf("WARNING: '%s' size is greater than 4Kb.\n",
					TimDescriptorParser.TimDescriptorBinFilePath().c_str());
		}

		if (bRet && CommandLineParser.bConcatenate) {
			if (bRet && !ProcessConcatenatedImages(fsTimBinFile)) {
				printf("Failed to process concatenated images.\n");
				bRet = false;
			}
		}

		long lFileSize = 0;
		char *pTIMBuffer = 0;

		if (!LoadTimBinFileToTimBuffer(fsTimBinFile, pTIMBuffer, &lFileSize))
			return false;

		if (pTIMBuffer == 0)
			return false;

		if (pTIMBuffer != 0 && TIMHashSize > 0
				&& (TimDescriptorParser.TimDescriptor().Image(0)->ImageSizeToHash()
						!= 0)
				&& (TimDescriptorParser.TimDescriptor().Image(0)->ImageId().compare(
						0, 3, "TIM") == 0)) {
			if (bRet
					&& !GenerateTIMHash(pTIMBuffer, lFileSize, TIMHash,
							TIMHashSize))
				bRet = false;

			if (bRet && !UpdateTIMHashInTimBin(fsTimBinFile, TIMHash))
				bRet = false;

			if (bRet && CommandLineParser.bIsTimVerify == true) {
				if (!VerifyTIMHash(fsTimBinFile))
					bRet = false;
			}
		}

		CloseTimBinFile();
		if (pTIMBuffer != 0) {
			delete[] pTIMBuffer;
			pTIMBuffer = 0;
		}
	}

	if (bRet) {
		printf("\n  Success: NTIM Processing has completed successfully!\n\n");
	} else {
		string sBadNTIMBinName = TimDescriptorParser.TimDescriptorBinFilePath();
		sBadNTIMBinName += ".bad";
		ifstream badIfs;
		badIfs.open(sBadNTIMBinName.c_str(), ios_base::in);
		if (badIfs.good() && !badIfs.fail()) {
			badIfs.close();
			remove(sBadNTIMBinName.c_str());
		}

		printf("\n  Failed: NTIM Processing has failed!\n\n");
		if (0
				!= rename(
						TimDescriptorParser.TimDescriptorBinFilePath().c_str(),
						sBadNTIMBinName.c_str())) {
			printf("\n  Unable to rename <%s> to <%s> \n",
					TimDescriptorParser.TimDescriptorBinFilePath().c_str(),
					sBadNTIMBinName.c_str());
		} else {
			printf(" Renamed incomplete NTIM binary file <%s> to <%s> \n",
					TimDescriptorParser.TimDescriptorBinFilePath().c_str(),
					sBadNTIMBinName.c_str());
		}
	}

	return bRet;
}

int CImageBuilder::GetTIMHashSize() {
	int iTotalHashSize = TimDescriptorParser.TimDescriptor().GetTimImageSize(
			CommandLineParser.bOneNANDPadding, CommandLineParser.uiPaddedSize);
	return iTotalHashSize;
}

bool CImageBuilder::ProcessImages(fstream& fsTimBinFile,
		unsigned int* pTIMHashSize) {
	if (pTIMHashSize == 0)
		return false;

	bool bRet = true;
	IMAGE_INFO_3_5_0 ImageInfo;

	string lcTimTxtFilePath = CTimLib::ToLower(
			CommandLineParser.TimTxtFilePath);
	bool bIsBlf = lcTimTxtFilePath.find(".blf")
			== lcTimTxtFilePath.length() - 4;

	CCTIM & TimHeader = TimDescriptorParser.TimDescriptor().getTimHeader();
	// return error if number of images > MAX_IMAGES
	if (TimHeader.getNumImages() > MAX_IMAGES) {
		printf(
				"  Error: Maximum allowable number of images in the descriptor file is %d!\n",
				MAX_IMAGES);
		return false;
	}

	t_ImagesList& Images = TimDescriptorParser.TimDescriptor().ImagesList();
	if (TimHeader.getNumImages() != Images.size()) {
		printf(
				"  Error: Inconsistent number of images in TimDescriptorParser m_Images and TimHeader!\n");
		return false;
	}

	CImageDescription* pImageDesc = 0;
	t_ImagesIter ImagesIter = Images.begin();
	for (int i = 0; i < (int) TimHeader.getNumImages(); i++) {
		if (ImagesIter == Images.end())
			break;

		pImageDesc = (CImageDescription*) *ImagesIter;
		memset(&ImageInfo, 0, sizeof(IMAGE_INFO_3_5_0));

		ImageInfo.ImageID = Translate(pImageDesc->ImageIdTag());
		ImageInfo.NextImageID = Translate(pImageDesc->NextImageIdTag());
		ImageInfo.FlashEntryAddr = Translate(pImageDesc->FlashEntryAddress());
		ImageInfo.LoadAddr = Translate(pImageDesc->LoadAddress());
		ImageInfo.ImageSizeToHash = pImageDesc->ImageSizeToHash();
		ImageInfo.HashAlgorithmID = pImageDesc->HashAlgorithmId();
		ImageInfo.ImageSize = pImageDesc->ImageSize();
		ImageInfo.PartitionNumber = pImageDesc->PartitionNumber();
		ImageInfo.EncAlgorithmID = pImageDesc->EncryptAlgorithmId();// See ENCRYPTALGORITHMID_T
		ImageInfo.EncryptStartOffset = pImageDesc->EncryptStartOffset();
		ImageInfo.EncryptSize = pImageDesc->EncryptSize();

		if (CommandLineParser.bImageLengthPadding) {
			if ((ImageInfo.ImageSize % CommandLineParser.uiImageModSize) != 0) {
				ImageInfo.ImageSize += CommandLineParser.uiImageModSize
						- (ImageInfo.ImageSize
								% CommandLineParser.uiImageModSize);
			}
		}

		if (i == 0) {
			// TIM IMAGE
			if ((ImageInfo.ImageID & TYPEMASK) != (TIMIDENTIFIER & TYPEMASK)) // check for TIM*
					{
				printf(
						"  Error: The ImageID value is incorrect for the TIM image!\n");
				bRet = false;
				break;
			}

			if ((((int) ImageInfo.ImageSizeToHash > *pTIMHashSize)
					|| (ImageInfo.ImageSizeToHash == 0xFFFFFFFF))) {
				// Full (N)TIM hash
				ImageInfo.ImageSizeToHash = *pTIMHashSize;
			} else {
				// otherwise, set tim hash size to requested size from TIM.txt
				*pTIMHashSize = ImageInfo.ImageSizeToHash;
			}

			unsigned ResSize = TimHeader.getSizeOfReserved();
			if (CommandLineParser.bOneNANDPadding) {
				// hack SizeOfReserved to include 0xff padded area
				unsigned int unpadded =
						TimDescriptorParser.TimDescriptor().GetTimImageSize(
								false, 0);
				unsigned int padded =
						TimDescriptorParser.TimDescriptor().GetTimImageSize(
								true, CommandLineParser.uiPaddedSize);
				TimHeader.setSizeOfReserved(
						TimHeader.getSizeOfReserved() + (padded - unpadded));
			}

			if (TimHeader.getVersion() <= TIM_3_6_00) {
				fsTimBinFile.write((const char*) &TimHeader.getTimHeader(),
						TimHeader.getSize());
			} else {
				printf(
						"  Error: Cannot write to Tim.bin of version greater than %d !",
						TIM_3_6_00);
				bRet = false;
				break;
			}

			if (CommandLineParser.bOneNANDPadding)
				// restore correct SizeOfReserved
				TimHeader.setSizeOfReserved(ResSize);
		} else {
			// NOT A TIM Image

			ifstream ifsImage;
			ifsImage.open(pImageDesc->ImageFilePath().c_str(),
					ios_base::in | ios_base::binary);
			if (ifsImage.bad() || ifsImage.fail()) {
				printf("  Error: Cannot open file name <%s> !",
						pImageDesc->ImageFilePath().c_str());
				bRet = false;
				break;
			}

			if (PrependID(ifsImage, *pImageDesc) == FALSE) {
				bRet = false;
				break;
			}

			if (ImageInfo.ImageSizeToHash != 0) {
				if ((ImageInfo.ImageSizeToHash > ImageInfo.ImageSize)
						|| (ImageInfo.ImageSizeToHash == 0xFFFFFFFF)) {
					// Full image hash
					ImageInfo.ImageSizeToHash = ImageInfo.ImageSize;
				}

				if (CImageBuilder::GenerateHash(ifsImage, *pImageDesc) == FALSE) {
					bRet = false;
					break;
				}
			}

			ifsImage.close();
		}

		memcpy(ImageInfo.Hash, pImageDesc->Hash, sizeof(ImageInfo.Hash));

		if (TimHeader.getVersion() < TIM_3_2_00) {
			// < TIM_3_2_00 uses IMAGE_INFO_3_1_0
			IMAGE_INFO_3_1_0 ImageInfo_3_1;
			ImageInfo_3_1.ImageID = ImageInfo.ImageID;
			ImageInfo_3_1.NextImageID = ImageInfo.NextImageID;
			ImageInfo_3_1.FlashEntryAddr = ImageInfo.FlashEntryAddr;
			ImageInfo_3_1.LoadAddr = ImageInfo.LoadAddr;
			ImageInfo_3_1.ImageSize = ImageInfo.ImageSize;
			ImageInfo_3_1.ImageSizeToHash = ImageInfo.ImageSizeToHash;
			ImageInfo_3_1.HashAlgorithmID = ImageInfo.HashAlgorithmID;
			memcpy(&ImageInfo_3_1.Hash, &ImageInfo.Hash,
					sizeof(ImageInfo_3_1.Hash));
			fsTimBinFile.write((const char*) &ImageInfo_3_1,
					sizeof(ImageInfo_3_1));
		} else if (TimHeader.getVersion() < TIM_3_4_00) {
			// >= TIM_3_2_00 && < TIM_3_4_00 uses IMAGE_INFO_3_2_0
			IMAGE_INFO_3_2_0 ImageInfo_3_2;
			ImageInfo_3_2.ImageID = ImageInfo.ImageID;
			ImageInfo_3_2.NextImageID = ImageInfo.NextImageID;
			ImageInfo_3_2.FlashEntryAddr = ImageInfo.FlashEntryAddr;
			ImageInfo_3_2.LoadAddr = ImageInfo.LoadAddr;
			ImageInfo_3_2.ImageSize = ImageInfo.ImageSize;
			ImageInfo_3_2.ImageSizeToHash = ImageInfo.ImageSizeToHash;
			ImageInfo_3_2.HashAlgorithmID = ImageInfo.HashAlgorithmID;
			memcpy(&ImageInfo_3_2.Hash, &ImageInfo.Hash,
					sizeof(ImageInfo_3_2.Hash));
			ImageInfo_3_2.PartitionNumber = ImageInfo.PartitionNumber;
			fsTimBinFile.write((const char*) &ImageInfo_3_2,
					sizeof(ImageInfo_3_2));
		} else if (TimHeader.getVersion() == TIM_3_4_00) {
			IMAGE_INFO_3_4_0 ImageInfo_3_4;
			ImageInfo_3_4.ImageID = ImageInfo.ImageID;
			ImageInfo_3_4.NextImageID = ImageInfo.NextImageID;
			ImageInfo_3_4.FlashEntryAddr = ImageInfo.FlashEntryAddr;
			ImageInfo_3_4.LoadAddr = ImageInfo.LoadAddr;
			ImageInfo_3_4.ImageSize = ImageInfo.ImageSize;
			ImageInfo_3_4.ImageSizeToHash = ImageInfo.ImageSizeToHash;
			ImageInfo_3_4.HashAlgorithmID = ImageInfo.HashAlgorithmID;
			memcpy(&ImageInfo_3_4.Hash, &ImageInfo.Hash,
					sizeof(ImageInfo_3_4.Hash));
			ImageInfo_3_4.PartitionNumber = ImageInfo.PartitionNumber;
			fsTimBinFile.write((const char*) &ImageInfo_3_4,
					sizeof(IMAGE_INFO_3_4_0));
		} else if (TimHeader.getVersion() >= TIM_3_5_00) {
			// >= TIM_3_5_00 uses IMAGE_INFO_3_5_0 until a new image info format is needed
			fsTimBinFile.write((const char*) &ImageInfo,
					sizeof(IMAGE_INFO_3_5_0));
		}

		ImagesIter++;
	}

	return bRet;
}

bool CImageBuilder::GenerateHash(ifstream& ifsImage,
		CImageDescription& ImageDesc) {
	// NTIM only supports SHA1 by default using the "Image Size to Crc in bytes" field
	// support can be added for SHA256/512 by adding a HashAlgorithm ID field 
	// to image section to the NTIM if needed later

	// A hash is generated from each image in its entirety and stored in the
	// Hash array of the IMAGE_INFO struct for that image in the TIM.

	// get the file length for allocating a buffer to hold the file
	ifsImage.seekg(0, ios_base::end);
	ifsImage.clear();
	unsigned int ifsPos = (long) ifsImage.tellg();
	ifsImage.seekg(0, ios_base::beg);
	if (ifsPos <= 0) {
		printf("\n  Error: File is empty!");
		return false;
	}

	// alloc a data array to load the file
	char* FileData = new char[ifsPos];
	memset(FileData, 0, ifsPos);

	ifsImage.read(FileData, ifsPos);
	if (ifsImage.fail()) {
		printf("\n  Error: Failed to read all of file!");
		delete[] FileData;
		return false;
	}

	// determine if Rem size is word aligned
	unsigned int pad = sizeof(UINT_T) - (ifsPos % sizeof(UINT_T));
	if ((pad % 4) != 0) {
		printf(
				"\nWarning:  Image File: <%s>, Size used for calculating Hash of file is not 4-byte aligned, <%u bytes>.\n",
				ImageDesc.ImageFilePath().c_str(), ImageDesc.ImageSizeToHash());
	}

	if (ImageDesc.HashAlgorithmId() == HASHALGORITHMID_T::SHA512)
		CryptoPP::SHA512().CalculateDigest((byte *) ImageDesc.Hash,
				(unsigned char *) FileData, ifsPos);
	else if (ImageDesc.HashAlgorithmId() == HASHALGORITHMID_T::SHA256)
		CryptoPP::SHA256().CalculateDigest((byte *) ImageDesc.Hash,
				(unsigned char *) FileData, ifsPos);
	else
		CryptoPP::SHA1().CalculateDigest((byte *) ImageDesc.Hash,
				(unsigned char *) FileData, ifsPos);

	delete[] FileData;

	return TRUE;
}

bool CImageBuilder::PrependID(ifstream& ifsImage,
		CImageDescription& ImageDesc) {
	string sImageOutFilename;
	ofstream ofsImageOutFile;

	// This routine simply prepends a 32 bit Image ID to a binary image and
	// saves the resultant off as a new file.

	ifsImage.clear();
	ifsImage.seekg(0, ios_base::beg);

	if (!CreateOutputImageName(ImageDesc.ImageFilePath(), sImageOutFilename))
		return false;

	ofsImageOutFile.open(sImageOutFilename.c_str(),
			ios_base::out | ios_base::trunc | ios_base::binary);
	if (ofsImageOutFile.bad() || ofsImageOutFile.fail()) {
		printf("\n  Error: fopen failed to open file: <%s>.\n",
				sImageOutFilename.c_str());
		return false;
	}

	printf("\n  Prepending ImageTag <%s> to image file: <%s>.\n",
			ImageDesc.ImageIdTag().c_str(), sImageOutFilename.c_str());
	// Prepend Image ID to output image
	int ImageId = Translate((char*) ImageDesc.ImageIdTag().c_str());
	ofsImageOutFile.write((char*) &ImageId, sizeof(UINT_T));

	const int CHUNK_SIZE = 8192 * 8;
	char* pData = new char[CHUNK_SIZE];

	// Copy input image to output image
	std::streamsize iBlockSize = CHUNK_SIZE;
	while (iBlockSize == CHUNK_SIZE) {
		memset(pData, 0, CHUNK_SIZE);

		// read a block of a size up to CHUNK_SIZE
		ifsImage.read(pData, iBlockSize);
		iBlockSize = ifsImage.gcount();
		ofsImageOutFile.write(pData, iBlockSize);
	}

	bool bRet = true;
	if (CommandLineParser.bImageLengthPadding) {
		bRet = PadImageToModNLength(ofsImageOutFile, sImageOutFilename,
				iBlockSize, CommandLineParser.uiImageModSize);
	}

	ofsImageOutFile.close();

	delete[] pData;
	return bRet;
}

bool CImageBuilder::PadImageToModNLength(ofstream& ofsImageOutFile,
		string& sImageOutFilename, std::streamsize iBlockSize,
		unsigned int uiModSize) {
	// PAD IMAGE_H.bin with filler bytes at end of file if needed
	const int IMAGEID_SIZE = 4;
	char chFill = 0x00;
	char* Fill = new char[uiModSize];
	memset(Fill, chFill, uiModSize);

	if (((iBlockSize + IMAGEID_SIZE) % uiModSize) != 0) {
		int FillSize = (uiModSize - ((iBlockSize + IMAGEID_SIZE) % uiModSize));
		printf(
				"\n  Padding image file: <%s> with <%d> bytes of filler at end of file.\n",
				sImageOutFilename.c_str(), FillSize);
		ofsImageOutFile.write((char*) Fill, FillSize);
	}

	delete[] Fill;

	return !(ofsImageOutFile.bad() || ofsImageOutFile.fail());
}

bool CImageBuilder::UpdateTIMHashInTimBin(fstream& fsTimBinFile,
		unsigned char *pTIMHash) {
	CCTIM TimHeader;
	streamoff ImageOffset = 0;

	// A check sum of the entire TIM is generated by this function. The value
	// is stored in the first image module of the TIM. Refer to header file
	// Tim.h for the layout of the IMAGE_INFO struct. The check sum will reside
	// in the base position of the Hash array within that struct.

	fsTimBinFile.seekg(0, ios_base::beg);  // Set position to SOF
	fsTimBinFile.read((char*) &TimHeader.getTimHeader(), TimHeader.getSize());
	ImageOffset = fsTimBinFile.tellg();

	if (TimHeader.getVersion() < TIM_3_2_00) {
		// < TIM_3_2_00 uses IMAGE_INFO_3_1_0
		IMAGE_INFO_3_1_0 ImageInfo_3_1;
		fsTimBinFile.read((char*) &ImageInfo_3_1, sizeof(IMAGE_INFO_3_1_0));
		memcpy(&ImageInfo_3_1.Hash, pTIMHash, sizeof(ImageInfo_3_1.Hash));
		fsTimBinFile.seekg(ImageOffset, ios_base::beg);
		fsTimBinFile.write((const char*) &ImageInfo_3_1,
				sizeof(IMAGE_INFO_3_1_0));
	} else if (TimHeader.getVersion() < TIM_3_4_00) {
		// >= TIM_3_2_00 && < TIM_3_4_00 uses IMAGE_INFO_3_2_0
		IMAGE_INFO_3_2_0 ImageInfo_3_2;
		fsTimBinFile.read((char*) &ImageInfo_3_2, sizeof(IMAGE_INFO_3_2_0));
		memcpy(&ImageInfo_3_2.Hash, pTIMHash, sizeof(ImageInfo_3_2.Hash));
		fsTimBinFile.seekg(ImageOffset, ios_base::beg);
		fsTimBinFile.write((const char*) &ImageInfo_3_2, sizeof(ImageInfo_3_2));
	} else if (TimHeader.getVersion() < TIM_3_5_00) {
		// >= TIM_3_4_00 uses IMAGE_INFO_3_4_0 until a new image info format is needed
		IMAGE_INFO_3_4_0 ImageInfo_3_4;
		fsTimBinFile.read((char*) &ImageInfo_3_4, sizeof(IMAGE_INFO_3_4_0));
		memcpy(&ImageInfo_3_4.Hash, pTIMHash, sizeof(ImageInfo_3_4.Hash));
		fsTimBinFile.seekg(ImageOffset, ios_base::beg);
		fsTimBinFile.write((const char*) &ImageInfo_3_4,
				sizeof(IMAGE_INFO_3_4_0));
	} else if (TimHeader.getVersion() >= TIM_3_5_00) {
		// >= TIM_3_5_00 uses IMAGE_INFO_3_5_0 until a new image info format is needed
		IMAGE_INFO_3_5_0 ImageInfo_3_5;
		fsTimBinFile.read((char*) &ImageInfo_3_5, sizeof(IMAGE_INFO_3_5_0));
		memcpy(&ImageInfo_3_5.Hash, pTIMHash, sizeof(ImageInfo_3_5.Hash));
		fsTimBinFile.seekg(ImageOffset, ios_base::beg);
		fsTimBinFile.write((const char*) &ImageInfo_3_5,
				sizeof(IMAGE_INFO_3_5_0));
	}

	return true;
}

bool CImageBuilder::ProcessReservedData(fstream& fsTimBinFile) {
	int iTotal = 0;
	t_ErdBaseVectorIter ErdIter;
	// create a local list that combines the ReservedData and the ExtendedReservedData
	CTimDescriptor& TimDescriptor = TimDescriptorParser.TimDescriptor();
	t_ReservedDataList ReservedDataList = TimDescriptor.ReservedDataList();
	int nBytesAdded = TimDescriptor.ExtendedReservedData().Combine(
			ReservedDataList);

	// number of packages
	int numberOfReservedPkgs = ReservedDataList.size();
	ErdIter = TimDescriptor.ExtendedReservedData().ErdVec.begin();
	if (numberOfReservedPkgs == 0)
		return true;  // no reserved data to process

	fsTimBinFile.seekg(0, ios_base::end); // Set position to EOF
	fsTimBinFile.clear();

	// reserved data area id
	DWORD dwValue = WTPRESERVEDAREAID;
	fsTimBinFile.write((const char*) &dwValue, sizeof(DWORD));
	iTotal++;

	dwValue = (DWORD) (numberOfReservedPkgs + 1); // +1 for reserved data terminator package
	fsTimBinFile.write((const char*) &dwValue, sizeof(DWORD));
	iTotal++;

	CReservedPackageData * pPackageData = 0;
	t_ReservedDataListIter iterPackage(ReservedDataList.begin());
	while (iterPackage != ReservedDataList.end()) {
		pPackageData = *iterPackage;
		// package tag id
		dwValue = Translate(pPackageData->PackageIdTag());
		fsTimBinFile.write((const char*) &dwValue, sizeof(DWORD));
		iTotal++;

		// package size in bytes
		dwValue = pPackageData->Size();
		fsTimBinFile.write((const char*) &dwValue, sizeof(DWORD));
		iTotal++;

		t_stringList& PackageDataList = pPackageData->PackageDataList();
		t_stringListIter iterData(PackageDataList.begin());
		while (iterData != PackageDataList.end()) {
			// package data values
			dwValue = Translate(*(*iterData));
			fsTimBinFile.write((const char*) &dwValue, sizeof(DWORD));
			iTotal++;
			iterData++;
		}
		iterPackage++;
	}
	int MVStructuresSize = 0;

	dwValue = TERMINATORID;
	fsTimBinFile.write((const char*) &dwValue, sizeof(DWORD));
//	fsTimBinFile << dwValue;
	iTotal++;

	dwValue = 8; // size of terminator
	fsTimBinFile.write((const char*) &dwValue, sizeof(DWORD));
//	fsTimBinFile << dwValue;
	iTotal++;

	if ((iTotal * sizeof(DWORD) + MVStructuresSize)
			!= TimDescriptor.getTimHeader().getSizeOfReserved()) {
		printf(
				"////////////////////////////////////////////////////////////\n");
		printf(
				"  Error: The number of reserved data bytes written, %d, does not equal\n",
				iTotal * 4);
		printf(
				"the number indicated by 'Size of Reserved in bytes' directive\n");
		printf("in the descriptor file ");
		if (TimDescriptorParser.TimDescriptor().ExtendedReservedData().Size()
				> 0) {
			printf("plus the size of the Extended Reserved Data, combined\n");
		}
		printf(" SizeOfReserved = %u\n",
				TimDescriptorParser.TimDescriptor().getTimHeader().getSizeOfReserved());
		printf(
				"////////////////////////////////////////////////////////////\n");
		return false;
	}

	return true;
}

bool CImageBuilder::ProcessConcatenatedImages(fstream& fsTimBinFile) {
	unsigned int location = 0;
	BYTE padding = 0x0;

	CImageDescription* pImageDesc = 0;
	t_ImagesList& Images = TimDescriptorParser.TimDescriptor().ImagesList();
	t_ImagesIter ImagesIter = Images.begin();
	while (ImagesIter != Images.end()) {
		pImageDesc = (CImageDescription*) *ImagesIter;
		// concatenate other images to TIM bin
		if (pImageDesc->ImageId().compare(0, 3, "TIM") == 0) {
			ImagesIter++;
			continue;
		}

		fsTimBinFile.seekg(0, ios_base::end); // Set position to EOF
		fsTimBinFile.clear();
		// note that the (unsigned int) cast is dangerous for very large files but should not 
		// be an issue here
		location = (unsigned int) fsTimBinFile.tellg();

		//pad out the current binary until we get to the location of where the next image should start
		unsigned int uiFlashEntryAddr = Translate(
				(char*) pImageDesc->FlashEntryAddress().c_str());
		if (location > uiFlashEntryAddr) {
			printf(
					"  Error: FlashEntryAddress is < location in file, overlapping images?...\n");
			return false;
		}

		printf("\nPadding file to match FlashEntryAddr...\n");
		if (uiFlashEntryAddr > location) {
			for (unsigned int i = 0; i < (uiFlashEntryAddr - location); i++)
				fsTimBinFile.write((char*) &padding, 1);
		}

		//open the next image file
		ifstream ifsImage;
		ifsImage.open(pImageDesc->ImageFilePath().c_str(),
				ios_base::in | ios_base::binary);
		if (ifsImage.bad() || ifsImage.fail()) {
			printf("  Error: Cannot open file name <%s> !\n",
					pImageDesc->ImageFilePath().c_str());
			return false;
		}

		printf("Concatenating file <%s>...\n",
				pImageDesc->ImageFilePath().c_str());
		const int CHUNK_SIZE = 8192 * 8;
		BYTE* pData = new BYTE[CHUNK_SIZE];
		memset(pData, 0, sizeof(*pData));

		//now write out image in 8K chunks
		int iBlockSize = CHUNK_SIZE;
		while (iBlockSize == CHUNK_SIZE) {
			ifsImage.read((char*) pData, iBlockSize);
			// note that the (unsigned int) cast is dangerous for very large files but should not 
			// be an issue here
			iBlockSize = (unsigned int) ifsImage.gcount();
			fsTimBinFile.write((char*) pData, iBlockSize);
		}
		delete[] pData;

		ifsImage.close();
		ImagesIter++;
	}

	return true;
}

bool CImageBuilder::GenerateTIMHash(char *TIMBuffer, long lFileSize,
		unsigned char *pTIMHash, unsigned int TimSizeToHash) {
	// A hash of the NTIM is calculated by this function
	t_ImagesList& Images = TimDescriptorParser.TimDescriptor().ImagesList();
	CImageDescription* pImageDesc = 0;
	t_ImagesIter ImagesIter = Images.begin();
	while (!Images.empty()) {
		pImageDesc = (CImageDescription*) *ImagesIter;
		if (pImageDesc->ImageId() == "TIMH")
			break;
	}

	// determine if hash size is greater than the complete file size and adjust TimSizeToHash
	if ((long) TimSizeToHash > lFileSize)
		TimSizeToHash = lFileSize;

	if (TimSizeToHash > 0) {
		printf("\nTim size to hash read from file: %d\n", TimSizeToHash);

		unsigned char TIMHash[MAX_HASH] = { 0 };
		ofstream ofsTIMHash;
		unsigned int SHAHashSize = 0; //size of digest in words
		string sHashValue;

		memset(TIMHash, 0, sizeof(TIMHash)); //clear the hash buffers

		if (CommandLineParser.bVerbose) {
			stringstream ss;
			string value;
			ofstream ofsTIMmsg;
			string sTIMmsgFilePath = "Tim_msg.txt";
			PrependPathIfNone(sTIMmsgFilePath);
			ofsTIMmsg.open(sTIMmsgFilePath.c_str(),
					ios_base::out | ios_base::trunc);
			for (unsigned int i = 0; i < TimSizeToHash; i++) {
				if (i != 0 && (i % 4 == 0))
					ss << endl;
				ss << hex << setw(2) << setfill('0')
						<< ((int) TIMBuffer[i] & 0x000000ff);
			}
			ofsTIMmsg << ss.str();
			ofsTIMmsg.flush();
			ofsTIMmsg.close();
		}

		//Calculate the SHA hash of the TIM file
		switch (pImageDesc->HashAlgorithmId()) {
		case HASHALGORITHMID_T::SHA160:
			CryptoPP::SHA1().CalculateDigest((byte *) TIMHash,
					(unsigned char *) TIMBuffer, TimSizeToHash);
			SHAHashSize = 5;
			break;
		case HASHALGORITHMID_T::SHA256:
			CryptoPP::SHA256().CalculateDigest((byte *) TIMHash,
					(unsigned char *) TIMBuffer, TimSizeToHash);
			SHAHashSize = 8;
			break;
		case HASHALGORITHMID_T::SHA512:
			CryptoPP::SHA512().CalculateDigest((byte *) TIMHash,
					(unsigned char *) TIMBuffer, TimSizeToHash);
			SHAHashSize = 16;
			break;
		}

		if (CommandLineParser.bVerbose) {
			string sTIMHashTxtPath = "TIMHash.txt";
			PrependPathIfNone(sTIMHashTxtPath);
			ofsTIMHash.open(sTIMHashTxtPath.c_str(),
					ios_base::out | ios_base::trunc);
			if (ofsTIMHash.bad() || ofsTIMHash.fail()) {
				printf("\n  Error: fopen failed opening TIMHash.txt\n");
			}

			printf("\nTIM Hash size (in bits) %u\n", SHAHashSize * 32);

			stringstream ss1;
			for (int i = 0; (unsigned int) i < SHAHashSize; i++) {
				sHashValue = HexFormattedAscii(((unsigned int *) TIMHash)[i]);
				ss1 << sHashValue << endl;
			}

			ss1 << endl;

			printf("%s\n", ss1.str().c_str());

			ofsTIMHash << ss1.str();
			ofsTIMHash.flush();
			ofsTIMHash.close();
		}

		memcpy(pTIMHash, TIMHash, sizeof(TIMHash));
	}

	return true;
}

bool CImageBuilder::VerifyTIMHash(fstream& fsTimBinFile) {
	// algorithm:
	// read the TIM file contents
	// locate the TIM image structure
	// calculate the TIM image hash
	// compare the TIM image hash to the hash structure in the TIM binary
	long lFileSize = 0;
	char *TIMBuffer = NULL;

	string lcTimTxtFilePath = CTimLib::ToLower(
			CommandLineParser.TimTxtFilePath);
	bool bIsBlf = lcTimTxtFilePath.find(".blf")
			== lcTimTxtFilePath.length() - 4;

	if (!LoadTimBinFileToTimBuffer(fsTimBinFile, TIMBuffer, &lFileSize))
		return false;

	if (TIMBuffer == NULL) {
		delete[] TIMBuffer;
		return false;
	}

	unsigned int iPos = 0;
	if (!ParseTimHeader(iPos, TIMBuffer)) {
		printf(
				"////////////////////////////////////////////////////////////\n");
		printf("  Error: VerifyTIMHash() couldn't parse NTIM binary file.\n");
		printf(
				"////////////////////////////////////////////////////////////\n");
		if (TIMBuffer != NULL)
			delete[] TIMBuffer;
		return false;
	}

	CImageDescription TimImage(bIsBlf, TimDescriptor);
	ParseImages(iPos, TIMBuffer, TimImage);

	unsigned char CalcTIMHash[MAX_HASH] = { 0 };

	unsigned int SHAHashSize = 0;
	if (TimImage.ImageSizeToHash() != 0) {
		SHAHashSize =
				(TimImage.HashAlgorithmId() == HASHALGORITHMID_T::SHA512) ? 16 :
				(TimImage.HashAlgorithmId() == HASHALGORITHMID_T::SHA256) ?
						8 : 5;

		//Calculate the SHA hash of the TIM file
		if (TimImage.HashAlgorithmId() == HASHALGORITHMID_T::SHA512)
			CryptoPP::SHA512().CalculateDigest((byte *) CalcTIMHash,
					(unsigned char *) TIMBuffer, TimImage.ImageSizeToHash());
		else if (TimImage.HashAlgorithmId() == HASHALGORITHMID_T::SHA256)
			CryptoPP::SHA256().CalculateDigest((byte *) CalcTIMHash,
					(unsigned char *) TIMBuffer, TimImage.ImageSizeToHash());
		else
			CryptoPP::SHA1().CalculateDigest((byte *) CalcTIMHash,
					(unsigned char *) TIMBuffer, TimImage.ImageSizeToHash());
	}

	for (unsigned int i = 0; i < SHAHashSize; i++) {
		if (TimImage.Hash[i] != *(unsigned int*) &CalcTIMHash[i * 4]) {
			printf(
					"////////////////////////////////////////////////////////////\n");
			printf(
					"  Error: NTIM Hash verification fails in VerifyTIMHash()!\n");
			printf(
					"////////////////////////////////////////////////////////////\n");
			if (TIMBuffer != NULL)
				delete[] TIMBuffer;
			return false;
		}
	}

	printf("\nNTIM Hash verification successful.\n\n");

	if (TIMBuffer != NULL)
		delete TIMBuffer;

	return true;
}

bool CImageBuilder::ParseTimHeader(unsigned int& iPos, char* pTimBuffer) {
	if (pTimBuffer == NULL)
		return false;

	// parse tim header
	// if the version translates as ascii characters, then this is likely an _h.bin rather than a bin
	string sVersion = HexAsciiToText(
			HexFormattedAscii(*(unsigned int*) &pTimBuffer[iPos]));
	if (sVersion.length() == 0)
		return false;

	CCTIM& TIMHeader = TimDescriptor.getTimHeader();
	TIMHeader.setVersion((*(unsigned int*) &pTimBuffer[iPos]));
	iPos += 4;
	if (TIMHeader.getVersion() > TIM_3_2_00
			&& TIMHeader.getVersion() < TIM_3_3_00)
		TIMHeader.setVersion(TIM_3_2_00);
	if (TIMHeader.getVersion() > TIM_3_3_00
			&& TIMHeader.getVersion() < TIM_3_4_00)
		TIMHeader.setVersion(TIM_3_3_00);
	if (TIMHeader.getVersion() > TIM_3_4_00
			&& TIMHeader.getVersion() < TIM_3_5_00)
		TIMHeader.setVersion(TIM_3_4_00);
	if (TIMHeader.getVersion() > TIM_3_5_00
			&& TIMHeader.getVersion() < TIM_3_6_00)
		TIMHeader.setVersion(TIM_3_5_00);
	if (TIMHeader.getVersion() > TIM_3_6_00)
		TIMHeader.setVersion(TIM_3_6_00);

	TIMHeader.setIdentifier((*(unsigned int*) &pTimBuffer[iPos]));
	iPos += 4;

	TIMHeader.setTrusted((*(unsigned int*) &pTimBuffer[iPos]));
	iPos += 4;
	// trusted value must be 0 or 1 or indicates a bad bin file format
	if (!(TIMHeader.getTrusted() == 0
			|| TimDescriptor.getTimHeader().getTrusted() == 1))
		return false;

	TIMHeader.setIssueDate((*(unsigned int*) &pTimBuffer[iPos]));
	iPos += 4;
	TIMHeader.setOEMUniqueId((*(unsigned int*) &pTimBuffer[iPos]));
	iPos += 4;
	if (TIMHeader.getVersion() == TIM_3_6_00) {
		TIMHeader.setStepping((*(unsigned int*) &pTimBuffer[iPos]));
		iPos += 20;
		TIMHeader.setBootRomFlashSignature(
				(*(unsigned int*) &pTimBuffer[iPos]));
		iPos += 4;
	} else {
		TIMHeader.setWtmSaveStateFlashSignature(
				(*(unsigned int*) &pTimBuffer[iPos]));
		iPos += 4;
		TIMHeader.setWtmSaveStateFlashEntryAddress(
				(*(unsigned int*) &pTimBuffer[iPos]));
		iPos += 4;
		TIMHeader.setWtmSaveStateBackupEntryAddress(
				(*(unsigned int*) &pTimBuffer[iPos]));
		iPos += 4;
		TIMHeader.setWtmSaveStatePatchSignature(
				(*(unsigned int*) &pTimBuffer[iPos]));
		iPos += 4;
		TIMHeader.setWtmSaveStatePatchAddress(
				(*(unsigned int*) &pTimBuffer[iPos]));
		iPos += 4;
		TIMHeader.setBootRomFlashSignature(
				(*(unsigned int*) &pTimBuffer[iPos]));
		iPos += 4;
	}
	TIMHeader.setNumImages((*(unsigned int*) &pTimBuffer[iPos]));
	iPos += 4;
	// number of images is suspiciously large and indicates this is not a proper bin file
	if (TimDescriptor.getTimHeader().getNumImages() > MAX_IMAGES)
		return false;

	TIMHeader.setNumKeys((*(unsigned int*) &pTimBuffer[iPos]));
	iPos += 4;
	// number of keys is suspiciously large and indicates this is not a proper bin file
	if (TIMHeader.getNumKeys() > MAX_KEYS)
		return false;

	TIMHeader.setSizeOfReserved((*(unsigned int*) &pTimBuffer[iPos]));
	iPos += 4;
	return true;
}

void CImageBuilder::ParseImages(unsigned int& iPos, char* pTimBuffer,
		CImageDescription& TimImage) {
	string lcTimTxtFilePath = CTimLib::ToLower(
			CommandLineParser.TimTxtFilePath);
	bool bIsBlf = lcTimTxtFilePath.find(".blf")
			== lcTimTxtFilePath.length() - 4;

	if (TimDescriptor.getTimHeader().getVersion() <= TIM_3_3_00) {
		for (unsigned int i = 0;
				i < TimDescriptor.getTimHeader().getNumImages(); i++) {
			CImageDescription* pImage = new CImageDescription(bIsBlf,
					TimDescriptor);

			pImage->ImageIdTag(
					HexFormattedAscii(*(unsigned int*) &pTimBuffer[iPos]));
			iPos += 4;
			pImage->NextImageIdTag(
					HexFormattedAscii(*(unsigned int*) &pTimBuffer[iPos]));
			iPos += 4;
			pImage->FlashEntryAddress(
					HexFormattedAscii(*(unsigned int*) &pTimBuffer[iPos]));
			iPos += 4;
			pImage->LoadAddress(
					HexFormattedAscii(*(unsigned int*) &pTimBuffer[iPos]));
			iPos += 4;
			pImage->ImageSize(*(unsigned int*) &pTimBuffer[iPos]);
			iPos += 4;
			pImage->ImageSizeToHash(*(unsigned int*) &pTimBuffer[iPos]);
			iPos += 4;

			unsigned int HashAlgorithmID = *(unsigned int*) &pTimBuffer[iPos];
			iPos += 4;
			pImage->HashAlgorithmId((HASHALGORITHMID_T) HashAlgorithmID);
			unsigned int SHAHashSize =
					(pImage->HashAlgorithmId() == HASHALGORITHMID_T::SHA512) ?
							16 :
					(pImage->HashAlgorithmId() == HASHALGORITHMID_T::SHA256) ?
							8 : 5;
			// remember location of image hash
			int iTimHashPos = iPos;
			for (unsigned int j = 0; j < SHAHashSize; j++) {
				pImage->Hash[j] = *(unsigned int*) &pTimBuffer[iPos];
				iPos += 4;
			}

			// skip unused bytes in the hash array in the TIM bin file
			iPos += (8 * 4) - (SHAHashSize * 4);

			if (TimDescriptor.getTimHeader().getVersion() >= TIM_3_2_00) {
				pImage->PartitionNumber(*(unsigned int*) &pTimBuffer[iPos]);
				iPos += 4;
			}

			// after saving the tim hash in the image object, clear the timhash in the timbuffer
			// so we can calculate a new hash on the first image which must be a TIM*
			if (i == 0 && pImage->ImageId().compare(0, 3, "TIM") == 0) {
				TimImage = *pImage;
				memset(&pTimBuffer[iTimHashPos], 0, SHAHashSize * 4);
			}

			TimDescriptor.ImagesList().push_back(pImage);
		}
	} else if (TimDescriptor.getTimHeader().getVersion() == TIM_3_4_00) {
		for (unsigned int i = 0;
				i < TimDescriptor.getTimHeader().getNumImages(); i++) {
			CImageDescription* pImage = new CImageDescription(bIsBlf,
					TimDescriptor);

			pImage->ImageIdTag(
					HexFormattedAscii(*(unsigned int*) &pTimBuffer[iPos]));
			iPos += 4;
			pImage->NextImageIdTag(
					HexFormattedAscii(*(unsigned int*) &pTimBuffer[iPos]));
			iPos += 4;
			pImage->FlashEntryAddress(
					HexFormattedAscii(*(unsigned int*) &pTimBuffer[iPos]));
			iPos += 4;
			pImage->LoadAddress(
					HexFormattedAscii(*(unsigned int*) &pTimBuffer[iPos]));
			iPos += 4;
			pImage->ImageSize(*(unsigned int*) &pTimBuffer[iPos]);
			iPos += 4;
			pImage->ImageSizeToHash(*(unsigned int*) &pTimBuffer[iPos]);
			iPos += 4;

			unsigned int HashAlgorithmID = *(unsigned int*) &pTimBuffer[iPos];
			iPos += 4;
			pImage->HashAlgorithmId((HASHALGORITHMID_T) HashAlgorithmID);
			unsigned int SHAHashSize =
					(pImage->HashAlgorithmId() == HASHALGORITHMID_T::SHA512) ?
							16 :
					(pImage->HashAlgorithmId() == HASHALGORITHMID_T::SHA256) ?
							8 : 5;

			// remember location of image hash
			int iTimHashPos = iPos;
			for (unsigned int j = 0; j < SHAHashSize; j++) {
				pImage->Hash[j] = *(unsigned int*) &pTimBuffer[iPos];
				iPos += 4;
			}

			// skip unused bytes in the hash array in the TIM bin file
			iPos += ((sizeof(pImage->Hash) / 4) - SHAHashSize) * 4;

			pImage->PartitionNumber(*(unsigned int*) &pTimBuffer[iPos]);
			iPos += 4;

			// after saving the tim hash in the image object, clear the timhash in the timbuffer
			// so we can calculate a new hash on the first image which must be a TIM*
			if (i == 0 && pImage->ImageId().compare(0, 3, "TIM") == 0) {
				TimImage = *pImage;
				memset(&pTimBuffer[iTimHashPos], 0, SHAHashSize * 4);
			}

			TimDescriptor.ImagesList().push_back(pImage);
		}
	} else if (TimDescriptor.getTimHeader().getVersion() >= TIM_3_5_00) {
		for (unsigned int i = 0;
				i < TimDescriptor.getTimHeader().getNumImages(); i++) {
			CImageDescription* pImage = new CImageDescription(bIsBlf,
					TimDescriptor);

			pImage->ImageIdTag(
					HexFormattedAscii(*(unsigned int*) &pTimBuffer[iPos]));
			iPos += 4;
			pImage->NextImageIdTag(
					HexFormattedAscii(*(unsigned int*) &pTimBuffer[iPos]));
			iPos += 4;
			pImage->FlashEntryAddress(
					HexFormattedAscii(*(unsigned int*) &pTimBuffer[iPos]));
			iPos += 4;
			pImage->LoadAddress(
					HexFormattedAscii(*(unsigned int*) &pTimBuffer[iPos]));
			iPos += 4;
			pImage->ImageSize(*(unsigned int*) &pTimBuffer[iPos]);
			iPos += 4;
			pImage->ImageSizeToHash(*(unsigned int*) &pTimBuffer[iPos]);
			iPos += 4;

			unsigned int HashAlgorithmID = *(unsigned int*) &pTimBuffer[iPos];
			iPos += 4;
			pImage->HashAlgorithmId((HASHALGORITHMID_T) HashAlgorithmID);
			unsigned int SHAHashSize =
					(pImage->HashAlgorithmId() == HASHALGORITHMID_T::SHA512) ?
							16 :
					(pImage->HashAlgorithmId() == HASHALGORITHMID_T::SHA256) ?
							8 : 5;

			// remember location of image hash
			int iTimHashPos = iPos;
			for (unsigned int j = 0; j < SHAHashSize; j++) {
				pImage->Hash[j] = *(unsigned int*) &pTimBuffer[iPos];
				iPos += 4;
			}

			// skip unused bytes in the hash array in the TIM bin file
			iPos += ((sizeof(pImage->Hash) / 4) - SHAHashSize) * 4;

			pImage->PartitionNumber(*(unsigned int*) &pTimBuffer[iPos]);
			iPos += 4;

			unsigned int EncryptAlgorithmID = *(unsigned int*) &pTimBuffer[iPos];
			iPos += 4;
			pImage->EncryptAlgorithmId(
					(ENCRYPTALGORITHMID_T) EncryptAlgorithmID);
			pImage->EncryptStartOffset(*(unsigned int*) &pTimBuffer[iPos]);
			iPos += 4;
			pImage->EncryptSize(*(unsigned int*) &pTimBuffer[iPos]);
			iPos += 4;

			// after saving the tim hash in the image object, clear the timhash in the timbuffer
			// so we can calculate a new hash on the first image which must be a TIM*
			if (i == 0 && pImage->ImageId().compare(0, 3, "TIM") == 0) {
				TimImage = *pImage;
				memset(&pTimBuffer[iTimHashPos], 0, SHAHashSize * 4);
			}

			TimDescriptor.ImagesList().push_back(pImage);
		}
	}
}
