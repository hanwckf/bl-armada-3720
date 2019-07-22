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
#include <ostream>
#include <iostream>
#include <string>
#include "WtpImage.h"
#include "CommandlineParser.h"
#include "WtpDownloadApp.h"
#include "Port.h"
#include "TimParser.h"
#if WINDOWS
#include "USBPort.h"
#include "WinUSBPort.h"
#include "WinUSBException.h"
#include "UARTPort.h"
#endif


//#include "FlashCodes.h"
#if LINUX
//#include <unistd.h>
//#include <time.h>
#include <stdlib.h>
#include "UARTPortLinux.h"
#include <cstring>
#endif
CWtpDownloadApp theApp;

CWtpDownloadApp::~CWtpDownloadApp(void)
{
	list<CWtpImage*>::iterator iter;
	iter = lImageFiles.begin();
	while( iter != lImageFiles.end() )
	{
		delete *iter++;
	}
    lImageFiles.clear();
}

string CWtpDownloadApp::ConvertSecondsToTimeString(double seconds)
{
	int hours = (int)(seconds / 3600);
	seconds = (int) seconds % 3600;
	int minutes = (int)seconds / 60;
	seconds = (int) seconds % 60;
	stringstream hhmmss;
	hhmmss << hours << "hr:" << minutes << "min:" << seconds << "s";
	return hhmmss.str();

}

CWtpDownloadApp::CWtpDownloadApp(void)
{
	//init all bools to false
	TARGET_PROTOCOL_VERSION.MajorVersion = 0x01;
	TARGET_PROTOCOL_VERSION.MinorVersion = 0x01;
	TARGET_PROTOCOL_VERSION.Build = 0x0000;

	sLogFilename = "WtpDownload.txt";
	theApp.bPlainTextWtpCR = false;
	bVerbose = false;
	bGetPC = false;
	bTimFile = false;
	bImageFile = false;
	bJtagFile = false;
	bMessageMode = false;
	bPartitionFile = false;
	bUSBInterface = false;
	bUSBPacketSize = false;
	bJtagMedia = false;
	bDownload = false;
	bForceUARTImageLoad = false;
	bForceBootROMConsoleMode = false;
	bSendFFStream = false;
	bSendFFStream_DLoad = false;

	bUARTCommPort = false;
	bUARTBaudRate = false;
	bLogFile = false;
	bPlatform = false;
	bFastDownload = true;
	bUploadSpecFile = false;
	bHelp = false;
	iCommPort = 1;
	iDownloadBaudRate = 115200;
	iUSBPacketSize = PACKET_SIZE;
	iPortType = UNKNOWNPORT;
	ePlatformType = PXAMAX_PT;
	eCurrentMessageMode = MSG_AND_DOWNLOAD;
    bEnumerateDelay = false;
    iEnumerateDelay = 0;
    bEnumerateArriveWait = false;
    iEnumerateArriveWait = 0;
    bUSBReadDelay = false;
    iUSBReadDelay = 0;
	inumImagesinCLtoDL = 0;
    bEnablerExePath = false;
	sEnableExePath = "";

    exePath = "";
}
/**
* Method:	GetImageFromImageList
* Input:	DWORD
* Output:	CWtpImage*
* Desc:		Returns pointer to Image object of type ImageType if in ImageList.
*			else returns 0
*/
CWtpImage* CWtpDownloadApp::GetImageFromImageList (unsigned int ImageType)
{
	t_WtpImageIter iter = lImageFiles.begin();
    while(iter != lImageFiles.end())
	{
		if((*iter)->GetEImageType() == (unsigned int)ImageType)
			return *iter;
		iter++;
	}
	return 0;
}

