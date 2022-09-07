/*++

Copyright (c) 1991-1999,  Microsoft Corporation  All rights reserved.

Module Name:

    c_gb18030.c

Abstract:

    This file contains functions to convert GB18030-2000 (code page 54936) into Unicode, and vice versa.
    The target module is c_g18030.dll.  This will be the external DLL used by WideCharToMultiByte()
    and MultiByteToWideChar() to perform the conversion for GB18030 codepage.

    External Routines in this file:
      DllEntry
      NlsDllCodePageTranslation

Notes:
    GB18030-2000 (aka GBK2K) is designed to be mostly compatible with GBK (codepage 936), 
    while supports the full range of Unicode code points (BMP + 16 supplementary planes).

    The structure for GB18030 is:
        * Single byte: 
            0x00 ~ 0x7f
        * Two-byte: 
            0x81 ~ 0xfe, 0x40 ~ 0x7e    (leading byte, trailing byte)
            0x81 ~ 0xfe, 0x80 ~ 0xfe    (leading byte, trailing byte)
        * Four-byte:
            0x81 ~ 0xfe, 0x30 ~ 0x39, 0x81 ~ 0xfe, 0x30 ~ 0x39.
            The surrogare pair will be encoded from 0x90, 0x30, 0x81, 0x30

    The BMP range is fully supported in GB18030 using 1-byte, 2-byte and 4-byte sequences.
    In valid 4-byte GB18030, there are two gaps that can not be mapped to Unicode characters.
        0x84, 0x31, 0xa5, 0x30 (just after the GB18030 bytes for U+FFFF(*)) ~ 0x8f, 0x39, 0xfe, 0x39 (just before the first GB18030 bytes for U+D800,U+DC00)
        0xe3, 0x32, 0x9a, 0x36 (just after the GB18030 bytes for U+DBFF U+DFFF(**)) ~ 0xfe, 0x39, 0xfe, 0x39
        

        Note1: U+FFFF = 0x84, 0x31, 0xa4, 0x39
        Note2: U+DBFF U+DFFF = 0xe3, 0x32, 0x9a, 0x35

    Tables used in c_g18030.dll:
        * From Unicode to bytes:
            * g_wUnicodeToGB:
                Used to convert Unicode character to 2-byte GBK, 2-byte GB18030, or 4-byte GB18030.
                The index is 0x0000 ~ 0xffff, for Unicode BMP range.
                When the valures are:
                
                    Value       Meaning
                    ======      =======
                    0xffff      2-byte GB18030, which is compatible with GBK.  Call WC2MB(936,...) to convert.
                    0xfffe ~ [0xfffe - (ARRAYSIZE(g_wUnicodeToGBTwoBytes))+1]
                                2-byte GB18030, which is NOT compatible with GBK.  (0xfffe - Value) will be indexed into
                                a second table g_wUnicodeToGBTwoBytes, which contains the two-byte GB18030 values.
                                E.g. if the value is 0xfffe, the index into g_wUnicodeToGBTwoBytes is 0, so the two-byte
                                GB18030 will be 0xa8, 0xbf (which are stored g_wUnicodeToGBTwoBytes[0],g_wUnicodeToGBTwoBytes[1])
                    0x0000 ~ 0x99fb
                                An offset value that can be used to convert to 4-byte GB18030
                                If the value is 0x000, the 4-byte GB18030 is 0x81, 0x30, 0x81, 0x30.
                                
        * From bytes to Unicode
            * Two-byte GB18030 to Unicode:
                * g_wGBLeadByteOffset
                    The index into this table is lead byte 0x80 ~ 0xff (converted to index 0x00 ~ 0x7f).  
                    If the value is 0x0000, it means that this lead byte is compatible with GBK.  
                    Otherwise, the value can be:
                    0x0100  This is used to indexed into g_wUnicodeFromGBTwoBytes[0x0000 ~ 0x00ff].  
                            The value of g_wUnicodeFromGBTwoBytesis the Unicode value for this lead byte with the next valid trailing byte.
                    0x0200  This is used to indexed into g_wUnicodeFromGBTwoBytes[0x0100 ~ 0x01ff].  
                    0x0300  This is used to indexed into g_wUnicodeFromGBTwoBytes[0x0200 ~ 0x02ff].  
                    0x0400  This is used to indexed into g_wUnicodeFromGBTwoBytes[0x0300 ~ 0x03ff].  
                    
                    E.g. g_wGBLeadByteOffset[0x07] = 0x0000. It means that GB18030 two-byte lead byte 0x87 is compatible with GBK.
                    g_wGBLeadByteOffset[0x28] = 0x0200.  It means that GB18030 two-byte lead byte 0xa8 (0x28+0x80 = 0xa8) is NOT compatible with GBK.
                    The Unicode value for 0xa8, <trail byte> will be stored in g_wUnicodeFromGBTwoBytes[0x0100+<trail byte>]
                    
            * Four-byte GB18030 to Unicode:
                * g_wGBFourBytesToUnicode
                    The table is used to convert 4-byte GB18030 into a Unicode.
                    
                    The index value is the offset of the 4-byte GB18030.

                    4-byte GB18030      Index value
                    ==============      ===========
                    81,30,81,30         0
                    81,30,81,31         1
                    81,30,81,32         2
                    ...                 ...

                    The value of g_wGBFourBytesToUnicode cotains the Unicode codepoint for the offset of the 
                    corresponding 4-byte GB18030.

                    E.g. g_wGBFourBytesToUnicode[0] = 0x0080.  This means that GB18030 0x81, 0x30, 0x81, 0x30 will be converted to Unicode U+0800.
    
Revision History:

    02-20-2001    YSLin    Created.
    
--*/




