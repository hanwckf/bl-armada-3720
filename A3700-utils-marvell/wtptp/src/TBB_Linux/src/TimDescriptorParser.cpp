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
#include "TimDescriptorParser.h"
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
#include "GenPurposePatch.h"
#include "GPPOperations.h"
#include "ImageMapERD.h"
#include "CoreReleaseERD.h"
#include "EscapeSeqV2ERD.h"
#include "OnDemandBootERD.h"
#include "DDRTypeERD.h"
#include "FlashGeometryReqERD.h"

#include <assert.h>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <libgen.h>

CTimDescriptorLine* CTimDescriptorParser::pPrevLine = 0;

CTimDescriptorParser::CTimDescriptorParser(
		CCommandLineParser& CommandLineParser) :
		m_CommandLineParser(CommandLineParser) {
	Reset();
}

CTimDescriptorParser::~CTimDescriptorParser(void) {
	Reset();
}

void CTimDescriptorParser::Reset() {
	m_sTimFilePath = "";
	m_sTimBinFilename = "";
	m_sImageOutFilename = "";
}

ifstream& CTimDescriptorParser::OpenTimDescriptorTextFile(
		ios_base::openmode mode) {
	// if already open, close it and reopen in correct mode
	CloseTimDescriptorTextFile();

	m_ifsTimDescriptorTxtFile.open(m_sTimFilePath.c_str(), mode);
	if (m_ifsTimDescriptorTxtFile.bad() || m_ifsTimDescriptorTxtFile.fail()) {
		printf("\n  Error: Cannot open file name <%s> !",
				m_sTimFilePath.c_str());
	}
	return m_ifsTimDescriptorTxtFile;
}

void CTimDescriptorParser::CloseTimDescriptorTextFile() {
	if (m_ifsTimDescriptorTxtFile.is_open())
		m_ifsTimDescriptorTxtFile.close();
	m_ifsTimDescriptorTxtFile.clear();
}

bool CTimDescriptorParser::GetTimDescriptorLinesFromIncludeFile(
		string& sIncludeFilePath, CTimDescriptorLine*& pIncludeLine) {
	string lcTimIncludeFilePath = CTimLib::ToLower(sIncludeFilePath);
	bool bIsBlf = lcTimIncludeFilePath.find(".blf")
			== lcTimIncludeFilePath.length() - 4;

	bool bIsPem = lcTimIncludeFilePath.find(".pem")
			== lcTimIncludeFilePath.length() - 4;

	lcTimIncludeFilePath = CTimLib::MakePathAbsoluteOrRelativeToTIMTxtPath(
			lcTimIncludeFilePath, m_TimDescriptor.TimDescriptorFilePath(),

			m_CommandLineParser.bTimTxtRelative);

	ifstream ifsInclude;

	ifsInclude.open(lcTimIncludeFilePath.c_str(), ios_base::in);
	if (ifsInclude.bad() || ifsInclude.fail()) {
		printf("\n  Error: Cannot open include file name <%s> !",
				lcTimIncludeFilePath.c_str());
		return false;
	}

	CTimDescriptorLine* pLine = 0;
	while (!ifsInclude.eof()) {
		pLine = new CTimDescriptorLine;
		if (pIncludeLine) {
			*pLine = *pIncludeLine;
			delete pIncludeLine;
			pIncludeLine = 0;
		}

		if (pLine->ParseLine(ifsInclude, *this, bIsBlf)) {
			// if we have a field or just a comment, save the line, else delete it
			if (pLine->m_FieldName.empty() && pLine->m_FieldValue.empty()) {
				delete pLine;
				pLine = 0;
			} else {
				m_TimDescriptor.TimDescriptorLines().push_back(pLine);
			}
		} else {
			delete pLine;
			pLine = 0;
		}
	}

	// remove trailing blank lines of just comment lines at the end of an include file
	if (pLine != 0 && ifsInclude.eof()) {
		if (pLine->m_FieldName == "" && pLine->m_FieldValue == "") {
			m_TimDescriptor.TimDescriptorLines().pop_back();
			delete pLine;
		}
	}
	ifsInclude.close();

	return true;
}

bool CTimDescriptorParser::GetTimDescriptorLines(
		CCommandLineParser& CommandLineParser) {
	Reset();

	// save file path
	m_sTimFilePath = CommandLineParser.TimTxtFilePath;

	if (m_sTimFilePath.length() == 0) {
		printf("\n  Error: No TIM Descriptor file <%s>!",
				m_sTimFilePath.c_str());
		return false;
	}

	m_TimDescriptor.SaveTimDescriptorLines();
	m_TimDescriptor.DiscardTimDescriptorLines();

	// restore file path after m_TimDescriptor.Reset()
	m_TimDescriptor.TimDescriptorFilePath(m_sTimFilePath);

	OpenTimDescriptorTextFile(ios_base::in);

	string lcTimTxtFilePath = CTimLib::ToLower(
			m_TimDescriptor.TimDescriptorFilePath());
	bool bIsBlf = lcTimTxtFilePath.find(".blf")
			== lcTimTxtFilePath.length() - 4;

	if (m_ifsTimDescriptorTxtFile.is_open()) {
		while (!m_ifsTimDescriptorTxtFile.eof()) {
			CTimDescriptorLine* pLine = new CTimDescriptorLine;
			if (pLine->ParseLine(m_ifsTimDescriptorTxtFile, *this, bIsBlf)) {
				// if we have a field or just a comment, save the line, else delete it
				if (pLine->m_FieldName.empty() && pLine->m_FieldValue.empty()) {
					delete pLine;
				} else {
					m_TimDescriptor.TimDescriptorLines().push_back(pLine);

					string* psValue = &pLine->m_FieldName;

					// see if we have an include file statement
					size_t nIncludePos = 0;
					nIncludePos = psValue->find("#include ");
					if (nIncludePos == 0) {
						// remove the include line because it is now changed to a comment
						m_TimDescriptor.TimDescriptorLines().pop_back();

						size_t nPathStartPos = 0;
						size_t nPathEndPos = 0;
						nPathStartPos = psValue->find('\"');
						nPathEndPos = psValue->find('\"', nPathStartPos + 1);

						if ((nPathStartPos != string::npos)
								&& (nPathEndPos != string::npos)) {
							string sIncludeFilePath = psValue->substr(
									(nPathStartPos + 1),
									(nPathEndPos - nPathStartPos - 1));
							// pLine is passed to be used to initialize the first line of the include file with
							// the include line as a comment, after which it is deleted
							if (!GetTimDescriptorLinesFromIncludeFile(
									sIncludeFilePath, pLine)) {
								printf(
										"\nError: Include file parsing failed for <%s>!\n",
										sIncludeFilePath.c_str());
							}
						} else {
							printf(
									"\nError: Include file path not delimited by '\"' \n Error: Did not process #include for <%s>!\n",
									(*psValue).c_str());
						}
					}
				}
			} else
				delete pLine;
		}
	} else {
		printf(
				"\n\nTimDescriptor text file not opened GetTimDescriptorLines() \n");
		return false;
	}

	CloseTimDescriptorTextFile();

	// we read from an empty TIM, so try to restore
	if (m_TimDescriptor.TimDescriptorLines().size() == 0)
		m_TimDescriptor.RestoreTimDescriptorLines();

	return true;
}

bool CTimDescriptorParser::GetTimDescriptorLines(t_stringList& Lines) {
	Reset();

	m_sTimFilePath = m_TimDescriptor.TimDescriptorFilePath();

	m_TimDescriptor.SaveTimDescriptorLines();
	m_TimDescriptor.DiscardTimDescriptorLines();

	// restore file path after m_TimDescriptor.Reset()
	m_TimDescriptor.TimDescriptorFilePath(m_sTimFilePath);

	t_stringListIter iter;
	string lcTimTxtFilePath = CTimLib::ToLower(
			m_TimDescriptor.TimDescriptorFilePath());
	bool bIsBlf = lcTimTxtFilePath.find(".blf")
			== lcTimTxtFilePath.length() - 4;

	// we read from an empty TIM, so try to restore
	if (Lines.size() == 0)
		m_TimDescriptor.RestoreTimDescriptorLines();
	else {
		// restart at beginning each time
		while (iter = Lines.begin(), iter != Lines.end()) {
			CTimDescriptorLine* pLine = new CTimDescriptorLine;
			if (pLine->ParseLine(Lines, *this, bIsBlf)) {
				// if we have a field or just a comment, save the line, else delete it
				if (pLine->m_FieldName.empty() && pLine->m_FieldValue.empty()) {
					delete pLine;
				} else {
					m_TimDescriptor.TimDescriptorLines().push_back(pLine);
					string* psValue = &pLine->m_FieldName;

					// see if we have an include file statement
					size_t nIncludePos = 0;
					nIncludePos = psValue->find("#include ");

					if (nIncludePos == 0) {
						// remove the include line because it is now changed to a comment
						m_TimDescriptor.TimDescriptorLines().pop_back();

						size_t nPathStartPos = 0;
						size_t nPathEndPos = 0;
						nPathStartPos = psValue->find('\"');
						nPathEndPos = psValue->find('\"', nPathStartPos + 1);

						if ((nPathStartPos != string::npos)
								&& (nPathEndPos != string::npos)) {
							string sIncludeFilePath = psValue->substr(
									(nPathStartPos + 1),
									(nPathEndPos - nPathStartPos - 1));
							// pLine is passed to be used to initialize the first line of the include file with
							// the include line as a comment, after which it is deleted
							if (!GetTimDescriptorLinesFromIncludeFile(
									sIncludeFilePath, pLine)) {
								printf(
										"\nError: Include file parsing failed for <%s>!\n",
										sIncludeFilePath.c_str());
							}
						} else {
							printf(
									"\nError: Include file path not delimited by '\"' \n Error: Did not process #include for <%s>!\n",
									(*psValue).c_str());
						}
					}
				}
			} else
				delete pLine;
		}
	}

	return true;
}

bool CTimDescriptorParser::ParseDescriptor(
		CCommandLineParser& CommandLineParser) {
	bool bRet = true;
	pPrevLine = 0;

	// save file path
	m_sTimFilePath = m_TimDescriptor.TimDescriptorFilePath();

	m_TimDescriptor.DiscardAll();
	m_TimDescriptor.Reset();

	// restore file path after m_TimDescriptor.Reset()
	m_TimDescriptor.TimDescriptorFilePath(m_sTimFilePath);

	m_TimDescriptor.DiscardTimDescriptorErrors();
	m_TimDescriptor.DiscardTimDescriptorWarnings();

	string lcTimTxtFilePath = CTimLib::ToLower(
			m_TimDescriptor.TimDescriptorFilePath());
	bool bIsBlf = lcTimTxtFilePath.find(".blf")
			== lcTimTxtFilePath.length() - 4;

	CTimDescriptorLine* pLine = 0;

	if (!VerifyDescriptorIntegrity()) {
		printf(
				"\n  Error: TIM Descriptor file format incorrect. Cannot process file name <%s> !",
				m_sTimFilePath.c_str());
		bRet = false;
		goto Exit;
	}

	if ((pLine = m_TimDescriptor.GetLineField("Version", true))) {
		pPrevLine = pLine;
		m_TimDescriptor.getTimHeader().setVersion(
				Translate(pLine->m_FieldValue));
		if (m_TimDescriptor.getTimHeader().getVersion() >= TIM_3_2_00
				&& m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00)
			m_TimDescriptor.getTimHeader().setVersion(TIM_3_2_00);
		if (m_TimDescriptor.getTimHeader().getVersion() >= TIM_3_3_00
				&& m_TimDescriptor.getTimHeader().getVersion() < TIM_3_4_00)
			m_TimDescriptor.getTimHeader().setVersion(TIM_3_3_00);
		if (m_TimDescriptor.getTimHeader().getVersion() >= TIM_3_4_00
				&& m_TimDescriptor.getTimHeader().getVersion() < TIM_3_5_00)
			m_TimDescriptor.getTimHeader().setVersion(TIM_3_4_00);
		if (m_TimDescriptor.getTimHeader().getVersion() >= TIM_3_5_00
				&& m_TimDescriptor.getTimHeader().getVersion() < TIM_3_6_00)
			m_TimDescriptor.getTimHeader().setVersion(TIM_3_5_00);
		if (m_TimDescriptor.getTimHeader().getVersion() >= TIM_3_6_00)
			m_TimDescriptor.getTimHeader().setVersion(TIM_3_6_00);
	} else
		pLine = pPrevLine;

	if ((pLine = m_TimDescriptor.GetNextLineField("Trusted", pLine, true, 1))) {
		pPrevLine = pLine;
		m_TimDescriptor.getTimHeader().setTrusted(
				Translate(pLine->m_FieldValue));
	} else
		pLine = pPrevLine;

	// get first Image ID which must be the TIM*
	if ((pLine = m_TimDescriptor.GetLineField(
			bIsBlf ? "1_Image_Image_ID" : "Image ID", true))) {
		pPrevLine = pLine;
		m_TimDescriptor.getTimHeader().setIdentifier(TIMIDENTIFIER); // default

		if ((Translate(pLine->m_FieldValue) & TYPEMASK)
				== (TIMIDENTIFIER & TYPEMASK))
			m_TimDescriptor.getTimHeader().setIdentifier(
					Translate(pLine->m_FieldValue));
		else
			m_TimDescriptor.ParsingWarning(
					"  Warning: TIM* type is not the first Image ID in descriptor txt file.\n",
					true);
	} else
		pLine = pPrevLine;

	if (!m_TimDescriptor.getTimHeader().getTrusted()) {
		if (CommandLineParser.CheckCommandLineOptions() == false)
			return false;
		int iOption = CommandLineParser.iOption;
		if (iOption > 1) {
			stringstream ss;
			ss << "////////////////////////////////////////////////////////////"
					<< endl;
			ss << "  Error: You have selected option mode" << iOption << "!"
					<< endl;
			ss << "You can only build a Non Trusted Image in option mode 1."
					<< endl;
			ss << "Example: tbb -m 1 -k ntimkey.txt" << endl << endl;
			ss << "Also check the Trusted directive in the TIM descriptor file."
					<< endl;
			ss << "Example: Trusted: 0  (for Non Trusted Image)" << endl;
			ss << "Example: Trusted: 1  (for Trusted Image)" << endl;
			ss << "////////////////////////////////////////////////////////////"
					<< endl;
			m_TimDescriptor.ParsingError(ss.str(), true);
			bRet = false;
			goto Exit;
		}
	}

	if ((pLine = m_TimDescriptor.GetLineField("Trusted", true))) {
		pPrevLine = pLine;
		// just reposition line before continuing to parse
	} else
		pLine = pPrevLine;

	if (!m_TimDescriptor.getTimHeader().getTrusted()) {
		if (!ParseNonTrustedDescriptor(CommandLineParser, pLine)) {
			bRet = false;
			goto Exit;
		}
	} else {
		return ParseTrustedDescriptor(CommandLineParser, pLine);
	}

	Exit: if (!bRet) {
		// failed either to parse or the Tim text Image
		printf("\n\nErrors occurred parsing descriptor file!\n");
	}

	return bRet;
}

