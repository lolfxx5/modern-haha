/*++

Copyright (c) 1994-1999  Microsoft Corporation

Module Name:

    Support.c

Abstract:

    Support routines for compact utility

Author:

    Matthew Bradburn    [mattbr]        05-Oct-1994
    Robert Reichel      [RobertRe]      01-Apr-1997

Revision History:


--*/

#undef UNICODE
#define UNICODE

#undef _UNICODE
#define _UNICODE

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <windows.h>
#include "support.h"
#include "msg.h"



//
//  Declare routines to put out internationalized messages
//

typedef enum {
    READ_ACCESS,
    WRITE_ACCESS
} STREAMACCESS;


DWORD MySetThreadUILanguage(DWORD dwParam)
{
    HMODULE  hKernel32Dll  = NULL;
    DWORD    hr = ERROR_SUCCESS; 
    typedef LANGID (WINAPI * SetThreadUILanguageFunc)(DWORD dwReserved);
    SetThreadUILanguageFunc   pSetThreadUILanguage  = NULL;

    hKernel32Dll = LoadLibraryW(L"kernel32.dll");
    if (NULL == hKernel32Dll) { 
        return GetLastError();
    }

    pSetThreadUILanguage = (SetThreadUILanguageFunc)GetProcAddress(hKernel32Dll, "SetThreadUILanguage");
    if (NULL != pSetThreadUILanguage) {
        pSetThreadUILanguage(dwParam);
    } else {
        hr = GetLastError();
    }

    FreeLibrary(hKernel32Dll);  
	
    return hr; 
}

HANDLE
GetStandardStream(
    IN HANDLE   Handle,
    IN STREAMACCESS Access
    );

HANDLE hInput;
HANDLE hOutput;
HANDLE hError;

#define STDIN   0
#define STDOUT  1
#define STDERR  2

BOOL ConsoleInput;
BOOL ConsoleOutput;
BOOL ConsoleError;

int
FileIsConsole(int fh)
{
    unsigned htype;
    DWORD dwMode;
    HANDLE hFile;

    hFile = (HANDLE)_get_osfhandle(fh);
    htype = GetFileType(hFile);
    htype &= ~FILE_TYPE_REMOTE;

    if (FILE_TYPE_CHAR == htype) {

        switch (fh) {
        case STDIN:
            hFile = GetStdHandle(STD_INPUT_HANDLE);
            break;
        case STDOUT:
            hFile = GetStdHandle(STD_OUTPUT_HANDLE);
            break;
        case STDERR:
            hFile = GetStdHandle(STD_ERROR_HANDLE);
            break;
        }

        if (GetConsoleMode(hFile, &dwMode)) {
            return TRUE;
        }
    }

    return FALSE;

}


VOID
InitializeIoStreams()
{
    hInput = GetStdHandle(STD_INPUT_HANDLE);
    ConsoleInput = FileIsConsole(STDIN);

    hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    ConsoleOutput = FileIsConsole(STDOUT);

    hError = GetStdHandle(STD_ERROR_HANDLE);
    ConsoleError = FileIsConsole(STDERR);
}

