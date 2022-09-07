/***
*towlower.c - convert wide character to lower case
*
*       Copyright (c) 1991-2001, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Defines towlower().
*
*Revision History:
*       10-11-91  ETC   Created.
*       12-10-91  ETC   Updated nlsapi; added multithread.
*       04-06-92  KRS   Make work without _INTL also.
*       01-19-93  CFW   Changed LCMapString to LCMapStringW.
*       04-06-93  SKS   Replace _CRTAPI* with _cdecl
*       06-02-93  SRW   ignore _INTL if _NTSUBSET_ defined.
*       06-11-93  CFW   Fix error handling bug.
*       09-15-93  CFW   Use ANSI conformant "__" names.
*       09-22-93  CFW   Use __crtxxx internal NLS API wrapper.
*       09-29-93  GJF   Merged NT SDK and Cuda versions.
*       11-09-93  CFW   Add code page for __crtxxx().
*       02-07-94  CFW   POSIXify.
*       09-06-94  CFW   Remove _INTL switch.
*       10-25-94  GJF   Sped up for C locale. Also, added _towlower_lk.
*       09-26-95  GJF   New locking macro, and scheme, for functions which
*                       reference the locale.
*       04-01-96  BWT   POSIX work.
*       06-25-96  GJF   Removed DLL_FOR_WIN32S and cleaned up the format a
*                       wee bit.
*       08-27-98  GJF   Revised multithread support based on threadlocinfo
*                       struct.
*
*******************************************************************************/

#include <cruntime.h>
#include <ctype.h>
#include <stdio.h>
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>
#include <awint.h>

/***
*wchar_t towlower(c) - convert wide character to lower case
*
*Purpose:
*       towlower() returns the lowercase equivalent of its argument
*
*Entry:
*       c - wchar_t value of character to be converted
*
*Exit:
*       if c is an upper case letter, returns wchar_t value of lower case
*       representation of c. otherwise, it returns c.
*
*Exceptions:
*
*******************************************************************************/

wchar_t __cdecl towlower (
        wchar_t c
        )
{
#if     !defined(_NTSUBSET_) && !defined(_POSIX_)

#ifdef  _MT

        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        if ( c == WEOF )
            return c;

        if ( ptloci->lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
            return __ascii_towlower(c);

        return __towlower_mt(ptloci, c);
}

/***
*wchar_t __towlower_mt(ptloci, c) - convert wide character to lower case
*
*Purpose:
*       Multi-thread function only! Non-locking version of towlower.
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

wchar_t __cdecl __towlower_mt (
        pthreadlocinfo ptloci,
        wchar_t c
        )
{

#endif  /* _MT */

        wchar_t widechar;

        if (c == WEOF)
            return c;

#ifndef _MT
        if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
            return __ascii_towlower(c);
#endif

        /* if checking case of c does not require API call, do it */
        if ( c < 256 ) {
#ifdef  _MT
            if ( !__iswupper_mt(ptloci, c) ) {
#else
            if ( !iswupper(c) ) {
#endif
                return c;
            }
        }

        /* convert wide char to lowercase */
#ifdef  _MT
        if ( 0 == __crtLCMapStringW( ptloci->lc_handle[LC_CTYPE], 
#else
        if ( 0 == __crtLCMapStringW( __lc_handle[LC_CTYPE], 
#endif
                                     LCMAP_LOWERCASE,
                                     (LPCWSTR)&c, 
                                     1, 
                                     (LPWSTR)&widechar, 
                                     1, 
#ifdef  _MT
                                     ptloci->lc_codepage ) )
#else
                                     __lc_codepage ) )
#endif
        {
            return c;
        }

        return widechar;

#else   /* _NTSUBSET_/_POSIX_ */

        return (iswupper(c) ? (c + (wchar_t)(L'a' - L'A')) : c);

#endif  /* _NTSUBSET_/_POSIX_ */
}
