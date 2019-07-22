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

#ifdef WINDOWS
#undef FAILED
#endif

#include "Typedef.h"
#include <string>
using namespace std;

#define DATALENGTH    8192 //(4096*2)-8+1
#define RETURNDATALENGTH (1024*8)-6
#define MESSAGELENGTH 256
//#define WTPSTATUSLENGTH 3996

#ifndef _WINDEF_
typedef unsigned char BYTE;
#endif

#define MIN_ARGS 2
#define MAX_PATH 260
#define MAX_IMAGE_FILES 20

typedef struct
{
	DWORD FileType;
	char FileName[MAX_PATH];
	char ShortFileName[MAX_PATH];
} IMAGEFILE;

typedef enum {
   DISABLED = 1,		// Standard WTPTP download
   MSG_ONLY = 2,		//Turn on message mode(listen for msgs forever)
   MSG_AND_DOWNLOAD = 3, //Msg mode and WTPTP download
   DOWNLOAD_ONLY = 4
} eMessageMode;

#define USB 0
#define UART 1
#define JTAG 2
#define UNKNOWNPORT -1

//define PACKET_SIZE = 504;  // plus command makes 512
#define PACKET_SIZE  4088;  // plus command makes 4096

#define ACK 0
#define NACK 1
#define MESSAGEPENDING 1
#define MESSAGESTRING 0
#define MESSAGEERRORCODE 1

#define GET           0      //added for fast download proj--flag for fast download.
#define SET           1      //added for fast download proj--flag for fast download.
#define FDMODE        1      //dummy/default value when getting the value for fast download.

typedef enum {
   IMAGE_DOWNLOAD_ERROR = 1,		// Download of binary failed
   } WTPTP_ERRORS;

// Protocol command definitions

#define PREAMBLE 0x00
#define PLAINTEXTWTPCR 0x77
#define PUBLICKEY 0x24
#define PASSWORD 0x28
#define SIGNEDPASSWORD 0x25
#define GETVERSION 0x20
#define SELECTIMAGE 0x26
#define VERIFYIMAGE 0x27
#define DATAHEADER 0x2A
#define DATA 0x22
#define MESSAGE 0x2B
#define OTPVIEW 0x2C
#define DEBUGBOOT 0x2D
#define DONE 0x30
#define DISCONNECT 0x31
#define UPLOADDATAHEADER 0x32
#define UPLOADDATA 0x33
#define PROTOCOLVERSION 0x34
#define GETPARAMETERS 0x35
#define JTAGOVERMEDIA 0x36
#define GETPC 0x37
#define UNKNOWNCOMMAND 0xFF

#pragma pack(push,1)
typedef struct
{
	BYTE CMD;
	BYTE SEQ;
	BYTE CID;
	BYTE Flags;
	UINT_T LEN;
	BYTE Data[DATALENGTH];
} WTPCOMMAND;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
	BYTE CMD;
	BYTE SEQ;
	BYTE CID;
	BYTE Status;
	BYTE Flags;
	BYTE DLEN;
	BYTE Data[RETURNDATALENGTH];
} WTPSTATUS;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    BYTE MajorVersion;
    BYTE MinorVersion;
    short Build;
} PROTOCOL_VERSION;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    unsigned int BufferSize;
    unsigned int Rsvd1;
    unsigned int Rsvd2;
    unsigned int Rsvd3;
} TARGET_PARAMS;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    BYTE Type;
    BYTE SubType;
    BYTE Partition;
    BYTE Rsvd1;
    unsigned int Offset;
    unsigned int DataSize;
} UPLOAD_DATA_PARAMS;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    UPLOAD_DATA_PARAMS Params;
    string sOutputFileName;
    string sComparisonFileName;
} UPLOAD_DATA_SPEC;
#pragma pack(pop)

