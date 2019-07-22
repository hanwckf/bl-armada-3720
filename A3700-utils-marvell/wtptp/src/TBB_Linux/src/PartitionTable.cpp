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

#include "PartitionTable.h"
#include "Partition.h"
#include "TimLib.h"
#include "TimDescriptorParser.h"

#include <cstring>
#include <cstdlib>

CPartitionTable::CPartitionTable(void) :
		CTimLib() {
	m_pPartitionInfo = 0;
	Reset();
}

CPartitionTable::~CPartitionTable(void) {
	DiscardAll();
}

void CPartitionTable::Reset() {
	m_PartitionTableVersion = 0x31303031;
	m_PartitionFilePath = "";
	m_bChanged = false;
	m_bNotWritten = false;

	DiscardAll();
}

void CPartitionTable::DiscardAll() {
	memset(&m_PartitionHeader, 0, sizeof(PartitionTable_T));

	if (m_Partitions.size() > 0) {
		for_each(m_Partitions.begin(), m_Partitions.end(),
				[](CPartitionData*& pPartition) {delete pPartition;});
		m_Partitions.clear();
	}

	if (m_pPartitionInfo) {
		free(m_pPartitionInfo);
		m_pPartitionInfo = 0;
	}

}

CPartitionTable::CPartitionTable(const CPartitionTable& rhs) :
		CTimLib() {
	// copy constructor
	m_PartitionFilePath = rhs.m_PartitionFilePath;
	m_PartitionTableVersion = rhs.m_PartitionTableVersion;
	m_bChanged = rhs.m_bChanged;
	m_bNotWritten = rhs.m_bNotWritten;

	// need to do a deep copy of lists to avoid dangling references
	CPartitionTable& nc_rhs = const_cast<CPartitionTable&>(rhs);
	t_PartitionListIter iterPartition = nc_rhs.m_Partitions.begin();
	while (iterPartition != nc_rhs.m_Partitions.end()) {
		m_Partitions.push_back(new CPartitionData(*(*iterPartition)));
		iterPartition++;
	}
	m_pPartitionInfo = rhs.m_pPartitionInfo;
}

CPartitionTable& CPartitionTable::operator=(const CPartitionTable& rhs) {
	// assignment operator
	if (&rhs != this) {
		// delete the existing list and recreate a new one
		DiscardAll();

		m_PartitionFilePath = rhs.m_PartitionFilePath;
		m_PartitionTableVersion = rhs.m_PartitionTableVersion;
		m_bChanged = rhs.m_bChanged;
		m_bNotWritten = rhs.m_bNotWritten;

		// need to do a deep copy of lists to avoid dangling references
		CPartitionTable& nc_rhs = const_cast<CPartitionTable&>(rhs);
		t_PartitionListIter iterPartition = nc_rhs.m_Partitions.begin();
		while (iterPartition != nc_rhs.m_Partitions.end()) {
			m_Partitions.push_back(new CPartitionData(*(*iterPartition)));
			iterPartition++;
		}
	}
	return *this;
}

string CPartitionTable::GetText() {
	CTimLib TimLib;
	stringstream ss;
	ss << "Version: " << TimLib.HexFormattedAscii(m_PartitionTableVersion)
			<< endl;
	ss << "Number of Partitions: " << m_Partitions.size() << endl << endl;

	t_PartitionListIter iterPartition = m_Partitions.begin();
	while (iterPartition != m_Partitions.end()) {
		ss << "ID: "
				<< TimLib.HexFormattedAscii((*iterPartition)->PartitionId())
				<< endl;
		ss << "Usage: " << (*iterPartition)->PartitionUsage() << endl;
		ss << "Type: " << (*iterPartition)->PartitionType() << endl;
		ss << "Attributes: "
				<< TimLib.HexFormattedAscii(
						(*iterPartition)->PartitionAttributes()) << endl;
		ss << "Start Address: "
				<< TimLib.HexFormattedAscii64(
						(*iterPartition)->PartitionStartAddress()) << endl;
		ss << "End Address: "
				<< TimLib.HexFormattedAscii64(
						(*iterPartition)->PartitionEndAddress()) << endl;
		ss << "RP Start Address: "
				<< TimLib.HexFormattedAscii64(
						(*iterPartition)->ReservedPoolStartAddress()) << endl;
		ss << "RP Size: "
				<< TimLib.HexFormattedAscii(
						(*iterPartition)->ReservedPoolSize()) << endl;
		ss << "RP Algorithm: " << (*iterPartition)->ReservedPoolAlgorithm()
				<< endl;
		ss << "Runtime BBT Type: " << (*iterPartition)->RuntimeBBTType()
				<< endl;
		ss << "Runtime BBT Location: "
				<< TimLib.HexFormattedAscii(
						(*iterPartition)->RuntimeBBTLocation()) << endl;
		ss << "Backup BBT Location: "
				<< TimLib.HexFormattedAscii(
						(*iterPartition)->BackupRuntimeBBTLocation()) << endl;
		ss << endl;
		iterPartition++;
	}

	return ss.str();
}

