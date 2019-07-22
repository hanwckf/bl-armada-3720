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

#include "../sys.h"

#define ll_write64(addr, data)                                  (*(volatile unsigned long long *)(uintptr_t)(addr)=data)                        //low level write
#define ll_write32(addr, data)                                  (*(volatile unsigned int *) (addr)=data)                                        //low level write
#define ll_read32(addr)                                         (*(volatile unsigned int *)(addr))                                              //low level read

extern int debug_level;
extern int debug_module;
#define LogMsg(log_level, log_module, fmt, ...) do { if ( (DEBUG) && (log_level <= debug_level) && ((log_module & debug_module) > 0) ) { printf( fmt, ##__VA_ARGS__ ); } } while(0)

enum training { PHYINIT_SYNC2=0, INIT_TIMING=1, TERM=2, QS_GATE=3, VREF_READ=4, VREF_WRITE=5, DLL_TUNE=6};

void replace_val(unsigned int addr, unsigned int data, unsigned int offset, unsigned int mask);
void logs_training_regs(enum training type);
