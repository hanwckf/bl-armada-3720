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
#if LINUX
#include <errno.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include "UARTPortLinux.h"
#include "WtpDownloadApp.h"
#include "WtpCommand.h"
#include "WTPDEF.h"
#include <iostream>
#include <sstream>
#include <string.h>
#include <unistd.h>
using namespace std;

CUARTPortLinux::~CUARTPortLinux(void){}
CUARTPortLinux::CUARTPortLinux(void){}

/**
* Method:	OpenPort
* Input:	N/A
* Output:	void
* Desc:		This is a overriden method from Port class.
*			Opens UART port.
*/
void CUARTPortLinux::OpenPort() throw (CWtpException)
{
	struct termios options;
	stringstream portNumber;
	portNumber << theApp.iCommPort;
	UARTDeviceName ="/dev/ttyUSB" ;
	UARTDeviceName.append(portNumber.str());
	cout << endl << UARTDeviceName << endl;
	uartLinuxFileDesc = open(UARTDeviceName.c_str(),O_RDWR|O_NOCTTY);
	if(uartLinuxFileDesc < 0)
	{
		cout << endl << " ERROR: Open UART driver failed:" << endl;
		throw CWtpException(CWtpException::OPENUARTPORT,0,UARTDeviceName);
	}
	fcntl(uartLinuxFileDesc,F_SETFL,0);
	memset(&options, 0, sizeof(options));
	cfsetispeed(&options, B115200);
	cfsetospeed(&options, B115200);
	options.c_iflag     &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	options.c_lflag     &= ~(ICANON|ECHO|ECHOE|ISIG|ECHONL|IEXTEN);
	options.c_oflag     &= ~OPOST;
	options.c_cc[VMIN]   = 0;
	options.c_cc[VTIME]  = 0;
	options.c_cflag &= ~(CSIZE|PARENB);
	options.c_cflag |= CS8;
	tcsetattr(uartLinuxFileDesc, TCSANOW, &options);
}



void CUARTPortLinux::SetCommReadTimeout(DWORD msecs) {
//	CommTimeOuts.ReadTotalTimeoutMultiplier = msecs;

}


/**
* Method:	ReadPort
* Input:	CWtpStatus, DWORD
* Output:	void
* Desc:		This is a overriden method from Port class.
*			Retrieves CWtpStatus message sent on UART port by target.
*/
void CUARTPortLinux::ReadPort(UINT32_T dwStatusSize)throw (CWtpException)
{
	int result = 0;
	int BytesReceived = 0, BytesRemaining = dwStatusSize;
    char* buffer = new char[dwStatusSize];
    memset( buffer, 0, dwStatusSize );
    //cout << "CUARTPortLinux::ReadPort(UINT_T dwSize)" << endl;

	// Read the initial 6 bytes of the target's command response.
	while(BytesRemaining > 0)
	{

		result = read(uartLinuxFileDesc, buffer + BytesReceived, BytesRemaining);
		if(result == -1)
		{
			delete[] buffer;
			throw CWtpException(CWtpException::READUARTPORT);
		}
		BytesRemaining -= result;
		BytesReceived += result;
	}

	// Now check Length payload for this command and prime the host.
	memcpy(&WtpStatus.GetWtpStatus(),buffer,BytesReceived);
	if(WtpStatus.GetDLEN() > 0)
		BytesRemaining = WtpStatus.GetDLEN();
	else if(WtpStatus.GetCMD() == UPLOADDATA)
		BytesRemaining = theApp.ProtocolManager.UploadPacketLength;

	// Read additional data specified by LEN field
	//cout << "ReadPort remaining bytes after 6 bytes of response in is: " << BytesRemaining << endl;
	if(BytesRemaining > 0)
	{

		result = 0;
		BytesReceived = 0;
		delete[] buffer;
		buffer = new char[WtpStatus.GetDLEN()];
		while (BytesRemaining > 0)
		{
			result = read(uartLinuxFileDesc, buffer + BytesReceived, BytesRemaining);
			if(result == -1)
			{
				delete[] buffer;
				cout << endl << "Error: ReadUart returned " <<  result << endl;
				throw CWtpException(CWtpException::READUARTPORT);
			}
			BytesReceived += result;
			BytesRemaining -= result;
		}
		memcpy(WtpStatus.GetData(),buffer,WtpStatus.GetDLEN());
	}
	delete[] buffer;
}


void CUARTPortLinux::WriteRaw(char * buffer, DWORD count) throw (CWtpException)
{
	DWORD bytestransmitted;
	if (write(uartLinuxFileDesc, buffer, count) == 0)
	{
		stringstream ss;
		ss << "Error: " << "WriteRaw failed.. " << endl;
		throw CWtpException(CWtpException::WRITEUARTPORT);
	}
}

int CUARTPortLinux::ReadRaw(char * buffer, DWORD count)
{
	int BytesRead, i = 0;
	BytesRead = read(uartLinuxFileDesc, buffer, count);
#if DEV_DEBUG
	if (BytesRead > 0) {
		cout << "Bytes Read: ";
		for (i = 0; i < BytesRead; i++) {
			cout  << hex << (unsigned int) buffer[i] << " " << endl;
		}
		cout << endl;
	}
#endif
	return BytesRead;
}



/**
* Method:	WriteUart
* Input:	UINT_T
* Output:	void
* Desc:		Send command on UART
*/
void CUARTPortLinux::WritePort (UINT32_T dwCmdSize)throw (CWtpException)
{
	int result = 0;
	WTPCOMMAND WtpCmdStruct = WtpCmd->GetWtpCommand();
	result = write( uartLinuxFileDesc,(char *)&WtpCmdStruct,dwCmdSize);
	if (result < 0)
		throw CWtpException(CWtpException::WRITEUARTPORT);
}
/**
* Method:	WritePort
* Input:
* Output:	void
* Desc:		This is a overriden method from Port class.
*			Send command on UART and get response in CWtpstatus object.
*/
void CUARTPortLinux::WritePort() throw (CWtpException)
{
	bool IsOk = true;
	int CmdSize = 8 + WtpCmd->GetLEN(), StatusSize = 6;
	stringstream Message;

	if(WtpCmd->GetCMD() == PREAMBLE)
	{
		CmdSize = 4; StatusSize = 4;
	}
	else if (WtpCmd->GetCMD() == PLAINTEXTWTPCR)
	{
		CmdSize = 4;
		StatusSize = 5;
	}

	WritePort (CmdSize);
	ReadPort(StatusSize);

	if(WtpStatus.GetCMD() == MESSAGE)
	{
		if(!WtpStatus.GetDLEN() > 0)
			WtpStatus.SetStatus(NACK);
		else
			GetWtpRemainingMessage();
	}
}
/**
* Method:	ClosePort
* Input:	N/A
* Output:	void
* Desc:		This is a overriden method from Port class.
*			Closes UART port.
*/
void CUARTPortLinux::ClosePort ()
{
	close (uartLinuxFileDesc);
}
#endif