/**
* Method:	RequestImageType
* Input:	CWtpImage*
* Output:	boolean
* Desc:		Request target for type of Image it needs for download.
*			Implements download protocol till SELECTIMAGE command.
*			After target requests image type, image lookup is done
*			in imagelist and control is handed over to downloadImage method
*			in CWtpImage class where rest of the protocol is implemented.
*/
void CWtpDownloadApp::RequestImageType(CWtpImage*& Image) throw (CWtpException)
{
	ostringstream Message;
	unsigned int ImageType = 0;
	Image = 0;
	bool IsOk = true;
	CMisc misc;
	//Send Preamble Command
	theApp.ProtocolManager.SendPreamble();
	if ((theApp.eCurrentMessageMode == MSG_AND_DOWNLOAD))
		theApp.ProtocolManager.GetTargetMessage();

	//Send GetVersion Command
	ProtocolManager.GetVersion();

	//Send SelectImage Command
	ProtocolManager.SelectImage(&ImageType);

	Message.str("");
	Message << endl << "Target Request Image Type: 0x" << std::hex << ImageType << endl;
	misc.UserMessagePrintStr( theApp.bVerbose, Message.str() );

	//Check if target-requested ImageType exists in ImageList
    Image = theApp.GetImageFromImageList(ImageType);

	if(Image == 0)
	{
		if ( ImageType == PARTIONIDENTIFIER )
			throw CWtpException(CWtpException::NO_PARTITION_TABLE);
		else
		{
			Message.str("");
			Message << ": 0x" << hex << ImageType;
            throw CWtpException(CWtpException::NO_IMAGE, 0, Message.str());
		}
	}
	Message.str("");
	Message << "Downloading file: "<< Image->GetSImageFileName() << endl;
	misc.UserMessagePrintStr( theApp.bVerbose, Message.str());

}

/**
* Method:	DownloadImages
* Input:	boolean
* Output:	boolean
* Desc:		Downloads images in ImageList in the order asked by target.
*/
void CWtpDownloadApp::DownloadImages(int numImages) throw (CWtpException)
{
	int i = 0;
	CWtpImage* Image = 0;
	stringstream Message;
	bool retry= false;
	CMisc misc;

	Message << endl << "Number of files to download: " << lImageFiles.size() << endl;
	misc.UserMessagePrintStr( true, Message.str());




	//Send 'w','t','p','CR' if Plaintext_WTP is selected

	if (theApp.bPlainTextWtpCR)
	{
		if (theApp.iPortType == USB)
		{
			throw CWtpException(CWtpException::NOTSUPPORTED, -1, "Cannot send \"w t p CR\" string over USB!");
		}
		theApp.ProtocolManager.SendPlainTextWtpCR();
	}
	for(i = 0; i < numImages; i++ )
	{
		if(Image!=0 && Image->GetEImageType() == (int)DKBIDENTIFIER)
			break;

		time(&theApp.imgDwnldStartTime);
		try
		{
			RequestImageType(Image);
		}
		catch(CWtpException& Err)
		{
            if (Err.getErrorNumber() == (int)CWtpException::NO_IMAGE)
            {
                ProtocolManager.VerifyImage(NACK);
                theApp.ProtocolManager.Done();
                theApp.ProtocolManager.Disconnect();
                throw;
            }
            else if (Err.getErrorNumber() == (int)CWtpException::NO_PARTITION_TABLE)
			{
				Err.printError();
				Message.str("");
				Message << endl << "Error: Continuing with next image" << endl;
				misc.UserMessagePrintStr( true, Message.str());
				retry = true;

				// this retry is used when a target requests a PartitionTable
				// but none is provided.  The retry in effect ignores the
				// request and continues to download the remaining images
				//theApp.UserMessagePrintStr( true, string("\nRetrying this image download!\n"));
				// retry downloading the image that just failed
			}

			else if (theApp.ePlatformType == CWtpDownloadApp::PXA168)
			{
				//Bug fix for old ASPN bootROM. This bug has been fixed in recent
				//BootROM but retain this code for backward compatibility
				Err.printError();
				Message.str("");
				Message << endl << "Platform Type: 0x"  << theApp.ePlatformType << endl;
				misc.UserMessagePrintStr( true, Message.str());
				Message.str("");
				Message << endl << "Retrying download of all images!" << endl;
				misc.UserMessagePrintStr( true, Message.str());
				retry = true;
				i = -1;// retry all images
			}
			else
				throw;
		}
		if(retry == true)
		{
			ProtocolManager.VerifyImage(NACK);
			ProtocolManager.Done();
			retry = false;
			i--;
			if ( theApp.eCurrentMessageMode == MSG_AND_DOWNLOAD )
			{
				ProtocolManager.GetPort()->GetWtpStatus()->ResetWtpStatus();
				ProtocolManager.GetPort()->GetWtpStatus()->SetFlags(1);
				ProtocolManager.GetTargetMessage();
			}
			if (theApp.eCurrentMessageMode == DOWNLOAD_ONLY)
			{
				ProtocolManager.GetPort()->GetWtpStatus()->ResetWtpStatus();
				ProtocolManager.GetPort()->GetWtpStatus()->SetFlags(1);
			}
			continue;
		}
		try
		{
			Image->DownloadImage();
			time(&theApp.imgDwnldEndTime);
			Message.str("");
			Message << endl << "Download time for image " << i + 1 << "->" ;
			Message << ConvertSecondsToTimeString(difftime(theApp.imgDwnldEndTime,theApp.imgDwnldStartTime));
			misc.UserMessagePrintStr( true, Message.str() );
#if 0   // sah removed for lack of 64-bit IPPCP
#if TRUSTED
			// After downloading TIM, allow for a JTAG reenablement.
			if ((Image->GetEImageType() ==(int)TIMIDENTIFIER) && bJtagFile)
			{
                try
                {
                    Message.str("");
                    Message << endl << "JTAG Reenablement Mode" << endl;
                    misc.UserMessagePrintStr(true, Message.str());
                    ProtocolManager.JtagReEnable();
                }
                catch (CWtpException&)
                {
                    Message.str("");
                    Message << endl << " Error: JTAG Re-enable failed! " << endl;
                    misc.UserMessagePrintStr(true, Message.str());
                    ProtocolManager.Done();
                    ProtocolManager.Disconnect();
                    retry = false;
                    throw;
                }
			}
#endif
#endif
		}
		catch(CWtpException&)
		{
			Message.str("");
			Message << endl << " Error: Download failed for file number #" << i + 1 << endl;
			misc.UserMessagePrintStr(true,Message.str());
			throw;
		}
		Message.str("");
		Message << endl << "Success: Download file complete for image "  << i + 1 <<endl;
		misc.UserMessagePrintStr( true, Message.str() );
	}
}

