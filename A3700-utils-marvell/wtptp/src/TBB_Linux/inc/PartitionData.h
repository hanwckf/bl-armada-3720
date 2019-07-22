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


#ifndef __PARTITION_DATA_H__
#define __PARTITION_DATA_H__

#include "TimLib.h"

#include <string>
#include <list>
#include <iterator>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <cstdlib>
using namespace std;

class CPartitionData: public CTimLib {
public:
	CPartitionData(void);
	virtual ~CPartitionData(void);

	// copy constructor
	CPartitionData(const CPartitionData& rhs);
	// assignment operator
	CPartitionData& operator=(const CPartitionData& rhs);

	void PartitionId(const unsigned int Id) {
		m_PartitionId = Id;
		m_bChanged = true;
	}
	unsigned int PartitionId() {
		return m_PartitionId;
	}

	void PartitionAttributes(const unsigned int Attributes) {
		m_PartitionAttributes = Attributes;
		m_bChanged = true;
	}
	unsigned int PartitionAttributes() {
		return m_PartitionAttributes;
	}

	void PartitionUsage(const string& sUsage) {
		m_sPartitionUsage = sUsage;
		m_bChanged = true;
	}
	string& PartitionUsage() {
		return m_sPartitionUsage;
	}

	void PartitionType(const string& sType) {
		m_sPartitionType = sType;
		m_bChanged = true;
	}
	string& PartitionType() {
		return m_sPartitionType;
	}

	void PartitionStartAddress(const u_int64_t Address) {
		m_PartitionStartAddress = Address;
		m_bChanged = true;
	}
	u_int64_t PartitionStartAddress() {
		return m_PartitionStartAddress;
	}

	void PartitionEndAddress(const u_int64_t Address) {
		m_PartitionEndAddress = Address;
		m_bChanged = true;
	}
	u_int64_t PartitionEndAddress() {
		return m_PartitionEndAddress;
	}

	void ReservedPoolStartAddress(const u_int64_t Address) {
		m_ReservedPoolStartAddress = Address;
		m_bChanged = true;
	}
	u_int64_t ReservedPoolStartAddress() {
		return m_ReservedPoolStartAddress;
	}

	void RuntimeBBTType(const string& sType) {
		m_sRuntimeBBTType = sType;
		m_bChanged = true;
	}
	string& RuntimeBBTType() {
		return m_sRuntimeBBTType;
	}

	void RuntimeBBTLocation(const unsigned int Location) {
		m_RuntimeBBTLocation = Location;
		m_bChanged = true;
	}
	unsigned int RuntimeBBTLocation() {
		return m_RuntimeBBTLocation;
	}

	void BackupRuntimeBBTLocation(const unsigned int Location) {
		m_BackupRuntimeBBTLocation = Location;
		m_bChanged = true;
	}
	unsigned int BackupRuntimeBBTLocation() {
		return m_BackupRuntimeBBTLocation;
	}

	void ReservedPoolSize(const unsigned int Size) {
		m_ReservedPoolSize = Size;
		m_bChanged = true;
	}
	unsigned int ReservedPoolSize() {
		return m_ReservedPoolSize;
	}

	void ReservedPoolAlgorithm(const string& Algorithm) {
		m_sReservedPoolAlgorithm = Algorithm;
		m_bChanged = true;
	}
	string& ReservedPoolAlgorithm() {
		return m_sReservedPoolAlgorithm;
	}

	bool IsChanged() {
		return m_bChanged;
	}
	void Changed(bool bSet) {
		m_bChanged = bSet;
	}

private:
	unsigned int m_PartitionId;
	unsigned int m_PartitionAttributes;
	string m_sPartitionUsage;
	string m_sPartitionType;

	u_int64_t m_PartitionStartAddress;
	u_int64_t m_PartitionEndAddress;
	u_int64_t m_ReservedPoolStartAddress;

	string m_sRuntimeBBTType;
	unsigned int m_RuntimeBBTLocation;
	unsigned int m_BackupRuntimeBBTLocation;
	unsigned int m_ReservedPoolSize;
	string m_sReservedPoolAlgorithm;

	bool m_bChanged;
};

typedef list<CPartitionData*> t_PartitionList;
typedef list<CPartitionData*>::iterator t_PartitionListIter;

#endif //__PARTITION_DATA_H__