//
//  Include Files.
//

#include <share.h>
#include "c_gb18030.h"

//
//  Constant Declarations.
//


//
// Structure used in GetCPInfo().
//
CPINFO g_CPInfo = 
{
    //UINT    MaxCharSize;
    4,
    //BYTE    DefaultChar[MAX_DEFAULTCHAR];
    {0x3f, 0x00},
    //BYTE    LeadByte[MAX_LEADBYTES];
    // Since GBK2K can have up to 4 bytes, we don't return
    // 0x81-0xfe as lead bytes here.
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00},    
};

// This is the offset for the start of surrogate U+D800, U+DC00
#define SURROGATE_OFFSET        GET_FOUR_BYTES_OFFSET_FROM_BYTES(0x90, 0x30, 0x81, 0x30)
// This is the offset for the end of surrogate U+DBFF, U+DFFF
#define SURROGATE_MAX_OFFSET    GET_FOUR_BYTES_OFFSET_FROM_BYTES(0xe3, 0x32, 0x9a, 0x35)


//-------------------------------------------------------------------------//
//                            EXTERNAL ROUTINES                            //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  NlsDllCodePageTranslation
//
//  This routine is the main exported procedure for the functionality in
//  this DLL.  All calls to this DLL must go through this function.
//
//  02-20-2001    YSLin    Created.
////////////////////////////////////////////////////////////////////////////

STDAPI_(DWORD) NlsDllCodePageTranslation(
    DWORD CodePage,
    DWORD dwFlags,
    LPSTR lpMultiByteStr,
    int cchMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar,
    LPCPINFO lpCPInfo)
{

    //
    //  Error out if internally needed c_*.nls file is not installed.
    //
    if (!IsValidCodePage(CODEPAGE_GBK))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    switch (dwFlags)
    {
        case ( NLS_CP_CPINFO ) :
        {
            memcpy(lpCPInfo, &g_CPInfo, sizeof(CPINFO));
            return (TRUE);
        }
        case ( NLS_CP_MBTOWC ) :
        {
            return (BytesToUnicode((BYTE*)lpMultiByteStr, cchMultiByte, NULL, lpWideCharStr, cchWideChar));
        }
        case ( NLS_CP_WCTOMB ) :
        {
            return (UnicodeToBytes(lpWideCharStr, cchWideChar, lpMultiByteStr, cchMultiByte));
        }
    }

    //
    //  This shouldn't happen since this gets called by the NLS APIs.
    //
    SetLastError(ERROR_INVALID_PARAMETER);
    return (0);
}

