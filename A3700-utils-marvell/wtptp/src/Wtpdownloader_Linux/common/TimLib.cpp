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
#if TOOLS_GUI == 1
#include "StdAfx.h"
#include "MarvellBootUtility.h"
#include <functional>
#else
#if LINUX
#include <cstring>
#include <cstdlib>
#include <libgen.h>
#include <errno.h>
#include <unistd.h>
#else
#include <Shlwapi.h>
#include <direct.h>
#include <functional>
#endif
#endif

#include "TimLib.h"
#include "ImageDescription.h"
//#include "TimDescriptorLine.h"
//#include "TimDescriptor.h"

#include <algorithm>

#pragma warning ( disable : 4996 )

#if LINUX
#include <cstring>
#else
#if (SDFM == 1)
#include "SDFMCommandLineParser.h"
#endif
#if (TBBV4_DLL == 1)
#include "CommandLineParser.h"
    static char buf[1024]={0};
    #define printf(a, ...)  { sprintf_s( buf, 1024, a, __VA_ARGS__); CCommandLineParser::DisplayMsg( string(buf) ); }
#elif (TOOLS_GUI == 1)
    static char buf[1024]={0};
    #define printf(a, ...)  { sprintf_s( buf, 1024, a, __VA_ARGS__); DisplayMsg( string(buf) ); }
#else
    #define printf(a, ...)  { ::printf(a, __VA_ARGS__); fflush(stdout); }
#endif
#endif

CTimLib::CTimLib(void)
{
}

CTimLib::~CTimLib(void)
{
}

bool CTimLib::GetDWord (ifstream& ifs, const char *szString, unsigned int* pValue)
{
    size_t nPos = 0;
    string sValue;

    if ( pValue != 0 )
        *pValue = 0;

    while ( ifs.good() && GetNextLine( ifs, sValue ) )
    {
        // if 1st char (after leading white space) is a ;,
        //  then line is a comment so skip
        if ( sValue[0] != ';' )
            break;
    }

    if ( ifs.bad() || ifs.fail() )
        return false;

    if ( pValue == 0 )
    {
        if ( sValue != szString )
            return false;
    }
    else if ( nPos = sValue.find( szString ) == string::npos )
    {
        printf ("////////////////////////////////////////////////////////////\n");
        printf ("  Warning: Declaration statement is missing or corrupted!\n");
        printf ("  <%s>\n", szString);
        printf ("////////////////////////////////////////////////////////////\n");
        return false;
    }
    else
    {
        nPos += strlen( szString );
        sValue = sValue.substr( nPos );
        *pValue = Translate(sValue);
    }

    return true;
}

bool CTimLib::GetSValue (ifstream& ifs, const char *szString, string& sValue, bool bErrMsg )
{
    size_t nPos = 0;
    sValue = "";

    while ( ifs.good() && GetNextLine( ifs, sValue ) )
    {
        // if 1st char (after leading white space) is a ;,
        //  then line is a comment so skip
        if ( sValue[0] != ';' )
            break;
    }

    if ( ifs.bad() || ifs.fail() )
        return false;

    if ( nPos = sValue.find( szString ) == string::npos )
    {
        if ( bErrMsg )
        {
            printf ("////////////////////////////////////////////////////////////\n");
            printf ("  Warning: Declaration statement is missing or corrupted!\n");
            printf ("  <%s>\n", szString);
            printf ("////////////////////////////////////////////////////////////\n");
        }
        return false;
    }
    nPos += strlen( szString );

    // get the value following the szString
    sValue = sValue.substr( nPos );

    sValue = TrimWS( sValue );

    return true;
}

bool CTimLib::GetFieldValue (string& sLine, string& sField, DWORD& dwValue)
{
    size_t nPos = 0;
    string sValue = "";
    dwValue = 0;

    // if 1st char (after leading white space) is a ;,
    //  then line is a comment so skip
    if ( sLine[0] == ';' )
        return false;

    if ( sLine.find( sField ) == string::npos )
        return false;

    if ( string::npos == ( nPos = sLine.find_first_not_of( ": \n\t", sField.length() ) ) )
        return false;
    else
        sValue = sLine.substr(nPos);

    if ( sValue.length() > 0 )
    {
        dwValue = Translate (sValue);
        return true;
    }

    return false;
}

bool CTimLib::GetFieldValue (string& sLine, string& sField, string& sValue)
{
    size_t nPos = 0;
    sValue = "";

    // if 1st char (after leading white space) is a ;,
    //  then line is a comment so skip
    if ( sLine[0] == ';' )
        return false;

    if ( sLine.find( sField ) == string::npos )
        return false;

    if ( string::npos == ( nPos = sLine.find_first_not_of( ": \n\t", sField.length() ) ) )
        return false;
    else
        sValue = sLine.substr(nPos);

    sValue = TrimWS( sValue );

    if ( sValue.length() > 0 )
        return true;

    return false;
}

