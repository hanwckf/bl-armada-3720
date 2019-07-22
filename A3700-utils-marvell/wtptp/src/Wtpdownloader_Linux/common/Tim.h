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

#ifndef __TIM_H__
#define __TIM_H__

#include "general.h"

/******************* TIM Layout ************************************/
/* Below Presents the Maxiumum Size TIM layout in memory

VERSION_I      	VersionBind;         			//
FLASH_I        	FlashInfo;           			//
UINT_T         	NumImages;           			//
UINT_T         	NumKeys;						//
UINT_T         	SizeOfReserved;					//
IMAGE_INFO 		IMG[MAX_IMAGES];                //
KEY_MOD        	Key[MAX_KEYS];                  //
UINT_T         	Reserved[0x4E8];       			//
PLAT_DS        	TBTIM_DS;                       //
                                                //
Below Presents the Maxiumum Size NTIM layout in memory

VERSION_I   	VersionBind;         			//
FLASH_I     	FlashInfo;           			//
UINT_T      	NumImages;           			//
UINT_T         	NumKeys;						//
UINT_T      	SizeOfReserved;					//
IMAGE_INFO 		IMG[MAX_IMAGES];                //
UINT_T      	Reserved[0xD80];       			//
NTIM, *pNTIM;									//
/******************* TIM Layout ************************************/

// TIM Versions
#define TIM_3_2_00			0x30200			// Support for Partitioning
#define TIM_3_3_00			0x30300			// Support for ECDSA-256 and 64 bit addressing
#define TIM_3_4_00			0x30400			// Support for ECDSA-521
#define TIM_3_5_00			0x30500			// Support for Encrypted Boot
#define TIM_3_6_00			0x30600			// Support for Stepping in VERSION_I and refactoring FLASH_I

// B1 TIM
#include "Typedef.h"
#define MAX_IMAGES			256
#define MAX_KEYS	 		256
#define TIMBUFFER 			4096    		//4k for a Tim structure size

// Predefined Image Identifiers
#define TIMIDENTIFIER		0x54494D48		// "TIMH"
#define TIMDUALBOOTID		0x54494D44		// "TIMD"
#define WTMIDENTIFIER		0x57544D49		// "WTMI"
#define OBMIDENTIFIER		0x4F424D49		// "OBMI"
#define MONITORIDENTIFIER	0x4D4F4E49		// "MONI"
#define TZSWIDENTIFIER		0x545A5349		// "TZSI"    This is also used as consumer ID
#define TZIIDENTIFIER		0x545A4949		// "TZII"    This is also used as consumer ID
#define TBRIDENTIFIER		0x54425249		// "TBRI"	 This is also used as consumer ID
#define DKBIDENTIFIER		0x444B4249		// "DKBI"
#define JTAGIDENTIFIER		0x4A544147		// "JTAG"
#define PATCHIDENTIFIER		0x50415443		// "PATC"
#define TCAIDENTIFIER		0x5443414B		// "TCAK"
#define DSIGIDENTIFIER      0x44534947      // "DSIG"
#define ENCKIDENTIFIER      0x454E434B      // "ENCK"
#define OSLOADERID			0x4F534C4F		// "OSLO"
#define PARTIONIDENTIFIER	0x50415254      // "PART"
#define HSIBOOTID			0x48534949		// "HSII"
#define SDIOBOOTID			0x53444949		// "SDII"
#define OSLOADERRECOVERYID	0x4F534C52		// "OSLR"
#define KERNELID			0x4B524E4C		// "KRNL"
#define KERNELRECOVERYID	0x4B524E52		// "KRNR"
#define RAMDISKID			0x52414D44		// "RAMD"
#define RAMDISKRECOVERYID	0x52414D52		// "RAMR"
#define NONTZDTIMID			0x54494D4E		// "TIMN" representing Non-TZ DTIM
#define CSK0ID				0x43534B30		// "CSK0"
#define CSK1ID				0x43534B31		// "CSK1"
#define CSK2ID				0x43534B32		// "CSK2"
#define CSK3ID				0x43534B33		// "CSK3"
#define CSK4ID				0x43534B34		// "CSK4"
#define CSK5ID				0x43534B35		// "CSK5"
#define CSK6ID				0x43534B36		// "CSK6"
#define CSK7ID				0x43534B37		// "CSK7"
#define CSK8ID				0x43534B38		// "CSK8"
#define CSK9ID				0x43534B39		// "CSK9"
#define CSKAID				0x43534B41		// "CSKA"
#define CSKBID				0x43534B42		// "CSKB"
#define CSKCID				0x43534B43		// "CSKC"
#define CSKDID				0x43534B44		// "CSKD"
#define CSKEID				0x43534B45		// "CSKE"
#define CSKFID				0x43534B46		// "CSKF"

//Define ID types
//  Use upper 3 bytes as a type identifier and allow up to 256 of a particular type
//
#define TYPEMASK			0xFFFFFF00
#define DDRTYPE				0x444452	 	// "DDR"
#define TIMTYPE				0x54494D		// "TIM"
#define TZRTYPE				0x545A52		// "TZR"
#define GPPTYPE				0x475050		// "GPP"

// WTP Format Recognized Reserved Area Indicator
#define WTPRESERVEDAREAID	0x4F505448      // "OPTH"

// Reserved Area Package Headers
#define TERMINATORID		0x5465726D		// "Term"
#define GPIOID				0x4750494F		// "GPIO"
#define UARTID				0x55415254		// "UART"
#define USBID				0x00555342		// "USB"
#define USBVENDORREQ		0x56524551		// "VREQ"
#define TBR_XFER			0x54425258		// "TBRX"
#define ESCAPESEQID			0x45534353		// "ESCS"
#define ESCSEQID_V2         0x45535632      // "ESV2"   Escape Sequence Version 2
#define OEMCUSTOMID			0x43555354		// "CUST"
#define NOMONITORID			0x4E4F4D43		// "NOMC"
#define COREID				0x434F5245		// "CORE"
#define COREID_V2           0x43525632      // "CRV2"

//#define SOCPROFILEID		0x534F4350		// "SOCP"
#define BBMTYPEID			0x42424D54		// "BBMT"
#define QNXBOOT             0x514E5850      // "QNXP"
#define GPP1ID				0x47505031		// "GPP1"  Processed in RESERVEDAREA_State
#define GPP2ID				0x47505032		// "GPP2"  Processed in XFER_State
#define ROMRESUMEID			0x52736D33		// "Rsm3"
#define PINID               0x50494E50      // "PINP"
#define IMAPID              0x494D4150      // "IMAP"
#define ONDEMANDBOOTTID     0x4F444254      // "ODBT"
#define FLASHGEOPKGID       0x46475049      // "FGPI"

// DDR Related ID's
#define DDRID				0x44447248		// "DDrH"
#define DDRGID				0x44445247		// "DDRG"
#define DDRTID				0x44445254		// "DDRT"
#define DDRCID				0x44445243		// "DDRC"
#define CMCCID				0x434d4343		// "CMCC"
#define DTYPID				0x44545950		// "DTYP"

