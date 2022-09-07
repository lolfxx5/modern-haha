/*++

Copyright (c) Microsoft Corporation

Module Name:

    comclass.cpp

Abstract:

    Activation context section contributor for COM servers.

Author:

    Michael J. Grier (MGrier) 23-Feb-2000

Revision History:

--*/

#include "stdinc.h"
#include <windows.h>
#include "sxsp.h"

DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(clsid);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(iid);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(name);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(progid);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(proxyStubClsid32);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(runtimeVersion);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(threadingModel);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(tlbid);

#define ALLOCATE_BUFFER_SPACE(_bytesNeeded, _bufferCursor, _bytesLeft, _bytesWritten, _typeName, _ptr) \
do { \
    if (_bytesLeft < (_bytesNeeded)) \
        ORIGINATE_WIN32_FAILURE_AND_EXIT(NoRoom, ERROR_INSUFFICIENT_BUFFER); \
    _bytesLeft -= (_bytesNeeded); \
    _bytesWritten += (_bytesNeeded); \
    _ptr = (_typeName) _bufferCursor; \
    _bufferCursor = (PVOID) (((ULONG_PTR) _bufferCursor) + (_bytesNeeded)); \
} while (0)

#define ALLOCATE_BUFFER_SPACE_TYPE(_typeName, _bufferCursor, _bytesLeft, _bytesWritten, _ptr) \
    ALLOCATE_BUFFER_SPACE(sizeof(_typeName), _bufferCursor, _bytesLeft, _bytesWritten, _typeName *, _ptr)


typedef struct _COM_GLOBAL_CONTEXT *PCOM_GLOBAL_CONTEXT;
typedef struct _COM_FILE_CONTEXT *PCOM_FILE_CONTEXT;
typedef struct _COM_SERVER_ENTRY *PCOM_SERVER_ENTRY;

typedef struct _COM_SERVER_ENTRY
{
public:
    _COM_SERVER_ENTRY() { }

    CDequeLinkage m_Linkage;

    PCOM_FILE_CONTEXT m_FileContext;
    GUID m_ReferenceClsid;
    GUID m_ConfiguredClsid;
    GUID m_ImplementedClsid;
    GUID m_TypeLibraryId;
    ULONG m_ThreadingModel;
    CSmallStringBuffer m_ProgIdBuffer;
    CTinyStringBuffer m_TypeNameBuffer;
    CTinyStringBuffer m_ShimNameBuffer;
    CTinyStringBuffer m_RuntimeVersionBuffer;
    ULONG m_ShimType;
    bool m_IsFirstShim;
private:
    _COM_SERVER_ENTRY(const _COM_SERVER_ENTRY &);
    void operator =(const _COM_SERVER_ENTRY &);
} COM_SERVER_ENTRY;

typedef CDeque<COM_SERVER_ENTRY, offsetof(COM_SERVER_ENTRY, m_Linkage)> CComServerDeque;
typedef CDequeIterator<COM_SERVER_ENTRY, offsetof(COM_SERVER_ENTRY, m_Linkage)> CComServerDequeIterator;


typedef struct _COM_FILE_CONTEXT
{
public:
    _COM_FILE_CONTEXT() { }
    ~_COM_FILE_CONTEXT() { m_ServerList.ClearAndDeleteAll(); }


    CDequeLinkage m_Linkage;
    CSmallStringBuffer m_FileNameBuffer;
    CComServerDeque m_ServerList;
    ULONG m_Offset; // populated during section generation
    PCOM_GLOBAL_CONTEXT m_GlobalContext;

private:
    _COM_FILE_CONTEXT(const _COM_FILE_CONTEXT &);
    void operator =(const _COM_FILE_CONTEXT &);
} COM_FILE_CONTEXT;

typedef CDeque<COM_FILE_CONTEXT, offsetof(COM_FILE_CONTEXT, m_Linkage)> CComFileDeque;
typedef CDequeIterator<COM_FILE_CONTEXT, offsetof(COM_FILE_CONTEXT, m_Linkage)> CComFileDequeIterator;

typedef struct _COM_GLOBAL_CONTEXT
{
    _COM_GLOBAL_CONTEXT() { }
    ~_COM_GLOBAL_CONTEXT() { m_FileContextList.ClearAndDeleteAll(); }

    // Temporary holding buffer for the filename until the first COM server entry is
    // found, at which time a COM_FILE_CONTEXT is allocated and the filename moved to it.
    CSmallStringBuffer m_FileNameBuffer;
    CComFileDeque m_FileContextList;
    CTinyStringBuffer m_FirstShimNameBuffer;
    ULONG m_FirstShimNameOffset;
    ULONG m_FirstShimNameLength;

    // When the first clrClass entry is created, its file context is written here for
    // easy access in the future.  It will exist in the normal list of files as well,
    // however, and will get cleaned up when the file list goes away.
    PCOM_FILE_CONTEXT m_MscoreeFileContext;

    struct _CallBackLocalsStruct
    {
        CStringBuffer FileNameBuffer;
        CSmallStringBuffer VersionIndependentComClassIdBuffer;
        CStringBuffer TempBuffer;
        CSmallStringBuffer ProgIdBuffer;
        CSmallStringBuffer RuntimeVersionBuffer;
        CSmallStringBuffer NameBuffer;
    } CallbackLocals;

private:
    _COM_GLOBAL_CONTEXT(const _COM_GLOBAL_CONTEXT &);
    void operator =(const _COM_GLOBAL_CONTEXT &);
} COM_GLOBAL_CONTEXT;


