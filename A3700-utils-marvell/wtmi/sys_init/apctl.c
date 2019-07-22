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

#include "sys.h"
#include "apctl.h"

#define MAILBOX_MAX_ARGS		16
#define HOST_INT_CMD_COMPLETE_BIT	BIT0

#define MAILBOX_OUT_STATUS		(SECURE_CPU_REGS_BASE + 0x80)
#define MAILBOX_OUT_ARG(n)		(SECURE_CPU_REGS_BASE + 0x84 + n * 4)
#define HOST_INT_SET			(SECURE_CPU_REGS_BASE + 0x234)


/***************************************************************************************************
  * mbox_send
  *
  * status - status to return to host
  * args - arguments to return to host
  * nargs - number or argumens
 *
 ***************************************************************************************************/
static void mbox_send(u32 status, u32 *args, u32 nargs)
{
	u32 n, regval;

	if ((args == 0) || (nargs >= MAILBOX_MAX_ARGS))
		return;

	for (n = 0; n < nargs; n++)
		writel(args[n], MAILBOX_OUT_ARG(n));

	/* Set status */
	writel(status, MAILBOX_OUT_STATUS);
	/* Pop host INT CMD complete */
	regval = readl(HOST_INT_SET) | HOST_INT_CMD_COMPLETE_BIT;
	writel(regval, HOST_INT_SET);
}

int kick_ap(void)
{
	u32 args[MAILBOX_MAX_ARGS];

	args[0] = 0x04100000;
	mbox_send(0x1003, args, 1);
	return 0;
}

