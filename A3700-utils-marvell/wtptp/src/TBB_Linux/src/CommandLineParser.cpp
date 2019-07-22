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
#include <cstdlib>

CCommandLineParser::CCommandLineParser(void) {
	Reset();
}

CCommandLineParser::~CCommandLineParser(void) {
}

void CCommandLineParser::Reset() {
	bIsTimTxtFile = false;
	bIsBlfTxtFile = false;
	bIsHashFile = false;
	bIsDsFile = false;
	bIsTimInFile = false;
	bIsTimOutFile = false;
	bIsNonTrusted = true;
	bIsTimVerify = false;
	bIsPartitionDataFile = false;
	iOption = 1;
	bVerbose = false;
	bConcatenate = false;
	bOneNANDPadding = false;
	iProcessorType = UNDEFINED;
	uiPaddedSize = 0;
	bGenerateKeySpecFile = false;
	bUseKeyFile = false;
	bPinFile = false;
	bTimTxtRelative = false;
	bKeyByteSwap = false;
	bOtpHashPadding = true;

	uiImageModSize = 4;
	bImageLengthPadding = false;
}

bool CCommandLineParser::ParseCommandLine(int argc, char *argv[]) {
	int currentOption = 1;
	BYTE Option = 0;

	printf("\nCommandLineOptions:\n");

	while (currentOption < argc) {
		if ((*argv[currentOption] & 0x7f) == '-') {
			// (0x7f) filters 8-bit ascii to 7-bit
			// email/word sometimes automatically converts short - into a long -
			// when cutting/pasting commandlines and this can cause a bad parse
			Option = *++argv[currentOption++] & 0x7f;
			printf("-%c ", Option);
			switch (Option) {
			case OPTION:
				iOption = atoi(argv[currentOption]);
				if (iOption > 4 || iOption < 1) {
					iOption = 1;
					printf(
							"\n  Error: Parameter Invalid or Missing: -%c switch: ",
							(char) OPTION);
					printf("Defaulting to Option Mode 1...\n");
				} else {
					printf("%i ", iOption);
				}
				break;

			case DESCRIPTOR_TXT: {
				if (currentOption == argc) {
					printf(
							"\n  Error:   Command Line Parsing Error: -%c switch: ",
							(char) DESCRIPTOR_TXT);
					printf(
							" Missing TIM Descriptor.txt file name parameter.\n");
					return false;
				}
				TimTxtFilePath = argv[currentOption];
				string lcTimTxtFilePath = CTimLib::ToLower(TimTxtFilePath);
				// see if last 4 chars == ".blf", else assume a TIM.txt file
				if (lcTimTxtFilePath.find(".blf")
						== lcTimTxtFilePath.length() - 4)
					bIsBlfTxtFile = true;
				else
					bIsTimTxtFile = true;
				printf("%s ", TimTxtFilePath.c_str());
				break;
			}

			case HASH:
				if (currentOption == argc) {
					printf(
							"\n  Error:   Command Line Parsing Error: -%c switch: ",
							(char) HASH);
					printf(" Missing hash file name parameter.\n");
					return false;
				}
				HashFilePath = argv[currentOption];
				;
				bIsHashFile = true;
				printf("%s ", HashFilePath.c_str());
				break;

			case DS:
				if (currentOption == argc) {
					printf(
							"\n  Error:   Command Line Parsing Error: -%c switch: ",
							(char) DS);
					printf(" Missing hash file name parameter.\n");
					return false;
				}
				DsFilePath = argv[currentOption];
				;
				bIsDsFile = true;
				bIsNonTrusted = false;
				printf("%s ", DsFilePath.c_str());
				break;

			case TIMIN:
				if (currentOption == argc) {
					printf(
							"\n  Error:   Command Line Parsing Error: -%c switch: ",
							(char) TIMIN);
					printf(" Missing input file name parameter.\n");
					return false;
				}
				TimInFilePath = argv[currentOption];
				;
				bIsTimInFile = true;
				printf("%s ", TimInFilePath.c_str());
				break;

			case TIMOUT:
				if (currentOption == argc) {
					printf(
							"\n  Error:   Command Line Parsing Error: -%c switch: ",
							(char) TIMOUT);
					printf(" Missing output file name parameter.\n");
					return false;
				}
				TimOutFilePath = argv[currentOption];
				;
				bIsTimOutFile = true;
				printf("%s ", TimOutFilePath.c_str());
				break;

			case VERBOSE:
				bVerbose = true;
				break;

			case CONCATENATE:
				bConcatenate = true;
				break;

			case TIMVERIFY:
				bIsTimVerify = true;
				break;

			case PARTITION:
				if (currentOption == argc) {
					printf(
							"\n  Error:   Command Line Parsing Error: -%c switch: ",
							(char) PARTITION);
					printf(" Missing partition data file name parameter.\n");
					return FALSE;
				}
				PartitionFilePath = argv[currentOption];
				;
				bIsPartitionDataFile = true;
				printf("%s ", PartitionFilePath.c_str());
				break;

			case PROCESSOR_TYPE:
				iProcessorType = (eProcessorType) atoi(argv[currentOption]);
				if (iProcessorType <= PXA_NONE || iProcessorType > PXAMAX_PT) {
					printf(
							"\n  Error:   Command Line Parsing Error: -%c switch: ",
							(char) PROCESSOR_TYPE);
					printf(" Missing or invalid processor type parameter.\n");
					return false;
				}
				printf("\nProcessorType %i\n", iProcessorType);
				break;

			case PADDED_TIM_IMAGE:
				uiPaddedSize = atoi(argv[currentOption]);
				// normalize padded size to a mod 256 value
				uiPaddedSize = ((uiPaddedSize + 255) / 256) * 256;
				if (uiPaddedSize > 8192) {
					printf(
							"\n  Error:   Command Line Parsing Error: -%c switch: ",
							(char) PADDED_TIM_IMAGE);
					printf(
							" Missing or invalid padded TIM image size parameter. Value must not be between 0-8192.\n");
					return false;
				}
				bOneNANDPadding = true;
				printf("%u ", uiPaddedSize);
				break;

			case GENERATE_KEY:
				if (currentOption == argc) {
					printf(
							"\n  Error:   Command Line Parsing Error: -%c switch: ",
							(char) GENERATE_KEY);
					printf(" Missing <KeyGenSpec.txt> file name parameter.\n");
					return FALSE;
				}
				KeyGenSpecFilePath = argv[currentOption];
				;
				bGenerateKeySpecFile = true;
				printf("%s ", KeyGenSpecFilePath.c_str());
				break;

			case USE_KEY:
				if (currentOption == argc) {
					printf(
							"\n  Error:   Command Line Parsing Error: -%c switch: ",
							(char) USE_KEY);
					printf(
							" Missing <Key.bin(or Key.txt)> file name parameter.\n");
					return FALSE;
				}
				KeyFilePath = argv[currentOption];
				;
				bUseKeyFile = true;
				bIsNonTrusted = false;
				printf("%s ", KeyFilePath.c_str());
				break;

			case INJECT_PIN:
				if (currentOption == argc) {
					printf(
							"\n  Error:   Command Line Parsing Error: -%c switch: ",
							(char) INJECT_PIN);
					printf(" Missing input <PIN.txt> file name parameter.\n");
					return FALSE;
				}
				PinFilePath = argv[currentOption];
				;
				bPinFile = true;
				printf("%s ", PinFilePath.c_str());
				break;

			case TIM_TXT_RELATIVE:
				bTimTxtRelative = true;
				break;

			case KEYBYTESWAP:
				bKeyByteSwap = true;
				bIsNonTrusted = false;
				break;

			case OPT_HASH_PADDING:
				bOtpHashPadding = true;
				break;

			case PAD_IMAGE_MOD_N_LENGTH:
				uiImageModSize = atoi(argv[currentOption]);
				// normalize padded size to a mod 4 value
				uiImageModSize = ((uiImageModSize + 3) / 4) * 4;
				bImageLengthPadding = true;
				break;

			case HELP:
				PrintUsage();
				break;

			default:
				printf("\n  Error: Command line option -%c is invalid!\n",
						Option);
				return false;
			}
		} else {
			currentOption++;
		}
	}
	printf("\n");

	if (CheckCommandLineOptions() == false)
		return false;

	return true;
}

