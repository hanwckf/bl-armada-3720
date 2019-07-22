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
#include <cstring>
//#include <sstream>
#include <iomanip>
#include "WtpCommand.h"
#include <iostream>

using namespace std;


int CWtpCommand::iSequence = 0x00;

CWtpCommand::CWtpCommand(void)
{
	ResetWtpCommand();
}

CWtpCommand::~CWtpCommand(void){}

WTPCOMMAND CWtpCommand::GetWtpCommand()
{
	return this->WtpCmd;
}

void CWtpCommand::print()
{
	cout << "HOST::" << endl;
	cout << "CMD\tSEQ\tCID\tFLAGS\tLEN\t\t\tDATA" << endl;
	cout << hex <<  setfill('0') << setw(2) << "0x" << (unsigned int)WtpCmd.CMD;
	cout << hex <<  setfill('0') << setw(2) << "\t0x" << (unsigned int)WtpCmd.SEQ;
	cout << hex <<  setfill('0') << setw(2) << "\t0x" << (unsigned int)WtpCmd.CID;
	cout << hex <<  setfill('0') << setw(2) << "\t0x" << (unsigned int)WtpCmd.Flags;
	BYTE lenBuf[4];
	lenBuf[0] = (int)((WtpCmd.LEN & 0xFF000000) >> 24 );
	lenBuf[1] = (int)((WtpCmd.LEN & 0x00FF0000) >> 16 );
	lenBuf[2] = (int)((WtpCmd.LEN & 0x0000FF00) >> 8 );
	lenBuf[3] = (int)((WtpCmd.LEN & 0X000000FF));
	cout << "\t";
	for(int i = 0;i < 4 ; i++)
		cout << hex <<  setfill('0') << setw(2) << "0x" << (unsigned int)lenBuf[i] << " ";

	BYTE buffer[DATALENGTH];
	memset(buffer,0,DATALENGTH);

	if(memcmp(WtpCmd.Data,buffer,DATALENGTH) != 0)
		for(int i = 1 ; i <= WtpCmd.LEN ; i++)
		{
			if(i % 6 == 0)
				cout << endl << "\t\t\t\t\t\t";
			cout << hex << setfill('0') << setw(2) << "0x" << (unsigned int)WtpCmd.Data[i-1] << " ";
		}
	cout << endl << endl;
}

void CWtpCommand::ResetWtpCommand()
{
	memset(&(this->WtpCmd),0,sizeof(WTPCOMMAND));
}

unsigned char CWtpCommand::GetCMD()
{
	return this->WtpCmd.CMD;
}

unsigned char CWtpCommand::GetCID()
{
	return this->WtpCmd.CID;
}

unsigned char CWtpCommand::GetSEQ()
{
	return this->WtpCmd.SEQ;
}

unsigned char CWtpCommand::GetFlags()
{
	return this->WtpCmd.Flags;
}

long CWtpCommand::GetLEN()
{
	return this->WtpCmd.LEN;
}

unsigned char* CWtpCommand::GetData()
{
	return this->WtpCmd.Data;
}

void CWtpCommand::SetCMD(unsigned char CMD)
{
	this->WtpCmd.CMD = CMD;
}

void CWtpCommand::SetCID(unsigned char CID)
{
	this->WtpCmd.CID = CID;
}

void CWtpCommand::SetSEQ(unsigned char SEQ)
{
	this->WtpCmd.SEQ = SEQ;
}

void CWtpCommand::SetFlags(unsigned char Flags)
{
	this->WtpCmd.Flags = Flags;
}

void CWtpCommand::SetData(unsigned char Data,int pos)
{
	this->WtpCmd.Data[pos] = Data;
}
void CWtpCommand::SetData(unsigned char *Data, int length)
{
	for(int i = 0; i < length ; i++)
		SetData(Data[i],i);
}

CPlainTextWTPCR::CPlainTextWTPCR(void)
{
	ResetWtpCommand();
	WtpCmd.CMD = PLAINTEXTWTPCR;
	WtpCmd.SEQ = 0x74;
	WtpCmd.CID = 0x70;
	WtpCmd.Flags = 0xD;
}

CPreambleCommand::CPreambleCommand(void)
{
	ResetWtpCommand();
	WtpCmd.CMD = PREAMBLE;
	WtpCmd.SEQ = 0xD3;
	WtpCmd.CID = 0x02;
	WtpCmd.Flags = 0x2B;
//	WtpCmd.LEN = 0x00D3022B;
//	WtpCmd.LEN = 0x00000000;
}

