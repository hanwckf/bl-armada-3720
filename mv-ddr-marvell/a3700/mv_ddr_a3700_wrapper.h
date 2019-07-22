/*
* ***************************************************************************
* Copyright (C) 2017 Marvell International Ltd.
* ***************************************************************************
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* Neither the name of Marvell nor the names of its contributors may be used
* to endorse or promote products derived from this software without specific
* prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
* OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
***************************************************************************
*/

#ifndef _MV_DDR_A3700_WRAPPER_H
#define _MV_DDR_A3700_WRAPPER_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long uint64_t;
typedef uint64_t uintptr_t;

#define UINT32_MAX 0xffffffff

/* The following is a list of Marvell status */
#define MV_ERROR	(-1)
#define MV_OK		(0x00)	/* Operation succeeded                   */
#define MV_FAIL		(0x01)	/* Operation failed                      */
#define MV_BAD_VALUE	(0x02)	/* Illegal value (general)               */
#define MV_OUT_OF_RANGE	(0x03)	/* The value is out of range             */
#define MV_BAD_PARAM	(0x04)	/* Illegal parameter in function called  */
#define MV_BAD_PTR	(0x05)	/* Illegal pointer value                 */
#define MV_BAD_SIZE	(0x06)	/* Illegal size                          */
#define MV_BAD_STATE	(0x07)	/* Illegal state of state machine        */
#define MV_SET_ERROR	(0x08)	/* Set operation failed                  */
#define MV_GET_ERROR	(0x09)	/* Get operation failed                  */
#define MV_CREATE_ERROR	(0x0a)	/* Fail while creating an item           */
#define MV_NOT_FOUND	(0x0b)	/* Item not found                        */
#define MV_NO_MORE	(0x0c)	/* No more items found                   */
#define MV_NO_SUCH	(0x0d)	/* No such item                          */
#define MV_TIMEOUT	(0x0e)	/* Time Out                              */
#define MV_NO_CHANGE	(0x0f)	/* Parameter(s) is already in this value */
#define MV_NOT_SUPPORTED (0x10)	/* This request is not support           */
#define MV_NOT_IMPLEMENTED (0x11) /* Request supported but not implemented*/
#define MV_NOT_INITIALIZED (0x12) /* The item is not initialized          */
#define MV_NO_RESOURCE	(0x13)	/* Resource not available (memory ...)   */
#define MV_FULL		(0x14)	/* Item is full (Queue or table etc...)  */
#define MV_EMPTY	(0x15)	/* Item is empty (Queue or table etc...) */
#define MV_INIT_ERROR	(0x16)	/* Error occured while INIT process      */
#define MV_HW_ERROR	(0x17)	/* Hardware error                        */
#define MV_TX_ERROR	(0x18)	/* Transmit operation not succeeded      */
#define MV_RX_ERROR	(0x19)	/* Recieve operation not succeeded       */
#define MV_NOT_READY	(0x1a)	/* The other side is not ready yet       */
#define MV_ALREADY_EXIST (0x1b)	/* Tried to create existing item         */
#define MV_OUT_OF_CPU_MEM   (0x1c) /* Cpu memory allocation failed.      */
#define MV_NOT_STARTED	(0x1d)	/* Not started yet                       */
#define MV_BUSY		(0x1e)	/* Item is busy.                         */
#define MV_TERMINATE	(0x1f)	/* Item terminates it's work.            */
#define MV_NOT_ALIGNED	(0x20)	/* Wrong alignment                       */
#define MV_NOT_ALLOWED	(0x21)	/* Operation NOT allowed                 */
#define MV_WRITE_PROTECT (0x22)	/* Write protected                       */
#define MV_INVALID	(int)(-1)

#ifndef NULL
#define NULL ((void *)0)
#endif

#define MV_STATUS	int
#define MV_BOOL		int
#define MV_TRUE		(int)(1)
#define MV_FALSE	(int)(0)
#define MV_U8		u8
#define MV_U16		u16
#define MV_U32		u32
#define MV_8		char
#define MV_32		int

void mdelay(unsigned int msec);
void reg_bit_clrset(u32 addr, u32 val, u32 mask);
void reg_write(u32 addr, u32 val);
u32 reg_read(u32 addr);

/* printf function */
#include <stdio.h>
#include <string.h>

#endif /* _MV_DDR_A3700_WRAPPER_H */
