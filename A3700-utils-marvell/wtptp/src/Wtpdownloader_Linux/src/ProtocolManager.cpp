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
//#include <cstdlib>
#include <iostream>
#include "ProtocolManager.h"
#include "WtpImage.h"
#include "TimLib.h"
#include "WtpCommand.h"
#include "WtpDownloadApp.h"
#include "ErrorCodes.h"
#include "Security.h"

#if WINDOWS
#include "UARTPort.h"
#endif

#if LINUX
#include "UARTPortLinux.h"
#endif

using namespace std;

CProtocolManager::CProtocolManager(void)
{
	UploadPacketLength  = 0;
	memset(&TargetParams,0,sizeof(TARGET_PARAMS));
	memset(&TargetPV,0,sizeof(PROTOCOL_VERSION));
	Port = 0;

#if WINDOWS
	h_Child_STDIN_Read = NULL;
	h_Child_STDIN_Write = NULL;
	h_Child_STDOUT_Read = NULL;
	h_Child_STDOUT_Write = NULL;
#endif
}

CProtocolManager::~CProtocolManager(void){}

CWtpStatus* CProtocolManager::GetStatus()
{
	return Port->GetWtpStatus();
}

void CProtocolManager::SetWtpPort(CPort* Port)
{
	this->Port = Port;
}

CPort* CProtocolManager::GetPort()
{
	return this->Port;
}

/**
* Method:	SendPlainTextWtp
* Input:	N/A
* Output:	boolean
* Desc:		Sends 'w','t','p',CR  to target.
*			Expected from target is 'w','t','p',CR,LF
*/
void CProtocolManager::SendPlainTextWtpCR()
{
	stringstream Message;
	Message << "HOST:: Sending 'w','t','p','CR' ..." << endl;
	misc.UserMessagePrintStr(theApp.bVerbose, Message.str());
	Port->WtpStatus.ResetWtpStatus();
	Port->WtpCmd = new CPlainTextWTPCR;
	Port->WritePort();
#if DEV_DEBUG
	if (theApp.bVerbose)
	{
		Port->WtpCmd->print();
		Port->WtpStatus.print();
	}
#endif

	if (!(Port->GetWtpStatus()->GetCMD() == 0x77 &&
		Port->GetWtpStatus()->GetSEQ() == 0x74 &&
		Port->GetWtpStatus()->GetCID() == 0x70 &&
		Port->GetWtpStatus()->GetStatus() == 0xD &&
		Port->GetWtpStatus()->GetFlags() == 0xA ))
	{
		delete Port->WtpCmd;
		if (Port->WtpStatus.GetDLEN() > 0)
			Port->GetWtpRemainingMessage();
		throw CWtpException(CWtpException::NACK_WTPCR);
	}
	delete Port->WtpCmd;
}

/**
* Method:	SendContinuousForceUARTMode
* Input:	N/A
* Output:	boolean
* Desc:		Sends 0xBB, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0c77  to target until we receive 0x3E back.
*			Expected from target is 0x3E
*/
void CProtocolManager::SendContinuousForceUARTMode()
{


	stringstream Message;
	unsigned int LoopTimeout = 0;
	char * sendbuffer_escape = new char[8];
	char * sendbuffer_response = new char[1];
	char * recbuffer_escape = new char[8];
	char * recbuffer_3e_or_nack = new char[1];
	int i = 0;
	int nullcount = 0;
	int incount = 0;
	sendbuffer_response[0] = 0xd;
	Message << "HOST:: Sending 0xBB112233 and 0x44556677 ..." << endl;
	misc.UserMessagePrintStr(theApp.bVerbose, Message.str());




	sendbuffer_escape[0] = 0xbb;
	sendbuffer_escape[1] = 0x11;
	sendbuffer_escape[2] = 0x22;
	sendbuffer_escape[3] = 0x33;
	sendbuffer_escape[4] = 0x44;
	sendbuffer_escape[5] = 0x55;
	sendbuffer_escape[6] = 0x66;
	sendbuffer_escape[7] = 0x77;



	do {


		Port->WriteRaw(sendbuffer_escape, 8);
		nullcount = 0;
		incount = Port->ReadRaw(recbuffer_3e_or_nack, 1);

		//cout << recbuffer_3e_or_nack[0] << recbuffer_3e_or_nack[1] << recbuffer_3e_or_nack[2] << recbuffer_3e_or_nack[3] << endl;

			if(recbuffer_3e_or_nack[0] == 0x3e && incount == 1) {

				//cout << "I see a 3e" << endl;
				LoopTimeout = 10000; // force exit from while loop because we have seen a response
				//cout << "Sending a response" << endl;
				//Port->WriteRaw(sendbuffer_response, 1);




			} else if (recbuffer_3e_or_nack[0] == 0x00 && incount == 1 ) {
				nullcount++;
				for (i = 0; i< 3; i++) {
					//cout << "I see " << nullcount << " characters of a NACK..." << endl;
					incount = Port->ReadRaw(recbuffer_3e_or_nack, 1);

					if (recbuffer_3e_or_nack[0] == 0x00 && incount == 1) {
						//cout << recbuffer_3e_or_nack[0] << endl;
						nullcount++;
						if (nullcount == 4) {
							LoopTimeout = 10000;
							//cout << "I've been NACKed.." << endl;



						}
					}
				}

			} else {

				LoopTimeout++;
				if (LoopTimeout > 9999) { // We have not seen a response for 1000 tries. Give up and throw an exception.
					cout << "Too many tries.... stopping escape loop." << endl;
					break;

				}
			}
		} while (LoopTimeout < 10000);


	//Sleep(2000);

	//flush the target's input buffer
	//do {
	//	incount = Port->ReadRaw(recbuffer_3e_or_nack, 1);
	//} while (incount > 0);


}

/**
* Method:	SendContinuousFFStream_DLoad
* Input:	N/A
* Output:	boolean
* Desc:		Sends 0xff  to target until we receive 0x3E back.
*			Expected from target is 0x3E
*/
void CProtocolManager::SendContinuousFFStream_DLoad()
{


	stringstream Message;
	unsigned int LoopTimeout = 0;
	char * sendbuffer_escape = new char[8];
	char * sendbuffer_response = new char[1];
	char * recbuffer_escape = new char[8];
	char * recbuffer_3e_or_nack = new char[1];
	int i = 0;
	int nullcount = 0;
	int incount = 0;
	sendbuffer_response[0] = 0xd;
	Message << "HOST:: Sending 0xFF ..." << endl;
	misc.UserMessagePrintStr(theApp.bVerbose, Message.str());




	sendbuffer_escape[0] = 0xff;
	sendbuffer_escape[1] = 0xff;
	sendbuffer_escape[2] = 0xff;
	sendbuffer_escape[3] = 0xff;
	sendbuffer_escape[4] = 0xff;
	sendbuffer_escape[5] = 0xff;
	sendbuffer_escape[6] = 0xff;
	sendbuffer_escape[7] = 0xff;



	do {


		Port->WriteRaw(sendbuffer_escape, 8);
		nullcount = 0;
		incount = Port->ReadRaw(recbuffer_3e_or_nack, 1);

		//cout << recbuffer_3e_or_nack[0] << recbuffer_3e_or_nack[1] << recbuffer_3e_or_nack[2] << recbuffer_3e_or_nack[3] << endl;

		if (recbuffer_3e_or_nack[0] == 0x3e && incount == 1) {

			//cout << "I see a 3e" << endl;
			LoopTimeout = 1000000; // force exit from while loop because we have seen a response
			//cout << "Sending a response" << endl;
			//Port->WriteRaw(sendbuffer_response, 1);




		}
		else if (recbuffer_3e_or_nack[0] == 0x00 && incount == 1) {
			nullcount++;
			for (i = 0; i< 3; i++) {
				//cout << "I see " << nullcount << " characters of a NACK..." << endl;
				incount = Port->ReadRaw(recbuffer_3e_or_nack, 1);

				if (recbuffer_3e_or_nack[0] == 0x00 && incount == 1) {
					//cout << recbuffer_3e_or_nack[0] << endl;
					nullcount++;
					if (nullcount == 4) {
						LoopTimeout = 1000000;
						//cout << "I've been NACKed.." << endl;



					}
				}
			}

		}
		else {

			LoopTimeout++;
			if (LoopTimeout > 999999) { // We have not seen a response for 1000 tries. Give up and throw an exception.
				cout << "Too many tries.... stopping escape loop." << endl;
				break;

			}
		}
	} while (LoopTimeout < 1000000);


	//Sleep(2000);

	//flush the target's input buffer
	//do {
	//	incount = Port->ReadRaw(recbuffer_3e_or_nack, 1);
	//} while (incount > 0);


}