//-------------------------------------------------------------------------//
//                            INTERNAL ROUTINES                            //
//-------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////
//
//  GetBytesToUnicodeCount
//
//  Return the Unicode character count needed to convert the specified
//  GB18030 multi-byte string.
//
//  Parameters:
//      lpMultiByteStr  The multi-byte string to be converted.
//      cchMultiByte    The byte size of the multi-byte string to be converted
//      bSupportEncoder If TRUE and we have a lead byte at the end of string,
//                      we will not convert that lead byte.  Otherwise,
//                      convert it to the default character.
//
//  02-21-2001    YSLin    Created.
////////////////////////////////////////////////////////////////////////////

DWORD GetBytesToUnicodeCount(BYTE* lpMultiByteStr, int cchMultiByte, BOOL bSupportEncoder)
{
    int i = 0;
    BYTE ch;
    DWORD cchWCCount = 0;
    WORD wOffset;
    BYTE offset1, offset2, offset3, offset4;
    DWORD dwFourBytesOffset;
    
    if (cchMultiByte == -1)
    {
        cchMultiByte = strlen((LPSTR)lpMultiByteStr);
    }
    
    while (i < cchMultiByte)
    {
        ch = lpMultiByteStr[i];
        if (ch <= 0x7f)
        {
            cchWCCount++;
            i++;
        } else if (IS_GB_LEAD_BYTE(ch))
        {
            offset1 = (ch - GBK2K_BYTE1_MIN);
            //
            // If this is a lead byte, look ahead to see if this is
            // a two-byte GB18030 or four-byte GB18030.
            //
            if (i+1 < cchMultiByte)
            {
                if (IS_GB_TWO_BYTES_TRAILING(lpMultiByteStr[i+1]))
                {
                    
                    //
                    // The trailing byte is a GB18030 two-byte.
                    //
                    cchWCCount++;
                    i += 2;                        
                } else if (i+3 < cchMultiByte) 
                {
                    //
                    // Check if this is a four-byte GB18030.
                    //
                    if (IS_GB_FOUR_BYTES_TRAILING(lpMultiByteStr[i+1]) &&
                        IS_GB_LEAD_BYTE(lpMultiByteStr[i+2]) &&
                        IS_GB_FOUR_BYTES_TRAILING(lpMultiByteStr[i+3]))
                    {
                        offset2 = lpMultiByteStr[i+1] - GBK2K_BYTE2_MIN;
                        offset3 = lpMultiByteStr[i+2] - GBK2K_BYTE3_MIN;
                        offset4 = lpMultiByteStr[i+3] - GBK2K_BYTE4_MIN;
                        //
                        // Four-byte GB18030
                        //
                        dwFourBytesOffset = GET_FOUR_BYTES_OFFSET(offset1, offset2, offset3, offset4);
                        if (dwFourBytesOffset <= g_wMax4BytesOffset) 
                        {
                            //
                            // The Unicode will be in the BMP range.
                            //
                            cchWCCount++;                            
                        } else if (dwFourBytesOffset >= SURROGATE_OFFSET && dwFourBytesOffset <= SURROGATE_MAX_OFFSET)
                        {
                            //
                            // This will be converted to a surrogate pair.
                            //
                            cchWCCount+=2;
                        } else {
                            //
                            // Valid GBK2K code point, but can not be mapped to Unicode.
                            //
                            cchWCCount++;
                        }                        
                        i += 4;
                    } else 
                    {
                        if (bSupportEncoder)
                        {
                            // Set i to cchMultiByte so that we will bail out the while loop.
                            i = cchMultiByte;
                        } else 
                        {
                            //
                            // We have a lead byte, but do have have a valid trailing byte.
                            //
                            // Use default Unicode char.
                            i++;
                            cchWCCount++;
                        }                    
                    }
                }else
                {
                    if (bSupportEncoder)
                    {
                        // Set i to cchMultiByte so that we will bail out the while loop.
                        i = cchMultiByte;
                    } else 
                    {
                        //
                        // We have a lead byte, but do have have a valid trailing byte.
                        //
                        // Use default Unicode char.
                        i++;
                        cchWCCount++;
                    }
                }
            } else
            {
                //
                // We have a lead byte at the end of the string.
                //
                if (bSupportEncoder)
                {
                    i++;
                } else
                {
                    // Use default Unicode char.
                    i++;
                    cchWCCount++;
                }
            }
        }else
        {
            //
            // This byte is NOT between 0x00 ~ 0x7f, and not a lead byte.
            // Use the default character.
            //
            i++;
            cchWCCount++;
        }
    }

    return (cchWCCount);
    
}