// new DDR configuration related ID's
#define CIDPID              0x43494450      // "CIDP"
#define TZRIID	            0x545A5249	    // ”TZRI”
#define MAX_SCRATCH_MEMORY_ID	15			// 16 Scratch Memories defined
#define MAX_LABEL_CACHE_SIZE	32			// Entries
#define MAX_STACK_SIZE 			32			// Entries

// Operating mode related ID's
#define FREQID				0x46524551		// "FREQ"
#define VOLTID				0x564f4c54		// "VOLT"
#define OPMODEID			0x4d4f4445		// "MODE"
#define OPDIVID 			0x4f504456		// "OPDV"
#define CLKEID				0x434c4b45		// "CLKE"

// trust zone package
#define TZID				0x545A4944      // "TZID"
#define TZON                0x545A4f4E      // "TZON" Register based


// USB
#define DESCRIPTOR_RES_ID	0x55534200      // "USB"
#define NUM_USB_DESCRIPTORS			10
#define MAX_USB_STRINGS 			7

typedef enum
{
 USB_DEVICE_DESCRIPTOR   				= (DESCRIPTOR_RES_ID | 0),
 USB_CONFIG_DESCRIPTOR   				= (DESCRIPTOR_RES_ID | 1),
 USB_INTERFACE_DESCRIPTOR 				= (DESCRIPTOR_RES_ID | 2),
 USB_LANGUAGE_STRING_DESCRIPTOR   		= (DESCRIPTOR_RES_ID | 3),
 USB_MANUFACTURER_STRING_DESCRIPTOR   	= (DESCRIPTOR_RES_ID | 4),
 USB_PRODUCT_STRING_DESCRIPTOR   	  	= (DESCRIPTOR_RES_ID | 5),
 USB_SERIAL_STRING_DESCRIPTOR   	  	= (DESCRIPTOR_RES_ID | 6),
 USB_INTERFACE_STRING_DESCRIPTOR   		= (DESCRIPTOR_RES_ID | 7),
 USB_DEFAULT_STRING_DESCRIPTOR			= (DESCRIPTOR_RES_ID | 8),
 USB_ENDPOINT_DESCRIPTOR 				= (DESCRIPTOR_RES_ID | 9)
} USB_DESCRIPTORS;


// Support for FUSES feature
#define FUSESID             0x46555345      // "FUSE"

// FUSE SubPackages IDs
#define FUS1ID		0x66757301      // "fus"0x1     SocConfig subpackage
#define FUS2ID      0x66757302      // "fus"0x2     AP/CP Config subpackage
#define FUS3ID      0x66757303      // "fus"0x3     MpConfig subpackage
#define FUS4ID      0x66757304      // "fus"0x4     RKEK subpackage
#define FUS5ID		0x66757305      // "fus"0x5     OEM Platform Key subpackage
#define FUS6ID      0x66757306      // "fus"0x6     JTAG subpackage
#define FUS7ID      0x66757307      // "fus"0x7     UID subpackage
#define FUS8ID      0x66757308      // "fus"0x8     USBID subpackage
#define FUS9ID		0x66757309      // "fus"0x9     Security Config subpackage
#define FUSAID      0x6675730A      // "fus"0xA     LCS State subpackage
#define FUSBID      0x6675730B      // "fus"0xB     Lock Block subpackage
#define FUSCID      0x6675730C      // "fus"0xC     Pin subpackage
#define FUSDID      0x6675730D      // "fus"0xD     SW Version subpackage
#define FUSEID		0x6675730E      // "fus"0xE     Marvell Config subpackage
#define FUSFID		0x6675730F      // "fus"0xF     DRM Transport Key subpackage
#define FUS10ID		0x66757310      // "fus"0x10    Boolean Fuse subpackage
#define FUS11ID     0x66757311      // "fus"0x11    Block subpackage


typedef enum    // use with COREID	0x434F5245 // "CORE"
{
      COREID_MP1 = 0,
      COREID_MP2 = 1,
      COREID_MM  = 2,
	  COREID_MP1_MM = 3,        //***
	  COREID_MP2_MM = 4,        //***
	  COREID_MP3 = 5,
	  COREID_MP4 = 6,
	  COREID_NONE = 0x4E4F4E45	// "NONE"
}
COREID_T;

typedef enum    // use with COREID_v2  0x43525632 // "CRV2"
{
      CORE_AP0 = 0,
      CORE_AP1 = 1,
      CORE_AP2 = 2,
      CORE_AP3 = 3,
      CORE_AP4 = 4,
      CORE_AP5 = 5,
      CORE_AP6 = 6,
      CORE_AP7 = 7,
      CORE_NONE = 0x4E4F4E45                 // "NONE"
}
COREID_V2_T;


// Global Identifiers
#define FFIDENTIFIER		0x00004646		// "FF"
#define ALTIDENTIFIER		0x00414C54		// "ALT"
#define PINSIDENTIFIER		0x50696E73		// "Pins"
#define CI2IDENTIFIER       0x00434932     // "CI2"
#define OTGIDENTIFIER		0x554F5447		// "UOTG"
#define INVALIDID           0x21212121      // "!!!!" Should never appear in actual TIM file


// Miscellanous
#define BINDKEYSIZE			129				// (MAXKEYSIZEWORDS * 2)
#define FLASHSIGMASK		0x000000FF		// mask off the 8 bits of platform state (only 5 currently used)

/********** WTP Recognized Reserved Area Layout ********************************
*
*	WTPTP_Defined_Reserved_Format_ID    \	  This clues BR, OBM and DKB that the reserved area is in a known format
*	Number of Reserved Area Packages    /	  For each package there is a header, payload size and payload
*
*	Header	 	 		\	  Indicates what type of a Reserved Area Package
*	Size		  		 \	  Size Comprises a single Reserved Area Package
*	Payload		  		 /	  There may be any number of Packages so long as TIM/NTIM < 4KB
*			 			/
*
*	Header		 		\	  The Last one should be a Package with a Terminator Header
*	Size		 		/	  The size should be 8 bytes (the size of this package)
*
**********************************************************************************/
typedef struct
{
 UINT_T WTPTP_Reserved_Area_ID;	  	// This clues BR, OBM and DKB that the reserved area is in a known format
 UINT_T	NumReservedPackages;	    // For each package there is a header, payload size and payload
}WTP_RESERVED_AREA, *pWTP_RESERVED_AREA;

typedef struct
{
 UINT_T Identifier;					// Identification of this reserved area entry
 UINT_T Size;						// Size  = Payload Size + 2 words (8 bytes).
}WTP_RESERVED_AREA_HEADER, *pWTP_RESERVED_AREA_HEADER;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T 	bmRequestType;
 UINT_T 	bRequest;
 UINT_T		wValue;
 UINT_T		wIndex;
 UINT_T		wLength;
 UINT_T		wData; 							// First word of the proceeding Data. Note, there may be more traialing data
} USB_VENDOR_REQ, *pUSB_VENDOR_REQ;			// There is no restriction that data need be 32 bit aligned.