/**
* Method:	SendContinuousFFStream
* Input : N / A
* Output : boolean
* Desc : Sends 0xFF  to target until we receive 0x3E back.
*			Expected from target is 0x3E
*/
void CProtocolManager::SendContinuousFFStream()
{


	stringstream Message;
	unsigned int LoopTimeout = 0;
	char * sendbuffer_escape = new char[8];
	char * sendbuffer_response = new char[1];
	char * recbuffer_escape = new char[8];
	char * recbuffer_3e_or_nack = new char[1];
	int i = 0;
	int nullcount = 0;
	int incount = 0;
	sendbuffer_response[0] = 0xd;
	Message << "HOST:: Sending Stream of 0xFF ..." << endl;
	misc.UserMessagePrintStr(theApp.bVerbose, Message.str());

	sendbuffer_escape[0] = 0xff;
	sendbuffer_escape[1] = 0xff;
	sendbuffer_escape[2] = 0xff;
	sendbuffer_escape[3] = 0xff;
	sendbuffer_escape[4] = 0xff;
	sendbuffer_escape[5] = 0xff;
	sendbuffer_escape[6] = 0xff;
	sendbuffer_escape[7] = 0xff;

	do {


		Port->WriteRaw(sendbuffer_escape, 8);
		nullcount = 0;
		incount = Port->ReadRaw(recbuffer_3e_or_nack, 1);

		//cout << recbuffer_3e_or_nack[0] << recbuffer_3e_or_nack[1] << recbuffer_3e_or_nack[2] << recbuffer_3e_or_nack[3] << endl;

		if (recbuffer_3e_or_nack[0] == 0x3e && incount == 1) {

			//cout << "I see a 3e" << endl;
			LoopTimeout = 1000000; // force exit from while loop because we have seen a response
			//cout << "Sending a response" << endl;
			//Port->WriteRaw(sendbuffer_response, 1);




		}
		else if (recbuffer_3e_or_nack[0] == 0x00 && incount == 1) {
			nullcount++;
			for (i = 0; i< 3; i++) {
				//cout << "I see " << nullcount << " characters of a NACK..." << endl;
				incount = Port->ReadRaw(recbuffer_3e_or_nack, 1);

				if (recbuffer_3e_or_nack[0] == 0x00 && incount == 1) {
					//cout << recbuffer_3e_or_nack[0] << endl;
					nullcount++;
					if (nullcount == 4) {
						LoopTimeout = 1000000;
						//cout << "I've been NACKed.." << endl;



					}
				}
			}

		}
		else {

			LoopTimeout++;
			if (LoopTimeout > 999999) { // We have not seen a response for 100000 tries. Give up and throw an exception.
				cout << "Too many tries.... stopping escape loop." << endl;
				break;

			}
		}
	} while (LoopTimeout < 1000000);


	//Sleep(2000);

	//flush the target's input buffer
	//do {
	//	incount = Port->ReadRaw(recbuffer_3e_or_nack, 1);
	//} while (incount > 0);


}

/**
* Method:	SendContinuousForceConsoleMode
* Input:	N/A
* Output:	boolean
* Desc:		Sends 0xDD, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0c77  to target until we receive 0x3E back.
*			Expected from target is 0x3E
*/
void CProtocolManager::SendContinuousForceConsoleMode()
{
	stringstream Message;
	stringstream pCom;
	unsigned int LoopTimeout = 0;
	unsigned int waitLoop = 0;
	int nullcount = 0;
	int i = 0;
	char * sendbuffer_escape = new char[8];
	char * sendbuffer_response = new char[1];
	char * recbuffer_escape = new char[8];
	char * recbuffer_3e = new char[1];
	sendbuffer_response[0] = 0xd;
	int incount = 0;
	Message << "HOST:: Sending 0xDD112233 and 0x44556677 ..." << endl;
	misc.UserMessagePrintStr(theApp.bVerbose, Message.str());
	sendbuffer_escape[0] = 0xdd;
	sendbuffer_escape[1] = 0x11;
	sendbuffer_escape[2] = 0x22;
	sendbuffer_escape[3] = 0x33;
	sendbuffer_escape[4] = 0x44;
	sendbuffer_escape[5] = 0x55;
	sendbuffer_escape[6] = 0x66;
	sendbuffer_escape[7] = 0x77;



	do {

			Port->WriteRaw(sendbuffer_escape, 8);

			incount = Port->ReadRaw(recbuffer_3e, 1);

			if (recbuffer_3e[0] == 0x3e) {
				//cout << "I see a 3e" << endl;

				LoopTimeout = 1000000; // force exit from while loop because we have seen a response



				//cout << "Sending a response" << endl;
				//Port->WriteRaw(sendbuffer_response, 1);
			} else if (recbuffer_3e[0] == 0x00 && incount == 1) {
				nullcount++;
				for (i = 0; i< 3; i++) {
					//cout << "I see " << nullcount << " characters of a NACK..." << endl;
					incount = Port->ReadRaw(recbuffer_3e, 1);

					if (recbuffer_3e[0] == 0x00 && incount == 1) {
						//cout << recbuffer_3e[0] << endl;
						nullcount++;
						if (nullcount == 4) {
							LoopTimeout = 1000000;
							//cout << "I've been NACKed.." << endl;

						}
					}
				}

			} else {

			LoopTimeout++;
			//cout << LoopTimeout << endl;
			if (LoopTimeout > 999999) { // We have not seen a response for 1000 tries. Give up and throw an exception.
				cout << "Too many tries... exiting program." << endl;
				break;
			}
		}
	} while (LoopTimeout < 1000000);

	//Sleep(2000);

	//flush the target's input buffer
	//do {
	//	incount = Port->ReadRaw(recbuffer_3e, 1);
	//} while (incount > 0);

	//exit(0);
}


/**
* Method:	SendPreamble
* Input:	N/A
* Output:	boolean
* Desc:		Sends PREAMBLE command to target.
*			PREAMBLE is the first command in download protocol.
*			Target sends an ACK/NACK back in CWtpStatus object.
*/
void CProtocolManager::SendPreamble () throw (CWtpException)
{
	stringstream Message;
	Message << "HOST:: Sending preamble..." << endl;
	misc.UserMessagePrintStr( theApp.bVerbose, Message.str());
	Port->WtpStatus.ResetWtpStatus();
	Port->WtpCmd = new CPreambleCommand;
	Port->WritePort();
#if DEV_DEBUG
	if(theApp.bVerbose)
	{
		Port->WtpCmd->print();
		Port->WtpStatus.print();
	}
#endif
	if (Port->WtpStatus.GetStatus() == NACK)
	{
		delete Port->WtpCmd;
		if (Port->WtpStatus.GetDLEN() > 0)
			Port->GetWtpRemainingMessage();
		throw CWtpException(CWtpException::NACK_PREAMBLE);
	}
	delete Port->WtpCmd;
}

/**
* Method:	GetVersion
* Input:	N/A
* Output:	boolean
* Desc:		Sends GETVERSION command to target.
*			GETVERSION is the second command in download protocol.
*			Target sends an ACK/NACK back in CWtpStatus object.
*/
void CProtocolManager::GetVersion () throw (CWtpException)
{
	stringstream Message;
	HandlePendingMessages();
	Port->WtpStatus.ResetWtpStatus();
	Message << "HOST:: Sending GetVersion command..." << endl;
	misc.UserMessagePrintStr( theApp.bVerbose, Message.str());
	Port->WtpCmd = new CGetVersionCommand;
	Port->WritePort();
#if DEV_DEBUG
	if(theApp.bVerbose)
	{
		Port->WtpCmd->print();
		Port->WtpStatus.print();
	}
#endif
	if (Port->WtpStatus.GetStatus() == NACK)
	{
		delete Port->WtpCmd;
		if (Port->WtpStatus.GetDLEN() > 0)
			Port->GetWtpRemainingMessage();

		HandlePendingMessages();

		throw CWtpException(CWtpException::NACK_GETVERSION);
	}
	else if (Port->WtpStatus.GetStatus() == ACK)
    {
		Message.str("");
		Message << "Version: "<< Port->WtpStatus.GetData()[3];
		Message << Port->WtpStatus.GetData()[2];
		Message << Port->WtpStatus.GetData()[1];
		Message << Port->WtpStatus.GetData()[0] << endl;
		int val = 0;
		val = Port->WtpStatus.GetData()[7] << 8;
		val = (val | Port->WtpStatus.GetData()[6]) << 8;
		val = (val | Port->WtpStatus.GetData()[5]) << 8;
		val = (val | Port->WtpStatus.GetData()[4]);
		Message << "Date: " << hex << val << endl;

		Message << "Processor: " << Port->WtpStatus.GetData()[11];
		Message << Port->WtpStatus.GetData()[10];
		Message << Port->WtpStatus.GetData()[9];
		Message << Port->WtpStatus.GetData()[8] << endl;

		if((Port->WtpStatus.GetFlags() & 0x4) &&
			Port->WtpStatus.GetDLEN() == 21)
		{
			// f81d4fae-7dec-11d0-a765-00a0c91e6bf6
			Message << "UUID:" << Port->WtpStatus.GetData()[20];
			Message << Port->WtpStatus.GetData()[19];
			Message << Port->WtpStatus.GetData()[18];
			Message << Port->WtpStatus.GetData()[17];
			Message << Port->WtpStatus.GetData()[16];
			Message << Port->WtpStatus.GetData()[15];
			Message << Port->WtpStatus.GetData()[14];
			Message << Port->WtpStatus.GetData()[13];
			Message << Port->WtpStatus.GetData()[12] << endl;
		}

		misc.UserMessagePrintStr(true,Message.str());
    }
	delete Port->WtpCmd;
	HandlePendingMessages();
}

