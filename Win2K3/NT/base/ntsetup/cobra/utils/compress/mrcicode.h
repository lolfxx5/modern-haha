/*
 *  Microsoft Confidential
 *  Copyright (c) 1994 Microsoft Corporation
 *  All Rights Reserved.
 *
 *  MRCICODE.H
 *
 *  MRCI 1 & MRCI 2 maxcompress and decompress functions
 */

extern unsigned Mrci1MaxCompress(unsigned char FAR *pchbase,unsigned cchunc,
        unsigned char FAR *pchcmpBase,unsigned cchcmpMax);

extern unsigned Mrci1Decompress(unsigned char FAR *pchin,unsigned cchin,
        unsigned char FAR *pchdecBase,unsigned cchdecMax);

extern unsigned Mrci2MaxCompress(unsigned char FAR *pchbase,unsigned cchunc,
        unsigned char FAR *pchcmpBase,unsigned cchcmpMax);

extern unsigned Mrci2Decompress(unsigned char FAR *pchin,unsigned cchin,
        unsigned char FAR *pchdecBase,unsigned cchdecMax);