typedef struct
{
 volatile int *Addr;
 UINT_T Value;
}GPIO_DEF, *pGPIO_DEF;

typedef struct
{
 UINT_T GPIO_Num;
 UINT_T Value;
}GPIO_PIN_DEF, *pGPIO_PIN_DEF;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T XferLoc;						//location to put the XFER table
 UINT_T NumPairs;
 XFER_DATA_PAIR_T pXferPairs[1];
}OPT_XFER_SET, *pOPT_XFER_SET;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T Version;					// 0 = not supported, 1 = MBBT v1, 2 = MBBT v2
}BBM_VERSION_REQ, *pBBM_VERSION_REQ;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T Port;
 UINT_T Enabled; 					// 1 - Enabled
}OPT_PROTOCOL_SET, *pOPT_PROTCOL_SET;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T EscSeqTimeOutMS;  				// Value is interpreted in milli secs.
// GPIO_DEF EscSeqGpioPackage;            // GPIO register and value
}OPT_ESCAPE_SEQUENCE, *P_OPT_ESCAPE_SEQUENCE;


typedef struct
{
    UINT_T MFPR_address;         //MFPR address that corresponds to this GPIO
    UINT_T MFPR_value;           //value to set the MFPR
}MFPR_PAIR_T, *pMFPR_PAIR_T;

typedef struct
{
    WTP_RESERVED_AREA_HEADER WRAH;
    UINT_T GPIO_number;          //GPIO to check
    UINT_T GPIO_trigger_val;     //value of GPIO to trigger the escape sequence
    UINT_T num_MFPR_pairs;       //number of MFPR pairs contained in this package
    MFPR_PAIR_T mfpr_pairs[1];   //MFPR address/value pairs: array will be of size ‘num_MFPR_pairs”
} OPT_ESC_SEQ_v2, *P_OPT_ESC_SEQ_v2;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T NumGpios;
 pGPIO_DEF	GPIO;
}OPT_GPIO_SET, *pOPT_GPIO_SET;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T ResumePackageDDRAddress;				// Address of the Resume Package in DDR
}OPT_TIM_RESUME_INFO, *pOPT_TIM_RESUME_INFO;


/* Do we need this.
typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T ACCR_VALUE;
 UINT_T MDCNFG_VALUE;
 UINT_T DDR_HCAL_VALUE;
 UINT_T MDREFR_VALUE;
}OPT_DDR_SET, *pOPT_DDR_SET;
*/

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;
 UINT_T CoreId;		    // See COREID_T
 UINT_T AddressMapping;
}OPT_COREID, *pOPT_COREID;

// new V2 core release package
typedef struct
{
    COREID_V2_T CoreId;
    UINT_T VectorAddress;
}
CORE_RELEASE_PAIR_T, *pCORE_RELEASE_PAIR_T;;

typedef struct
{
    WTP_RESERVED_AREA_HEADER      WRAH;
    UINT_T                        num_cores_release_pairs;        // number of cores to release
    CORE_RELEASE_PAIR_T           core_pairs[1];                  // will be size of "num_cores"
}OPT_CORE_RELEASE, *pOPT_CORE_RELEASE;


//typedef struct
//{
// WTP_RESERVED_AREA_HEADER WRAH;
// UINT_T FuseIndex;		// Currently not used
// UINT_T StoreAddress;
//}OPT_SOCPROFILEID, *pOPT_SOCPROFILEID;

typedef struct
{
    WTP_RESERVED_AREA_HEADER WRAH;
    unsigned int    PinA;   // low 32 bits of 64-bit PIN
    unsigned int 	PinB;   // high 32 bits of 64-bit PIN
} PIN, *pPIN;

typedef enum
{
   PRIMARY = 0,                // Primary image
   RECOVERY = 1                // Recovery image
} OTA_IMAGE_TYPE;

typedef struct
{
    UINT_T ImageID;                    // 0x54494Dxx   - secondary tim ID (TIMx)
    OTA_IMAGE_TYPE ImageType;          // OTA_IMAGE_TYPE (Primary VS Recovery)
    UINT_T FlashEntryAddr[2];          // Reserve 2 words for future compatibility
    UINT_T PartitionNumber;            // Partition # in the flash
} IMG_MAP_INFO, *pIMG_MAP_INFO;

typedef struct
{
 WTP_RESERVED_AREA_HEADER WRAH;     // Let it contain “IMAP” as package ID
    UINT_T NumberOfMappedImages;       // Number of images with mapping information
    IMG_MAP_INFO pImgMapInfo[1];       // First image map information struct
} IMAGE_MAP, *pIMAGE_MAP;


// DDR Configuration related structures
typedef struct
{
    unsigned int	ConsumerID;
    unsigned int    NumPackagesToConsume;
    unsigned int	PackageIdentifier;	// first in the list
} CIDP_ENTRY, *pCIDP_ENTRY;

typedef struct
{
    WTP_RESERVED_AREA_HEADER WRAH;
    unsigned int	NumConsumers;
    CIDP_ENTRY		Consumers;		// first in the list
} CIDP_PACKAGE, *pCIDP_PACKAGE;

typedef struct
{
    unsigned int 	OpId;
    unsigned int	OpValue;
} DDR_OPERATION, *pDDR_OPERATION;

 typedef enum DDR_OPERATION_SPEC_E
{
    DDR_NOP = 0,                // Enums specify the following:
    DDR_INIT_ENABLE = 1,        // DDR init flag (1 = init)
    DDR_MEMTEST_ENABLE = 2,     // DDR Memtest flag (1 = test)
    DDR_MEMTEST_START_ADDR = 3, // Memtest start addr (Current default is 0)
    DDR_MEMTEST_SIZE = 4,       // NumBytes to test (Current default is 2K)
    DDR_INIT_LOOP_COUNT = 5,    // Loop Count for initialization attempts
    DDR_IGNORE_INST_TO = 6,		// Treat a time out waiting for a bit to set/clear
                                //   	as informational only, operation continues
    // New values go above here
    DDR_OPERATION_SPEC_E_MAX
} DDR_OPERATION_SPEC_T;

typedef enum
{
	INSTR_ILLEGAL_OP = 0,
	INSTR_TEST_FOR_EQ = 1,
	INSTR_TEST_FOR_NE = 2,
	INSTR_TEST_FOR_LT = 3,
	INSTR_TEST_FOR_LTE = 4,
	INSTR_TEST_FOR_GT = 5,
	INSTR_TEST_FOR_GTE = 6,
}
INSTRUCTION_TEST_OPERATORS_T;