/**
* Method:	SelectImage
* Input:	PDWORD
* Output:	boolean
* Desc:		Sends SELECTIMAGE command to target.
*			SELECTIMAGE is the third command in download protocol
*			The target sends back the image type it is expecting
*			in the CWtpStatus object.
*/
void CProtocolManager::SelectImage (unsigned int *pImageType) throw (CWtpException)
{
	stringstream Message;
	Message << "HOST:: Sending SelectImage command..." << endl;
	misc.UserMessagePrintStr( theApp.bVerbose,Message.str());

	Port->WtpStatus.ResetWtpStatus();
	HandlePendingMessages();
	Port->WtpStatus.ResetWtpStatus();
	Port->WtpCmd = new CSelectImageCommand;
	Port->WritePort();

	*pImageType = Port->WtpStatus.GetData()[0];
	*pImageType |= (Port->WtpStatus.GetData()[1] << 8);
    *pImageType |= (Port->WtpStatus.GetData()[2] << 16);
    *pImageType |= (Port->WtpStatus.GetData()[3] << 24);

#if DEV_DEBUG
	if(theApp.bVerbose)
	{
		Port->WtpCmd->print();
		Port->WtpStatus.print();
	}
#endif

	if (Port->WtpStatus.GetStatus() == NACK)
	{
		delete Port->WtpCmd;
		if (Port->WtpStatus.GetDLEN() > 0)
			Port->GetWtpRemainingMessage();
		HandlePendingMessages();
		throw CWtpException(CWtpException::NACK_SELECTIMAGE);
	}

	HandlePendingMessages();
	delete Port->WtpCmd;
}

/**
* Method:	VerifyImage
* Input:	BYTE
* Output:	boolean
* Desc:		Sends VERIFYIMAGE command to target.
*			VERIFYIMAGE is the fourth command in download protocol
*			The target sends back an ACK/NACK in CWtpStatus object.
*/
void CProtocolManager::VerifyImage (unsigned char AckOrNack) throw (CWtpException)
{
	stringstream Message;
	Message << "HOST:: Sending VerifyImage command..." << endl;
	misc.UserMessagePrintStr( theApp.bVerbose,Message.str());

	Port->WtpStatus.ResetWtpStatus();
	HandlePendingMessages();
	Port->WtpStatus.ResetWtpStatus();
    Port->WtpCmd = new CVerifyImageCommand(AckOrNack);
	Port->WritePort();
#if DEV_DEBUG
	if(theApp.bVerbose)
	{
		Port->WtpCmd->print();
		Port->WtpStatus.print();
	}
#endif

	if (Port->WtpStatus.GetStatus() == NACK)
	{
		delete Port->WtpCmd;
		if (Port->WtpStatus.GetDLEN() > 0)
			Port->GetWtpRemainingMessage();

		HandlePendingMessages();
		throw CWtpException(CWtpException::NACK_VERIFYIMAGE);
	}

	HandlePendingMessages();
	delete Port->WtpCmd;
}

/**
* Method:	DataHeader
* Input:	unsigned int
* Output:	boolean
* Desc:		Sends DATAHEADER command to target.
*			Host sends the size of data in this command.
*			The target sends back an ACK/NACK in CWtpStatus object
*			along with the BUFFERSIZE that it can handle.
*/
void CProtocolManager::DataHeader(unsigned int uiRemainingData) throw (CWtpException)
{
    stringstream Message;
    Message << "HOST:: Sending DataHeader command..." << endl;
    misc.UserMessagePrintStr(theApp.bVerbose, Message.str());
    Port->WtpStatus.ResetWtpStatus();
    HandlePendingMessages();
    Port->WtpStatus.ResetWtpStatus();
    Port->WtpCmd = new CDataHeaderCommand(theApp.bFastDownload);

    Port->WtpCmd->SetData((uiRemainingData & 0x000000FF), 0);
    Port->WtpCmd->SetData((uiRemainingData & 0x0000FF00) >> 8, 1);
    Port->WtpCmd->SetData((uiRemainingData & 0x00FF0000) >> 16, 2);
    Port->WtpCmd->SetData((uiRemainingData & 0xFF000000) >> 24, 3);

    Port->WritePort();

#if DEV_DEBUG
    if(theApp.bVerbose)
    {
        Port->WtpCmd->print();
        Port->WtpStatus.print();
    }
#endif

    if (Port->WtpStatus.GetStatus() == NACK)
    {
        delete Port->WtpCmd;
		if (Port->WtpStatus.GetDLEN() > 0)
			Port->GetWtpRemainingMessage();
        HandlePendingMessages();
        throw CWtpException(CWtpException::NACK_DATAHEADER);
    }
    HandlePendingMessages();
    delete Port->WtpCmd;

#if USE_USB_DELAY_SWITCHES == 1
    // this sleep is requested by Mike and Bruce to address an issue with the USB/BootROM terminating a transfer
    // in the middle of a data block
    if (theApp.bUSBReadDelay)
    {
#if WINDOWS
        Sleep(theApp.iUSBReadDelay);
#else
        sleep(theApp.iUSBReadDelay);
#endif
    }
#endif

}

/**
* Method:	Data
* Input:	PBYTE, int
* Output:	boolean
* Desc:		Sends DATA command to target.
*			Host sends the data of length the target can handle
*			in this command.
*			The target sends back an ACK/NACK in CWtpStatus object
*/
void CProtocolManager::Data (unsigned char *pData,int Length) throw (CWtpException)
{
	stringstream Message;

	Message << "HOST:: Sending Data..." << endl;
	misc.UserMessagePrintStr( theApp.bVerbose,Message.str());
	HandlePendingMessages();
	Port->WtpCmd = new CDataCommand(pData,Length);
	Port->WtpStatus.ResetWtpStatus();
	Port->WritePort();

#if DEV_DEBUG
	if(theApp.bVerbose)
	{
		Port->WtpCmd->print();
		Port->WtpStatus.print();
	}
#endif

	if ( !theApp.bFastDownload || isLastData(GET, FDMODE))
    {
		if (Port->WtpStatus.GetStatus() == NACK)
		{
			delete Port->WtpCmd;
			if (Port->WtpStatus.GetDLEN() > 0)
				Port->GetWtpRemainingMessage();

			HandlePendingMessages();
			throw CWtpException(CWtpException::NACK_DATA);
		}
    }
	HandlePendingMessages();
	delete Port->WtpCmd;
}



/**
* Method:	Done
* Input:	N/A
* Output:	boolean
* Desc:		Sends DONE command to target.
*			Host sends this command to indicate that there is no
*			more data to send.
*			The target sends back an ACK/NACK in CWtpStatus object
*/
void CProtocolManager::Done () throw (CWtpException)
{
	stringstream Message;
	Message << "HOST:: Sending Done Command..." << endl;
	misc.UserMessagePrintStr( theApp.bVerbose,Message.str());
	Port->WtpCmd = new CDoneCommand;
	Port->WtpStatus.ResetWtpStatus();
	Port->WritePort();

#if DEV_DEBUG
	if(theApp.bVerbose)
	{
		Port->WtpCmd->print();
		Port->WtpStatus.print();
	}
#endif

	if (Port->WtpStatus.GetStatus() == NACK)
	{
		delete Port->WtpCmd;
		if (Port->WtpStatus.GetDLEN() > 0)
			Port->GetWtpRemainingMessage();
		HandlePendingMessages();
		throw CWtpException(CWtpException::NACK_DONE);
	}
	HandlePendingMessages();
	delete Port->WtpCmd;
}

