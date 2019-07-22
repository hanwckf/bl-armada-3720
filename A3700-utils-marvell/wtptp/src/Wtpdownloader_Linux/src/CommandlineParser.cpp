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
//#include <iostream>
#include <algorithm>
#include "CommandlineParser.h"
#include "WtpImage.h"
#include "WtpDownloadApp.h"

#if LINUX
#include "UARTPortLinux.h"
#include "USBPortLinux.h"
#include <unistd.h>//get current directory
#define GetParentDirectory getcwd
#else
//#if WINUSB
#include "WinUSBPort.h"
//#else
#include "USBPort.h"
//#endif
#include "UARTPort.h"
#include <direct.h> //get current directory
#define GetParentDirectory _getcwd
#endif
#include "Tim.h"
#include "WTPDEF.h"
using namespace std;

CCommandlineParser::CCommandlineParser(void){}

CCommandlineParser::~CCommandlineParser(void){}

/**
* Method:	InsufficientParameters
* Input:	unsigned char
* Output:	void
* Desc:		Constructs relevant Error Messages to be output
*			as part of the CWtpException exception
*/
void CCommandlineParser::InsufficientParameters(unsigned char eCmdLineOpt) throw (CWtpException)
{
	stringstream Message1,Message2,Message;
	CMisc misc;

	Message1 << endl << "Error: Command Line Parsing Error: " << (char)eCmdLineOpt << " switch: ";
	Message2 << endl << "Error: Missing ";
	switch(eCmdLineOpt)
	{
	case TIM_BIN_FILE:
	case IMAGE_FILE:
	case UPLOAD_DATA:
	case PARTITION_TABLE:
	case LOG_FILE:
	case JTAG_KEY:
					misc.UserMessagePrintStr(theApp.bVerbose, Message1.str());
					Message << endl << "Missing input file name parameter." << endl;
					misc.UserMessagePrintStr( theApp.bVerbose, Message.str());
					throw CWtpException(CWtpException::COMMANDLINE_PARSE_ERROR);

	case MESSAGE_MODE:
						Message2 << " message mode" << endl ;break;
	case PORT_TYPE:
						Message2 << " Port Type" << endl; break;
	case COMM_PORT:
						Message2 << " Comm Port" << endl; break;
	case INTERFACE_NAME:
						Message2 << " Interface Name" << endl; break;
	case USB_PACKET_SIZE:
						Message2 << " USB Packet Size" << endl;break;
	case BAUD_RATE:
						Message2 << " BaudRate ";break;
	case PLATFORM_TYPE:
						if (theApp.ePlatformType < theApp.PXAMIN_PT || theApp.ePlatformType > theApp.PXAMAX_PT)
						{
							Message.str("");
							Message << endl << "Error: Command Line Parsing Error." << endl;
							misc.UserMessagePrintStr( theApp.bVerbose, Message.str());
							Message.str("");
							Message << endl << "Missing or invalid platform type parameter." << endl;
							misc.UserMessagePrintStr( theApp.bVerbose, Message.str());
							throw CWtpException(CWtpException::COMMANDLINE_PARSE_ERROR);
						}
	default: return ;
	}

	misc.UserMessagePrintStr(theApp.bVerbose, Message2.str());
	throw CWtpException(CWtpException::COMMANDLINE_PARSE_ERROR);
}

