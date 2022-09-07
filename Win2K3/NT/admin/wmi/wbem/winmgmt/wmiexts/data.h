#ifndef __DATA_H__
#define __DATA_H__

#include <objbase.h>

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)


MIDL_DEFINE_GUID(CLSID, CLSID_WmiRepository_SQL,0x89B9BAF8,0x6A06,0x11d3,0xA5,0xFE,0x00,0x10,0x5A,0x0A,0x31,0x02);
MIDL_DEFINE_GUID(CLSID, CLSID_WmiRepository_Jet,0x89B9BAFC,0x6A06,0x11d3,0xA5,0xFE,0x00,0x10,0x5A,0x0A,0x31,0x02);
MIDL_DEFINE_GUID(CLSID, CLSID_WmiRepositoryQuery,0x29129B3F,0x7899,0x4E82,0xA3,0xC3,0xA9,0x67,0x49,0x58,0x9D,0xCE);
MIDL_DEFINE_GUID(CLSID, CLSID_UmiDefURL,0xd4b21cc2,0xf2a5,0x453e,0x84,0x59,0xb2,0x7f,0x36,0x2c,0xb0,0xe0);
MIDL_DEFINE_GUID(CLSID, CLSID_PseudoSink,0xE002E4F0,0xE6EA,0x11d2,0x9C,0xB3,0x00,0x10,0x5A,0x1F,0x48,0x01);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemClassObjectProxy,0x4590f812,0x1d3a,0x11d0,0x89,0x1f,0x00,0xaa,0x00,0x4b,0x2e,0x24);
MIDL_DEFINE_GUID(CLSID, CLSID_UmiObjectWrapperProxy,0x4E6AC63C,0xBF69,0x495d,0x90,0x00,0xE4,0x5A,0x4E,0x51,0x7B,0x0C);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemEventSubsystem,0x5d08b586,0x343a,0x11d0,0xad,0x46,0x00,0xc0,0x4f,0xd8,0xfd,0xff);
MIDL_DEFINE_GUID(CLSID, CLSID_HmmpEventConsumerProvider,0x08a59b5d,0xdd50,0x11d0,0xad,0x6b,0x00,0xc0,0x4f,0xd8,0xfd,0xff);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemFilterProxy,0x6c19be35,0x7500,0x11d1,0xad,0x94,0x00,0xc0,0x4f,0xd8,0xfd,0xff);
MIDL_DEFINE_GUID(CLSID, CLSID_InProcWbemLevel1Login,0x4fa18276,0x912a,0x11d1,0xad,0x9b,0x00,0xc0,0x4f,0xd8,0xfd,0xff);
MIDL_DEFINE_GUID(CLSID, CLSID__WmiFreeFormObject,0x3252F829,0x8694,0x46a8,0xB4,0xCF,0x83,0xF6,0xA0,0xFF,0xEF,0xA9);
MIDL_DEFINE_GUID(CLSID, CLSID__WmiObjectFactory,0x78103FB7,0xAED7,0x4066,0x8B,0xCD,0x30,0xBB,0x27,0xB0,0x23,0x31);
MIDL_DEFINE_GUID(CLSID, CLSID__WbemUMIContextWrapper,0x390150A7,0xAB20,0x45ff,0xA2,0xE0,0x6B,0x98,0x55,0x54,0xCA,0xA8);
MIDL_DEFINE_GUID(CLSID, CLSID__WmiErrorObject,0xE2569DC9,0x38FA,0x4749,0xBE,0xE5,0xA2,0x63,0xE4,0x03,0x35,0x9F);
MIDL_DEFINE_GUID(CLSID, CLSID__UmiErrorObject,0xF7D04323,0x5378,0x40c1,0xB5,0x88,0xC8,0x4F,0x91,0xE2,0xB8,0x2C);
MIDL_DEFINE_GUID(CLSID, CLSID_WinmgmtMofCompiler,0xC10B4771,0x4DA0,0x11d2,0xA2,0xF5,0x00,0xC0,0x4F,0x86,0xFB,0x7D);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemTokenCache,0xdc923725,0x0fdd,0x45e1,0xae,0x74,0xea,0x09,0x18,0x2e,0x73,0x9b);
MIDL_DEFINE_GUID(CLSID, CLSID_WmiESS,0xf3130cdb,0xaa52,0x4c3a,0xab,0x32,0x85,0xff,0xc2,0x3a,0xf9,0xc1);
MIDL_DEFINE_GUID(CLSID, CLSID_WmiProvSS,0x4de225bf,0xcf59,0x4cfc,0x85,0xf7,0x68,0xb9,0x0f,0x18,0x53,0x55);
MIDL_DEFINE_GUID(CLSID, CLSID_WmiProviderBindingFactory,0xF5A55D36,0x8750,0x432C,0xAB,0x52,0xAD,0x49,0xA0,0x16,0xEA,0xBC);
MIDL_DEFINE_GUID(CLSID, CLSID_WmiProviderSharedFactory,0x24F1D9A7,0xB682,0x4CF3,0x88,0x0C,0x18,0xFD,0x90,0xD5,0x5C,0xD6);
MIDL_DEFINE_GUID(CLSID, CLSID_WmiProviderDedicatedFactory,0x4b4baaa2,0xaaf3,0x4b08,0x9c,0x9e,0xc4,0x67,0x69,0x60,0x7e,0xba);
MIDL_DEFINE_GUID(CLSID, CLSID_WmiProviderInProcFactory,0x8BEBCE8B,0x1AF0,0x4323,0x8B,0x4D,0x36,0x99,0x45,0x67,0xCA,0xE1);
MIDL_DEFINE_GUID(CLSID, CLSID_IWmiCoreServices,0x1860e246,0xe924,0x4f73,0xb2,0xc5,0x93,0xe0,0x57,0x7e,0x3a,0xa1);
MIDL_DEFINE_GUID(CLSID, CLSID__WmiWbemClass,0x0859CCC9,0x209D,0x4110,0x96,0x13,0xDA,0xB6,0xEA,0xF9,0x09,0x3F);
MIDL_DEFINE_GUID(CLSID, CLSID__WmiWbemInstance,0xDD51FE78,0x43E1,0x405a,0xA4,0x4E,0x6C,0x22,0x63,0x91,0xCF,0x0D);
MIDL_DEFINE_GUID(CLSID, CLSID__WmiQuery,0xda1fc6d2,0x40e4,0x4e2f,0xbb,0x42,0xe7,0x0d,0x28,0xc8,0x91,0xb3);
MIDL_DEFINE_GUID(CLSID, CLSID__WbemUMIObjectWrapper,0xC601737E,0x9213,0x489f,0xAD,0xC8,0x92,0x2A,0x89,0x4A,0x4A,0x65);
MIDL_DEFINE_GUID(CLSID, CLSID__WbemEmptyClassObject,0x695B5458,0xD6E9,0x4a6a,0x88,0x1F,0xB6,0x8F,0x95,0x33,0x97,0xD8);
MIDL_DEFINE_GUID(CLSID, CLSID__IWbemCallSec,0x1108be51,0xf58a,0x4cda,0xbb,0x99,0x7a,0x02,0x27,0xd1,0x1d,0x5e);
MIDL_DEFINE_GUID(CLSID, CLSID__WbemConfigureRefreshingSvcs,0xCD1ABFC8,0x6C5E,0x4a8d,0xB9,0x0B,0x2A,0x3B,0x15,0x3B,0x88,0x6D);
MIDL_DEFINE_GUID(CLSID, CLSID__WbemRefresherMgr,0xDCF33DF4,0xB510,0x439f,0x83,0x2A,0x16,0xB6,0xB5,0x14,0xF2,0xA7);
MIDL_DEFINE_GUID(CLSID, CLSID__WbemHostedRefresherMgr,0x288d70f7,0x11c8,0x42d5,0xa6,0x12,0x8e,0x46,0xa7,0xe9,0x22,0xd8);
MIDL_DEFINE_GUID(CLSID, CLSID__WbemComBinding,0xED51D12E,0x511F,0x4999,0x8D,0xCD,0xC2,0xBA,0xC9,0x1B,0xE8,0x6E);
MIDL_DEFINE_GUID(CLSID, CLSID__UmiComBinding,0xD5D3ACEA,0xEEC7,0x4efd,0xA0,0x6D,0xFF,0x54,0xB4,0x27,0x16,0x55);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemLocator,0x4590f811,0x1d3a,0x11d0,0x89,0x1f,0x00,0xaa,0x00,0x4b,0x2e,0x24);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemConnection,0x4c6055d8,0x84b9,0x4111,0xa7,0xd3,0x66,0x23,0x89,0x4e,0xed,0xb3);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemContext,0x674B6698,0xEE92,0x11d0,0xAD,0x71,0x00,0xC0,0x4F,0xD8,0xFD,0xFF);
MIDL_DEFINE_GUID(CLSID, CLSID_UnsecuredApartment,0x49bd2028,0x1523,0x11d1,0xad,0x79,0x00,0xc0,0x4f,0xd8,0xfd,0xff);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemClassObject,0x9A653086,0x174F,0x11d2,0xB5,0xF9,0x00,0x10,0x4B,0x70,0x3E,0xFD);
MIDL_DEFINE_GUID(CLSID, CLSID_MofCompiler,0x6daf9757,0x2e37,0x11d2,0xae,0xc9,0x00,0xc0,0x4f,0xb6,0x88,0x20);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemStatusCodeText,0xeb87e1bd,0x3233,0x11d2,0xae,0xc9,0x00,0xc0,0x4f,0xb6,0x88,0x20);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemBackupRestore,0xC49E32C6,0xBC8B,0x11d2,0x85,0xD4,0x00,0x10,0x5A,0x1F,0x83,0x04);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemRefresher,0xc71566f2,0x561e,0x11d1,0xad,0x87,0x00,0xc0,0x4f,0xd8,0xfd,0xff);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemObjectTextSrc,0x8D1C559D,0x84F0,0x4bb3,0xA7,0xD5,0x56,0xA7,0x43,0x5A,0x9B,0xA6);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemAdministrativeLocator,0xcb8555cc,0x9128,0x11d1,0xad,0x9b,0x00,0xc0,0x4f,0xd8,0xfd,0xff);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemAuthenticatedLocator,0xcd184336,0x9128,0x11d1,0xad,0x9b,0x00,0xc0,0x4f,0xd8,0xfd,0xff);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemUnauthenticatedLocator,0x443E7B79,0xDE31,0x11d2,0xB3,0x40,0x00,0x10,0x4B,0xCC,0x4B,0x4A);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemDecoupledRegistrar,0x4cfc7932,0x0f9d,0x4bef,0x9c,0x32,0x8e,0xa2,0xa6,0xb5,0x6f,0xcb);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemDecoupledBasicEventProvider,0xf5f75737,0x2843,0x4f22,0x93,0x3d,0xc7,0x6a,0x97,0xcd,0xa6,0x2f);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemLevel1Login,0x8BC3F05E,0xD86B,0x11d0,0xA0,0x75,0x00,0xC0,0x4F,0xB6,0x88,0x20);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemDCOMTransport,0xF7CE2E13,0x8C90,0x11d1,0x9E,0x7B,0x00,0xC0,0x4F,0xC3,0x24,0xA8);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemLocalAddrRes,0xA1044801,0x8F7E,0x11d1,0x9E,0x7C,0x00,0xC0,0x4F,0xC3,0x24,0xA8);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemUninitializedClassObject,0x7a0227f6,0x7108,0x11d1,0xad,0x90,0x00,0xc0,0x4f,0xd8,0xfd,0xff);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemDefPath,0xcf4cc405,0xe2c5,0x4ddd,0xb3,0xce,0x5e,0x75,0x82,0xd8,0xc9,0xfa);
MIDL_DEFINE_GUID(CLSID, CLSID_WbemQuery,0xEAC8A024,0x21E2,0x4523,0xAD,0x73,0xA7,0x1A,0x0A,0xA2,0xF5,0x6A);
MIDL_DEFINE_GUID(CLSID, CLSID_WMIExtension,0xf0975afe,0x5c7f,0x11d2,0x8b,0x74,0x00,0x10,0x4b,0x2a,0xfb,0x41);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemLocator,0x76A64158,0xCB41,0x11d1,0x8B,0x02,0x00,0x60,0x08,0x06,0xD9,0xB6);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemLocatorEx,0xBF37162F,0x9E73,0x48ed,0xB0,0x09,0x92,0xE2,0xF7,0x32,0x25,0x2F);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemNamedValueSet,0x9AED384E,0xCE8B,0x11d1,0x8B,0x05,0x00,0x60,0x08,0x06,0xD9,0xB6);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemNamedValueSetEx,0xF9EF137C,0xC934,0x4e55,0x85,0x34,0x20,0x4F,0xEE,0x2E,0xDA,0x77);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemObjectPath,0x5791BC26,0xCE9C,0x11d1,0x97,0xBF,0x00,0x00,0xF8,0x1E,0x84,0x9C);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemObjectPathEx,0x51217565,0xE9CE,0x4534,0xB0,0x68,0x6F,0x50,0xFF,0x77,0xC2,0xC7);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemLastError,0xC2FEEEAC,0xCFCD,0x11d1,0x8B,0x05,0x00,0x60,0x08,0x06,0xD9,0xB6);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemSink,0x75718C9A,0xF029,0x11d1,0xA1,0xAC,0x00,0xC0,0x4F,0xB6,0xC2,0x23);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemDateTime,0x47DFBE54,0xCF76,0x11d3,0xB3,0x8F,0x00,0x10,0x5A,0x1F,0x47,0x3A);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemRefresher,0xD269BF5C,0xD9C1,0x11d3,0xB3,0x8F,0x00,0x10,0x5A,0x1F,0x47,0x3A);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemServices,0x04B83D63,0x21AE,0x11d2,0x8B,0x33,0x00,0x60,0x08,0x06,0xD9,0xB6);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemServicesEx,0x62E522DC,0x8CF3,0x40a8,0x8B,0x2E,0x37,0xD5,0x95,0x65,0x1E,0x40);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemObject,0x04B83D62,0x21AE,0x11d2,0x8B,0x33,0x00,0x60,0x08,0x06,0xD9,0xB6);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemObjectEx,0xD6BDAFB2,0x9435,0x491f,0xBB,0x87,0x6A,0xA0,0xF0,0xBC,0x31,0xA2);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemObjectSet,0x04B83D61,0x21AE,0x11d2,0x8B,0x33,0x00,0x60,0x08,0x06,0xD9,0xB6);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemNamedValue,0x04B83D60,0x21AE,0x11d2,0x8B,0x33,0x00,0x60,0x08,0x06,0xD9,0xB6);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemNamedValueEx,0xEE10B42E,0x02C0,0x4bef,0x80,0xB1,0xAE,0x94,0x92,0x39,0x1E,0x39);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemQualifier,0x04B83D5F,0x21AE,0x11d2,0x8B,0x33,0x00,0x60,0x08,0x06,0xD9,0xB6);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemQualifierSet,0x04B83D5E,0x21AE,0x11d2,0x8B,0x33,0x00,0x60,0x08,0x06,0xD9,0xB6);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemProperty,0x04B83D5D,0x21AE,0x11d2,0x8B,0x33,0x00,0x60,0x08,0x06,0xD9,0xB6);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemPropertyEx,0x00F2D3B2,0x6AC8,0x4406,0xB9,0x8D,0x2D,0x0E,0xA9,0x57,0xC2,0x91);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemPropertySet,0x04B83D5C,0x21AE,0x11d2,0x8B,0x33,0x00,0x60,0x08,0x06,0xD9,0xB6);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemMethod,0x04B83D5B,0x21AE,0x11d2,0x8B,0x33,0x00,0x60,0x08,0x06,0xD9,0xB6);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemMethodSet,0x04B83D5A,0x21AE,0x11d2,0x8B,0x33,0x00,0x60,0x08,0x06,0xD9,0xB6);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemEventSource,0x04B83D58,0x21AE,0x11d2,0x8B,0x33,0x00,0x60,0x08,0x06,0xD9,0xB6);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemSecurity,0xB54D66E9,0x2287,0x11d2,0x8B,0x33,0x00,0x60,0x08,0x06,0xD9,0xB6);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemPrivilege,0x26EE67BC,0x5804,0x11d2,0x8B,0x4A,0x00,0x60,0x08,0x06,0xD9,0xB6);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemPrivilegeSet,0x26EE67BE,0x5804,0x11d2,0x8B,0x4A,0x00,0x60,0x08,0x06,0xD9,0xB6);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemRefreshableItem,0x8C6854BC,0xDE4B,0x11d3,0xB3,0x90,0x00,0x10,0x5A,0x1F,0x47,0x3A);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemObjectPathComponents,0xF575AF1A,0xE58D,0x11d3,0xB3,0x91,0x00,0x10,0x5A,0x1F,0x47,0x3A);
MIDL_DEFINE_GUID(CLSID, CLSID_SWbemTransaction,0x80B736A1,0x75B0,0x471c,0xA7,0xA4,0x85,0x52,0x91,0x7D,0x85,0x39);
MIDL_DEFINE_GUID(CLSID, CLSID_WmiMessageMsmqSender,0x122D47A6,0xCEEC,0x4de1,0x80,0x56,0xB6,0xD1,0x6F,0x29,0xBC,0x97);
MIDL_DEFINE_GUID(CLSID, CLSID_WmiMessageMsmqReceiver,0x9E007F18,0x9C24,0x4630,0x8B,0x3E,0x61,0xF9,0x62,0x80,0xC5,0x93);
MIDL_DEFINE_GUID(CLSID, CLSID_WmiMessageDcomSender,0x622D47B6,0xCEEC,0x4de1,0x80,0x56,0xB6,0xD1,0x6F,0x29,0xBC,0x97);
MIDL_DEFINE_GUID(CLSID, CLSID_WmiMessageDcomReceiver,0x9F007F18,0x9C24,0x4630,0x8B,0x3E,0x61,0xF9,0x62,0x80,0xC5,0x93);
MIDL_DEFINE_GUID(CLSID, CLSID_WmiMessageMultiSendReceive,0x89F9F7B0,0x8DE3,0x4ae0,0x8B,0x41,0x10,0x9A,0xBA,0xB3,0x21,0x51);
MIDL_DEFINE_GUID(CLSID, CLSID_WmiMessageQueue,0xC89DBDC4,0x5491,0x409a,0x8D,0x00,0xE3,0x45,0x38,0x21,0x1F,0xED);
MIDL_DEFINE_GUID(CLSID, CLSID_WmiMessageQueueManager,0xFF10E656,0x2B7C,0x421e,0xB1,0x45,0x7A,0xB3,0x37,0xFB,0x86,0x5F);
MIDL_DEFINE_GUID(CLSID, CLSID_WmiMessageService,0xCE69CC1E,0x1EC0,0x4847,0x9C,0x0D,0xD2,0xF2,0xD8,0x0D,0x07,0xCF);
MIDL_DEFINE_GUID(CLSID, CLSID_WmiSmartObjectMarshal,0xC169CC11,0x1EC1,0x4847,0x9C,0x0D,0xD2,0xF2,0xD8,0x0D,0x07,0xCF);
MIDL_DEFINE_GUID(CLSID, CLSID_MsiMethodStatusSink,0xFDD70FF2,0x0195,0x11d3,0xA9,0x7D,0x00,0xA0,0xC9,0x95,0x49,0x21);
MIDL_DEFINE_GUID(CLSID, CLSID_MsiProductMethods,0xAB4184C0,0xFDAD,0x11D2,0xA9,0x7B,0x00,0xA0,0xC9,0x95,0x49,0x21);
MIDL_DEFINE_GUID(CLSID, CLSID_MsiSoftwareFeatureMethods,0xE9B5C790,0xFDAD,0x11D2,0xA9,0x7B,0x00,0xA0,0xC9,0x95,0x49,0x21);
MIDL_DEFINE_GUID(CLSID, CLSID_WMIObjectBroker,0x4BA59771,0x8FBF,0x4E67,0x99,0x17,0x3B,0xBB,0x39,0xB7,0x43,0xAB);
MIDL_DEFINE_GUID(CLSID, CLSID_WMIObjectBrokerRegistration,0x9ECF8EC8,0xA9ED,0x47DF,0xBB,0x9A,0x81,0xDC,0xB3,0x69,0x85,0x07);
MIDL_DEFINE_GUID(CLSID, CLSID_CMSnapin,0x0F3621F1,0x23C6,0x11D1,0xAD,0x97,0x00,0xAA,0x00,0xB8,0x8E,0x5A);
MIDL_DEFINE_GUID(CLSID, CLSID_CMSnapinAbout,0xA1B9E020,0x3226,0x11D2,0x88,0x3E,0x00,0x10,0x4B,0x2A,0xFB,0x46);
MIDL_DEFINE_GUID(CLSID, CLSID_NSDrive,0x6E8E0081,0x19CD,0x11D1,0xAD,0x91,0x00,0xAA,0x00,0xB8,0xE0,0x5A);
MIDL_DEFINE_GUID(CLSID, CLSID_NSDriveAbout,0x692A8956,0x1089,0x11D2,0x88,0x37,0x00,0x10,0x4B,0x2A,0xFB,0x46);
MIDL_DEFINE_GUID(CLSID, CLSID_SDSnapin,0xBD95BA60,0x2E26,0xAAD1,0xAD,0x99,0x00,0xAA,0x00,0xB8,0xE0,0x5A);
MIDL_DEFINE_GUID(CLSID, CLSID_SDSnapinAbout,0xA1B9E04A,0x3226,0x11D2,0x88,0x3E,0x00,0x10,0x4B,0x2A,0xFB,0x46);
MIDL_DEFINE_GUID(CLSID, CLSID_WMISnapin,0x5C659257,0xE236,0x11D2,0x88,0x99,0x00,0x10,0x4B,0x2A,0xFB,0x46);
MIDL_DEFINE_GUID(CLSID, CLSID_WMISnapinAbout,0x5C659258,0xE236,0x11D2,0x88,0x99,0x00,0x10,0x4B,0x2A,0xFB,0x46);
MIDL_DEFINE_GUID(CLSID, CLSID_PseudoProvider,0xE002E4EE,0xE6EA,0x11d2,0x9C,0xB3,0x00,0x10,0x5A,0x1F,0x48,0x01);
MIDL_DEFINE_GUID(CLSID, CLSID_KernelTraceProvider,0x9877D8A7,0xFDA1,0x43F9,0xAE,0xEA,0xF9,0x07,0x47,0xEA,0x66,0xB0);
MIDL_DEFINE_GUID(CLSID, CLSID_NCProvider,0x29F06F0C,0xFB7F,0x44A5,0x83,0xCD,0xD4,0x17,0x05,0xD5,0xC5,0x25);
MIDL_DEFINE_GUID(CLSID, CLSID_VSAPlugIn,0x2169E810,0xFE80,0x4107,0xAE,0x18,0x79,0x8D,0x50,0x68,0x4A,0x71);
MIDL_DEFINE_GUID(CLSID, CLSID_VSAToWMIEventProvider,0x13A77B61,0x226B,0x46A9,0x91,0xB9,0x22,0x52,0x12,0x79,0x6D,0x92);
MIDL_DEFINE_GUID(CLSID, CLSID_DcomMsgReceiveStub,0x7879B294,0xBF5F,0x476b,0xAB,0xC9,0x9B,0x1F,0x17,0x61,0x6A,0xFB);