bool CTimLib::CheckGetSValue (ifstream& ifs, char *szString, string& sValue)
{
    // same functionality as GetSValue but with no error warning if match not found

    size_t nPos = 0;
    sValue = "";

    while ( ifs.good() && GetNextLine( ifs, sValue ) )
    {
        // if 1st char (after leading white space) is a ;,
        //  then line is a comment so skip
        if ( sValue[0] != ';' )
            break;
    }

    if ( ifs.bad() || ifs.fail() )
        return false;

    if ( nPos = sValue.find( szString ) == string::npos )
    {
//		printf ("////////////////////////////////////////////////////////////\n");
//		printf ("  Warning: Declaration statement is missing or corrupted!\n");
//		printf ("  <%s>\n", szString);
//		printf ("////////////////////////////////////////////////////////////\n");
        return false;
    }
    nPos += strlen( szString );

    // get the value following the szString
    sValue = sValue.substr( nPos );

    sValue = TrimWS( sValue );

    return true;
}

unsigned int CTimLib::Translate (const char* pValue)
{
    unsigned int iRetVal = 0;
#if LINUX
    if (pValue[0]== '0' && pValue[1]== 'x')
        sscanf( pValue, "%x", &iRetVal );
    else
        sscanf( pValue, "%i", &iRetVal );
#else
    int isscanRet = 0;
    isscanRet = sscanf_s( pValue, "%i", &iRetVal );
    if ( isscanRet == 0 || isscanRet == EOF )
        iRetVal = 0;
#endif
    return iRetVal;
}

unsigned int CTimLib::Translate (string& sValue)
{
    unsigned int iRetVal = 0;

    sValue = TrimWS( sValue );

#if LINUX
    if ( sValue.find("0x")!=string::npos || sValue.find("0X")!=string::npos)
    {
        sscanf( sValue.c_str(), "%x", &iRetVal );
    }
    else
        sscanf( sValue.c_str(), "%i", &iRetVal );
#else
    int isscanRet = 0;
    isscanRet = sscanf_s( sValue.c_str(), "%i", &iRetVal );
    if ( isscanRet == 0 || isscanRet == EOF )
        iRetVal = 0;
#endif
    return iRetVal;
}

void CTimLib::Endian_Convert (UINT_T x,UINT_T *y)
{
    UINT_T a0, a1, a2, a3;

    a0 = x & 0xFF;
    a1 = (x & 0xFF00) >> 8;
    a2 = (x & 0xFF0000) >> 16;
    a3 = (x & 0xFF000000) >> 24;
    *y = (a0 << 24) | (a1 << 16) | (a2 << 8) | a3;
}

UINT32_T CTimLib::CheckSum (unsigned char *Address, UINT_T length)
{
    UINT32_T Key = 0x04C11DB7;
    UINT32_T Val = 0xFFFFFFFF;
    UINT_T i = 0, t = 0;

    // This routine is used when building Non Trusted images where a
    // check sum is generated from an entire image or buffer of data.

    while (i < length)
    {
        if (((( Val >> 31 ) & 0x1 ) ^ ((Address[i] >> t) & 0x1)) == 1)
            Val = ( Val << 1) ^ Key;
        else
            Val = Val << 1;

        if (t == 7)
        {
            t = 0;
            i++;
        }
        else t++;
    }

    return Val;
}


bool CTimLib::CreateOutputImageName (string& sImageFilename, string& sImageOutFilename)
{
    string sTemp( sImageFilename );
    string::size_type slashpos = sImageFilename.find_last_of('\\');

    // deals with the issue of \ / in paths prior to searching the .ext or no ext
    string::size_type slashposl = sImageFilename.find_last_of('\\');
    string::size_type slashposr = sImageFilename.find_last_of('/');

    // got a '\' but no '/'
    if ( (slashposl != string::npos) && (slashposr == string::npos) )
        slashpos = slashposl;

    // got a '/' but no '\'
    if ( (slashposr != string::npos) && (slashposl == string::npos) )
        slashpos = slashposr;

    // got both, so pick the max pos
    if ( (slashposl != string::npos) && (slashposr != string::npos) )
        slashpos = max( slashposl, slashposr );

    string::size_type dotpos = sImageFilename.find_last_of('.');
    if ( dotpos == string::npos )
    {
        // no ext
//        sTemp += "_h.bin";
        sTemp += "_h.";
        sImageOutFilename = sTemp;
    }
    else
    {
        if ( (slashpos == string::npos) || (slashpos < dotpos) )
            sTemp.resize( dotpos );

        sTemp += "_h";

        if( (slashpos != string::npos) && (slashpos < dotpos) )
            sTemp += sImageFilename.substr( dotpos );
        else if ( (slashpos == string::npos) && ( dotpos != string::npos ) )
            sTemp += sImageFilename.substr( dotpos );

        sImageOutFilename = sTemp;
    }

    return true;
}

