//
// Copyright 1997 - Microsoft
//

//
// GUIDS.H - GUID definitions
//

#ifndef _GUIDS_H_
#define _GUIDS_H_

//
// External CLSIDs
//

// {0F65B1BF-740F-11d1-BBE6-0060081692B3}
DEFINE_GUID(CLSID_Computer, 
0xf65b1bf, 0x740f, 0x11d1, 0xbb, 0xe6, 0x0, 0x60, 0x8, 0x16, 0x92, 0xb3);

// {AC409538-741C-11d1-BBE6-0060081692B3}
DEFINE_GUID(CLSID_Service, 
0xac409538, 0x741c, 0x11d1, 0xbb, 0xe6, 0x0, 0x60, 0x8, 0x16, 0x92, 0xb3);

// {C641CF88-892F-11d1-BBEB-0060081692B3}
DEFINE_GUID(CLSID_Group, 
0xc641cf88, 0x892f, 0x11d1, 0xbb, 0xeb, 0x0, 0x60, 0x8, 0x16, 0x92, 0xb3);

// {D6D8C25A-4E83-11d2-8424-00C04FA372D4}
DEFINE_GUID(CLSID_NewComputerExtension,
0xd6d8c25a, 0x4e83, 0x11d2, 0x84, 0x24, 0x0, 0xc0, 0x4f, 0xa3, 0x72, 0xd4);

// {C1293E17-534E-11d2-8424-00C04FA372D4}
DEFINE_GUID(CLSID_RIQueryForm, 
0xc1293e17, 0x534e, 0x11d2, 0x84, 0x24, 0x0, 0xc0, 0x4f, 0xa3, 0x72, 0xd4);

// {55650117-5b71-47f7-9fc1-0431f53c006f}
DEFINE_GUID(CLSID_RISrvQueryForm,
0x55650117, 0x5b71, 0x47f7, 0x9f, 0xc1, 0x04, 0x31, 0xf5, 0x3c, 0x00, 0x6f);

//
// Internally Used Private Interfaces
//

// {F6215ED8-819C-11d1-BBE9-00C04FB953EA}
DEFINE_GUID(IID_IMAO, 
0xf6215ed8, 0x819c, 0x11d1, 0xbb, 0xe9, 0x0, 0xc0, 0x4f, 0xb9, 0x53, 0xea);

// {C88158C5-87A2-11d1-BBEA-00C04FB953EA}
DEFINE_GUID(IID_IIntelliMirrorSAP, 
0xc88158c5, 0x87a2, 0x11d1, 0xbb, 0xea, 0x0, 0xc0, 0x4f, 0xb9, 0x53, 0xea);

// {FA7C2CE0-889D-11d1-BBEA-00C04FB953EA}
DEFINE_GUID(IID_IEnumIMSIFs, 
0xfa7c2ce0, 0x889d, 0x11d1, 0xbb, 0xea, 0x0, 0xc0, 0x4f, 0xb9, 0x53, 0xea);

// {562B752D-9140-11d1-BBEF-00C04FB953EA}
DEFINE_GUID(IID_IEnumSAPs, 
0x562b752d, 0x9140, 0x11d1, 0xbb, 0xef, 0x0, 0xc0, 0x4f, 0xb9, 0x53, 0xea);

// {D2378471-523D-11d2-8424-00C04FA372D4}
DEFINE_GUID(IID_ITab, 
0xd2378471, 0x523d, 0x11d2, 0x84, 0x24, 0x0, 0xc0, 0x4f, 0xa3, 0x72, 0xd4);

#endif // _GUIDS_H_