/**
* Method:	Disconnect
* Input:	N/A
* Output:	boolean
* Desc:		Sends DISCONNECT command to target.
*			Host sends this command to terminate connection with target
*			The target sends back an ACK/NACK in CWtpStatus object
*/
void CProtocolManager::Disconnect () throw (CWtpException)
{
	stringstream Message;
	bool IsOk = true;
	int retry = 0;
    static bool DisconnectSent = false;

    if (!DisconnectSent)
    {
        // prevent sending disconnect more than once
        DisconnectSent = true;

        do
        {
            Port->WtpStatus.ResetWtpStatus();
            Message << "HOST:: Sending Disconnect command..." << endl;
            misc.UserMessagePrintStr(theApp.bVerbose, Message.str());
            try{
				//cout << "constructing disconnect command" << endl;
				Port->WtpCmd = new CDisconnectCommand;
				//cout << "About to do a write to port." << endl;
                Port->WritePort();
				//cout << "Finished writing to port..." << endl;
            }
            catch (CWtpException e)
            {
				//cout << "got exception sending disconnect command. ";
				delete Port->WtpCmd;
                Port->WtpCmd = 0;
                CWtpStatus temp;
                temp.ResetWtpStatus();
                if (memcmp((void *)&Port->WtpStatus.GetWtpStatus(), (void *)&temp.GetWtpStatus(), sizeof(WTPSTATUS)) == 0)
                    break;
                else
                    throw;
            }
            if (Port->WtpStatus.GetStatus() == ACK)
                break;

			if (Port->WtpStatus.GetStatus() == NACK && Port->WtpStatus.GetFlags())
			{
				if (Port->WtpStatus.GetDLEN() > 0)
					Port->GetWtpRemainingMessage();
				cout << endl;
				HandlePendingMessages();
			}
            else
                retry++;

			if (retry < 2)
				delete Port->WtpCmd;
        } while (retry < 2);

#if DEV_DEBUG
        if(theApp.bVerbose)
        {
            Port->WtpCmd->print();
            Port->WtpStatus.print();
        }
#endif

        if (Port->WtpStatus.GetStatus() == NACK)
        {
			if (Port->WtpStatus.GetDLEN() > 0)
				Port->GetWtpRemainingMessage();
            HandlePendingMessages();
			delete Port->WtpCmd;
            throw CWtpException(CWtpException::NACK_DISCONNECT);
        }
        delete Port->WtpCmd;
    }
}

/**
* Method:	GetTargetProtocolVersion
* Input:	PROTOCOL_VERSION*
* Output:	boolean
* Desc:		Sends PROTOCOLVERSION command to target.
*			This command is sent in order to know which version
*			of target is run. The version number if target indicates
*			what features are supported by target.
*/
void CProtocolManager::GetTargetProtocolVersion() throw (CWtpException)
{
	ostringstream Message;
	Message << "HOST:: Sending GetTargetProtocolVersion command..." << endl;
	misc.UserMessagePrintStr( theApp.bVerbose,Message.str());
	HandlePendingMessages();
	Port->WtpStatus.ResetWtpStatus();
	Port->WtpCmd = new CTargetProtocolVersionCommand;
	Port->WritePort();


#if DEV_DEBUG
	if(theApp.bVerbose)
	{
		Port->WtpCmd->print();
		Port->WtpStatus.print();
	}
#endif

	if (Port->WtpStatus.GetStatus() == NACK)
	{
		delete Port->WtpCmd;
		if (Port->WtpStatus.GetDLEN() > 0)
			Port->GetWtpRemainingMessage();
		cout << endl;
		HandlePendingMessages();
		throw CWtpException(CWtpException::NACK_GETPROTOCOLVERSION);
	}
	TargetPV.MajorVersion =(unsigned char) Port->WtpStatus.GetData()[0];
	TargetPV.MinorVersion =(unsigned char)Port->WtpStatus.GetData()[1];
	TargetPV.Build = Port->WtpStatus.GetData()[2];

	Message.str("");
	int val = TargetPV.MajorVersion;
	Message << "Major Version:" << val << endl;
	val = TargetPV.MinorVersion;
	Message << "Minor Version:" << val << endl;
	Message << "Build:" << TargetPV.Build << endl;
	misc.UserMessagePrintStr(true,Message.str());

	HandlePendingMessages();
	delete Port->WtpCmd;
}

/**
* Method:	DoUploads
* Input:	TARGET_PARAMS*
* Output:	boolean
* Desc:		Does uploads and compares the uploaded data
*			with file provided for comparision.
*/
void CProtocolManager:: DoUploads() throw (CWtpException)
{
	bool IsOk = true;
	stringstream Message;

	ParseUploadSpecFile();
	list<UPLOAD_DATA_SPEC*>::iterator UploadSpecsListIter = lstUploadSpecs.begin();
    while ( UploadSpecsListIter != lstUploadSpecs.end() )
    {
		UPLOAD_DATA_SPEC* UploadSpec = *UploadSpecsListIter;
		UPLOAD_DATA_PARAMS* UploadParameters = &(*UploadSpecsListIter)->Params;

		Message << endl << "HOST:: Uploading..." << endl;
		int val = UploadParameters->Type;
		Message << "Type:" << val << endl;
		val = UploadParameters->SubType;
		Message << "SubType:" << val << endl;
		val = UploadParameters->Partition;
		Message << "Partition:" << val << endl;
		Message << "Offset:" << UploadParameters->Offset << endl;
		Message << "DataSize:" << UploadParameters->DataSize << endl;
		Message << "Output File:" << UploadSpec->sOutputFileName << endl;
		Message << "Comparison File:" << UploadSpec->sComparisonFileName << endl;
		misc.UserMessagePrintStr( theApp.bVerbose, Message.str());

        unsigned char* pUploadDataBuffer = new unsigned char[ UploadParameters->DataSize ];
		try
		{
			if ( pUploadDataBuffer )
			{
				memset( pUploadDataBuffer, 0, UploadParameters->DataSize );
				UploadDataHeader( UploadParameters );

				unsigned int BytesRemaining = UploadParameters->DataSize;
				unsigned int BytesReceived = 0;

				while ( BytesRemaining > 0 )
				{
					UploadPacketLength =  min( BytesRemaining, TargetParams.BufferSize - 6 );
					UploadData();
					memcpy( pUploadDataBuffer + BytesReceived, Port->WtpStatus.GetData(), UploadPacketLength );
					// downcount the bytes remaining, not counting the wtpstatus header in the buffer
					BytesReceived  += UploadPacketLength;
					BytesRemaining -= UploadPacketLength;
					Message.str("");
					Message << "Bytes received = " << BytesReceived;
					Message << ". Bytes remaining = " << BytesRemaining << endl;
					misc.UserMessagePrintStr( theApp.bVerbose, Message.str());
				}

				Message.str("");
				Message << "Upload Completed!" << endl;
				misc.UserMessagePrintStr( true, Message.str());
				misc.SaveUploadToFile( pUploadDataBuffer, UploadSpec->sOutputFileName, UploadSpec->Params.DataSize );
				misc.CompareUploadToFile( pUploadDataBuffer, UploadSpec->sComparisonFileName,UploadSpec->Params.DataSize );
				delete[] pUploadDataBuffer;
			}
		}
		catch(CWtpException&)
		{
			HandlePendingMessages();
			delete[] pUploadDataBuffer;
			throw;
		}
        UploadSpecsListIter++;
	}
	Done();
}

/**
* Method:	GetParameters
* Input:	TARGET_PARAMS*
* Output:	boolean
* Desc:		Sends GETPARAMETERS command to target.
*			The target returns parameters
*/
void CProtocolManager::GetParameters() throw (CWtpException)
{
	stringstream Message;
	HandlePendingMessages();
	Port->WtpStatus.ResetWtpStatus();
	Message << "HOST:: Sending GetParameters Command..." << endl;
	misc.UserMessagePrintStr(theApp.bVerbose, Message.str());
	Port->WtpCmd = new CGetParametersCommand;
	Port->WritePort();

#if DEV_DEBUG
	if(theApp.bVerbose)
	{
		Port->WtpCmd->print();
		Port->WtpStatus.print();
	}
#endif

	if (Port->WtpStatus.GetStatus() == NACK)
	{
		if (Port->WtpStatus.GetDLEN() > 0)
			Port->GetWtpRemainingMessage();
		throw CWtpException(CWtpException::NACK_GETPARAMETERS);
	}


	if(Port->WtpStatus.GetDLEN() != 16)
	{
		Message << endl << "GetParameters returned LEN is not correct! LEN = 0x" << hex << (int)Port->WtpStatus.GetDLEN() << endl;
		misc.UserMessagePrintStr(true,Message.str());
		HandlePendingMessages();
		delete Port->WtpCmd;
		throw CWtpException(CWtpException::NACK_GETPARAMETERS);
	}

	int val = 0;
	val = Port->WtpStatus.GetData()[3] << 8;
	val = (val | Port->WtpStatus.GetData()[2]) << 8;
	val = (val | Port->WtpStatus.GetData()[1]) << 8;
	val = (val | Port->WtpStatus.GetData()[0]);
	TargetParams.BufferSize = val;

	val = Port->WtpStatus.GetData()[7] << 8;
	val = (val | Port->WtpStatus.GetData()[6]) << 8;
	val = (val | Port->WtpStatus.GetData()[5]) << 8;
	val = (val | Port->WtpStatus.GetData()[4]);
    	TargetParams.Rsvd1 = val;

	val = Port->WtpStatus.GetData()[11] << 8;
	val = (val | Port->WtpStatus.GetData()[10]) << 8;
	val = (val | Port->WtpStatus.GetData()[9]) << 8;
	val = (val | Port->WtpStatus.GetData()[8]);
    	TargetParams.Rsvd2 = val;

	val = Port->WtpStatus.GetData()[15] << 8;
	val = (val | Port->WtpStatus.GetData()[14]) << 8;
	val = (val | Port->WtpStatus.GetData()[13]) << 8;
	val = (val | Port->WtpStatus.GetData()[12]);
    	TargetParams.Rsvd3 = val;

	Message << "BufferSize = " << TargetParams.BufferSize << endl;
	Message << "Rsvd1: " << TargetParams.Rsvd1 << endl;
	Message << "Rsvd2: " << TargetParams.Rsvd2 << endl;
	Message << "Rsvd3: " << TargetParams.Rsvd3 << endl;
	misc.UserMessagePrintStr(true,Message.str());
	delete Port->WtpCmd;
}

