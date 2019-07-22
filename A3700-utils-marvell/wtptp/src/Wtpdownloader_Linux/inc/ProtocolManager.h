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
#pragma once
#include <list>
#include "Port.h"
#include "Misc.h"
#include "WtpException.h"
#if WINDOWS
#include <windows.h>
#include "USB_INTERFACE_API.h"
#endif
using namespace std;



class CPort;
class CWtpCommand;
class CWtpException;
class CUARTPort;

class CProtocolManager
{
private:
	CPort* Port;
	CUARTPort* SerialPort;
	void PublicKey () throw(CWtpException);
	void Password () throw(CWtpException);
	void SignedPassword () throw(CWtpException);
	void UploadData() throw(CWtpException);
	void UploadDataHeader (UPLOAD_DATA_PARAMS* pUploadDataParams) throw (CWtpException);
	void HandlePendingMessages();

#if WINDOWS
	//Methods for pipe setup and handling
	void createJtagEnablerChildProcess(string args) throw(CWtpException);

	void writeToPipe(string message) throw(CWtpException);

#endif
public:

	PROTOCOL_VERSION TargetPV;
	TARGET_PARAMS TargetParams;
	list<UPLOAD_DATA_SPEC*> lstUploadSpecs;
	int UploadPacketLength;
	CMisc misc;

#if WINDOWS
	//Members related to pipes set up for JtagEnabler
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;

	//Pipe handles for JtagEnabler (Child process)
	//and WtpDownload (Parent process)
	HANDLE h_Child_STDIN_Read;
	HANDLE h_Child_STDIN_Write;
	HANDLE h_Child_STDOUT_Read;
	HANDLE h_Child_STDOUT_Write;
	HANDLE m_DownloadThreadHandle;
	void JtagOverMedia (string jtagArgsString) throw(CWtpException);
#endif

	void SendPlainTextWtpCR();
	void SendContinuousForceUARTMode();
	void SendContinuousFFStream();
	void SendContinuousFFStream_DLoad();
	void SendContinuousForceConsoleMode();
	void SendPreamble () throw(CWtpException);
	void GetVersion () throw(CWtpException);
	//void SelectImage (unsigned long *pImageType) throw(CWtpException);
	void SelectImage (unsigned int *pImageType) throw(CWtpException);
	void VerifyImage (unsigned char AckOrNack) throw(CWtpException);
	void DataHeader (unsigned int uiRemainingData) throw(CWtpException);
	void Data (unsigned char *pData,int Length) throw(CWtpException);
	void GetWtpMessage ();
	void GetTargetMessage ();
	//bool DebugBoot ();
	//bool OtpView ();
	void Done () throw(CWtpException);
	void Disconnect () throw(CWtpException);
	void JtagReEnable () throw(CWtpException);

	void GetPC() throw (CWtpException);

	void GetTargetProtocolVersion() throw(CWtpException);
	void GetParameters() throw(CWtpException);
	void DoUploads() throw(CWtpException);
	bool isLastData(bool mode, bool value);
	void ParseUploadSpecFile() throw(CWtpException);

	void readFromPipe() throw(CWtpException);

	//helper functions
	CPort* GetPort();
	void SetWtpPort(CPort* Port);
	CWtpStatus* GetStatus();


	CProtocolManager(void);
	~CProtocolManager(void);
#if WINDOWS
    CUSB_INTERFACE_DLL m_USBInterfaceAPI;
#endif
};

