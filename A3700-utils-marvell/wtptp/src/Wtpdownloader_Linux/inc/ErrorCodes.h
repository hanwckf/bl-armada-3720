/**************************************************************************
**
** (C) Copyright December 2012 Marvell International Ltd.
**
** All Rights Reserved.
**
**
** THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE
** IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
** PURPOSE ARE EXPRESSLY DISCLAIMED.
**
**  FILENAME:	ErrorCodes.h
**
**  PURPOSE: 	Contains DKB error definitions
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

#ifndef ERRORCODES
#define ERRORCODES

#define MAXERRORCODES 54

typedef struct
{
	BYTE ErrorCode;
	char Description[50];
} ERRORCODETABLE;

// General error code definitions			0x0 - 0x1F
#define NoError            					0x0
#define NotFoundError      					0x1
#define GeneralError       					0x2
#define WriteError         					0x3
#define ReadError		   					0x4
#define NotSupportedError  					0x5
#define InvalidPlatformConfigError			0x6
#define PlatformBusy						0x7
#define PlatformReady						0x8
#define PlatformDisconnect					0xC

// Flash Related Errors 					0x20 - 0x3F
#define EraseError		 					0x20
#define ProgramError						0x21
#define InvalidBootTypeError				0x22
#define ProtectionRegProgramError			0x23
#define NoOTPFound							0x24

// DFC Related Errors						0x40 - 0x5F
#define DFCDoubleBitError    				0x40
#define DFCSingleBitError    				0x41
#define DFCCS0BadBlockDetected  			0x42
#define DFCCS1BadBlockDetected  			0x43
#define	DFCInitFailed						0x44

// Security Related Errors 					0x60 - 0x8F
#define InvalidOEMVerifyKeyError			0x60
#define InvalidOBMImageError				0x61
#define SecureBootFailureError				0x62
#define InvalidSecureBootMethodError		0x63
#define UnsupportedFlashError				0x64
#define InvalidCaddoFIFOEntryError      	0x65
#define InvalidCaddoKeyNumberError			0x66
#define InvalidCaddoKeyTypeError			0x67
#define RSADigitalSignatureDecryptError 	0x68
#define InvalidHashValueLengthError     	0x69
#define InvalidTIMImageError				0x6A
#define HashSizeMismatch					0x6B
#define InvalidKeyHashError					0x6C
#define TIMNotFound							0x6D
#define WTMStateError						0x6E
#define FuseRWError							0x6F
#define InvalidOTPHashError					0x70
#define CRCFailedError						0x71

// Download Protocols						0x90 - 0xAF
#define DownloadPortError					0x90
#define DownloadError						0x91
#define FlashNotErasedError					0x92
#define InvalidKeyLengthError				0x93
#define DownloadImageTooBigError			0x94
#define UsbPreambleError					0x95
#define UsbPreambleTimeOutError				0x96
#define UartReadWriteTimeOutError			0x97
#define UnknownImageError					0x98
#define MessageBufferFullError				0x99
#define NoEnumerationResponseTimeOutError 	0x9A
#define UnknownProtocolCmd					0x9B

//JTAG ReEnable Error Codes					0xB0 - 0xCF
#define JtagReEnableError					0xB0
#define JtagReEnableOEMPubKeyError			0xB1
#define JtagReEnableOEMSignedPassWdError	0xB2
#define JtagReEnableTimeOutError			0xB3
#define JtagReEnableOEMKeyLengthError   	0xB4

const ERRORCODETABLE ErrorCodes[MAXERRORCODES] =
{
	{NoError,"NoError"},
	{NotFoundError,"NotFoundError"},
	{GeneralError,"GeneralError"},
	{WriteError,"WriteError"},
	{ReadError,"ReadError"},
	{NotSupportedError,"NotSupportedError"},
	{InvalidPlatformConfigError,"InvalidPlatformConfigError"},
	{PlatformBusy,"PlatformBusy"},
	{PlatformReady,"PlatformReady"},
	{EraseError,"EraseError"},
	{ProgramError,"ProgramError"},
	{InvalidBootTypeError,"InvalidBootTypeError"},
	{ProtectionRegProgramError,"ProtectionRegProgramError"},
	{NoOTPFound,"NoOTPFound"},
	{DFCDoubleBitError,"DFCDoubleBitError"},
	{DFCSingleBitError,"DFCSingleBitError"},
	{DFCCS0BadBlockDetected,"DFCCS0BadBlockDetected"},
	{DFCCS1BadBlockDetected,"DFCCS1BadBlockDetected"},
	{DFCInitFailed,"DFCInitFailed"},
	{InvalidOEMVerifyKeyError,"InvalidOEMVerifyKeyError"},
	{InvalidOBMImageError,"InvalidOBMImageError"},
	{SecureBootFailureError,"SecureBootFailureError"},
	{InvalidSecureBootMethodError,"InvalidSecureBootMethodError"},
	{UnsupportedFlashError,"UnsupportedFlashError"},
	{InvalidCaddoFIFOEntryError,"InvalidCaddoFIFOEntryError"},
	{InvalidCaddoKeyNumberError,"InvalidCaddoKeyNumberError"},
	{InvalidCaddoKeyTypeError,"InvalidCaddoKeyTypeError"},
	{RSADigitalSignatureDecryptError,"RSADigitalSignatureDecryptError"},
	{InvalidHashValueLengthError,"InvalidHashValueLengthError"},
	{InvalidTIMImageError,"InvalidTIMImageError"},
	{HashSizeMismatch,"HashSizeMismatch"},
	{InvalidKeyHashError,"InvalidKeyHashError"},
	{TIMNotFound,"TIMNotFound"},
	{WTMStateError,"WTMStateError"},
	{FuseRWError,"FuseRWError"},
	{InvalidOTPHashError,"InvalidOTPHashError"},
	{CRCFailedError,"CRCFailedError"},
	{DownloadPortError,"DownloadPortError"},
	{DownloadError,"DownloadError"},
	{FlashNotErasedError,"FlashNotErasedError"},
	{InvalidKeyLengthError,"InvalidKeyLengthError"},
	{DownloadImageTooBigError,"DownloadImageTooBigError"},
	{UsbPreambleError,"UsbPreambleError"},
	{UsbPreambleTimeOutError,"UsbPreambleTimeOutError"},
	{UartReadWriteTimeOutError,"UartReadWriteTimeOutError"},
	{UnknownImageError,"UnknownImageError"},
	{MessageBufferFullError,"MessageBufferFullError"},
	{NoEnumerationResponseTimeOutError,"NoEnumerationResponseTimeOutError"},
	{UnknownProtocolCmd,"UnknownProtocolCmd"},
	{JtagReEnableError,"JtagReEnableError"},
	{JtagReEnableOEMPubKeyError,"JtagReEnableOEMPubKeyError"},
	{JtagReEnableOEMSignedPassWdError,"JtagReEnableOEMSignedPassWdError"},
	{JtagReEnableTimeOutError,"JtagReEnableTimeOutError"},
	{JtagReEnableOEMKeyLengthError,"JtagReEnableOEMKeyLengthError"}
};

#endif