/**
* Method:	UploadDataHeader
* Input:	UPLOAD_DATA_PARAMS*
* Output:	boolean
* Desc:		Sends UPLOADDATAHEADER command to target.
*/
void CProtocolManager::UploadDataHeader ( UPLOAD_DATA_PARAMS* pUploadDataParams ) throw (CWtpException)
{
	stringstream Message;
	Message << "HOST:: Sending UploadDataHeader command..." << endl;
	misc.UserMessagePrintStr(theApp.bVerbose,Message.str());
	HandlePendingMessages();
	Port->WtpStatus.ResetWtpStatus();
	Port->WtpCmd = new CUploadDataHeaderCommand;
	Port->WtpCmd->SetData((unsigned char *)pUploadDataParams,sizeof(UPLOAD_DATA_PARAMS));
	Port->WritePort();

#if DEV_DEBUG
	if(theApp.bVerbose)
	{
		Port->WtpCmd->print();
		Port->WtpStatus.print();
	}
#endif

	if(Port->WtpStatus.GetStatus() == NACK)
	{
		if(Port->WtpStatus.GetDLEN() > 0)
			Port->GetWtpRemainingMessage();
		HandlePendingMessages();
		delete Port->WtpCmd;
		throw CWtpException(CWtpException::NACK_UPLOADDATAHEADER);
	}
	HandlePendingMessages();
	delete Port->WtpCmd;
}

/**
* Method:	UploadData
* Input:	void
* Output:	void
* Desc:		Sends UPLOADDATA command to target.
*/
void CProtocolManager::UploadData() throw (CWtpException)
{
	stringstream Message;
	Message << "HOST:: Sending UploadData command..." << endl;
	misc.UserMessagePrintStr(theApp.bVerbose,Message.str());
	HandlePendingMessages();
	Port->WtpStatus.ResetWtpStatus();
	Port->WtpCmd = new CUploadDataCommand;
	Port->WritePort();

#if DEV_DEBUG
	if(theApp.bVerbose)
	{
		Port->WtpCmd->print();
		Port->WtpStatus.print();
	}
#endif

	if(Port->WtpStatus.GetStatus() == NACK)
	{
		if(Port->WtpStatus.GetDLEN() > 0)
			Port->GetWtpRemainingMessage();
		HandlePendingMessages();
		delete Port->WtpCmd;
		throw CWtpException(CWtpException::NACK_UPLOADDATA);
	}
	HandlePendingMessages();
	delete Port->WtpCmd;
}


/**
* Method:	GetPC
* Input:
* Output:	boolean
* Desc:		Sends GETPC command to target.
*/
void CProtocolManager::GetPC() throw (CWtpException)
{
	stringstream Message;
	Message << "HOST:: Sending GetPC command..." << endl;
	misc.UserMessagePrintStr(theApp.bVerbose,Message.str());
	HandlePendingMessages();
	Message.str("");

	Port->WtpStatus.ResetWtpStatus();
	Port->WtpCmd = new CGetPCCommand;
	Port->WritePort();

#if DEV_DEBUG
	if(theApp.bVerbose)
	{
		Port->WtpCmd->print();
		Port->WtpStatus.print();
	}
#endif

	if(Port->WtpStatus.GetStatus()==NACK)
	{
		delete Port->WtpCmd;
		if (Port->WtpStatus.GetDLEN() > 0)
			Port->GetWtpRemainingMessage();

		HandlePendingMessages();
		throw CWtpException(CWtpException::NACK_GETPC);
	}
	else if (Port->WtpStatus.GetStatus() == ACK)
    {
		if(Port->WtpStatus.GetDLEN() != 4)
		{
			delete Port->WtpCmd;
			Message << endl << "GetPC returned LEN is not correct! LEN = 0x" << hex << (int)Port->WtpStatus.GetDLEN() << endl;
			misc.UserMessagePrintStr(true,Message.str());
			HandlePendingMessages();
			throw CWtpException(CWtpException::NACK_GETPC);
		}

		Message.str("");

		unsigned int targetPC = 0;
		targetPC = Port->WtpStatus.GetData()[3] << 24;
		targetPC |= (Port->WtpStatus.GetData()[2]) << 16;
		targetPC |= (Port->WtpStatus.GetData()[1]) << 8;
		targetPC |= (unsigned int)(Port->WtpStatus.GetData()[0]);

		Message << "Target PC: " << "0x" << hex <<  setfill('0') << setw(8) << targetPC << endl;
		misc.UserMessagePrintStr(true,Message.str());
	}

	HandlePendingMessages();
	delete Port->WtpCmd;
}

/**
* Method:	PublicKey
* Input:	N/A
* Output:	boolean
* Desc:		Sends PUBLICKEY command to target.
*/
void CProtocolManager::PublicKey () throw (CWtpException)
{
	stringstream Message;
	Message << "HOST:: Sending Public Key..." << endl;
	misc.UserMessagePrintStr( theApp.bVerbose, Message.str());
	Port->WtpStatus.ResetWtpStatus();
	Port->WtpCmd = new CPublicKeyCommand;
	Port->WritePort();

#if DEV_DEBUG
	if(theApp.bVerbose)
	{
		Port->WtpCmd->print();
		Port->WtpStatus.print();
	}
#endif

	if (Port->WtpStatus.GetStatus() == NACK)
	{
		if (Port->WtpStatus.GetDLEN() > 0)
			Port->GetWtpRemainingMessage();

		HandlePendingMessages();
		delete Port->WtpCmd;
		throw CWtpException(CWtpException::NACK_PUBLICKEY);
	}
	HandlePendingMessages();
	delete Port->WtpCmd;
}

/**
* Method:	Password
* Input:	N/A
* Output:	boolean
* Desc:		Sends PASSWORD command to target.
*/
void CProtocolManager::Password () throw (CWtpException)
{
	stringstream Message;
	Message << "HOST:: Sending Password..." << endl;
	misc.UserMessagePrintStr( theApp.bVerbose,Message.str());
	Port->WtpStatus.ResetWtpStatus();
	Port->WtpCmd = new CPasswordCommand;
	Port->WritePort();

#if DEV_DEBUG
	if(theApp.bVerbose)
	{
		Port->WtpCmd->print();
		Port->WtpStatus.print();
	}
#endif

	if (Port->WtpStatus.GetStatus() == NACK)
	{
		if (Port->WtpStatus.GetDLEN() > 0)
			Port->GetWtpRemainingMessage();
		HandlePendingMessages();
		delete Port->WtpCmd;
		throw CWtpException(CWtpException::NACK_PASSWORD);
	}
	HandlePendingMessages();
	delete Port->WtpCmd;
}

/**
* Method:	SignedPassword
* Input:	N/A
* Output:	boolean
* Desc:		Sends SIGNEDPASSWORD command to target.
*/
void CProtocolManager::SignedPassword () throw (CWtpException)
{
	stringstream Message;
	Message << "HOST:: Sending Signed Password..." << endl;
	misc.UserMessagePrintStr( theApp.bVerbose, Message.str());
	HandlePendingMessages();
	Port->WtpStatus.ResetWtpStatus();
	Port->WtpCmd = new CSignedPasswordCommand;
	Port->WritePort();

#if DEV_DEBUG
	if(theApp.bVerbose)
	{
		Port->WtpCmd->print();
		Port->WtpStatus.print();
	}
#endif

	if (Port->WtpStatus.GetStatus() == NACK)
	{
		if (Port->WtpStatus.GetDLEN() > 0)
			Port->GetWtpRemainingMessage();

		HandlePendingMessages();
		delete Port->WtpCmd;
    	throw CWtpException(CWtpException::NACK_SIGNEDPASSWORD);
	}
	HandlePendingMessages();
	delete Port->WtpCmd;
}