/**
* Method:	Parse
* Input:	int, char*[]
* Output:	boolean
* Desc:		This is a overriden method from Parser class.
*			Parses commandline input.
*/
void CCommandlineParser::Parse(int argc, char* argv[]) throw (CWtpException)
{
	int currentOption = 1;
	unsigned char Option = 0;
	ostringstream Message;

	CMisc misc;
	string s,cmdOpt;
	stringstream jtagCmdLine;

	while (currentOption < argc)
	{
		cmdOpt = argv[currentOption];

        // (0x7f) filters 8-bit ascii to 7-bit
        // email/word sometimes automatically converts short - into a long -
        // when cutting/pasting commandlines and this can cause a bad parse
        //cmdOpt = *++argv[currentOption++];
		//Option = *++argv[currentOption++] & 0x7f;

		//Option = toupper(cmdOpt[1]);
		Option = cmdOpt[1];

		currentOption++;



		if((cmdOpt[0] & 0x7f) == '-' && cmdOpt.length() == 3 && Option == 'J')
		{
#if LINUX
			Message << "Jtag Over Media not supported in Linux environment!";
			misc.PrintUsage();
			throw CWtpException(CWtpException::INVALID_COMMANDLINE_OPTION,0,Message.str());
#endif
			theApp.bJtagMedia = true;
			//Option = toupper(cmdOpt[2]);
			Option = cmdOpt[2];
			switch(Option)
			{
                case JTAG_CONNECT_TEST:
                case INQUIRE_TARGET:
				case ENABLE_JTAG_CMD:
				case SET_PARAMETERS_CMD:
				case FORCE_RESET:
				case PARAM_FORCE_DOWNLOAD:
				case PARAM_FORCE_UART_LOG:
				case AC_PERMANENTLY_DISABLE:
				case AC_FA_STATE:
				case USE_WTM_TO_RUN_PROTOCOL:
				case ENABLER_HELP:
                case TRACE:
                case ENABLER_VERBOSE:
					jtagCmdLine << "-" << Option << " ";
					break;

				//case USB_INTERFACE_NAME:
				case JTAG_KEY_FILE:
                case SIGNED_UID_KEY_FILE:
                    jtagCmdLine << "-" << Option << " ";
					jtagCmdLine << "\"" << argv[currentOption++] << "\"" << " ";
					break;

				case PROC_TYPE:
				case CORES_TO_ENABLE:
				case PARAM_BOOT_STATE:
				case AC_REQUEST_TYPE:
				case CHANGE_TCK_FREQ:
				case PORTTYPE:
				case COMMPORT:
                case BAUDRATE:
                case TARGET_TIMEOUT:
                case POLLING_DELAY:
                case GET_KEY_PROVIDER:
                    jtagCmdLine << "-" << Option << " ";
					jtagCmdLine << argv[currentOption++] << " ";
					break;

                default:
					Message.str("");
					Message << cmdOpt;
					throw CWtpException(CWtpException::INVALID_COMMANDLINE_OPTION,0,Message.str());
			}
			continue;
		}
		else if((cmdOpt[0] & 0x7f) == '-' && cmdOpt.length() == 2)
		{
            if(currentOption == argc)
			    InsufficientParameters(Option);

			switch (Option)
			{
				case HELP:
					theApp.bHelp = true;
					break;

				case PLAINTEXT_WTPCR:
					theApp.bPlainTextWtpCR = true;
					break;

				case VERBOSE:
					theApp.bVerbose = true;
					break;

				case GET_PC:
					theApp.bGetPC = true;
					Message.str("");
					Message << endl << "Warning: GetPC will execute and immediately disconnect and exit WtpDownload." << endl;
					break;

				case TIM_BIN_FILE:
					theApp.bDownload = true;
					AddImageToList(argv[currentOption++],TIMIDENTIFIER);
					theApp.bTimFile = true;
					theApp.inumImagesinCLtoDL++;
					break;

				case IMAGE_FILE:
					AddImageToList(argv[currentOption++],OBMIDENTIFIER);
					theApp.bImageFile = true;
					theApp.inumImagesinCLtoDL++;
					break;

				case JTAG_KEY:
					theApp.bDownload = true;
					AddImageToList(argv[currentOption++],JTAGIDENTIFIER);
					theApp.bJtagFile = true;
					theApp.inumImagesinCLtoDL++;
					break;

				case UPLOAD_DATA:
					theApp.bDownload = true;
					theApp.sUploadSpecFileName = argv[currentOption++];
					theApp.bUploadSpecFile = true;
					break;

				case PARTITION_TABLE:
					AddImageToList(argv[currentOption++],PARTIONIDENTIFIER);
					theApp.bPartitionFile = true;
					theApp.inumImagesinCLtoDL++;
					break;

                case ENABLER_EXE_PATH:
					theApp.bEnablerExePath = true;
					theApp.sEnableExePath  = argv[currentOption++];
					break;

                    break;

				case MESSAGE_MODE:
					theApp.bMessageMode = true;
					if ( (eMessageMode)atoi (argv[currentOption]) == 1 )
						theApp.eCurrentMessageMode = DISABLED;
					else if ( (eMessageMode)atoi (argv[currentOption]) == 2 )
						theApp.eCurrentMessageMode = MSG_ONLY;
					else if  ( (eMessageMode)atoi (argv[currentOption]) == 3 )
						theApp.eCurrentMessageMode = MSG_AND_DOWNLOAD;
					else if ((eMessageMode)atoi(argv[currentOption]) == 4)
						theApp.eCurrentMessageMode = DOWNLOAD_ONLY;
					else
						throw CWtpException(CWtpException::INVALID_MESSAGEMODE);
					currentOption++;
					break;

				case PORT_TYPE:
					s = argv[currentOption++];
					transform(s.begin(),s.end(),s.begin(),::toupper);
					if (s.find("USB") != string::npos)
					{
						theApp.iPortType = USB;
#if LINUX
						theApp.ProtocolManager.SetWtpPort(new CUSBPortLinux);
//#else
//#if WINUSB
//  					theApp.ProtocolManager.SetWtpPort(new CWinUSBPort);
//#else
//						theApp.ProtocolManager.SetWtpPort(new CUSBPort);
//#endif
#endif
					}
					else if (s.find("UART") != string::npos)
					{
#if LINUX
						theApp.ProtocolManager.SetWtpPort(new CUARTPortLinux);
#else
						theApp.ProtocolManager.SetWtpPort(new CUARTPort);
#endif
						theApp.iPortType = UART;
					}
					else
						throw CWtpException(CWtpException::INVALID_PORTTYPE);
					break;

				case COMM_PORT:
					theApp.iCommPort = atoi(argv[currentOption++]);
					theApp.bUARTCommPort = true;
					break;

				case INTERFACE_NAME:
					theApp.bUSBInterface = true;
					theApp.sUSBInterface = argv[currentOption++];
					break;

				case USB_PACKET_SIZE:
					theApp.bUSBPacketSize = true;
					theApp.iUSBPacketSize = atoi(argv[currentOption++]);
					break;

				case BAUD_RATE:
					theApp.bUARTBaudRate = true;
					theApp.iDownloadBaudRate =  atoi (argv[currentOption++]);
					break;

				case PLATFORM_TYPE:
					theApp.ePlatformType = (CWtpDownloadApp::EPlatformType)atoi(argv[currentOption++]);
					theApp.bPlatform = true;
					break;


				case DISABLE_FAST_DOWNLOAD:
					Message.str("");
					Message << endl << "Warning: Fast Download has been disabled!" << endl;
					misc.UserMessagePrintStr( theApp.bVerbose, Message.str());
					theApp.bFastDownload = false;
					break;

				case LOG_FILE:
					theApp.bLogFile = true;
					theApp.sLogFilename  = argv[currentOption++];
					theApp.hLog.open(theApp.sLogFilename.c_str(),ios::app);
					if(!theApp.hLog.is_open())
						throw CWtpException(CWtpException::OPENLOGFILE);
					break;

				case FORCE_BOOTROM_CONSOLE_MODE:
					theApp.bForceBootROMConsoleMode = true;
					Message.str("");
					Message << endl << "Force BootROM console enabled" << endl;
					misc.UserMessagePrintStr( theApp.bVerbose, Message.str());
					break;

				case SEND_FF_STREAM:
					theApp.bSendFFStream = true;
					Message.str("");
					Message << endl << "Sending FF Stream" << endl;
					misc.UserMessagePrintStr(theApp.bVerbose, Message.str());
					break;

				case SEND_FF_STREAM_DLOAD:
					theApp.bSendFFStream_DLoad = true;
					Message.str("");
					Message << endl << "Sending FF Stream with Download" << endl;
					misc.UserMessagePrintStr(theApp.bVerbose, Message.str());
					break;

				case FORCE_UART_IMAGE_LOAD:
					theApp.bForceUARTImageLoad = true;
					Message.str("");
					Message << endl << "Force UART Image Load enabled" << endl;
					misc.UserMessagePrintStr( theApp.bVerbose, Message.str());
					break;

#if USE_USB_DELAY_SWITCHES == 1
                case ENUMERATE_DELAY:
                    theApp.bEnumerateDelay = true;
                    theApp.iEnumerateDelay = atoi(argv[currentOption++]);
                    break;

                case ENUMERATE_ARRIVE_WAIT:
                    theApp.bEnumerateArriveWait = true;
                    theApp.iEnumerateArriveWait = atoi(argv[currentOption++]);
                    break;

                case USB_READ_DELAY:
                    theApp.bUSBReadDelay = true;
                    theApp.iUSBReadDelay = atoi(argv[currentOption++]);
                    break;
#endif

                /*******************************************************************
					THIS IS DEFEATURED FROM WTPDOWNLOAD BUT RETAINED TO SUPPORT MBU
				*******************************************************************/

				default:
					Message.str("");
					Message << (char)currentOption;
					misc.PrintUsage();
					throw CWtpException(CWtpException::INVALID_COMMANDLINE_OPTION,0,Message.str());
			}
		}
		else
		{
			Message.str("");
			Message << cmdOpt;
			misc.PrintUsage();
			throw CWtpException(CWtpException::INVALID_COMMANDLINE_OPTION,0,Message.str());
		}
	}

	if(theApp.iPortType == UART || theApp.bJtagFile)
		theApp.bFastDownload = false;
	else
		RecalculateUSBPacketSize();


	if(jtagCmdLine.str().length() > 0)
		theApp.jtagArgsString = jtagCmdLine.str();

	CheckYourOptions();
}