bool CTimDescriptorParser::ParseNonTrustedDescriptor(
		CCommandLineParser& CommandLineParser, CTimDescriptorLine*& pLine) {
	bool bRet = true;

	string lcTimTxtFilePath = CTimLib::ToLower(
			m_TimDescriptor.TimDescriptorFilePath());
	bool bIsBlf = lcTimTxtFilePath.find(".blf")
			== lcTimTxtFilePath.length() - 4;

	if (bIsBlf) {
		printf("\nParsing non trusted TIM in blf file...\n\n");
		m_TimDescriptor.BlfHeader().ParseBlfHeader(pLine);
	} else {
		printf("\nParsing non trusted TIM...\n\n");
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
		pPrevLine = pLine;
		if (ToUpper(pLine->m_FieldValue) == "UNDEFINED")
			pLine->m_FieldValue = "<undefined>";

		if (!m_TimDescriptor.ProcessorTypeStr(pLine->m_FieldValue)) {
			stringstream ss;
			ss << endl
					<< "Error: 'Processor Type' not recognized. See 'Host and Target Message' window.";
			m_TimDescriptor.ParsingError(ss.str(), true, pLine);
			printf("\nRecognized processor types:\n");
			printf(
					"   PXA168, PXA30x, PXA31x, PXA32x, ARMADA610, PXA91x, PXA92x, PXA93x, PXA94x, PXA95x,\n");
			printf(
					"   ARMADA16x, PXA955, PXA968, PXA1701, PXA978, PXA2128, ARMADA622, PXA1202, PXA1801,\n");
			printf(
					"   88PA62_70, PXA988, PXA1920, PXA2101, PXA192, PXA1928, PXA1986, PXA1802, PXA986,\n");
			printf(
					"    PXA1206, PXA888, PXA1088, PXA1812, PXA1822, PXA1U88, PXA1936, PXA1908, PXA1826, 88PA62_10,\n");
			printf("    88PA62_10, PXA1956, VEGA, BOWIE, ULC2, <undefined>\n");
		}
	} else {
		pLine = pPrevLine;
		stringstream ss;
		ss << endl << bIsBlf ?
				"Warning: Parsing of 'Processor_Type' missing " :
				"Warning: Parsing of 'Processor Type' missing ";
		if (m_TimDescriptor.pPrevLine)
			ss << "near line: '" << m_TimDescriptor.pPrevLine->m_FieldName
					<< "'";
		ss << endl;
		m_TimDescriptor.ParsingWarning(ss.str(), true,
				m_TimDescriptor.pPrevLine);
	}

	// since processor type may be in two places we need to find "Boot Flash Signature"
	// starting at the beginning again
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

	//return error if number of images > 256
	if ((int) TimHeader.getNumImages() > MAX_IMAGES) {
		m_TimDescriptor.ParsingError(
				"\n  Error: Maximum allowable number of images in the descriptor file is 10!\n",
				true, pLine);
		bRet = false;
	}

	pLine = bIsBlf ?
			m_TimDescriptor.GetLineField("Size_of_Reserved_in_bytes", false) :
			m_TimDescriptor.GetNextLineField("Size of Reserved in bytes", pLine,
					true, 15);
	if (pLine != 0) {
		pPrevLine = pLine;
		TimHeader.setSizeOfReserved(Translate(pLine->m_FieldValue));
	} else {
		pLine = pPrevLine;
		if (!bIsBlf) // ignore in blf because not in original blf syntax
			bRet = false;
	}
	if (TimHeader.getVersion() < TIM_3_6_00) {
		TimHeader.setWtmSaveStateFlashSignature(0xFFFFFFFF);
		TimHeader.setWtmSaveStateFlashEntryAddress(0xFFFFFFFF);
		TimHeader.setWtmSaveStateBackupEntryAddress(0xFFFFFFFF);
		TimHeader.setWtmSaveStatePatchSignature(0xFFFFFFFF);
		TimHeader.setWtmSaveStatePatchAddress(0xFFFFFFFF);
	}

	if (TimHeader.getNumImages() > MAX_IMAGES)
		m_TimDescriptor.ParsingError(
				"\n  Warning: The number of images exceeds 256!\n", true);

	string NextImageId("");
	stringstream ssImageNum;
	int nImage = 0;

	for (unsigned int i = 0; i < TimHeader.getNumImages(); i++) {
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
						true);
				bRet = false;
			}

			// ParseImageInfo already parsed the image path
			m_sTimBinFilename = pImageDesc->ImageFilePath();

			m_sTimBinFilename = CTimLib::MakePathAbsoluteOrRelativeToTIMTxtPath(
					m_sTimBinFilename, CommandLineParser.TimTxtFilePath,
					CommandLineParser.bTimTxtRelative);

			pImageDesc->ImageSize(
					m_TimDescriptor.GetTimImageSize(
							CommandLineParser.bOneNANDPadding,
							CommandLineParser.uiPaddedSize));
			if (!ParseImageEncryptSize(pImageDesc, pLine)) {
				bRet = false;
				delete pImageDesc;
				return false;
			}
		} else {
			if (pImageDesc->ImageId() != NextImageId) {
				m_TimDescriptor.ParsingError(
						"\n  Error: The ImageID doesn't match NextImageID from the previous image!",
						true);
				bRet = false;
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

	if (!ParseReservedData())
		return false;

	// if reserved data auto migrated then make sure to reset SizeOfReserved
	if (m_TimDescriptor.ReservedDataList().size() == 0)
		TimHeader.setSizeOfReserved(0);

	if (ParseExtendedReservedData(CommandLineParser) == false)
		return false;

	//check to make sure the images do not overlap each other
	if (CheckImageOverlap(m_TimDescriptor.ImagesList()) == false) {
		bRet = false;
	}

	if (CommandLineParser.bConcatenate) {
		ifstream ifsImage;
		t_ImagesIter iter = m_TimDescriptor.ImagesList().begin();
		while (iter != m_TimDescriptor.ImagesList().end()) {
			ifsImage.open((*iter)->ImageFilePath().c_str(),
					ios_base::in | ios_base::binary);
			if (ifsImage.bad() || ifsImage.fail()) {
				printf("\n  Error: Cannot open image file name <%s>.\n",
						(*iter)->ImageFilePath().c_str());
				return false;
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
					"  Success: Non-trusted Blf file parsing has completed successfully!\n");
		} else {
			printf(
					"  Success: Non-trusted Tim Descriptor file parsing has completed successfully!\n");
		}
	} else {
		if (bIsBlf) {
			printf(
					"  Failed: Non-trusted Blf file parsing has completed with parsing errors!\n");
		} else {
			printf(
					"  Failed: Non-trusted Tim Descriptor file parsing has completed with parsing errors!\n");
		}
	}

	return bRet;
}

bool CTimDescriptorParser::ParseImageInfo(CImageDescription& ImageDesc,
		CTimDescriptorLine*& pLine, string sNum) {
	bool bRet = true;
	string lcTimTxtFilePath = CTimLib::ToLower(
			m_TimDescriptor.TimDescriptorFilePath());
	bool bIsBlf = lcTimTxtFilePath.find(".blf")
			== lcTimTxtFilePath.length() - 4;

	if (bIsBlf) {
		bRet = ImageDesc.ParseImageInfoBlf(pLine, pPrevLine, sNum);
	} else {
		bRet = ImageDesc.ParseImageInfoTIM(pLine, pPrevLine);
	}

	return bRet;
}

bool CTimDescriptorParser::VerifyDescriptorIntegrity() {
	bool IsOk = true;

	string sVersion;
	string sTrusted;
	string sLine;
	string sSizeOfReserved;
	bool bRet = true;

	// This function is the main entry point for verifying integrity of the
	// descriptor text file before any work is done. It is meant to be a quick
	// sanity check.
	string lcTimTxtFilePath = CTimLib::ToLower(
			m_TimDescriptor.TimDescriptorFilePath());
	bool bIsBlf = lcTimTxtFilePath.find(".blf")
			== lcTimTxtFilePath.length() - 4;

	if (bIsBlf) {
		printf("\nVerifying Blf TIM Descriptor Integrity...\n\n");
	} else {
		printf("\nVerifying Descriptor Integrity...\n\n");
	}

	stringstream ss;

	CTimDescriptorLine* pLine = 0;
	if ((pLine = CTimDescriptor::GetLineField("Version", true))) {
		pPrevLine = pLine;
		sVersion = pLine->m_FieldValue;
	} else {
		pLine = pPrevLine;
		bRet = false;
	}

	if ((pLine = m_TimDescriptor.GetNextLineField("Trusted", pLine, true, 1))) {
		pPrevLine = pLine;
		sTrusted = pLine->m_FieldValue;
	} else {
		pLine = pPrevLine;
		bRet = false;
	}

	pLine = bIsBlf ?
			m_TimDescriptor.GetLineField("Size_of_Reserved_in_bytes", false) :
			m_TimDescriptor.GetNextLineField("Size of Reserved in bytes", pLine,
					true, 15);
	if (pLine != 0) {
		pPrevLine = pLine;
		sSizeOfReserved = pLine->m_FieldValue;
	} else {
		pLine = pPrevLine;
		if (!bIsBlf) // ignore in blf because not in original blf syntax
			bRet = false;
	}

	if (VerifyNumberOfImages() == false)
		IsOk = false;
	if ((Translate(sSizeOfReserved) != 0)
			&& (Translate(sVersion) >= (0x030102))) {
		if (VerifyReservedData() == false)
			IsOk = false;
	}

	if (Translate(sTrusted) == 0) {
		if (VerifyNonTrusted() == false)
			IsOk = false;
	} else {
		if (VerifyNumberOfKeys() == false)
			IsOk = false;
	}

	return IsOk && bRet;
}

bool CTimDescriptorParser::VerifyNumberOfImages() {
	unsigned int i = 0;
	int NumberFound = 0;
	unsigned int iNumImages = 0;
	bool bRet = true;

	string lcTimTxtFilePath = CTimLib::ToLower(
			m_TimDescriptor.TimDescriptorFilePath());
	bool bIsBlf = lcTimTxtFilePath.find(".blf")
			== lcTimTxtFilePath.length() - 4;

	CTimDescriptorLine* pLine = 0;

	pLine = bIsBlf ?
			m_TimDescriptor.GetLineField("Number_of_Images", true) :
			m_TimDescriptor.GetLineField("Number of Images", true);
	if (pLine == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else
		pPrevLine = pLine;

	if (pLine)
		iNumImages = Translate(pLine->m_FieldValue);

	if (iNumImages < 1 || iNumImages > MAX_IMAGES) {
		stringstream ss;
		ss << endl << "  Error: "
				<< (bIsBlf ? "<Number_of_Images>" : "<Number of Images>")
				<< " declaration statement must be > 0 and <" << MAX_IMAGES
				<< "!" << endl;
		m_TimDescriptor.ParsingError(ss.str(), true);
		bRet = false;
	}

	// find first image_ID
	stringstream ssImageNum;
	int nImage = 1;
	ssImageNum.str("");
	ssImageNum << nImage << "_";

	pLine = bIsBlf ?
			m_TimDescriptor.GetLineField(ssImageNum.str() + "Image_Image_ID",
					true) :
			m_TimDescriptor.GetLineField("Image ID", true);
	if (pLine == 0) {
		pLine = pPrevLine;
		bRet = false;
	} else {
		pPrevLine = pLine;
		NumberFound++;
	}

	// find the remaining images
	while (pLine) {
		nImage++;
		ssImageNum.str("");
		ssImageNum << nImage << "_";

		pLine = bIsBlf ?
				m_TimDescriptor.GetNextLineField(
						ssImageNum.str() + "Image_Image_ID", pLine, false, -1) :
				m_TimDescriptor.GetNextLineField("Image ID", pLine, false, 13);
		if (pLine) {
			pPrevLine = pLine;
			NumberFound++;
			pLine = bIsBlf ?
					m_TimDescriptor.GetNextLineField(
							ssImageNum.str() + "Image_Path", pLine, false, -1) :
					m_TimDescriptor.GetNextLineField("Image Filename", pLine,
							false, 13);
			if (pLine != 0) {
				pPrevLine = pLine;
			}
		}
	}

	if (NumberFound != iNumImages) {
		stringstream ss;
		ss << endl << "  Error: The number of images found <" << NumberFound
				<< ">";
		ss << " does not match the 'Number of Images: " << iNumImages << "'"
				<< endl;
		m_TimDescriptor.ParsingError(ss.str(), true);
		bRet = false;
	}

	return bRet;
}

bool CTimDescriptorParser::VerifyNonTrusted() {
	bool bRet = true;

	string lcTimTxtFilePath = CTimLib::ToLower(
			m_TimDescriptor.TimDescriptorFilePath());
	bool bIsBlf = lcTimTxtFilePath.find(".blf")
			== lcTimTxtFilePath.length() - 4;

	CTimDescriptorLine* pLine = 0;
	if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_6_00) {
		pLine = bIsBlf ?
				CTimDescriptor::GetLineField("WTM_Save_State", false) :
				CTimDescriptor::GetLineField("WTM Save State", false);
		if (pLine != 0) {
			pLine = pPrevLine;
			stringstream ss;
			ss
					<< "  Error: The non trusted descriptor file should not contain any"
					<< endl;
			ss << (bIsBlf ? "<WTM_Save_State>" : "<WTM Save State>")
					<< " declaration statements!" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true);
			bRet = false;
		} else
			pPrevLine = pLine;
	}

	if (!bIsBlf) {
		// the blf file contains a number_of_keys field regardless of trusted or non-trusted
		// for we block checking it for now
		pLine = bIsBlf ?
				CTimDescriptor::GetLineField("Number_of_Keys", false) :
				CTimDescriptor::GetLineField("Number of Keys", false);
		if (pLine != 0) {
			pLine = pPrevLine;
			stringstream ss;
			ss
					<< "  Error: The non trusted descriptor file should not contain a"
					<< endl;
			ss << (bIsBlf ? "<Number_of_Keys>" : "<Number of Keys>")
					<< " declaration statement!" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true);
			bRet = false;
		} else
			pPrevLine = pLine;
	}

	pLine = bIsBlf ?
			CTimDescriptor::GetLineField("Key_ID", false) :
			CTimDescriptor::GetLineField("Key ID", false);
	if (pLine != 0) {
		pLine = pPrevLine;
		stringstream ss;
		ss << "  Error: The non trusted descriptor file should not contain a"
				<< endl;
		ss << (bIsBlf ? "<Key_ID>" : "<Key ID>") << " declaration statement!"
				<< endl;
		m_TimDescriptor.ParsingError(ss.str(), true);
		bRet = false;
	} else
		pPrevLine = pLine;

	if (m_TimDescriptor.getTimHeader().getVersion() < TIM_3_3_00) {
		pLine = bIsBlf ?
				CTimDescriptor::GetLineField("Modulus Size in bytes", false) :
				CTimDescriptor::GetLineField("Modulus_Size_in_bytes", false);
		if (pLine != 0) {
			pLine = pPrevLine;
			stringstream ss;
			ss
					<< "  Error: The non trusted descriptor file should not contain a"
					<< endl;
			ss
					<< (bIsBlf ?
							"<Modulus_Size_in_bytes>" :
							"<Modulus Size in bytes>")
					<< " declaration statement!" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true);
			bRet = false;
		} else
			pPrevLine = pLine;

		pLine = bIsBlf ?
				CTimDescriptor::GetLineField("Public_Key_Size_in_bytes",
						false) :
				CTimDescriptor::GetLineField("Public Key Size in bytes", false);
		if (pLine != 0) {
			pLine = pPrevLine;
			stringstream ss;
			ss
					<< "  Error: The non trusted descriptor file should not contain a"
					<< endl;
			ss
					<< (bIsBlf ?
							"<Public_Key_Size_in_bytes>" :
							"<Public Key Size in bytes>")
					<< " declaration statement!" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true);
			bRet = false;
		} else
			pPrevLine = pLine;
	} else {
		pLine = bIsBlf ?
				CTimDescriptor::GetLineField("Key_Size_in_bits", false) :
				CTimDescriptor::GetLineField("Key Size in bits", false);
		if (pLine != 0) {
			pLine = pPrevLine;
			stringstream ss;
			ss
					<< "  Error: The non trusted descriptor file should not contain a"
					<< endl;
			ss << (bIsBlf ? "<Key_Size_in_bits>" : "<Key Size in bits>")
					<< " declaration statement!" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true);
			bRet = false;
		} else
			pPrevLine = pLine;

		pLine = bIsBlf ?
				CTimDescriptor::GetLineField("Public_Key_Size_in_bits", false) :
				CTimDescriptor::GetLineField("Public Key Size in bits", false);
		if (pLine != 0) {
			pLine = pPrevLine;
			stringstream ss;
			ss
					<< "  Error: The non trusted descriptor file should not contain a"
					<< endl;
			ss
					<< (bIsBlf ?
							"<Public_Key_Size_in_bits>" :
							"<Public Key Size in bits>")
					<< " declaration statement!" << endl;
			m_TimDescriptor.ParsingError(ss.str(), true);
			bRet = false;
		} else
			pPrevLine = pLine;
	}

	pLine = bIsBlf ?
			CTimDescriptor::GetLineField("RSA_Public_Exponent", false) :
			CTimDescriptor::GetLineField("RSA Public Exponent", false);
	if (pLine != 0) {
		pLine = pPrevLine;
		stringstream ss;
		ss << "  Error: The non trusted descriptor file should not contain a"
				<< endl;
		ss << (bIsBlf ? "<RSA_Public_Exponent>" : "<RSA Public Key Exponent>")
				<< " declaration statement!" << endl;
		m_TimDescriptor.ParsingError(ss.str(), true);
		bRet = false;
	} else
		pPrevLine = pLine;

	pLine = bIsBlf ?
			CTimDescriptor::GetLineField("RSA_System_Modulus", false) :
			CTimDescriptor::GetLineField("RSA System Modulus", false);
	if (pLine != 0) {
		pLine = pPrevLine;
		stringstream ss;
		ss << "  Error: The non trusted descriptor file should not contain a"
				<< endl;
		ss << (bIsBlf ? "<RSA_System_Modulus>" : "<RSA System Modulus>")
				<< " declaration statement!" << endl;
		m_TimDescriptor.ParsingError(ss.str(), true);
		bRet = false;
	} else
		pPrevLine = pLine;

	pLine = bIsBlf ?
			CTimDescriptor::GetLineField("DSA_Algorithm_ID", false) :
			CTimDescriptor::GetLineField("DSA Algorithm ID", false);
	if (pLine != 0) {
		pLine = pPrevLine;
		stringstream ss;
		ss << "  Error: The non trusted descriptor file should not contain a"
				<< endl;
		ss << (bIsBlf ? "<DSA_Algorithm_ID>" : "<DSA Algorithm ID>")
				<< " declaration statement!" << endl;
		m_TimDescriptor.ParsingError(ss.str(), true);
		bRet = false;
	} else
		pPrevLine = pLine;

	pLine = bIsBlf ?
			CTimDescriptor::GetLineField("RSA_Public_Exponent", false) :
			CTimDescriptor::GetLineField("RSA Public Exponent", false);
	if (pLine != 0) {
		pLine = pPrevLine;
		stringstream ss;
		ss << "  Error: The non trusted descriptor file should not contain a"
				<< endl;
		ss << (bIsBlf ? "<RSA_Public_Exponent>" : "<RSA Public Exponent>")
				<< " declaration statement!" << endl;
		m_TimDescriptor.ParsingError(ss.str(), true);
		bRet = false;
	} else
		pPrevLine = pLine;

	pLine = bIsBlf ?
			CTimDescriptor::GetLineField("RSA_Private_Key", false) :
			CTimDescriptor::GetLineField("RSA Private Key", false);
	if (pLine != 0) {
		pLine = pPrevLine;
		stringstream ss;
		ss << "  Error: The non trusted descriptor file should not contain a"
				<< endl;
		ss << (bIsBlf ? "<RSA_Private_Key>" : "<RSA Private Key>")
				<< " declaration statement!" << endl;
		m_TimDescriptor.ParsingError(ss.str(), true);
		bRet = false;
	} else
		pPrevLine = pLine;

	return bRet;
}

