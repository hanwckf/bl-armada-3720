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
#include <iostream>
#include <string.h>
#include "Misc.h"
#include "WtpDownloadApp.h"
#include "CommandlineParser.h"

using namespace std;

CMisc::CMisc(void){}

CMisc::~CMisc(void){}

/**
* Method:	CompareUploadToFile
* Input:	unsigned char* , string , unsigned int
* Output:	boolean
* Desc:		Compares iDataSize bytes of data in pUploadDataBuffer
*			with iDataSize bytes of file contents of file with filename sFileName
*/
bool CMisc::CompareUploadToFile( unsigned char* pUploadDataBuffer, string& sFileName,unsigned int iDataSize )
{
	if ( pUploadDataBuffer == 0 || iDataSize == 0 )
		throw CWtpException(CWtpException::UPLOAD,0,"Upload data is empty to compare!");

	stringstream Message;
	Message << endl << "Comparing Upload to binary file:" << sFileName << endl;
	UserMessagePrintStr( true, Message.str());

    ifstream ifs;
	unsigned char* CompareAry;
	unsigned int iFileSize;
	bool bRet = true;

	ifs.open( sFileName.c_str(), ios_base::in | ios::binary );
	try
	{
		if (ifs.fail())
   			throw CWtpException(CWtpException::UPLOAD,0,"Upload Compare binary file open failed:" + sFileName);


		ifs.seekg( 0, ios_base::end );
		iFileSize = (unsigned int)ifs.tellg();
		ifs.seekg( 0, ios_base::beg );

		if (ifs.fail())
			throw CWtpException(CWtpException::UPLOAD,0,"Upload Compare binary file access failed:" + sFileName);

		CompareAry = new unsigned char[iFileSize];
		memset( CompareAry, 0, iFileSize );
		ifs.read( (char*)CompareAry, iFileSize );

		if (ifs.fail())
			throw CWtpException(CWtpException::UPLOAD,0,"Upload Compare binary file read failed:" + sFileName);
	}
	catch(CWtpException&)
	{
		delete[] CompareAry;
		if(ifs.is_open())
			ifs.close();
		throw;
	}
    ifs.close();

	int CompareSize = min(iFileSize, iDataSize );
	Message.str("");
    if (iFileSize > iDataSize )
	{
		Message << endl << "Upload Compare file is bigger than upload data!" << endl;
		UserMessagePrintStr( true, Message.str());
	}
    else if ( iFileSize > iDataSize )
	{
		Message << endl << "Upload Compare file is smaller than upload data!" << endl;
		UserMessagePrintStr( true, Message.str());
	}

	Message << endl << "Comparing " << CompareSize << endl;
	UserMessagePrintStr( true, Message.str());

    for ( int i = 0; i < CompareSize; i++ )
    {
        if ( pUploadDataBuffer[i] != CompareAry[i] )
        {
			Message.str("");
			Message << endl << "Comparing failed starting at byte location " << i << " ..." << endl;
			UserMessagePrintStr( true,Message.str());
            bRet = false;
			delete [] CompareAry;
            return bRet;
        }
    }
    Message.str("");
	Message << endl << "Compare Upload to File is Successful!" << endl;
	UserMessagePrintStr( true, Message.str());

    delete [] CompareAry;
    return bRet;
}
/**
* Method:	SaveUploadToFile
* Input:	unsigned char* , string , unsigned int
* Output:	void
* Desc:		Save iDataSize bytes of data in pUploadDataBuffer
*			in file with filename sFileName
*/
void CMisc::SaveUploadToFile( unsigned char* pUploadDataBuffer, string& sFileName,unsigned int iDataSize )
{
	if ( pUploadDataBuffer == 0 || iDataSize == 0 )
		throw CWtpException(CWtpException::UPLOAD,0,"Upload data is empty to save!");

	stringstream Message;
	Message << endl << "Saving Upload to binary file:" << sFileName << endl;
	UserMessagePrintStr( true, Message.str());

    ofstream ofs;
    ofs.open( sFileName.c_str(), ios_base::out | ios::binary );
	try
	{
		if ( ofs.fail() )
    		throw CWtpException(CWtpException::UPLOAD,0,"Upload Output binary file open failed:" + sFileName);

		ofs.write( (char*)pUploadDataBuffer, iDataSize );

		if (ofs.fail() )
			throw CWtpException(CWtpException::UPLOAD,0,"Upload Output binary file write failed:" + sFileName);

		ofs.close();
	}
	catch(CWtpException&)
	{
		if(ofs.is_open())
			ofs.close();
		throw;
	}
}