VOID
__fastcall
SxspComClassRedirectionContributorCallback(
    PACTCTXCTB_CALLBACK_DATA Data
    )
{
    FN_TRACE();

    PGUID_SECTION_GENERATION_CONTEXT GSGenContext = (PGUID_SECTION_GENERATION_CONTEXT) Data->Header.ActCtxGenContext;
    CSmartPtr<COM_GLOBAL_CONTEXT> ComGlobalContext;
    ULONG ThreadingModel = 0;
    GUID ReferenceClsid;
    GUID ConfiguredClsid;
    GUID ImplementedClsid;
    GUID TypeLibraryId = GUID_NULL;
    GUID iid;
    bool fFound = false;
    SIZE_T cb = 0;

    if (GSGenContext != NULL)
        ComGlobalContext.AttachNoDelete((PCOM_GLOBAL_CONTEXT) ::SxsGetGuidSectionGenerationContextCallbackContext(GSGenContext));

    switch (Data->Header.Reason)
    {
    case ACTCTXCTB_CBREASON_ACTCTXGENBEGINNING:
        Data->GenBeginning.Success = FALSE;

        INTERNAL_ERROR_CHECK(ComGlobalContext == NULL);
        INTERNAL_ERROR_CHECK(GSGenContext == NULL);

        IFW32FALSE_EXIT(ComGlobalContext.Win32Allocate(__FILE__, __LINE__));

        ComGlobalContext->m_FirstShimNameOffset = 0;
        ComGlobalContext->m_FirstShimNameLength = 0;
        ComGlobalContext->m_MscoreeFileContext = NULL;

        IFW32FALSE_EXIT(
            ::SxsInitGuidSectionGenerationContext(
                &GSGenContext,
                ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_FORMAT_WHISTLER,
                &::SxspComClassRedirectionGuidSectionGenerationCallback,
                ComGlobalContext));

        ComGlobalContext.Detach();

        Data->Header.ActCtxGenContext = GSGenContext;
        Data->GenBeginning.Success = TRUE;

        break;

    case ACTCTXCTB_CBREASON_ACTCTXGENENDED:

        ::SxsDestroyGuidSectionGenerationContext(GSGenContext);
        FUSION_DELETE_SINGLETON(ComGlobalContext.Detach());
        break;

    case ACTCTXCTB_CBREASON_ALLPARSINGDONE:
        Data->AllParsingDone.Success = FALSE;

        if (GSGenContext != NULL)
            IFW32FALSE_EXIT(::SxsDoneModifyingGuidSectionGenerationContext(GSGenContext));

        Data->AllParsingDone.Success = TRUE;
        break;

    case ACTCTXCTB_CBREASON_GETSECTIONSIZE:
        Data->GetSectionSize.Success = FALSE;
        // Someone shouldn't be asking for the section size if this is a parse-only
        // run.  These two asserts should be equivalent...
        INTERNAL_ERROR_CHECK(Data->Header.ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT);
        INTERNAL_ERROR_CHECK(GSGenContext != NULL);
        IFW32FALSE_EXIT(::SxsGetGuidSectionGenerationContextSectionSize(GSGenContext, &Data->GetSectionSize.SectionSize));
        Data->GetSectionSize.Success = TRUE;
        break;

    case ACTCTXCTB_CBREASON_ELEMENTPARSED:
        Data->ElementParsed.Success = FALSE;

        if ((Data->ElementParsed.ParseContext->XMLElementDepth == 2) &&
            (::FusionpCompareStrings(
                    Data->ElementParsed.ParseContext->ElementPath,
                    Data->ElementParsed.ParseContext->ElementPathCch,
                    L"urn:schemas-microsoft-com:asm.v1^assembly!urn:schemas-microsoft-com:asm.v1^file",
                    NUMBER_OF(L"urn:schemas-microsoft-com:asm.v1^assembly!urn:schemas-microsoft-com:asm.v1^file") - 1,
                    false) == 0))
        {
            INTERNAL_ERROR_CHECK2(
                ComGlobalContext != NULL,
                "context NULL in callback.");

            CStringBuffer &FileNameBuffer = ComGlobalContext->CallbackLocals.FileNameBuffer;
            SIZE_T cbBytesWritten = 0;

            // capture the name of the file
            IFW32FALSE_EXIT(
                ::SxspGetAttributeValue(
                    0,
                    &s_AttributeName_name,
                    &Data->ElementParsed,
                    fFound,
                    sizeof(FileNameBuffer),
                    &FileNameBuffer,
                    cbBytesWritten,
                    NULL,
                    NULL));

            // If there's no NAME attribute, someone else will puke; we'll handle it
            // gracefully.
            if (fFound || (FileNameBuffer.Cch() == 0))
            {
                IFW32FALSE_EXIT(ComGlobalContext->m_FileNameBuffer.Win32Assign(FileNameBuffer));
            }
        }
        else if (
            (Data->ElementParsed.ParseContext->XMLElementDepth == 3) &&
            (::FusionpCompareStrings(
                    Data->ElementParsed.ParseContext->ElementPath,
                    Data->ElementParsed.ParseContext->ElementPathCch,
                    L"urn:schemas-microsoft-com:asm.v1^assembly!urn:schemas-microsoft-com:asm.v1^file!urn:schemas-microsoft-com:asm.v1^comClass",
                    NUMBER_OF(L"urn:schemas-microsoft-com:asm.v1^assembly!urn:schemas-microsoft-com:asm.v1^file!urn:schemas-microsoft-com:asm.v1^comClass") - 1,
                    false) == 0))
        {
            INTERNAL_ERROR_CHECK2(
                ComGlobalContext != NULL,
                "COM global context NULL while processing comClass tag");

            CSmallStringBuffer &VersionIndependentComClassIdBuffer = ComGlobalContext->CallbackLocals.VersionIndependentComClassIdBuffer;
            VersionIndependentComClassIdBuffer.Clear();
            CSmartPtr<COM_SERVER_ENTRY> Entry;
            CSmartPtr<COM_FILE_CONTEXT> FileContext;
            CStringBuffer &TempBuffer = ComGlobalContext->CallbackLocals.TempBuffer;
            TempBuffer.Clear();
            CSmallStringBuffer &ProgIdBuffer = ComGlobalContext->CallbackLocals.ProgIdBuffer;
            ProgIdBuffer.Clear();

            IFW32FALSE_EXIT(
                ::SxspGetAttributeValue(
                    SXSP_GET_ATTRIBUTE_VALUE_FLAG_REQUIRED_ATTRIBUTE,
                    &s_AttributeName_clsid,
                    &Data->ElementParsed,
                    fFound,
                    sizeof(VersionIndependentComClassIdBuffer),
                    &VersionIndependentComClassIdBuffer,
                    cb,
                    NULL,
                    0));

            INTERNAL_ERROR_CHECK(fFound);

            IFW32FALSE_EXIT(::SxspParseGUID(VersionIndependentComClassIdBuffer,
                                            VersionIndependentComClassIdBuffer.Cch(),
                                            ReferenceClsid));

            IFW32FALSE_EXIT(
                ::SxspGetAttributeValue(
                    0,
                    &s_AttributeName_threadingModel,
                    &Data->ElementParsed,
                    fFound,
                    sizeof(TempBuffer),
                    &TempBuffer,
                    cb,
                    NULL,
                    0));

            if (fFound)
                IFW32FALSE_EXIT(::SxspParseThreadingModel(TempBuffer, TempBuffer.Cch(), &ThreadingModel));
            else
                ThreadingModel = ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_THREADING_MODEL_SINGLE;

            IFW32FALSE_EXIT(
                ::SxspGetAttributeValue(
                    0,
                    &s_AttributeName_progid,
                    &Data->ElementParsed,
                    fFound,
                    sizeof(ProgIdBuffer),
                    &ProgIdBuffer,
                    cb,
                    NULL,
                    0));

            IFW32FALSE_EXIT(
                ::SxspGetAttributeValue(
                    0,
                    &s_AttributeName_tlbid,
                    &Data->ElementParsed,
                    fFound,
                    sizeof(TempBuffer),
                    &TempBuffer,
                    cb,
                    NULL,
                    0));

            if (fFound)
                IFW32FALSE_EXIT(::SxspParseGUID(TempBuffer, TempBuffer.Cch(), TypeLibraryId));
            else
                TypeLibraryId = GUID_NULL;

            // That was sufficient if we are generating a context.
            if (Data->Header.ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT)
            {
                BOOL fNewAllocate = FALSE;
                IFW32FALSE_EXIT(Data->Header.ClsidMappingContext->Map->MapReferenceClsidToConfiguredClsid(
                            &ReferenceClsid,
                            Data->ElementParsed.AssemblyContext,
                            &ConfiguredClsid,
                            &ImplementedClsid));

                // See if we already have a file context; if we do not, allocate one.
                if (ComGlobalContext->m_FileNameBuffer.Cch() != 0)
                {
                    IFW32FALSE_EXIT(FileContext.Win32Allocate(__FILE__, __LINE__));
                    fNewAllocate = TRUE;

                    IFW32FALSE_EXIT(FileContext->m_FileNameBuffer.Win32Assign(ComGlobalContext->m_FileNameBuffer));
                    ComGlobalContext->m_FileContextList.AddToHead(FileContext.DetachAndHold());
                    FileContext->m_GlobalContext = ComGlobalContext;
                }
                else
                {
                    CComFileDequeIterator Iter(&ComGlobalContext->m_FileContextList);
                    Iter.Reset();
                    FileContext.AttachNoDelete(Iter);
                }

                ASSERT(FileContext != NULL);

                IFW32FALSE_EXIT(Entry.Win32Allocate(__FILE__, __LINE__));

                Entry->m_ShimType = ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM_TYPE_OTHER;
                Entry->m_ReferenceClsid = ReferenceClsid;
                Entry->m_ConfiguredClsid = ConfiguredClsid;
                Entry->m_ImplementedClsid = ImplementedClsid;
                Entry->m_TypeLibraryId = TypeLibraryId;
                Entry->m_ThreadingModel = ThreadingModel;
                Entry->m_FileContext = FileContext.DetachAndHold();
                IFW32FALSE_EXIT(Entry->m_ProgIdBuffer.Win32Assign(ProgIdBuffer));

                IFW32FALSE_EXIT(
                    ::SxsAddGuidToGuidSectionGenerationContext(
                        (PGUID_SECTION_GENERATION_CONTEXT) Data->ElementParsed.Header.ActCtxGenContext,
                        &ReferenceClsid,
                        Entry,
                        Data->ElementParsed.AssemblyContext->AssemblyRosterIndex,
                        ERROR_SXS_DUPLICATE_CLSID));

                FileContext->m_ServerList.AddToHead(Entry.Detach());

                // And we add another, indexed by the configured clsid
                IFW32FALSE_EXIT(Entry.Win32Allocate(__FILE__, __LINE__));

                Entry->m_ReferenceClsid = ReferenceClsid;
                Entry->m_ConfiguredClsid = ConfiguredClsid;
                Entry->m_ImplementedClsid = ImplementedClsid;
                Entry->m_TypeLibraryId = TypeLibraryId;
                Entry->m_ThreadingModel = ThreadingModel;
                Entry->m_FileContext = FileContext;
                IFW32FALSE_EXIT(Entry->m_ProgIdBuffer.Win32Assign(ProgIdBuffer));

                IFW32FALSE_EXIT(
                    ::SxsAddGuidToGuidSectionGenerationContext(
                        (PGUID_SECTION_GENERATION_CONTEXT) Data->ElementParsed.Header.ActCtxGenContext,
                        &ConfiguredClsid,
                        Entry,
                        Data->ElementParsed.AssemblyContext->AssemblyRosterIndex,
                        ERROR_SXS_DUPLICATE_CLSID));

                FileContext->m_ServerList.AddToHead(Entry.Detach());
            }
        }
        else if (
            (Data->ElementParsed.ParseContext->XMLElementDepth == 3) &&
            (::FusionpCompareStrings(
                    Data->ElementParsed.ParseContext->ElementPath,
                    Data->ElementParsed.ParseContext->ElementPathCch,
                    L"urn:schemas-microsoft-com:asm.v1^assembly!urn:schemas-microsoft-com:asm.v1^file!urn:schemas-microsoft-com:asm.v1^comInterfaceProxyStub",
                    NUMBER_OF(L"urn:schemas-microsoft-com:asm.v1^assembly!urn:schemas-microsoft-com:asm.v1^file!urn:schemas-microsoft-com:asm.v1^comInterfaceProxyStub") - 1,
                    false) == 0))
        {
            INTERNAL_ERROR_CHECK2(
                ComGlobalContext != NULL,
                "COM global context NULL while processing comInterfaceProxyStub tag");

            CSmartPtr<COM_SERVER_ENTRY> Entry;
            CSmartPtr<COM_FILE_CONTEXT> FileContext;
            CStringBuffer &TempBuffer = ComGlobalContext->CallbackLocals.TempBuffer;
            TempBuffer.Clear();

            IFW32FALSE_EXIT(
                ::SxspGetAttributeValue(
                    SXSP_GET_ATTRIBUTE_VALUE_FLAG_REQUIRED_ATTRIBUTE,
                    &s_AttributeName_iid,
                    &Data->ElementParsed,
                    fFound,
                    sizeof(iid),
                    &iid,
                    cb,
                    &::SxspValidateGuidAttribute,
                    0));

            IFW32FALSE_EXIT(
                ::SxspGetAttributeValue(
                    0,
                    &s_AttributeName_proxyStubClsid32,
                    &Data->ElementParsed,
                    fFound,
                    sizeof(ReferenceClsid),
                    &ReferenceClsid,
                    cb,
                    &::SxspValidateGuidAttribute,
                    0));

            if (!fFound)
                ReferenceClsid = iid;

            ThreadingModel = ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_THREADING_MODEL_BOTH;

            // That was sufficient if we are generating a context.
            if (Data->Header.ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT)
            {
                IFW32FALSE_EXIT(Data->Header.ClsidMappingContext->Map->MapReferenceClsidToConfiguredClsid(
                            &ReferenceClsid,
                            Data->ElementParsed.AssemblyContext,
                            &ConfiguredClsid,
                            &ImplementedClsid));

                // See if we already have a file context; if we do not, allocate one.
                if (ComGlobalContext->m_FileNameBuffer.Cch() != 0)
                {
                    IFW32FALSE_EXIT(FileContext.Win32Allocate(__FILE__, __LINE__));
                    IFW32FALSE_EXIT(FileContext->m_FileNameBuffer.Win32Assign(ComGlobalContext->m_FileNameBuffer));
                    ComGlobalContext->m_FileContextList.AddToHead(FileContext.DetachAndHold());
                    FileContext->m_GlobalContext = ComGlobalContext;
                }
                else
                {
                    CComFileDequeIterator Iter(&ComGlobalContext->m_FileContextList);
                    Iter.Reset();                    
                    FileContext.AttachNoDelete(Iter);
                }

                INTERNAL_ERROR_CHECK(FileContext != NULL);

                IFW32FALSE_EXIT(Entry.Win32Allocate(__FILE__, __LINE__));

                Entry->m_ShimType = ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM_TYPE_OTHER;
                Entry->m_ReferenceClsid = ReferenceClsid;
                Entry->m_ConfiguredClsid = ConfiguredClsid;
                Entry->m_ImplementedClsid = ImplementedClsid;
                Entry->m_TypeLibraryId = GUID_NULL;
                Entry->m_ThreadingModel = ThreadingModel;
                Entry->m_FileContext = FileContext;
                Entry->m_ProgIdBuffer.Clear();

                IFW32FALSE_EXIT(
                    ::SxsAddGuidToGuidSectionGenerationContext(
                        (PGUID_SECTION_GENERATION_CONTEXT) Data->ElementParsed.Header.ActCtxGenContext,
                        &ReferenceClsid,
                        Entry,
                        Data->ElementParsed.AssemblyContext->AssemblyRosterIndex,
                        ERROR_SXS_DUPLICATE_CLSID));

                FileContext->m_ServerList.AddToHead(Entry.Detach());

                // We don't want to delete the FileContext on exit any more if it was newly allocated
                FileContext.DetachAndHold();

                // And we add another, indexed by the configured clsid
                IFW32FALSE_EXIT(Entry.Win32Allocate(__FILE__, __LINE__));

                Entry->m_ReferenceClsid = ReferenceClsid;
                Entry->m_ConfiguredClsid = ConfiguredClsid;
                Entry->m_ImplementedClsid = ImplementedClsid;
                Entry->m_TypeLibraryId = GUID_NULL;
                Entry->m_ThreadingModel = ThreadingModel;
                Entry->m_FileContext = FileContext;
                Entry->m_ProgIdBuffer.Clear();

                IFW32FALSE_EXIT(
                    ::SxsAddGuidToGuidSectionGenerationContext(
                        (PGUID_SECTION_GENERATION_CONTEXT) Data->ElementParsed.Header.ActCtxGenContext,
                        &ConfiguredClsid,
                        Entry,
                        Data->ElementParsed.AssemblyContext->AssemblyRosterIndex,
                        ERROR_SXS_DUPLICATE_CLSID));

                FileContext->m_ServerList.AddToHead(Entry.Detach());
            }
        }
        else if (
            (Data->ElementParsed.ParseContext->XMLElementDepth == 2) &&
            (::FusionpCompareStrings(
                    Data->ElementParsed.ParseContext->ElementPath,
                    Data->ElementParsed.ParseContext->ElementPathCch,
                    L"urn:schemas-microsoft-com:asm.v1^assembly!urn:schemas-microsoft-com:asm.v1^clrClass",
                    NUMBER_OF(L"urn:schemas-microsoft-com:asm.v1^assembly!urn:schemas-microsoft-com:asm.v1^clrClass") - 1,
                    false) == 0))
        {

            INTERNAL_ERROR_CHECK2(
                ComGlobalContext != NULL,
                "COM global context NULL while processing ndpClass tag");

            CSmallStringBuffer &VersionIndependentComClassIdBuffer = ComGlobalContext->CallbackLocals.VersionIndependentComClassIdBuffer;
            VersionIndependentComClassIdBuffer.Clear();
            CSmartPtr<COM_SERVER_ENTRY> Entry;
            CSmartPtr<COM_FILE_CONTEXT> FileContext;
            CStringBuffer &TempBuffer = ComGlobalContext->CallbackLocals.TempBuffer;
            TempBuffer.Clear();
            CSmallStringBuffer &ProgIdBuffer = ComGlobalContext->CallbackLocals.ProgIdBuffer;
            ProgIdBuffer.Clear();
            CSmallStringBuffer &RuntimeVersionBuffer = ComGlobalContext->CallbackLocals.RuntimeVersionBuffer;
            RuntimeVersionBuffer.Clear();
            CSmallStringBuffer &NameBuffer = ComGlobalContext->CallbackLocals.NameBuffer;
            NameBuffer.Clear();
            bool fIsFirstShim = false;

            IFW32FALSE_EXIT(
                ::SxspGetAttributeValue(
                    SXSP_GET_ATTRIBUTE_VALUE_FLAG_REQUIRED_ATTRIBUTE,
                    &s_AttributeName_name,
                    &Data->ElementParsed,
                    fFound,
                    sizeof(NameBuffer),
                    &NameBuffer,
                    cb,
                    NULL,
                    0));

            INTERNAL_ERROR_CHECK(fFound);

            IFW32FALSE_EXIT(
                ::SxspGetAttributeValue(
                    SXSP_GET_ATTRIBUTE_VALUE_FLAG_REQUIRED_ATTRIBUTE,
                    &s_AttributeName_clsid,
                    &Data->ElementParsed,
                    fFound,
                    sizeof(VersionIndependentComClassIdBuffer),
                    &VersionIndependentComClassIdBuffer,
                    cb,
                    NULL,
                    0));

            INTERNAL_ERROR_CHECK(fFound);

            IFW32FALSE_EXIT(
                ::SxspParseGUID(
                    VersionIndependentComClassIdBuffer,
                    VersionIndependentComClassIdBuffer.Cch(),
                    ReferenceClsid));

            IFW32FALSE_EXIT(
                ::SxspGetAttributeValue(
                    0,
                    &s_AttributeName_threadingModel,
                    &Data->ElementParsed,
                    fFound,
                    sizeof(TempBuffer),
                    &TempBuffer,
                    cb,
                    NULL,
                    0));

            if (fFound)
                IFW32FALSE_EXIT(::SxspParseThreadingModel(TempBuffer, TempBuffer.Cch(), &ThreadingModel));
            else
                ThreadingModel = ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_THREADING_MODEL_BOTH;

            IFW32FALSE_EXIT(
                ::SxspGetAttributeValue(
                    0,
                    &s_AttributeName_progid,
                    &Data->ElementParsed,
                    fFound,
                    sizeof(ProgIdBuffer),
                    &ProgIdBuffer,
                    cb,
                    NULL,
                    0));

            IFW32FALSE_EXIT(
                ::SxspGetAttributeValue(
                    0,
                    &s_AttributeName_tlbid,
                    &Data->ElementParsed,
                    fFound,
                    sizeof(TempBuffer),
                    &TempBuffer,
                    cb,
                    NULL,
                    0));

            if (fFound)
                IFW32FALSE_EXIT(::SxspParseGUID(TempBuffer, TempBuffer.Cch(), TypeLibraryId));
            else
                TypeLibraryId = GUID_NULL;

            IFW32FALSE_EXIT(
                ::SxspGetAttributeValue(
                    0,
                    &s_AttributeName_runtimeVersion,
                    &Data->ElementParsed,
                    fFound,
                    sizeof(RuntimeVersionBuffer),
                    &RuntimeVersionBuffer,
                    cb,
                    NULL,
                    0));

            // That was sufficient if we are generating a context.
            if (Data->Header.ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT)
            {
                if (ComGlobalContext->m_FirstShimNameBuffer.Cch() == 0)
                {
                    fIsFirstShim = true;
                    IFW32FALSE_EXIT(ComGlobalContext->m_FirstShimNameBuffer.Win32Assign(L"MSCOREE.DLL", 11));
                }
                else
                {
                    IFW32FALSE_EXIT(
                        ComGlobalContext->m_FirstShimNameBuffer.Win32Equals(
                            L"MSCOREE.DLL", 11,
                            fIsFirstShim,
                            true));
                }

                IFW32FALSE_EXIT(
                    Data->Header.ClsidMappingContext->Map->MapReferenceClsidToConfiguredClsid(
                        &ReferenceClsid,
                        Data->ElementParsed.AssemblyContext,
                        &ConfiguredClsid,
                        &ImplementedClsid));

                // If we don't already have a file context for mscoree, then we have to create a new one.
                if (ComGlobalContext->m_MscoreeFileContext == NULL)
                {
                    IFW32FALSE_EXIT(FileContext.Win32Allocate(__FILE__, __LINE__));
                    IFW32FALSE_EXIT(FileContext->m_FileNameBuffer.Win32Assign("mscoree.dll", 11));

                    ComGlobalContext->m_FileContextList.AddToHead(FileContext.DetachAndHold());
                    ComGlobalContext->m_MscoreeFileContext = FileContext;
                    FileContext->m_GlobalContext = ComGlobalContext;
                }
                else
                {
                    FileContext.AttachNoDelete(ComGlobalContext->m_MscoreeFileContext);
                }


                INTERNAL_ERROR_CHECK(FileContext != NULL);

                IFW32FALSE_EXIT(Entry.Win32Allocate(__FILE__, __LINE__));

                Entry->m_ShimType = ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM_TYPE_CLR_CLASS;
                Entry->m_ReferenceClsid = ReferenceClsid;
                Entry->m_ConfiguredClsid = ConfiguredClsid;
                Entry->m_ImplementedClsid = ImplementedClsid;
                Entry->m_TypeLibraryId = TypeLibraryId;
                Entry->m_ThreadingModel = ThreadingModel;
                Entry->m_FileContext = FileContext;
                IFW32FALSE_EXIT(Entry->m_ProgIdBuffer.Win32Assign(ProgIdBuffer));
                IFW32FALSE_EXIT(Entry->m_ShimNameBuffer.Win32Assign(L"MSCOREE.DLL", 11));
                IFW32FALSE_EXIT(Entry->m_RuntimeVersionBuffer.Win32Assign(RuntimeVersionBuffer));
                IFW32FALSE_EXIT(Entry->m_TypeNameBuffer.Win32Assign(NameBuffer));
                Entry->m_IsFirstShim = fIsFirstShim;

                IFW32FALSE_EXIT(
                    ::SxsAddGuidToGuidSectionGenerationContext(
                        (PGUID_SECTION_GENERATION_CONTEXT) Data->ElementParsed.Header.ActCtxGenContext,
                        &ReferenceClsid,
                        Entry,
                        Data->ElementParsed.AssemblyContext->AssemblyRosterIndex,
                        ERROR_SXS_DUPLICATE_CLSID));

                FileContext->m_ServerList.AddToHead(Entry.Detach());

                // We don't need to delete FileContext on exit any more...
                FileContext.DetachAndHold();

                // And we add another, indexed by the configured clsid
                IFW32FALSE_EXIT(Entry.Win32Allocate(__FILE__, __LINE__));

                Entry->m_ShimType = ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM_TYPE_CLR_CLASS;
                Entry->m_ReferenceClsid = ReferenceClsid;
                Entry->m_ConfiguredClsid = ConfiguredClsid;
                Entry->m_ImplementedClsid = ImplementedClsid;
                Entry->m_TypeLibraryId = TypeLibraryId;
                Entry->m_ThreadingModel = ThreadingModel;
                Entry->m_FileContext = FileContext;
                IFW32FALSE_EXIT(Entry->m_ProgIdBuffer.Win32Assign(ProgIdBuffer));
                IFW32FALSE_EXIT(Entry->m_ShimNameBuffer.Win32Assign(L"MSCOREE.DLL", 11));
                IFW32FALSE_EXIT(Entry->m_TypeNameBuffer.Win32Assign(NameBuffer));
                IFW32FALSE_EXIT(Entry->m_RuntimeVersionBuffer.Win32Assign(RuntimeVersionBuffer));
                Entry->m_IsFirstShim = fIsFirstShim;

                IFW32FALSE_EXIT(
                    ::SxsAddGuidToGuidSectionGenerationContext(
                        (PGUID_SECTION_GENERATION_CONTEXT) Data->ElementParsed.Header.ActCtxGenContext,
                        &ConfiguredClsid,
                        Entry,
                        Data->ElementParsed.AssemblyContext->AssemblyRosterIndex,
                        ERROR_SXS_DUPLICATE_CLSID));

                FileContext->m_ServerList.AddToHead(Entry.Detach());
            }
        }

        // Everything's groovy!
        Data->ElementParsed.Success = TRUE;
        break;

    case ACTCTXCTB_CBREASON_GETSECTIONDATA:
        Data->GetSectionData.Success = FALSE;
        INTERNAL_ERROR_CHECK(GSGenContext != NULL);
        INTERNAL_ERROR_CHECK(Data->Header.ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT);
        IFW32FALSE_EXIT(::SxsGetGuidSectionGenerationContextSectionData(GSGenContext, Data->GetSectionData.SectionSize, Data->GetSectionData.SectionDataStart, NULL));
        Data->GetSectionData.Success = TRUE;
        break;
    }
Exit:
    ;
}