//
// Generic structures for handling instructions
//	Number of parameters are inherent in the command type according to:
//	Function					Descriptor			InstructionID	P1		P2				P3
//								   Text
//	 Write Register				WRITE				0x1				Addr	 Value			n/a
//   Read Register				READ				0x2				Addr	Num of reads	n/a
//   Delay Specified Time		DELAY				0x3				Value	n/a 			n/a
//   WaitForOperationToSetBit	WAIT_FOR_BIT_SET	0x4				Addr	Mask			TimeOut Value
//   WaitForOperationToClearBit	WAIT_FOR_BIT_CLEAR	0x5				Addr	Mask			TimeOut Value
//   AND value with Register	AND_VAL				0x6				Addr	Value			n/a
//   OR value with Register		OR_VAL				0x7				Addr 	Value			n/a
//

typedef enum INSTRUCTION_OP_CODE_SPEC_E
{
    // Enums specify the following:
    INSTR_NOP = 0,
	// Register instructions
    INSTR_WRITE = 1,
    INSTR_READ  = 2 ,
    INSTR_DELAY = 3,
    INSTR_WAIT_FOR_BIT_SET = 4,
    INSTR_WAIT_FOR_BIT_CLEAR = 5,
    INSTR_AND_VAL = 6,
    INSTR_OR_VAL = 7,
    // new DDR Script instructions
    INSTR_SET_BITFIELD = 8,
    INSTR_WAIT_FOR_BIT_PATTERN = 9,
    INSTR_TEST_IF_ZERO_AND_SET = 10,
    INSTR_TEST_IF_NOT_ZERO_AND_SET = 11,
	// Scratch Memory (SM) instructions
    INSTR_LOAD_SM_ADDR = 12,
    INSTR_LOAD_SM_VAL = 13,
    INSTR_STORE_SM_ADDR = 14,
    INSTR_MOV_SM_SM = 15,
    INSTR_RSHIFT_SM_VAL = 16,
    INSTR_LSHIFT_SM_VAL = 17,
    INSTR_AND_SM_VAL = 18,
    INSTR_OR_SM_VAL = 19,
    INSTR_OR_SM_SM = 20,
    INSTR_AND_SM_SM = 21,
    INSTR_TEST_SM_IF_ZERO_AND_SET = 22,
    INSTR_TEST_SM_IF_NOT_ZERO_AND_SET = 23,
    // conditional execution branch and subroutine call instructions
    INSTR_LABEL = 24,
    INSTR_TEST_ADDR_AND_BRANCH = 25,
    INSTR_TEST_SM_AND_BRANCH = 26,
    INSTR_BRANCH = 27,
	// Interpreter Directives
    INSTR_END = 28,
	//Add/Subtract
    INSTR_ADD_SM_VAL = 29,
    INSTR_ADD_SM_SM = 30,
    INSTR_SUB_SM_VAL = 31,
    INSTR_SUB_SM_SM = 32,
    // Load/Store Indirect
    INSTR_LOAD_SM_FROM_ADDR_IN_SM = 33,
    INSTR_STORE_SM_TO_ADDR_IN_SM = 34,
	//
	//INSTR_DEBUG_STOP = 35,
	//INSTR_CLEAR_BITFIELD = 36,
	//INSTR_SVC_MEMCPY = 37,
	//INSTR_SVC_UART_PUTCHAR = 38,
	//INSTR_SVC_UART_INIT = 39,
    INSTRUCTION_OP_CODE_E_MAX,              // New values go above here

    // These enums identify pseudo op code that
    // are used by tools for syntax shortcuts.  The
    // bootrom does not recognize the pseudo op
    // code.  When TBB is writing the TIM.bin, the pseudo
    // op codes are converted to one or more instructions
    // using the above INSTRUCTION_OP_CODE_SPEC_T enums.

    // pseudo instruction op codes used only by tools
    // Enums specify the following:
    INSTRUCTION_PSEUDO_OP_CODE_E_MIN = 10000,

    INSTR_PP_TABLEHEADER = 10000,
    INSTR_PP_WRITE = 10001,

    // New values go above here
    INSTRUCTION_PSEUDO_OP_CODE_E_MAX

} INSTRUCTION_OP_CODE_SPEC_T;


typedef struct
{
    unsigned int	InstructionId;  // InstructionID determines # of params for defined instructions
    unsigned int	Parameters[1];		// first of the variable sized list of Parameters[ MAX_NUMBER_PARAMS = 3 ]
} INSTRUCTION_S, *pINSTRUCTION_S;

typedef struct
{
    WTP_RESERVED_AREA_HEADER WRAH;
    unsigned int		NumberOperations;
    unsigned int 		NumberInstructions;
    DDR_OPERATION		DDR_Operations;          // first in a list of DDR_Op[NumberOperations];
    INSTRUCTION_S		DDR_Instructions;		// first in a list of DDR_Instruction[NumberInstructions];
} DDR_PACKAGE, *pDDR_PACKAGE;
// End New DDR Configuration Related structures

// New Trustzone related structures
typedef struct
{
    unsigned int 	OpId;
    unsigned int	OpValue;
} TZ_OPERATION, *pTZ_OPERATION;

typedef enum TZ_OPERATION_SPEC_E
{
    TZ_NOP = 0,              	    // Enums specify the following:
    TZ_CONFIG_ENABLE = 1,           // TZ config enable flag (1 = configure)
	TZ_IGNORE_INST_TO = 6,		  	// Treat a time out waiting for a bit to set/clear
								    // as informational only, operation continues
    // New values go above here
    TZ_OPERATION_SPEC_E_MAX
} TZ_OPERATION_SPEC_T;

typedef struct
{
    WTP_RESERVED_AREA_HEADER WRAH;
    unsigned int		NumberOperations;
    unsigned int 		NumberInstructions;
    TZ_OPERATION		TZ_Operations;       // first in a list of TZ_Op[NumberOperations];
    INSTRUCTION_S	 	TZ_Instructions;	 // first in a list of TZ_Instruction[NumberInstructions];
} TZ_PACKAGE, *pTZ_PACKAGE;
// End new Trustzone related structures

// General Purpose Patch (GPPx) related structures
typedef struct
{
	unsigned int 	OpId;
	unsigned int	OpValue;
} GPP_OPERATION, *pGPP_OPERATION;

typedef enum GPP_OPERATION_SPEC_E
{
    GPP_NOP = 0,              	// Enums specify the following:
	GPP_IGNORE_INST_TO = 6,	 	// Treat a time out waiting for a bit to set/clear
								// as informational only, operation continues
    // New values go above here
    GPP_OPERATION_SPEC_E_MAX
} GPP_OPERATION_SPEC_T;

typedef struct
{
	WTP_RESERVED_AREA_HEADER WRAH;
    unsigned int		NumberOperations;
	unsigned int 		NumberInstructions;
	GPP_OPERATION		GPP_Operations;     // GPP_Op[NumberOperations];
	INSTRUCTION_S	 	GPP_Instructions;	// GPP_Instruction[NumberInstructions];
} GPP_PACKAGE, *pGPP_PACKAGE;
// End new Trustzone related structures


