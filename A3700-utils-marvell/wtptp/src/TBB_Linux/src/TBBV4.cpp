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

// TBBV4.cpp : Defines the entry point for the console application.
#include "CommandLineParser.h"
#include "TimDescriptorParser.h"
#include "ImageBuilder.h"
#include "TrustedImageBuilder.h"
#include "TrustedTimDescriptorParser.h"
#include "KeyGenSpec.h"
#include "PartitionTable.h"

#include <unistd.h>
#include <time.h>

char TBBVersion[] = { "3.3.12.1" };
char Date[] = { "2017-03-17" };

bool verifyTimBinFile(char *path) {
	fstream timFile;
	bool retval = true;
	timFile.open(path, ios_base::in | ios_base::binary);
	if (timFile.bad() || timFile.fail()) {
		printf("\n Cannot open binary file %s", path);
		return false;
	}

	timFile.seekg(0, ios_base::end);  // Set position to EOF
	timFile.clear();
	unsigned int size = (long) timFile.tellg();
	char *pTIMBuffer = new char[size];
	if (0 == pTIMBuffer) {
		printf(
				"  Error: Memory allocation for TimBuffer failed in LoadTimBinFileToTimBuffer()!\n");
		return false;
	}

	timFile.seekg(0, ios_base::beg);  // Set position SOF

	timFile.read(pTIMBuffer, size);
	if (timFile.bad() || timFile.fail()) {
		printf(
				"  Error: LoadTimBinFileToTimBuffer() couldn't read all of the %d bytes\n",
				size);
		delete[] pTIMBuffer;
		return false;
	}

	pPLAT_DS ds = (pPLAT_DS) (pTIMBuffer + size - sizeof(PLAT_DS));

	CDigitalSignature DigS;
	DigS.FromPlatDs(*ds);
	if ((ds->DSAlgorithmID == ECDSA_256) || (ds->DSAlgorithmID == ECDSA_521)) {
		retval = CTrustedImageBuilder::verifyMessageUsingCryptoPPECDSA(
				(unsigned char *) pTIMBuffer,
				size
						- (sizeof(UINT_T)
								* ((MAXRSAKEYSIZEWORDS * 3)
										- (MAXECCKEYSIZEWORDS * 2))), DigS);
	} else if ((ds->DSAlgorithmID == PKCS_v2_2_Ippcp)
			|| (ds->DSAlgorithmID == PKCS1_v1_5_Ippcp)) {
		retval = CTrustedImageBuilder::verifyMessageUsingCryptoPPRSA(
				(unsigned char *) pTIMBuffer,
				size - (sizeof(UINT_T) * (MAXRSAKEYSIZEWORDS)), DigS);
	} else
		retval = false;

	return retval;

}

bool handleBUseKeyFile(CCommandLineParser &CommandLineParser,
		CTrustedTimDescriptorParser &TimDescriptorParser,
		bool ignorePrivateKey) {
	bool bRet;
	CDigitalSignature DSig;
	if (CommandLineParser.KeyFilePath.substr(
			CommandLineParser.KeyFilePath.length() - 4) == ".bin")
		bRet = TimDescriptorParser.ParseBinaryKeyFile(
				CommandLineParser.KeyFilePath, CommandLineParser, DSig.DSKey());
	else
		bRet = TimDescriptorParser.ParseKeyFile(CommandLineParser,
				CommandLineParser.KeyFilePath, DSig.DSKey(), ignorePrivateKey);
	if (bRet) {
		DSig.KeySize(DSig.DSKey()->KeySize());
		TimDescriptorParser.TimDescriptor().DigitalSignature() = DSig;
	}
	return bRet;
}

bool handleGenerateKeyFile(CCommandLineParser &CommandLineParser,
		CTrustedImageBuilder &ImageBuilder) {
	bool bRet = true;
	if (CommandLineParser.bGenerateKeySpecFile) {
		CKeyGenSpec& KeyGenSpec = ImageBuilder.KeyGenSpec();
		KeyGenSpec.KeyGenSpecFilename(CommandLineParser.KeyGenSpecFilePath);
		bRet = KeyGenSpec.ParseKeyGenSpecFile();
		if (bRet) {
			bRet = ImageBuilder.GenerateKey(CommandLineParser, KeyGenSpec);
		}
	}
	return bRet;
}

bool handleTimTextFile(CTrustedTimDescriptorParser &TimDescriptorParser,
		CCommandLineParser &CommandLineParser,
		CTrustedImageBuilder &ImageBuilder) {
	bool bRet = true;
	// Parse the TIM descriptor
	if (CommandLineParser.bIsTimTxtFile || CommandLineParser.bIsBlfTxtFile) {
		bRet = TimDescriptorParser.GetTimDescriptorLines(CommandLineParser);
		if (bRet) {
			bRet = TimDescriptorParser.ParseDescriptor(CommandLineParser);
		}

		// Inject PIN ????
		if (bRet && CommandLineParser.bPinFile) {
			if (TimDescriptorParser.TimDescriptor().getTimHeader().getTrusted()) {
				bRet = TimDescriptorParser.TimDescriptor().InjectPin(
						CommandLineParser.PinFilePath);
			} else {
				printf(
						"\nFailure: Cannot Inject a PIN in a non-trusted build.\n");
				bRet = false;
			}
		}

		// Check Use Generated Key file for DS
		if (bRet
				&& TimDescriptorParser.TimDescriptor().getTimHeader().getTrusted()) {
			if (CommandLineParser.bUseKeyFile) {
				bRet = handleBUseKeyFile(CommandLineParser, TimDescriptorParser,
						CommandLineParser.iOption == 2 ? true : false);
			}
		}

		/// Build Descriptor...
		if (bRet) {
			bRet = ImageBuilder.BuildDescriptorFile();
		}
	}
	return bRet;
}