/**
* Method:	GetWtpMessage
* Input:	N/A
* Output:	boolean
* Desc:		Sends MESSAGE command to target.
*/
void CProtocolManager::GetWtpMessage ()
{
	stringstream Message;
	Port->WtpStatus.ResetWtpStatus();
#if DEV_DEBUG
	Message << "HOST:: Sending Message Command..." << endl;
	misc.UserMessagePrintStr( theApp.bVerbose, Message.str());
#endif
	Port->WtpCmd = new CMessageCommand;
	Port->WritePort();

#if DEV_DEBUG
	if(theApp.bVerbose)
	{
		Port->WtpCmd->print();
		Port->WtpStatus.print();
	}
#endif
	if (Port->WtpStatus.GetStatus() == NACK)
	{
		if (Port->WtpStatus.GetDLEN() > 0)
			Port->GetWtpRemainingMessage();

#if DEV_DEBUG
		Message.str("");
		Message << "Target NACKed: No more messages!" << endl;
#endif
	}
	delete Port->WtpCmd;
}

/**
* Method:	GetTargetMessage
* Input:	N/A
* Output:	boolean
* Desc:		Retrives messages in queue sent by target
*/
void CProtocolManager::GetTargetMessage ()
{
	bool IsPlatformBusy = false;
	int Seconds = 0, DelayTime = 120, PlatformStatus = 0;
	stringstream Message;

	Seconds = Port->getTickCount() / 1000;
	do
    	{
			GetWtpMessage();
			PlatformStatus = Port->WtpStatus.GetData()[0];
			switch(PlatformStatus)
			{
			case PlatformReady: IsPlatformBusy = false;
								break;
			case PlatformDisconnect:
								Disconnect();
								throw CWtpException(CWtpException::PLATFORM_DISCONNECT);
			case PlatformBusy:
				IsPlatformBusy = true;
#ifdef WINDOWS
				if ( theApp.iUSBPacketSize < 2000 ) // target running bootrom
				    Port->sleep(40);
				else
				    Port->sleep(10); // target running dkb
#endif
				continue;
			default: if(IsPlatformBusy)
					 {
						 Port->WtpStatus.ResetWtpStatus();
					 	 continue;
					 }
					 break;
			}
			if ((Port->WtpStatus.GetFlags() & 0x1) != MESSAGEPENDING)
				break;
    }while ((Port->WtpStatus.GetStatus() == ACK) &&
			((Port->getTickCount() / 1000) < (Seconds + DelayTime)));
}

/**
* Method:	HandlePendingMessages
* Input:	N/A
* Output:	void
* Desc:		Retrives messages in queue sent by target
*/
void CProtocolManager::HandlePendingMessages()
{
    do
	{
		if ( !((theApp.eCurrentMessageMode != DISABLED ) && ((Port->WtpStatus.GetFlags() & 0x1) == MESSAGEPENDING)) )
			break;
		GetTargetMessage ();
	}
	while( Port->WtpStatus.GetStatus() != ACK );
}
/**
* Method:	ParseUploadSpecFile
* Input:	list
* Output:	boolean
* Desc:		Parser specifications file provided by user for doing upload
*/
void CProtocolManager::ParseUploadSpecFile() throw (CWtpException)
{
	bool IsOk = true;
    ifstream ifs;
	CTimLib TimLib;
    UPLOAD_DATA_SPEC* pNewSpec = 0;

	lstUploadSpecs.clear();

	try
	{
		if ( theApp.bUploadSpecFile)
			ifs.open(theApp.sUploadSpecFileName.c_str(), ios_base::in );
		else
			throw CWtpException(CWtpException::UPLOAD,0,string("Upload Specification file not found!"));

		if (ifs.fail() )
				throw CWtpException(CWtpException::UPLOAD,0,string("Upload Specification file open failed!"));

		while ( !ifs.eof() && !ifs.fail() )
		{
			string sLine = "" , sAttribute = "";
			unsigned long dwValue = 0;
			pNewSpec = 0;
			bool IsOk = true;

			for(int i=0; i < 9; i++)
			{
				IsOk = true;
				if (!TimLib.GetNextLineNoWSSkipComments( ifs, sLine ))
					goto Exit;

				if ( ifs.eof() || ifs.fail())
					goto Exit;

				switch(i)
				{
					case 0: if(sLine.find("Upload Spec:",0) == string::npos)
								throw  CWtpException(CWtpException::UPLOAD,0,"\"Upload Spec:\" directive not found in line " + sLine);
							else
							{
								pNewSpec = new UPLOAD_DATA_SPEC;
								//reset only the upload_data_params not upload_data_spec memory length since upload_data_spec has two string objects that should not be reset to 0.
							    memset(&pNewSpec->Params,0,sizeof(UPLOAD_DATA_PARAMS));
								continue;
							}
							break;
					case 1: sAttribute = "Type";break;
					case 2: sAttribute = "Subtype";break;
					case 3: sAttribute = "Partition";break;
					case 4: sAttribute = "Offset";break;
					case 5: sAttribute = "Data Size";break;
					case 6: sAttribute = "Output File";break;
					case 7: sAttribute = "Compare File";break;
					case 8: if(sLine.find( "End Upload Spec:",0) == string::npos)
							{
								delete pNewSpec;
                                pNewSpec = 0;
								throw  CWtpException(CWtpException::UPLOAD,0,"\"End Upload Spec:\" directive not found in line " + sLine);
							}
							continue;
					default: throw CWtpException(CWtpException::UPLOAD,0,"Invalid line! " + sLine);
				}

				string stringValue = "";
				if( i <= 5)
					IsOk = TimLib.GetFieldValue(sLine, sAttribute, dwValue); // Get numeric values
				else
					IsOk = TimLib.GetFieldValue(sLine,sAttribute,stringValue);//Get string values

				if(!IsOk)
				{
					delete pNewSpec;
                    pNewSpec = 0;
					throw CWtpException(CWtpException::UPLOAD,0,"Invalid attribute value for " + sAttribute + "!");
				}
				switch(i)
				{
					case 1: pNewSpec->Params.Type		= (unsigned char)dwValue;break;
					case 2: pNewSpec->Params.SubType	= (unsigned char)dwValue;break;
					case 3: pNewSpec->Params.Partition	= (unsigned char)dwValue;break;
					case 4: pNewSpec->Params.Offset		= dwValue;break;
					case 5: pNewSpec->Params.DataSize	= dwValue;break;
					case 6: pNewSpec->sOutputFileName	= stringValue.substr(0,stringValue.length());break;
					case 7:	pNewSpec->sComparisonFileName = stringValue.substr(0,stringValue.length());break;
					default: break;
				}
			}
			lstUploadSpecs.push_back( pNewSpec );
		}
	Exit:
		if ( ifs.eof() )
		{
			// done processing file
			ifs.close();
			delete pNewSpec;
            pNewSpec = 0;
        }
		else
		{
			delete pNewSpec;
            pNewSpec = 0;
            // got an error before reaching eof
			throw CWtpException(CWtpException::UPLOAD);
		}
	}
	catch(CWtpException&)
	{
		if (ifs.is_open())
			ifs.close();
		delete pNewSpec;
        pNewSpec = 0;
        throw;
    }
}

/**
* Method:	isLastData
* Input:	boolean, boolean
* Output:	boolean
* Desc:		Sets lastData static variable to true when
*			last data packet is sent to target.  Also returns
*			value of lastData.
*/
bool CProtocolManager::isLastData(bool mode, bool value)   //added to set/get the last data condition.
{
	static bool lastData = GET;

	if (mode)
	    lastData = value;
	else
		return (lastData == 1 ? true : false) ;

	return true;
}

#if TRUSTED
/**
* Method:	getByteArray
* Input:	BYTE*&, unsigned int[], int
* Output:	void
* Desc:		Converts src (unsigned int array) of intLength
*			to dest (byte array)
*/
void getByteArray(unsigned char*& dest,unsigned int src[],int intLength)
{
	for(int i=0 ;i < intLength ; i++)
	{
		dest[i*4+3] = (unsigned char)((src[i] >> 24) & 0xFF) ;
		dest[i*4+2] = (unsigned char)((src[i] >> 16) & 0xFF) ;
		dest[i*4+1] = (unsigned char)((src[i] >> 8) & 0XFF);
		dest[i*4]	= (unsigned char)((src[i] & 0XFF));
	}

}

void printPassword(unsigned int* passwd,int length)
{
	if (length < 4)
	{
		for(int i = 0; i < length ;i++)
			cout << "0x" << hex <<  setfill('0') << setw(8) << (unsigned int)passwd[i]  << " ";
	}
	else
	for(int i = 0; i < length ;i++)
	{
		cout << endl;
		cout << "0x" << hex <<  setfill('0') << setw(8) << (unsigned int)passwd[4 * i]  << " ";
		cout << "0x" << hex <<  setfill('0') << setw(8) << (unsigned int)passwd[4 * i + 1] << " ";
		cout << "0x" << hex <<  setfill('0') << setw(8) << (unsigned int)passwd[4 * i + 2] << " ";
		cout << "0x" << hex <<  setfill('0') << setw(8) << (unsigned int)passwd[4 * i + 3] << " ";
	}
	cout << endl;
}