CGetVersionCommand::CGetVersionCommand(void)
{
	ResetWtpCommand();
	CWtpCommand::iSequence = 0x00;
	WtpCmd.CMD = GETVERSION;
	WtpCmd.SEQ = 0x00;
	WtpCmd.CID = 0x00;
	WtpCmd.Flags = 0x8;
	WtpCmd.LEN = 0x00000000;
}

CSelectImageCommand::CSelectImageCommand()
{
	ResetWtpCommand();
	WtpCmd.CMD = SELECTIMAGE;
	WtpCmd.SEQ = 0x00;
	WtpCmd.CID = 0x00;
	WtpCmd.Flags = 0x00;
	WtpCmd.LEN = 0x00000000;
}

CVerifyImageCommand::CVerifyImageCommand(unsigned char AckOrNoAck)
{
	ResetWtpCommand();
	iSequence = 0x00;
	WtpCmd.CMD = VERIFYIMAGE;
	WtpCmd.SEQ = 0x00;
	WtpCmd.CID = 0x00;
	WtpCmd.Flags = 0x00;
	WtpCmd.LEN = 0x00000001;
	WtpCmd.Data[0] = AckOrNoAck;
}

CDataHeaderCommand::CDataHeaderCommand(bool bFastDownload)
{
	ResetWtpCommand();
	iSequence++;
	WtpCmd.CMD = DATAHEADER;
	WtpCmd.SEQ = iSequence;
	WtpCmd.CID = 0x00;
	WtpCmd.Flags = 0x00;
	WtpCmd.LEN = 0x00000004;
	if ( bFastDownload)
		WtpCmd.Flags = 0x4;
}

CDataCommand::CDataCommand(unsigned char *pData, int Length)
{
	ResetWtpCommand();
	iSequence++;
	WtpCmd.CMD = DATA;
	WtpCmd.SEQ = iSequence;
	WtpCmd.CID = 0x00;
	WtpCmd.Flags = 0x00;
	WtpCmd.LEN = Length;
	SetData(pData,Length);
}

CDoneCommand::CDoneCommand()
{
	ResetWtpCommand();
	iSequence = 0x00;
	WtpCmd.CMD = DONE;
	WtpCmd.SEQ = 0x00;
	WtpCmd.CID = 0x00;
	WtpCmd.Flags = 0x00;
	WtpCmd.LEN = 0x00000000;
}


CDisconnectCommand::CDisconnectCommand()
{
	ResetWtpCommand();
	iSequence = 0x00;
	WtpCmd.CMD = DISCONNECT;
	WtpCmd.SEQ = 0x00;
	WtpCmd.CID = 0x00;
	WtpCmd.Flags = 0x00;
	WtpCmd.LEN = 0x00000000;
}

CPasswordCommand::CPasswordCommand()
{
	ResetWtpCommand();
	iSequence = 0x00;
	WtpCmd.CMD = PASSWORD;
	WtpCmd.SEQ = iSequence;
	WtpCmd.CID = 0x00;
	WtpCmd.Flags = 0x00;
	WtpCmd.LEN = 0x00000000;
}

CPublicKeyCommand::CPublicKeyCommand()
{
	ResetWtpCommand();
	iSequence = 0x00;
	WtpCmd.CMD = PUBLICKEY;
	WtpCmd.SEQ = iSequence;
	WtpCmd.CID = 0x00;
	WtpCmd.Flags = 0x00;
	WtpCmd.LEN = 0x00000000;
}

CSignedPasswordCommand::CSignedPasswordCommand()
{
	ResetWtpCommand();
	iSequence = 0x00;
	WtpCmd.CMD = SIGNEDPASSWORD;
	WtpCmd.SEQ = iSequence;
	WtpCmd.CID = 0x00;
	WtpCmd.Flags = 0x00;
	WtpCmd.LEN = 0x00000000;
}

CMessageCommand::CMessageCommand()
{
	ResetWtpCommand();
	iSequence = 0x00;
	WtpCmd.CMD = MESSAGE;
	WtpCmd.SEQ = iSequence;
	WtpCmd.CID = 0x00;
	WtpCmd.Flags = 0x00;
	WtpCmd.LEN = 0x00000000;
}

