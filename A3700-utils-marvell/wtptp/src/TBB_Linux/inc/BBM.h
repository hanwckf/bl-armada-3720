/******************************************************************************
 **
 **  COPYRIGHT (C) 2002, 2003 Intel Corporation.
 **
 **  This software as well as the software described in it is furnished under
 **  license and may only be used or copied in accordance with the terms of the
 **  license. The information in this file is furnished for informational use
 **  only, is subject to change without notice, and should not be construed as
 **  a commitment by Intel Corporation. Intel Corporation assumes no
 **  responsibility or liability for any errors or inaccuracies that may appear
 **  in this document or any software that may be provided in association with
 **  this document.
 **  Except as permitted by such license, no part of this document may be
 **  reproduced, stored in a retrieval system, or transmitted in any form or by
 **  any means without the express written consent of Intel Corporation.
 **
 **  FILENAME:	Flash.h
 **
 **  PURPOSE: 	Holds all flash related definitions
 **
 ******************************************************************************/
  /********************************************************************************
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

#ifndef __BBM_H__
#define __BBM_H__

#include "Typedef.h"

#define MARVELL_BBT_VERSION	0x31303031

// Indicates the state of the BBT in volatile memory
typedef enum {
	BBT_INVALID = 1, BBT_UNCHANGED = 2, BBT_CHANGED = 3, BBT_ERASED = 4
} ReloState_T;

// BBT Types
typedef enum {
	BBT_TYPE_NA = 0x00000000,		//Runtime BBT not being used
	BBT_TYPE_LEGACY = 0x0000524E,		//Legacy version
	BBT_TYPE_MBBT = 0x4D424254,	//"MBBT" Marvell Bad Block Table (NOTE: used as ID for factory AND runtime MBBT)
	BBT_TYPE_MBBT_FACT = 0x46616374,//"Fact" Factory Bad Block Table (sub type of the MBBT)
	BBT_TYPE_MBBT_RUN = 0x52756E74,	//"Runt" Runtime Bad Block Table (sub type of the MBBT)
	BBT_TYPE_WNCE = 0x574E4345,		//"WNCE" WinCE Bad Block Table
	BBT_TYPE_LINX = 0x4C695E78		//"Linx" Linux Bad Block Table
} BBT_TYPE;

//Relocation Pairs - same for MBBT, FBBT, and Legacy BBT
typedef struct {
	USHORT From;
	USHORT To;
} ReloPair_T, *P_ReloPair_T;

// Marvell BBT Structure
typedef struct {
	UINT_T Identifier;
	UINT_T Version;
	UINT_T Type;
	UINT_T Reserved;
	UINT_T PartitionID;
	UINT_T NumRelo;
	UINT_T BBTLocation;
	UINT_T Reserved1;//for now, the upper 32bits of 64bit address are reserved
	UINT_T BackupBBTLoc;
	UINT_T Reserved2;//for now, the upper 32bits of 64bit address are reserved
	ReloPair_T Entry[2]; //need to make it an array
} MBBT_Table_T, *P_MBBT_Table_T;

// Factory BBT Structure
typedef struct {
	UINT_T Identifier;
	UINT_T Version;
	UINT_T Type;
	UINT_T Reserved0;
	UINT_T Reserved1;
	UINT_T NumRelo;
	UINT_T BBTLocation;
	UINT_T Reserved2;//for now, the upper 32bits of 64bit address are reserved
	UINT_T Reserved3;
	UINT_T Reserved4;
	USHORT BadBlocks[2]; //need to make it an array
} FBBT_Table_T, *P_FBBT_Table_T;

// Legacy BBT Structure
typedef struct {
	USHORT Header;
	USHORT NumReloc;
	ReloPair_T Relo[2];
} ReloTable_T, *P_ReloTable_T;

#endif
