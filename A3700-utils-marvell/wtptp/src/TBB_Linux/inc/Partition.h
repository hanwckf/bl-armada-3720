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


#ifndef __Partition_H__
#define __Partition_H__

#include "Typedef.h"
#include "BBM.h"

//Marvell Master Partition Table ID: "MRVL MPT"
#define MARVELL_PARTITION_TABLE_ID0	0x4D52564C
#define MARVELL_PARTITION_TABLE_ID1	0x204D5054

/* Partition Info values */
//Type
#define PI_TYPE_PHYSICAL	0x50687973		//"Phys"
#define PI_TYPE_LOGICAL		0x4C6F6769		//"Logi"

//Usage
#define PI_USAGE_BOOT		0x424F4F54		//"Boot" Boot Partition
#define PI_USAGE_OSLD		0x4F534C44		//"OSLD" OS Loader Partition
#define PI_USAGE_KRNL		0x4B524E4C		//"KRNL" Kernal Partition
#define PI_USAGE_FFOS		0x46464F53		//"FFOS" FF OS Partition
#define PI_USAGE_FSYS		0x46535953		//"FSYS" File System Partition

//Partition Attributes
typedef struct {
	UINT ReadOnly :1;
	UINT Writable :1;
	UINT Executable :1;
	UINT Compressed :1;
	UINT CompressType :4;
	UINT Reserved :24;
} PartAttributes_T;

//Reserved Pool Algorithm
#define PI_RP_ALGO_NA		0x00000000		//Reserved Pool Not Used
#define PI_RP_ALGO_UPWD		0x55505744		//"UPWD" Reserved Pool grows Upwards
#define PI_RP_ALGO_DNWD		0x444E5744		//"DNWD" Reserved Pool grows Downwards

/*  End of Partition Info values */

// Indicates the state of the PT in volatile memory
typedef enum {
	PT_INVALID = 1, PT_VALID = 2, PT_NEW = 3//this means that either the PT was loaded externally, OR block 0 was erased
} PTState_T;

//Partition table structure:
typedef struct {
	UINT_T Type;				//Partition Type
	UINT_T Usage;				//Partition Usage
	UINT_T Indentifier;		//Partition ID
	PartAttributes_T Attributes;			//Attributes for this partition
	UINT_T StartAddr;			//Absolute address of start of partition
	UINT_T Reserved1;//for now, the upper 32bits of 64bit address are reserved
	UINT_T EndAddr;			//Absolute address of end of partition
	UINT_T Reserved2;//for now, the upper 32bits of 64bit address are reserved
	UINT_T ReserveStartAddr;	//Absolute address of start of reserved pool
	UINT_T Reserved3;//for now, the upper 32bits of 64bit address are reserved
	UINT_T ReserveSize;		//Size of the reserved pool
	UINT_T Reserved4;//for now, the upper 32bits of 64bit address are reserved
	UINT_T ReserveAlgorithm;	//Reserved pool algorithm
	BBT_TYPE BBT_Type; 			//runtime BBT type
	UINT_T RBBT_Location;		//Absolute address of runtime BBT
	UINT_T Reserved5;//for now, the upper 32bits of 64bit address are reserved
	UINT_T BackupRBBT_Location;			//Absoulte address of backup runtime BBT
	UINT_T Reserved6;//for now, the upper 32bits of 64bit address are reserved
	UINT_T Reserved[2];
} PartitionInfo_T, *P_PartitionInfo_T;

typedef struct {
	union {
		UINT64 Identifier;		//Partiton Table ID
		struct {
			UINT_T Identifier0;	// low half - for easier code access
			UINT_T Identifier1;	// hi half
		};
	};

	UINT_T Version;		//Partition Version
	UINT_T NumPartitions;	//Number of partitions
	UINT_T Reserved[2];
} PartitionTable_T, *P_PartitionTable_T;

#endif