bool CTimDescriptorParser::VerifyReservedData() {
	char *pData = NULL;
	DWORD dwValue = 0;
	int numOfHeaders = 0;
	int iTotal = 0;
	int headerCount = 0;
	bool termIdHeader = false;
	bool isWtpReservedAreaId = false;
	char* pToken = 0;
	DWORD dwPackageSize = 0;
	bool bPackageSizeNext = false;
	bool bRet = true;
	string sData;
	eProcessorType PT = m_TimDescriptor.ProcessorType();

	string lcTimTxtFilePath = CTimLib::ToLower(
			m_TimDescriptor.TimDescriptorFilePath());
	bool bIsBlf = lcTimTxtFilePath.find(".blf")
			== lcTimTxtFilePath.length() - 4;

	CTimDescriptorLine* pLine = 0;
	if (m_TimDescriptor.getTimHeader().getSizeOfReserved() > 0) {
		if (bIsBlf)
			pLine = CTimDescriptor::GetLineField("[Reserved_Data]");
		else
			pLine = CTimDescriptor::GetLineField("Reserved Data");
	}

	if (pLine) {
		while ((pLine = m_TimDescriptor.GetNextLineField(pLine))) {
			if (!pLine->m_FieldName.empty())
				break;
			dwValue = Translate(pLine->m_FieldValue);
			iTotal++;
			// Check for reserved area ID
			if ((iTotal == 1) && (dwValue == WTPRESERVEDAREAID)) {
				isWtpReservedAreaId = true;
				continue;
			}

			if (isWtpReservedAreaId == true) {
				// If there is a reserved area ID, next field should contain the number of package header ID's
				if (iTotal == 2) {
					numOfHeaders = (int) dwValue;
					continue;
				}
				//Count the number of package Header ID's
				else if ((dwPackageSize == 0) & (	// predefined package IDs
						(dwValue == AUTOBIND) || (dwValue == COREID)
								|| (dwValue == BBMTID)
								|| (dwValue == ESCAPESEQID)
								|| (dwValue == GPIOID)
								|| (dwValue == RESUMEBLID)
								|| (dwValue == ROMRESUMEID)
								|| (dwValue == TBR_XFER) || (dwValue == UARTID)
								|| (dwValue == USBID)
								|| (dwValue == USBVENDORREQ)
								|| (dwValue == USB_DEVICE_DESCRIPTOR)
								|| (dwValue == USB_CONFIG_DESCRIPTOR)
								|| (dwValue == USB_INTERFACE_DESCRIPTOR)
								|| (dwValue == USB_LANGUAGE_STRING_DESCRIPTOR)
								|| (dwValue
										== USB_MANUFACTURER_STRING_DESCRIPTOR)
								|| (dwValue == USB_PRODUCT_STRING_DESCRIPTOR)
								|| (dwValue == USB_SERIAL_STRING_DESCRIPTOR)
								|| (dwValue == USB_INTERFACE_STRING_DESCRIPTOR)
								|| (dwValue == USB_DEFAULT_STRING_DESCRIPTOR)
								|| (dwValue == USB_ENDPOINT_DESCRIPTOR)
								|| (dwValue == OEMCUSTOMID)
								|| (dwValue == DTYPID)
								|| (dwValue == NOMONITORID)
								|| ((dwValue == CLKEID)
										&& (PT == UNDEFINED
												|| (PT == PXA168
														|| PT == ARMADA16x
														|| PT == PXA91x
														|| PT == PXA92x
														|| PT == PXA93x
														|| PT == PXA94x
														|| PT == PXA95x
														|| PT == ARMADA610
														|| PT == ARMADA622
														|| PT == PXA2128)))
								|| ((dwValue == DDRGID)
										&& (PT == UNDEFINED
												|| (PT == PXA168
														|| PT == ARMADA16x
														|| PT == PXA91x
														|| PT == PXA92x
														|| PT == PXA93x
														|| PT == PXA94x
														|| PT == PXA95x
														|| PT == ARMADA610
														|| PT == ARMADA622
														|| PT == PXA2128)))
								|| ((dwValue == DDRTID)
										&& (PT == UNDEFINED
												|| (PT == PXA168
														|| PT == ARMADA16x
														|| PT == PXA91x
														|| PT == PXA92x
														|| PT == PXA93x
														|| PT == PXA94x
														|| PT == PXA95x
														|| PT == ARMADA610
														|| PT == ARMADA622
														|| PT == PXA2128)))
								|| ((dwValue == DDRCID)
										&& (PT == UNDEFINED
												|| (PT == PXA168
														|| PT == ARMADA16x
														|| PT == PXA91x
														|| PT == PXA92x
														|| PT == PXA93x
														|| PT == PXA94x
														|| PT == PXA95x
														|| PT == ARMADA610
														|| PT == ARMADA622
														|| PT == PXA2128)))
								|| ((dwValue == FREQID)
										&& (PT == UNDEFINED
												|| (PT == PXA168
														|| PT == ARMADA16x
														|| PT == PXA91x
														|| PT == PXA92x
														|| PT == PXA93x
														|| PT == PXA94x
														|| PT == PXA95x
														|| PT == ARMADA610
														|| PT == ARMADA622
														|| PT == PXA2128)))
								|| ((dwValue == VOLTID)
										&& (PT == UNDEFINED
												|| (PT == PXA168
														|| PT == ARMADA16x
														|| PT == PXA91x
														|| PT == PXA92x
														|| PT == PXA93x
														|| PT == PXA94x
														|| PT == PXA95x
														|| PT == ARMADA610
														|| PT == ARMADA622
														|| PT == PXA2128)))
								|| ((dwValue == CMCCID)
										&& (PT == UNDEFINED
												|| (PT == PXA168
														|| PT == ARMADA16x
														|| PT == PXA91x
														|| PT == PXA92x
														|| PT == PXA93x
														|| PT == PXA94x
														|| PT == PXA95x
														|| PT == ARMADA610
														|| PT == ARMADA622
														|| PT == PXA2128)))
								|| ((dwValue == PINID)
										&& (PT == UNDEFINED || PT == ARMADA610
												|| PT == ARMADA622
												|| PT == PXA2128
												|| PT == PXA1928
												|| PT == PXA1U88
												|| PT == PXA1936
												|| PT == P88PA62_70
												|| PT == P88PA62_20
												|| PT == P88PA62_10
												|| PT == PXA988 || PT == PXA1920
												|| PT == PXA2101 || PT == PXA192
												|| PT == PXA1986
												|| PT == PXA1826
												|| PT == PXA1956 || PT == VEGA
												|| PT == BOWIE || PT == ULC2))
								|| ((dwValue == IMAPID)
										&& (PT == UNDEFINED || PT == ARMADA610
												|| PT == ARMADA622
												|| PT == PXA2128
												|| PT == PXA1701 || PT == PXA978
												|| PT == PXA1801
												|| PT == PXA1802
												|| PT == PXA1812
												|| PT == PXA1822
												|| PT == PXA1928
												|| PT == P88PA62_70
												|| PT == P88PA62_20
												|| PT == P88PA62_10
												|| PT == P88PA62_20
												|| PT == P88PA62_10
												|| PT == PXA988 || PT == PXA1920
												|| PT == PXA2101 || PT == PXA192
												|| PT == PXA1986 || PT == PXA986
												|| PT == PXA888 || PT == PXA1088
												|| PT == PXA1908))
								|| ((dwValue == TZID)
										&& (PT == UNDEFINED || PT == ARMADA610
												|| PT == ARMADA622
												|| PT == PXA2128))
								|| ((dwValue == TZON)
										&& (PT == UNDEFINED || PT == ARMADA610
												|| PT == ARMADA622
												|| PT == PXA2128))
								|| ((dwValue == OPDIVID)
										&& (PT == UNDEFINED || PT == PXA168
												|| PT == ARMADA16x))
								|| ((dwValue == OPMODEID)
										&& (PT == UNDEFINED || PT == PXA168
												|| PT == ARMADA16x))
								|| ((dwValue == CIDPID)
										&& (PT == UNDEFINED || PT == ARMADA610
												|| PT == PXA1920
												|| PT == ARMADA622
												|| PT == PXA2128 || PT == PXA988
												|| PT == PXA1088
												|| PT == PXA1986
												|| PT == PXA1701 || PT == PXA978
												|| PT == PXA1202
												|| PT == PXA1801
												|| PT == PXA1802
												|| PT == PXA1812
												|| PT == PXA1822
												|| PT == PXA1986
												|| PT == PXA1928
												|| PT == P88PA62_70
												|| PT == P88PA62_20
												|| PT == P88PA62_10
												|| PT == PXA1U88
												|| PT == PXA1936
												|| PT == PXA1908
												|| PT == PXA1826
												|| PT == PXA1956 || PT == VEGA
												|| PT == BOWIE || PT == ULC2))
								|| ((dwValue == TZRIID)
										&& (PT == UNDEFINED || PT == ARMADA610
												|| PT == PXA1920
												|| PT == ARMADA622
												|| PT == PXA2128 || PT == PXA988
												|| PT == PXA1088
												|| PT == PXA1701 || PT == PXA978
												|| PT == PXA1202
												|| PT == PXA1801
												|| PT == PXA1802
												|| PT == PXA1812
												|| PT == PXA1822
												|| PT == PXA1986
												|| PT == PXA1928
												|| PT == P88PA62_70
												|| PT == P88PA62_20
												|| PT == P88PA62_10
												|| PT == PXA1U88
												|| PT == PXA1936
												|| PT == PXA1908
												|| PT == PXA1826
												|| PT == PXA1956 || PT == VEGA
												|| PT == BOWIE || PT == ULC2))
								|| ((((dwValue & TYPEMASK) >> 8) == DDRTYPE)
										&& (PT == UNDEFINED || PT == ARMADA610
												|| PT == PXA1920
												|| PT == ARMADA622
												|| PT == PXA2128 || PT == PXA988
												|| PT == PXA1088 || PT == PXA978
												|| PT == PXA1701
												|| PT == PXA1202
												|| PT == PXA1801
												|| PT == PXA1802
												|| PT == PXA1812
												|| PT == PXA1822
												|| PT == PXA1986
												|| PT == PXA1928
												|| PT == P88PA62_70
												|| PT == P88PA62_20
												|| PT == P88PA62_10
												|| PT == PXA1U88 || PXA1936
												|| PT == PXA1908
												|| PT == PXA1826
												|| PT == PXA1956 || PT == VEGA
												|| PT == BOWIE || PT == ULC2))
								|| ((((dwValue & TYPEMASK) >> 8) == TZRTYPE)
										&& (PT == UNDEFINED || PT == ARMADA610
												|| PT == PXA1920
												|| PT == ARMADA622
												|| PT == PXA2128 || PT == PXA988
												|| PT == PXA1088 || PT == PXA978
												|| PT == PXA1701
												|| PT == PXA1202
												|| PT == PXA1801
												|| PT == PXA1802
												|| PT == PXA1812
												|| PT == PXA1822
												|| PT == PXA1986
												|| PT == PXA1928
												|| PT == P88PA62_70
												|| PT == P88PA62_20
												|| PT == P88PA62_10
												|| PT == PXA1U88 || PXA1936
												|| PT == PXA1908
												|| PT == PXA1826
												|| PT == PXA1956 || PT == VEGA
												|| PT == BOWIE || PT == ULC2))
								|| ((((dwValue & TYPEMASK) >> 8) == GPPTYPE)
										&& (PT == UNDEFINED || PT == ARMADA610
												|| PT == PXA1920
												|| PT == ARMADA622
												|| PT == PXA2128 || PT == PXA988
												|| PT == PXA1088 || PT == PXA978
												|| PT == PXA1701
												|| PT == PXA1202
												|| PT == PXA1801
												|| PT == PXA1802
												|| PT == PXA1812
												|| PT == PXA1822
												|| PT == PXA1986
												|| PT == PXA1928
												|| PT == P88PA62_70
												|| PT == P88PA62_20
												|| PT == P88PA62_10
												|| PT == PXA1U88 || PXA1936
												|| PT == PXA1908
												|| PT == PXA1826
												|| PT == PXA1956 || PT == VEGA
												|| PT == BOWIE || PT == ULC2))
								|| ((((dwValue & TYPEMASK) >> 8) == ESCSEQID_V2)
										&& (PT == UNDEFINED || PT == PXA1986))
								|| ((((dwValue & TYPEMASK) >> 8) == COREID_V2)
										&& (PT == UNDEFINED || PT == PXA1986))
								|| ((dwValue == ONDEMANDBOOTTID)
										&& (PT == UNDEFINED))
								|| ((dwValue == MV_IMAGE_PKGS_TAG)
										&& (PT == UNDEFINED))
								|| ((dwValue == MV_IMAGE_HEADER_TAG)
										&& (PT == UNDEFINED))
								|| ((dwValue == MV_CUST_KEY_PKG_TAG)
										&& (PT == UNDEFINED))
								|| ((dwValue == MV_EXT_RSA_KEY_PKG_TAG)
										&& (PT == UNDEFINED)))) {
					headerCount++;
					bPackageSizeNext = true;
					continue;
				}
				// ReservedArea has to contain a terminator ID if it contains a reserved area ID as 1st field.
				else if (dwValue == TERMINATORID) {
					termIdHeader = true;
					headerCount++;
					break;
				} else if (bPackageSizeNext) {
					dwPackageSize = dwValue - 8; // -8 because size includes tag and pkg len fields
					bPackageSizeNext = false;
					continue;
				} else {
					// we are adding a data field to a package
					if (dwPackageSize > 0) {
						dwPackageSize -= 4;
						continue;
					} else {
						// finished adding data to the package
						// reinit for next package
						dwPackageSize = 0;
					}
				}

				// if we get here, then we are not currently processing
				// a package and we didn't recognize the dwValue as a predefined
				// package id, so we assume it is a custom package id
				// immediately followed by a package size field
				if (dwPackageSize == 0) {
					headerCount++;
					bPackageSizeNext = true;
				}
			}
		}
	}

	if (dwPackageSize != 0) {
		stringstream ss;
		ss << "  Error: Package size not consistent with supplied field values."
				<< endl;
		m_TimDescriptor.ParsingError(ss.str(), true);
		bRet = false;
	}

	if (!termIdHeader && (isWtpReservedAreaId == true)) {
		stringstream ss;
		ss << "  Error: Terminator ID is incorrect or is missing." << endl;
		m_TimDescriptor.ParsingError(ss.str(), true);
		bRet = false;
	}

	if ((headerCount != numOfHeaders) && (isWtpReservedAreaId == true)) {
		stringstream ss;
		ss << "  Error: Number of Reserved Area Packages " << headerCount
				<< " does not equal the number of headers " << numOfHeaders
				<< endl;
		m_TimDescriptor.ParsingError(ss.str(), true);
		bRet = false;
	}

	return bRet;
}