// On-demand Boot ("ODBT") Package
typedef struct
{
      WTP_RESERVED_AREA_HEADER WRAH;
      UINT_T MagicWord;                 // Allow user to specify the magic word in TIM package
      UINT_T MagicWordAddress;          // Address to look for the Magic word
      UINT_T MagicWordPartition;        // Partition of the Magic Word
      UINT_T NumImages;
      UINT_T ODBTImageList[1];          // First image ID in new list
} ODBT_PACKAGE, *pODBT_PACKAGE;



//FUSE SubPackage
typedef struct
{
	UINT_T SubPackageId;    // enum value
	UINT_T SubPackageSize;  // In bytes
      UINT_T DataQualifier;   // Specifies the data source (and amount of data)
	UINT_T Options;		// Burn/Debug/Error handling options
	UINT_T Data[1];		// First 32 bit Data word (LSW)
}FUSE_SUBPACKAGE_S, *pFUSE_SUBPACKAGE_S;

//FUSE Package
typedef struct
{
	WTP_RESERVED_AREA_HEADER WRAH;
  	UINT_T GlobalFuseBurnDisable;
  	unsigned int 	NumberSubPackages;
	FUSE_SUBPACKAGE_S	FuseSubPackage;

}FUSE_PACKAGE_S, *pFUSE_PACKAGE_S;


/////////////////////////////////////////////////////
// Structs to Support DEBU MV Secure Boot

// Reserved Data Package IDs for MV Secure Boot Reserved Data Packages
//
//
#define MV_IMAGE_PKGS_TAG    0x4D564953      // MVIP Tag for the MV_IMAGES_PKG package containing the list of MV_IMAGE_HEADER_PKG packages.

// Each MV_IMAGE_HEADER_PKG is tied to an IMAGE_INFO_3_5 in the TIM header by the ImageID fields.
// If ImageID tags are generated then use the MV_IMAGE_HEADER_TAG with the last 2 ASCII numbers for
// the Image Description ImageID and the MV_IMAGE_HEADER_PKG ImageID
#define MV_IMAGE_HEADER_TAG    0x4D563030   // MV00 Note: 00 is a unique image number for each image
                                            // Any Tag can be used so long as it is consistent with the
                                            // related ImageInfo in the TIM Header.

#define MV_CUST_KEY_PKG_TAG    0x4D434B41   // MCKA
#define MV_EXT_RSA_KEY_PKG_TAG 0x4D45524B   // MERK


typedef struct {
 UINT32_T HeaderVersion;	// version information
 UINT16_T MagicNumber;		// magic number showing this is an image
 UINT8_T  CustKeyType;		// 0: no encryption, 1: marvell own cust key
							// 2: generic custkey
 UINT8_T  CodeType;			// 0:BCM EROM, 1: ARM Sys_Init,
							// 2:Marvell A/V firmware, 3. ARM TZ
							// 4:ARM Boot loader, Linux Kernel, RootFS,
							// 5:OEM firmware,6 ARM App
 UINT8_T  SecurityLevel;	// for BCM EROM to interpret
 UINT8_T  Reserved0;
 UINT16_T Reserved1;
 UINT8_T  WrappedUserKey[16];// custkey encrypted user key
 UINT32_T MarketId;
 UINT32_T MarketIdMask;
 UINT8_T  Version;
 UINT8_T  VersionMask;
 UINT16_T Reserved2;
 UINT32_T ImageSize;		// image size in bytes
 UINT8_T  ImageHash[256];	// SHA256 hash of the image
 UINT8_T  Signature[256];	// Signature of the header
} MV_IMAGE_HEADER;


typedef struct {
//    WTP_RESERVED_AREA_HEADER WRAH;                    // APARNA - Use either WRAH or just ImageID, whichever is easier for you
                                                        // ImageID is sufficient because the package size is fixed at (sizeof(MV_IMAGE_HEADER_PKG))
    UINT_T    ImageID;      // MV_IMAGE_HEADER_TAG
    // fields needed by DEBU MV Secure Boot feature
    MV_IMAGE_HEADER   MvImageHeader;
} MV_IMAGE_HEADER_PKG, *pMV_IMAGE_HEADER_PKG;


typedef struct {
    WTP_RESERVED_AREA_HEADER WRAH;
    UINT_T NumOfMVImages;
    MV_IMAGE_HEADER_PKG     MvImageHeaderPkgs[1];
} MV_IMAGES_PKG, *pMV_IMAGES_PKG;


typedef struct {
 UINT32_T HeaderVersion;		// version information
 UINT16_T MagicNumber;			// magic number showing this is an image
 UINT8_T Type;					// 0:marvell cust key 1: generic custkey
 UINT8_T Reserved0;				// 0:MDK 1: MDK2
 UINT8_T RootAesKey;			// bit[3:0]: 0:ROMMDK, 1:MDK, 2:RKEK, 3:ROMKx,
								// 4-7: inavlid
								// bit[7:4]: index for ROMKx
 UINT8_T SubKeyIndex;			// index of subkey
 UINT8_T SubKeyParam;			// parameters to generate the subkey
 UINT8_T WrappedCustKey[16];	// encrypted customer key
 UINT8_T Signature[32];			// AES CMAC signature
} MV_CUST_KEY;

typedef struct {
    WTP_RESERVED_AREA_HEADER WRAH;
    MV_CUST_KEY MV_CustKey;
} MV_CUST_KEY_PKG, pMV_CUST_KEY_PKG;

typedef struct {
 UINT32_T HeaderVersion;		// version information
 UINT16_T MagicNumber;			// magic number
 UINT8_T CustKeyType;			// 0: marvell custkey 1:generic cust key
 UINT8_T Reserved0;				// default 0
 UINT16_T SigningRights;		// bit 0:BCM, 1: Sys_Init
								// bit 2:Marvell A/V FW 3: ARM TZ
								// bit 4:ARM Boot loader,Linux Kernel etc
								// bit 5:OEM AV FW,
								// bit 6:ARM Apps, 7-15: reserved,
 UINT8_T RootRsaKey;			// bit[3:0]: value 0:ROMSIGNK,
								// 1: SIGNK pointed by pub_key_index,
								// 2: Marvell SIGNK,
								// value 3-15: invalid
								// bit[7:4] selects from ROMSIGNK0-7
								// value 8-15 is invalid
 UINT8_T PubExponent;			// 0:0x10001, other values are invalid
 UINT32_T MarketID;
 UINT32_T MarketIDMask;
 UINT8_T Version;
 UINT8_T VersionMask;
 UINT16_T Reserved1;			// default to be 0
 UINT8_T RsaModulus[256];		// public modulus of the rsa key
 UINT8_T RootKeyModulus[256];	// public modulus of the root
 UINT8_T RsaSignature[256];		// signature of the RSA key
} MV_EXT_RSA_KEY;

