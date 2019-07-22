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
#include "DigitalSignature.h"

CDigitalSignature::CDigitalSignature(void) :
		CTimLib() {
	m_pKey = 0;
	m_bIncludeInTim = true;
	m_bChanged = false;
	DiscardAll();
}

CDigitalSignature::~CDigitalSignature(void) {
	DiscardAll();
	if (m_pKey != 0)
		delete m_pKey;
}

CDigitalSignature::CDigitalSignature(const CDigitalSignature& rhs) :
		CTimLib(rhs) {
	// copy constructor
	CDigitalSignature& nc_rhs = const_cast<CDigitalSignature&>(rhs);

	m_bIncludeInTim = rhs.m_bIncludeInTim;

	if (nc_rhs.m_RsaDigSList.size() > 0) {
		for_each(nc_rhs.m_RsaDigSList.begin(), nc_rhs.m_RsaDigSList.end(),
				[this](string*& ps) {m_RsaDigSList.push_back(new string(*ps));});
	}

	if (nc_rhs.m_ECDigS_RList.size() > 0) {
		for_each(nc_rhs.m_ECDigS_RList.begin(), nc_rhs.m_ECDigS_RList.end(),
				[this](string*& ps) {m_ECDigS_RList.push_back(new string(*ps));});
	}

	if (nc_rhs.m_ECDigS_SList.size() > 0) {
		for_each(nc_rhs.m_ECDigS_SList.begin(), nc_rhs.m_ECDigS_SList.end(),
				[this](string*& ps) {m_ECDigS_SList.push_back(new string(*ps));});
	}

	if (nc_rhs.m_pKey != 0)
		m_pKey = nc_rhs.m_pKey->newCopy();

	m_bChanged = rhs.m_bChanged;
}

CDigitalSignature& CDigitalSignature::operator=(const CDigitalSignature& rhs) {
	// assignment operator
	if (&rhs != this) {
		CTimLib::operator=(rhs);

		CDigitalSignature& nc_rhs = const_cast<CDigitalSignature&>(rhs);

		// delete the existing list and recreate a new one
		DiscardAll();

		m_bIncludeInTim = rhs.m_bIncludeInTim;

		if (nc_rhs.m_RsaDigSList.size() > 0) {
			for_each(nc_rhs.m_RsaDigSList.begin(), nc_rhs.m_RsaDigSList.end(),
					[this](string*& ps) {m_RsaDigSList.push_back(new string(*ps));});
		}

		if (nc_rhs.m_ECDigS_RList.size() > 0) {
			for_each(nc_rhs.m_ECDigS_RList.begin(), nc_rhs.m_ECDigS_RList.end(),
					[this](string*& ps) {m_ECDigS_RList.push_back(new string(*ps));});
		}

		if (nc_rhs.m_ECDigS_SList.size() > 0) {
			for_each(nc_rhs.m_ECDigS_SList.begin(), nc_rhs.m_ECDigS_SList.end(),
					[this](string*& ps) {m_ECDigS_SList.push_back(new string(*ps));});
		}

		if (nc_rhs.m_pKey != 0)
			m_pKey = nc_rhs.m_pKey->newCopy();
	}

	return *this;
}

void CDigitalSignature::DiscardAll() {
	if (m_RsaDigSList.size() > 0) {
		for_each(m_RsaDigSList.begin(), m_RsaDigSList.end(),
				[](string*& ps) {delete ps;});
		m_RsaDigSList.clear();
	}

	if (m_ECDigS_RList.size() > 0) {
		for_each(m_ECDigS_RList.begin(), m_ECDigS_RList.end(),
				[](string*& ps) {delete ps;});
		m_ECDigS_RList.clear();
	}

	if (m_ECDigS_SList.size() > 0) {
		for_each(m_ECDigS_SList.begin(), m_ECDigS_SList.end(),
				[](string*& ps) {delete ps;});
		m_ECDigS_SList.clear();
	}

	if (m_pKey != 0) {
		m_pKey->DiscardAll();
		delete m_pKey;
		m_pKey = 0;
	}

	m_bIncludeInTim = true;
	m_bChanged = false;
}

void CDigitalSignature::Changed(bool bSet) {
	m_bChanged = bSet;
	if (m_pKey)
		m_pKey->Changed(bSet);
}

bool CDigitalSignature::IsChanged() {
	if (m_pKey)
		return m_pKey->IsChanged() || m_bChanged;

	return m_bChanged;
}