bool CTimDescriptorParser::VerifySizeOfTim(
		CCommandLineParser& CommandLineParser) {
	int iTotalImageSize = m_TimDescriptor.GetTimImageSize(
			CommandLineParser.bOneNANDPadding, CommandLineParser.uiPaddedSize);
	return true;
}

bool CTimDescriptorParser::ParseTrustedDescriptor(
		CCommandLineParser& CommandLineParser, CTimDescriptorLine*& pLine) {
	stringstream ss;
	ss << "  Error: You can only build a Non Trusted Image" << endl;
	ss << "  with the Non Trusted only version of the TBB Tool!" << endl
			<< endl;
	ss << "  Processing of TIM descriptor has stopped because of errors!"
			<< endl;
	m_TimDescriptor.ParsingError(ss.str(), true);
	return false;
}

bool CTimDescriptorParser::ParseReservedData() {
	DWORD dwValue = 0;
	int numOfHeaders = 0;
	int iTotal = 0, headerCount = 0;
	bool termIdHeader = false;
	bool isWtpReservedAreaId = false;
	DWORD dwPackageSize = 0;
	CReservedPackageData* pPackageData = 0;
	bool bPackageSizeNext = false;
	bool bRet = true;
	eProcessorType PT = m_TimDescriptor.ProcessorType();

	string lcTimTxtFilePath = CTimLib::ToLower(
			m_TimDescriptor.TimDescriptorFilePath());
	bool bIsBlf = lcTimTxtFilePath.find(".blf")
			== lcTimTxtFilePath.length() - 4;

	CTimDescriptorLine* pLine = 0;
	if (m_TimDescriptor.getTimHeader().getSizeOfReserved() > 0) {
		if (bIsBlf)
			pLine = CTimDescriptor::GetLineField("[Reserved_Data]");
		else
			pLine = CTimDescriptor::GetLineField("Reserved Data");
	}

	if (pLine) {
		while ((pLine = m_TimDescriptor.GetNextLineField(pLine))) {
			if (!pLine->m_FieldName.empty())
				break;

			dwValue = Translate(pLine->m_FieldValue);
			iTotal++;

			// Check for reserved area ID
			if ((iTotal == 1) && (dwValue == WTPRESERVEDAREAID)) {
				isWtpReservedAreaId = true;
				continue;
			}

			if (isWtpReservedAreaId == true) {
				// If there is a reserved area ID, next field should contain the number of package header ID's
				if (iTotal == 2) {
					numOfHeaders = (int) dwValue;
					continue;
				}
				//Count the number of package Header ID's
				else if ((dwPackageSize == 0) & (	// predefined package IDs
						(dwValue == AUTOBIND) || (dwValue == COREID)
								|| (dwValue == BBMTID)
								|| (dwValue == ESCAPESEQID)
								|| (dwValue == GPIOID)
								|| (dwValue == RESUMEBLID)
								|| (dwValue == ROMRESUMEID)
								|| (dwValue == TBR_XFER) || (dwValue == UARTID)
								|| (dwValue == USBID)
								|| (dwValue == USBVENDORREQ)
								|| (dwValue == USB_DEVICE_DESCRIPTOR)
								|| (dwValue == USB_CONFIG_DESCRIPTOR)
								|| (dwValue == USB_INTERFACE_DESCRIPTOR)
								|| (dwValue == USB_LANGUAGE_STRING_DESCRIPTOR)
								|| (dwValue
										== USB_MANUFACTURER_STRING_DESCRIPTOR)
								|| (dwValue == USB_PRODUCT_STRING_DESCRIPTOR)
								|| (dwValue == USB_SERIAL_STRING_DESCRIPTOR)
								|| (dwValue == USB_INTERFACE_STRING_DESCRIPTOR)
								|| (dwValue == USB_DEFAULT_STRING_DESCRIPTOR)
								|| (dwValue == USB_ENDPOINT_DESCRIPTOR)
								|| (dwValue == OEMCUSTOMID)
								|| (dwValue == DTYPID)
								|| (dwValue == NOMONITORID)
								|| ((dwValue == CLKEID)
										&& (PT == UNDEFINED
												|| (PT == PXA168
														|| PT == ARMADA16x
														|| PT == PXA91x
														|| PT == PXA92x
														|| PT == PXA93x
														|| PT == PXA94x
														|| PT == PXA95x
														|| PT == ARMADA610
														|| PT == ARMADA622
														|| PT == PXA2128)))
								|| ((dwValue == DDRGID)
										&& (PT == UNDEFINED
												|| (PT == PXA168
														|| PT == ARMADA16x
														|| PT == PXA91x
														|| PT == PXA92x
														|| PT == PXA93x
														|| PT == PXA94x
														|| PT == PXA95x
														|| PT == ARMADA610
														|| PT == ARMADA622
														|| PT == PXA2128)))
								|| ((dwValue == DDRTID)
										&& (PT == UNDEFINED
												|| (PT == PXA168
														|| PT == ARMADA16x
														|| PT == PXA91x
														|| PT == PXA92x
														|| PT == PXA93x
														|| PT == PXA94x
														|| PT == PXA95x
														|| PT == ARMADA610
														|| PT == ARMADA622
														|| PT == PXA2128)))
								|| ((dwValue == DDRCID)
										&& (PT == UNDEFINED
												|| (PT == PXA168
														|| PT == ARMADA16x
														|| PT == PXA91x
														|| PT == PXA92x
														|| PT == PXA93x
														|| PT == PXA94x
														|| PT == PXA95x
														|| PT == ARMADA610
														|| PT == ARMADA622
														|| PT == PXA2128)))
								|| ((dwValue == FREQID)
										&& (PT == UNDEFINED
												|| (PT == PXA168
														|| PT == ARMADA16x
														|| PT == PXA91x
														|| PT == PXA92x
														|| PT == PXA93x
														|| PT == PXA94x
														|| PT == PXA95x
														|| PT == ARMADA610
														|| PT == ARMADA622
														|| PT == PXA2128)))
								|| ((dwValue == VOLTID)
										&& (PT == UNDEFINED
												|| (PT == PXA168
														|| PT == ARMADA16x
														|| PT == PXA91x
														|| PT == PXA92x
														|| PT == PXA93x
														|| PT == PXA94x
														|| PT == PXA95x
														|| PT == ARMADA610
														|| PT == ARMADA622
														|| PT == PXA2128)))
								|| ((dwValue == CMCCID)
										&& (PT == UNDEFINED
												|| (PT == PXA168
														|| PT == ARMADA16x
														|| PT == PXA91x
														|| PT == PXA92x
														|| PT == PXA93x
														|| PT == PXA94x
														|| PT == PXA95x
														|| PT == ARMADA610
														|| PT == ARMADA622
														|| PT == PXA2128)))
								|| ((dwValue == PINID)
										&& (PT == UNDEFINED || PT == ARMADA610
												|| PT == ARMADA622
												|| PT == PXA2128 || PT == PXA988
												|| PT == PXA1928
												|| PT == PXA1U88
												|| PT == PXA1936
												|| PT == P88PA62_70
												|| PT == P88PA62_20
												|| PT == P88PA62_10
												|| PT == PXA1920
												|| PT == PXA2101 || PT == PXA192
												|| PT == PXA1986
												|| PT == PXA1826
												|| PT == PXA1956 || PT == VEGA
												|| PT == BOWIE || PT == ULC2))
								|| ((dwValue == IMAPID)
										&& (PT == UNDEFINED || PT == ARMADA610
												|| PT == ARMADA622
												|| PT == PXA2128 || PT == PXA988
												|| PT == PXA1701 || PT == PXA978
												|| PT == PXA1801
												|| PT == PXA1802
												|| PT == PXA1812
												|| PT == PXA1822
												|| PT == PXA1928
												|| PT == P88PA62_70
												|| PT == P88PA62_20
												|| PT == P88PA62_10
												|| PT == PXA988 || PT == PXA1920
												|| PT == PXA2101 || PT == PXA192
												|| PT == PXA1986 || PT == PXA986
												|| PT == PXA888 || PT == PXA1088
												|| PT == PXA1908))
								|| ((dwValue == TZID)
										&& (PT == UNDEFINED || PT == ARMADA610
												|| PT == ARMADA622
												|| PT == PXA2128))
								|| ((dwValue == TZON)
										&& (PT == UNDEFINED || PT == ARMADA610
												|| PT == ARMADA622
												|| PT == PXA2128))
								|| ((dwValue == OPDIVID)
										&& (PT == UNDEFINED || PT == PXA168
												|| PT == ARMADA16x))
								|| ((dwValue == OPMODEID)
										&& (PT == UNDEFINED || PT == PXA168
												|| PT == ARMADA16x))
								|| ((dwValue == CIDPID)
										&& (PT == UNDEFINED || PT == ARMADA610
												|| PT == PXA1920
												|| PT == ARMADA622
												|| PT == PXA2128 || PT == PXA988
												|| PT == PXA1088
												|| PT == PXA1986
												|| PT == PXA1701 || PT == PXA978
												|| PT == PXA1202
												|| PT == PXA1801
												|| PT == PXA1802
												|| PT == PXA1812
												|| PT == PXA1822
												|| PT == PXA1986
												|| PT == PXA1928
												|| PT == P88PA62_70
												|| PT == P88PA62_20
												|| PT == P88PA62_10
												|| PT == PXA1U88
												|| PT == PXA1936
												|| PT == PXA1908
												|| PT == PXA1826
												|| PT == PXA1956 || PT == VEGA
												|| PT == BOWIE || PT == ULC2))
								|| ((dwValue == TZRIID)
										&& (PT == UNDEFINED || PT == ARMADA610
												|| PT == PXA1920
												|| PT == ARMADA622
												|| PT == PXA2128 || PT == PXA988
												|| PT == PXA1088
												|| PT == PXA1701 || PT == PXA978
												|| PT == PXA1202
												|| PT == PXA1801
												|| PT == PXA1802
												|| PT == PXA1812
												|| PT == PXA1822
												|| PT == PXA1986
												|| PT == PXA1928
												|| PT == P88PA62_70
												|| PT == P88PA62_20
												|| PT == P88PA62_10
												|| PT == PXA1U88
												|| PT == PXA1936
												|| PT == PXA1908
												|| PT == PXA1826
												|| PT == PXA1956 || PT == VEGA
												|| PT == BOWIE || PT == ULC2))
								|| ((((dwValue & TYPEMASK) >> 8) == DDRTYPE)
										&& (PT == UNDEFINED || PT == ARMADA610
												|| PT == PXA1920
												|| PT == ARMADA622
												|| PT == PXA2128 || PT == PXA988
												|| PT == PXA1088 || PT == PXA978
												|| PT == PXA1701
												|| PT == PXA1202
												|| PT == PXA1801
												|| PT == PXA1802
												|| PT == PXA1812
												|| PT == PXA1822
												|| PT == PXA1986
												|| PT == PXA1928
												|| PT == P88PA62_70
												|| PT == P88PA62_20
												|| PT == P88PA62_10
												|| PT == PXA1U88
												|| PT == PXA1936
												|| PT == PXA1908
												|| PT == PXA1826
												|| PT == PXA1956 || PT == VEGA
												|| PT == BOWIE || PT == ULC2))
								|| ((((dwValue & TYPEMASK) >> 8) == TZRTYPE)
										&& (PT == UNDEFINED || PT == ARMADA610
												|| PT == PXA1920
												|| PT == ARMADA622
												|| PT == PXA2128 || PT == PXA988
												|| PT == PXA1088 || PT == PXA978
												|| PT == PXA1701
												|| PT == PXA1202
												|| PT == PXA1801
												|| PT == PXA1802
												|| PT == PXA1812
												|| PT == PXA1822
												|| PT == PXA1986
												|| PT == PXA1928
												|| PT == P88PA62_70
												|| PT == P88PA62_20
												|| PT == P88PA62_10
												|| PT == PXA1U88
												|| PT == PXA1936
												|| PT == PXA1908
												|| PT == PXA1826
												|| PT == PXA1956 || PT == VEGA
												|| PT == BOWIE || PT == ULC2))
								|| ((((dwValue & TYPEMASK) >> 8) == GPPTYPE)
										&& (PT == UNDEFINED || PT == ARMADA610
												|| PT == PXA1920
												|| PT == ARMADA622
												|| PT == PXA2128 || PT == PXA988
												|| PT == PXA1088 || PT == PXA978
												|| PT == PXA1701
												|| PT == PXA1202
												|| PT == PXA1801
												|| PT == PXA1802
												|| PT == PXA1812
												|| PT == PXA1822
												|| PT == PXA1986
												|| PT == PXA1928
												|| PT == P88PA62_70
												|| PT == P88PA62_20
												|| PT == P88PA62_10
												|| PT == PXA1U88
												|| PT == PXA1936
												|| PT == PXA1908
												|| PT == PXA1826
												|| PT == PXA1956 || PT == VEGA
												|| PT == BOWIE || PT == ULC2))
								|| ((((dwValue & TYPEMASK) >> 8) == ESCSEQID_V2)
										&& (PT == UNDEFINED || PT == PXA1986))
								|| ((((dwValue & TYPEMASK) >> 8) == COREID_V2)
										&& (PT == UNDEFINED || PT == PXA1986))
								|| ((dwValue == ONDEMANDBOOTTID)
										&& (PT == UNDEFINED))
								|| ((dwValue == MV_IMAGE_PKGS_TAG)
										&& (PT == UNDEFINED))
								|| ((dwValue == MV_IMAGE_HEADER_TAG)
										&& (PT == UNDEFINED))
								|| ((dwValue == MV_CUST_KEY_PKG_TAG)
										&& (PT == UNDEFINED))
								|| ((dwValue == MV_EXT_RSA_KEY_PKG_TAG)
										&& (PT == UNDEFINED)))) {
					// save the previous package if any
					if (pPackageData) {
						m_TimDescriptor.ReservedDataList().push_back(
								pPackageData);
					}

					headerCount++;
					pPackageData = new CReservedPackageData;

					// make hex digits all upper case, ignore leading 0x
					pPackageData->PackageIdTag(HexFormattedAscii(dwValue));
					bPackageSizeNext = true;
					continue;
				}
				// ReservedArea has to contain a terminator ID if it contains a reserved area ID as 1st field.
				else if (dwValue == TERMINATORID) {
					termIdHeader = true;
					headerCount++;

					// save the previous package if any
					if (pPackageData) {
						m_TimDescriptor.ReservedDataList().push_back(
								pPackageData);
					}

					pPackageData = 0;
					break;
				} else if (pPackageData != 0) {
					// we do not actually store the size in the package
					// but use it to track if the package is fully loaded
					if (bPackageSizeNext) {
						dwPackageSize = dwValue - 8; // -8 because size includes tag and pkg len fields
						bPackageSizeNext = false;
						continue;
					} else {
						// we are adding a data field to a package
						if (dwPackageSize > 0) {
							string* psValue = new string(
									HexFormattedAscii(dwValue));
							pPackageData->AddData(psValue, new string(""));
							dwPackageSize -= 4;
							continue;
						} else {
							// save the previous package if any
							if (pPackageData) {
								m_TimDescriptor.ReservedDataList().push_back(
										pPackageData);
							}

							// finished adding data to the package
							// reinit for next package
							pPackageData = 0;
							dwPackageSize = 0;
						}
					}
				}

				// if we get here, then we are not currently processing
				// a package and we didn't recognize the dwValue as a predefined
				// package id, so we assume it is a custom package id
				// immediately followed by a package size field
				if (pPackageData == 0 && dwPackageSize == 0) {
					headerCount++;

					// unrecognized custom field tags
					pPackageData = new CReservedPackageData;
					// make hex digits all upper case, ignore leading 0x
					pPackageData->PackageIdTag(HexFormattedAscii(dwValue));
					bPackageSizeNext = true;
				}
			}
		}
	}

	assert(pPackageData == 0);

	if (dwPackageSize != 0) {
		stringstream ss;
		ss << "  Error: Package size not consistent with supplied field values."
				<< endl;
		m_TimDescriptor.ParsingError(ss.str(), true);
		bRet = false;
	}

	if (!termIdHeader && (isWtpReservedAreaId == true)) {
		stringstream ss;
		ss << "  Error: Terminator ID is incorrect or is missing." << endl;
		m_TimDescriptor.ParsingError(ss.str(), true);
		bRet = false;
	}

	if ((headerCount != numOfHeaders) && (isWtpReservedAreaId == true)) {
		stringstream ss;
		ss << "   Error: Number of Reserved Area Packages " << headerCount
				<< " does not equal the number of headers " << numOfHeaders
				<< endl;
		m_TimDescriptor.ParsingError(ss.str(), true);
		bRet = false;
	}

	if (pPackageData) {
		stringstream ss;
		ss << "   Error: Did not complete parsing of package data <"
				<< pPackageData->PackageIdTag() << "> " << endl;
		m_TimDescriptor.ParsingError(ss.str(), true);
		delete pPackageData;
		bRet = false;
	}

	return bRet;
}