bool CTimLib::CreateOutputTimBinImageName (string& sImageFilename, string& sImageOutFilename)
{
    string sTemp( sImageFilename );
    string::size_type slashpos = sImageFilename.find_last_of('\\');

    // deals with the issue of \ / in paths prior to searching the .ext or no ext
    string::size_type slashposl = sImageFilename.find_last_of('\\');
    string::size_type slashposr = sImageFilename.find_last_of('/');

    // got a '\' but no '/'
    if ( (slashposl != string::npos) && (slashposr == string::npos) )
        slashpos = slashposl;

    // got a '/' but no '\'
    if ( (slashposr != string::npos) && (slashposl == string::npos) )
        slashpos = slashposr;

    // got both, so pick the max pos
    if ( (slashposl != string::npos) && (slashposr != string::npos) )
        slashpos = max( slashposl, slashposr );

    string::size_type dotpos = sImageFilename.find_last_of('.');
    if ( dotpos == string::npos )
    {

        // no ext
        sTemp += ".bin";
        sImageOutFilename = sTemp;
    }
    else
    {
        if ( (slashpos == string::npos) || (slashpos < dotpos) )
            sTemp.resize( dotpos );

        if( (slashpos != string::npos) && (slashpos < dotpos) )
            sTemp += sImageFilename.substr( dotpos );
        else if ( (slashpos == string::npos) && ( dotpos != string::npos ) )
            sTemp += sImageFilename.substr( dotpos );

        sImageOutFilename = sTemp;
    }

    return true;
}

bool CTimLib::CheckImageOverlap( t_ImagesList& Images )
{
    unsigned int i = 0, j = 0;
    //starting and ending addresses of the two images being compared
    unsigned int s1 = 0, s2 = 0, e1 = 0, e2 = 0;

    t_ImagesIter iter = Images.begin();
    while( iter != Images.end() )
    {
        t_ImagesIter innerIter = Images.begin();
        while( innerIter != Images.end() )
        {
            if ( innerIter != iter )
            {
                if ( (*iter)->PartitionNumber() == (*innerIter)->PartitionNumber() )
                {
                    s1 = Translate((*iter)->FlashEntryAddress());
                    s2 = Translate((*innerIter)->FlashEntryAddress());

                    e1 = s1 + (*iter)->ImageSize();
                    e2 = s2 + (*innerIter)->ImageSize();
                    if (s1 > s2)//if image 2 is at a lower location in flash
                    {
                        if(e2 > s1)//make sure the end of image 2 is also at a lower location in flash
                        {
                            printf("Error: Images <%s> and <%s> overlap in flash!\n", (*iter)->ImageIdTag().c_str(), (*innerIter)->ImageIdTag().c_str());
                            //printf("Bad: i1 %d i2 %d s1 %d s2 %d e1 %d e2 %d", i, j, s1, s2, e1, e2);
                            return false;
                        }
                    }
                    else		//if image 1 is at a lower location in flash
                    {
                        if(e1 > s2)//make sure the end of image 1 is also at a lower location in flash
                        {
                            printf("Error: Images <%s> and <%s> overlap in flash!\n", (*iter)->ImageIdTag().c_str(), (*innerIter)->ImageIdTag().c_str());
                            //printf("Bad: i1 %d i2 %d s1 %d s2 %d e1 %d e2 %d", i, j, s1, s2, e1, e2);
                            return false;
                        }
                    }
                }
            }
            innerIter++;
        }
        iter++;
    }
    return true;
}

bool CTimLib::GetQWord (ifstream& ifs, char *szString, UINT64 *pValue)
{
    size_t nPos = 0;
    string sValue;

    if ( pValue != 0 )
        *pValue = 0;

    while ( ifs.good() && GetNextLine( ifs, sValue ) )
    {
        // if 1st char (after leading white space) is a ;,
        //  then line is a comment so skip
        if ( sValue[0] != ';' )
            break;
    }

    if ( ifs.bad() || ifs.fail() )
        return false;

    if ( pValue == 0 )
    {
        if ( sValue != szString )
            return false;
    }
    else if ( nPos = sValue.find( szString ) == string::npos )
    {
        printf ("////////////////////////////////////////////////////////////\n");
        printf ("  Warning: Declaration statement is missing or corrupted!\n");
        printf ("  <%s>\n", szString);
        printf ("////////////////////////////////////////////////////////////\n");
        return false;
    }
    else
    {
        nPos += strlen( szString );
        sValue = sValue.substr( nPos );
        *pValue = TranslateQWord(sValue.c_str());
    }

    return true;
}