typedef struct {
    WTP_RESERVED_AREA_HEADER WRAH;
    MV_EXT_RSA_KEY MV_ExtRsaKey;
} MV_EXT_RSA_KEY_PKG, *pMV_EXT_RSA_KEY_PKG;

// End of Structs to Support DEBU MV Secure Boot
////////////////////////////////////////////////////////////////

/****  Used only for backwards compatability!!! ***/
typedef struct
{
 UINT_T Identifier;
 UINT_T PortType;
 UINT_T Port;
 UINT_T GPIOPresent;
 UINT_T NumGpios;
 pGPIO_DEF	GPIO;
}OPT_SET, *pOPT_SET;

typedef struct
{
	WTP_RESERVED_AREA_HEADER WRAH;
	UINT PageSize;
	UINT BlockSize;
	UINT NumBlocks;
	UINT Reserved1;
	UINT Reserved2;
	UINT Reserved3;
}FLASH_GEOMETRY_REQ, *pFLASH_GEOMETRY_REQ;



/*********************************************************************************/
typedef struct {
	WTP_RESERVED_AREA_HEADER WRAH;
	int DDRType;
}DDR_TYPE, *pDDR_TYPE;
typedef enum
{
    PlatformVerificationKey,
    NetworkOperatorCAKey,
    SoftwareUpdateCAKey,
    DRMDeviceCAKey,
    OEMReservedKey1,
    OEMReservedKey2,
    NUMKEYMODULES
}
KEYMODULES_T;

typedef enum
{
	Marvell_DS = 0x00000000,
	NO_ENCRYPTION = 0x00000000,
	PKCS1_v1_5_Caddo = 0x00000001,
//	PKCS1_v2_1_Caddo = 0x00000002,
	PKCS1_v1_5_Ippcp = 0x00000003,
//	PKCS1_v2_1_Ippcp = 0x00000004,
	ECDSA_256 = 0x00000005,
	ECDSA_521 = 0x00000006,
	PKCS_v2_2_Ippcp = 0x00000007,
    // from wtm_kernel.h
    // partial copy of values from
    // typedef enum _Cryptographic_Scheme
    //                                        //  CAT PKCS HASH  AES
    //                                        //   |    |    |    |
    ////AES                                        |    |    |    |
    AES_ECB128 = 0x00008000,                // 1000-0000-0000-0000
    AES_ECB192 = 0x00008002,                // 1000-0000-0000-0010
    AES_ECB256 = 0x00008001,                // 1000-0000-0000-0001
    AES_CBC128 = 0x00008004,                // 1000-0000-0000-0100
    AES_CBC192 = 0x00008006,                // 1000-0000-0000-0110
    AES_CBC256 = 0x00008005,                // 1000-0000-0000-0101
    AES_CTR128 = 0x00008008,                // 1000-0000-0000-1000
    AES_CTR192 = 0x0000800A,                // 1000-0000-0000-1010
    AES_CTR256 = 0x00008009,                // 1000-0000-0000-1001
    AES_XTS256 = 0x0000800C,                // 1000-0000-0000-1100
    AES_XTS512 = 0x0000800D,                // 1000-0000-0000-1101

    AES_CTS_ECB128 = 0x0000E000,            // 1110-0000-0000-0000
    AES_CTS_ECB192 = 0x0000E002,            // 1110-0000-0000-0010
    AES_CTS_ECB256 = 0x0000E001,            // 1110-0000-0000-0001
    AES_CTS_CBC128 = 0x0000E004,            // 1110-0000-0000-0100
    AES_CTS_CBC192 = 0x0000E006,            // 1110-0000-0000-0110
    AES_CTS_CBC256 = 0x0000E005,            // 1110-0000-0000-0101

    AES_TB_CTS_ECB128 = 0x0001E000,
    AES_TB_CTS_ECB192 = 0x0001E002,
    AES_TB_CTS_ECB256 = 0x0001E001,
    AES_TB_CTS_CBC128 = 0x0001E004,
    AES_TB_CTS_CBC192 = 0x0001E006,
    AES_TB_CTS_CBC256 = 0x0001E005,

    DUMMY_ENALG = 0xFFFFFFFF
}
ENCRYPTALGORITHMID_T;

typedef enum _Cryptographic_Scheme
{                                           //  CAT PKCS HASH  AES
											//   |    |    |    |
	// AES
	//AES_ECB128 = 0x00008000,                // 1000-0000-0000-0000
	//AES_ECB192 = 0x00008002,                // 1000-0000-0000-0010
	//AES_ECB256 = 0x00008001,                // 1000-0000-0000-0001
	//AES_CBC128 = 0x00008004,                // 1000-0000-0000-0100
	//AES_CBC192 = 0x00008006,                // 1000-0000-0000-0110
	//AES_CBC256 = 0x00008005,                // 1000-0000-0000-0101

	//AES_TB_CTS_ECB128 = 0x0001E000,
	//AES_TB_CTS_ECB192 = 0x0001E002,
	//AES_TB_CTS_ECB256 = 0x0001E001,
	//AES_TB_CTS_CBC128 = 0x0001E004,
	//AES_TB_CTS_CBC192 = 0x0001E006,
	//AES_TB_CTS_CBC256 = 0x0001E005,

	//PKCS#1 v1.5 Digital Signature
	PKCSv1_SHA1_1024RSA = 0x0000A100,     // 1010-0001-0000-0000
	PKCSv1_SHA256_1024RSA = 0x0000A110,     // 1010-0001-0001-0000
	PKCSv1_SHA1_2048RSA = 0x0000A200,     // 1010-0010-0000-0000
	PKCSv1_SHA256_2048RSA = 0x0000A210,     // 1010-0010-0001-0000

	//PKCS#1 v2.2 RSASSA-PSS Digital Signature
	PKCSv1_PSS_SHA1_1024RSA = 0x0000A300,
	PKCSv1_PSS_SHA256_1024RSA = 0x0000A310,
	PKCSv1_PSS_SHA1_2048RSA = 0x0000A400,
	PKCSv1_PSS_SHA256_2048RSA = 0x0000A410,


	//                                          ECC
	//                                          CAT FIELD HASH DH/DSA/MQV
	//ECCP                                       |    |    |    |
	ECCP256_FIPS_DSA_SHA1 = 0x0000B101,   // 1011-0001-0000-0001
	ECCP256_FIPS_DSA_SHA256 = 0x0000B111,   // 1011-0001-0001-0001
	ECCP256_FIPS_DSA_SHA512 = 0x0000B141,   // 1011-0001-0100-0001

	ECCP521_FIPS_DSA_SHA1 = 0x0000B301,   // 1011-0011-0000-0001
	ECCP521_FIPS_DSA_SHA256 = 0x0000B311,   // 1011-0011-0001-0001
	ECCP521_FIPS_DSA_SHA512 = 0x0000B341,   // 1011-0011-0100-0001

	CRYPTOGRAPHIC_SCHEME_LAST_ONE,
} CRYPTO_SCHEME_ENUM;

#define Intel_DS Marvell_DS