bool CTimDescriptorParser::ParseImageEncryptSize(CImageDescription* pImageDesc,
		CTimDescriptorLine* pLine) {
	if (m_TimDescriptor.getTimHeader().getVersion() >= TIM_3_5_00) {
		// if EncryptSize is all 1's or larger than image, set to image size - EncryptStartOffset
		if ((pImageDesc->EncryptSize() == 0xFFFFFFFF)
				|| ((pImageDesc->EncryptSize()
						+ pImageDesc->EncryptStartOffset())
						> pImageDesc->ImageSize())) {
			if (pImageDesc->EncryptStartOffset() > pImageDesc->ImageSize()) {
				m_TimDescriptor.ParsingError(
						"Error: The EncryptStartOffset value is greater than the ImageSize!\n",
						true, pLine);
				return false;
			}

			m_TimDescriptor.ParsingWarning(
					"Warning: The EncryptSize value is adjusted to (ImageSize - EncryptStartAddress)!\n",
					true, pLine);
			// only adjusted in TBB, not in MBU GUI
			pImageDesc->EncryptSize(
					pImageDesc->ImageSize() - pImageDesc->EncryptStartOffset());
		}
	}

	return true;
}

bool CTimDescriptorParser::ParseExtendedReservedData(
		CCommandLineParser& CommandLineParser) {
	bool bRet = true;
	string sData;
	DWORD dwValue = 0;
	string sProcessorType;
	CErdBase* pErd = 0;
	string sEnd("End ");

	string lcTimTxtFilePath = CTimLib::ToLower(
			m_TimDescriptor.TimDescriptorFilePath());
	bool bIsBlf = lcTimTxtFilePath.find(".blf")
			== lcTimTxtFilePath.length() - 4;

	// create a proxy reference to the ExtendedReservedData
	CExtendedReservedData& Ext = m_TimDescriptor.ExtendedReservedData();

	CTimDescriptorLine* pLine = 0;
	// parse optional extended reserved data area if it is found
	if ((pLine = CTimDescriptor::GetLineField(
			bIsBlf ? "[Extended_Reserved_Data]" : sExtendedReservedData, false))) {
		sProcessorType = TimDescriptor().ProcessorTypeStr();
		Ext.ProcessorTypeStr(sProcessorType);

		bool bRetry = false;
		unsigned int idx = 0;
		string sPackageEndLabel;

		while ((pLine = m_TimDescriptor.GetNextLineField(pLine))) {
			idx = 0;
			bRetry = false;
			Retry: if (bRetry)
				bRetry = false;

			if (pLine->m_FieldName.length() == 0)
				continue;

			if (pLine->m_FieldName.find(
					bIsBlf ?
							SpaceToUnderscore(sEnd + sExtendedReservedData) :
							(sEnd + sExtendedReservedData)) != string::npos)
				break;

			if (pLine->m_FieldName.find(
					(bIsBlf ?
							SpaceToUnderscore("Processor_Type") :
							"Processor Type")) != string::npos) {
				if (m_TimDescriptor.ProcessorTypeStr().empty())
					m_TimDescriptor.ProcessorTypeStr(pLine->m_FieldValue);
				Ext.ProcessorTypeStr(m_TimDescriptor.ProcessorTypeStr());
				continue;
			}

			// if there is no processor type, then can't parse ERD
			if (m_TimDescriptor.ProcessorTypeStr().empty()) {
				stringstream ss;
				ss << endl << "Error: No 'Processor Type' defined. " << endl;
				//                ss << "Cannot parse Extended Reserved Data area correctly if Processor Type is not defined in descriptor text file." << endl;
				m_TimDescriptor.ParsingError(ss.str(), true, pLine);
				//                return false;
			}

			eProcessorType PT = m_TimDescriptor.ProcessorType();

			// if the processor type does not support ERD, then can't parse ERD
			if (PT == PXA30x || PT == PXA31x || PT == PXA32x) {
				stringstream ss;
				ss << endl
						<< "Extended Reserved Data area is not supported for the Processor Type defined in descriptor text file."
						<< endl;
				m_TimDescriptor.ParsingError(ss.str(), true, pLine);
				return false;
			}

			if (PT == UNDEFINED || PT == PXA168 || PT == PXA91x || PT == PXA92x
					|| PT == PXA93x || PT == PXA94x || PT == PXA95x
					|| PT == ARMADA16x || PT == ARMADA610 || PT == ARMADA622
					|| PT == PXA2128) {
				// parse Clock Enable section
				if (pLine->m_FieldName.find(
						bIsBlf ? SpaceToUnderscore(sClockEnable) : sClockEnable)
						!= string::npos) {
					sPackageEndLabel = (sEnd + sClockEnable);
					sPackageEndLabel = (
							bIsBlf ?
									SpaceToUnderscore(sPackageEndLabel) :
									sPackageEndLabel);
					ParseDDRPackage(pLine, Ext.ClockEnableFields,
							Ext.g_ClockEnableFields, idx, Ext,
							&CExtendedReservedData::AddClockEnableField,
							sClockEnable, sPackageEndLabel, bRetry);
					if (pLine == 0 || pLine->m_FieldName != sPackageEndLabel) {
						TimDescriptor().ParsingError(
								"Field Not Found: <" + sPackageEndLabel + ">",
								true, pLine);
						pLine = pPrevLine;
					}
					continue;
				}
				// parse DDR Geometry section
				else if (pLine->m_FieldName.find(
						bIsBlf ? SpaceToUnderscore(sDDRGeometry) : sDDRGeometry)
						!= string::npos) {
					sPackageEndLabel = (sEnd + sDDRGeometry);
					sPackageEndLabel = (
							bIsBlf ?
									SpaceToUnderscore(sPackageEndLabel) :
									sPackageEndLabel);
					ParseDDRPackage(pLine, Ext.DDRGeometryFields,
							Ext.g_SdramSpecFields, idx, Ext,
							&CExtendedReservedData::AddDDRGeometryField,
							sDDRGeometry, sPackageEndLabel, bRetry);
					if (pLine == 0
							|| pLine->m_FieldName
									!= (bIsBlf ?
											SpaceToUnderscore(
													sPackageEndLabel) :
											sPackageEndLabel)) {
						TimDescriptor().ParsingError(
								"Field Not Found: <" + sPackageEndLabel + ">",
								true, pLine);
						pLine = pPrevLine;
					}
					continue;
				}
				// parse DDR Timing section
				else if (pLine->m_FieldName.find(
						bIsBlf ? SpaceToUnderscore(sDDRTiming) : sDDRTiming)
						!= string::npos) {
					sPackageEndLabel = (sEnd + sDDRTiming);
					sPackageEndLabel = (
							bIsBlf ?
									SpaceToUnderscore(sPackageEndLabel) :
									sPackageEndLabel);
					ParseDDRPackage(pLine, Ext.DDRTimingFields,
							Ext.g_SdramSpecFields, idx, Ext,
							&CExtendedReservedData::AddDDRTimingField,
							sDDRTiming, sPackageEndLabel, bRetry);
					if (pLine == 0 || pLine->m_FieldName != sPackageEndLabel) {
						TimDescriptor().ParsingError(
								"Field Not Found: <" + sPackageEndLabel + ">",
								true, pLine);
						pLine = pPrevLine;
					}
					continue;
				}
				// parse DDR Custom section
				else if (pLine->m_FieldName.find(
						bIsBlf ? SpaceToUnderscore(sDDRCustom) : sDDRCustom)
						!= string::npos) {
					sPackageEndLabel = (sEnd + sDDRCustom);
					sPackageEndLabel = (
							bIsBlf ?
									SpaceToUnderscore(sPackageEndLabel) :
									sPackageEndLabel);
					ParseDDRPackage(pLine, Ext.DDRCustomFields,
							Ext.g_DDRCustomFields, idx, Ext,
							&CExtendedReservedData::AddDDRCustomField,
							sDDRCustom, sPackageEndLabel, bRetry);
					if (pLine == 0 || pLine->m_FieldName != sPackageEndLabel) {
						TimDescriptor().ParsingError(
								"Field Not Found: <" + sPackageEndLabel + ">",
								true, pLine);
						pLine = pPrevLine;
					}
					continue;
				}
				// parse Frequency section
				else if (pLine->m_FieldName.find(
						bIsBlf ? SpaceToUnderscore(sFrequency) : sFrequency)
						!= string::npos) {
					sPackageEndLabel = (sEnd + sFrequency);
					sPackageEndLabel = (
							bIsBlf ?
									SpaceToUnderscore(sPackageEndLabel) :
									sPackageEndLabel);
					ParseDDRPackage(pLine, Ext.FrequencyFields,
							Ext.g_FrequencyFields, idx, Ext,
							&CExtendedReservedData::AddFrequencyField,
							sFrequency, sPackageEndLabel, bRetry);
					if (pLine == 0 || pLine->m_FieldName != sPackageEndLabel) {
						TimDescriptor().ParsingError(
								"Field Not Found: <" + sPackageEndLabel + ">",
								true, pLine);
						pLine = pPrevLine;
					}
					continue;
				}
				// parse Voltages section
				else if (pLine->m_FieldName.find(
						bIsBlf ? SpaceToUnderscore(sVoltages) : sVoltages)
						!= string::npos) {
					sPackageEndLabel = (sEnd + sVoltages);
					sPackageEndLabel = (
							bIsBlf ?
									SpaceToUnderscore(sPackageEndLabel) :
									sPackageEndLabel);
					ParseDDRPackage(pLine, Ext.VoltagesFields,
							Ext.g_VoltagesFields, idx, Ext,
							&CExtendedReservedData::AddVoltagesField, sVoltages,
							sPackageEndLabel, bRetry);
					if (pLine == 0 || pLine->m_FieldName != sPackageEndLabel) {
						TimDescriptor().ParsingError(
								"Field Not Found: <" + sPackageEndLabel + ">",
								true, pLine);
						pLine = pPrevLine;
					}
					continue;
				}
				// parse CCMC section
				else if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(sConfigMemoryControl) :
								sConfigMemoryControl) != string::npos) {
					sPackageEndLabel = (sEnd + sConfigMemoryControl);
					sPackageEndLabel = (
							bIsBlf ?
									SpaceToUnderscore(sPackageEndLabel) :
									sPackageEndLabel);
					ParseDDRPackage(pLine, Ext.ConfigMemoryControlFields,
							Ext.g_ConfigMemoryControlFields, idx, Ext,
							&CExtendedReservedData::AddConfigMemoryControlField,
							sConfigMemoryControl, sPackageEndLabel, bRetry);
					if (pLine == 0 || pLine->m_FieldName != sPackageEndLabel) {
						TimDescriptor().ParsingError(
								"Field Not Found: <" + sPackageEndLabel + ">",
								true, pLine);
						pLine = pPrevLine;
					}
					continue;
				}
			}

			if (PT == UNDEFINED || PT == ARMADA610 || PT == ARMADA622
					|| PT == PXA2128) {
				// parse Trust Zone Regid section
				if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(sTrustZoneRegid) :
								sTrustZoneRegid) != string::npos) {
					sPackageEndLabel = (sEnd + sTrustZoneRegid);
					sPackageEndLabel = (
							bIsBlf ?
									SpaceToUnderscore(sPackageEndLabel) :
									sPackageEndLabel);
					ParseDDRPackage(pLine, Ext.TrustZoneRegidFields,
							Ext.g_TrustZoneRegidFields, idx, Ext,
							&CExtendedReservedData::AddTrustZoneRegidField,
							sTrustZoneRegid, sPackageEndLabel, bRetry);
					if (pLine == 0 || pLine->m_FieldName != sPackageEndLabel) {
						TimDescriptor().ParsingError(
								"Field Not Found: <" + sPackageEndLabel + ">",
								true, pLine);
						pLine = pPrevLine;
					}
					continue;
				}
				// parse Trust Zone section
				else if (pLine->m_FieldName.find(
						bIsBlf ? SpaceToUnderscore(sTrustZone) : sTrustZone)
						!= string::npos) {
					sPackageEndLabel = (sEnd + sTrustZone);
					sPackageEndLabel = (
							bIsBlf ?
									SpaceToUnderscore(sPackageEndLabel) :
									sPackageEndLabel);
					ParseDDRPackage(pLine, Ext.TrustZoneFields,
							Ext.g_TrustZoneFields, idx, Ext,
							&CExtendedReservedData::AddTrustZoneField,
							sTrustZone, sPackageEndLabel, bRetry);
					if (pLine == 0 || pLine->m_FieldName != sPackageEndLabel) {
						TimDescriptor().ParsingError(
								"Field Not Found: <" + sPackageEndLabel + ">",
								true, pLine);
						pLine = pPrevLine;
					}
					continue;
				}
			}

			if (PT == UNDEFINED || PT == PXA168 || PT == ARMADA16x) {
				// parse OpDiv section
				if (pLine->m_FieldName.find(
						bIsBlf ? SpaceToUnderscore(sOpDiv) : sOpDiv)
						!= string::npos) {
					sPackageEndLabel = (sEnd + sOpDiv);
					sPackageEndLabel = (
							bIsBlf ?
									SpaceToUnderscore(sPackageEndLabel) :
									sPackageEndLabel);
					ParseDDRPackage(pLine, Ext.OpDivFields, Ext.g_OpDivFields,
							idx, Ext, &CExtendedReservedData::AddOpDivField,
							sOpDiv, sPackageEndLabel, bRetry);
					if (pLine == 0 || pLine->m_FieldName != sPackageEndLabel) {
						TimDescriptor().ParsingError(
								"Field Not Found: <" + sPackageEndLabel + ">",
								true, pLine);
						pLine = pPrevLine;
					}
					continue;
				}
				// parse OpMode section
				else if (pLine->m_FieldName.find(
						bIsBlf ? SpaceToUnderscore(sOpMode) : sOpMode)
						!= string::npos) {
					sPackageEndLabel = (sEnd + sOpMode);
					sPackageEndLabel = (
							bIsBlf ?
									SpaceToUnderscore(sPackageEndLabel) :
									sPackageEndLabel);
					ParseDDRPackage(pLine, Ext.OpModeFields, Ext.g_OpModeFields,
							idx, Ext, &CExtendedReservedData::AddOpModeField,
							sOpMode, sPackageEndLabel, bRetry);
					if (pLine == 0 || pLine->m_FieldName != sPackageEndLabel) {
						TimDescriptor().ParsingError(
								"Field Not Found: <" + sPackageEndLabel + ">",
								true, pLine);
						pLine = pPrevLine;
					}
					continue;
				}
			}

			if (pErd == 0
					&& (PT == UNDEFINED || PT == ARMADA610 || PT == ARMADA622
							|| PT == PXA2128 || PT == PXA988 || PT == P88PA62_70
							|| PT == P88PA62_20 || PT == P88PA62_10
							|| PT == PXA1928 || PT == PXA1U88 || PT == PXA1936
							|| PT == PXA1920 || PT == PXA2101 || PT == PXA192
							|| PT == PXA1986 || PT == PXA1826 || PT == PXA1956
							|| PT == VEGA || PT == BOWIE || PT == ULC2)) {
				if (pLine->m_FieldName.find(
						bIsBlf ? SpaceToUnderscore(CPin::Begin) : CPin::Begin)
						!= string::npos) {
					pErd = new CPin;
					ParseERDPackage(pLine, pErd, CPin::Begin, CPin::End,
							bIsBlf);
				}
			}

			if (pErd == 0
					&& (PT == UNDEFINED || PT == ARMADA610 || PT == ARMADA622
							|| PT == PXA2128 || PT == PXA988 || PT == PXA1701
							|| PT == PXA978 || PT == P88PA62_70
							|| PT == P88PA62_20 || PT == P88PA62_10
							|| PT == PXA1928 || PT == PXA1U88 || PT == PXA1936
							|| PT == PXA988 || PT == PXA1920 || PT == PXA2101
							|| PT == PXA192 || PT == PXA1986 || PT == PXA986
							|| PT == PXA888 || PT == PXA1088 || PT == PXA1801
							|| PT == PXA1802 || PT == PXA1812 || PT == PXA1822
							|| PT == PXA1908 || PT == PXA1826 || PT == PXA1956
							|| PT == VEGA || PT == BOWIE || PT == ULC2)) {
				if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CImageMaps::Begin) :
								CImageMaps::Begin) != string::npos) {
					pErd = new CImageMaps(bIsBlf);
					ParseERDPackage(pLine, pErd, CImageMaps::Begin,
							CImageMaps::End, bIsBlf);
				}
			}

			if (pErd == 0) {
				if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CGpioSet::Begin) :
								CGpioSet::Begin) != string::npos) {
					pErd = new CGpioSet;
					ParseERDPackage(pLine, pErd, CGpioSet::Begin, CGpioSet::End,
							bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CUsbVendorReq::Begin) :
								CUsbVendorReq::Begin) != string::npos) {
					pErd = new CUsbVendorReq;
					ParseERDPackage(pLine, pErd, CUsbVendorReq::Begin,
							CUsbVendorReq::End, bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ? SpaceToUnderscore(CUsb::Begin) : CUsb::Begin)
						!= string::npos) {
					pErd = new CUsb;
					ParseERDPackage(pLine, pErd, CUsb::Begin, CUsb::End,
							bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ? SpaceToUnderscore(CUart::Begin) : CUart::Begin)
						!= string::npos) {
					pErd = new CUart;
					ParseERDPackage(pLine, pErd, CUart::Begin, CUart::End,
							bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CAutoBind::Begin) :
								CAutoBind::Begin) != string::npos) {
					pErd = new CAutoBind;
					ParseERDPackage(pLine, pErd, CAutoBind::Begin,
							CAutoBind::End, bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CResumeDdr::Begin) :
								CResumeDdr::Begin) != string::npos) {
					pErd = new CResumeDdr;
					ParseERDPackage(pLine, pErd, CResumeDdr::Begin,
							CResumeDdr::End, bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CROMResume::Begin) :
								CROMResume::Begin) != string::npos) {
					pErd = new CROMResume;
					ParseERDPackage(pLine, pErd, CROMResume::Begin,
							CROMResume::End, bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CTBRXferSet::Begin) :
								CTBRXferSet::Begin) != string::npos) {
					pErd = new CTBRXferSet;
					ParseERDPackage(pLine, pErd, CTBRXferSet::Begin,
							CTBRXferSet::End, bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CEscapeSeq::Begin) :
								CEscapeSeq::Begin) != string::npos) {
					pErd = new CEscapeSeq;
					ParseERDPackage(pLine, pErd, CEscapeSeq::Begin,
							CEscapeSeq::End, bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CCoreReset::Begin) :
								CCoreReset::Begin) != string::npos) {
					pErd = new CCoreReset;
					ParseERDPackage(pLine, pErd, CCoreReset::Begin,
							CCoreReset::End, bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ? SpaceToUnderscore(CBbmt::Begin) : CBbmt::Begin)
						!= string::npos) {
					pErd = new CBbmt;
					ParseERDPackage(pLine, pErd, CBbmt::Begin, CBbmt::End,
							bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CDDRType::Begin) :
								CDDRType::Begin) != string::npos) {
					pErd = new CDDRType;
					ParseERDPackage(pLine, pErd, CDDRType::Begin, CDDRType::End,
							bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CFlashGeometryReq::Begin) :
								CFlashGeometryReq::Begin) != string::npos) {
					pErd = new CFlashGeometryReq;
					ParseERDPackage(pLine, pErd, CFlashGeometryReq::Begin,
							CFlashGeometryReq::End, bIsBlf);
				}
			}

			if (pErd) {
				// save the object
				Ext.ErdVec.push_back(pErd);
				// clear temp ptr
				pErd = 0;

				if (bRetry)
					goto Retry;

				continue;
			}

			if (PT == UNDEFINED || PT == ARMADA610 || PT == ARMADA622
					|| PT == PXA2128 || PT == PXA988 || PT == PXA1088
					|| PT == PXA1986 || PT == PXA1701 || PT == PXA1202
					|| PT == PXA978 || PT == P88PA62_70 || PT == P88PA62_20
					|| PT == P88PA62_10 || PT == PXA1928 || PT == PXA1U88
					|| PT == PXA1936 || PT == PXA92x || PT == PXA1920
					|| PT == PXA1801 || PT == PXA1802 || PT == PXA1812
					|| PT == PXA1822 || PT == PXA1908 || PT == PXA1826
					|| PT == PXA1956 || PT == VEGA || PT == BOWIE
					|| PT == ULC2) {
				if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CConsumerID::Begin) :
								CConsumerID::Begin) != string::npos) {
					CConsumerID* pConsumer = new CConsumerID;
					pErd = pConsumer;
					ParseERDPackage(pLine, pErd, CConsumerID::Begin,
							CConsumerID::End, bIsBlf);
					// save the object
					Ext.m_Consumers.push_back(pConsumer);
					// clear temp ptr
					pErd = 0;
					continue;
				} else if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CDDRInitialization::Begin) :
								CDDRInitialization::Begin) != string::npos) {
					pErd = new CDDRInitialization;
					ParseERDPackage(pLine, pErd, CDDRInitialization::Begin,
							CDDRInitialization::End, bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CInstructions::Begin) :
								CInstructions::Begin) != string::npos) {
					pErd = new CInstructions;
					ParseERDPackage(pLine, pErd, CInstructions::Begin,
							CInstructions::End, bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CDDROperations::Begin) :
								CDDROperations::Begin) != string::npos) {
					pErd = new CDDROperations;
					ParseERDPackage(pLine, pErd, CDDROperations::Begin,
							CDDROperations::End, bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CTzInitialization::Begin) :
								CTzInitialization::Begin) != string::npos) {
					pErd = new CTzInitialization;
					ParseERDPackage(pLine, pErd, CTzInitialization::Begin,
							CTzInitialization::End, bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CTzOperations::Begin) :
								CTzOperations::Begin) != string::npos) {
					pErd = new CTzOperations;
					ParseERDPackage(pLine, pErd, CTzOperations::Begin,
							CTzOperations::End, bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CGenPurposePatch::Begin) :
								CGenPurposePatch::Begin) != string::npos) {
					pErd = new CGenPurposePatch;
					ParseERDPackage(pLine, pErd, CGenPurposePatch::Begin,
							CGenPurposePatch::End, bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CGPPOperations::BlfBegin) :
								CGPPOperations::Begin) != string::npos) {
					pErd = new CGPPOperations;
					ParseERDPackage(pLine, pErd, CGPPOperations::Begin,
							CGPPOperations::End, bIsBlf);
				}
			}

			if (pErd == 0 && (PT == UNDEFINED || PT == PXA1986)) {
				if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CCoreRelease::Begin) :
								CCoreRelease::Begin) != string::npos) {
					pErd = new CCoreRelease;
					ParseERDPackage(pLine, pErd, CCoreRelease::Begin,
							CCoreRelease::End, bIsBlf);
				} else if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(CEscapeSeqV2::Begin) :
								CEscapeSeqV2::Begin) != string::npos) {
					pErd = new CEscapeSeqV2;
					ParseERDPackage(pLine, pErd, CEscapeSeqV2::Begin,
							CEscapeSeqV2::End, bIsBlf);
				}
			}

			if (pErd == 0 && (PT == UNDEFINED
			// processor type for this package is not yet determined
					)) {
				if (pLine->m_FieldName.find(
						bIsBlf ?
								SpaceToUnderscore(COnDemandBoot::Begin) :
								COnDemandBoot::Begin) != string::npos) {
					pErd = new COnDemandBoot;
					ParseERDPackage(pLine, pErd, COnDemandBoot::Begin,
							COnDemandBoot::End, bIsBlf);
				}

			}

			if (pErd != 0) {
				Ext.ErdVec.push_back(pErd);
				// clear temp ptr
				pErd = 0;
			} else {
				stringstream ss;
				ss << endl
						<< "Error: Unrecognized field in Extended Reserved Data."
						<< endl;
				m_TimDescriptor.ParsingError(ss.str(), true, pLine);
				bRet = false;
			}

			if (bRetry)
				goto Retry;
		}

		if (pLine == 0) {
			stringstream ss;
			ss << endl << "Error: Failed to find End Extended Reserved Data:"
					<< endl;
			m_TimDescriptor.ParsingError(ss.str(), true, pLine);
		}

	}

	int nBytesAdded = Ext.Size();
	if (nBytesAdded > 0) {
		// fix tim header sizes
		if (m_TimDescriptor.getTimHeader().getSizeOfReserved() == 0)
			nBytesAdded += 16;  // + header & terminator pkgs

		m_TimDescriptor.getTimHeader().setSizeOfReserved(
				m_TimDescriptor.getTimHeader().getSizeOfReserved()
						+ nBytesAdded);
	}
	if (m_TimDescriptor.ImagesList().size() > 0) {
		CImageDescription* pImage = (*(m_TimDescriptor.ImagesList().begin()));
		if (pImage > 0
				&& ((Translate(pImage->ImageIdTag()) & TYPEMASK)
						== (TIMIDENTIFIER & TYPEMASK))) {
			if (pImage->ImageSizeToHash() == pImage->ImageSize())
				pImage->ImageSizeToHash(
						pImage->ImageSizeToHash() + nBytesAdded);

			pImage->ImageSize(
					m_TimDescriptor.GetTimImageSize(
							CommandLineParser.bOneNANDPadding,
							CommandLineParser.uiPaddedSize));

		}
	}

	return bRet;
}