unsigned long long CTimLib::TranslateQWord (const char *pValue)
{
    // This function converts a string to a number.
    unsigned long long result = 0;

#if LINUX
    int i;
    char *end = NULL;

    // If pValue contains "0x" then treat it as a hexidecimal value
    // otherwise treat it as a regular integer.

    //printf("***************%s",pValue);
    if (strstr (pValue,"0x") == NULL)
    {
        return (unsigned long long) strtoul(pValue,&end,18);
    }
    else if (strstr (pValue,"0X") == NULL)
    {
        return (unsigned long long) strtoul(pValue,&end,18);
    }
    else
    {
        for (i = 2; i < 18 && pValue[i] != '\0'; i++)
        {
//			printf("result h: %X\n", *(unsigned int*)(((unsigned int)&result) + 4));
//			printf("result l: %X\n", *(unsigned int*)&result);
            result = (result << 4);

            if (pValue[i] >= 'a' && pValue[i] <= 'f')
                result += (unsigned long long)(pValue[i] - 'a' + 0xA);
            else if (pValue[i] >= 'A' && pValue[i] <= 'F')
                result += (unsigned long long)(pValue[i] - 'A' + 0xA);
            else if (pValue[i] >= '0' && pValue[i] <= '9')
                result += (unsigned long long)(pValue[i] - '0');
            else if (pValue[i] == ' ')
                return (result >> 4);
            else return 0;
        }
    }
    return result;
#else
	int isscanRet = 0;
	isscanRet = sscanf_s(pValue, "%I64i", &result);
	if (isscanRet == 0 || isscanRet == EOF)
		result = 0;

	return result;
#endif
}


void CTimLib::PrependPathIfNone( string& sFilePath )
{
    // prepend current directory if no path on file

#if LINUX
    char szAbsPath[ 256 ] = {0};
    realpath( sFilePath.c_str(), szAbsPath );
#else
    char szAbsPath[ _MAX_PATH ] = {0};

    // unquote file path before call to _fullpath
    size_t nPos = 0;
    while( ( nPos = sFilePath.find('"')) != string::npos )
    {
        sFilePath.replace( nPos, nPos+1, "" );
    }

    if (NULL == _fullpath(szAbsPath, sFilePath.c_str(), _MAX_PATH))
    {
        printf("\n Failed to create a valid path! \n")
    }

#endif

    if ( sFilePath.size() == 0 )
    {
        sFilePath = szAbsPath;
        sFilePath += "\\";
    }
    else
        sFilePath = szAbsPath;

//    sFilePath = QuotedFilePath( sFilePath );

    return;
}

void CTimLib::ToRelativePath( string& sFilePath, string& sRelativeTo )
{
#if LINUX
    char szAbsPath[256] = { 0 };
    char szCurWD[256] = { 0 };
    char szFilePath[256] = { 0 };

    // save the current working directory
   getcwd(szCurWD,256);
    // make the cur dir the relative to
    strcpy(szFilePath, sFilePath.c_str());
    chdir(sRelativeTo.c_str());
    // convert the file path to absolute using the relative to wd
    realpath(szFilePath, szAbsPath);
    sFilePath = szAbsPath;
    // put the wd back to where it was
    chdir(szCurWD);
#else
    char szRelPath[_MAX_PATH] = { 0 };
    if (PathRelativePathTo(szRelPath, sRelativeTo.c_str(), 0, sFilePath.c_str(), 0))
        sFilePath = szRelPath;
#endif

    return;
}


string& CTimLib::HexFormattedAscii( unsigned int iNum )
{
    static string sRet;
    sRet = "0x"; // instead of using showbase because we want 0x in lowercase
                 // but the hex alpha in uppercase

    // create a string for hex formatted ASCII string creation
    stringstream ss;
    ss << hex << setw( 8 ) << uppercase << setfill('0') << iNum;
    sRet += ss.str();
    return sRet;
}

string& CTimLib::TextToHexFormattedAscii( string& sText )
{
    static string sRet;
    // only converts the first 4 bytes of sText to a HexFormattedAscii string
    // turn first 4 ascii chars of Id into a hex encoded ascii tag
    string sIdReversed;
    size_t i = min((int)sText.length(),4);
    for ( ; i > 0; i-- )
        sIdReversed += sText[i-1];
    while( sIdReversed.length() < 4 ) sIdReversed += '\0'; // nulls reserve bytes for int* dereference

    sRet = HexFormattedAscii(*((int*)sIdReversed.c_str()));
    return sRet;
}

