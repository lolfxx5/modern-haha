/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    parse.c

Abstract:

    This module contains UI code for the OS chooser

Author:

    Geoff Pease (GPease) May 29 1998

Revision History:

--*/

#ifndef __PARSE_H__
#define __PARSE_H__

enum ACTIONS {
    ACTION_NOP = 0,
    ACTION_REBOOT,
    ACTION_JUMP,
    ACTION_LOGIN    
#if defined(_BUILDING_OSDISP_)
    ,ACTION_REFRESH
#endif
};

extern enum ACTIONS SpecialAction;

extern CHAR DomainName[256];
extern CHAR UserName[256];
extern CHAR Password[128];
extern CHAR AdministratorPassword[OSC_ADMIN_PASSWORD_LEN+1];
extern CHAR AdministratorPasswordConfirm[OSC_ADMIN_PASSWORD_LEN+1];

#ifndef _FILETIME_
#define _FILETIME_
typedef struct _FILETIME {
    ULONG dwLowDateTime;
    ULONG dwHighDateTime;
} FILETIME, *PFILETIME;
#endif // _FILETIME_

extern FILETIME GlobalFileTime;
extern TIME_FIELDS ArcTimeForUTCTime;
extern ULONG AuthenticationType;

#define OSCHOICE_AUTHENETICATE_TYPE_NTLM_V1         0x00000001
#define OSCHOICE_AUTHENETICATE_TYPE_NTLM_V2         0x00000002

NTSTATUS
SetFileTimeFromTimeString(
    IN PSTR TimeString,
    OUT PFILETIME FileTime,
    OUT TIME_FIELDS *ArcTime
    );


CHAR
BlProcessScreen(
    IN PCHAR InputString,
    OUT PCHAR OutputString
    );

#define ASCI_CSI_OUT    TEXT("\033[")     // escape-leftbracket

#define ATT_FG_BLUE     4
#define ATT_FG_WHITE    7
#define ATT_BG_BLUE     (ATT_FG_BLUE    << 4)
#define ATT_BG_WHITE    (ATT_FG_WHITE   << 4)
#define DEFATT          (ATT_FG_WHITE | ATT_BG_BLUE)
#define INVATT          (ATT_FG_BLUE |  ATT_BG_WHITE)

#endif // __PARSE_H__