bool CTimDescriptorParser::ParseDDRPackage(CTimDescriptorLine*& pLine,
		t_PairList& Fields, t_stringVector& g_FieldsNames, unsigned int& idx,
		CExtendedReservedData& Ext,
		void (CExtendedReservedData::*AddField)(
				std::pair<unsigned int, unsigned int>*&),
		const string& sPackage, string& sEndField, bool& bRetry) {
	bRetry = false;
	while ((pLine = m_TimDescriptor.GetNextLineField(pLine))) {
		if (pLine) {
			if (pLine->m_FieldName.length() == 0) {
				pPrevLine = pLine;
				continue;
			}

			if (ValidFieldIdx(pLine->m_FieldName, idx, g_FieldsNames)) {
				pair<unsigned int, unsigned int>* pPair = new pair<unsigned int,
						unsigned int>(idx, Translate(pLine->m_FieldValue));
				(Ext.*AddField)(pPair);
			} else {
				if (pLine->m_FieldName.size() == 0) {
					stringstream ss;
					ss
							<< "Error: Cannot parse field name, may be missing ':', <"
							<< pLine->m_FieldValue.c_str() << ">";
					ss << "in <" << sPackage << "> package" << endl;
					m_TimDescriptor.ParsingError(ss.str(), true, pLine);

					if (!bRetry) {
						pPrevLine = pLine;
						continue;
					}
				}

				if (pLine->m_FieldName != sEndField) {
					stringstream ss;
					ss << "Error: Found unrecognized field <"
							<< pLine->m_FieldName.c_str() << "> ";
					ss << "in <" << sPackage << "> package" << endl;
					m_TimDescriptor.ParsingError(ss.str(), true, pLine);

					if (!bRetry) {
						pPrevLine = pLine;
						continue;
					}
				}
				break;

				pPrevLine = pLine;
			}
		}
	}
	return bRetry;
}