string& CTimLib::HexFormattedAscii64( UINT64 iNum )
{
    static string sRet;
    sRet = "0x"; // instead of using showbase because we want 0x in lowercase
                 // but the hex alpha in uppercase

    // create a string for hex formatted ASCII string creation
    stringstream ss;
    ss << hex << setw( 16 ) << uppercase << setfill('0') << iNum;
    sRet += ss.str();
    return sRet;
}


string CTimLib::QuotedFilePath( string& sFilePath )
{
    string sTemp(sFilePath);
    if ( sFilePath.length() > 0 )
    {
        // is there a space in path?
        if ( sFilePath.find(" ") != string::npos )
        {
            // is path already quoted?
            if ( sFilePath.find('\"') == string::npos )
            {
                sTemp = '\"' + sFilePath;
                sTemp += '\"';
            }
        }
    }
    return sTemp;
}

string CTimLib::HexAsciiToText( const string& sHexAscii )
{
    string sText = "*";

    if ( (sHexAscii.find("0x") == 0) || (sHexAscii.find("0X") == 0) )
    {
        char text[5]={0};
        unsigned int uHex = Translate( sHexAscii.c_str() );
        if ( uHex != 0 )
        {
            text[0] = ((char)((uHex & 0xFF000000)>>24));
            text[1] = ((char)((uHex & 0x00FF0000)>>16));
            text[2] = ((char)((uHex & 0x0000FF00)>>8));
            text[3] = ((char)((uHex & 0x000000FF)));
            sText = text;
        }

        // if result produced invalid characters, then reset to original
        if ( !IsAlphaNumeric( sText ) || sText.size() == 0 )
        {
            if ( IsAlphaNumeric( sHexAscii ) )
                sText = sHexAscii;
            else
                sText = "0x0";
        }
    }

    return sText;
}

string CTimLib::ToUpper( const string& instring, bool bHexNum )
{
    string upper;
    if ( instring.length() > 0 )
    {
        transform( instring.begin(), instring.end(), back_inserter( upper ), ptr_fun<int,int>( toupper ));

        // special handling for hex numbers
        if ( bHexNum )
        {
            if ( upper[0] == '0' && upper[1] == 'X' )
                upper[1] = 'x';
        }
    }
    return upper;
}

string CTimLib::ToLower( const string& instring, bool bHexNum )
{
    string lower;
    if ( instring.length() > 0 )
    {
        transform( instring.begin(), instring.end(), back_inserter( lower ), ptr_fun<int,int>( tolower ));

        // special handling for hex numbers
        if ( bHexNum )
        {
            if ( lower[0] == '0' && lower[1] == 'X' )
                lower[1] = 'x';
        }
    }
    return lower;
}