bool CPartitionTable::ParsePartition() {
	if (m_PartitionFilePath.size() == 0) {
		printf("\n  Error: No Partition File Path to open!\n");
		return false;
	}

	DiscardAll();

	ifstream ifsPartitionFile;
	memset(&m_PartitionHeader, 0, sizeof(PartitionTable_T));

	ifsPartitionFile.open(m_PartitionFilePath.c_str(), ios_base::in);
	if (ifsPartitionFile.fail() || ifsPartitionFile.bad()) {
		printf("\n  Error: Cannot open file name <%s>.\n",
				m_PartitionFilePath.c_str());
		return false;
	}

	CTimLib TimLib;

	if (ParsePartitionTextFile(ifsPartitionFile, m_PartitionHeader,
			m_pPartitionInfo)) {
		m_PartitionTableVersion = m_PartitionHeader.Version;

		for (unsigned int i = 0; i < m_PartitionHeader.NumPartitions; i++) {
			if (m_pPartitionInfo != 0) {
				CPartitionData* pPartition = new CPartitionData;
				pPartition->PartitionId(m_pPartitionInfo[i].Indentifier);
				pPartition->PartitionUsage(
						TimLib.HexAsciiToText(
								TimLib.HexFormattedAscii(
										m_pPartitionInfo[i].Usage)));
				pPartition->PartitionType(
						TimLib.HexAsciiToText(
								TimLib.HexFormattedAscii(
										m_pPartitionInfo[i].Type)));

				// need some way to convert PartAttributes_T to unsigned int
				// when standard casts do not compile
				union {
					PartAttributes_T BitFields;
					unsigned int uAttributes;
				} Attribs;

				Attribs.BitFields = m_pPartitionInfo[i].Attributes;
				pPartition->PartitionAttributes(Attribs.uAttributes);

				pPartition->PartitionStartAddress(
						m_pPartitionInfo[i].StartAddr);
				pPartition->PartitionEndAddress(m_pPartitionInfo[i].EndAddr);
				pPartition->ReservedPoolStartAddress(
						m_pPartitionInfo[i].ReserveStartAddr);
				pPartition->ReservedPoolSize(m_pPartitionInfo[i].ReserveSize);
				pPartition->ReservedPoolAlgorithm(
						TimLib.HexAsciiToText(
								TimLib.HexFormattedAscii(
										m_pPartitionInfo[i].ReserveAlgorithm)));
				pPartition->RuntimeBBTType(
						TimLib.HexAsciiToText(
								TimLib.HexFormattedAscii(
										m_pPartitionInfo[i].BBT_Type)));
				pPartition->RuntimeBBTLocation(
						m_pPartitionInfo[i].RBBT_Location);
				pPartition->BackupRuntimeBBTLocation(
						m_pPartitionInfo[i].BackupRBBT_Location);

				Partitions().push_back(pPartition);
			}
		}
	}
	ifsPartitionFile.close();

	return true;
}

bool CPartitionTable::IsChanged() {
	if (m_bChanged == true)
		return m_bChanged;

	t_PartitionListIter iterPartition = m_Partitions.begin();
	while (iterPartition != m_Partitions.end()) {
		if ((*iterPartition)->IsChanged())
			return true;

		iterPartition++;
	}

	return m_bChanged;
}

