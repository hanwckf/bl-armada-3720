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

#include "TimDescriptorLine.h"
#include "TimDescriptorParser.h"

CTimDescriptorLine::CTimDescriptorLine(void) :
		CTimLib() {
	m_bParsingError = false;
	m_sParsingError = "No error specified";
	m_bParsingWarning = false;
	m_sParsingWarning = "No warning specified";
}

CTimDescriptorLine::~CTimDescriptorLine(void) {
	DiscardAll();
}

void CTimDescriptorLine::DiscardAll() {

}

// copy constructor
CTimDescriptorLine::CTimDescriptorLine(const CTimDescriptorLine& rhs) :
		CTimLib(rhs) {
	m_FieldName = rhs.m_FieldName;
	m_FieldValue = rhs.m_FieldValue;

	m_bParsingError = rhs.m_bParsingError;
	m_sParsingError = rhs.m_sParsingError;
	m_bParsingWarning = rhs.m_bParsingWarning;
	m_sParsingWarning = rhs.m_sParsingWarning;

}

// assignment operator
CTimDescriptorLine& CTimDescriptorLine::operator=(
		const CTimDescriptorLine& rhs) {
	// assignment operator
	if (&rhs != this) {
		CTimLib::operator=(rhs);

		DiscardAll();

		m_FieldName = rhs.m_FieldName;
		m_FieldValue = rhs.m_FieldValue;

		m_bParsingError = rhs.m_bParsingError;
		m_sParsingError = rhs.m_sParsingError;
		m_bParsingWarning = rhs.m_bParsingWarning;
		m_sParsingWarning = rhs.m_sParsingWarning;

	}

	return *this;
}

bool CTimDescriptorLine::ParseLine(ifstream& ifs, CTimDescriptorParser& Parser,
		bool bIsBlf) {
	string sValue;
	size_t nPos = 0;
	size_t nEndPos = 0;
	string sRawText;

	// get PreCommentLines
	while (!ifs.eof() && ifs.good() && Parser.GetNextLine(ifs, sValue, true)) {
		sRawText = sValue;

		// if 1st char (after leading white space) is a ;, 
		//  then line is a comment so ignore leading whitespace
		nPos = sValue.find_first_of(" \n\r\t");
		nEndPos = sValue.find_first_not_of(" \n\r\t");
		if (nPos == 0 && nEndPos != string::npos) {
			// remove leading whitespace
			sValue = sValue.substr(nEndPos);
		}

		if (!(nPos == 0 && nEndPos == string::npos))
			if (sValue.length() > 0 && sValue[0] != ';')
				break; // not a comment, so process as a field/value

	}

	if (!ifs.eof() && (ifs.bad() || ifs.fail()))
		return false;

	// parse out post comment
	if ((nPos = sValue.find(';')) != string::npos) {
		// include ws between field and post comment
		while (nPos > 0 && (nPos != string::npos)) {
			if (!isspace(0x000000FF & sValue[nPos - 1]))
				break;
			nPos--;
		}

		sValue = sValue.substr(0, nPos);
	}

	// parse out field name and field value
	if ((nPos = sValue.find_first_of(":=")) != string::npos) {
		m_FieldName = sValue.substr(0, nPos);
		m_FieldValue = sValue.substr(nPos + 1);
		if (string::npos
				!= (nPos = m_FieldValue.find_first_not_of(" \n\r\t"))) {
			m_FieldValue = m_FieldValue.substr(nPos);

			if (bIsBlf) {
				while ((nPos = m_FieldValue.find_first_of("<,>"))
						!= string::npos) {
					m_FieldValue.replace(nPos, 1, 1, ' ');
				}
			}
		}
	} else {
		sValue = TrimWS(sValue);
		// no fieldname delimiter, so assume it is all hex value(s)
		// like in the reserved data or the key data areas
		if (CTimLib::IsNumeric(sValue) || CTimLib::IsHexNumeric(sValue)) {
			m_FieldName = "";
			m_FieldValue = sValue;
		} else {
			m_FieldName = sValue;
			m_FieldValue = "";
		}
	}

	// this ensures no white space gets into field name and values
	// and fixes issues with Linux line endings
	m_FieldName = TrimWS(m_FieldName);
	m_FieldValue = TrimWS(m_FieldValue);

	return true;
}

bool CTimDescriptorLine::ParseLine(t_stringList& Lines,
		CTimDescriptorParser& Parser, bool bIsBlf) {
	string sValue;
	size_t nPos = 0;
	size_t nEndPos = 0;
	string sRawText;

	// get PreCommentLines
	t_stringListIter iter;
	//reset to begin of list on each loop
	while (iter = Lines.begin(), iter != Lines.end()) {
		sValue = *(*iter);
		//remove the line being processed
		delete (*iter);
		Lines.remove(*iter);

		// save the raw text of the line
		sRawText = sValue;

		// if 1st char (after leading white space) is a ;, 
		//  then line is a comment so ignore leading whitespace
		nPos = sValue.find_first_of(" \n\r\t");
		nEndPos = sValue.find_first_not_of(" \n\r\t");
		if (nPos == 0 && nEndPos != string::npos) {
			// remove leading whitespace
			sValue = sValue.substr(nEndPos);
		}

		if (!(nPos == 0 && nEndPos == string::npos))
			if (sValue.length() > 0 && sValue[0] != ';')
				break; // not a comment, so process as a field/value
	}

	// parse out post comment
	if ((nPos = sValue.find(';')) != string::npos) {
		// include ws between field and post comment
		while (nPos > 0 && (nPos != string::npos))
		{
			if (!isspace(0x000000FF & sValue[nPos - 1]))
				break;
			nPos--;
		}
		sValue = sValue.substr(0, nPos);
	}

	// parse out field name and field value
	if ((nPos = sValue.find_first_of(":=")) != string::npos) {
		m_FieldName = sValue.substr(0, nPos);
		m_FieldValue = sValue.substr(nPos + 1);
		if (string::npos
				!= (nPos = m_FieldValue.find_first_not_of(" \n\r\t"))) {

			m_FieldValue = m_FieldValue.substr(nPos);

			if (bIsBlf) {
				while ((nPos = m_FieldValue.find_first_of("<,>"))
						!= string::npos) {
					m_FieldValue.replace(nPos, 1, 1, ' ');
				}
			}
		}
	} else {
		sValue = TrimWS(sValue);
		// no fieldname delimiter, so assume it is all hex value(s)
		// like in the reserved data or the key data areas
		if (CTimLib::IsNumeric(sValue) || CTimLib::IsHexNumeric(sValue)) {
			m_FieldName = "";
			m_FieldValue = sValue;
		} else {
			m_FieldName = sValue;
			m_FieldValue = "";
		}
	}

	// this ensures no white space gets into field name and values
	// and fixes issues with Linux line endings
	m_FieldName = TrimWS(m_FieldName);
	m_FieldValue = TrimWS(m_FieldValue);

	return true;
}