bool CTimDescriptorParser::ParseERDPackage(CTimDescriptorLine*& pLine,
		CErdBase*& pErd, const string& sBegin, const string& sEnd,
		bool bIsBlf) {
	if ((!pLine) || !pErd->Parse(m_TimDescriptor, pLine, bIsBlf)
			|| (pLine->m_FieldName.find(bIsBlf ? SpaceToUnderscore(sEnd) : sEnd)
					== string::npos)) {
		stringstream ss;
		ss << endl << "Error: Parsing of "
				<< (bIsBlf ? SpaceToUnderscore(sBegin) : sBegin);
		m_TimDescriptor.ParsingError(ss.str(), true, pLine);
		return false;
	}

	return true;
}

bool CTimDescriptorParser::SearchFieldValue(string& sLine, unsigned int& idx,
		t_stringVector& Fields, DWORD& dwValue) {
	bool bFound = false;
	for (idx = 0; !bFound && (idx < Fields.size()); idx++) {
		// find a valid field and get the value
		if (GetFieldValue(sLine, *Fields[idx], dwValue)) {
			bFound = true;
			sLine = "";
			break;
		}
	}

	return bFound;
}

bool CTimDescriptorParser::ValidFieldIdx(string& sFieldName, unsigned int& idx,
		t_stringVector& Fields) {
	bool bFound = false;
	for (idx = 0; !bFound && (idx < Fields.size()); idx++) {
		// find a valid field and get the value
		if (sFieldName == *Fields[idx]) {
			bFound = true;
			break;
		}
	}

	return bFound;
}