typedef enum
{
	SHA160 = 0x00000014,   //20
	SHA256 = 0x00000020,   //32
	SHA512 = 0x00000040,   //64
	DUMMY_HASH = 0x7FFFFFFF
}
HASHALGORITHMID_T;

typedef struct                           // pre-TIM 3.5
{
 UINT_T Version;
 UINT_T Identifier;                      // "TIMH"
 UINT_T Trusted;                         // 1- Trusted, 0 Non
 UINT_T IssueDate;
 UINT_T OEMUniqueID;
} VERSION_I, *pVERSION_I;                // 0x14 bytes

typedef struct                           // pre-TIM 3.5
{
	UINT_T Version;
	UINT_T Identifier;                      // "TIMH"
	UINT_T Trusted;                         // 1- Trusted, 0 Non
	UINT_T IssueDate;
	UINT_T OEMUniqueID;
	UINT_T Stepping;
} VERSION_I_3_6, *pVERSION_I_3_6;                // 0x14 bytes

typedef struct
{
 UINT_T ImageID;						// Indicate which Image
 UINT_T NextImageID;					// Indicate next image in the chain
 UINT_T FlashEntryAddr;					// Block numbers for NAND
 UINT_T LoadAddr;
 UINT_T ImageSize;
 UINT_T ImageSizeToHash;
 HASHALGORITHMID_T HashAlgorithmID;		// See HASHALGORITHMID_T
 UINT_T Hash[16];						// Reserve 512 bits for the hash
 UINT_T PartitionNumber;
 ENCRYPTALGORITHMID_T EncAlgorithmID;	// See ENCRYPTALGORITHMID_T
 UINT_T EncryptStartOffset;
 UINT_T EncryptSize;
} IMAGE_INFO_3_5_0, *pIMAGE_INFO_3_5_0;			// 0x60 bytes

typedef struct
{
 UINT_T ImageID;					// Indicate which Image
 UINT_T NextImageID;				// Indicate next image in the chain
 UINT_T FlashEntryAddr;			 	// Block numbers for NAND
 UINT_T LoadAddr;
 UINT_T ImageSize;
 UINT_T ImageSizeToHash;
 HASHALGORITHMID_T HashAlgorithmID;            // See HASHALGORITHMID_T
 UINT_T Hash[16];					// Reserve 512 bits for the hash
 UINT_T PartitionNumber;
} IMAGE_INFO_3_4_0, *pIMAGE_INFO_3_4_0;			// 0x60 bytes

typedef struct
{
 UINT_T ImageID;					// Indicate which Image
 UINT_T NextImageID;				// Indicate next image in the chain
 UINT_T FlashEntryAddr;			 	// Block numbers for NAND
 UINT_T LoadAddr;
 UINT_T ImageSize;
 UINT_T ImageSizeToHash;
 HASHALGORITHMID_T HashAlgorithmID;            // See HASHALGORITHMID_T
 UINT_T Hash[8];					// Reserve 256 bits for the hash
 UINT_T PartitionNumber;			// This is new for V3.2.0
} IMAGE_INFO_3_2_0, *pIMAGE_INFO_3_2_0;			// 0x40 bytes



typedef struct
{
 UINT_T WTMFlashSign;
 UINT_T WTMEntryAddr;
 UINT_T WTMEntryAddrBack;
 UINT_T WTMPatchSign;
 UINT_T WTMPatchAddr;
 UINT_T BootFlashSign;
} FLASH_I, *pFLASH_I;				// 0x10 bytes

typedef struct
{
	UINT_T Reserved1;
	UINT_T Reserved2;
	UINT_T Reserved3;
	UINT_T Reserved4;
	UINT_T BootFlashSign;
} FLASH_I_3_6, *pFLASH_I_3_6;                   // 0x14 bytes


typedef struct
{
 UINT_T	KeyID;						// Associate an ID with this key
 HASHALGORITHMID_T HashAlgorithmID;    // See HASHALGORITHMID_T
 UINT_T KeySize;					// Specified in bits
 UINT_T PublicKeySize;				// Specified in bits
 UINT_T RSAPublicExponent[MAXRSAKEYSIZEWORDS]; // Contents depend on PublicKeySize
 UINT_T RSAModulus[MAXRSAKEYSIZEWORDS]; // Up to 2K bits
 UINT_T KeyHash[8]; 				// Reserve 256 bits for the hash
} KEY_MOD_3_2_0, *pKEY_MOD_3_2_0;				// 0x22C bytes

#if RVCT
#pragma anon_unions
#endif
typedef struct
{
    UINT_T KeyID;				// Associate an ID with this key
    HASHALGORITHMID_T HashAlgorithmID;     // See HASHALGORITHMID_T
    UINT_T KeySize;			    // Specified in bits
    UINT_T PublicKeySize;		// Specified in bits
    ENCRYPTALGORITHMID_T EncryptAlgorithmID;	// See ENCRYPTALGORITHMID_T;
    union
    {
        struct
        {
            UINT_T RSAPublicExponent[MAXRSAKEYSIZEWORDS];	// Contents depend on PublicKeySize
            UINT_T RSAModulus[MAXRSAKEYSIZEWORDS];			// Up to 2K bits
        }Rsa;

        struct
        {
            UINT_T PublicKeyCompX[MAXECCKEYSIZEWORDS]; // Contents depend on PublicKeySize
            UINT_T PublicKeyCompY[MAXECCKEYSIZEWORDS]; // Up to 521 bits
            // Pad this struct so it remains consistent with RSA struct
            UINT_T Reserved[(2*MAXRSAKEYSIZEWORDS)-(2*MAXECCKEYSIZEWORDS)];
        }Ecdsa;
    };

    UINT_T KeyHash[8]; 				// Reserve 256 bits for the hash
} KEY_MOD_3_3_0, *pKEY_MOD_3_3_0;		//