VOID
DisplayMsg(DWORD MsgNum, ... )
{
    DWORD len, bytes_written = 0;
    BOOL success = FALSE;
    DWORD status;
    va_list ap;
    LPTSTR DisplayBuffer;

    MySetThreadUILanguage(0);

    va_start(ap, MsgNum);

    len = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL, MsgNum, 0,
        (LPVOID)&DisplayBuffer, 0, &ap);

    if (len && DisplayBuffer) {

        if (ConsoleOutput) {
            success = WriteConsole(hOutput, (LPVOID)DisplayBuffer, len,
                     &bytes_written, NULL);
    
        } else {
            DWORD ascii_len;
            LPSTR DisplayBufferAscii;
    
            len = lstrlen(DisplayBuffer);
    
            ascii_len = WideCharToMultiByte(
                CP_OEMCP,                                 // Unicode -> OEM
                0,                                        // gives best visual match
                DisplayBuffer, len+1,                     // source & length
                NULL, 0,                                  // dest & length
                NULL,                                     // default char
                NULL);

            if (ascii_len) {
                DisplayBufferAscii = LocalAlloc(LPTR, ascii_len);
                if (DisplayBufferAscii != NULL) {

                    ascii_len = WideCharToMultiByte(
                                    CP_OEMCP,
                                    0,
                                    DisplayBuffer, len+1,
                                    DisplayBufferAscii, ascii_len,
                                    NULL,
                                    NULL);
                    if (ascii_len) {
                        success = WriteFile(hOutput, (LPVOID)DisplayBufferAscii, ascii_len,
                                 &bytes_written, NULL);
            
                        len = ascii_len;
                    }
        
                    LocalFree(DisplayBufferAscii);
                }
            }
    
        }
    
        if (!success || bytes_written != len) {
            status = GetLastError();
        }
    
        LocalFree(DisplayBuffer);
    }

    va_end(ap);
}

VOID
DisplayErr(
    PTCHAR Prefix,
    DWORD MsgNum,
    ...
    )
{
    DWORD len, bytes_written, PrefixLen;
    BOOL success = FALSE;
    BOOL errset = FALSE;
    DWORD status;
    va_list ap;
    LPTSTR DisplayBuffer;
    LPTSTR MessageBuffer;
    #define PREFIX_MSG TEXT(": ")

    MySetThreadUILanguage(0);

    va_start(ap, MsgNum);

    len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            NULL, MsgNum, 0,
            (LPVOID)&MessageBuffer, 0, &ap);
    if (len && MessageBuffer) {

        if (Prefix == NULL) {
            PrefixLen = 0;
        } else {
            PrefixLen = wcslen(Prefix)+wcslen(PREFIX_MSG);
        }
    
        DisplayBuffer = LocalAlloc(LPTR, (len+PrefixLen+1) * sizeof(WCHAR));
        if (DisplayBuffer != NULL) {
    
            if (NULL != Prefix) {
                lstrcpy(DisplayBuffer, Prefix);
                lstrcat(DisplayBuffer, PREFIX_MSG);
            } else {
                DisplayBuffer[0] = UNICODE_NULL;
            }
    
            lstrcat(DisplayBuffer, MessageBuffer);
    
            len = lstrlen(DisplayBuffer);
    
            if (ConsoleError) {
                success = WriteConsole(hError, (LPVOID)DisplayBuffer, len,
                         &bytes_written, NULL);
    
            } else {
                DWORD ascii_len;
                LPSTR DisplayBufferAscii;
    
                ascii_len = WideCharToMultiByte(
                    CP_OEMCP,                                 // Unicode -> OEM
                    0,                                        // gives best visual match
                    DisplayBuffer, len+1,                     // source & length
                    NULL, 0,                                  // dest & length
                    NULL,                                     // default char
                    NULL);

                if (ascii_len) {

                    DisplayBufferAscii = LocalAlloc(LPTR, ascii_len);
                    if (DisplayBufferAscii != NULL) {
                        ascii_len = WideCharToMultiByte(
                                        CP_OEMCP,
                                        0,
                                        DisplayBuffer, len+1,
                                        DisplayBufferAscii, ascii_len,
                                        NULL,
                                        NULL);

                        if (ascii_len) {
                            success = WriteFile(hError, (LPVOID)DisplayBufferAscii, ascii_len,
                                     &bytes_written, NULL);
                        } else {
                            status = GetLastError();
                            errset = TRUE;
                        }
        
        
                        LocalFree(DisplayBufferAscii);
                    } else {
                        status = ERROR_NOT_ENOUGH_MEMORY;
                        errset = TRUE;
                    }
                } else {
                    status = GetLastError();

                    //
                    // No need to get last error again
                    //

                    errset = TRUE;

                }
    
            }
    
            LocalFree(DisplayBuffer);
        }
    
        if (!success && !errset) {
            status = GetLastError();
        }
    
        LocalFree(MessageBuffer);
    }

    va_end(ap);
}