bool CCommandLineParser::CheckCommandLineOptions() {
	// Check for TIM verify functionality for trusted operation.
	if ((iOption != 1) && (bIsNonTrusted && bIsTimVerify)) {
		printf(
				"////////////////////////////////////////////////////////////\n");
		printf("  Error: Invalid Options\n");
		printf(
				"Missing switch, non-trusted descriptor file or incorrect option mode.\n");
		printf("Example: -m 1 -r timdescriptorfile.txt -V\n");
		printf(
				"////////////////////////////////////////////////////////////\n");
		return false;
	}

	switch (iOption) {
	case 1:
		if (!(bIsTimTxtFile || bIsBlfTxtFile) && !bIsPartitionDataFile && !bGenerateKeySpecFile)
		{
			printf(
					"////////////////////////////////////////////////////////////\n");
			printf(
					"  Error: Invalid Options for -m 1, need -r, -P, or -G with -m 1.\n");
			printf(
					"Missing TIM descriptor file name parameter. (-r filename.txt)\n");
			printf("Example: -m 1 -r timdescriptorfile.txt\n");
			printf(
					"Missing output Partition.txt file name parameter. (-P filename.txt)\n");
			printf("Example: -m 1 -P Partition.txt\n");

			printf(
					"Missing input Key Generate file name parameter. (-G filename.txt)\n");
			printf("Example: -m 1 -G KeyGenSpec.txt\n");
			printf(
					"Missing output EncryptKey file name parameter. (-E filename.bin)\n");
			printf(
					"Example: -m 1 -r timdescriptorfile.txt -E EncryptKey.bin\n");
			printf(
					"////////////////////////////////////////////////////////////\n");
			return false;
		}
		break;

	case 2:
		if (!((bIsTimTxtFile || bIsBlfTxtFile) && bIsHashFile)) {
			printf(
					"////////////////////////////////////////////////////////////\n");
			printf("  Error: Invalid Options\n");
			printf(
					"Missing TIM descriptor file name parameter. (-r filename.txt) OR\n");
			printf(
					"Missing output hash key file name parameter. (-h filename.bin) OR\n");
			printf("Example: -m 2 -r timdescriptorfile.txt -h TIMHash.bin\n");
			printf(
					"////////////////////////////////////////////////////////////\n");
			return false;
		}
		break;

	case 3:
		if (!((bIsTimTxtFile || bIsBlfTxtFile) && bIsHashFile && bIsDsFile)) {
			printf(
					"////////////////////////////////////////////////////////////\n");
			printf("  Error: Invalid Options\n");
			printf(
					"Missing private key file name parameter. (-r filename.txt) OR\n");
			printf(
					"Missing input hash key file name parameter. (-h filename.bin) OR\n");
			printf(
					"Missing output digital signature file name parameter. (-s filename.bin)\n");
			printf(
					"////////////////////////////////////////////////////////////\n");
			return false;
		}
		break;

	case 4:
		if (!(bIsTimInFile && bIsTimOutFile && bIsDsFile)) {
			printf(
					"////////////////////////////////////////////////////////////\n");
			printf("  Error: Invalid Options Error:\n");
			printf(
					"Missing input TIM file name parameter. (-i filename.bin) OR\n");
			printf(
					"Missing output TIM file name parameter. (-o filename.bin) OR\n");
			printf(
					"Missing input digital signature file name parameter. (-s filename.bin)\n");
			printf(
					"////////////////////////////////////////////////////////////\n");
			return false;
		}
		break;

	default:
		printf(
				"////////////////////////////////////////////////////////////\n");
		printf("  Error: Invalid Options Error:\n");
		printf("You entered an incorrect value for the -m option or\n");
		printf("Example: tbb -m 1 -r TimDescriptorFile.txt\n");
		printf("Note: If you leave out the -m option the program will\n");
		printf("      default this option to 1.\n");
		printf(
				"////////////////////////////////////////////////////////////\n");
		return false;
	}

	return true;
}