// use EncryptedBitMask to access bit 31 of EncryptedAlgorithmID
// if bit 31 == 0 the access KEY_MOD_3_4_0 using the Rsa or Ecdsa structs
// if bit 31 == 1 the access KEY_MOD_3_4_0 using the EncryptedRsa or Encrypted Ecdsa structs
#define EncryptedBitMask 0x80000000
typedef struct
{
    UINT_T KeyID;                      // Associate an ID with this key
    HASHALGORITHMID_T HashAlgorithmID;    // See HASHALGORITHMID_T
    UINT_T KeySize;            // Specified in bits
    UINT_T PublicKeySize;      // Specified in bits
    ENCRYPTALGORITHMID_T EncryptAlgorithmID; // See ENCRYPTALGORITHMID_T;
    union
    {
        struct
        {
            UINT_T RSAPublicExponent[MAXRSAKEYSIZEWORDS];   // Contents depend on PublicKeySize
            UINT_T RSAModulus[MAXRSAKEYSIZEWORDS];          // Up to 2K bits
        }Rsa;

        struct
        {
            UINT_T PublicKeyCompX[MAXECCKEYSIZEWORDS]; // Contents depend on PublicKeySize
            UINT_T PublicKeyCompY[MAXECCKEYSIZEWORDS]; // Up to 521 bits
            // Pad this struct so it remains consistent with RSA struct
            UINT_T Reserved[(2*MAXRSAKEYSIZEWORDS)-(2*MAXECCKEYSIZEWORDS)];
        }Ecdsa;

        struct
        {
            UINT_T EncryptedHashRSAPublicExponent[MAXRSAKEYSIZEWORDS];  // Contents depend on PublicKeySize
            UINT_T EncryptedHashRSAModulus[MAXRSAKEYSIZEWORDS];         // Up to 2K bits
        }EncryptedRsa;

        struct
        {
            UINT_T EncryptedHashPublicKeyCompX_R[MAXECCKEYSIZEWORDS]; // Contents depend on PublicKeySize
            UINT_T EncryptedHashPublicKeyCompX_S[MAXECCKEYSIZEWORDS]; // Contents depend on PublicKeySize

            UINT_T EncryptedHashPublicKeyCompY_R[MAXECCKEYSIZEWORDS]; // Up to 521 bits
            UINT_T EncryptedHashPublicKeyCompY_S[MAXECCKEYSIZEWORDS]; // Up to 521 bits

            // Pad this struct so it remains consistent with encrypted RSA struct
            UINT_T Reserved[(2 * MAXRSAKEYSIZEWORDS)-(4 * MAXECCKEYSIZEWORDS)];
        }EncryptedEcdsa;
    };

    UINT_T KeyHash[16];                  // Reserve 512 bits for the hash
} KEY_MOD_3_4_0, *pKEY_MOD_3_4_0;        //



typedef struct
{
    ENCRYPTALGORITHMID_T DSAlgorithmID;   // See ENCRYPTALGORITHMID_T
    HASHALGORITHMID_T HashAlgorithmID; // See HASHALGORITHMID_T
    UINT_T KeySize;	        // Specified in bits
    UINT_T Hash[8];			  // Reserve 256 bits for optional key hash
    union 	// Note that this union should not be included as part of the hash for TIM in the Digital Signature
    {
        struct
        {
            UINT_T RSAPublicExponent[MAXRSAKEYSIZEWORDS];
            UINT_T RSAModulus[MAXRSAKEYSIZEWORDS];           	// Up to 2K bits
            UINT_T RSADigS[MAXRSAKEYSIZEWORDS];				// Contains TIM Hash
        }Rsa;

        struct
        {
            UINT_T ECDSAPublicKeyCompX[MAXECCKEYSIZEWORDS]; // Allow for 544 bits (17 words, 68 bytes for use with EC-521)
            UINT_T ECDSAPublicKeyCompY[MAXECCKEYSIZEWORDS];
            UINT_T ECDSADigS_R[MAXECCKEYSIZEWORDS];
            UINT_T ECDSADigS_S[MAXECCKEYSIZEWORDS];
            // Pad this struct so it remains consistent with RSA struct
            UINT_T Reserved[(MAXRSAKEYSIZEWORDS*3)-(MAXECCKEYSIZEWORDS*4)];
        } Ecdsa;
    };
} PLAT_DS, *pPLAT_DS;

// Constant part of the TIMs
typedef struct                           // pre-TIM 3.6
{
	union
	{
		struct
		{
			VERSION_I      VersionBind;             // 0
			FLASH_I        FlashInfo;               // 0x14
		}VersionFlash3_5;

		struct
		{
			VERSION_I_3_6	VersionBind;
			FLASH_I_3_6		FlashInfo;
		}VersionFlash3_6;
	};
 UINT_T         NumImages;               // 0x24
 UINT_T         NumKeys;                 // 0x28
 UINT_T         SizeOfReserved;          // 0x2C
} CTIM, *pCTIM;                          // 0x30

// TIM structure for use by DKB/OBM/BootROM
typedef struct
{
 pCTIM   		    pConsTIM;			// Constant part
 pIMAGE_INFO_3_5_0	pImg;				// Pointer to Images
 pKEY_MOD_3_4_0		pKey;				// Pointer to Keys
 PUINT				pReserved;			// Pointer to Reserved Area
 pPLAT_DS       	pTBTIM_DS;			// Pointer to Digital Signature
} TIM, *pTIM;

// NTIM structure for use by DKB/OBM/BootROM
typedef struct
{
 pCTIM        		pConsTIM;			// Constant part
 pIMAGE_INFO_3_5_0	pImg;				// Pointer to Images
 PUINT				pReserved;			// Pointer to Reserved Area
} NTIM, *pNTIM;


// include the obsolete definitions needed for tools to be backward compatible
#include "Tim2.h"

#if 0
void SetTIMPointers( UINT8_T * StartAddr, TIM *pTIM_h);
UINT_T LoadTim(UINT_T TIMArea, TIM *pTIM_h, P_MEDIA_T pMedia);
#if !BOOTROM
	UINT_T LoadDTimOTA(TIM *pTIM_h, pIMG_MAP_INFO pImgMapInfo, UINT_T TIMID, P_MEDIA_T pMedia);
#endif
pIMAGE_INFO_3_5_0 ReturnPImgPtr(pTIM pTIM_h, UINT_T ImageNumber);
UINT_T ReturnImgPartitionNumber(pTIM pTIM_h, pIMAGE_INFO_3_5_0 pImg);
UINT_T GetTIMValidationStatus(void);
void SetTIMValidationStatus(UINT_T status);
pIMAGE_INFO_3_5_0 FindImageInTIM(pTIM, UINT_T);
pKEY_MOD_3_4_0 FindKeyInTIM(pTIM pTIM_h, UINT_T ImageKeyID);
pWTP_RESERVED_AREA_HEADER FindPackageInReserved (UINT_T * Retval, pTIM pTIM_h, UINT_T Identifier);
#if !BOOTROM
	pIMG_MAP_INFO FindImageMapInfoInIMAP (pIMAGE_MAP pImgMap, UINT_T ImageID, OTA_IMAGE_TYPE OTAImageType);
#endif
// These 2 functions allow for multiple instances of one "TYPE" of
// package to be retrieved from the reserved area of the TIM
//-----------------------------------------------------------------------------------------------------
pWTP_RESERVED_AREA_HEADER FindFirstPackageTypeInReserved (UINT_T * Retval, pTIM pTIM_h, UINT_T Identifier);
pWTP_RESERVED_AREA_HEADER FindNextPackageTypeInReserved (UINT_T * Retval);
pCIDP_ENTRY FindMyConsumerArray(pTIM pTIM_h, UINT_T CID);
UINT_T CheckReserved (pTIM pTIM_h);
UINT_T InitTIMPort( pFUSE_SET pFuses, pTIM pTIM_h);
void InitDefaultPort(pFUSE_SET pFuses);
// Returns a pointer to the static tim in tim.c
pTIM GetTimPointer(void);
#endif
#endif
