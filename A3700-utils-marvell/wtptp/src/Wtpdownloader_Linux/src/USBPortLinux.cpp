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
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include "USBPortLinux.h"
#include "WtpDownloadApp.h"
#include "WtpCommand.h"
#include "WTPDEF.h"
#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>
#if LINUX
#include <iomanip>
#endif
using namespace std;

CUSBPortLinux::~CUSBPortLinux(void){}
CUSBPortLinux::CUSBPortLinux(void){}

/**
* Method:	GetDeviceName
* Input:	N/A
* Output:	void
* Desc:		Identify MARVELL USB device and intialize for communication
*/
void CUSBPortLinux::getDeviceName() throw (CWtpException)
{
	FILE* fp;
	char* buffer = new char[256];
	string cmd = "ls /dev/MARVELL:*";
	fp = popen(cmd.c_str(),"r");

	while(fgets(buffer,256,fp) != NULL)
	{
		string temp = buffer;
		lVidList.push_back(temp.substr(0,temp.length() -1));
	}
	pclose(fp);
	if(lVidList.empty())
		throw CWtpException(CWtpException::OPENUSBPORT,0,"Marvell device(s) not found!");

	if(theApp.bUSBInterface)
	{
		bool deviceFound = false;
		VidIter = lVidList.begin();
		while(!lVidList.empty())
		{
			string device = *VidIter;
			if(device.find(theApp.sUSBInterface))
			{
				deviceFound = true;break;
			}
			VidIter++;
		}
		if(deviceFound)
			UsbDeviceName.append("/dev/" + theApp.sUSBInterface);
		else
			throw CWtpException(CWtpException::OPENUSBPORT,0,UsbDeviceName + " not found!");
	}
	else
	{
		if(lVidList.size() > 1)
		{
			cout << "Note: There is more than one MARVELL device connected to system" << endl;
			cout << "If you want to specify which device to download, please use the -G option" << endl;
			cout << "For current download the first device in the list is considered:" << endl;
		}
		theApp.sUSBInterface = lVidList.front();
		UsbDeviceName.append(lVidList.front());
	}
}

/**
* Method:	OpenPort
* Input:	N/A
* Output:	void
* Desc:		Overriden method of Port class.
*			Opens USB port
*/
void CUSBPortLinux::OpenPort () throw (CWtpException)
{
	struct termios options, old_options;
	getDeviceName();
	cout << "Opened Port to device: " <<  UsbDeviceName << endl;
	usbLinuxFileDesc = open(UsbDeviceName.c_str(), O_RDWR | O_NOCTTY);

	if (usbLinuxFileDesc < 0)
		throw CWtpException(CWtpException::OPENUSBPORT,0,UsbDeviceName);

	fcntl(usbLinuxFileDesc, F_SETFL, 0);
	cfsetispeed(&options, B115200);
	cfsetospeed(&options, B115200);
	options.c_lflag     &= ~(ICANON|ECHO|ECHOE|ISIG|ECHONL|IEXTEN);
	options.c_oflag     &= ~OPOST;
	options.c_iflag	    &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	options.c_cc[VMIN]   = 0;
	options.c_cc[VTIME]  = 10;
	options.c_cflag &= ~(CSIZE|PARENB);
	options.c_cflag |= CS8;
	tcsetattr(usbLinuxFileDesc, TCSANOW, &options);
}

void CUSBPortLinux::WriteRaw(char * buffer, DWORD count) throw(CWtpException)
{
}

int CUSBPortLinux::ReadRaw(char * buffer, DWORD count)
{
	return 0;
}

void CUSBPortLinux::SetCommReadTimeout(DWORD msecs)
{
//	CommTimeOuts.ReadTotalTimeoutMultiplier = msecs;

}