/**
* Method:	JtagReEnable
* Input:	N/A
* Output:	boolean
* Desc:		Re-enables Jtag. By default JTag is disabled.
*			This can be done in trusted version only.
*			Implements the Wtp-JTAG-ReEnablement protocol.

*			NOTE: This is not JTagReeabler tool functionality.
*			JTagReEnabler uses a different protocol for JTagReEnablement
*/
#if 0 // sah for 64
void CProtocolManager::JtagReEnable () throw (CWtpException)
{
	int ErrorStatus = 0;
	unsigned int* password = new unsigned int[64];
	unsigned int* DigitalSignature = new unsigned int[64];
	unsigned char* buffer;
	CSecurity security;
	CWtpImage* Image = 0;
	memset(password,0,64);
	memset(DigitalSignature,0,64);
	stringstream Message;

	Message << endl << "******************" << endl;
	Message << "JTAG Reenablement" << endl;
	Message << "******************" << endl;
	misc.UserMessagePrintStr( true,Message.str());
	Message.str("");
	if ( theApp.bPlatform &&
        (theApp.ePlatformType == theApp.PXA988 || theApp.ePlatformType == theApp.PXA1088 || theApp.ePlatformType == theApp.PXA1920))
    {
         Message << endl << "******************" << endl;
         Message << "DELAY 1:: Allowing 1 second for target to be ready..." << endl;
         misc.UserMessagePrintStr( true,Message.str());
		 Message.str("");

        Port->sleep(1000);

         Message << "Assuming target is ready to continue." << endl;
         Message << "******************" << endl;
         misc.UserMessagePrintStr( true,Message.str());
		 Message.str("");
    }

	try{
        Image = theApp.GetImageFromImageList(JTAGIDENTIFIER);
		if ( Image == 0)
		{
			delete[] password;
			delete[] DigitalSignature;
			throw CWtpException(CWtpException::NO_KEYFILE);
		}
		security.OpeniFile(Image->GetSImageFileName());
		security.ReadKeyFile();

		if (ErrorStatus != NoError)
		{
			delete[] password;
			delete[] DigitalSignature;
			throw CWtpException(CWtpException::READ_KEYFILE,0,Image->GetSImageFileName());
		}
		CRSAKeys rsaKey  = security.GetRSAKeys();
		buffer = new unsigned char[8 * rsaKey.GetKeyLength()];
		unsigned char* buf = buffer;
		getByteArray(buf,rsaKey.GetPublicKey(),rsaKey.GetKeyLength());

		buf = buffer + 4 * rsaKey.GetKeyLength();
		getByteArray(buf,rsaKey.GetModulus(),rsaKey.GetKeyLength());

		if(theApp.bVerbose)
		{
			//rsaKey.printPrivateKey();
			rsaKey.printPublicKey();
			rsaKey.printModulus();
		}

		SendPreamble();
		PublicKey();
		if ( theApp.bPlatform &&
            (theApp.ePlatformType == theApp.PXA988 || theApp.ePlatformType == theApp.PXA1088 || theApp.ePlatformType == theApp.PXA1920 ))
		{
			Message.str("");
			Message << endl << "******************" << endl;
			Message << "DELAY 2:: Allowing 1 s for target to be ready..." << endl;
			misc.UserMessagePrintStr( true,Message.str());
			Message.str("");
			Port->sleep(1000);

			Message << "Assuming target is ready to continue." << endl;
			Message << "******************" << endl;
			misc.UserMessagePrintStr( true,Message.str());
			Message.str("");
		}
		Data(buffer,8 * rsaKey.GetKeyLength());


		Password();

		memcpy(password,Port->WtpStatus.GetData(),(int)Port->WtpStatus.GetDLEN());
		rsaKey.PrivateKeySignPassword(password,DigitalSignature);

		if(theApp.bVerbose)
		{
			misc.UserMessagePrintStr(true,string("\nHOST:: Password \n"));
			printPassword(password,2);
			misc.UserMessagePrintStr(true,string("\nHOST::Digital Signature: \n"));
			printPassword(DigitalSignature,rsaKey.GetKeyLength()/4);
		}

		SignedPassword();

		memset(buffer,0,8 * rsaKey.GetKeyLength());
		getByteArray(buffer,DigitalSignature,rsaKey.GetKeyLength());

		Data(buffer,4 * rsaKey.GetKeyLength());
		Done();

		delete[] buffer;
		delete[] password;
		delete[] DigitalSignature;
	}
	catch(CWtpException&)
	{
		delete[] buffer;
		delete[] password;
		delete[] DigitalSignature;
		throw;
	}
}
# endif // sah for 64

#if WINDOWS
/**
* Method:	readFromPipe
* Input:	void
* Output:	void
* Desc:		Read output from the child process's pipe for STDOUT
*			and write to the parent process's pipe for STDOUT.
*			Stop when there is no more data.
*			Read messages sent from JtagEnabler
*/

void CProtocolManager::readFromPipe() throw(CWtpException)
{
	stringstream Message;
    // Close the write end of the pipe before reading from the
    // read end of the pipe.
#define BUFSIZE (4096*128)
	char* pchBuf = new char[BUFSIZE];

	//HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwRead = 0;
    DWORD ExitCode = 0;
	DWORD dwWrite = 0;
	int readCount =0;
	stringstream buf;

    if (h_Child_STDOUT_Write )
    {
        CloseHandle(h_Child_STDOUT_Write);
        h_Child_STDOUT_Write = 0;
    }

    memset( pchBuf, 0, BUFSIZE  );

    // give exe process up to 2 seconds to start up if system is very busy
    int iWait = 2000;
    while(ExitCode != STILL_ACTIVE)// while ( iWait > 0 && piProcInfo.hProcess != 0 )
    {
        // determine if downloader process is now active
        if ( GetExitCodeProcess( piProcInfo.hProcess, &ExitCode ) )
        {
            if ( ExitCode == STILL_ACTIVE )
                break;
        }
    }

    if ( ExitCode != STILL_ACTIVE && ExitCode != 0 )
    {
        Message << "Error:  Tool exe failed to execute.  Check installation of builder." << endl << endl;
		misc.UserMessagePrintStr(true,Message.str());
        return ;
    }

	 while( piProcInfo.hProcess != 0 && ExitCode == STILL_ACTIVE )
    {
		ExitCode = 0;
        // determine if builder process is still active or has exited
        if ( GetExitCodeProcess( piProcInfo.hProcess, &ExitCode ) )
        {
            if ( ExitCode == STILL_ACTIVE )
            {
				if (ReadFile(h_Child_STDOUT_Read, pchBuf, BUFSIZE , &dwRead, NULL) && (dwRead != 0))
                {
					cout << string(pchBuf);// << flush;
					//buf << string(pchBuf);
					memset(pchBuf,0,dwRead);
                    dwRead = 0;
                }
            }
        }
     }

     while ( ReadFile(h_Child_STDOUT_Read, pchBuf, BUFSIZE, &dwRead, NULL) && (dwRead != 0) )
     {
        cout << string(pchBuf);
        memset(pchBuf,0,dwRead);
        dwRead = 0;
     }

     //while ( ReadFile(h_Child_STDOUT_Read, pchBuf, BUFSIZE , &dwRead, NULL) && dwRead != 0 )
 //   {
	//	//misc.UserMessagePrintStr(true,"read 2");
	//	cout << string(pchBuf);// << flush;
	//	//buf << string(pchBuf);
 //       //misc.UserMessagePrintStr(true,string(pchBuf)) ;
	//	//printf("%s",pchBuf);
 //       memset(pchBuf,0,dwRead);
 //       dwRead = 0;
 //   }
	//
    if ( ExitCode != STILL_ACTIVE && ExitCode != 0 )
    {
        stringstream ssMsg;
        ssMsg << endl << "Error:  Builder exited with error. Exit code: ";
        ssMsg << (int)ExitCode << endl;
		misc.UserMessagePrintStr(true,string(ssMsg.str()));
    }

    delete[] pchBuf;
}

/**
* Method:	writeToPipe
* Input:	void
* Output:	void
* Desc:		Write its commandline contents to the pipe for JtagEnabler's STDIN.
*			Stop when there is no more data.
*			Launches JTagEnabler as a child process with its commandline arguments
*/
void CProtocolManager::writeToPipe(string message) throw(CWtpException)
{
	DWORD dwRead=0, dwWritten;
	BOOL bSuccess = FALSE;
	do
	{
		bSuccess = WriteFile(h_Child_STDIN_Write, message.c_str(), dwRead, &dwWritten, NULL);
	} while(bSuccess);
}

void JtagMediaThread( LPVOID lpParameter )
{
    CProtocolManager* pProtocolMgr = (CProtocolManager*)lpParameter;
	CMisc misc;
	misc.UserMessagePrintStr(true,"JtagEnabler Output:");
    // Drain remaining messages from pipe
    pProtocolMgr->readFromPipe();

    //	ExitThread(0);
    //return DWORD(bRet);
}

