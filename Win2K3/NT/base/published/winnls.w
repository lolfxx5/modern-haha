/*++

Copyright (c) Microsoft Corporation  All rights reserved.

Module Name:

    winnls.h

Abstract:

    Procedure declarations, constant definitions, and macros for the
    NLS component.

--*/

/*++ BUILD Version: 0003    // Increment this if a change has global effects    ;internal_NT
                                                                                ;internal_NT
Copyright (c) Microsoft Corporation.  All rights reserved.                      ;internal_NT
                                                                                ;internal_NT
Module Name:                                                                    ;internal_NT
                                                                                ;internal_NT
    winnlsp.h                                                                   ;internal_NT
                                                                                ;internal_NT
Abstract:                                                                       ;internal_NT
                                                                                ;internal_NT
    Private procedure declarations, constant definitions, and macros for the    ;internal_NT
    NLS component.                                                              ;internal_NT
                                                                                ;internal_NT
--*/                                                                            ;internal_NT
                                                                                ;internal_NT

#ifndef _WINNLS_
#define _WINNLS_

#ifndef _WINNLSP_            ;internal_NT
#define _WINNLSP_            ;internal_NT
                             ;both
#ifdef __cplusplus           ;both
extern "C" {                 ;both
#endif                       ;both
                             ;both


#ifndef NONLS

#ifdef _MAC
#include <macwin32.h>
#endif



////////////////////////////////////////////////////////////////////////////
//
//  Constants
//
//  Define all constants for the NLS component here.
//
////////////////////////////////////////////////////////////////////////////

//
//  String Length Maximums.
//
#define MAX_LEADBYTES             12          // 5 ranges, 2 bytes ea., 0 term.
#define MAX_DEFAULTCHAR           2           // single or double byte


//
//  MBCS and Unicode Translation Flags.
//
#define MB_PRECOMPOSED            0x00000001  // use precomposed chars
#define MB_COMPOSITE              0x00000002  // use composite chars
#define MB_USEGLYPHCHARS          0x00000004  // use glyph chars, not ctrl chars
#define MB_ERR_INVALID_CHARS      0x00000008  // error for invalid chars

#define WC_COMPOSITECHECK         0x00000200  // convert composite to precomposed
#define WC_DISCARDNS              0x00000010  // discard non-spacing chars
#define WC_SEPCHARS               0x00000020  // generate separate chars
#define WC_DEFAULTCHAR            0x00000040  // replace w/ default char

;begin_winver_500
#define WC_NO_BEST_FIT_CHARS      0x00000400  // do not use best fit chars
;end_winver_500

                                                                ;internal_NT
//                                                              ;internal_NT
//  Flags for DLL Code Page Translation Function.               ;internal_NT
//                                                              ;internal_NT
#define NLS_CP_CPINFO             0x10000000                    ;internal_NT
#define NLS_CP_CPINFOEX           0x20000000                    ;internal_NT
#define NLS_CP_MBTOWC             0x40000000                    ;internal_NT
#define NLS_CP_WCTOMB             0x80000000                    ;internal_NT
                                                                ;internal_NT
                                                                ;both
//
//  Character Type Flags.
//
#define CT_CTYPE1                 0x00000001  // ctype 1 information
#define CT_CTYPE2                 0x00000002  // ctype 2 information
#define CT_CTYPE3                 0x00000004  // ctype 3 information

//
//  CType 1 Flag Bits.
//
#define C1_UPPER                  0x0001      // upper case
#define C1_LOWER                  0x0002      // lower case
#define C1_DIGIT                  0x0004      // decimal digits
#define C1_SPACE                  0x0008      // spacing characters
#define C1_PUNCT                  0x0010      // punctuation characters
#define C1_CNTRL                  0x0020      // control characters
#define C1_BLANK                  0x0040      // blank characters
#define C1_XDIGIT                 0x0080      // other digits
#define C1_ALPHA                  0x0100      // any linguistic character
#define C1_DEFINED                0x0200      // defined character

//
//  CType 2 Flag Bits.
//
#define C2_LEFTTORIGHT            0x0001      // left to right
#define C2_RIGHTTOLEFT            0x0002      // right to left

#define C2_EUROPENUMBER           0x0003      // European number, digit
#define C2_EUROPESEPARATOR        0x0004      // European numeric separator
#define C2_EUROPETERMINATOR       0x0005      // European numeric terminator
#define C2_ARABICNUMBER           0x0006      // Arabic number
#define C2_COMMONSEPARATOR        0x0007      // common numeric separator

#define C2_BLOCKSEPARATOR         0x0008      // block separator
#define C2_SEGMENTSEPARATOR       0x0009      // segment separator
#define C2_WHITESPACE             0x000A      // white space
#define C2_OTHERNEUTRAL           0x000B      // other neutrals

#define C2_NOTAPPLICABLE          0x0000      // no implicit directionality

//
//  CType 3 Flag Bits.
//
#define C3_NONSPACING             0x0001      // nonspacing character
#define C3_DIACRITIC              0x0002      // diacritic mark
#define C3_VOWELMARK              0x0004      // vowel mark
#define C3_SYMBOL                 0x0008      // symbols

#define C3_KATAKANA               0x0010      // katakana character
#define C3_HIRAGANA               0x0020      // hiragana character
#define C3_HALFWIDTH              0x0040      // half width character
#define C3_FULLWIDTH              0x0080      // full width character
#define C3_IDEOGRAPH              0x0100      // ideographic character
#define C3_KASHIDA                0x0200      // Arabic kashida character
#define C3_LEXICAL                0x0400      // lexical character

#define C3_ALPHA                  0x8000      // any linguistic char (C1_ALPHA)

#define C3_NOTAPPLICABLE          0x0000      // ctype 3 is not applicable


//
//  String Flags.
//
#define NORM_IGNORECASE           0x00000001  // ignore case
#define NORM_IGNORENONSPACE       0x00000002  // ignore nonspacing chars
#define NORM_IGNORESYMBOLS        0x00000004  // ignore symbols

#define NORM_IGNOREKANATYPE       0x00010000  // ignore kanatype
#define NORM_IGNOREWIDTH          0x00020000  // ignore width

#define NORM_STOP_ON_NULL         0x10000000  // stop at the null termination       ;internal_NT
                                                                                    ;internal_NT

//
//  Locale Independent Mapping Flags.
//
#define MAP_FOLDCZONE             0x00000010  // fold compatibility zone chars
#define MAP_PRECOMPOSED           0x00000020  // convert to precomposed chars
#define MAP_COMPOSITE             0x00000040  // convert to composite chars
#define MAP_FOLDDIGITS            0x00000080  // all digits to ASCII 0-9

;begin_winver_500
#define MAP_EXPAND_LIGATURES      0x00002000  // expand all ligatures
;end_winver_500


//
//  Locale Dependent Mapping Flags.
//
#define LCMAP_LOWERCASE           0x00000100  // lower case letters
#define LCMAP_UPPERCASE           0x00000200  // upper case letters
#define LCMAP_SORTKEY             0x00000400  // WC sort key (normalize)
#define LCMAP_BYTEREV             0x00000800  // byte reversal

#define LCMAP_HIRAGANA            0x00100000  // map katakana to hiragana
#define LCMAP_KATAKANA            0x00200000  // map hiragana to katakana
#define LCMAP_HALFWIDTH           0x00400000  // map double byte to single byte
#define LCMAP_FULLWIDTH           0x00800000  // map single byte to double byte

#define LCMAP_LINGUISTIC_CASING   0x01000000  // use linguistic rules for casing

#define LCMAP_SIMPLIFIED_CHINESE  0x02000000  // map traditional chinese to simplified chinese
#define LCMAP_TRADITIONAL_CHINESE 0x04000000  // map simplified chinese to traditional chinese

#define LCMAP_IGNOREDBCS          0x80000000  // don't casemap DBCS characters      ;internal_win40
                                                                                    ;internal_win40

//
//  Language Group Enumeration Flags.
//
#define LGRPID_INSTALLED          0x00000001  // installed language group ids
#define LGRPID_SUPPORTED          0x00000002  // supported language group ids


//
//  Locale Enumeration Flags.
//
#define LCID_INSTALLED            0x00000001  // installed locale ids
#define LCID_SUPPORTED            0x00000002  // supported locale ids
#define LCID_ALTERNATE_SORTS      0x00000004  // alternate sort locale ids


//
//  Code Page Enumeration Flags.
//
#define CP_INSTALLED              0x00000001  // installed code page ids
#define CP_SUPPORTED              0x00000002  // supported code page ids


//
//  Sorting Flags.
//
//    WORD Sort:    culturally correct sort
//                  hyphen and apostrophe are special cased
//                  example: "coop" and "co-op" will sort together in a list
//
//                        co_op     <-------  underscore (symbol)
//                        coat
//                        comb
//                        coop
//                        co-op     <-------  hyphen (punctuation)
//                        cork
//                        went
//                        were
//                        we're     <-------  apostrophe (punctuation)
//
//
//    STRING Sort:  hyphen and apostrophe will sort with all other symbols
//
//                        co-op     <-------  hyphen (punctuation)
//                        co_op     <-------  underscore (symbol)
//                        coat
//                        comb
//                        coop
//                        cork
//                        we're     <-------  apostrophe (punctuation)
//                        went
//                        were
//
#define SORT_STRINGSORT           0x00001000  // use string sort method


//
//  Compare String Return Values.
//
#define CSTR_LESS_THAN            1           // string 1 less than string 2
#define CSTR_EQUAL                2           // string 1 equal to string 2
#define CSTR_GREATER_THAN         3           // string 1 greater than string 2


//
//  Code Page Default Values.
//
#define CP_ACP                    0           // default to ANSI code page
#define CP_OEMCP                  1           // default to OEM  code page
#define CP_MACCP                  2           // default to MAC  code page
#define CP_THREAD_ACP             3           // current thread's ANSI code page
#define CP_SYMBOL                 42          // SYMBOL translations

#define CP_UTF7                   65000       // UTF-7 translation
#define CP_UTF8                   65001       // UTF-8 translation


//
//  Country/Region Codes.
//
#define CTRY_DEFAULT              0

#define CTRY_ALBANIA              355         // Albania
#define CTRY_ALGERIA              213         // Algeria
#define CTRY_ARGENTINA            54          // Argentina
#define CTRY_ARMENIA              374         // Armenia
#define CTRY_AUSTRALIA            61          // Australia
#define CTRY_AUSTRIA              43          // Austria
#define CTRY_AZERBAIJAN           994         // Azerbaijan
#define CTRY_BAHRAIN              973         // Bahrain
#define CTRY_BELARUS              375         // Belarus
#define CTRY_BELGIUM              32          // Belgium
#define CTRY_BELIZE               501         // Belize
#define CTRY_BOLIVIA              591         // Bolivia
#define CTRY_BRAZIL               55          // Brazil
#define CTRY_BRUNEI_DARUSSALAM    673         // Brunei Darussalam
#define CTRY_BULGARIA             359         // Bulgaria
#define CTRY_CANADA               2           // Canada
#define CTRY_CARIBBEAN            1           // Caribbean
#define CTRY_CHILE                56          // Chile
#define CTRY_COLOMBIA             57          // Colombia
#define CTRY_COSTA_RICA           506         // Costa Rica
#define CTRY_CROATIA              385         // Croatia
#define CTRY_CZECH                420         // Czech Republic
#define CTRY_DENMARK              45          // Denmark
#define CTRY_DOMINICAN_REPUBLIC   1           // Dominican Republic
#define CTRY_ECUADOR              593         // Ecuador
#define CTRY_EGYPT                20          // Egypt
#define CTRY_EL_SALVADOR          503         // El Salvador
#define CTRY_ESTONIA              372         // Estonia
#define CTRY_FAEROE_ISLANDS       298         // Faeroe Islands
#define CTRY_FINLAND              358         // Finland
#define CTRY_FRANCE               33          // France
#define CTRY_GEORGIA              995         // Georgia
#define CTRY_GERMANY              49          // Germany
#define CTRY_GREECE               30          // Greece
#define CTRY_GUATEMALA            502         // Guatemala
#define CTRY_HONDURAS             504         // Honduras
#define CTRY_HONG_KONG            852         // Hong Kong S.A.R., P.R.C.
#define CTRY_HUNGARY              36          // Hungary
#define CTRY_ICELAND              354         // Iceland
#define CTRY_INDIA                91          // India
#define CTRY_INDONESIA            62          // Indonesia
#define CTRY_IRAN                 981         // Iran
#define CTRY_IRAQ                 964         // Iraq
#define CTRY_IRELAND              353         // Ireland
#define CTRY_ISRAEL               972         // Israel
#define CTRY_ITALY                39          // Italy
#define CTRY_JAMAICA              1           // Jamaica
#define CTRY_JAPAN                81          // Japan
#define CTRY_JORDAN               962         // Jordan
#define CTRY_KAZAKSTAN            7           // Kazakstan
#define CTRY_KENYA                254         // Kenya
#define CTRY_KUWAIT               965         // Kuwait
#define CTRY_KYRGYZSTAN           996         // Kyrgyzstan
#define CTRY_LATVIA               371         // Latvia
#define CTRY_LEBANON              961         // Lebanon
#define CTRY_LIBYA                218         // Libya
#define CTRY_LIECHTENSTEIN        41          // Liechtenstein
#define CTRY_LITHUANIA            370         // Lithuania
#define CTRY_LUXEMBOURG           352         // Luxembourg
#define CTRY_MACAU                853         // Macau S.A.R., PRC
#define CTRY_MACEDONIA            389         // Former Yugoslav Republic of Macedonia
#define CTRY_MALAYSIA             60          // Malaysia
#define CTRY_MALDIVES             960         // Maldives
#define CTRY_MEXICO               52          // Mexico
#define CTRY_MONACO               33          // Principality of Monaco
#define CTRY_MONGOLIA             976         // Mongolia
#define CTRY_MOROCCO              212         // Morocco
#define CTRY_NETHERLANDS          31          // Netherlands
#define CTRY_NEW_ZEALAND          64          // New Zealand
#define CTRY_NICARAGUA            505         // Nicaragua
#define CTRY_NORWAY               47          // Norway
#define CTRY_OMAN                 968         // Oman
#define CTRY_PAKISTAN             92          // Islamic Republic of Pakistan
#define CTRY_PANAMA               507         // Panama
#define CTRY_PARAGUAY             595         // Paraguay
#define CTRY_PERU                 51          // Peru
#define CTRY_PHILIPPINES          63          // Republic of the Philippines
#define CTRY_POLAND               48          // Poland
#define CTRY_PORTUGAL             351         // Portugal
#define CTRY_PRCHINA              86          // People's Republic of China
#define CTRY_PUERTO_RICO          1           // Puerto Rico
#define CTRY_QATAR                974         // Qatar
#define CTRY_ROMANIA              40          // Romania
#define CTRY_RUSSIA               7           // Russia
#define CTRY_SAUDI_ARABIA         966         // Saudi Arabia
#define CTRY_SERBIA               381         // Serbia
#define CTRY_SINGAPORE            65          // Singapore
#define CTRY_SLOVAK               421         // Slovak Republic
#define CTRY_SLOVENIA             386         // Slovenia
#define CTRY_SOUTH_AFRICA         27          // South Africa
#define CTRY_SOUTH_KOREA          82          // Korea
#define CTRY_SPAIN                34          // Spain
#define CTRY_SWEDEN               46          // Sweden
#define CTRY_SWITZERLAND          41          // Switzerland
#define CTRY_SYRIA                963         // Syria
#define CTRY_TAIWAN               886         // Taiwan
#define CTRY_TATARSTAN            7           // Tatarstan
#define CTRY_THAILAND             66          // Thailand
#define CTRY_TRINIDAD_Y_TOBAGO    1           // Trinidad y Tobago
#define CTRY_TUNISIA              216         // Tunisia
#define CTRY_TURKEY               90          // Turkey
#define CTRY_UAE                  971         // U.A.E.
#define CTRY_UKRAINE              380         // Ukraine
#define CTRY_UNITED_KINGDOM       44          // United Kingdom
#define CTRY_UNITED_STATES        1           // United States
#define CTRY_URUGUAY              598         // Uruguay
#define CTRY_UZBEKISTAN           7           // Uzbekistan
#define CTRY_VENEZUELA            58          // Venezuela
#define CTRY_VIET_NAM             84          // Viet Nam
#define CTRY_YEMEN                967         // Yemen
#define CTRY_ZIMBABWE             263         // Zimbabwe


//
//  Locale Types.
//
//  These types are used for the GetLocaleInfo NLS API routine.
//  Some of these types are also used for the SetLocaleInfo NLS API routine.
//

//
//  The following LCTypes may be used in combination with any other LCTypes.
//
//    LOCALE_NOUSEROVERRIDE is also used in GetTimeFormat and
//    GetDateFormat.
//
//    LOCALE_USE_CP_ACP is used in many of the A (Ansi) apis that need
//    to do string translation.
//
//    LOCALE_RETURN_NUMBER will return the result from GetLocaleInfo as a
//    number instead of a string.  This flag is only valid for the LCTypes
//    beginning with LOCALE_I.
//
#define LOCALE_NOUSEROVERRIDE         0x80000000   // do not use user overrides
#define LOCALE_USE_CP_ACP             0x40000000   // use the system ACP

;begin_winver_400
#define LOCALE_RETURN_NUMBER          0x20000000   // return number instead of string
;end_winver_400

//
//  The following LCTypes are mutually exclusive in that they may NOT
//  be used in combination with each other.
//
#define LOCALE_ILANGUAGE              0x00000001   // language id
#define LOCALE_SLANGUAGE              0x00000002   // localized name of language
#define LOCALE_SENGLANGUAGE           0x00001001   // English name of language
#define LOCALE_SABBREVLANGNAME        0x00000003   // abbreviated language name
#define LOCALE_SNATIVELANGNAME        0x00000004   // native name of language

#define LOCALE_ICOUNTRY               0x00000005   // country code
#define LOCALE_SCOUNTRY               0x00000006   // localized name of country
#define LOCALE_SENGCOUNTRY            0x00001002   // English name of country
#define LOCALE_SABBREVCTRYNAME        0x00000007   // abbreviated country name
#define LOCALE_SNATIVECTRYNAME        0x00000008   // native name of country
#define LOCALE_IGEOID                 0x0000005B   // geographical location id  ;internal_NT

#define LOCALE_IDEFAULTLANGUAGE       0x00000009   // default language id
#define LOCALE_IDEFAULTCOUNTRY        0x0000000A   // default country code
#define LOCALE_IDEFAULTCODEPAGE       0x0000000B   // default oem code page
#define LOCALE_IDEFAULTANSICODEPAGE   0x00001004   // default ansi code page
#define LOCALE_IDEFAULTMACCODEPAGE    0x00001011   // default mac code page

#define LOCALE_SLIST                  0x0000000C   // list item separator
#define LOCALE_IMEASURE               0x0000000D   // 0 = metric, 1 = US

#define LOCALE_SDECIMAL               0x0000000E   // decimal separator
#define LOCALE_STHOUSAND              0x0000000F   // thousand separator
#define LOCALE_SGROUPING              0x00000010   // digit grouping
#define LOCALE_IDIGITS                0x00000011   // number of fractional digits
#define LOCALE_ILZERO                 0x00000012   // leading zeros for decimal
#define LOCALE_INEGNUMBER             0x00001010   // negative number mode
#define LOCALE_SNATIVEDIGITS          0x00000013   // native ascii 0-9

#define LOCALE_SCURRENCY              0x00000014   // local monetary symbol
#define LOCALE_SINTLSYMBOL            0x00000015   // intl monetary symbol
#define LOCALE_SMONDECIMALSEP         0x00000016   // monetary decimal separator
#define LOCALE_SMONTHOUSANDSEP        0x00000017   // monetary thousand separator
#define LOCALE_SMONGROUPING           0x00000018   // monetary grouping
#define LOCALE_ICURRDIGITS            0x00000019   // # local monetary digits
#define LOCALE_IINTLCURRDIGITS        0x0000001A   // # intl monetary digits
#define LOCALE_ICURRENCY              0x0000001B   // positive currency mode
#define LOCALE_INEGCURR               0x0000001C   // negative currency mode

#define LOCALE_SDATE                  0x0000001D   // date separator
#define LOCALE_STIME                  0x0000001E   // time separator
#define LOCALE_SSHORTDATE             0x0000001F   // short date format string
#define LOCALE_SLONGDATE              0x00000020   // long date format string
#define LOCALE_STIMEFORMAT            0x00001003   // time format string
#define LOCALE_IDATE                  0x00000021   // short date format ordering
#define LOCALE_ILDATE                 0x00000022   // long date format ordering
#define LOCALE_ITIME                  0x00000023   // time format specifier
#define LOCALE_ITIMEMARKPOSN          0x00001005   // time marker position
#define LOCALE_ICENTURY               0x00000024   // century format specifier (short date)
#define LOCALE_ITLZERO                0x00000025   // leading zeros in time field
#define LOCALE_IDAYLZERO              0x00000026   // leading zeros in day field (short date)
#define LOCALE_IMONLZERO              0x00000027   // leading zeros in month field (short date)
#define LOCALE_S1159                  0x00000028   // AM designator
#define LOCALE_S2359                  0x00000029   // PM designator

#define LOCALE_ICALENDARTYPE          0x00001009   // type of calendar specifier
#define LOCALE_IOPTIONALCALENDAR      0x0000100B   // additional calendar types specifier
#define LOCALE_IFIRSTDAYOFWEEK        0x0000100C   // first day of week specifier
#define LOCALE_IFIRSTWEEKOFYEAR       0x0000100D   // first week of year specifier

#define LOCALE_SDAYNAME1              0x0000002A   // long name for Monday
#define LOCALE_SDAYNAME2              0x0000002B   // long name for Tuesday
#define LOCALE_SDAYNAME3              0x0000002C   // long name for Wednesday
#define LOCALE_SDAYNAME4              0x0000002D   // long name for Thursday
#define LOCALE_SDAYNAME5              0x0000002E   // long name for Friday
#define LOCALE_SDAYNAME6              0x0000002F   // long name for Saturday
#define LOCALE_SDAYNAME7              0x00000030   // long name for Sunday
#define LOCALE_SABBREVDAYNAME1        0x00000031   // abbreviated name for Monday
#define LOCALE_SABBREVDAYNAME2        0x00000032   // abbreviated name for Tuesday
#define LOCALE_SABBREVDAYNAME3        0x00000033   // abbreviated name for Wednesday
#define LOCALE_SABBREVDAYNAME4        0x00000034   // abbreviated name for Thursday
#define LOCALE_SABBREVDAYNAME5        0x00000035   // abbreviated name for Friday
#define LOCALE_SABBREVDAYNAME6        0x00000036   // abbreviated name for Saturday
#define LOCALE_SABBREVDAYNAME7        0x00000037   // abbreviated name for Sunday
#define LOCALE_SMONTHNAME1            0x00000038   // long name for January
#define LOCALE_SMONTHNAME2            0x00000039   // long name for February
#define LOCALE_SMONTHNAME3            0x0000003A   // long name for March
#define LOCALE_SMONTHNAME4            0x0000003B   // long name for April
#define LOCALE_SMONTHNAME5            0x0000003C   // long name for May
#define LOCALE_SMONTHNAME6            0x0000003D   // long name for June
#define LOCALE_SMONTHNAME7            0x0000003E   // long name for July
#define LOCALE_SMONTHNAME8            0x0000003F   // long name for August
#define LOCALE_SMONTHNAME9            0x00000040   // long name for September
#define LOCALE_SMONTHNAME10           0x00000041   // long name for October
#define LOCALE_SMONTHNAME11           0x00000042   // long name for November
#define LOCALE_SMONTHNAME12           0x00000043   // long name for December
#define LOCALE_SMONTHNAME13           0x0000100E   // long name for 13th month (if exists)
#define LOCALE_SABBREVMONTHNAME1      0x00000044   // abbreviated name for January
#define LOCALE_SABBREVMONTHNAME2      0x00000045   // abbreviated name for February
#define LOCALE_SABBREVMONTHNAME3      0x00000046   // abbreviated name for March
#define LOCALE_SABBREVMONTHNAME4      0x00000047   // abbreviated name for April
#define LOCALE_SABBREVMONTHNAME5      0x00000048   // abbreviated name for May
#define LOCALE_SABBREVMONTHNAME6      0x00000049   // abbreviated name for June
#define LOCALE_SABBREVMONTHNAME7      0x0000004A   // abbreviated name for July
#define LOCALE_SABBREVMONTHNAME8      0x0000004B   // abbreviated name for August
#define LOCALE_SABBREVMONTHNAME9      0x0000004C   // abbreviated name for September
#define LOCALE_SABBREVMONTHNAME10     0x0000004D   // abbreviated name for October
#define LOCALE_SABBREVMONTHNAME11     0x0000004E   // abbreviated name for November
#define LOCALE_SABBREVMONTHNAME12     0x0000004F   // abbreviated name for December
#define LOCALE_SABBREVMONTHNAME13     0x0000100F   // abbreviated name for 13th month (if exists)

#define LOCALE_SPOSITIVESIGN          0x00000050   // positive sign
#define LOCALE_SNEGATIVESIGN          0x00000051   // negative sign
#define LOCALE_IPOSSIGNPOSN           0x00000052   // positive sign position
#define LOCALE_INEGSIGNPOSN           0x00000053   // negative sign position
#define LOCALE_IPOSSYMPRECEDES        0x00000054   // mon sym precedes pos amt
#define LOCALE_IPOSSEPBYSPACE         0x00000055   // mon sym sep by space from pos amt
#define LOCALE_INEGSYMPRECEDES        0x00000056   // mon sym precedes neg amt
#define LOCALE_INEGSEPBYSPACE         0x00000057   // mon sym sep by space from neg amt

;begin_winver_400
#define LOCALE_FONTSIGNATURE          0x00000058   // font signature
#define LOCALE_SISO639LANGNAME        0x00000059   // ISO abbreviated language name
#define LOCALE_SISO3166CTRYNAME       0x0000005A   // ISO abbreviated country name
;end_winver_400

;begin_winver_500
#define LOCALE_IDEFAULTEBCDICCODEPAGE 0x00001012   // default ebcdic code page
#define LOCALE_IPAPERSIZE             0x0000100A   // 1 = letter, 5 = legal, 8 = a3, 9 = a4
#define LOCALE_SENGCURRNAME           0x00001007   // english name of currency
#define LOCALE_SNATIVECURRNAME        0x00001008   // native name of currency
#define LOCALE_SYEARMONTH             0x00001006   // year month format string
#define LOCALE_SSORTNAME              0x00001013   // sort name
#define LOCALE_IDIGITSUBSTITUTION     0x00001014   // 0 = context, 1 = none, 2 = national

//                                                 ;internal_NT
// LCType to represent the registry locale value   ;internal_NT
//                                                 ;internal_NT
#define LOCALE_SLOCALE                (-1)         ;internal_NT
;end_winver_500


//
//  Time Flags for GetTimeFormat.
//
#define TIME_NOMINUTESORSECONDS   0x00000001  // do not use minutes or seconds
#define TIME_NOSECONDS            0x00000002  // do not use seconds
#define TIME_NOTIMEMARKER         0x00000004  // do not use time marker
#define TIME_FORCE24HOURFORMAT    0x00000008  // always use 24 hour format


//
//  Date Flags for GetDateFormat.
//
#define DATE_SHORTDATE            0x00000001  // use short date picture
#define DATE_LONGDATE             0x00000002  // use long date picture
#define DATE_USE_ALT_CALENDAR     0x00000004  // use alternate calendar (if any)

;begin_winver_500
#define DATE_YEARMONTH            0x00000008  // use year month picture
#define DATE_LTRREADING           0x00000010  // add marks for left to right reading order layout
#define DATE_RTLREADING           0x00000020  // add marks for right to left reading order layout
#define DATE_ADDHIJRIDATETEMP     0x80000000  // use AddHijriDateTemp reg value  ;internal_NT
;end_winver_500


//
//  Calendar Types.
//
//  These types are used for the EnumCalendarInfo and GetCalendarInfo
//  NLS API routines.
//  Some of these types are also used for the SetCalendarInfo NLS API
//  routine.
//

//
//  The following CalTypes may be used in combination with any other CalTypes.
//
//    CAL_NOUSEROVERRIDE
//
//    CAL_USE_CP_ACP is used in the A (Ansi) apis that need to do string
//    translation.
//
//    CAL_RETURN_NUMBER will return the result from GetCalendarInfo as a
//    number instead of a string.  This flag is only valid for the CalTypes
//    beginning with CAL_I.
//
;begin_winver_500
#define CAL_NOUSEROVERRIDE        LOCALE_NOUSEROVERRIDE  // do not use user overrides
#define CAL_USE_CP_ACP            LOCALE_USE_CP_ACP      // use the system ACP
#define CAL_RETURN_NUMBER         LOCALE_RETURN_NUMBER   // return number instead of string
;end_winver_400

//
//  The following CalTypes are mutually exclusive in that they may NOT
//  be used in combination with each other.
//
#define CAL_ICALINTVALUE          0x00000001  // calendar type
#define CAL_SCALNAME              0x00000002  // native name of calendar
#define CAL_IYEAROFFSETRANGE      0x00000003  // starting years of eras
#define CAL_SERASTRING            0x00000004  // era name for IYearOffsetRanges
#define CAL_SSHORTDATE            0x00000005  // short date format string
#define CAL_SLONGDATE             0x00000006  // long date format string
#define CAL_SDAYNAME1             0x00000007  // native name for Monday
#define CAL_SDAYNAME2             0x00000008  // native name for Tuesday
#define CAL_SDAYNAME3             0x00000009  // native name for Wednesday
#define CAL_SDAYNAME4             0x0000000a  // native name for Thursday
#define CAL_SDAYNAME5             0x0000000b  // native name for Friday
#define CAL_SDAYNAME6             0x0000000c  // native name for Saturday
#define CAL_SDAYNAME7             0x0000000d  // native name for Sunday
#define CAL_SABBREVDAYNAME1       0x0000000e  // abbreviated name for Monday
#define CAL_SABBREVDAYNAME2       0x0000000f  // abbreviated name for Tuesday
#define CAL_SABBREVDAYNAME3       0x00000010  // abbreviated name for Wednesday
#define CAL_SABBREVDAYNAME4       0x00000011  // abbreviated name for Thursday
#define CAL_SABBREVDAYNAME5       0x00000012  // abbreviated name for Friday
#define CAL_SABBREVDAYNAME6       0x00000013  // abbreviated name for Saturday
#define CAL_SABBREVDAYNAME7       0x00000014  // abbreviated name for Sunday
#define CAL_SMONTHNAME1           0x00000015  // native name for January
#define CAL_SMONTHNAME2           0x00000016  // native name for February
#define CAL_SMONTHNAME3           0x00000017  // native name for March
#define CAL_SMONTHNAME4           0x00000018  // native name for April
#define CAL_SMONTHNAME5           0x00000019  // native name for May
#define CAL_SMONTHNAME6           0x0000001a  // native name for June
#define CAL_SMONTHNAME7           0x0000001b  // native name for July
#define CAL_SMONTHNAME8           0x0000001c  // native name for August
#define CAL_SMONTHNAME9           0x0000001d  // native name for September
#define CAL_SMONTHNAME10          0x0000001e  // native name for October
#define CAL_SMONTHNAME11          0x0000001f  // native name for November
#define CAL_SMONTHNAME12          0x00000020  // native name for December
#define CAL_SMONTHNAME13          0x00000021  // native name for 13th month (if any)
#define CAL_SABBREVMONTHNAME1     0x00000022  // abbreviated name for January
#define CAL_SABBREVMONTHNAME2     0x00000023  // abbreviated name for February
#define CAL_SABBREVMONTHNAME3     0x00000024  // abbreviated name for March
#define CAL_SABBREVMONTHNAME4     0x00000025  // abbreviated name for April
#define CAL_SABBREVMONTHNAME5     0x00000026  // abbreviated name for May
#define CAL_SABBREVMONTHNAME6     0x00000027  // abbreviated name for June
#define CAL_SABBREVMONTHNAME7     0x00000028  // abbreviated name for July
#define CAL_SABBREVMONTHNAME8     0x00000029  // abbreviated name for August
#define CAL_SABBREVMONTHNAME9     0x0000002a  // abbreviated name for September
#define CAL_SABBREVMONTHNAME10    0x0000002b  // abbreviated name for October
#define CAL_SABBREVMONTHNAME11    0x0000002c  // abbreviated name for November
#define CAL_SABBREVMONTHNAME12    0x0000002d  // abbreviated name for December
#define CAL_SABBREVMONTHNAME13    0x0000002e  // abbreviated name for 13th month (if any)

;begin_winver_500
#define CAL_SYEARMONTH            0x0000002f  // year month format string
#define CAL_ITWODIGITYEARMAX      0x00000030  // two digit year max
;end_winver_500

//
//  Calendar Enumeration Value.
//
#define ENUM_ALL_CALENDARS        0xffffffff  // enumerate all calendars


//
//  Calendar ID Values.
//
#define CAL_GREGORIAN                  1      // Gregorian (localized) calendar
#define CAL_GREGORIAN_US               2      // Gregorian (U.S.) calendar
#define CAL_JAPAN                      3      // Japanese Emperor Era calendar
#define CAL_TAIWAN                     4      // Taiwan calendar
#define CAL_KOREA                      5      // Korean Tangun Era calendar
#define CAL_HIJRI                      6      // Hijri (Arabic Lunar) calendar
#define CAL_THAI                       7      // Thai calendar
#define CAL_HEBREW                     8      // Hebrew (Lunar) calendar
#define CAL_GREGORIAN_ME_FRENCH        9      // Gregorian Middle East French calendar
#define CAL_GREGORIAN_ARABIC           10     // Gregorian Arabic calendar
#define CAL_GREGORIAN_XLIT_ENGLISH     11     // Gregorian Transliterated English calendar
#define CAL_GREGORIAN_XLIT_FRENCH      12     // Gregorian Transliterated French calendar


//
//  Language Group ID Values.
//
#define LGRPID_WESTERN_EUROPE        0x0001   // Western Europe & U.S.
#define LGRPID_CENTRAL_EUROPE        0x0002   // Central Europe
#define LGRPID_BALTIC                0x0003   // Baltic
#define LGRPID_GREEK                 0x0004   // Greek
#define LGRPID_CYRILLIC              0x0005   // Cyrillic
#define LGRPID_TURKISH               0x0006   // Turkish
#define LGRPID_JAPANESE              0x0007   // Japanese
#define LGRPID_KOREAN                0x0008   // Korean
#define LGRPID_TRADITIONAL_CHINESE   0x0009   // Traditional Chinese
#define LGRPID_SIMPLIFIED_CHINESE    0x000a   // Simplified Chinese
#define LGRPID_THAI                  0x000b   // Thai
#define LGRPID_HEBREW                0x000c   // Hebrew
#define LGRPID_ARABIC                0x000d   // Arabic
#define LGRPID_VIETNAMESE            0x000e   // Vietnamese
#define LGRPID_INDIC                 0x000f   // Indic
#define LGRPID_GEORGIAN              0x0010   // Georgian
#define LGRPID_ARMENIAN              0x0011   // Armenian




////////////////////////////////////////////////////////////////////////////
//
//  Typedefs
//
//  Define all types for the NLS component here.
//
////////////////////////////////////////////////////////////////////////////

//
//  Language Group ID.
//
typedef DWORD LGRPID;


//
//  Locale type constant.
//
typedef DWORD LCTYPE;


//
//  Calendar type constant.
//
typedef DWORD CALTYPE;


//
//  Calendar ID.
//
typedef DWORD CALID;


//
//  CP Info.
//

typedef struct _cpinfo {
    UINT    MaxCharSize;                    // max length (in bytes) of a char
    BYTE    DefaultChar[MAX_DEFAULTCHAR];   // default character
    BYTE    LeadByte[MAX_LEADBYTES];        // lead byte ranges
} CPINFO, *LPCPINFO;

typedef struct _cpinfoex% {
    UINT    MaxCharSize;                    // max length (in bytes) of a char
    BYTE    DefaultChar[MAX_DEFAULTCHAR];   // default character (MB)
    BYTE    LeadByte[MAX_LEADBYTES];        // lead byte ranges
    WCHAR   UnicodeDefaultChar;             // default character (Unicode)
    UINT    CodePage;                       // code page id
    TCHAR%  CodePageName[MAX_PATH];         // code page name (Unicode)
} CPINFOEX%, *LPCPINFOEX%;


//
//  Number format.
//

typedef struct _numberfmt% {
    UINT    NumDigits;                 // number of decimal digits
    UINT    LeadingZero;               // if leading zero in decimal fields
    UINT    Grouping;                  // group size left of decimal
    LPTSTR% lpDecimalSep;              // ptr to decimal separator string
    LPTSTR% lpThousandSep;             // ptr to thousand separator string
    UINT    NegativeOrder;             // negative number ordering
} NUMBERFMT%, *LPNUMBERFMT%;


//
//  Currency format.
//

typedef struct _currencyfmt% {
    UINT    NumDigits;                 // number of decimal digits
    UINT    LeadingZero;               // if leading zero in decimal fields
    UINT    Grouping;                  // group size left of decimal
    LPTSTR% lpDecimalSep;              // ptr to decimal separator string
    LPTSTR% lpThousandSep;             // ptr to thousand separator string
    UINT    NegativeOrder;             // negative currency ordering
    UINT    PositiveOrder;             // positive currency ordering
    LPTSTR% lpCurrencySymbol;          // ptr to currency symbol string
} CURRENCYFMT%, *LPCURRENCYFMT%;

//
//  NLS function capabilities
//

enum SYSNLS_FUNCTION{
    COMPARE_STRING    =  0x0001,
};
typedef DWORD NLS_FUNCTION;


//
//  NLS version structure.
//

typedef struct _nlsversioninfo{ 
    DWORD dwNLSVersionInfoSize; 
    DWORD dwNLSVersion; 
    DWORD dwDefinedVersion; 
} NLSVERSIONINFO, *LPNLSVERSIONINFO; 


//
//  GEO defines
//

typedef LONG    GEOID;
typedef DWORD   GEOTYPE;
typedef DWORD   GEOCLASS;

#define GEOID_NOT_AVAILABLE -1

//
//  GEO information types for clients to query
//

enum SYSGEOTYPE {
    GEO_NATION      =       0x0001,
    GEO_LATITUDE    =       0x0002,
    GEO_LONGITUDE   =       0x0003,
    GEO_ISO2        =       0x0004,
    GEO_ISO3        =       0x0005,
    GEO_RFC1766     =       0x0006,
    GEO_LCID        =       0x0007,
    GEO_FRIENDLYNAME=       0x0008,
    GEO_OFFICIALNAME=       0x0009,
    GEO_TIMEZONES   =       0x000A,
    GEO_OFFICIALLANGUAGES = 0x000B,
};

//
//  More GEOCLASS defines will be listed here
//

enum SYSGEOCLASS {
    GEOCLASS_NATION  = 16,
    GEOCLASS_REGION  = 14,
};


//
//  Enumeration function constants.
//

#ifdef STRICT

typedef BOOL (CALLBACK* LANGUAGEGROUP_ENUMPROCA)(LGRPID, LPSTR, LPSTR, DWORD, LONG_PTR);
typedef BOOL (CALLBACK* LANGGROUPLOCALE_ENUMPROCA)(LGRPID, LCID, LPSTR, LONG_PTR);
typedef BOOL (CALLBACK* UILANGUAGE_ENUMPROCA)(LPSTR, LONG_PTR);
typedef BOOL (CALLBACK* LOCALE_ENUMPROCA)(LPSTR);
typedef BOOL (CALLBACK* CODEPAGE_ENUMPROCA)(LPSTR);
typedef BOOL (CALLBACK* DATEFMT_ENUMPROCA)(LPSTR);
typedef BOOL (CALLBACK* DATEFMT_ENUMPROCEXA)(LPSTR, CALID);
typedef BOOL (CALLBACK* TIMEFMT_ENUMPROCA)(LPSTR);
typedef BOOL (CALLBACK* CALINFO_ENUMPROCA)(LPSTR);
typedef BOOL (CALLBACK* CALINFO_ENUMPROCEXA)(LPSTR, CALID);

typedef BOOL (CALLBACK* LANGUAGEGROUP_ENUMPROCW)(LGRPID, LPWSTR, LPWSTR, DWORD, LONG_PTR);
typedef BOOL (CALLBACK* LANGGROUPLOCALE_ENUMPROCW)(LGRPID, LCID, LPWSTR, LONG_PTR);
typedef BOOL (CALLBACK* UILANGUAGE_ENUMPROCW)(LPWSTR, LONG_PTR);
typedef BOOL (CALLBACK* LOCALE_ENUMPROCW)(LPWSTR);
typedef BOOL (CALLBACK* CODEPAGE_ENUMPROCW)(LPWSTR);
typedef BOOL (CALLBACK* DATEFMT_ENUMPROCW)(LPWSTR);
typedef BOOL (CALLBACK* DATEFMT_ENUMPROCEXW)(LPWSTR, CALID);
typedef BOOL (CALLBACK* TIMEFMT_ENUMPROCW)(LPWSTR);
typedef BOOL (CALLBACK* CALINFO_ENUMPROCW)(LPWSTR);
typedef BOOL (CALLBACK* CALINFO_ENUMPROCEXW)(LPWSTR, CALID);
typedef BOOL (CALLBACK* GEO_ENUMPROC)(GEOID);

#else // !STRICT

typedef FARPROC LANGUAGEGROUP_ENUMPROCA;
typedef FARPROC LANGGROUPLOCALE_ENUMPROCA;
typedef FARPROC UILANGUAGE_ENUMPROCA;
typedef FARPROC LOCALE_ENUMPROCA;
typedef FARPROC CODEPAGE_ENUMPROCA;
typedef FARPROC DATEFMT_ENUMPROCA;
typedef FARPROC DATEFMT_ENUMPROCEXA;
typedef FARPROC TIMEFMT_ENUMPROCA;
typedef FARPROC CALINFO_ENUMPROCA;
typedef FARPROC CALINFO_ENUMPROCEXA;
typedef FARPROC GEO_ENUMPROC;

typedef FARPROC LANGUAGEGROUP_ENUMPROCW;
typedef FARPROC LANGGROUPLOCALE_ENUMPROCW;
typedef FARPROC UILANGUAGE_ENUMPROCW;
typedef FARPROC LOCALE_ENUMPROCW;
typedef FARPROC CODEPAGE_ENUMPROCW;
typedef FARPROC DATEFMT_ENUMPROCW;
typedef FARPROC DATEFMT_ENUMPROCEXW;
typedef FARPROC TIMEFMT_ENUMPROCW;
typedef FARPROC CALINFO_ENUMPROCW;
typedef FARPROC CALINFO_ENUMPROCEXW;

#endif // !STRICT

#ifdef UNICODE

#define LANGUAGEGROUP_ENUMPROC    LANGUAGEGROUP_ENUMPROCW
#define LANGGROUPLOCALE_ENUMPROC  LANGGROUPLOCALE_ENUMPROCW
#define UILANGUAGE_ENUMPROC       UILANGUAGE_ENUMPROCW
#define LOCALE_ENUMPROC           LOCALE_ENUMPROCW
#define CODEPAGE_ENUMPROC         CODEPAGE_ENUMPROCW
#define DATEFMT_ENUMPROC          DATEFMT_ENUMPROCW
#define DATEFMT_ENUMPROCEX        DATEFMT_ENUMPROCEXW
#define TIMEFMT_ENUMPROC          TIMEFMT_ENUMPROCW
#define CALINFO_ENUMPROC          CALINFO_ENUMPROCW
#define CALINFO_ENUMPROCEX        CALINFO_ENUMPROCEXW

#else

#define LANGUAGEGROUP_ENUMPROC    LANGUAGEGROUP_ENUMPROCA
#define LANGGROUPLOCALE_ENUMPROC  LANGGROUPLOCALE_ENUMPROCA
#define UILANGUAGE_ENUMPROC       UILANGUAGE_ENUMPROCA
#define LOCALE_ENUMPROC           LOCALE_ENUMPROCA
#define CODEPAGE_ENUMPROC         CODEPAGE_ENUMPROCA
#define DATEFMT_ENUMPROC          DATEFMT_ENUMPROCA
#define DATEFMT_ENUMPROCEX        DATEFMT_ENUMPROCEXA
#define TIMEFMT_ENUMPROC          TIMEFMT_ENUMPROCA
#define CALINFO_ENUMPROC          CALINFO_ENUMPROCA
#define CALINFO_ENUMPROCEX        CALINFO_ENUMPROCEXA

#endif // !UNICODE




////////////////////////////////////////////////////////////////////////////
//
//  Macros
//
//  Define all macros for the NLS component here.
//
////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////
//
//  Function Prototypes
//
//  Only prototypes for the NLS APIs should go here.
//
////////////////////////////////////////////////////////////////////////////

//
//  Code Page Dependent APIs.
//

WINBASEAPI
BOOL
WINAPI
IsValidCodePage(
    IN UINT  CodePage);

WINBASEAPI
UINT
WINAPI
GetACP(void);

WINBASEAPI
UINT
WINAPI
GetOEMCP(void);

WINBASEAPI
BOOL
WINAPI
GetCPInfo(
    IN UINT       CodePage,
    OUT LPCPINFO  lpCPInfo);

WINBASEAPI
BOOL
WINAPI
GetCPInfoEx%(
    IN UINT          CodePage,
    IN DWORD         dwFlags,
    OUT LPCPINFOEX%  lpCPInfoEx);

WINBASEAPI
BOOL
WINAPI
IsDBCSLeadByte(
    IN BYTE  TestChar);

WINBASEAPI
BOOL
WINAPI
IsDBCSLeadByteEx(
    IN UINT  CodePage,
    IN BYTE  TestChar);

WINBASEAPI
int
WINAPI
MultiByteToWideChar(
    IN UINT     CodePage,
    IN DWORD    dwFlags,
    IN LPCSTR   lpMultiByteStr,
    IN int      cbMultiByte,
    OUT LPWSTR  lpWideCharStr,
    IN int      cchWideChar);

WINBASEAPI
int
WINAPI
WideCharToMultiByte(
    IN UINT     CodePage,
    IN DWORD    dwFlags,
    IN LPCWSTR  lpWideCharStr,
    IN int      cchWideChar,
    OUT LPSTR   lpMultiByteStr,
    IN int      cbMultiByte,
    IN LPCSTR   lpDefaultChar,
    OUT LPBOOL  lpUsedDefaultChar);


//
//  Locale Dependent APIs.
//

WINBASEAPI
int
WINAPI
CompareString%(
    IN LCID     Locale,
    IN DWORD    dwCmpFlags,
    IN LPCTSTR%  lpString1,
    IN int      cchCount1,
    IN LPCTSTR%  lpString2,
    IN int      cchCount2);

WINBASEAPI
int
WINAPI
LCMapString%(
    IN LCID     Locale,
    IN DWORD    dwMapFlags,
    IN LPCTSTR%  lpSrcStr,
    IN int      cchSrc,
    OUT LPTSTR%  lpDestStr,
    IN int      cchDest);

WINBASEAPI
int
WINAPI
GetLocaleInfo%(
    IN LCID     Locale,
    IN LCTYPE   LCType,
    OUT LPTSTR%  lpLCData,
    IN int      cchData);

WINBASEAPI
BOOL
WINAPI
SetLocaleInfo%(
    IN LCID     Locale,
    IN LCTYPE   LCType,
    IN LPCTSTR%  lpLCData);

#if (WINVER >= 0x040A)
WINBASEAPI
int
WINAPI
GetCalendarInfo%(
    LCID     Locale,
    CALID    Calendar,
    CALTYPE  CalType,
    LPTSTR%   lpCalData,
    int      cchData,
    LPDWORD  lpValue);

WINBASEAPI
BOOL
WINAPI
SetCalendarInfo%(
    LCID     Locale,
    CALID    Calendar,
    CALTYPE  CalType,
    LPCTSTR%  lpCalData);
#endif

WINBASEAPI
int
WINAPI
GetTimeFormat%(
    IN LCID             Locale,
    IN DWORD            dwFlags,
    IN CONST SYSTEMTIME *lpTime,
    IN LPCTSTR%          lpFormat,
    OUT LPTSTR%          lpTimeStr,
    IN int              cchTime);

WINBASEAPI
int
WINAPI
GetDateFormat%(
    IN LCID             Locale,
    IN DWORD            dwFlags,
    IN CONST SYSTEMTIME *lpDate,
    IN LPCTSTR%          lpFormat,
    OUT LPTSTR%          lpDateStr,
    IN int              cchDate);

WINBASEAPI
int
WINAPI
GetNumberFormat%(
    IN LCID             Locale,
    IN DWORD            dwFlags,
    IN LPCTSTR%          lpValue,
    IN CONST NUMBERFMT% *lpFormat,
    OUT LPTSTR%          lpNumberStr,
    IN int              cchNumber);

WINBASEAPI
int
WINAPI
GetCurrencyFormat%(
    IN LCID               Locale,
    IN DWORD              dwFlags,
    IN LPCTSTR%            lpValue,
    IN CONST CURRENCYFMT% *lpFormat,
    OUT LPTSTR%            lpCurrencyStr,
    IN int                cchCurrency);

WINBASEAPI
BOOL
WINAPI
EnumCalendarInfo%(
    IN CALINFO_ENUMPROC% lpCalInfoEnumProc,
    IN LCID              Locale,
    IN CALID             Calendar,
    IN CALTYPE           CalType);

;begin_winver_500
WINBASEAPI
BOOL
WINAPI
EnumCalendarInfoEx%(
    IN CALINFO_ENUMPROCEX% lpCalInfoEnumProcEx,
    IN LCID                Locale,
    IN CALID               Calendar,
    IN CALTYPE             CalType);
;end_winver_500

WINBASEAPI
BOOL
WINAPI
EnumTimeFormats%(
    IN TIMEFMT_ENUMPROC% lpTimeFmtEnumProc,
    IN LCID              Locale,
    IN DWORD             dwFlags);

WINBASEAPI
BOOL
WINAPI
EnumDateFormats%(
    IN DATEFMT_ENUMPROC% lpDateFmtEnumProc,
    IN LCID              Locale,
    IN DWORD             dwFlags);

;begin_winver_500
WINBASEAPI
BOOL
WINAPI
EnumDateFormatsEx%(
    IN DATEFMT_ENUMPROCEX% lpDateFmtEnumProcEx,
    IN LCID                Locale,
    IN DWORD               dwFlags);
;end_winver_500

;begin_winver_500
WINBASEAPI
BOOL
WINAPI
IsValidLanguageGroup(
    IN LGRPID  LanguageGroup,
    IN DWORD   dwFlags);
;end_winver_500

WINBASEAPI
BOOL
WINAPI
GetNLSVersion(
    IN  NLS_FUNCTION     Function,
    IN  LCID             Locale,
    OUT LPNLSVERSIONINFO lpVersionInformation);

WINBASEAPI
BOOL
WINAPI
IsNLSDefinedString(
    IN NLS_FUNCTION     Function,
    IN DWORD            dwFlags,
    IN LPNLSVERSIONINFO lpVersionInformation,
    IN LPCWSTR          lpString,
    IN INT              cchStr);

WINBASEAPI
BOOL
WINAPI
IsValidLocale(
    IN LCID   Locale,
    IN DWORD  dwFlags);

WINBASEAPI
int
WINAPI
GetGeoInfo%(
    GEOID       Location,
    GEOTYPE     GeoType,
    LPTSTR%     lpGeoData,
    int         cchData,
    LANGID      LangId);

WINBASEAPI
BOOL
WINAPI
EnumSystemGeoID(
    GEOCLASS        GeoClass,
    GEOID           ParentGeoId,
    GEO_ENUMPROC    lpGeoEnumProc);

WINBASEAPI
GEOID
WINAPI
GetUserGeoID(
    GEOCLASS    GeoClass);

WINBASEAPI
BOOL
WINAPI
SetUserGeoID(
    GEOID       GeoId);

WINBASEAPI
LCID
WINAPI
ConvertDefaultLocale(
    IN OUT LCID   Locale);

WINBASEAPI
LCID
WINAPI
GetThreadLocale(void);

WINBASEAPI
BOOL
WINAPI
SetThreadLocale(
    IN LCID  Locale
    );

;begin_winver_500
WINBASEAPI
LANGID
WINAPI
GetSystemDefaultUILanguage(void);

WINBASEAPI
LANGID
WINAPI
GetUserDefaultUILanguage(void);
;end_winver_500

WINBASEAPI
LANGID
WINAPI
GetSystemDefaultLangID(void);

WINBASEAPI
LANGID
WINAPI
GetUserDefaultLangID(void);

WINBASEAPI
LCID
WINAPI
GetSystemDefaultLCID(void);

WINBASEAPI
LCID
WINAPI
GetUserDefaultLCID(void);


WINBASEAPI                                                      ;internal_win40
BOOL                                                            ;internal_win40
WINAPI                                                          ;internal_win40
InvalidateNLSCache(void);                                       ;internal_win40
                                                                ;internal_win40

//                                                              ;internal_NT
//  This private API is only called by the Complex Script       ;internal_NT
//  Language Pack (CSLPK).                                      ;internal_NT
//                                                              ;internal_NT
ULONG                                                           ;internal_NT
WINAPI NlsGetCacheUpdateCount(void);                            ;internal_NT
                                                                ;internal_NT

//                                                              ;internal_NT
// This API is called only from intl.cpl when the user          ;internal_NT
// locale changes.                                              ;internal_NT
//                                                              ;internal_NT
void                                                            ;internal_NT
WINAPI                                                          ;internal_NT
NlsResetProcessLocale(void);                                    ;internal_NT


//                                                              ;internal_NT
// This API is called by system console Apps                    ;internal_NT
//                                                              ;internal_NT
LANGID                                                          ;internal_NT
WINAPI                                                          ;internal_NT
SetThreadUILanguage(WORD wReserved);                            ;internal_NT

//                                                              ;internal_NT
// This API can be used to verify if a UI language is installed.;internal_NT
//                                                              ;internal_NT
BOOL                                                            ;internal_NT
WINAPI                                                          ;internal_NT
IsValidUILanguage(LANGID UILangID);                             ;internal_NT


//
//  Locale Independent APIs.
//

WINBASEAPI
BOOL
WINAPI
GetStringTypeEx%(
    IN LCID     Locale,
    IN DWORD    dwInfoType,
    IN LPCTSTR%  lpSrcStr,
    IN int      cchSrc,
    OUT LPWORD  lpCharType);

//
//  NOTE: The parameters for GetStringTypeA and GetStringTypeW are
//        NOT the same.  The W version was shipped in NT 3.1.  The
//        A version was then shipped in 16-bit OLE with the wrong
//        parameters (ported from Win95).  To be compatible, we
//        must break the relationship between the A and W versions
//        of GetStringType.  There will be NO function call for the
//        generic GetStringType.
//
//        GetStringTypeEx (above) should be used instead.
//
WINBASEAPI
BOOL
WINAPI
GetStringTypeA(
    IN LCID     Locale,
    IN DWORD    dwInfoType,
    IN LPCSTR   lpSrcStr,
    IN int      cchSrc,
    OUT LPWORD  lpCharType);

WINBASEAPI
BOOL
WINAPI
GetStringTypeW(
    IN DWORD    dwInfoType,
    IN LPCWSTR  lpSrcStr,
    IN int      cchSrc,
    OUT LPWORD  lpCharType);


WINBASEAPI
int
WINAPI
FoldString%(
    IN DWORD    dwMapFlags,
    IN LPCTSTR%  lpSrcStr,
    IN int      cchSrc,
    OUT LPTSTR%  lpDestStr,
    IN int      cchDest);

;begin_winver_500
WINBASEAPI
BOOL
WINAPI
EnumSystemLanguageGroups%(
    IN LANGUAGEGROUP_ENUMPROC% lpLanguageGroupEnumProc,
    IN DWORD                   dwFlags,
    IN LONG_PTR                lParam);

WINBASEAPI
BOOL
WINAPI
EnumLanguageGroupLocales%(
    IN LANGGROUPLOCALE_ENUMPROC% lpLangGroupLocaleEnumProc,
    IN LGRPID                    LanguageGroup,
    IN DWORD                     dwFlags,
    IN LONG_PTR                  lParam);

WINBASEAPI
BOOL
WINAPI
EnumUILanguages%(
    IN UILANGUAGE_ENUMPROC% lpUILanguageEnumProc,
    IN DWORD                dwFlags,
    IN LONG_PTR             lParam);
;end_winver_500

WINBASEAPI
BOOL
WINAPI
EnumSystemLocales%(
    IN LOCALE_ENUMPROC% lpLocaleEnumProc,
    IN DWORD            dwFlags);

WINBASEAPI
BOOL
WINAPI
EnumSystemCodePages%(
    IN CODEPAGE_ENUMPROC% lpCodePageEnumProc,
    IN DWORD              dwFlags);

;begin_internal_NT
//
// These definitions are used by both winnls and base\server
//

//
//  Names of Registry Value Entries.
//
#define NLS_VALUE_ACP              L"ACP"
#define NLS_VALUE_OEMCP            L"OEMCP"
#define NLS_VALUE_MACCP            L"MACCP"
#define NLS_VALUE_DEFAULT          L"Default"

//  User Info
#define NLS_VALUE_LOCALE           L"Locale"
#define NLS_VALUE_SLANGUAGE        L"sLanguage"
#define NLS_VALUE_ICOUNTRY         L"iCountry"
#define NLS_VALUE_SCOUNTRY         L"sCountry"
#define NLS_VALUE_SLIST            L"sList"
#define NLS_VALUE_IMEASURE         L"iMeasure"
#define NLS_VALUE_IPAPERSIZE       L"iPaperSize"
#define NLS_VALUE_SDECIMAL         L"sDecimal"
#define NLS_VALUE_STHOUSAND        L"sThousand"
#define NLS_VALUE_SGROUPING        L"sGrouping"
#define NLS_VALUE_IDIGITS          L"iDigits"
#define NLS_VALUE_ILZERO           L"iLZero"
#define NLS_VALUE_INEGNUMBER       L"iNegNumber"
#define NLS_VALUE_SNATIVEDIGITS    L"sNativeDigits"
#define NLS_VALUE_IDIGITSUBST      L"NumShape"
#define NLS_VALUE_SCURRENCY        L"sCurrency"
#define NLS_VALUE_SMONDECIMALSEP   L"sMonDecimalSep"
#define NLS_VALUE_SMONTHOUSANDSEP  L"sMonThousandSep"
#define NLS_VALUE_SMONGROUPING     L"sMonGrouping"
#define NLS_VALUE_ICURRDIGITS      L"iCurrDigits"
#define NLS_VALUE_ICURRENCY        L"iCurrency"
#define NLS_VALUE_INEGCURR         L"iNegCurr"
#define NLS_VALUE_SPOSITIVESIGN    L"sPositiveSign"
#define NLS_VALUE_SNEGATIVESIGN    L"sNegativeSign"
#define NLS_VALUE_STIMEFORMAT      L"sTimeFormat"
#define NLS_VALUE_STIME            L"sTime"
#define NLS_VALUE_ITIME            L"iTime"
#define NLS_VALUE_ITLZERO          L"iTLZero"
#define NLS_VALUE_ITIMEMARKPOSN    L"iTimePrefix"
#define NLS_VALUE_S1159            L"s1159"
#define NLS_VALUE_S2359            L"s2359"
#define NLS_VALUE_SSHORTDATE       L"sShortDate"
#define NLS_VALUE_SDATE            L"sDate"
#define NLS_VALUE_IDATE            L"iDate"
#define NLS_VALUE_SYEARMONTH       L"sYearMonth"
#define NLS_VALUE_SLONGDATE        L"sLongDate"
#define NLS_VALUE_ICALENDARTYPE    L"iCalendarType"
#define NLS_VALUE_IFIRSTDAYOFWEEK  L"iFirstDayOfWeek"
#define NLS_VALUE_IFIRSTWEEKOFYEAR L"iFirstWeekOfYear"


//
//  String constants for CreateSection/OpenSection name string.
//
#define NLS_SECTION_CPPREFIX       L"\\NLS\\NlsSectionCP"
#define NLS_SECTION_LANGPREFIX     L"\\NLS\\NlsSectionLANG"

#define NLS_SECTION_UNICODE        L"\\NLS\\NlsSectionUnicode"
#define NLS_SECTION_LOCALE         L"\\NLS\\NlsSectionLocale"
#define NLS_SECTION_CTYPE          L"\\NLS\\NlsSectionCType"
#define NLS_SECTION_SORTKEY        L"\\NLS\\NlsSectionSortkey"
#define NLS_SECTION_SORTTBLS       L"\\NLS\\NlsSectionSortTbls"
#define NLS_SECTION_LANG_INTL      L"\\NLS\\NlsSectionLANG_INTL"
#define NLS_SECTION_LANG_EXCEPT    L"\\NLS\\NlsSectionLANG_EXCEPT"
#define NLS_SECTION_GEO            L"\\NLS\\NlsSectionGeo"

//
//  Unicode file names.
//  These files will always be installed by setup in the system directory,
//  so there is no need to put these names in the registry.
//
#define NLS_FILE_UNICODE           L"unicode.nls"
#define NLS_FILE_LOCALE            L"locale.nls"
#define NLS_FILE_CTYPE             L"ctype.nls"
#define NLS_FILE_SORTKEY           L"sortkey.nls"
#define NLS_FILE_SORTTBLS          L"sorttbls.nls"
#define NLS_FILE_LANG_INTL         L"l_intl.nls"
#define NLS_FILE_LANG_EXCEPT       L"l_except.nls"
#define NLS_FILE_GEO               L"geo.nls"

//
//  Default file names if registry is corrupt.
//
#define NLS_DEFAULT_FILE_ACP       L"c_1252.nls"
#define NLS_DEFAULT_FILE_OEMCP     L"c_437.nls"



//
//  Default section names if registry is corrupt.
//
#define NLS_DEFAULT_SECTION_ACP    L"\\NLS\\NlsSectionCP1252"
#define NLS_DEFAULT_SECTION_OEMCP  L"\\NLS\\NlsSectionCP437"

#ifdef _WINDOWS_BASE
//
//  winnls routines that are called from base\server.  The prototypes must
//  continue to match the typedefs.
//

typedef ULONG
(*PNLS_CONVERT_INTEGER_TO_STRING)(
    UINT Value,
    UINT Base,
    UINT Padding,
    LPWSTR pResultBuf,
    UINT Size);
ULONG
NlsConvertIntegerToString(
    UINT Value,
    UINT Base,
    UINT Padding,
    LPWSTR pResultBuf,
    UINT Size);

typedef BOOL
(*PGET_CP_FILE_NAME_FROM_REGISTRY)(
    UINT    CodePage,
    LPWSTR  pResultBuf,
    UINT    Size);
BOOL
GetCPFileNameFromRegistry(
    UINT    CodePage,
    LPWSTR  pResultBuf,
    UINT    Size);

typedef ULONG
(*PCREATE_NLS_SECURITY_DESCRIPTOR)(
    PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    UINT                    SecurityDescriptorSize,
    ACCESS_MASK             AccessMask);

ULONG
CreateNlsSecurityDescriptor(
    PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    UINT                    SecurityDescriptorSize,
    ACCESS_MASK             AccessMask);

typedef ULONG
(*PGET_NLS_SECTION_NAME)(
    UINT Value,
    UINT Base,
    UINT Padding,
    LPWSTR pwszPrefix,
    LPWSTR pwszSecName,
    UINT cchSecName);
ULONG
GetNlsSectionName(
    UINT Value,
    UINT Base,
    UINT Padding,
    LPWSTR pwszPrefix,
    LPWSTR pwszSecName,
    UINT cchSecName);


typedef WINBASEAPI BOOL
(WINAPI *PIS_VALID_CODEPAGE)(
    UINT CodePage);
WINBASEAPI BOOL WINAPI
IsValidCodePage(
    UINT CodePage);

typedef ULONG
(*POPEN_DATA_FILE)(HANDLE *phFile, LPWSTR pFile);
ULONG OpenDataFile(HANDLE *phFile, LPWSTR pFile);

typedef ULONG
(*PGET_DEFAULT_SORTKEY_SIZE)(PLARGE_INTEGER pSize);
ULONG GetDefaultSortkeySize(PLARGE_INTEGER pSize);

typedef ULONG
(*PGET_LINGUIST_LANG_SIZE)(PLARGE_INTEGER pSize);
ULONG GetLinguistLangSize(PLARGE_INTEGER pSize);

typedef BOOL
(*PVALIDATE_LOCALE)(LCID Locale);
BOOL ValidateLocale(LCID Locale);

typedef BOOL
(*PVALIDATE_LCTYPE)(PVOID pInfo, LCTYPE LCType, LPWSTR *ppwReg, LPWSTR *ppwCache);
BOOL ValidateLCType(PNLS_USER_INFO pInfo, LCTYPE LCType, LPWSTR *ppwReg, LPWSTR *ppwCache);

typedef BOOL
(*PNLS_LOAD_STRING_EX_W)(HMODULE hModule, UINT wID, LPWSTR lpBuffer, int cchBufferMax, WORD wLangId);
int NlsLoadStringExW(HMODULE hModule, UINT wID, LPWSTR lpBuffer, int cchBufferMax, WORD wLangId);

#endif // _WINDOWS_BASE

;end_internal_NT

#endif // NONLS


#ifdef __cplusplus           ;both
}                            ;both
#endif                       ;both
                             ;both
#endif // _WINNLS_
#endif // _WINNLSP_          ;internal_NT