/**
* Method:	RecalculateUSBPacketSize
* Input:	int
* Output:	void
* Desc:		Recalculates the USB packet size based on fastdownload
*			was set/reset
*/
void CCommandlineParser::RecalculateUSBPacketSize()
{
	int iSize = theApp.iUSBPacketSize;
	if ( theApp.bFastDownload )
		theApp.iUSBPacketSize = (((iSize+511)/512) * 512);
	else
		theApp.iUSBPacketSize = (((iSize+63)/64) * 64)-8;
}

#if 0
/**
* Method:	AddImageToList
* Input:	char* , DWORD
* Output:	void
* Desc:		Creates an image object  and adds to ImageList list
*/
void CCommandlineParser::AddImageToList(char* FileName, unsigned int ID) throw (CWtpException)
{
	CMisc misc;
	string sFileName = FileName;
	ostringstream Message;
	unsigned long ImageId = ID;

	long lFileSize = 0;

	ifstream ifFile;

	ifFile.open(FileName,ios_base::in | ios_base::binary );
	if ( !ifFile.is_open() )
	{
		char currentDirPath[FILENAME_MAX];
        if (GetParentDirectory(currentDirPath, sizeof(currentDirPath)))
        {
            currentDirPath[sizeof(currentDirPath) - 1] = '\0';
            Message << endl << "Parent Directory:" << currentDirPath << endl;
            misc.UserMessagePrintStr(theApp.bVerbose, Message.str());
            throw CWtpException(CWtpException::FILE_OPEN_FAIL, 0, sFileName);
        }
	}
	CWtpImage* WtpImage = new CWtpImage;
	WtpImage->SetSImageFileName(sFileName);

	// Ron's hack. For boot images, get the actual ID from the NTIM/TIM itself
	if(ImageId == OBMIDENTIFIER)
		ImageId = FileGetImageID(sFileName);

	WtpImage->SetEImageType(ImageId);
	theApp.lImageFiles.push_back(WtpImage);

	ifFile.seekg(0,ios::end);   // Set position to EOF

	ios::pos_type iosSize = ifFile.tellg();
	lFileSize = (long)iosSize;

	if (((ImageId & TYPEMASK) == (TIMIDENTIFIER & TYPEMASK)) || (ImageId == PARTIONIDENTIFIER))
		lFileSize = (long)ifFile.tellg(); // Get last position of file = file size.
	else
	    lFileSize -= 4;
	WtpImage->SetIImageFileSize((int)lFileSize);
	ifFile.seekg(0,ios::beg);

	ifFile.close();
}
#endif