bool CTimLib::IsAlpha( const string& instring )
{
    string sin = instring;
    sin = TrimWS(sin);
    return (sin.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") == string::npos);
}

bool CTimLib::IsNumeric( const string& instring )
{
    string sin = instring;
    sin = TrimWS(sin);
    return (sin.find_first_not_of(" 0123456789") == string::npos);
}

bool CTimLib::IsHexNumeric( const string& instring )
{
    string sin = instring;
    sin = TrimWS(sin);
    // make sure a hex value has at least 1 of 0x or 0X, and the rest looks like a hex value
    if ( (sin.find("0x") != string::npos) || (sin.find("0X") != string::npos) )
        return (sin.find_first_not_of(" xX0123456789abcdefABCDEF") == string::npos );

    return false;
}

bool CTimLib::IsAlphaNumeric( const string& instring )
{
    string sin = instring;
    sin = TrimWS(sin);
    return (sin.find_first_not_of("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") == string::npos);
}

ifstream& CTimLib::GetNextLineNoWSSkipComments( ifstream& ifs, string& sLine )
{
    while ( ifs.good() && GetNextLine( ifs, sLine ) )
    {
        // if 1st char (after leading white space) is a ;,
        //  then line is a comment so skip
        if ( sLine[0] == ';' )
            continue;

        // parse out post comment
       	size_t nPos = 0;
    	if ( (nPos = sLine.find( ';' )) != string::npos )
	    {
		    // include ws between field and post comment
		    while( nPos-1 > 0 )
		    {
			    if ( !isspace(0x000000FF & sLine[nPos-1]) )
				    break;
			    nPos--;
		    }

    		sLine = sLine.substr(0, nPos);
	    }

        if ( sLine.length() == 0 )
            continue;
        else
            break;
    }
    return ifs;
}

ifstream& CTimLib::GetNextLine( ifstream& ifs, string& sLine, bool bIncludeWS )
{
    // makes sure we only get a new data
    while(ifs.good() && !ifs.eof() && !ifs.bad() )
    {
#if LINUX
        //TBD THIS SECTION IS TO KEEP LINUX COMPILER HAPPY
        //{{
        char szStr[1024] = {0};
        ifs.getline( szStr, 1023, '\n');
        sLine = szStr;
        //}}
        // THE ABOVE SECTION SHOULD BE REPLACED WITH THIS AFTER LINUX COMPILER IS FIXED
#else
        ::getline(ifs, sLine, '\n');
#endif
        // handle text files with \r\n line endings
        if ( !sLine.empty() && sLine[sLine.length()-1] == '\r' )
            sLine.erase(sLine.length()-1,1);

        if ( !bIncludeWS )
        {
            // remove leading whitespace
            while ( sLine.length() > 0 && isspace(0x000000FF & sLine[0]) )
                sLine.erase(0,1);

            // remove any trailing whitespace
            while ( sLine.length() > 0 && isspace(0x000000FF & sLine[sLine.length()-1]) )
                sLine.erase(sLine.length()-1,1);

            if ( sLine.length() > 0 )
                break;
        }
        else
            // include ws so return even if it's a blank line
            break;
    }
    return ifs;
}


#if TOOLS_GUI == 1
bool CTimLib::SaveFieldState( stringstream& ss, string& sField )
{
    // save the field to the project file
    ss << sField << endl;

    // save associated CTimDescriptorLine if any
    CTimDescriptorLine* pLine = 0;
    // update the object line reference
    if ( (pLine = CTimDescriptor::GetLineField( "", false, &sField )) != 0 )
    {
        ss << true << endl;
        return pLine->SaveState( ss );
    }
    else
        ss << false << endl;

    return true;
}

bool CTimLib::SaveFieldState( stringstream& ss, unsigned int& iField )
{
    // save the field to the project file
    ss << iField << endl;

    // save associated CTimDescriptorLine if any
    CTimDescriptorLine* pLine = 0;
    // update the object line reference
    if ( (pLine = CTimDescriptor::GetLineField( "", false, &iField )) != 0 )
    {
        ss << true << endl;
        return pLine->SaveState( ss );
    }
    else
        ss << false << endl;

    return true;
}

bool CTimLib::SaveFieldState( stringstream& ss, bool& bField )
{
    // save the field to the project file
    ss << bField << endl;

    // save associated CTimDescriptorLine if any
    CTimDescriptorLine* pLine = 0;
    // update the object line reference
    if ( (pLine = CTimDescriptor::GetLineField( "", false, &bField )) != 0 )
    {
        ss << true << endl;
        return pLine->SaveState( ss );
    }
    else
        ss << false << endl;

    return true;
}

bool CTimLib::SaveFieldState( stringstream& ss, pair<unsigned int, unsigned int>& rPair )
{
    // save the field to the project file
    ss << rPair.first << endl;
    ss << rPair.second << endl;

    // save associated CTimDescriptorLine if any
    CTimDescriptorLine* pLine = 0;
    // update the object line reference
    if ( (pLine = CTimDescriptor::GetLineField( "", false, &rPair )) != 0 )
    {
        ss << true << endl;
        return pLine->SaveState( ss );
    }
    else
        ss << false << endl;

    return true;
}

bool CTimLib::LoadFieldState( ifstream& ifs, string& sField )
{
    ::getline( ifs, sField );
    if ( theApp.ProjectVersion >= 0x03021400 )
    {
        string sbuf;
        ::getline( ifs, sbuf );
        if ( Translate(sbuf) == 1 )
        {
            CTimDescriptorLine* pLine = new CTimDescriptorLine;
            pLine->LoadState( ifs );
            CTimDescriptor::g_TimDescriptorLines.push_back( pLine );
            pLine->AddRef( &sField );
        }
    }

    return ifs.good() && !ifs.fail();
}

bool CTimLib::SaveFieldStatePtr( stringstream& ss, void* pObj )
{
    // save the field to the project file
    ss << pObj << endl;

    // save associated CTimDescriptorLine if any
    CTimDescriptorLine* pLine = 0;
    // update the object line reference
    if ( (pLine = CTimDescriptor::GetLineField( "", false, pObj )) != 0 )
    {
        ss << true << endl;
        return pLine->SaveState( ss );
    }
    else
        ss << false << endl;

    return true;
}

bool CTimLib::LoadFieldStatePtr( ifstream& ifs, void* pObj )
{
    string sbuf;

    if ( theApp.ProjectVersion >= 0x03021400 )
    {
        // get's the saved This pointer and ignore it
        ::getline( ifs, sbuf );

        ::getline( ifs, sbuf );
        if ( Translate(sbuf) == 1 )
        {
            CTimDescriptorLine* pLine = new CTimDescriptorLine;
            pLine->LoadState( ifs );
            CTimDescriptor::g_TimDescriptorLines.push_back( pLine );
            pLine->AddRef( pObj );
        }
    }
    else
        return false;

    return ifs.good() && !ifs.fail();
}

bool CTimLib::LoadFieldState( ifstream& ifs, unsigned int& iField )
{
    string sbuf;

    ::getline( ifs, sbuf );
    iField = Translate(sbuf);

    if ( theApp.ProjectVersion >= 0x03021400 )
    {
        ::getline( ifs, sbuf );
        if ( Translate(sbuf) == 1 )
        {
            CTimDescriptorLine* pLine = new CTimDescriptorLine;
            pLine->LoadState( ifs );
            CTimDescriptor::g_TimDescriptorLines.push_back( pLine );
            pLine->AddRef( &iField );
        }
    }

    return ifs.good() && !ifs.fail();
}


bool CTimLib::LoadFieldState( ifstream& ifs, bool& bField )
{
    string sbuf;

    ::getline( ifs, sbuf );
    bField = Translate(sbuf) == 1 ? true : false;

    if ( theApp.ProjectVersion >= 0x03021400 )
    {
        ::getline( ifs, sbuf );
        if ( Translate(sbuf) == 1 )
        {
            CTimDescriptorLine* pLine = new CTimDescriptorLine;
            pLine->LoadState( ifs );
            CTimDescriptor::g_TimDescriptorLines.push_back( pLine );
            pLine->AddRef( &bField );
        }
    }

    return ifs.good() && !ifs.fail();
}


bool CTimLib::LoadFieldState( ifstream& ifs, pair<unsigned int, unsigned int>& rPair )
{
    string sbuf;

    ::getline( ifs, sbuf );
    rPair.first = Translate(sbuf);

    ::getline( ifs, sbuf );
    rPair.second = Translate(sbuf);

    if ( theApp.ProjectVersion >= 0x03021400 )
    {
        ::getline( ifs, sbuf );
        if ( Translate(sbuf) == 1 )
        {
            CTimDescriptorLine* pLine = new CTimDescriptorLine;
            pLine->LoadState( ifs );
            CTimDescriptor::g_TimDescriptorLines.push_back( pLine );
            pLine->AddRef( &rPair );
        }
    }

    return ifs.good() && !ifs.fail();
}

#endif

string CTimLib::TrimInternalWS( string& sValue )
{
    string sTrimmedWs = sValue;
    // reduce internal white space to a single space
    unsigned int pos = 0;
    while ( (sTrimmedWs.length()) > 0 && (pos < sTrimmedWs.length()-1) )
    {
        if ( (isspace(0x000000FF & sTrimmedWs[pos]) && (isspace(0x000000FF & sTrimmedWs[pos+1]))) )
        {
            sTrimmedWs.erase(pos,1);
            continue;
        }
        pos++;
    }

    return sTrimmedWs;
}

string CTimLib::TrimWS( string& sValue )
{
    string sTLWS = TrimLeadingWS( sValue );
    return TrimTrailingWS( sTLWS );
}

string CTimLib::TrimLeadingWS( string& sValue )
{
    string sNoLeadingWs = sValue;
    // remove leading whitespace
    while ( sNoLeadingWs.length() > 0 && isspace(0x000000FF & sNoLeadingWs[0]) )
        sNoLeadingWs.erase(0,1);

    return sNoLeadingWs;
}


string CTimLib::TrimTrailingWS( string& sValue )
{
    string sNoTrailingWs = sValue;
    // remove any trailing whitespace
    while ( sNoTrailingWs.length() > 0 && isspace(0x000000FF & sNoTrailingWs[sNoTrailingWs.length()-1]) )
        sNoTrailingWs.erase(sNoTrailingWs.length()-1,1);

    return sNoTrailingWs;
}

string CTimLib::TrimQuotes( string& sValue )
{
    string sNoQuote = TrimWS(sValue);
    // remove leading whitespace
    if ( sNoQuote.length() > 0 && sNoQuote[0] == '\"' )
        sNoQuote.erase(0,1);

    if ( sNoQuote.length() > 0 && sNoQuote[sNoQuote.length()-1] == '\"' )
        sNoQuote.erase(sNoQuote.length()-1,1);

    return sNoQuote;
}

string CTimLib::SpaceToUnderscore( const string& sValue )
{
    string sUsValue = sValue;
    size_t nPos = string::npos;
    while( ( nPos = sUsValue.find(' ') ) != string::npos )
        sUsValue.replace( nPos, 1, 1, '_' );
    return sUsValue;
}

bool CTimLib::ToArrayFromList( t_stringList& List, unsigned int* pUintAry, unsigned int size )
{
    if ( size < List.size() )
        return false;

    unsigned int i = 0;
    t_stringListIter iter = List.begin();
    while( iter != List.end() && i < size )
    {
        pUintAry[i] = Translate( *(*iter) );
        iter++;
        i++;
    }

    return true;
}

bool CTimLib::ToListFromArray( unsigned int* pUintAry, t_stringList& List, unsigned int size )
{
    if ( size > List.size() )
        return false;

    unsigned int i = 0;
    t_stringListIter iter = List.begin();
    while( iter != List.end() && i < size )
    {
        *(*iter) = HexFormattedAscii( pUintAry[i] );
        iter++;
        i++;
    }

    return true;
}

bool CTimLib::ResizeList( t_stringList& List, unsigned int size, bool& bChanged )
{
    if ( (unsigned int)List.size() < size )
    {
        while ( (unsigned int)List.size() < size )
        {
            List.push_back( new string("0x00000000") );
            bChanged = true;
        }
    }
    else if ( (unsigned int)List.size() > size )
    {
        // shrink the lists
        while ( (unsigned int)List.size() > size )
        {
            delete List.back();
            List.pop_back();

            bChanged = true;
        }
    }

    return true;
}

unsigned int CTimLib::HexFormattedTodayDate()
{
    struct tm *timenow = 0;
    time_t now = 0;
    time( &now );
    timenow = localtime( &now );
    if ( timenow )
    {
        unsigned int year = timenow->tm_year+1900;
        unsigned int mon = timenow->tm_mon+1;
        unsigned int day = timenow->tm_mday;
        unsigned int hexday = day % 10;
        day /= 10;
        hexday |= (day % 10) << 4;
        unsigned int hexmon = mon % 10;
        mon /= 10;
        hexmon |= (mon % 10) << 4;
        unsigned int hexyear = year % 10;
        year /= 10;
        hexyear |= (year % 10) << 4;
        year /= 10;
        hexyear |= (year % 10) << 8;
        year /= 10;
        hexyear |= (year % 10) << 12;

        return (hexyear << 16 | hexmon << 8 | hexday);
    }
    else return 0;
}

bool CTimLib::SetPathAsCWD( string& sFilePath )
{
#if LINUX
        char path[256] = {0};
        char* dir;
        strcpy( path, sFilePath.c_str());
        dir = dirname( path );
        chdir( dir );
#else
        char path_buffer[_MAX_PATH]={0};
        char drive[_MAX_DRIVE]={0};
        char dir[_MAX_DIR]={0};
        errno_t err;

        strcpy_s( path_buffer, _MAX_PATH, sFilePath.c_str() );
        err = _splitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, 0, 0, 0, 0 );
        if (err != 0)
        {
            printf("Error splitting the path. Error code %d.\n", err);
            return false;
        }
        string sDir = drive;
        sDir += dir;
#if TOOLS_GUI == 1
        if( !SetCurrentDirectory(sDir.c_str()) )
        {
            printf("SetCurrentDirectory to <%s> failed. Error: %s\n", sDir.c_str(), FormatWindowsGetLastErrorToString(GetLastError()).c_str());
            return false;
        }
#else
        if (-1 == _chdir(sDir.c_str()) )
        {
            printf("_chdir to <%s> failed. Error: %s\n", sDir.c_str(), FormatWindowsGetLastErrorToString(GetLastError()).c_str());
        }
#endif
#endif

    return true;
}

