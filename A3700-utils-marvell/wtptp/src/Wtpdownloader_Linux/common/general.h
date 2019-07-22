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
#ifndef _general_h
#define _general_h

#include "Typedef.h"

#ifndef BYTES_PER_WORD
	#define BYTES_PER_WORD (sizeof( UINT_T ))
#endif

#define MAXRSAKEYSIZEWORDS	 	64				// 2048 bits
#define MAXECCKEYSIZEWORDS 		17				// 521 bits+


/*
 *  Fuse definition structure
 */
struct PLATFORM_FUSES
{
	unsigned int DebugDisable		:1;		// Bit 0 of SYS_BOOT_CNTRL and bit 79 dis_strap_override of AP config bits
	unsigned int PlatformState		:5;		// Bit 62-66 of AP config bits and bits 22-26 of SYS_BOOT_CNTRL
	unsigned int TZInitialized		:1;		// TrustZone Initialized
	unsigned int JTAG_Disable		:1;		// Bit 6 of SYS_BOOT_CNTRL
	unsigned int UseWTM				:1;		// Bit 20 of SYS_BOOT_CNTRL (MMP2 and MRNA only)
	unsigned int CheckOEMKeyOnProbe :1; 	// Force checking Platform Verification keys on Probe.
	unsigned int EscapeSeqDisable	:1;		// Bit 17 of SYS_BOOT_CNTRL (Tavor TD only)
	unsigned int USBDisable			:1;		// Bits 12 USBDisable of SYS_BOOT_CNTRL
	unsigned int UARTDisable		:1;		// Bits 13 UARTDisable of SYS_BOOT_CNTRL
	unsigned int MMCDisable			:1;		// Bit 14 MMC Enable of SYS_BOOT_CNTRL
	unsigned int Resume				:1;		//
	unsigned int USBWakeup			:1;		// Bit 16 of SYS_BOOT_CNTRL
	unsigned int PortEnabled		:1;		// Bit 17: 1 = Ports have been enabled by bootROM
	unsigned int DDRInitialized		:1; 	// Bit 18: If the BootROM already initializes the DDR
	unsigned int Download_Disable	:1;		// Bit 19 of SYS_BOOT_CNTRL
	unsigned int SBE				:1;		// Bit 20 of SYS_BOOT_CNTRL (Not for MRNA or MMP2)
	unsigned int OverrideDisable	:1;		// Bit 78 of AP fuse config bits dis_aib_override
	unsigned int BootPlatformState	:5;		// Bits 22-26: There are the bits the BootROM booted from
    unsigned int USBPort            :2;     // Bits 26-27: 0 - DIFFUSB 1 - SEUSB
    unsigned int UARTPort           :2;     // Bits 28-29: 0 - FFUART, 1 - AltUART
	unsigned int TBROpMode			:2;		// Bits 30-31: Bits 10,11 od SYS_BOOT_CNTRL (MRNA and MMP2) only.
};

typedef enum
{
	FFUART_PORT = 0,
	ALTUART_PORT = 1
} UART_PORTS;

typedef enum
{
	USB_DIFF_PORT = 0,
	USB_SE_PORT = 1,
	USB_U2D_PORT = 2,
	USB_CI2_PORT = 3
} USB_PORTS;


typedef union{
	unsigned int value;
	struct PLATFORM_FUSES bits;
}FUSE_SET, *pFUSE_SET;

//list of IDS for use in XFER structure
typedef enum
{
	TIM_DATA = 0x54494D48,	// "TIMH" - refers to tim image
	PT_DATA = 0x4D505420, 	// "MPT " - refers to partition table
	BBT_DATA = 0x4D424254, 	// "MBBT" - refers to bad block table
	RD_DATA = 0x52444953,	// "RDIS" - refers to read disturb list
	TIMFAIL_DATA = 0x54494D46	// "TIMF" - refers to Tim Boot Failure Structure
} XFER_DATA;

typedef struct
{
	XFER_DATA data_id;
	UINT_T location;
}	XFER_DATA_PAIR_T, *P_XFER_DATA_PAIR_T;

// This is the Transfer Structure.
//  The first word is the id = 'TBRX'
//  The next few words are fixed data
//  lastly, there is a key/value array to indicate what the bootrom is passing to next image
typedef struct
{
	UINT_T TransferID;
	UINT_T SOD_ACCR0;
	UINT_T FuseVal;
	UINT_T ErrorCode;
	UINT_T ResumeParam[4];
	UINT_T num_data_pairs;
	XFER_DATA_PAIR_T data_pairs[1];
} TRANSFER_STRUCT, *P_TRANSFER_STRUCT;

typedef enum
{
	Load_Failure 	   = 0,
	Validation_Failure = 1
} FailureCause;

typedef struct
{
	UINT_T ImageId;
	FailureCause Cause;
} TIM_FAILURE_STRUCT, pTIM_FAILURE_STRUCT;


typedef enum
{
	BACKUPTIM_NOTINUSE  = 0, //This is the initial state of backup boot when the platform is booting with
							 //TIM and BootRom hasn't gotten to BackupTim yet
	BACKUPTIM_INUSE     = 1,
	BACKUPTIM_FAILED	= 2,
	BACKUPTIM_NOTSUPPORTED = 3
} BACKUPTIM_STATUS;


// **************** Flash Boot State Offsets *************
// Default TIM flash locations
#define TIMOffset_CS2						0x00000000
#define TIMOffset_NAND						0x00000000
#define TIMOffset_MDOC						0x00000000	// Could be in partition 2
#define TIMOffset_ONENAND					0x00000000
#define TIMOffset_SDMMC						0x00000000	// Could be in Partition 1
#define BackupTIMOffset_SDMMC               0x1400      //dec:5120, 11th block

// For debug purposes only
#define HEX_LED_CONTROL (volatile unsigned long *)( 0x08000040 )
#define HEX_LED_WRITE (volatile unsigned long *)( 0x08000010 )

#endif