bool handlePartitionFile(CTrustedTimDescriptorParser &TimDescriptorParser,
		CCommandLineParser &CommandLineParser,
		CTrustedImageBuilder &ImageBuilder) {
	bool bRet = true;
	if (CommandLineParser.bIsPartitionDataFile
			&& (!CommandLineParser.bIsTimTxtFile
					|| TimDescriptorParser.TimDescriptor().getTimHeader().getVersion()
							>= TIM_3_2_00)) {
		string sPartitionPath = CommandLineParser.PartitionFilePath;
		TimDescriptorParser.PrependPathIfNone(sPartitionPath);

		CPartitionTable PartitionTable;
		PartitionTable.PartitionFilePath(sPartitionPath);

		if (bRet) {
			bRet = PartitionTable.ParsePartition();
		}

		if (bRet) {
			string sPartitionBinPath = "partition.bin";
			TimDescriptorParser.PrependPathIfNone(sPartitionBinPath);
			bRet = PartitionTable.GeneratePartitionBinary(sPartitionBinPath);
		}
	}
	return bRet;
}
int main(int argc, char *argv[]) {
	CCommandLineParser CommandLineParser;
	CTrustedTimDescriptorParser TimDescriptorParser(CommandLineParser);
	CTrustedImageBuilder ImageBuilder(CommandLineParser, TimDescriptorParser);
	bool bRet = true;

	printf("\n\nTBB Version		: %s\n", TBBVersion);
	printf("TBB Date		: %s\n\n", Date);

	// Display operating system-style date and time.
	char tmpbuf[100];
	time_t starttime = time(NULL);
	struct tm st = { 0 };
	localtime(&starttime);
	memcpy(&st, localtime(&starttime), sizeof(st));
	strftime(tmpbuf, 100, "%m/%d/%y %H:%M:%S", &st);
	printf("Start time: %s\n", tmpbuf);

	// Check for minimum number of arguments....
	if (argc < MIN_ARGS) {
		CommandLineParser.PrintUsage();
		fflush(stdout);
		return false;
	}

	// Check if only Tim verification is required..
	if (strcmp(argv[1], "--VerifyOnly") == 0) {
		bRet = verifyTimBinFile(argv[2]);
		if (!bRet)
			printf("TIM file invalid...\n");
		else
			printf("TIM file valid...\n");

		return bRet;
	}

	fflush(stdout);
	if (CommandLineParser.ParseCommandLine(argc, argv) == FALSE)
		return FALSE;
	fflush(stdout);

	CTimLib TimLib;
	TimLib.PrependPathIfNone(CommandLineParser.TimTxtFilePath);
	TimDescriptorParser.TimDescriptor().TimDescriptorFilePath(
			CommandLineParser.TimTxtFilePath);

	int iOption = CommandLineParser.iOption;

	//Generate Key files if required...
	bRet = handleGenerateKeyFile(CommandLineParser, ImageBuilder);
	if (!bRet) {
		printf("Error!! Key generation failed... \n");
	}

	// Handle if m=1 or m=2
	if ((iOption == 1) || (iOption == 2)) {
		bRet = handleTimTextFile(TimDescriptorParser, CommandLineParser,
				ImageBuilder);
		if (!bRet) {
			printf("Error!! Tim Text File parser failed!!!! \n");
		}
		bRet = handlePartitionFile(TimDescriptorParser, CommandLineParser,
				ImageBuilder);
		if (!bRet) {
			printf("Error!! Tim Partition File failed!!!! \n");
		}
	} // End iOption 1 and 2...

	else if (iOption == 3) {
		if (TimDescriptorParser.ParseKeyFile(CommandLineParser,
				CommandLineParser.TimTxtFilePath,
				TimDescriptorParser.TimDescriptor().DigitalSignature().DSKey())) {
			bRet = ImageBuilder.BuildDigitalSignature();
			if (!bRet) {
				printf("\n\nTBB Failed to build Digital Signature!\n");
			} else {
				printf("\n\nTBB Succeeded building Digital Signature!\n");
			}
		}
	} else if (iOption == 4) {
		bRet = ImageBuilder.AddDigitalSignatureToTim();
		if (!bRet) {
			printf(
					"\n\nTBB Failed to Add Digital Signature to TIM binary file!\n");
		} else {
			printf("\n\nTBB Added Digital Signature to TIM binary file!\n");
		}
	} else {
		printf("\nError: Invalid Option Mode Selected!");
	}

	fflush(stdout);

	time_t endtime = time(NULL);
	struct tm et = { 0 };
	localtime(&endtime);
	memcpy(&et, localtime(&endtime), sizeof(et));
	strftime(tmpbuf, 100, "%m/%d/%y %H:%M:%S", &et);
	printf("\nFinish time: %s\n", tmpbuf);
	printf("\nTBB Exiting...!\n");

	return bRet ? 0 : 1; // return 0 on success
}