/**
* Method:	UserMessagePrintStr
* Input:	bool, string
* Output:	void
* Desc:		Prints Message of width 40 charaters.
*			Verbose is used to turn on/off the message printing
*/
void CMisc::UserMessagePrintStr( bool Verbose,const string& Message )
{
	static int column;

	if ( theApp.bLogFile)
		theApp.hLog << Message ;
	else
	{
		if ( Verbose )
		{
			if ( column > 0 )
			{
				//printf("\r\n");
				cout << endl;
				column = 0;
			}
			//printf(Message.c_str());
			cout << Message;
		}
		else
		{
			cout << "+";
			column++;
			if ( column == 40 )
			{
				//printf("\r\n");
				cout << endl;
				column = 0;
			}
		}
	}
	cout.flush();
//	fflush(NULL)
}

/**
* Method:	PrintUsage
* Input:	N/A
* Output:	void
* Desc:		Prints correct usage of commandline options
*/
void CMisc::PrintUsage()
{
	cout << "WtpDownload HELP Menu." << endl;
	cout << "======================" << endl;
	cout << "OPTIONS:" << endl;
    cout << "========" << endl << endl;
	cout << " -" << (char)CCommandlineParser::VERBOSE
		 << "                         Verbose mode. Print all debug messages." << endl
		 << "                            Default: false" << endl;
	cout << " -" << (char)CCommandlineParser::TIM_BIN_FILE
		 << " [TIM.bin]               Input TIM file (binary)" << endl;
	cout << " -" << (char)CCommandlineParser::IMAGE_FILE
		 << " [Image_h.bin]           Input any image file (_h binary)" << endl;
#if TRUSTED
	cout << " -" << (char)CCommandlineParser::JTAG_KEY
		 << " [JTAG_key.txt]          Input Jtag Key File (ASCII)" << endl;
#endif
	cout << " -" << (char)CCommandlineParser::MESSAGE_MODE
		 << " [Message Mode] 	    <DISABLED = 1>" << endl
		 << "			    <MESSAGE_ONLY = 2>" << endl
		 << "			    <MESSAGE_AND_DOWNLOAD = 3>" << endl
		 << "			    <DOWNLOAD_ONLY = 4>" << endl
		 << "                            Default: 3" << endl;
	cout << " -" << (char)CCommandlineParser::PORT_TYPE
		 << " [Port Type]             USB" << endl
		 << "                            UART" << endl;
	cout << " -" << (char)CCommandlineParser::INTERFACE_NAME
		 << " [USB Interface string]  Optional. Valid only when Port type is USB" << endl;
	cout << " -" << (char)CCommandlineParser::USB_PACKET_SIZE
		 << " [Packet Size]           Size of download packets." << endl
		 << "                            Actual packet size may be normalized to nearest valid size." << endl
		 << "                            Default: 4096" << endl;
	cout << " -" << (char)CCommandlineParser::COMM_PORT
		 << " [port number]           UART Port number" << endl
		 << "                            Default: 1" << endl;
	cout << " -" << (char)CCommandlineParser::BAUD_RATE
		 << " [baud rate]             110, 300, 600, 1200,"
		 <<	"						  2400, 4800, 9600, 14400,"
		 <<	"						  19200, 38400, 57600, 115200,"
		 <<	"						  12800, 256000" << endl;
	cout << " -" << (char)CCommandlineParser::FORCE_UART_IMAGE_LOAD
		<< " [Special]		Force UART Download Mode" << endl;
	cout << " -" << (char)CCommandlineParser::FORCE_BOOTROM_CONSOLE_MODE
		<< " [Special]		Force UART R/W Mode" << endl;
    cout << " -" << (char)CCommandlineParser::PLATFORM_TYPE
        << " [Platform type]	    <PXA168 = 0> ; ASPEN-A0" << endl
        << "			    <PXA30x = 1> ; MH-L" << endl
        << "			    <PXA31x = 2> ; MH-LV" << endl
        << "			    <PXA32x = 3> ; MH-P" << endl
        << "			    <ARMADA610 = 4> ; MMP2" << endl
        << "			    <PXA91x = 5> ; TTC" << endl
        << "			    <PXA92x = 6> ; TAVOR TD" << endl
        << "			    <PXA93x = 7> ; TAVOR P/PV" << endl
        << "			    <PXA94x = 8> ; TAVOR PV" << endl
        << "			    <PXA95x = 9> ; TAVOR PV2" << endl
        << "			    <ARMADA16x = 10> ; Aspen family" << endl
        << "			    <PXA955 = 12> ; TAVOR MG1" << endl
        << "			    <PXA968 = 13> ; TAVOR MG2" << endl
        << "			    <PXA1701 = 14> ; CP888/ESHEL" << endl
        << "			    <PXA978 = 15> ; NEVO" << endl
        << "			    <PXA2128 = 16> ; MMP3 Family" << endl
        << "			    <ARMADA622 = 18> ; MMP2X" << endl
        << "			    <PXA1202 = 19> ; WUKONG" << endl
        << "			    <PXA1801 = 20> ; ESHEL2" << endl
        << "                <88PA62-70 = 21> ; GRANITE2" << endl
		<< "			    <PXA988 = 22> ; EMEI" << endl
		<< "			    <PXA1920 = 23> ;  HELN LTE" << endl
		<< "			    <PXA2101 = 24> ;  BIRCH-40" << endl
		<< "			    <PXA192 = 25> ; BIRCH-28" << endl
		<< "			    <PXA1928 = 26> ;  EDEN" << endl
		<< "			    <PXA1986 = 27> ;  ADIR" << endl
		<< "			    <PXA1802 = 28> ;  NEZHA" << endl
		<< "			    <PXA986 = 29> ; KUNLUN" << endl
		<< "			    <PXA1206 = 30> ;  WUJIN" << endl
		<< "			    <PXA888 = 31> ; WUTAI" << endl
		<< "			    <PXA1088 = 32> ;  HELN" << endl
		<< "			    <PXA1812 = 33> ;  ESHEL3" << endl
		<< "			    <PXA1822 = 34> ;  NEZHA2" << endl
		<< "			    <PXA1U88 = 35> ;  HELAN2" << endl
        << "			    <PXA1908 = 36> ;  ULC1" << endl
        << "			    <PXA1826 = 37> ;  NEZHA3" << endl
        << "                <P88PA62_20 = 38> ; GEMSTONE2" << endl
        << "                <P88PA62_10 = 39> ; // BASALT2" << endl
		<< "				<PXA1956 = 40> ; // HELAN4" << endl
		<< "				<VEGA = 41> ; // VEGA" << endl
		<< "				<BOWIE = 42> ; // BOWIE" << endl
		<< "				<ULC2 = 43> ; // ULC2" << endl;

	cout << " -" << (char)CCommandlineParser::PARTITION_TABLE
		 << " [partition.bin]         Download partition table bin file." << endl;
	cout << " -" << (char)CCommandlineParser::DISABLE_FAST_DOWNLOAD
		 << "                         Disables the fast download mode(if supported)." << endl;
	cout << " -" << (char)CCommandlineParser::LOG_FILE
		 << " [LogFilename.txt]       Log all messages of download to a log file." << endl;
	cout << " -" << (char)CCommandlineParser::UPLOAD_DATA
		 << " [UploadSpec.txt]        Upload Specification file. Upload data from target to PC" << endl;

#if USE_USB_DELAY_SWITCHES == 1
    cout << " -" << (char)CCommandlineParser::ENUMERATE_DELAY
        << " [delay in ms]            Time to delay after opening USB port. Time is in milliseconds." << endl;
    cout << " -" << (char)CCommandlineParser::ENUMERATE_ARRIVE_WAIT
        << " [usb wait in sec]        Time to wait for a target to arrive on the USB port. Time is in seconds." << endl << endl;
    cout << " -" << (char)CCommandlineParser::USB_READ_DELAY
        << " [usb delay in ms]        Time to delay for a target to after a read on the USB port. Time is in milliseconds." << endl << endl;
#else
    cout << endl;
#endif

#if WINDOWS
	cout << "JtagOverMedia Command Line Options" << endl;
	cout << "==================================" << endl;
    cout << " -J" << (char)CCommandlineParser::JTAG_CONNECT_TEST
         << "                         JTAG Connection Test only." << endl;

    cout << " -J" << (char)CCommandlineParser::INQUIRE_TARGET
         << "                         Inquire Target to get supported Set Parameter Options and status." << endl;

    cout << " -J" << (char)CCommandlineParser::ENABLE_JTAG_CMD
         << "                         Enable Secure Jtag." << endl;

    cout << " -J" << (char)CCommandlineParser::CORES_TO_ENABLE
         << "                         Cores to enable." << endl
         << "                         0 = No Cores" << endl
         << "                         1 = GP (such as PJ4)" << endl
         << "                         2 = SP (secure processor)" << endl
         << "                         4 = PD (platform dependent, such as CP)" << endl
         << "                             for multiple cores, OR values to enable multiple processors" << endl
         << "                             for example: 3 = GP and SP, 6 = SP and PD, etc." << endl;

    cout << " -J" << (char)CCommandlineParser::JTAG_KEY_FILE
         << " [JTAG_KEY_FILE.txt]     Use JTAG Key File path for key definitions." << endl;

    cout << " -J" << (char)CCommandlineParser::GET_KEY_PROVIDER
        << " [<SystemName>:<PortNumber>]" << endl
        << "                          Get Key Provider via a socket connection for secure key handling." << endl
        << "                          <SystemName> is optional, default is local host." << endl
        << "                          ':' is required delimiter," << endl
        << "                          <PortNumber> is optional, default is port 2000. PortNumber must match KeyProvider port number." << endl;

    cout << " -J" << (char)CCommandlineParser::SIGNED_UID_KEY_FILE
        << " [SIGNED_UID_KEY_FILE.txt]     Use Signed_UID_Key.txt File path to base enabling on UID instead of random nonce." << endl;


    cout << " -J" << (char)CCommandlineParser::PROC_TYPE
//         << " [<PXA168=0><PXA30x=1><PXA31x=2><PXA32x=3><ARMADA610=4><PXA91x=5><PXA92x=6>" << endl;
//         << "  <PXA93x=7><PXA94x=8><PXA95x=9><ARMADA16x=10><reserved1=11>" << endl;
//         << "  <PXA955=12><PXA968=13><PXA1701=14><PXA978=15><PXA2128=16><reserved2=17><ARMADA622=18><PXA1202=19><PXA1801=20>] << endl";
//         << "  <88PA62_70=21><PXA928=22><PXA1920=23><PXA2101=24><PXA192=25><PXA1928=26><PXA1986=27><PXA1802=28>\n" );
//         << "  <PXA986=29><PXA1206=30><PXA888=31><PXA1088=32><PXA1812=33><PXA1822=34><PXA1U88=35>" << endl;
//         << "  <PXA1908=36><PXA1826=37><88PA62_20=38><88PA62_10=39><undefined=40>]" << endl;
         << " [<PXA2128=26><P88PA62_70=21><PXA1928=26><PXA1986=27><PXA1U88=35><PXA1908=36><P88PA62_20=38><P88PA62_10=39>]" << endl
         << "                         Available types: PXA2128, 88PA62_70, PXA1928, PXA1986, PXA1U88, PXA1908(USB only), 88PA62_20, and 88   PA62_10." << endl;

    cout << " -J" << (char)CCommandlineParser::FORCE_RESET
         << "                         Force Target to Reset before starting protocol." << endl;

    cout << " -J" << (char)CCommandlineParser::SET_PARAMETERS_CMD
         << "                         Perform a Set Parameters Command, using -J"
				 << (char)CCommandlineParser::PARAM_BOOT_STATE
		<< ", and -J" << (char)CCommandlineParser::PARAM_FORCE_DOWNLOAD
		<< " switches." << endl;

    cout << " -J" << (char)CCommandlineParser::PARAM_BOOT_STATE
         << " [Boot State value]      Boot State value for the Set Params command." << endl;

    cout << " -J" << (char)CCommandlineParser::PARAM_FORCE_DOWNLOAD
         << "                         Force download parameter for the Set Params command." << endl;

    cout << " -J" << (char)CCommandlineParser::PARAM_FORCE_UART_LOG
         << "                         Force UART log parameter for the Set Params command." << endl;

    cout << " -J" << (char)CCommandlineParser::AC_REQUEST_TYPE
         << " [Request Type value]    Request type value must be 1 of the following values:" << endl
         << "                          JTAG Protocol V1 values:" << endl
		 << "                          ========================" << endl
         << "                             0 - Software Debug only." << endl
         << "                             1 - NIDEN." << endl
         << "                             2 - DBGEN." << endl
         << "                             4 - SPNIDEN." << endl
         << "                             8 - SPIDEN." << endl
         << "                             16 - SUIDEN." << endl
         << "                          JTAG Protocol V2+ values:" << endl
		 << "                          =========================" << endl
         << "                             0 - Software Debug only." << endl
         << "                             1 - NIDEN." << endl
         << "                             2 - SPNIDEN." << endl
         << "                             4 - SPIDEN." << endl;
    cout << " -J" << (char)CCommandlineParser::AC_PERMANENTLY_DISABLE
         << "                         Permanently Disable JTAG access." << endl;

    cout << " -J" << (char)CCommandlineParser::AC_FA_STATE
         << "                         Switch to Failure Analysis state." << endl;

    cout << " -J" << (char)CCommandlineParser::CHANGE_TCK_FREQ
         << " [Tck Hz value]          Tck Hz value to configure JTAG clock speed." << endl;

    cout << " -J" << (char)CCommandlineParser::USE_WTM_TO_RUN_PROTOCOL
         << "                         Use WTM to handle JTAG Reenable without target SRESET." << endl;

	cout << " -J" << (char)CCommandlineParser::PORTTYPE << " [USB/UART]"
         << "                         Reenable JTAG over USB/UART port" << endl;
	cout << " -J" << (char)CCommandlineParser::COMMPORT << " [COMM port Number]"
         << "                         UART Port number" << endl;
	cout << " -J" << (char)CCommandlineParser::BAUDRATE << " [UART baud rate]"
         << "                         UART Baud Rate" << endl;
    cout << " -J" << (char)CCommandlineParser::TARGET_TIMEOUT << " [Target Timeout value]"
        << "                         Target timeout in seconds. Default is 5 sec." << endl;
    cout << " -J" << (char)CCommandlineParser::POLLING_DELAY << " [Polling delay value]"
        << "                         Polling Delay in milliseconds. Default is 10 ms." << endl;

#endif
	cout << " -" << (char)CCommandlineParser::HELP
		 << "                         Help. Displays this help message." << endl << endl;
    cout << "EXAMPLES:" << endl;
    cout << "=========" << endl;

#if WINDOWS
	cout << "WtpDownload for enabling JTAG using Jtag Over media:" << endl;
	cout << "====================================================" << endl ;
	cout << "Example 1:  Output to Cmd window" << endl;
	cout << "=================================" << endl
         << "WtpDownload.exe -JY USB -JT 4 -JS -JB 1 -JG 2 -JW 10 -JF -JE -JR 0 -JK OEMKey1024PubExt.txt" << endl << endl;
    cout << "Example 2:  Output in log file" << endl
		 << "=================================" << endl
         << "WtpDownload.exe -JY USB -JT 4 -JS -JB 1 -JG 2 -JW 10 -JF -JE -JR 0 -JK OEMKey1024PubExt.txt >> <LogFileName>.log" << endl;
#endif

#if TRUSTED
    cout << "WtpDownload for Jtag Re-Enabling:" << endl ;
	cout << "=================================" << endl ;
	cout << "WtpDownload.exe -" <<(char)CCommandlineParser::JTAG_KEY <<" JtagOEMkeyfile.txt" << endl << endl;
	cout << "WtpDownload of TIM-JTAG Reenable-Image Usage:" << endl ;
    cout << "=============================================" << endl ;
	cout << "WtpDownload.exe -" << (char)CCommandlineParser::PORT_TYPE << " UART -" << (char)CCommandlineParser::TIM_BIN_FILE << " <NTIM/TIM.bin> -" << (char)CCommandlineParser::IMAGE_FILE << " <Image_h.bin> -" << (char)CCommandlineParser::JTAG_KEY << " JtagOEMKeyfile.txt" << endl;
#endif

    cout << "WtpDownload for Downloading images:" << endl ;
	cout << "===================================" << endl ;
	cout << "WtpDownload.exe -" << (char)CCommandlineParser::PORT_TYPE << " USB -" << (char)CCommandlineParser::TIM_BIN_FILE << " TIM.bin -" << (char)CCommandlineParser::IMAGE_FILE << " OBM_h.bin" << endl << endl ;
    cout << "WtpDownload with MESSAGE_ONLY Mode(Mode 2):" << endl ;
    cout << "===========================================" << endl ;
	cout << "WtpDownload.exe -" << (char)CCommandlineParser::PORT_TYPE << " USB -" << (char)CCommandlineParser::MESSAGE_MODE << " 2" << endl << endl ;
    cout << "WtpDownload with MESSAGE_AND_DOWNLOAD Mode (Mode 3):" << endl ;
    cout << "====================================================" << endl ;
	cout << "WtpDownload.exe -" << (char)CCommandlineParser::PORT_TYPE << " USB -" << (char)CCommandlineParser::TIM_BIN_FILE << " <NTIM/TIM.bin> -" << (char)CCommandlineParser::IMAGE_FILE << " <Image_h.bin> -" << (char)CCommandlineParser::MESSAGE_MODE << " 3" << endl << endl ;
	cout << "WtpDownload with DOWNLOAD_ONLY Mode (Mode 4):" << endl;
	cout << "====================================================" << endl;
	cout << "WtpDownload.exe -" << (char)CCommandlineParser::PORT_TYPE << " USB -" << (char)CCommandlineParser::TIM_BIN_FILE << " <NTIM/TIM.bin> -" << (char)CCommandlineParser::IMAGE_FILE << " <Image_h.bin> -" << (char)CCommandlineParser::MESSAGE_MODE << " 4" << endl << endl;
}