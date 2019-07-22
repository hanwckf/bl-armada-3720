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
#define WTPTP_3_2_X 0
#include "WtpException.h"

using namespace std;

class CWtpException;
class CCommandlineParser
{
private:
	unsigned int FileGetImageID(string fileName) throw(CWtpException);
public:

#if WTPTP_3_2_X // previous version commandline switches now obsolete
	enum eCommandlineOption{VERBOSE			= 'V',
							TIM_BIN_FILE	= 't',
							IMAGE_FILE		= 'f',
							JTAG_KEY		= 'k',
							MESSAGE_MODE	= 'M',
							PORT_TYPE		= 'P',
							INTERFACE_NAME	= 'i',
							USB_PACKET_SIZE	= 'U',
							COMM_PORT		= 'C',
							BAUD_RATE		= 'B',
							PLATFORM_TYPE	= 'T',
							PARTITION_TABLE	= 'p',
							DISABLE_FAST_DOWNLOAD = 'N',
							LOG_FILE		= 'L',
							UPLOAD_DATA     = 'W',
							HELP			= 'h'};
#endif
						enum eCommandlineOption{
							VERBOSE = 'V',
							TIM_BIN_FILE = 'B',
							IMAGE_FILE = 'I',
							JTAG_KEY = 'J',
							MESSAGE_MODE = 'M',
							PORT_TYPE = 'P',
							INTERFACE_NAME = 'G',
							USB_PACKET_SIZE = 'U',
							COMM_PORT = 'C',
							BAUD_RATE = 'R',
							PLATFORM_TYPE = 'T',
							PARTITION_TABLE = 'Q',
							DISABLE_FAST_DOWNLOAD = 'N',
							LOG_FILE = 'L',
							UPLOAD_DATA = 'W',
							GET_PC = 'D',
							FORCE_UART_IMAGE_LOAD = 'O',
							FORCE_BOOTROM_CONSOLE_MODE = 'Y',
							SEND_FF_STREAM = 'y',
							SEND_FF_STREAM_DLOAD = 'o',
					#if USE_USB_DELAY_SWITCHES == 1
							ENUMERATE_DELAY = 'E',
							ENUMERATE_ARRIVE_WAIT = 'A',
							USB_READ_DELAY  = 'Z',
					#endif
							HELP = 'H',
							ENABLER_EXE_PATH = 'X', // overrides default
							PLAINTEXT_WTPCR = 'E'
												};

	enum eJtagEnablerCommandLineOption{
                    JTAG_CONNECT_TEST       = 'J',
                    INQUIRE_TARGET          = 'I',
                    PROC_TYPE		        = 'T',
                    FORCE_RESET				= 'X',

                    ENABLE_JTAG_CMD	        = 'E',
                    CORES_TO_ENABLE         = 'P',
                    JTAG_KEY_FILE           = 'K',
                    SIGNED_UID_KEY_FILE     = 'k',
                    GET_KEY_PROVIDER        = 'G',

                    // Password Access Control word:
                    AC_REQUEST_TYPE         = 'R',

                    // Set Parameter Command
                    SET_PARAMETERS_CMD      = 'S',
                    PARAM_BOOT_STATE        = 'B',
                    PARAM_FORCE_DOWNLOAD    = 'F',
                    PARAM_FORCE_UART_LOG    = 'L',

                    AC_PERMANENTLY_DISABLE  = 'D',
                    AC_FA_STATE             = 'A',

                    // JTAG Config
                    CHANGE_TCK_FREQ         = 'C',

                    USE_WTM_TO_RUN_PROTOCOL = 'U',
					PORTTYPE				= 'Y',
					COMMPORT				= 'N',
                    BAUDRATE                = 'M',
                    //USB_INTERFACENAME       = 'Q',

                    // SLE target controls
                    TARGET_TIMEOUT = 'O',
                    POLLING_DELAY = 'W',

                    TRACE                   = 'Z',
                    ENABLER_VERBOSE         = 'V',

                    ENABLER_HELP	        = 'H'
	};

	void Parse(int argc, char* argv[]) throw(CWtpException);
	void ParseJtagEnablerCommandLine(int argc, char* argv[]) throw (CWtpException);


	void InsufficientParameters(unsigned char eCmdLineOpt) throw(CWtpException);
	void RecalculateUSBPacketSize( );

	void AddImageToList(char* FileName, unsigned int ID) throw (CWtpException);
	void CheckYourOptions() throw(CWtpException);

	CCommandlineParser(void);
	~CCommandlineParser(void);
};