/**
* Method:	Upload
* Input:	N/A
* Output:	boolean
* Desc:		Implements upload feature of protocol.
*/
void CWtpDownloadApp::Upload() throw (CWtpException)
{
	ProtocolManager.SendPreamble();
	ProtocolManager.GetTargetProtocolVersion();

	if(ProtocolManager.TargetPV.MajorVersion >= theApp.TARGET_PROTOCOL_VERSION.MajorVersion &&
		ProtocolManager.TargetPV.MinorVersion >= theApp.TARGET_PROTOCOL_VERSION.MinorVersion &&
		ProtocolManager.TargetPV.Build >= theApp.TARGET_PROTOCOL_VERSION.Build)
	{
		ProtocolManager.GetParameters();
		ProtocolManager.DoUploads();
	}
}

/**
* Method:	ListenForMessages
* Input:	N/A
* Output:	void
* Desc:		MESSAGE_ONLY mode operation:
*			Listens for messages from target after sending preamble command
*/
void CWtpDownloadApp::ListenForMessages()
{
	bool Connected = false;
	stringstream Message;
	CMisc misc;
	Message << "******************************" << endl;
	Message << "**MESSAGE_ONLY mode enabled***" << endl;
	Message << "******************************" << endl;
	misc.UserMessagePrintStr( true,Message.str());

	while(!Connected)
	{
		try
		{
			ProtocolManager.SendPreamble();
			Connected = true;
		}
		/*Not an Exception. Keep trying to connect to target until PREAMBLE acks*/
		catch(CWtpException&){}
	}

	while (true)
	{
		ProtocolManager.GetWtpMessage();
		if ( (ProtocolManager.GetPort()->GetWtpStatus()->GetFlags() & 0x1) == MESSAGEPENDING )
			ProtocolManager.GetTargetMessage();
		else
			return;
	}
}


/**
* Method:	GetPC
* Input:	N/A
* Output:	int (target version)
* Desc:		Get the PC from the target
*/
void CWtpDownloadApp::GetPC() throw (CWtpException)
{
	ProtocolManager.SendPreamble();
	if ( theApp.eCurrentMessageMode == MSG_AND_DOWNLOAD )
		theApp.ProtocolManager.GetTargetMessage();

//	ProtocolManager.GetTargetProtocolVersion();
	ProtocolManager.GetPC();
	ProtocolManager.Done();
}