bool CTimDescriptorParser::FillList(CTimDescriptorLine*& pLine,
		t_stringList& List, unsigned int iSize) {
	unsigned int iTotal = 0;
	string sLine;
	string sData;
	size_t nBeg = 0;
	size_t nEnd = 0;
	CTimDescriptorLine* pPrevLine = pLine;
	bool bRet = true;

	// This function grabs one line at a time and parses out tokens that
	// are delimited by spaces, tabs or carriage returns. This process will
	// continue until it reads a line containing a semicolon or it reaches
	// the end of the file. The semicolon signifies a new section of the
	// descriptor text file.

	t_stringListIter iter = List.begin();

	while (iter != List.end()) {
		// iTotal is number of ints in array
		// normalize iSize(bytes) to the min # of ints required to hold iSize bytes
		// some bytes may not be used, i.e. ECDSA_521
		if ((iTotal * 4) > (((iSize + 3) / 4) * 4)) {
			m_TimDescriptor.ParsingError(
					"  Error: Key file parsing error reading, Data bytes read > max allowable\n",
					true);
			bRet = false;
		}

		// fail if the line has a field, but load it if it is just values

		if ((pLine = m_TimDescriptor.GetNextLineField(pLine, false)) == 0)
			break;

		// done with list of values
		if (!pLine->m_FieldName.empty()) {
			pLine = pPrevLine;
			return bRet;
		}
		pPrevLine = pLine;

		sLine = pLine->m_FieldValue;

		if (sLine.length() > 0) {
			nBeg = 0;
			nEnd = 0;
			// parse one or more values per line, separated by white space
			do {
				if (string::npos == (nEnd = sLine.find_first_of(" \n\t", nBeg)))
					sData = sLine.substr(nBeg);
				else
					sData = sLine.substr(nBeg, nEnd - nBeg);

				if (sData.length() == 0)
					// skip white space until next data
					nBeg++;
				else {
					*(*iter) = sData;
					iter++;
					nBeg += (nEnd - nBeg) + 1;
					iTotal++;
				}
			} while (nEnd != string::npos && iter != List.end());
		}
	}

	// iTotal is number of ints in array
	// normalize iSize(bytes) to the min # of ints required to hold iSize bytes
	// some bytes may not be used, i.e. ECDSA_521
	if ((iTotal * 4) > (((iSize + 3) / 4) * 4)) {
		stringstream ss;
		ss << "Error: Data bytes read = " << iTotal * 4
				<< " is > than max allowable " << (((iSize + 3) / 4) * 4)
				<< endl;
		m_TimDescriptor.ParsingError(ss.str(), true);
		bRet = false;
	}

	return bRet;
}

bool CTimDescriptorParser::FillListBlf(CTimDescriptorLine*& pLine,
		t_stringList& List, unsigned int iSize) {
	unsigned int iTotal = 0;
	string sLine;
	string sData;
	size_t nBeg = 0;
	size_t nEnd = 0;
	CTimDescriptorLine* pPrevLine = pLine;
	bool bRet = true;

	// This function grabs one line at a time and parses out tokens that
	// are delimited by spaces, tabs or carriage returns. This process will
	// continue until it reads a line containing a semicolon or it reaches
	// the end of the file. The semicolon signifies a new section of the
	// descriptor text file.

	t_stringListIter iter = List.begin();

	stringstream ssKeyEntryNum;
	int nEntryNum = 0;

	while (iter != List.end()) {
		// iTotal is number of ints in array
		// normalize iSize(bytes) to the min # of ints required to hold iSize bytes
		// some bytes may not be used, i.e. ECDSA_521
		if ((iTotal * 4) > (((iSize + 3) / 4) * 4)) {
			m_TimDescriptor.ParsingError(
					"  Error: Key file parsing error reading, Data bytes read > max allowable\n",
					true);
			bRet = false;
		}

		// fail if the line has a field, but load it if it is just values

		if ((pLine = m_TimDescriptor.GetNextLineField(pLine, false)) == 0)
			break;

		nEntryNum++;
		ssKeyEntryNum.str("");
		ssKeyEntryNum << "#" << nEntryNum;

		// done with list of values
		if (pLine->m_FieldName != ssKeyEntryNum.str()) {
			pLine = pPrevLine;
			return bRet;
		}
		pPrevLine = pLine;

		sLine = pLine->m_FieldValue;

		if (sLine.length() > 0) {
			nBeg = 0;
			nEnd = 0;
			// parse one or more values per line, separated by white space
			do {
				if (string::npos == (nEnd = sLine.find_first_of(" \n\t", nBeg)))
					sData = sLine.substr(nBeg);
				else
					sData = sLine.substr(nBeg, nEnd - nBeg);

				if (sData.length() == 0)
					// skip white space until next data
					nBeg++;
				else {
					*(*iter) = sData;
					iter++;
					nBeg += (nEnd - nBeg) + 1;
					iTotal++;
				}
			} while (nEnd != string::npos && iter != List.end());
		}
	}

	// iTotal is number of ints in array
	// normalize iSize(bytes) to the min # of ints required to hold iSize bytes
	// some bytes may not be used, i.e. ECDSA_521
	if ((iTotal * 4) > (((iSize + 3) / 4) * 4)) {
		stringstream ss;
		ss << "Error: Data bytes read = " << iTotal * 4
				<< " is > than max allowable " << (((iSize + 3) / 4) * 4)
				<< endl;
		m_TimDescriptor.ParsingError(ss.str(), true);
		bRet = false;
	}

	return bRet;
}
bool CTimDescriptorParser::FillList(ifstream& ifs, t_stringList& List,
		unsigned int iSize) {
	DWORD ImageOffset = 0;
	unsigned int iTotal = 0;
	string sLine;
	string sData;
	size_t nBeg = 0;
	size_t nEnd = 0;
	bool bRet = true;

	t_stringListIter iter = List.begin();

	// This function grabs one line at a time and parses out tokens that
	// are delimited by spaces, tabs or carriage returns. This process will
	// continue until it reads a line containing a semicolon or it reaches
	// the end of the file. The semicolon signifies a new section of the
	// descriptor text file.

	while (ifs.good() && (iTotal < List.size())) {
		while (ifs.good() && GetNextLine(ifs, sLine)) {
			if (ifs.eof())
				break;

			// if 1st char (after leading white space) is a ;,
			//  then line is a comment so skip
			if (sLine[0] != ';')
				break;
		}

		if (sLine.length() > 0) {
			// if there is a ":" then there are no more values to parse
			if (sLine.find(":") != string::npos) {
				ifs.seekg(ImageOffset, ios::beg);
				break;
			}

			nBeg = 0;
			nEnd = 0;
			// parse one or more values per line, separated by white space
			do {
				if (string::npos == (nEnd = sLine.find_first_of(" \n\t", nBeg)))
					sData = sLine.substr(nBeg);
				else
					sData = sLine.substr(nBeg, nEnd - nBeg);

				if (sData.length() == 0)
					// skip white space until next data
					nBeg++;
				else {
					*(*iter) = sData;
					iter++;
					nBeg += (nEnd - nBeg) + 1;
					iTotal++;
				}
			} while (iter != List.end() && nEnd != string::npos);
		}

		// note that the (DWORD) cast is dangerous for very large files but should not be an issue here
		ImageOffset = (DWORD) ifs.tellg();

		if (ifs.eof())
			break;

		if (ifs.bad() || ifs.fail())
			return false;
	}

	// iTotal is number of ints in array
	// normalize iSize(bytes) to the min # of ints required to hold iSize bytes
	// some bytes may not be used, i.e. ECDSA_521
	if ((iTotal * 4) > (((iSize + 3) / 4) * 4)) {
		stringstream ss;
		ss << "Error: Data bytes read = " << iTotal * 4
				<< " is > than max allowable " << (((iSize + 3) / 4) * 4)
				<< endl;
		m_TimDescriptor.ParsingError(ss.str(), true);
		bRet = false;
	}

	return bRet;

}