BOOL __forceinline PutDefaultCharacter(UINT* pCchWCCount, UINT cchWideChar, LPWSTR lpWideCharStr)
{
    //
    // This byte is NOT between 0x00 ~ 0x7f, not a lead byte.
    //
    if (*pCchWCCount >= cchWideChar)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (FALSE);
    }
    lpWideCharStr[(*pCchWCCount)++] = GB18030_DEFAULT_UNICODE_CHAR; 
    return (TRUE);
}

STDAPI_(DWORD) BytesToUnicode(
    BYTE* lpMultiByteStr,
    UINT cchMultiByte,
    UINT* pcchLeftOverBytes,
    LPWSTR lpWideCharStr,
    UINT cchWideChar)
{

    UINT i = 0;
    BYTE ch;
    UINT cchWCCount = 0;
    BYTE offset1, offset2, offset3, offset4;
    WORD wOffset;
    DWORD dwOffset;
    int nResult;
    
    if ((lpWideCharStr == NULL) || (cchWideChar == 0))
    {
        return (GetBytesToUnicodeCount(lpMultiByteStr, cchMultiByte, (pcchLeftOverBytes != NULL)));
    }
    
    if (cchMultiByte == -1)
    {
        cchMultiByte = strlen((LPSTR)lpMultiByteStr);
    }

    if (pcchLeftOverBytes != NULL)
    {
        *pcchLeftOverBytes = 0;
    }

    //
    // NOTENOTE YSLin:
    // If you make fix in the following code, remember to make the appropriate fix
    // in GetBytesToUnicodeCount() as well.
    //
    while (i < cchMultiByte)
    {
        ch = lpMultiByteStr[i];
        if (ch <= 0x7f)
        {
            // 
            // This byte is from 0x00 ~ 0x7f.
            //
            if (cchWCCount >= cchWideChar)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return (0);
            }                
            lpWideCharStr[cchWCCount++] = ch;
            i++;
        } else if (IS_GB_LEAD_BYTE(ch))
        {
            offset1 = ch - GBK2K_BYTE1_MIN;
            //
            // If this is a lead byte, just look ahead to see if this is
            // a two-byte GB18030 or four-byte GB18030.
            //
            if (i+1 < cchMultiByte)
            {
                if (IS_GB_TWO_BYTES_TRAILING(lpMultiByteStr[i+1]))
                {
                    //
                    // The trailing byte is a GB18030 two-byte.
                    //
                
                    //
                    // Look up the table to see if we have the table for
                    // the mapping Unicode character.
                    //                
                    wOffset = g_wGBLeadByteOffset[ch - 0x80];
                    if (wOffset == 0x0000)
                    {
                        if (cchWCCount == cchWideChar)
                        {
                            SetLastError(ERROR_INSUFFICIENT_BUFFER);
                            return (0);
                        }
                        //
                        // We don't have the table, because this is a GBK compatible two-byte GB18030.
                        //
                        
                        //
                        // Two-byte GB18030
                        //
                        nResult = MultiByteToWideChar(CODEPAGE_GBK, 0, (LPCSTR)(lpMultiByteStr+i), 2, lpWideCharStr+cchWCCount, 1);
                        if (nResult == 0)
                        {
                            return (0);
                        }
                        cchWCCount++; 
                        i += 2;                        
                    } else
                    {
                        if (cchWCCount == cchWideChar)
                        {
                            SetLastError(ERROR_INSUFFICIENT_BUFFER);
                            return (0);
                        }                                    
                        wOffset -= 0x0100;
                        lpWideCharStr[cchWCCount++] = g_wUnicodeFromGBTwoBytes[wOffset + lpMultiByteStr[i+1]];
                        i+= 2;
                    }                
                } else if (i+3 < cchMultiByte) 
                {
                    if (IS_GB_FOUR_BYTES_TRAILING(lpMultiByteStr[i+1]) &&
                        IS_GB_LEAD_BYTE(lpMultiByteStr[i+2]) &&
                        IS_GB_FOUR_BYTES_TRAILING(lpMultiByteStr[i+3]))
                    {
                        offset2 = lpMultiByteStr[i+1] - GBK2K_BYTE2_MIN;
                        offset3 = lpMultiByteStr[i+2] - GBK2K_BYTE3_MIN;
                        offset4 = lpMultiByteStr[i+3] - GBK2K_BYTE4_MIN;
                        
                        //
                        // Four-byte GB18030
                        //
                        dwOffset = GET_FOUR_BYTES_OFFSET(offset1, offset2, offset3, offset4);
                        if (dwOffset <= g_wMax4BytesOffset) 
                        {
                            if (cchWCCount == cchWideChar)
                            {
                                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                                return (0);
                            }                                    
                        
                            //
                            // The Unicode will be in the BMP range.
                            //
                            lpWideCharStr[cchWCCount++] = g_wGBFourBytesToUnicode[dwOffset];
                        } else if (dwOffset >= SURROGATE_OFFSET && dwOffset <= SURROGATE_MAX_OFFSET) 
                        {
                            if (cchWCCount + 2 > cchWideChar)
                            {
                                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                                return (0);
                            }                                    
                            //
                            // This will be converted to a surrogate pair.
                            //
                            dwOffset -= SURROGATE_OFFSET;
                            lpWideCharStr[cchWCCount++] = 0xd800 + (WORD)(dwOffset / 0x400);
                            lpWideCharStr[cchWCCount++] = 0xdc00 + (WORD)(dwOffset % 0x400);
                        } else
                        {
                            //
                            // Valid GBK2K code point, but can not be mapped to Unicode.
                            //
                            if (!PutDefaultCharacter(&cchWCCount, cchWideChar, lpWideCharStr))
                            {
                                return (0);
                            }    
                        }   
                        i += 4;
                    }else
                    {
                        if (!PutDefaultCharacter(&cchWCCount, cchWideChar, lpWideCharStr))
                        {
                            return (0);
                        }
                        i++;
                    }                    
                }else
                {
                    if (pcchLeftOverBytes != NULL)
                    {
                        *pcchLeftOverBytes = cchMultiByte - i;
                        // Set i to cchMultiByte so that we will bail out the while loop.
                        i = cchMultiByte;
                    } else 
                    {
                        //
                        // We have a lead byte, but do have have a valid trailing byte.
                        //
                        // Use default Unicode char.
                        if (!PutDefaultCharacter(&cchWCCount, cchWideChar, lpWideCharStr))
                        {
                            return (0);
                        }
                        i++;
                    }
                }
            } else
            {
                if (pcchLeftOverBytes != NULL) 
                {
                    *pcchLeftOverBytes = 1;
                    i++;
                } else
                {
                    // We have a lead byte, but do have have a trailing byte.
                    // Use default Unicode char.
                    if (!PutDefaultCharacter(&cchWCCount, cchWideChar, lpWideCharStr))
                    {
                        return (0);
                    }
                    i++;
                }
            }
        } else 
        {
            if (!PutDefaultCharacter(&cchWCCount, cchWideChar, lpWideCharStr))
            {
                return (0);
            }
            i++;
        }
    }
    return (cchWCCount);
}