MIDL_DEFINE_GUID(CLSID, CLSID_MyEventProvider, 0x4916157a, 0xfbe7, 0x11d1, 0xae, 0xc4, 0x0, 0xc0, 0x4f, 0xb6, 0x88, 0x20);


MIDL_DEFINE_GUID(CLSID, UUID_DCOMName, 0xa2f7d6c1, 0x8dcd, 0x11d1, 0x9e, 0x7c, 0x0, 0xc0, 0x4f, 0xc3, 0x24, 0xa8);

MIDL_DEFINE_GUID(CLSID, UUID_LocalAddResName, 0xa1044802, 0x8f7e, 0x11d1, 0x9e, 0x7c, 0x0, 0xc0, 0x4f, 0xc3, 0x24, 0xa8);


MIDL_DEFINE_GUID(CLSID, UUID_LocalAddrType, 0xa1044803, 0x8f7e, 0x11d1, 0x9e, 0x7c, 0x0, 0xc0, 0x4f, 0xc3, 0x24, 0xa8);


DEFINE_GUID(UUID_LocalAddrTypeName, 0xa1044804, 0x8f7e, 0x11d1, 0x9e, 0x7c, 0x0, 0xc0, 0x4f, 0xc3, 0x24, 0xa8);

DEFINE_GUID(CLSID_WinNTConnectionObject,0x7992c6eb,0xd142,0x4332,0x83,0x1e,0x31,0x54,0xc5,0x0a,0x83,0x16);
DEFINE_GUID(CLSID_LDAPConnectionObject,0x7da2a9c4,0x0c46,0x43bd,0xb0,0x4e,0xd9,0x2b,0x1b,0xe2,0x7c,0x45);