void CDigitalSignature::KeySize(unsigned int uiKeySize) {
	if (m_pKey) {
		m_pKey->KeySize(uiKeySize);

		if (m_pKey->EncryptAlgorithmId() == ECDSA_256
				|| m_pKey->EncryptAlgorithmId() == ECDSA_521) {
			// changing the size results in expansion or reduction in exponent and modulus
			ResizeList(m_ECDigS_RList, ((uiKeySize + 31) / 32), m_bChanged);
			ResizeList(m_ECDigS_SList, ((uiKeySize + 31) / 32), m_bChanged);
		} else {
			ResizeList(m_RsaDigSList, ((uiKeySize + 31) / 32), m_bChanged);
		}
	}
}

string& CDigitalSignature::RsaDigS() {
	m_sRsaDigS = "";
	int nPerLine = 0;

	t_stringListIter iterDS = RsaDigSList().begin();
	while (iterDS != RsaDigSList().end()) {
		m_sRsaDigS += *(*iterDS);
		if (++nPerLine % 4 == 0) // 4 is # of words per line
				{
			m_sRsaDigS += "\n";
		} else
			m_sRsaDigS += " ";

		iterDS++;
	}

	return m_sRsaDigS;
}

string& CDigitalSignature::ECDigS_R() {
	m_sECDigS_R = "";
	int nPerLine = 0;

	t_stringListIter iterDS = ECDigS_RList().begin();
	while (iterDS != ECDigS_RList().end()) {
		m_sECDigS_R += *(*iterDS);
		if (++nPerLine % 4 == 0) // 4 is # of words per line
				{
			m_sECDigS_R += "\n";
		} else
			m_sECDigS_R += " ";

		iterDS++;
	}

	return m_sECDigS_R;
}

string& CDigitalSignature::ECDigS_S() {
	m_sECDigS_S = "";
	int nPerLine = 0;

	t_stringListIter iterDS = ECDigS_SList().begin();
	while (iterDS != ECDigS_SList().end()) {
		m_sECDigS_S += *(*iterDS);
		if (++nPerLine % 4 == 0) // 4 is # of words per line
				{
			m_sECDigS_S += "\n";
		} else
			m_sECDigS_S += " ";

		iterDS++;
	}

	return m_sECDigS_S;
}

string& CDigitalSignature::RsaDigSPacked() {
	m_sRsaDigS = "";
	int nPerLine = 0;

	if (m_pKey) {
		unsigned int uiActiveSize = m_pKey->ActiveSizeOfList(RsaDigSList());

		t_stringListIter iterDS = RsaDigSList().begin();
		while (iterDS != RsaDigSList().end() && uiActiveSize > 0) {
			uiActiveSize -= 32;

			m_sRsaDigS += *(*iterDS);
			if (++nPerLine % 4 == 0 || uiActiveSize == 0) // 4 is # of words per line
					{
				m_sRsaDigS += "\n";
			} else
				m_sRsaDigS += " ";

			iterDS++;
		}
	}

	return m_sRsaDigS;
}

string& CDigitalSignature::ECDigS_RPacked() {
	m_sECDigS_R = "";
	int nPerLine = 0;

	if (m_pKey) {
		unsigned int uiActiveSize = m_pKey->ActiveSizeOfList(m_ECDigS_RList);

		t_stringListIter iterDS = m_ECDigS_RList.begin();
		while (iterDS != m_ECDigS_RList.end() && uiActiveSize > 0) {
			uiActiveSize -= 32;

			m_sECDigS_R += *(*iterDS);
			if (++nPerLine % 4 == 0 || uiActiveSize == 0) // 4 is # of words per line
					{
				m_sECDigS_R += "\n";
			} else
				m_sECDigS_R += " ";

			iterDS++;
		}
	}

	return m_sECDigS_R;
}

string& CDigitalSignature::ECDigS_SPacked() {
	m_sECDigS_S = "";
	int nPerLine = 0;

	if (m_pKey) {
		unsigned int uiActiveSize = m_pKey->ActiveSizeOfList(m_ECDigS_SList);

		t_stringListIter iterDS = m_ECDigS_SList.begin();
		while (iterDS != m_ECDigS_SList.end() && uiActiveSize > 0) {
			uiActiveSize -= 32;

			m_sECDigS_S += *(*iterDS);
			if (++nPerLine % 4 == 0 || uiActiveSize == 0) // 4 is # of words per line
					{
				m_sECDigS_S += "\n";
			} else
				m_sECDigS_S += " ";

			iterDS++;
		}
	}

	return m_sECDigS_S;
}