/**
* Method:	main
* Input:	int, char*[]
* Output:	int
* Desc:		Main
*/
int main(int argc,char* argv[])
{
	int fileCount = 1;
	size_t slashIndex = string::npos;
	bool bIsOk = true,Connected=false,SentDisconnect = false,IsPortOpen = false;
	CTimParser TimParser;
	stringstream Message;
	CMisc misc;
	//CUARTPort UartPort;
	char * dummybuffer = new char[8];
	char * dummybuffer1 = new char[1]; // a single byte buffer
	char * response = new char[1];
	response[0] = 0x0d;
	char * response4 = new char[4];
	response4[0] = 0x0d;
	response4[1] = 0x0d;
	response4[2] = 0x0d;
	response4[3] = 0x0d;

	int numchars = 0;

	slashIndex = string(argv[0]).find_last_of("/\\");
	if(slashIndex != string::npos)
		theApp.exePath = string(argv[0]).substr(0,slashIndex+1);

	time(&theApp.downloadStartTime);

	char tmpbuf[100];
    time_t starttime = time(NULL);
    struct tm st ={0};
#if LINUX
    localtime(&starttime);
    memcpy(&st, localtime(&starttime), sizeof(st));
#else
    localtime_s(&st,&starttime);
#endif
    strftime( tmpbuf, 100, "%m/%d/%y %H:%M:%S", &st );
    printf( "Start time: %s\n", tmpbuf );

	//Message << "Download Start Time: " << ctime(&theApp.downloadStartTime) << endl;
#if	X64_BUILD
	Message << "WtpDownload for 64-bit Host (Target Images Downloading Tool)" << endl;
#else
	Message << "WtpDownload for 32-bit Host (Target Images Downloading Tool)" << endl;
#endif

#if TRUSTED
	Message << endl << "Trusted version for TIM and NTIM" << endl;
#else
	Message << endl << "Non-trusted only version for NTIM" << endl;
#endif
	Message << "Version:      " << g_Version << endl;
	Message << "Release Date: " << g_Date << endl;
	misc.UserMessagePrintStr( true, Message.str());
	Message.str("");
	try
	{

		if(argc < MIN_ARGS)
			throw CWtpException(CWtpException::NO_MIN_ARGS);

		theApp.Parser = new CCommandlineParser;
		theApp.Parser->Parse(argc,argv);

		if(theApp.bHelp)
			misc.PrintUsage();

		if(!theApp.bVerbose)
		{
			Message << endl << "Verbose mode disabled..." << endl;
			misc.UserMessagePrintStr(true, Message.str());
		}

        //NEW API
        unsigned int uiExceptionCode = 0;
#if WINDOWS
		if (theApp.iPortType == USB)
		{

			try
			{
				string sInterfaceName = "";
				theApp.ProtocolManager.m_USBInterfaceAPI.InitializePort(sInterfaceName, uiExceptionCode);
				if (theApp.ProtocolManager.m_USBInterfaceAPI.APSE_WinUSBDriver())
				{
					theApp.ProtocolManager.SetWtpPort(new CWinUSBPort);
					Message << endl << "Using APSE_WinUSB Driver..." << endl;
					misc.UserMessagePrintStr(true, Message.str());
				}
				else
				{
					theApp.ProtocolManager.SetWtpPort(new CUSBPort);
					Message << endl << "Using WTPTP.sys Driver..." << endl;
					misc.UserMessagePrintStr(true, Message.str());
				}
			}
			catch (CWinUSBException& e)
			{
				uiExceptionCode = e.getErrorNumber();
				return false;
			}
		}
#endif
//        theApp.ProtocolManager.m_USBInterfaceAPI.OpenPort(uiExceptionCode);

        //END NEW API

		// Before continuing with a download, we need to check if we are trying to force either UART mode or Console mode through an escape sequence.
		// Typically this is done with hardware switches, but this method will enable remote control.

		if (theApp.bForceBootROMConsoleMode || theApp.bForceUARTImageLoad || theApp.bSendFFStream || theApp.bSendFFStream_DLoad) {

			if (theApp.iPortType == USB)
			{
				throw CWtpException(CWtpException::NOTSUPPORTED, -1, "Cannot send Console or UART force sequences string over USB!");
			}

			if ((theApp.bForceBootROMConsoleMode && theApp.bForceUARTImageLoad) ||
				(theApp.bForceBootROMConsoleMode && theApp.bSendFFStream) ||
				(theApp.bForceBootROMConsoleMode && theApp.bSendFFStream_DLoad) ||
				(theApp.bForceUARTImageLoad && theApp.bSendFFStream) ||
				(theApp.bForceUARTImageLoad && theApp.bSendFFStream_DLoad) ||
				(theApp.bSendFFStream && theApp.bSendFFStream_DLoad)) {
					Message << endl << "Illegal Parameters: UART Download Mode(-O) and BootROM Console mode(-Y)  and Send FF Stream(-y) and Send FF Stream Dload(-o) are mutually exclusive." << endl;
					misc.UserMessagePrintStr(true, Message.str());
			}
			else {
				//cout << "here man" << endl;
				theApp.ProtocolManager.GetPort()->SetCommReadTimeout(0);
				theApp.ProtocolManager.GetPort()->OpenPort();
				IsPortOpen = true;

				if (theApp.bForceBootROMConsoleMode) {

					theApp.ProtocolManager.SendContinuousForceConsoleMode();
					switch (theApp.iDownloadBaudRate) {
					case 115200:
						theApp.ProtocolManager.GetPort()->ClosePort();
						IsPortOpen = false;

						theApp.ProtocolManager.GetPort()->SetCommReadTimeout(1);

						theApp.ProtocolManager.GetPort()->OpenPort();
						IsPortOpen = true;

						break;
					case 300:
						theApp.ProtocolManager.GetPort()->ClosePort();
						IsPortOpen = false;

						theApp.ProtocolManager.GetPort()->SetCommReadTimeout(1000);

						theApp.ProtocolManager.GetPort()->OpenPort();
						IsPortOpen = true;
						break;
					default:
						theApp.ProtocolManager.GetPort()->ClosePort();
						IsPortOpen = false;

						theApp.ProtocolManager.GetPort()->SetCommReadTimeout(1);

						theApp.ProtocolManager.GetPort()->OpenPort();
						IsPortOpen = true;
						break;

					}

					theApp.ProtocolManager.SendContinuousForceConsoleMode();

					//Sleep(2000);
					theApp.ProtocolManager.GetPort()->WriteRaw(response4, 4);


					//Sleep(2000);

					do {
						numchars = theApp.ProtocolManager.GetPort()->ReadRaw(dummybuffer1, 1);
					} while (numchars > 0);

					exit(0);

				}

				if (theApp.bSendFFStream) {
					theApp.ProtocolManager.SendContinuousFFStream();
					theApp.ProtocolManager.GetPort()->WriteRaw(response4, 4);

					do {
						numchars = theApp.ProtocolManager.GetPort()->ReadRaw(dummybuffer1, 1);
					} while (numchars > 0);

					exit(0);
				}

				if(theApp.bSendFFStream_DLoad) {
					theApp.ProtocolManager.SendContinuousFFStream_DLoad();
					theApp.ProtocolManager.GetPort()->WriteRaw(response4, 4);

					theApp.ProtocolManager.GetPort()->ClosePort();
					IsPortOpen = false;

					theApp.ProtocolManager.GetPort()->SetCommReadTimeout(1000);

					theApp.ProtocolManager.GetPort()->OpenPort();
					IsPortOpen = true;


					do {
						numchars = theApp.ProtocolManager.GetPort()->ReadRaw(dummybuffer1, 1);
					} while (numchars > 0);


				}

				if (theApp.bForceUARTImageLoad) {
					theApp.ProtocolManager.SendContinuousForceUARTMode();

					switch(theApp.iDownloadBaudRate) {
					case 115200:
						theApp.ProtocolManager.GetPort()->ClosePort();
						IsPortOpen = false;

						theApp.ProtocolManager.GetPort()->SetCommReadTimeout(1);

						theApp.ProtocolManager.GetPort()->OpenPort();
						IsPortOpen = true;
						break;
					case 300:
						theApp.ProtocolManager.GetPort()->ClosePort();
						IsPortOpen = false;

						theApp.ProtocolManager.GetPort()->SetCommReadTimeout(1000);

						theApp.ProtocolManager.GetPort()->OpenPort();
						IsPortOpen = true;
						break;
					default:
						theApp.ProtocolManager.GetPort()->ClosePort();
						IsPortOpen = false;

						theApp.ProtocolManager.GetPort()->SetCommReadTimeout(1);

						theApp.ProtocolManager.GetPort()->OpenPort();
						IsPortOpen = true;
						break;

					}

					theApp.ProtocolManager.SendContinuousForceUARTMode();
					//theApp.ProtocolManager.GetPort()->SetCommReadTimeout(1000);
					//theApp.ProtocolManager.GetPort()->ReadRaw(dummybuffer, 8);

					//Sleep(2000);
					theApp.ProtocolManager.GetPort()->WriteRaw(response4, 4);


					//Sleep(2000);

					do {
						numchars = theApp.ProtocolManager.GetPort()->ReadRaw(dummybuffer1, 1);
					} while (numchars > 0);

				}


				theApp.ProtocolManager.GetPort()->ClosePort();
				IsPortOpen = false;

			}
		}

		theApp.ProtocolManager.GetPort()->SetCommReadTimeout(1000);
//#endif

        if (theApp.bGetPC)
		{
			if(theApp.iPortType == UNKNOWNPORT)
				throw CWtpException(CWtpException::INVALID_PORTTYPE);

			theApp.ProtocolManager.GetPort()->OpenPort();
			IsPortOpen = true;

			theApp.GetPC();
			theApp.ProtocolManager.Disconnect();
			SentDisconnect = true;
			goto Exit;
		}
#if WINDOWS
		if(theApp.bJtagMedia)
			theApp.ProtocolManager.JtagOverMedia(theApp.jtagArgsString);
#endif





		if(theApp.bDownload)
		{
			if(theApp.iPortType == UNKNOWNPORT)
				throw CWtpException(CWtpException::INVALID_PORTTYPE);

			theApp.ProtocolManager.GetPort()->OpenPort();
			IsPortOpen = true;




			if(theApp.bTimFile)
			{
				TimParser.ParseTim();									// Mainly used for sanity checking.
				theApp.DownloadImages(theApp.inumImagesinCLtoDL);		// Downloads based on image list specified at the command line.
			}
#if 0 // removed for lack of 64 bit IPPCP
#if TRUSTED
            else if (!theApp.bTimFile && theApp.bJtagFile)
            {
                try
                {
                    Message.str("");
                    Message << endl << "JTAG Reenablement Mode" << endl;
                    misc.UserMessagePrintStr(true, Message.str());
                    theApp.ProtocolManager.JtagReEnable();
                }
                catch (CWtpException&)
                {
                    Message.str("");
                    Message << endl << " Error: JTAG Re-enable failed! " << endl;
                    misc.UserMessagePrintStr(true, Message.str());
                    theApp.ProtocolManager.Done();
                    theApp.ProtocolManager.Disconnect();
                    SentDisconnect = true;
                    throw;
                }
            }
#endif
#endif
			if(theApp.bUploadSpecFile)
				theApp.Upload();

			if (theApp.eCurrentMessageMode == MSG_ONLY )
			{
				theApp.ListenForMessages();
#if 0
				theApp.ProtocolManager.Disconnect();
				SentDisconnect = true;
#endif
			}
			theApp.ProtocolManager.Disconnect();
			SentDisconnect = true;
		}
	}

	catch(CWtpException& Err)
	{
        Err.printError();
        bIsOk = false;
	}

Exit:

	if(IsPortOpen)
		theApp.ProtocolManager.GetPort()->ClosePort();
	delete theApp.ProtocolManager.GetPort();
	delete theApp.Parser;

	time(&theApp.downloadEndTime);
	Message.str("");
	if(!bIsOk)
	{
		Message << endl << "WtpDownload Incomplete!" << endl;
		Message << "Failure: WtpDownload Exiting with Failure Code!" << endl;
	}
	else
	{
		Message << endl << "WtpDownload Complete! ";
#ifdef LINUX
		Message << "Device:" << theApp.sUSBInterface << endl;
#endif
		Message << endl << "Success: WtpDownload Exiting with Success Code!" << endl;
	}

	if(theApp.bLogFile)
	{

		Message << "Check download results in log file:" << theApp.sLogFilename << endl;
		theApp.hLog.close();
	}
	time_t endtime = time(NULL);
    struct tm et ={0};
#if LINUX
    localtime(&endtime);
    memcpy(&st, localtime(&starttime), sizeof(et));
#else
    localtime_s(&et,&endtime);
#endif
    strftime( tmpbuf, 100, "%m/%d/%y %H:%M:%S", &et );
    printf( "\nFinish time: %s\n", tmpbuf );
	double timetaken = difftime(endtime,starttime);

	Message <<"Download Time:" << theApp.ConvertSecondsToTimeString(timetaken) ;
	misc.UserMessagePrintStr(true,Message.str());
	if(bIsOk)
		return 0;
	return 1;
}