#if TOOLS_GUI
bool CTimLib::GetFileTimes( FILETIMES& filetimes )
{
    HANDLE hFile = 0;
    BOOL err = 0;
    hFile = CreateFile( filetimes.sFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL );
    if ( hFile != 0 )
    {
        err = GetFileTime( hFile, &filetimes.creationTime,
                                &filetimes.LastAccessTime,
                                &filetimes.lastWriteTime );
        CloseHandle(hFile);
    }

    return (err == 0 ? false : true);
}
#endif

string CTimLib::MakePathAbsoluteOrRelativeToTIMTxtPath( string& sFilePath, string& sTIMTxtPath, bool bRelative )
{
    string sResultPath = sFilePath;

	if ( bRelative )
    {
        CTimLib TimLib;
        if ( sTIMTxtPath.length() > 0 )
        {
            // make path relative
            SetPathAsCWD( sTIMTxtPath );
            PrependPathIfNone(sResultPath);
            ToRelativePath(sResultPath, sTIMTxtPath);
        }
    }

	return sResultPath;
}


string CTimLib::FormatWindowsGetLastErrorToString(DWORD dwErrorCode)
{
    // Retrieve the system error message for the last-error code
    string sMsg;
#ifndef LINUX
    LPSTR lpMsgBuf = 0;

    if ( FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dwErrorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&lpMsgBuf,
            0, NULL) != 0 )
    {
        if (lpMsgBuf)
        {
            sMsg = lpMsgBuf;
            LocalFree(lpMsgBuf);
        }
    }
#else
    sMsg = strerror(errno);
#endif
    return sMsg;
}