bool CPartitionTable::ParsePartitionTextFile(ifstream& ifsPartitionFile,
		PartitionTable_T& PartitionHeader, P_PartitionInfo_T& pPartitionInfo) {
	string sValue;
	UINT_T IntParam = 0;
	UINT64 Int64Param = 0;
	bool bRet = true;

	printf("\n\nProcessing partitioning information...\n");

	ifsPartitionFile.seekg(0, ios_base::beg);

	PartitionHeader.Identifier0 = MARVELL_PARTITION_TABLE_ID0;
	PartitionHeader.Identifier1 = MARVELL_PARTITION_TABLE_ID1;

	if (!GetSValue(ifsPartitionFile, "Version:", sValue)) {
		printf("  Error: Key file parsing error reading, Version:\n");
		bRet = false;
	}

	PartitionHeader.Version = Translate(sValue);

//printf ("  Version: 0x%X\n",PartitionHeader.Version );
	if (!GetDWord(ifsPartitionFile, "Number of Partitions:", &IntParam)) {
		printf(
				"  Error: Key file parsing error reading, Number of Partitions:\n");
		bRet = false;
	}

	PartitionHeader.NumPartitions = IntParam;
	if (PartitionHeader.NumPartitions > 100) {
		printf(
				"  Error: Key file parsing error reading, Number of Partitions > 100!\n");
		bRet = false;
	} else {
		pPartitionInfo = (P_PartitionInfo_T) calloc(
				(unsigned int) PartitionHeader.NumPartitions,
				sizeof(PartitionInfo_T));
		if (pPartitionInfo == 0)
			return false;

		memset(pPartitionInfo, 0,
				(unsigned int) PartitionHeader.NumPartitions
						* sizeof(PartitionInfo_T));

		for (unsigned int i = 0; i < PartitionHeader.NumPartitions; i++) {
			//ID
			if (!GetDWord(ifsPartitionFile, "ID:", &IntParam)) {
				printf(
						"  Error: Key file parsing error reading, Partition ID:\n");
				bRet = false;
			}
			pPartitionInfo[i].Indentifier = IntParam;

			//printf ("  ID: %X\n", pPartitionInfo[i].PartitionInfoId );

			//Usage		
			if (!GetSValue(ifsPartitionFile, "Usage:", sValue)) {
				printf("  Error: Key file parsing error reading, Usage:\n");
				bRet = false;
			}

			//printf ("  Usage: 0x%X\n", pPartitionInfo[i].Usage );
			if (ToUpper(sValue) == "BOOT")
				pPartitionInfo[i].Usage = PI_USAGE_BOOT;
			else if (ToUpper(sValue) == "OSLD")
				pPartitionInfo[i].Usage = PI_USAGE_OSLD;
			else if (ToUpper(sValue) == "KRNL")
				pPartitionInfo[i].Usage = PI_USAGE_KRNL;
			else if (ToUpper(sValue) == "FFOS")
				pPartitionInfo[i].Usage = PI_USAGE_FFOS;
			else if (ToUpper(sValue) == "FSYS")
				pPartitionInfo[i].Usage = PI_USAGE_FSYS;
			else {
				string sIdReversed;
				size_t j = min((int) sValue.length(), 4);

				for (; j > 0; j--)
					sIdReversed += sValue[j - 1];
				while (sIdReversed.length() < 4)
					sIdReversed += '\0'; // nulls reserve bytes for int* dereference

				pPartitionInfo[i].Usage = *((int*) sIdReversed.c_str());
			}

			//Type
			if (!GetSValue(ifsPartitionFile, "Type:", sValue)) {
				printf("  Error: Key file parsing error reading, Type:\n");
				bRet = false;
			}

			if (sValue == "Logical" || ToUpper(sValue) == "LOGI") {
				pPartitionInfo[i].Type = PI_TYPE_LOGICAL;
			} else if (sValue == "Physical" || ToUpper(sValue) == "PHYS") {
				pPartitionInfo[i].Type = PI_TYPE_PHYSICAL;
			} else {
				pPartitionInfo[i].Type = 0;
			}

			//Attributes
			if (!GetDWord(ifsPartitionFile, "Attributes:", &IntParam)) {
				printf(
						"  Error: Key file parsing error reading, Partition Attributes:\n");
				bRet = false;
			}
			*(UINT*) &pPartitionInfo[i].Attributes = IntParam;

			//Start Address

			if (!GetDWord(ifsPartitionFile, "Start Address:", &IntParam)) {
				printf(
						"  Error: Key file parsing error reading, Partition Start Address:\n");
				bRet = false;
			}
			pPartitionInfo[i].StartAddr = IntParam;

			//End Address
			if (!GetDWord(ifsPartitionFile, "End Address:", &IntParam)) {
				printf(
						"  Error: Key file parsing error reading, Partition End Address:\n");
				bRet = false;
			}
			pPartitionInfo[i].EndAddr = IntParam;

			//RP Start Address
			// remember location in file
			streamoff ifsPos = ifsPartitionFile.tellg();
			if (!GetDWord(ifsPartitionFile, "RP Start Address:", &IntParam)) {
				pPartitionInfo[i].ReserveStartAddr = 0;
				// ignore line not found and reposition to parse line again
				ifsPartitionFile.seekg(ifsPos, ios_base::beg);
			} else {
				pPartitionInfo[i].ReserveStartAddr = IntParam;
			}

			//RP Size
			// remember location in file
			ifsPos = ifsPartitionFile.tellg();
			if (!GetDWord(ifsPartitionFile, "RP Size:", &IntParam)) {
				pPartitionInfo[i].ReserveSize = 0;
				// ignore line not found and reposition to parse line again
				ifsPartitionFile.seekg(ifsPos, ios_base::beg);
			} else {
				pPartitionInfo[i].ReserveSize = IntParam;
			}

			//RP Algorithm
			// remember location in file
			ifsPos = ifsPartitionFile.tellg();
			if (!GetSValue(ifsPartitionFile, "RP Algorithm:", sValue)) {
				pPartitionInfo[i].ReserveAlgorithm = 0;
				// ignore line not found and reposition to parse line again
				ifsPartitionFile.seekg(ifsPos, ios_base::beg);
			} else {
				if (ToUpper(sValue) == "UPWARD" || sValue == "UPWD")
					pPartitionInfo[i].ReserveAlgorithm = PI_RP_ALGO_UPWD;
				else if (ToUpper(sValue) == "DOWNWARD" || sValue == "DNWD")
					pPartitionInfo[i].ReserveAlgorithm = PI_RP_ALGO_DNWD;
				else
					pPartitionInfo[i].ReserveAlgorithm = 0;
			}

			//RT BBT Type
			// remember location in file
			ifsPos = ifsPartitionFile.tellg();
			if (!GetSValue(ifsPartitionFile, "Runtime BBT Type:", sValue)) {
				*(UINT*) &pPartitionInfo[i].BBT_Type = 0;
				// ignore line not found and reposition to parse line again
				ifsPartitionFile.seekg(ifsPos, ios_base::beg);
			} else {
				if (ToUpper(sValue) == "MBBT")
					*(UINT*) &pPartitionInfo[i].BBT_Type = BBT_TYPE_MBBT;
				else if (ToUpper(sValue) == "WNCE")
					*(UINT*) &pPartitionInfo[i].BBT_Type = BBT_TYPE_WNCE;
				else if (ToUpper(sValue) == "LINXID")
					*(UINT*) &pPartitionInfo[i].BBT_Type = BBT_TYPE_LINX;
				else
					*(UINT*) &pPartitionInfo[i].BBT_Type = 0;
			}

			//RT BBT Location
			// remember location in file
			ifsPos = ifsPartitionFile.tellg();
			if (!GetDWord(ifsPartitionFile, "Runtime BBT Location:",
					&IntParam)) {
				pPartitionInfo[i].RBBT_Location = 0;
				// ignore line not found and reposition to parse line again
				ifsPartitionFile.seekg(ifsPos, ios_base::beg);
			} else
				pPartitionInfo[i].RBBT_Location = IntParam;

			//Backup BBT Location
			// remember location in file
			ifsPos = ifsPartitionFile.tellg();
			if (!GetDWord(ifsPartitionFile, "Backup BBT Location:",
					&IntParam)) {
				pPartitionInfo[i].BackupRBBT_Location = 0;
				// ignore line not found and reposition to parse line again
				ifsPartitionFile.seekg(ifsPos, ios_base::beg);
			} else
				pPartitionInfo[i].BackupRBBT_Location = IntParam;
		}
	}
	//end for loop 

	if (bRet) {
		printf(
				"\n  Success: Partition file parsing has completed successfully!\n");
	} else {
		printf("\n  Failed: Partition file parsing has errors!\n");
	}

	return bRet;
}

bool CPartitionTable::GeneratePartitionBinary(string& sPartitionBinPath) {
	if (m_pPartitionInfo == 0) {
		printf("\n  Error: No PartitionInfo to process!\n");
		return false;
	}

	ofstream ofsPartitionBinFile;
	ofsPartitionBinFile.open(sPartitionBinPath.c_str(),
			ios_base::out | ios_base::binary | ios_base::trunc);
	if (ofsPartitionBinFile.bad() || ofsPartitionBinFile.fail()) {
		printf("\n  Error: Cannot open file name <%s> !\n",
				sPartitionBinPath.c_str());
		return false;
	}

	ofsPartitionBinFile.write((char*) &m_PartitionHeader,
			sizeof(PartitionTable_T));
	for (int j = 0; j < (int) m_PartitionHeader.NumPartitions; j++) {
		ofsPartitionBinFile.write((char*) &m_pPartitionInfo[j],
				sizeof(PartitionInfo_T));
	}

	ofsPartitionBinFile.close();

	printf(
			"\n  Success: Partitioning binary has been generated successfully!\n");
	return true;
}