CTargetProtocolVersionCommand::CTargetProtocolVersionCommand()
{
	ResetWtpCommand();
	WtpCmd.CMD = PROTOCOLVERSION;
	WtpCmd.SEQ = iSequence;
	WtpCmd.CID = 0x00;
	WtpCmd.Flags = 0x00;
	WtpCmd.LEN = 0x00000000;
}

CGetParametersCommand::CGetParametersCommand()
{
	ResetWtpCommand();
	WtpCmd.CMD = GETPARAMETERS;
	WtpCmd.SEQ = iSequence;
	WtpCmd.CID = 0x00;
	WtpCmd.Flags = 0x00;
	WtpCmd.LEN = 0x00000000;
}

CUploadDataHeaderCommand::CUploadDataHeaderCommand()
{
	iSequence = 0x00;
	ResetWtpCommand();
	WtpCmd.CMD = UPLOADDATAHEADER;
	WtpCmd.SEQ = iSequence;
	WtpCmd.CID = 0x00;
	WtpCmd.Flags = 0x00;
	WtpCmd.LEN = sizeof(UPLOAD_DATA_PARAMS);
}

CUploadDataCommand::CUploadDataCommand()
{
	ResetWtpCommand();
	WtpCmd.CMD = UPLOADDATA;
	WtpCmd.SEQ = iSequence;
	WtpCmd.CID = 0x00;
	WtpCmd.Flags = 0x00;
	WtpCmd.LEN = 0x00000000;
}

CGetPCCommand::CGetPCCommand()
{
	ResetWtpCommand();
	WtpCmd.CMD = GETPC;
	WtpCmd.SEQ = iSequence;
	WtpCmd.CID = 0x00;
	WtpCmd.Flags = 0x00;
	WtpCmd.LEN = 0x00000004;
}

CJtagOverMediaDataCommand::CJtagOverMediaDataCommand(unsigned char *pData, int Length)
{
	iSequence++;
	ResetWtpCommand();
	WtpCmd.CMD = JTAGOVERMEDIA;
	WtpCmd.SEQ = iSequence;
	WtpCmd.CID = 0x00;
	WtpCmd.Flags = 0x00;
	WtpCmd.LEN = Length;
	SetData(pData,Length);
}

CPlainTextWTPCR::~CPlainTextWTPCR(){}
CPreambleCommand::~CPreambleCommand(){}
CGetVersionCommand::~CGetVersionCommand(){}
CSelectImageCommand::~CSelectImageCommand(){}
CVerifyImageCommand::~CVerifyImageCommand(){}
CDataHeaderCommand::~CDataHeaderCommand(){}
CDataCommand::~CDataCommand(){}
CDoneCommand::~CDoneCommand(){}
CDisconnectCommand::~CDisconnectCommand(){}
CPasswordCommand::~CPasswordCommand(){}
CPublicKeyCommand::~CPublicKeyCommand(){}
CSignedPasswordCommand::~CSignedPasswordCommand(){}
CMessageCommand::~CMessageCommand(){}
CTargetProtocolVersionCommand::~CTargetProtocolVersionCommand(){}
CGetParametersCommand::~CGetParametersCommand(){}
CUploadDataHeaderCommand::~CUploadDataHeaderCommand(){}
CUploadDataCommand::~CUploadDataCommand(){}
CJtagOverMediaDataCommand::~CJtagOverMediaDataCommand(){}
CGetPCCommand::~CGetPCCommand(){}

//CDebugBootCommand::CDebugBootCommand()
//{
//	ResetWtpCommand();
//	WtpCmd.CMD = DEBUGBOOT;
//	WtpCmd.SEQ = 0x00;
//	WtpCmd.CID = 0x00;
//	WtpCmd.Flags = 0x00;
//	WtpCmd.LEN = 0x00000001;
//}

//COtpViewCommand::COtpViewCommand()
//{
//	ResetWtpCommand();
//	iSequence = 0x00;
//	WtpCmd.CMD = OTPVIEW;
//	WtpCmd.SEQ = iSequence;
//	WtpCmd.CID = 0x00;
//	WtpCmd.Flags = 0x00;
//	WtpCmd.LEN = 0x00000001;
//	WtpCmd.Data[0] = theApp.iOtpFlashType;
//}
//CDebugBootCommand::~CDebugBootCommand(){}
//COtpViewCommand::~COtpViewCommand(){}

