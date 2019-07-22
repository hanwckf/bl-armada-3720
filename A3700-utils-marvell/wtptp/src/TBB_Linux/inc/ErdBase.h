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


#ifndef ERDBASE_H
#define ERDBASE_H

#include "ReservedPackageData.h"
#include "TimLib.h"
#include "TimDescriptorLine.h"

#include <string>
#include <vector>
using namespace std;

typedef vector<string*> t_stringVector;
typedef vector<string*>::iterator t_stringVectorIter;

// forward declaration
class CTimDescriptor;

class CErdBase: public CTimLib {
public:

	enum ERD_PKG_TYPE {
		UNKNOWN_ERD = 0,
		AUTOBIND_ERD = 100,
		ESCAPESEQ_ERD,
		GPIOSET_ERD,
		GPIO_ERD,
		RESUME_ERD, // obsolete but kept to maintain pkg type id
		RESUME_DDR_ERD,
		TBRXFER_ERD,
		XFER_ERD,
		UART_ERD,
		USB_ERD,
		USBVENDORREQ_ERD,
		CONSUMER_ID_ERD,
		DDR_INITIALIZATION_ERD,
		INSTRUCTIONS_ERD,
		DDR_OPERATIONS_ERD,
		CORE_ID_ERD,
		TZ_INITIALIZATION_ERD,
		TZ_OPERATIONS_ERD,
		BBMT_ERD,
		GPP_ERD,
		GPP_OPERATIONS_ERD,
		ROM_RESUME_ERD,
		PIN_ERD,
		IMAGE_MAPS_ERD,
		IMAGE_MAP_INFO_ERD,
		ESC_SEQ_V2_ERD,
		CORE_PAIR_ERD,
		CORE_RELEASE_ERD,
		MFPR_PAIR_ERD,
		ONDEMAND_BOOT_ERD,
		FUSE_ERD,
		FUSE_SOC_CONFIG_ERD,
		FUSE_APCP_CONFIG_ERD,
		FUSE_MP_CONFIG_ERD,
		FUSE_RKEK_ERD,
		FUSE_OEM_PLATFORM_KEY_ERD,
		FUSE_JTAG_ERD,
		FUSE_UID_ERD,
		FUSE_USBID_ERD,
		FUSE_SECURITY_CONFIG_ERD,
		FUSE_LCS_STATE_ERD,
		FUSE_LOCK_BLOCK_ERD,
		FUSE_PIN_ERD,
		FUSE_SW_VERSION_ERD,
		FUSE_MARVELL_CONFIG_BITS_ERD,
		FUSE_DRM_TRANSPORT_KEY_ERD,
		FUSE_BOOLEAN_FUSE_BITS_ERD,
		FUSE_BLOCK_ERD,
		MV_IMAGE_HEADERS_PKG_ERD,
		MV_IMAGE_HEADER_ERD,
		MV_EXT_RSA_KEY_ERD,
		MV_CUST_KEY_ERD,
		DDR_TYPE_ERD,
		FLASH_GEOMETRY_REQ_ERD,
		MAX_ERD_PACKAGES = (MV_CUST_KEY_ERD - 100 + 2)
	};

	CErdBase(ERD_PKG_TYPE ErdType, int iMaxFieldNum);
	virtual ~CErdBase(void);

	// copy constructor
	CErdBase(const CErdBase& rhs);
	// assignment operator
	virtual CErdBase& operator=(const CErdBase& rhs);

	virtual void Reset();

	const ERD_PKG_TYPE ErdPkgType() {
		return m_eErdType;
	}
	static CErdBase* Create(ERD_PKG_TYPE ErdPkgType);
	static CErdBase* Create(CErdBase& src);
	static CErdBase* Create(string& sPackageName, bool bIsBlf = false);

	virtual const string& PackageName() = 0;

	virtual bool Parse(CTimDescriptor& TimDescriptor,
			CTimDescriptorLine*& pLine, bool bIsBlf);

	virtual bool ToBinary(ofstream& ofs) = 0;
	virtual unsigned int PackageSize() {
		return 0;
	}
	virtual int AddPkgStrings(CReservedPackageData* pRPD) = 0;

	unsigned int MaxFieldNum() {
		return m_iMaxFieldNum;
	}
	t_stringVector& FieldNames() {
		return m_FieldNames;
	}
	t_stringVector& FieldValues() {
		return m_FieldValues;
	}
	t_stringVector& FieldComments() {
		return m_FieldComments;
	}

	bool IsChanged() {
		return m_bChanged;
	}
	void Changed(bool bSet) {
		m_bChanged = bSet;
	}

	unsigned int m_iMaxFieldNum;
	t_stringVector m_FieldNames;
	t_stringVector m_FieldValues;
	t_stringVector m_FieldComments;

protected:
	bool m_bChanged;
	ERD_PKG_TYPE m_eErdType;

private:
	void Init();
};

typedef list<CErdBase*> t_ErdBaseVector;
typedef list<CErdBase*>::iterator t_ErdBaseVectorIter;

#endif //ERDBASE_H