DWORD GetUnicodeToBytesCount(LPWSTR lpWideCharStr, int cchWideChar)
{
    int i;
    WORD wch;
    int cchMBCount = 0;
    DWORD wOffset;

    if (cchWideChar == -1)
    {
        cchWideChar = wcslen(lpWideCharStr);
    }
    
    for (i = 0; i < cchWideChar; i++) 
    {
        wch = lpWideCharStr[i];

        if (wch <= 0x7f)
        {
            // One-byte GB18030.
            cchMBCount++;
        } else if (IS_HIGH_SURROGATE(wch))
        {
            //
            // Look ahead one character to see if the next char is a low surrogate.
            //
            if (i + 1 < cchWideChar)
            {
                if (IS_LOW_SURROGATE(lpWideCharStr[ i+1 ]))
                {
                    //
                    // Found a surrogate pair.  This will be a four-byte GB18030.
                    //
                    cchMBCount += 4;    
                    i++;
                } else
                {
                    //
                    // A High surrogate character without a trailing low surrogate character.
                    // In this case, we will convert this character to a default character.
                    //
                    cchMBCount++;
                }
            } else
            {
                //
                // A High surrogate character without a valid trailing low surrogate character.
                // In this case, we will convert this character to a default character.  
                //
                cchMBCount++;
            }
        } else if (IS_LOW_SURROGATE(wch))
        {
            //
            // Only a low surrogate character without a leading high surrogate.
            // In this case, we will convert this character to a default character.  
            //
            cchMBCount++;
        } else
        {
            //
            // Not a surrogate character.  Look up the table to see this BMP Unicode character
            // will be converted to a two-byte GB18030 or four-byte GB18030.
            //
            wOffset = g_wUnicodeToGB[wch];

            if (wOffset == 0xFFFF)
            {
                //
                // This Unicode character will be converted to GBK compatible two-byte code.
                //
                cchMBCount += 2;
            } else if (wOffset <= g_wMax4BytesOffset)
            {
                //
                // This Unicode character will be converted to four-byte GB18030.
                //
                cchMBCount += 4;                    
            } else
            {
                //
                // This Unicode character will be converted to two-byte GB18030, which is not compatible
                // with GBK.
                //
                cchMBCount += 2;
            }
        }                
    }
    return (cchMBCount);
}