/**
* Method:	WritePort
* Input:	DWORD
* Output:	void
* Desc:		Writes CWtpCommand message to USB port
*/
void CUSBPortLinux::WritePort (UINT32_T dwCmdSize)throw (CWtpException)
{
	int result = 0;
	WTPCOMMAND WtpCmdStruct = WtpCmd->GetWtpCommand();
	if(theApp.bFastDownload && WtpCmdStruct.CMD == DATA)
	{
		//cout << "Bytes to write:" << endl;
		for (int i= 1 ; i <= WtpCmdStruct.LEN; i++)
		{
			cout << hex << setfill('0') << setw(2) << "0x" << (unsigned int)WtpCmdStruct.Data[i-1] << " ";
		}
		result = write( usbLinuxFileDesc,(char *)(WtpCmdStruct.Data), WtpCmdStruct.LEN);

	}
	else
	{
		result = write( usbLinuxFileDesc,(char *)(&WtpCmdStruct),dwCmdSize);
		//WtpCmd->print();
	}
	//cout << "Number of bytes written:" << result << endl;
	if (result < 0)
		throw CWtpException(CWtpException::WRITEUSBPORT);
}

/**
* Method:	WritePort
* Input:	CWtpCommand, CWtpStatus
* Output:	void
* Desc:		Overriden method of Port class.
*			Writes CWtpCommand message to port and
*			gets response in CWtpStatus object
*/
void CUSBPortLinux::WritePort() throw (CWtpException)
{
	bool IsOk = true;
	int CmdSize = 8 + WtpCmd->GetLEN(), StatusSize = 6;
	if(WtpCmd->GetCMD() == PREAMBLE)
	{
		CmdSize = 4; StatusSize = 4;
	}
	WritePort (CmdSize);
	if(theApp.bFastDownload && WtpCmd->GetCMD() == DATA && !theApp.ProtocolManager.isLastData(GET,FDMODE));
	else
		ReadPort(StatusSize);
	if(WtpCmd->GetCMD() == MESSAGE)
	{
		if (!(WtpStatus.GetDLEN() > 0))
			WtpStatus.SetStatus(NACK);
		else
			GetWtpRemainingMessage();
	}
}
/**
* Method:	ReadPort
* Input:	CWtpStatus, DWORD
* Output:	void
* Desc:		Overriden method of Port class.
*			Reads CWtpStatus message sent by target
*/
void CUSBPortLinux::ReadPort(UINT32_T dwSize)throw (CWtpException)
{
	int result = 0;
	int recievedBytes = 0, remainingBytes = dwSize;
	char* buffer = new char[dwSize];
	memset(buffer,0,dwSize);
	while(remainingBytes > 0)
	{
		result = read(usbLinuxFileDesc, buffer + recievedBytes, remainingBytes);
		//cout << "Number of bytes read:" << result << endl;
		if(result == 0)
			throw CWtpException(CWtpException::READUSBPORT);
		remainingBytes -= result;
		recievedBytes += result;
	}
	memcpy(&(WtpStatus.GetWtpStatus()),buffer,recievedBytes);
	if(WtpStatus.GetDLEN() > 0)
		remainingBytes = WtpStatus.GetDLEN();
	else
	if(WtpStatus.GetCMD() == UPLOADDATA)
		remainingBytes = theApp.ProtocolManager.UploadPacketLength;

	if(remainingBytes > 0)
	{
		delete[] buffer;
		buffer = new char[remainingBytes];
		result = 0;
		recievedBytes = 0;
		while(remainingBytes > 0)
		{
			result = read(usbLinuxFileDesc,buffer + recievedBytes,remainingBytes);
			//cout << "Number of bytes read:" << result << endl;
			if(result == -1)
				throw CWtpException(CWtpException::READUSBPORT);
			remainingBytes -= result;
			recievedBytes += result;
		}
		memcpy(WtpStatus.GetData(),buffer,recievedBytes);
	}
	delete[] buffer;
}

/**
* Method:	ClosePort
* Input:	N/A
* Output:	void
* Desc:		Overriden method of Port class.
*		Closes USB port
*/
void CUSBPortLinux::ClosePort ()
{
	close (usbLinuxFileDesc);
}
#endif