BOOL
SxspComClassRedirectionGuidSectionGenerationCallback(
    PVOID Context,
    ULONG Reason,
    PVOID CallbackData
    )
{
    BOOL fSuccess = FALSE;

    FN_TRACE_WIN32(fSuccess);

    PCOM_GLOBAL_CONTEXT ComGlobalContext = (PCOM_GLOBAL_CONTEXT) Context;

    INTERNAL_ERROR_CHECK(CallbackData != NULL);

    switch (Reason)
    {
    case GUID_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETUSERDATASIZE:
        {
            INTERNAL_ERROR_CHECK(ComGlobalContext != NULL);

            PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATASIZE CBData =
                (PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATASIZE) CallbackData;
            CComFileDequeIterator Iter(&ComGlobalContext->m_FileContextList);

            CBData->DataSize = 0;

            // If we have a mscoree shim, add its size to the user data buffer area.
            if (ComGlobalContext->m_FirstShimNameBuffer.Cch() != 0)
                CBData->DataSize += ((ComGlobalContext->m_FirstShimNameBuffer.Cch() + 1) * sizeof(WCHAR));

            for (Iter.Reset(); Iter.More(); Iter.Next())
            {
                CBData->DataSize += ((Iter->m_FileNameBuffer.Cch() + 1) * sizeof(WCHAR));
            }

            break;
        }

    case GUID_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETUSERDATA:
        {
            INTERNAL_ERROR_CHECK( ComGlobalContext != NULL );

            PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATA CBData =
                (PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATA) CallbackData;
            SIZE_T BytesWritten = 0;
            SIZE_T BytesLeft = CBData->BufferSize;
            PWSTR Cursor = (PWSTR) CBData->Buffer;
            CComFileDequeIterator Iter(&ComGlobalContext->m_FileContextList);

            if (ComGlobalContext->m_FirstShimNameBuffer.Cch() != 0)
            {
                IFW32FALSE_EXIT(
                    ComGlobalContext->m_FirstShimNameBuffer.Win32CopyIntoBuffer(
                        &Cursor,
                        &BytesLeft,
                        &BytesWritten,
                        CBData->SectionHeader,
                        &ComGlobalContext->m_FirstShimNameOffset,
                        &ComGlobalContext->m_FirstShimNameLength));
            }

            for (Iter.Reset(); Iter.More(); Iter.Next())
            {
                IFW32FALSE_EXIT(
                    Iter->m_FileNameBuffer.Win32CopyIntoBuffer(
                        &Cursor,
                        &BytesLeft,
                        &BytesWritten,
                        CBData->SectionHeader,
                        &Iter->m_Offset,
                        NULL));                     // the length is tracked elsewhere
            }

            CBData->BytesWritten = BytesWritten;

            break;
        }

    case GUID_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_ENTRYDELETED:
        {
            INTERNAL_ERROR_CHECK( ComGlobalContext != NULL );

            PGUID_SECTION_GENERATION_CONTEXT_CBDATA_ENTRYDELETED CBData =
                (PGUID_SECTION_GENERATION_CONTEXT_CBDATA_ENTRYDELETED) CallbackData;
            PCOM_SERVER_ENTRY Entry = (PCOM_SERVER_ENTRY) CBData->DataContext;

            if (Entry != NULL)
            {
                if (Entry->m_FileContext != NULL)
                {
                    Entry->m_FileContext->m_ServerList.Remove(Entry);

                    //
                    // If that was the last entry in the file's server list, destroy the
                    // file entry (remove from global context, free
                    //
                    if (Entry->m_FileContext->m_ServerList.IsEmpty())
                    {
                        ComGlobalContext->m_FileContextList.Remove(Entry->m_FileContext);
                        FUSION_DELETE_SINGLETON(Entry->m_FileContext);
                        Entry->m_FileContext = NULL;
                    }
                }

                FUSION_DELETE_SINGLETON(Entry);
            }

            break;
        }

    case GUID_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETDATASIZE:
        {
            PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETDATASIZE CBData =
                (PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETDATASIZE) CallbackData;
            PCOM_SERVER_ENTRY Entry = (PCOM_SERVER_ENTRY) CBData->DataContext;

            CBData->DataSize = sizeof(ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION);

            if (Entry->m_ProgIdBuffer.Cch() != 0)
                CBData->DataSize += ((Entry->m_ProgIdBuffer.Cch() + 1) * sizeof(WCHAR));

            if (Entry->m_ShimNameBuffer.Cch() != 0)
            {
                CBData->DataSize += sizeof(ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM);

                if (Entry->m_RuntimeVersionBuffer.Cch() != 0)
                    CBData->DataSize += ((Entry->m_RuntimeVersionBuffer.Cch() + 1) * sizeof(WCHAR));

                if (!Entry->m_IsFirstShim)
                    CBData->DataSize += ((Entry->m_ShimNameBuffer.Cch() + 1) * sizeof(WCHAR));

                if (Entry->m_TypeNameBuffer.Cch() != 0)
                    CBData->DataSize += ((Entry->m_TypeNameBuffer.Cch() + 1) * sizeof(WCHAR));
            }

            break;
        }

    case GUID_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETDATA:
        {
            PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETDATA CBData =
                (PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETDATA) CallbackData;
            PCOM_SERVER_ENTRY Entry = (PCOM_SERVER_ENTRY) CBData->DataContext;
            PACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION Info;
            PACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM ShimInfo = NULL;
            PVOID Cursor = CBData->Buffer;

            SIZE_T BytesLeft = CBData->BufferSize;
            SIZE_T BytesWritten = 0;

            ALLOCATE_BUFFER_SPACE_TYPE(ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION, Cursor, BytesLeft, BytesWritten, Info);

            Info->Size = sizeof(ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION);
            Info->Flags = 0;
            Info->ThreadingModel = Entry->m_ThreadingModel;
            Info->ReferenceClsid = Entry->m_ReferenceClsid;
            Info->ConfiguredClsid = Entry->m_ConfiguredClsid;
            Info->ImplementedClsid = Entry->m_ImplementedClsid;
            Info->TypeLibraryId = Entry->m_TypeLibraryId;

            if (Entry->m_ShimNameBuffer.Cch() != 0)
            {
                PWSTR ShimName = NULL;
                SIZE_T OldBytesWritten = BytesWritten;
                SIZE_T ShimDataSize = 0;

                ALLOCATE_BUFFER_SPACE_TYPE(ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM, Cursor, BytesLeft, BytesWritten, ShimInfo);

                IFW32FALSE_EXIT(
                    Entry->m_TypeNameBuffer.Win32CopyIntoBuffer(
                        (PWSTR *) &Cursor,
                        &BytesLeft,
                        &BytesWritten,
                        ShimInfo,
                        &ShimInfo->TypeOffset,
                        &ShimInfo->TypeLength));

                ShimInfo->ModuleLength = static_cast<ULONG>(Entry->m_FileContext->m_FileNameBuffer.Cch() * sizeof(WCHAR));
                ShimInfo->ModuleOffset = Entry->m_FileContext->m_Offset;

                IFW32FALSE_EXIT(
                    Entry->m_RuntimeVersionBuffer.Win32CopyIntoBuffer(
                        (PWSTR *) &Cursor,
                        &BytesLeft,
                        &BytesWritten,
                        ShimInfo,
                        &ShimInfo->ShimVersionOffset,
                        &ShimInfo->ShimVersionLength));

                ShimDataSize = BytesWritten - OldBytesWritten;

                if (Entry->m_IsFirstShim)
                {
                    Info->ModuleOffset = Entry->m_FileContext->m_GlobalContext->m_FirstShimNameOffset;
                    Info->ModuleLength = Entry->m_FileContext->m_GlobalContext->m_FirstShimNameLength;
                }
                else
                {
                    IFW32FALSE_EXIT(
                        Entry->m_ShimNameBuffer.Win32CopyIntoBuffer(
                            (PWSTR *) &Cursor,
                            &BytesLeft,
                            &BytesWritten,
                            CBData->SectionHeader,
                            &Info->ModuleOffset,
                            &Info->ModuleLength));
                }

                ShimInfo->Size = sizeof(ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_SHIM);
                ShimInfo->Flags = 0;
                ShimInfo->Type = Entry->m_ShimType;

                ShimInfo->DataLength = 0;
                ShimInfo->DataOffset = 0;

                Info->ShimDataLength = static_cast<ULONG>(ShimDataSize);
                Info->ShimDataOffset = static_cast<ULONG>(((ULONG_PTR) ShimInfo) - ((ULONG_PTR) Info));
            }
            else
            {
                Info->ModuleLength = static_cast<ULONG>(Entry->m_FileContext->m_FileNameBuffer.Cch() * sizeof(WCHAR));
                Info->ModuleOffset = Entry->m_FileContext->m_Offset;
                Info->ShimDataLength = 0;
                Info->ShimDataOffset = 0;
            }

            IFW32FALSE_EXIT(Entry->m_ProgIdBuffer.Win32CopyIntoBuffer(
                (PWSTR *) &Cursor,
                &BytesLeft,
                &BytesWritten,
                Info,
                &Info->ProgIdOffset,
                &Info->ProgIdLength));

            CBData->BytesWritten = BytesWritten;
        }
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