DWORD
GetResourceString(
    LPWSTR *OutStr,
    DWORD MsgNum,
    ...
    )
{

    DWORD rc = ERROR_SUCCESS;
    DWORD len;
    va_list ap;

    MySetThreadUILanguage(0);

    va_start(ap, MsgNum);

    *OutStr = NULL;

    len = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            NULL, 
            MsgNum, 
            0,
            (LPVOID)OutStr, 
            0, 
            &ap
            );

    if (len && *OutStr) {

        //
        // Let's get rid of \n
        //
        
        len = wcslen( *OutStr );
        if (((*OutStr)[len-1] == 0x000a) && ((*OutStr)[len-2] == 0x000d) ) {

            (*OutStr)[len-2] = 0;

        }

    } else {
        rc = ERROR_NOT_ENOUGH_MEMORY;
    }

    va_end(ap);

    return rc;

}

BOOLEAN
IsNtldr(
    PTCHAR Path
    )
/*++

Routine Description:

    Used to keep the user from compressing \NTLDR (which would prevent
    the machine from booting).

Arguments:

    Path - the path to examine.

Return Value:

    TRUE - the path looks like \ntldr.
    FALSE - the path does not look like \ntldr.

--*/
{
    PTCHAR pch;

    // try "X:\ntldr"

    if (0 == lstricmp(Path + 2, TEXT("\\ntldr"))) {
        return TRUE;
    }

    // try "\\machine\share\ntldr"

    if ('\\' == Path[0] && '\\' != Path[1]) {
        pch = lstrchr(Path + 2, '\\');
        if (NULL == pch) {
            return FALSE;
        }
        pch = lstrchr(pch + 1, '\\');
        if (NULL == pch) {
            return FALSE;
        }
        if (0 == lstricmp(pch, TEXT("\\ntldr"))) {
            return TRUE;
        }
    }

    return FALSE;
}


ULONG
FormatFileSize(
    IN PLARGE_INTEGER FileSize,
    IN DWORD Width,
    OUT PTCHAR FormattedSize,
    IN BOOLEAN Commas
    )
{
    TCHAR Buffer[100];
    PTCHAR s, s1;
    ULONG DigitIndex, Digit;
    ULONG Size;
    LARGE_INTEGER TempSize;

    s = &Buffer[ 99 ];
    *s = TEXT('\0');
    DigitIndex = 0;
    TempSize = *FileSize;
    while (TempSize.HighPart != 0) {
        if (TempSize.HighPart != 0) {
            Digit = (ULONG)(TempSize.QuadPart % 10);
            TempSize.QuadPart = TempSize.QuadPart / 10;
        } else {
            Digit = TempSize.LowPart % 10;
            TempSize.LowPart = TempSize.LowPart / 10;
        }
        *--s = (TCHAR)(TEXT('0') + Digit);

        if ((++DigitIndex % 3) == 0 && Commas) {
            *--s = TEXT(',');
        }
    }
    Size = TempSize.LowPart;
    while (Size != 0) {
        *--s = (TCHAR)(TEXT('0') + (Size % 10));
        Size = Size / 10;

        if ((++DigitIndex % 3) == 0 && Commas) {
            *--s = TEXT(',');
        }
    }

    if (DigitIndex == 0) {
        *--s = TEXT('0');
    } else if (Commas && *s == TEXT(',')) {
        s += 1;
    }

    Size = lstrlen( s );
    if (Width != 0 && Size < Width) {
        s1 = FormattedSize;
        while (Width > Size) {
            Width -= 1;
            *s1++ = TEXT(' ');
        }
        lstrcpy( s1, s );
    } else {
        lstrcpy( FormattedSize, s );
    }

    return lstrlen( FormattedSize );
}