void CDigitalSignature::ResetRsaDigS() {
	if (m_RsaDigSList.size() > 0) {
		for_each(m_RsaDigSList.begin(), m_RsaDigSList.end(),
				[this](string*& ps) {*ps = "0x00000000";});
	}

	m_bChanged = true;
}

void CDigitalSignature::ResetECDigS_R() {
	if (ECDigS_RList().size() > 0) {
		for_each(ECDigS_RList().begin(), ECDigS_RList().end(),
				[this](string*& ps) {*ps = "0x00000000";});
	}

	m_bChanged = true;
}

void CDigitalSignature::ResetECDigS_S() {
	if (ECDigS_SList().size() > 0) {
		for_each(ECDigS_SList().begin(), ECDigS_SList().end(),
				[this](string*& ps) {*ps = "0x00000000";});
	}

	m_bChanged = true;
}

PLAT_DS CDigitalSignature::toPlatDs() {
	PLAT_DS stDs;

	memset(&stDs, 0, sizeof(stDs));

	if (m_pKey) {
		stDs = m_pKey->toPlatDs();

		if (m_pKey->EncryptAlgorithmId() == ECDSA_256
				|| m_pKey->EncryptAlgorithmId() == ECDSA_521) {
			ToArrayFromList(m_ECDigS_RList, stDs.Ecdsa.ECDSADigS_R,
					(m_pKey->KeySize() + 31) / 32);

			ToArrayFromList(m_ECDigS_SList, stDs.Ecdsa.ECDSADigS_S,
					(m_pKey->KeySize() + 31) / 32);
		} else {
			ToArrayFromList(m_RsaDigSList, stDs.Rsa.RSADigS,
					(m_pKey->KeySize() + 31) / 32);
		}
	}
	return stDs;
}

bool CDigitalSignature::FromPlatDs(PLAT_DS& stDs) {
	if (m_pKey == 0) {
		// use DSAlgorithmID to determine what kind of Key is needed
		if (stDs.DSAlgorithmID == ECDSA_256 || stDs.DSAlgorithmID == ECDSA_521)
			m_pKey = new CECCKey;
		else
			m_pKey = new CRSAKey;
	}
	if (m_pKey) {
		if (m_pKey->FromPlatDs(stDs)) {
			KeySize(stDs.KeySize);
			if (m_pKey->EncryptAlgorithmId() == ECDSA_256
					|| m_pKey->EncryptAlgorithmId() == ECDSA_521) {
				if (!ToListFromArray(stDs.Ecdsa.ECDSADigS_R, m_ECDigS_RList,
						((m_pKey->KeySize() + 31) / 32)))
					return false;

				if (!ToListFromArray(stDs.Ecdsa.ECDSADigS_S, m_ECDigS_SList,
						((m_pKey->KeySize() + 31) / 32)))
					return false;
			} else {
				if (!ToListFromArray(stDs.Rsa.RSADigS, m_RsaDigSList,
						((m_pKey->KeySize() + 31) / 32)))
					return false;
			}
		}
	}
	return true;
}

bool CDigitalSignature::DigSFromArray(UINT* pDigSAry, unsigned int size) {
	if (m_pKey) {
		if (m_pKey->EncryptAlgorithmId() == ECDSA_256
				|| m_pKey->EncryptAlgorithmId() == ECDSA_521) {
			// pDigSAry contains the complete ds, which for ECC consists of 2 components
			if (!ToListFromArray(pDigSAry, m_ECDigS_RList, size))
				return false;

			if (!ToListFromArray(&pDigSAry[size], m_ECDigS_SList, size))
				return false;
		} else {
			// pDigSAry contains the complete ds, for RSA consists of one component
			if (!ToListFromArray(pDigSAry, m_RsaDigSList, size))
				return false;
		}
	}
	return true;
}

bool CDigitalSignature::DigSToArray(UINT* pDigSAry, unsigned int size) {
	if (m_pKey) {
		if (m_pKey->EncryptAlgorithmId() == ECDSA_256
				|| m_pKey->EncryptAlgorithmId() == ECDSA_521) {
			// pack both components of the complete ds 
			if (!ToArrayFromList(m_ECDigS_RList, pDigSAry, size))
				return false;

			if (!ToArrayFromList(m_ECDigS_SList, &pDigSAry[size], size))
				return false;
		} else {
			if (!ToArrayFromList(m_RsaDigSList, pDigSAry, size))
				return false;
		}
	}
	return true;
}