MIDL_DEFINE_GUID(CLSID, CLSID_PerfProvider,0xF00B4404L,0xF8F1,0x11CE,0xA5,0xB6,0x00,0xAA,0x00,0x68,0x0C,0x3F);
MIDL_DEFINE_GUID(CLSID, CLSID_PerfPropProv, 0x72967903, 0x68ec, 0x11d0, 0xb7, 0x29, 0x0, 0xaa, 0x0, 0x62, 0xcb, 0xb7);
MIDL_DEFINE_GUID(CLSID, CLSID_RegistryEventProvider,0xfa77a74e,0xe109,0x11d0,0xad,0x6e,0x00,0xc0,0x4f,0xd8,0xfd,0xff);



MIDL_DEFINE_GUID(CLSID, CLSID__DSSvcExWrap,0xFD2057FA,0x99FE,0x4f10,0x89,0x08,0x9A,0xA2,0xAA,0xB3,0x2A,0x6E);
MIDL_DEFINE_GUID(IID, IID_IWMIRefreshableCooker,0x13ED7E55,0x8D63,0x41b0,0x90,0x86,0xD0,0xC5,0xC1,0x73,0x64,0xC8);
MIDL_DEFINE_GUID(IID, IID_IWMISimpleObjectCooker,0xA239BDF1,0x0AB1,0x45a0,0x87,0x64,0x15,0x91,0x15,0x68,0x95,0x89);
MIDL_DEFINE_GUID(IID, IID_IWMISimpleCooker,0x510ADF6E,0xD481,0x4a64,0xB7,0x4A,0xCC,0x71,0x2E,0x11,0xAA,0x34);


typedef struct _ArrayCLSID {
    //REFGUID Clsid; 
    const GUID * pClsid;
    const char * pStrClsid;
} ArrayCLSID;

extern DWORD g_nClsids;

#endif /*__DATA_H__*/