void CProtocolManager::createJtagEnablerChildProcess(string args) throw(CWtpException)
{
	// NOTE: this code is exerpted from MS VS 2005 Documentation
	//  from article "Creating a Child Process with Redirected Input and Output"
	// ms-help://MS.VSCC.v80/MS.MSDN.vAug06.en/dllproc/base/creating_a_child_process_with_redirected_input_and_output.htm
	//  Code has been modified and added as member functions to this class.

	string jtagEnablerExe;
	stringstream Message;
	BOOL bSuccess = FALSE;

    if ( theApp.bEnablerExePath )
        jtagEnablerExe = theApp.sEnableExePath + " ";
    else
        jtagEnablerExe = theApp.exePath + "JtagEnabler.exe ";

    if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(jtagEnablerExe.c_str()))
    {
        Message << CTimLib::FormatWindowsGetLastErrorToString(GetLastError()) << endl;
        throw CWtpException(CWtpException::JTAGENABLER_FAILED, 0, Message.str());
    }
    jtagEnablerExe += args;

	cout << "JTAG Enabler Command Line:" << endl;// << flush;
	cout << jtagEnablerExe << endl;// << flush;

	DWORD ThreadId = 0;

	if(m_DownloadThreadHandle == 0)
	{
			m_DownloadThreadHandle = CreateThread(
									NULL,
									0,
									(LPTHREAD_START_ROUTINE)JtagMediaThread,
									this,
									CREATE_SUSPENDED,
									&ThreadId
									);
	}


	// Set up members of the PROCESS_INFORMATION structure.
	ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );

	// Set up members of the STARTUPINFO structure.
	// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory( &siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = h_Child_STDOUT_Write;
	siStartInfo.hStdOutput = h_Child_STDOUT_Write;
	siStartInfo.hStdInput = h_Child_STDIN_Read;
	siStartInfo.dwFlags |= (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
	siStartInfo.wShowWindow = SW_HIDE;

	// Create the child process.
	bSuccess = CreateProcess( NULL,
	  (LPSTR)jtagEnablerExe.c_str(),
	  NULL,          // process security attributes
	  NULL,          // primary thread security attributes
	  TRUE,          // handles are inherited
	  0,             // creation flags
	  NULL,          // use parent's environment
	  NULL,          // use parent's current directory
	  &siStartInfo,  // STARTUPINFO pointer
	  &piProcInfo);  // receives PROCESS_INFORMATION


	// If an error occurs, exit the application.
	if ( ! bSuccess )
	{
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
        throw CWtpException(CWtpException::CREATEPROCESS, GetLastError(), CTimLib::FormatWindowsGetLastErrorToString(GetLastError()).c_str());
	}
	ResumeThread(m_DownloadThreadHandle);
	WaitForSingleObject(piProcInfo.hProcess, INFINITE);
}

/**
* Method:	JtagOverMedia
* Input:	void
* Output:	void
* Desc:		Launch JtagOverMedia i.e., enable Jtag over USB implemented in JTagEnabler
*			via pipes
*/
void CProtocolManager::JtagOverMedia (string jtagArgsString) throw (CWtpException)
{
	stringstream Message;
	Message << endl << "JTAG Reenablement Over Media" << endl;
	misc.UserMessagePrintStr( true,Message.str());
	Message.str("");
	SECURITY_ATTRIBUTES saAttr;

	// Set the bInheritHandle flag so pipe handles are inherited.

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle =TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	// Create a pipe for the child process's STDOUT.
	if(!CreatePipe(&h_Child_STDOUT_Read,&h_Child_STDOUT_Write,&saAttr,0))
	{
		Message << "JtagEnabler STDOUT pipe";
		throw CWtpException(CWtpException::CREATEPIPE,0,Message.str());
	}

	// Ensure the read handle to the pipe for STDOUT is not inherited.
   if ( ! SetHandleInformation(h_Child_STDOUT_Read, HANDLE_FLAG_INHERIT, 0) )
   {
		Message << "JtagEnabler's STDOUT Readpipe";
		throw CWtpException(CWtpException::SETHANDLEINFORMATION,0,Message.str());
   }

	// Create a pipe for the child process's STDIN.

	if (! CreatePipe(&h_Child_STDIN_Read, &h_Child_STDIN_Write, &saAttr, 0))
	{
		Message << "JtagEnabler STDIN pipe";
		throw CWtpException(CWtpException::CREATEPIPE,0,Message.str());
	}

	// Ensure the write handle to the pipe for STDIN is not inherited.
	if ( ! SetHandleInformation(h_Child_STDIN_Write, HANDLE_FLAG_INHERIT, 0) )
	{
		Message << "JtagEnabler's STDIN Writepipe";
		throw CWtpException(CWtpException::SETHANDLEINFORMATION,0,Message.str());
	}

	// Create the child process.
	createJtagEnablerChildProcess(jtagArgsString);

	// Read from pipe that is the standard output for child process.


    //readFromPipe();

	CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);
	CloseHandle(h_Child_STDOUT_Read);

	//CloseHandle(h_Child_STDOUT_Write);

	CloseHandle(h_Child_STDIN_Read);
	CloseHandle(h_Child_STDIN_Write);
}
#endif
#endif
/**
* Method:	OtpView
* Input:	N/A
* Output:	boolean
* Desc:		Sends OTPVIEW command to target
*/
/*
bool CProtocolManager::OtpView ()
{
	bool IsOk = true;
	int i;
	string Message;
	WtpStatus.ResetWtpStatus();

	//HandlePendingMessages();
	COtpViewCommand WtpCmd;
	Port->WritePort(WtpCmd,WtpStatus);
	if (WtpStatus.GetStatus() == NACK)
    {
        IsOk = true;//don't return an error
		theApp.UserMessagePrintStr( true, string("\n  Error: OtpView returned a NACK!\n") );
    }
    else
    {
		if ((WtpStatus.GetFlags() & 0x40) == 0x40)
			theApp.UserMessagePrintStr( true,string("OTP Register 0 is Locked\n") );
        else
			theApp.UserMessagePrintStr( true, string("OTP Register 0 is Unlocked\n"));

		if ((WtpStatus.GetFlags() & 0x80) == 0x80)
			theApp.UserMessagePrintStr( true,string("OTP Register 1 is Locked\n"));
        else
			theApp.UserMessagePrintStr( true, string("OTP Register 1 is Unlocked\n"));

		theApp.UserMessagePrintStr( true, string("OTP Register 0:"));

		stringstream Message;
		for (i = 0; i < 8; i++){
			Message << "0x" << std::hex <<std::setprecision(8) << (int)WtpStatus.GetData()[i]<<endl;
			theApp.UserMessagePrintStr( true, Message.str() );
			Message.str("");
		}

		theApp.UserMessagePrintStr( true,string("OTP Register 1:"));
		for (i = 8; i < 24; i++) {
			Message << "0x" << hex <<std::setprecision(8) << (int)WtpStatus.GetData()[i]<<endl;
			theApp.UserMessagePrintStr( true, Message.str() );
			Message.str("");
		}

		theApp.UserMessagePrintStr( true, string("OTP Register 2:"));
        for (i = 24; i < 40; i++){
			Message << "0x" << hex <<std::setprecision(8) << (int)WtpStatus.GetData()[i]<<endl;
			theApp.UserMessagePrintStr( true, Message.str() );
			Message.str("");
		}
    }
	//HandlePendingMessages();
    return IsOk;
}
*/

/**
* Method:	DebugBoot
* Input:	N/A
* Output:	boolean
* Desc:		Sends DEBUGBOOT command to target.
*			Host sends the data of length the target can handle
*			in this command.
*			The target sends back an ACK/NACK in CWtpStatus object
*/
/*bool CProtocolManager::DebugBoot()
{
	bool IsOk = true;
	stringstream Message;

	BYTE FT = theApp.iOtpFlashType;

	WtpStatus.ResetWtpStatus();

	if (FT >= MAXFLASHTYPES) return false;
	Message << "Flash type selected: " << FlashTypeCodes[FT].Description << endl;
	theApp.UserMessagePrintStr( true, Message.str());

	theApp.UserMessagePrintStr( true, string("\nSending preamble..."));
	if (SendPreamble() == false)
		return false;

	theApp.UserMessagePrintStr( true, string("\nSending DebugBOOT command..."));

	//HandlePendingMessages();
	CDebugBootCommand WtpCmd;

	IsOk = Port->WritePort(WtpCmd,WtpStatus);
	if (WtpStatus.GetStatus() == NACK)
    {
        IsOk = false;
		theApp.UserMessagePrintStr( true, string("\n  Error: DebugBoot returned a NACK!") );
    }

	if (!IsOk )
		return IsOk;

	Done();
    return true;
}
*/
