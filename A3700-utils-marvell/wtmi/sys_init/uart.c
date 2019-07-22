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

#include "types.h"
#include "io.h"
#include "regs.h"
#include "bits.h"
#include "uart.h"
#include "delay.h"

#define UART_CLOCK_FREQ		25804800

static void uart_set_baudrate(unsigned int baudrate)
{
	/*
	 * calculate divider.
	 * baudrate = clock / 16 / divider
	 */
	writel((UART_CLOCK_FREQ / baudrate / 16), MVEBU_UART0_BAUD_REG);
	/* set Programmable Oversampling Stack to 0, UART defaults to 16X scheme */
	writel(0, MVEBU_UART0_POSSR_REG);
}

int uart_init(unsigned int baudrate)
{
	uart_set_baudrate(baudrate);

	/* reset FIFOs */
	writel(BIT14 | BIT15, MVEBU_UART0_CTRL_REG);

	wait_ns(1000);

	/* No Parity, 1 Stop */
	writel(0, MVEBU_UART0_CTRL_REG);

	return 0;
}

void uart_putc(void *p, char c)
{
	if (c == '\n')
		uart_putc(NULL, '\r');

	while (readl(MVEBU_UART0_STATUS_REG) & BIT11)
		;

	writel(c, MVEBU_UART0_TX_REG);

	return;
}
