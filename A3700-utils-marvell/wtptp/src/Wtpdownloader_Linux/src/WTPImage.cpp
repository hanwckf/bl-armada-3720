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
#include <sstream>
#include <istream>
#include <string.h>
#include "WtpImage.h"
#include "WtpDownloadApp.h"
#include "WtpCommand.h"
#include "Tim.h"

using namespace std;

CWtpImage::CWtpImage(void)
{
	eImageType = 0;
	sImageFilename = "";
	iImageFileSize = 0;
}

CWtpImage::~CWtpImage(void){}

int CWtpImage::GetEImageType()
{
	return this->eImageType;
}

void CWtpImage::SetEImageType(int eImageType)
{
	this->eImageType = eImageType;
}

string& CWtpImage::GetSImageFileName()
{
	return sImageFilename;
}

void CWtpImage::SetSImageFileName(string& sImageFileName)
{
	this->sImageFilename = sImageFileName;
}

int CWtpImage::GetIImageFileSize()
{
	return iImageFileSize;
}

void CWtpImage::SetIImageFileSize(int iImageFileSize)
{
	this->iImageFileSize = iImageFileSize;
}

void CWtpImage::StartDataTransfer(ifstream& ifFile,int numOfBytes)
{
	unsigned int uiBufferSize = 0;
	unsigned int uiRemainingBytes = numOfBytes;
	ostringstream Message;
	CMisc misc;
	long BytesRead = 0;

	theApp.ProtocolManager.DataHeader(uiRemainingBytes);

	if ((theApp.ProtocolManager.GetStatus()->GetFlags() & 0x4) && theApp.bFastDownload )
	{
		Message.str("");
		Message << endl << "*******NOTE:: Fast Download is NOW activated!" << endl;
		misc.UserMessagePrintStr( true, Message.str());
	}
	else
		theApp.bFastDownload = false;

	int length = theApp.ProtocolManager.GetStatus()->GetDLEN();
	for (int i = 0; i < length; i++)
	{
		uiBufferSize |= theApp.ProtocolManager.GetStatus()->GetData()[i] << (8 * i);
	}

    while (uiRemainingBytes > 0)
    {
		if (uiBufferSize > uiRemainingBytes)
			uiBufferSize = uiRemainingBytes;
		uiRemainingBytes -= uiBufferSize;

		Message.str("");
		Message << "Buffer size = " << dec << uiBufferSize  << ",";
		Message << " Bytes remaining = " << uiRemainingBytes << endl;
		misc.UserMessagePrintStr( theApp.bVerbose,Message.str());

		char* buffer = new char[uiBufferSize];
		ifFile.read(buffer,uiBufferSize);

		try
		{
			if ((BytesRead = (long)ifFile.gcount()) != uiBufferSize)
			{
				Message << " should've read " << uiBufferSize;
				throw CWtpException(CWtpException::INSUFFICIENT_BYTES_READ, (int)BytesRead,(std::string)Message.str());
			}

			int totalSize = uiBufferSize;
			int mark = 0;

			//Total amount of data to send = uiBufferSize
			//Download over USB? data size to send = packetSize.
			//Download over UART? data size to send = uiBufferSize
			int packetSize = (theApp.iPortType == USB)? theApp.iUSBPacketSize : uiBufferSize;

			if(theApp.iPortType == USB && theApp.iUSBPacketSize > (int)uiBufferSize)
			{
				Message.str("");
				Message << "Packet size changed from " << packetSize << " to " << uiBufferSize << endl;
				misc.UserMessagePrintStr( theApp.bVerbose,Message.str());
				packetSize = uiBufferSize;
			}

			Message.str("");
			Message << "Packet size = " << dec << packetSize  << endl;
			misc.UserMessagePrintStr( theApp.bVerbose,Message.str());

			while(totalSize > 0)
			{
				if(totalSize <= packetSize)
				{
					packetSize = uiBufferSize;
					if(theApp.bFastDownload && uiRemainingBytes == 0 && theApp.iPortType == USB)
						theApp.ProtocolManager.isLastData(SET, true);
					packetSize = totalSize;
				}
				theApp.ProtocolManager.Data((unsigned char*)&buffer[mark],packetSize);

				mark += packetSize;
				totalSize -= packetSize;
			}
			delete [] buffer;

			if (!theApp.bFastDownload && uiRemainingBytes > 0)
			{
				theApp.ProtocolManager.DataHeader(uiRemainingBytes);
				uiBufferSize = 0;
				for (int i = 0; i < (int)theApp.ProtocolManager.GetStatus()->GetDLEN(); i++)
					uiBufferSize |= (int)theApp.ProtocolManager.GetStatus()->GetData()[i] << (8 * i);
			}
		}

		catch(CWtpException&)
		{
			delete [] buffer;
			throw;
		}
	} // End of download loop
	theApp.ProtocolManager.Done();

	if (theApp.bFastDownload)
		theApp.ProtocolManager.isLastData (SET, false);

	if(ifFile.is_open())
		ifFile.close();
}

/**
* Method:	DownloadImage
* Input:	N/A
* Output:	boolean
* Desc:		Downloads image asked by target. Protocol commands till
*			SELECTIMAGE is executed in RequestImageType method in
*			CWtpDownloadApp class since they are not related to a
*			particular image. VERIFYIMAGE command onwards is executed
*			inside this method since they are related to the image.
*/
void CWtpImage::DownloadImage()
{
	ostringstream Message;
	ifstream ifFile;
    	unsigned int uiRemainingBytes = 0;
	unsigned int uiBufferSize = 0;

	Message.str("");
	theApp.ProtocolManager.VerifyImage(ACK);

	ifFile.open(GetSImageFileName().c_str(), ios_base::in | ios_base::binary );
	if ( !ifFile.is_open() )
		throw CWtpException(CWtpException::FILE_OPEN_FAIL,0,GetSImageFileName());

    if ( ((GetEImageType() & TYPEMASK) == (TIMIDENTIFIER & TYPEMASK)) || (GetEImageType() == PARTIONIDENTIFIER))
	    ifFile.seekg(0,ios::beg); // Set position to SOF
    else
	    ifFile.seekg(4L,ios::beg); // Set position to SOF

	uiRemainingBytes = GetIImageFileSize();
	try
	{
		StartDataTransfer(ifFile,uiRemainingBytes);
	}
	catch(CWtpException&)
	{
		if(ifFile.is_open())
			ifFile.close();
		throw;
	}
}