/**
* Method:	AddImageToList
* Input:	char* , DWORD
* Output:	void
* Desc:		Creates an image object  and adds to ImageList list
*/
void CCommandlineParser::AddImageToList(char* FileName, unsigned int ID) throw (CWtpException)
{
	CMisc misc;
	string sFileName = FileName;
	ostringstream Message;
	unsigned long ImageId = ID;

	long lFileSize = 0;

	ifstream ifFile;

	ifFile.open(FileName, ios_base::in | ios_base::binary);
	if (!ifFile.is_open())
	{
		char currentDirPath[FILENAME_MAX];
		if (GetParentDirectory(currentDirPath, sizeof(currentDirPath)))
		{
			currentDirPath[sizeof(currentDirPath)-1] = '\0';
			Message << endl << "Parent Directory:" << currentDirPath << endl;
			misc.UserMessagePrintStr(theApp.bVerbose, Message.str());
			throw CWtpException(CWtpException::FILE_OPEN_FAIL, 0, sFileName);
		}
	}
	CWtpImage* WtpImage = new CWtpImage;
	WtpImage->SetSImageFileName(sFileName);


	if ((ImageId & TYPEMASK) == (TIMIDENTIFIER & TYPEMASK))
	{
		ifFile.seekg(sizeof(int), ios::beg);
		ifFile.read((char *)&ImageId, sizeof(unsigned int));
	}
	else if (ImageId == PARTIONIDENTIFIER)
		ImageId = ID;
	else
		ImageId = FileGetImageID(sFileName);

	WtpImage->SetEImageType(ImageId);
	theApp.lImageFiles.push_back(WtpImage);

	ifFile.seekg(0, ios::end);   // Set position to EOF

	ios::pos_type iosSize = ifFile.tellg();
	lFileSize = (long)iosSize;

	if (((ImageId & TYPEMASK) == (TIMIDENTIFIER & TYPEMASK)) || (ImageId == PARTIONIDENTIFIER))
		lFileSize = (long)ifFile.tellg(); // Get last position of file = file size.
	else
		lFileSize -= 4;
	WtpImage->SetIImageFileSize((int)lFileSize);
	ifFile.seekg(0, ios::beg);

	ifFile.close();
}