STDAPI_(DWORD) UnicodeToBytes(
    LPWSTR lpWideCharStr,
    UINT cchWideChar,
    LPSTR lpMultiByteStr,
    UINT cchMultiByte)
{
    UINT i;
    WORD wch;
    UINT cchMBCount = 0;
    CHAR MBTwoBytes[2];
    BYTE MBFourBytes[4];
    WORD wOffset;
    DWORD dwSurrogateOffset;
    int nResult;

    if ((lpMultiByteStr == NULL) || (cchMultiByte == 0))
    {
        return (GetUnicodeToBytesCount(lpWideCharStr, cchWideChar));
    }

    if (cchWideChar == -1)
    {
        cchWideChar = wcslen(lpWideCharStr);
    }
    //
    // NOTENOTE YSLin:
    // If you make fix in the following code, remember to make the appropriate fix
    // in GetUnicodeToBytesCount() as well.
    //
    for (i = 0; i < cchWideChar; i++) 
    {
        wch = lpWideCharStr[i];

        if (wch <= 0x7f)
        {
            if (cchMBCount == cchMultiByte)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return (0);
            }
            lpMultiByteStr[cchMBCount++] = (BYTE)wch;
        } else if (IS_HIGH_SURROGATE(wch))
        {
            //
            // Look ahead one character to see if the next char is a low surrogate.
            //
            if (i + 1 < cchWideChar)
            {
                if (IS_LOW_SURROGATE(lpWideCharStr[ i+1 ]))
                {
                    if (cchMBCount + 4 > cchMultiByte)
                    {
                        SetLastError(ERROR_INSUFFICIENT_BUFFER);
                        return (0);
                    }
                
                    i++;
                    //
                    // A surrogate pair will be converted to GB 18030 four-byte from
                    // 0x90308130 ~ 0xe339fe39.
                    //                
                    dwSurrogateOffset = (wch - 0xd800) * 0x0400 + (lpWideCharStr[i] - 0xdc00);
                    lpMultiByteStr[cchMBCount+3] = (BYTE)(dwSurrogateOffset % GBK2K_BYTE4_RANGE) + GBK2K_BYTE4_MIN;
                    dwSurrogateOffset /= GBK2K_BYTE4_RANGE;
                    lpMultiByteStr[cchMBCount+2] = (BYTE)(dwSurrogateOffset % GBK2K_BYTE3_RANGE) + GBK2K_BYTE3_MIN;
                    dwSurrogateOffset /= GBK2K_BYTE3_RANGE;
                    lpMultiByteStr[cchMBCount+1] = (BYTE)(dwSurrogateOffset % GBK2K_BYTE2_RANGE) + GBK2K_BYTE2_MIN;
                    dwSurrogateOffset /= GBK2K_BYTE2_RANGE;
                    lpMultiByteStr[cchMBCount]   = (BYTE)(dwSurrogateOffset % GBK2K_BYTE1_RANGE) + 0x90;

                    cchMBCount += 4;   
                } else
                {
                    if (cchMBCount == cchMultiByte)
                    {
                        SetLastError(ERROR_INSUFFICIENT_BUFFER);
                        return (0);
                    }
                    //
                    // A High surrogate character is at the end of string.
                    // In this case, we will convert this character to a default character.  
                    //
                    lpMultiByteStr[cchMBCount++] = GB18030_DEFAULT_CHAR;
                }
            }else
            {
                if (cchMBCount >= cchMultiByte)
                {
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }
                //
                // A High surrogate character without a valid trailing low surrogate character.
                // In this case, we will convert this character to a default character.  
                //
                lpMultiByteStr[cchMBCount++] = GB18030_DEFAULT_CHAR;
            }
        } else if (IS_LOW_SURROGATE(wch))
        {
            if (cchMBCount == cchMultiByte)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return (0);
            }
            //
            // Only a low surrogate character without a leading high surrogate.
            // In this case, we will convert this character to a default character.  
            //
            lpMultiByteStr[cchMBCount++] = GB18030_DEFAULT_CHAR;
        } else
        {
            //
            // This character is not below 0x7f, not a surrogate character.
            // Check the table to see how this Unicode character should be
            // converted.  It could be:
            //  1. Two-byte GB18030, which is compatible with GBK.  (wOffset == 0xffff)
            //  2. Two-byte GB18030, which is NOT compatible with GBK. (wOffset = 0xfffe and below)
            //  3. Four-byte GB18030. (wOffset >= 0 && wOffset < g_wMax4BytesOffset)
            //
            wOffset = g_wUnicodeToGB[wch];

            if (wOffset == 0xffff)
            {
                // 
                // This Unicode character will be converted to the same two-byte GBK code, so use GBK table.
                //
                if (cchMBCount + 2 > cchMultiByte) 
                {
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }
                nResult = WideCharToMultiByte(CODEPAGE_GBK, 0, lpWideCharStr+i, 1, lpMultiByteStr+cchMBCount, 2, NULL, NULL);
                if (nResult == 0) 
                {
                    return (0);
                }
                if (cchMBCount + nResult > cchMultiByte)
                {
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }                
                cchMBCount += nResult;
            } else if (wOffset <= g_wMax4BytesOffset)
            {
                if (cchMBCount + 4 > cchMultiByte)
                {
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }                
            
                //
                // This Unicode character will be converted to four-byte GB18030.
                //
                lpMultiByteStr[cchMBCount+3] = (wOffset % GBK2K_BYTE4_RANGE) + GBK2K_BYTE4_MIN;
                wOffset /= GBK2K_BYTE4_RANGE;
                lpMultiByteStr[cchMBCount+2] = (wOffset % GBK2K_BYTE3_RANGE) + GBK2K_BYTE3_MIN;
                wOffset /= GBK2K_BYTE3_RANGE;
                lpMultiByteStr[cchMBCount+1] = (wOffset % GBK2K_BYTE2_RANGE) + GBK2K_BYTE2_MIN;
                wOffset /= GBK2K_BYTE2_RANGE;
                lpMultiByteStr[cchMBCount]   = (wOffset % GBK2K_BYTE1_RANGE) + GBK2K_BYTE1_MIN;

                cchMBCount += 4;                    
            } else
            {
                if (cchMBCount + 2 > cchMultiByte)
                {
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }             
                //
                // This Unicode character will be converted to two-byte GB18030, which is not compatible
                // with GBK.
                //
                wOffset = 0xfffe - wOffset;
                // We don't have to check the range of wOffset here, since the value of wOffset is coming from
                // g_wUnicodeToGB.
                CopyMemory(lpMultiByteStr+cchMBCount, &g_wUnicodeToGBTwoBytes[wOffset * 2], 2);
                // Copy two bytes (a WORD) into lpMultiByteStr[cchMBCount].
                // Instead od CompMemory(), This is probably faster:
                // *((LPWORD)lpMultiByteStr[cchMBCount]) = *((LPWORD)g_wUnicodeToGBTwoBytes[wOffset * 2]);
                cchMBCount += 2;
            }
        }                
    }
            
    return (cchMBCount);
}