void CCommandLineParser::PrintUsage() {
	printf("\n WTP Trusted Image Module Builder\n\n");

	printf("  Usage:\n");
	printf("  ======\n");
	printf("  >> TBB.exe <Option>|<Option file> ...\n\n");
	printf("  Options:\n");
	printf("  ========\n");
	printf("  -v\t\tVerbose mode.  Debugging information will be printed.\n");

	printf(
			"  -T\t\t[<PXA168=0><PXA30x=1><PXA31x=2><PXA32x=3><ARMADA610=4><PXA91x=5><PXA92x=6>\n");
	printf(
			"    \t\t <PXA93x=7><PXA94x=8><PXA95x=9><ARMADA16x=10><reserved1=11>\n");
	printf(
			"    \t\t <PXA955=12><PXA968=13><PXA1701=14><PXA978=15><PXA2128=16><reserved2=17>\n");
	printf(
			"    \t\t <ARMADA622=18><PXA1202=19><PXA1801=20><88PA62_70=21><PXA988=22><PXA1920=23>\n");
	printf(
			"    \t\t <PXA2101=24><PXA192=25><PXA1928=26><PXA1986=27><PXA1802=28><PXA986=29>\n");
	printf(
			"    \t\t <PXA1206=30><PXA888=31><PXA1088=32><PXA1812=33><PXA1822=34><PXA1U88=35><PXA1936=36>\n");
	printf(
			"    \t\t <PXA1908 = 37><PXA1826 = 38><88PA62_20 = 39><88PA62_10 = 40><PXA1956 = 41><VEGA = 42><BOWIE = 43><ULC2 = 44><undefined = 45>]\n");

	printf(
			"  -P <Partition.txt>\t\tPartition.txt is the partition table data text file to process. Outputs Partition.bin\n");
	printf("  -r <TIM.txt file path>\tTIM.txt file to process.\n\n");
	printf(
			"  -m <1,2,3,4>\tBuild a trusted or non trusted image module:\n\tTBB -m 1 -r sample_tim.txt\n\n");
	printf(
			"    \t\t1 - Input: TIM.txt or TIM.blf,  Output: (N)TIM or TIMwithDS.bin\n");
	printf(
			"    \t\t2 - Input: TIM.txt or TIM.blf, with optional KeyFile.txt, Output: TIMNoDs.bin and TIMHash.bin\n");
	printf(
			"    \t\t3 - Input: TIMHash.bin and PrivateKey.txt, Output: DS.bin\n");
	printf(
			"    \t\t4 - Input: DS.bin and TIMNoDs.bin Build,  Output: TIMWithDS.bin\n");

	printf("  -i <input TIM.bin file path>\tTIM.bin file to read.\n");
	printf("  -o <output TIM.bin file path>\tTIM.bin file to write.\n");
	printf("  -h <input TIMHash.bin file path>\tTIMHash.bin file to read.\n");
	printf(
			"  -s <input DS.bin file path>\tDigitalSignature.bin file to read.\n");
	printf(
			"  -a \t\tByte swap the DigitalSignature.bin file indicated by the -s.\n");

	printf("  -D \t\tEnable padding in OtpHash calculation.\n");
	printf(
			"  -G <input KeyGenerateSpec file path>\tKeyGenerateSpec.txt file to read.\n");
	printf(
			"  -B <input Key.bin or Key.txt file path>\tKey.bin or Key.txt file to read.\n");
	printf(
			"  -O <n>\tONEnand TIM padding (0xff) in binary. Use values for <n> in the range 0-8192.\n");
	printf("  -V \t\tTIM Verify Digital Signature:\n\n");

	printf(
			"  -L <mod <n> size>\tPad Image length to a <mod <n> size> where <n> is divisible evenly by 4.\n");
	printf(
			"  -N <input PIN.txt>\tInject 64-bit PIN from PIN.txt into TIM.bin\n");
	printf(
			"  -U\t\tRelative paths are relative to TIM.txt path. Default is relative to current working directory.\n");

	printf("\n");

	printf(" ===========EXAMPLES======================\n");
	printf("  -m 1\tBuild a trusted or non trusted image module:\n");
	printf("  \t\t>>TBBv4.exe -m 1 -r TIM.txt -V\n\n");
	printf(
			"  -m 2\tBuild an image module without digital signature using the TIM.txt and\n");

	printf("  output a TIMNoDs.bin and hash bin of that image.\n");
	printf("  \t\t>>TBBv4 -m 2 -r TIM.txt -h TIMHash.bin\n");
	printf("  or:\n");
	printf(
			"  -m 2\tBuild an image module without digital signature using the TIM.txt and\n");
	printf(
			"  the KeyFile and output a TIMNoDs.bin and TIM hash bin of that image.\n");
	printf(
			"  \t\t>>TBBv4 -m 2 -r TIM.txt �B [KeyFile.txt or KeyFile.bin] -h TIMHash.bin\n\n");
	printf(
			"  -m 3\tBuild a digital signature from TIMhash bin and private key txt\n");
	printf("  \t\t>>TBBv4 -m 3 -r privatekey.txt -h TIMHash.bin -s ds.bin\n\n");
	printf(
			"  -m 4\tBuild a trusted image module from digital signature bin and TIM bin module\n");
	printf("  \t\t>>TBBv4 -m 4 -s ds.bin -i TIMNoDs.bin -o TIMWithDs.bin\n\n");
	printf("  or:\n");
	printf(
			"  -m 4\tBuild a trusted image module from digital signature bin, byte swapped, and TIM bin module\n");
	printf(
			"  \t\t>>TBBv4 -m 4 -s ds.bin -a -i TIMNoDs.bin -o TIMWithDs.bin\n\n");
	printf(" Processing partitioning information:\n");
	printf(" =========================================\n");
	printf(" >>TBBv4.exe -r NTIM.txt -P Partition.txt\n");
}