/**
* Method:	CheckYourOptions
* Input:
* Output:	boolean
* Desc:		Checks the different combinations of input options
*			that cannot be used together.
*/
void CCommandlineParser::CheckYourOptions() throw (CWtpException)
{
	stringstream Message;
	CMisc misc;
	if( theApp.eCurrentMessageMode == MSG_ONLY )
    {
        if (theApp.bJtagFile ||
			theApp.bTimFile ||
			theApp.bUploadSpecFile)
        	throw CWtpException(CWtpException::PORTTYPE_WITH_MESSAGE_ONLY);
    }
	else if ((theApp.eCurrentMessageMode == MSG_AND_DOWNLOAD) || (theApp.eCurrentMessageMode == DOWNLOAD_ONLY))
    {
		if(theApp.bJtagMedia && theApp.bJtagFile)
		{
			Message << endl << " Error: JtagOverMedia and Jtag Re-enable with WTPTP cannot be combined!" << endl;
			throw CWtpException(CWtpException::INVALID_COMMANDLINE_COMBINATION,0,Message.str());
		}

		//USB and UART specific flags cannot be combined
		if((theApp.bUARTBaudRate || theApp.bUARTCommPort) && (theApp.bUSBInterface || theApp.bUSBPacketSize))
		{
			Message << endl << " Error: USB port options and UART port options cannot be combined!" << endl;
			throw CWtpException(CWtpException::INVALID_COMMANDLINE_COMBINATION,0,Message.str());
		}

		if((theApp.bJtagMedia || theApp.bTimFile || theApp.bUploadSpecFile || theApp.bJtagFile) && theApp.bHelp)
		{
			Message << endl << " Error: Help option cannot be combined with any features of WtpDownload " << endl;
			throw CWtpException(CWtpException::INVALID_COMMANDLINE_COMBINATION,0,Message.str());
		}
    }
    else
    {
		//Message Mode : DISABLED
		throw CWtpException(CWtpException::NO_FEATURE_IN_DISABLED_MSG_MODE);
    }
}
/**
* Method:	FileGetImageID
* Input:	string
* Output:	DWORD
* Desc:		Opens and reads first word from binary file to get ImageType
*/
unsigned int CCommandlineParser::FileGetImageID(string fileName) throw (CWtpException)
{
	ifstream ifsImage;
	unsigned int FileId = 0;
	stringstream Message;

	ifsImage.open(fileName.c_str(),ios::binary | ios::in);
	if (!ifsImage.is_open())
	{
		throw CWtpException(CWtpException::FILE_OPEN_FAIL,0,fileName);
	}
	ifsImage.seekg(ios::beg);
	ifsImage.read((char *)&FileId,sizeof(unsigned int));
	ifsImage.close();
	return FileId;
}
