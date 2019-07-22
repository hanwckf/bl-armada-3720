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

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

/**
 * Useful constants
 **/
#define BIT0        0x00000001
#define BIT1        0x00000002
#define BIT2        0x00000004
#define BIT3        0x00000008
#define BIT4        0x00000010
#define BIT5        0x00000020
#define BIT6        0x00000040
#define BIT7        0x00000080
#define BIT8        0x00000100
#define BIT9        0x00000200
#define BIT10       0x00000400
#define BIT11       0x00000800
#define BIT12       0x00001000
#define BIT13       0x00002000
#define BIT14       0x00004000
#define BIT15       0x00008000
#define BIT16       0x00010000
#define BIT17       0x00020000
#define BIT18       0x00040000
#define BIT19       0x00080000
#define BIT20       0x00100000
#define BIT21       0x00200000
#define BIT22       0x00400000
#define BIT23       0x00800000
#define BIT24       0x01000000
#define BIT25       0x02000000
#define BIT26       0x04000000
#define BIT27       0x08000000
#define BIT28       0x10000000
#define BIT29       0x20000000
#define BIT30       0x40000000
#define BIT31       0x80000000
#define SET32       0xffffffff
#define CLEAR32     0x00000000

#define BU_U32          unsigned int
#define BU_U16          unsigned short
#define BU_U8           unsigned char

typedef volatile unsigned long  VUINT32_T;
typedef unsigned long           UINT32_T;
typedef unsigned long long      UINT64_T;
typedef volatile unsigned int   VUINT_T;
typedef unsigned int            UINT_T;
typedef int                     INT_T;
typedef unsigned short          UINT16_T, USHORT;
typedef volatile unsigned short VUINT16_T;
typedef unsigned char           UINT8_T;
typedef char                    INT8_T;

//#define PASSED                  0
//#define FAILED                  1
#define TRUE                      1
#define FALSE                     0
#define LOCKED                  1
#define UNLOCKED                0

//#define NULL                  0
#ifndef NULL
#define NULL                    0
#endif

typedef void(*FnPVOID)(void);

typedef unsigned int            UINT,     *PUINT;    // The size is not important
typedef unsigned long long      UINT64,   *PUINT64;
typedef unsigned int            UINT32,   *PUINT32;
typedef unsigned short          UINT16,   *PUINT16;
typedef unsigned char           UINT8,    *PUINT8;
typedef unsigned char           UCHAR,BYTE,*PUCHAR;

typedef int                     INT,      *PINT;    // The size is not important
typedef long long               INT64,    *PINT64;
typedef int                     INT32,    *PINT32;
typedef short                   INT16,    *PINT16;
//typedef char                    INT8,     *PINT8;
typedef char                    CHAR,     *PCHAR;
//typedef void                    VOID,     *PVOID;

typedef volatile  UINT          VUINT,    *PVUINT;    // The size is not important
typedef volatile  UINT64        VUINT64,  *PVUINT64;
typedef volatile  UINT32        VUINT32,  *PVUINT32;
typedef volatile  UINT16        VUINT16,  *PVUINT16;
typedef volatile  UINT8         VUINT8,   *PVUINT8;
typedef volatile  UCHAR         VUCHAR,   *PVUCHAR;

typedef volatile  INT           VINT,     *PVINT;    // The size is not important
typedef volatile  INT64         VINT64,   *PVINT64;
typedef volatile  INT32         VINT32,   *PVINT32;
typedef volatile  INT16         VINT16,   *PVINT16;
//typedef volatile  INT8          VINT8,    *PVINT8;
typedef volatile  CHAR          VCHAR,    *PVCHAR;

typedef UINT16    WORD;         /* Unsigned 16-bit quantity       */
typedef unsigned long DWORD;    /* Unsigned 32-bit quantity       */
typedef void*     VOID_PTR;

typedef struct
{
UINT_T  ErrorCode;
UINT_T  StatusCode;
UINT_T  PCRCode;
}FUNC_STATUS, *pFUNC_STATUS;

//some simple defines to clean up code a touch
#define reg_write(reg, val) *(volatile unsigned int *)reg = val; *(volatile unsigned int *)reg;
#define reg_bit_set(reg, val) *(volatile unsigned int *)reg |= val; *(volatile unsigned int *)reg;
#define reg_bit_clr(reg, val) *(volatile unsigned int *)reg &= ~val; *(volatile unsigned int *)reg;

#define BU_REG_READ(x) (*(volatile UINT32 *)(x))
#define BU_REG_WRITE(x,y) ((*(volatile UINT32 *)(x)) = y )

#define BU_REG_READ16(x) (*(volatile UINT16 *)(x) & 0xffff)
#define BU_REG_WRITE16(x,y) ((*(volatile UINT16 *)(x)) = y & 0xffff )

#define BU_REG_READ8(x) (*(volatile UINT8 *)(x) & 0xff)
#define BU_REG_WRITE8(x,y) ((*(volatile UINT8 *)(x)) = y & 0xff )

#define BU_REG_RDMDFYWR(x,y)  (BU_REG_WRITE(x,((BU_REG_READ(x))|y)))

#undef DBGPRINT

#ifdef DBGPRINT
    extern int armprintf(const char *fmt, ...);
    #define DEBUGMSG(cond,str) ((void)((cond)?(armprintf str),1:0))
#else
    #define DEBUGMSG(cond,str) ((void)0)
#endif // DEBUG


// Operating Mode Type for BootLoader
typedef enum {
  UPGRADESW = 1,
  DOWNLOAD,
  SINGLE_TIM_BOOT,
  DUAL_TIM_BOOT
} OPERATING_MODE_T;

#endif //_TYPEDEF_H_
