/*
Copyright (c) Microsoft Corporation
*/
#include "stdinc.h"
#include "actctxgenctx.h"

//
// ISSUE: jonwis 3/9/2002 - This file is full of missing parameter checking.
//
// NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Parameter checking (#1)
// NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - FN_PROLOG/EPILOG (#2)

#define DEFINE_ATTRIBUTE(attributeName, attributeType, typePrefix) \
    { \
        L ## #attributeName, \
        offsetof(CNodeFactory, m_ ## typePrefix ## _ ## attributeName), \
        offsetof(CNodeFactory, m_f ## attributeName ## _ ## Present), \
        &CNodeFactory::XMLParser_Parse_ ## attributeType \
    },

const static ASSEMBLY_VERSION assemblyVersion0 = {0,0,0,0};

typedef enum _in_xml_tag_when_identity_generated_{
    SXS_IN_INVALID_XML_TAG_WHEN_ASSEMBLY_IDENTITY_GENERATED,
    SXS_IN_ASSEMBLY_TAG,
    SXS_IN_DEPENDENCY_TAG
}SXS_IN_XML_TAG_WHEN_IDENTITY_GENERATED;

VOID
SxspDbgPrintXmlNodeInfo(
    ULONG Level,
    XML_NODE_INFO *pNode
 );

PCSTR
SxspFormatXmlNodeType(
    DWORD dwType
 );

struct EventIdErrorPair
{
    DWORD   dwEventId;
    LONG    nError;
};

const static EventIdErrorPair eventIdToErrorMap[] =
{
    #include "messages.hi" // generated from .x file, like .mc
};

// deliberately no extra paranetheses here
#define NODEFACTORY_STRING_AND_LENGTH(x) x, NUMBER_OF(x) - 1

const static SXS_NAME_LENGTH_PAIR IgnoredAttributesInDependencyTagForIdentity[]={
    //maybe more later
    { NODEFACTORY_STRING_AND_LENGTH(L"Description") }
};

const DWORD IGNORED_ATTRIBUTE_NUM_IN_DEPENDENCY_TAG = NUMBER_OF(IgnoredAttributesInDependencyTagForIdentity);

DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(baseInterface);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(clsid);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(description);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(flags);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(hash);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(hashalg);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(helpdir);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(iid);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(language);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(loadFrom);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(manifestVersion);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(metadataSatellite);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(name);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(newVersion);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(numMethods);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(oldVersion);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(optional);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(processorArchitecture);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(progid);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(proxyStubClsid32);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(publicKeyToken);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(publicKey);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(resourceid);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(runtimeVersion);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(size);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(threadingModel);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(tlbid);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(type);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(version);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(versioned);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(apply);

    // How to interpret the parser worker rules here:
    //
    //  First, the state of the parser must match m_xps for the rule to be considered.
    //      If the value eNotParsing is in the table, it matches any current parser
    //      state.  Use this to globally ignore some particular tag type when
    //      combined with a NULL m_pszTag and NULL m_pfn.
    //  Second, the type of the tag from the XML parser must match m_dwType.
    //  If m_pszTag is not NULL, a case-insensitive comparison is done between the
    //      string m_pszTag points to and the tag from the XML parser.  An m_pszTag
    //      value of NULL matches any tag.
    //  If the three criteria match, the worker function is called.  The worker function
    //  pointer may be NULL, in which case no action is taken.  (This is useful for
    //  callbacks from the parser about XML_WHITESPACE where we don't really have to do
    //  anything at all.)
    //

#define DEFINE_TAG_WORKER_IGNOREALLOFTYPE(dwType) { CNodeFactory::eNotParsing, (dwType), NULL, NULL, NULL, 0, 0, 0, NULL }

    //
    //  Notes on use of the DEFINE_TAG_WORKER_ELEMENT() macro:
    //
    //  The first parameter, sourceState, is part of the name of a XMLParseState
    //  enumeration value.  It is concatenated onto "eParsing" to form the name of
    //  the state which the rule will match.
    //
    //  The second parameter is both the text of the tag to match and is used to
    //  form the name of the function to invoke if the rule matches.  The tag is
    //  compared case-insensitively, and the name of the member function invoked
    //  is XMLParser_Element_ followed by the sourceState string followed by another
    //  underscore, followed by the tagName string.  So, for example, the following
    //  rule:
    //
    //  DEFINE_TAG_WORKER_ELEMENT(DepAssy, Version)
    //
    //  says that when the parser is in the eParsingDepAssy state and a "Version"
    //  tag is found, call the function CNodeFactory::XMLParser_Element_DepAssy_Version().
    //

#define DEFINE_TAG_WORKER_ELEMENT(sourceState, tagName) \
    { \
        CNodeFactory::eParsing_ ## sourceState, \
        XML_ELEMENT, \
        SXS_ASSEMBLY_MANIFEST_STD_NAMESPACE, \
        L ## #tagName, \
        s_rg_ ## sourceState ## _ ## tagName ## _attributes, \
        NUMBER_OF(SXS_ASSEMBLY_MANIFEST_STD_NAMESPACE) - 1, \
        NUMBER_OF(L ## #tagName) - 1, \
        NUMBER_OF(s_rg_ ## sourceState ## _ ## tagName ## _attributes), \
        &CNodeFactory::XMLParser_Element_ ## sourceState ## _ ## tagName, \
        CNodeFactory::eParsing_ ## sourceState ## _ ## tagName \
    }

#define DEFINE_TAG_WORKER_ELEMENT_NOCB(sourceState, tagName) \
    { \
        CNodeFactory::eParsing_ ## sourceState, \
        XML_ELEMENT, \
        SXS_ASSEMBLY_MANIFEST_STD_NAMESPACE, \
        L ## #tagName, \
        s_rg_ ## sourceState ## _ ## tagName ## _attributes, \
        NUMBER_OF(SXS_ASSEMBLY_MANIFEST_STD_NAMESPACE) - 1, \
        NUMBER_OF(L ## #tagName) - 1, \
        NUMBER_OF(s_rg_ ## sourceState ## _ ## tagName ## _attributes), \
        NULL, \
        CNodeFactory::eParsing_ ## sourceState ## _ ## tagName \
    }

#define DEFINE_TAG_WORKER_ELEMENT_NONS(sourceState, tagName) \
    { \
        CNodeFactory::eParsing_ ## sourceState, \
        XML_ELEMENT, \
        NULL, \
        L ## #tagName, \
        s_rg_ ## sourceState ## _ ## tagName ## _attributes, \
        0, \
        NUMBER_OF(L ## #tagName) - 1, \
        NUMBER_OF(s_rg_ ## sourceState ## _ ## tagName ## _attributes), \
        &CNodeFactory::XMLParser_Element_ ## sourceState ## _ ## tagName, \
        CNodeFactory::eParsing_ ## sourceState ## _ ## tagName \
    }

#define BEGIN_ELEMENT_LEGAL_ATTRIBUTES(_element) \
const static ELEMENT_LEGAL_ATTRIBUTE s_rg_ ## _element ## _attributes[] = { \
    { ELEMENT_LEGAL_ATTRIBUTE_FLAG_IGNORE, NULL, NULL },

#define DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(attributeName, validatorFlags, validator) { ELEMENT_LEGAL_ATTRIBUTE_FLAG_REQUIRED, &s_AttributeName_ ## attributeName, validator, validatorFlags },
#define DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(attributeName, validatorFlags, validator) { 0, &s_AttributeName_ ## attributeName, validator, validatorFlags },

#define END_ELEMENT_LEGAL_ATTRIBUTES(_element) };

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(manifestVersion, 0, NULL)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_description)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_description)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_noInherit)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_noInherit)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_noInheritable)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_noInheritable)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_assemblyIdentity)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(name, 0, NULL)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(version, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(type, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(processorArchitecture, SXSP_VALIDATE_PROCESSOR_ARCHITECTURE_ATTRIBUTE_FLAG_WILDCARD_ALLOWED, &::SxspValidateProcessorArchitectureAttribute)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(publicKeyToken, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(language, 0, &::SxspValidateLanguageAttribute)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(publicKey, 0, NULL)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_assemblyIdentity)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_dependency)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(optional, 0, &::SxspValidateBoolAttribute)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_dependency)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_dependency_dependentAssembly)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_dependency_dependentAssembly)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_dependency_dependentAssembly_assemblyIdentity)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(name, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(type, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(version, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(processorArchitecture, SXSP_VALIDATE_PROCESSOR_ARCHITECTURE_ATTRIBUTE_FLAG_WILDCARD_ALLOWED, &::SxspValidateProcessorArchitectureAttribute)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(publicKeyToken, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(language, SXSP_VALIDATE_LANGUAGE_ATTRIBUTE_FLAG_WILDCARD_ALLOWED, &::SxspValidateLanguageAttribute)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_dependency_dependentAssembly_assemblyIdentity)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_dependency_dependentAssembly_bindingRedirect)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(oldVersion, 0, NULL)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(newVersion, 0, NULL)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_dependency_dependentAssembly_bindingRedirect)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_file)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(name, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(hash, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(hashalg, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(loadFrom, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(size, 0, &::SxspValidateUnsigned64Attribute)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_file)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_file_comClass)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(clsid, 0, &::SxspValidateGuidAttribute)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(threadingModel, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(progid, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(tlbid, 0, &::SxspValidateGuidAttribute)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(description, 0, NULL)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_file_comClass)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_file_comClass_progid)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_file_comClass_progid)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_clrClass)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(name, 0, NULL)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(clsid, 0, &::SxspValidateGuidAttribute)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(progid, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(tlbid, 0, &::SxspValidateGuidAttribute)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(description, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(runtimeVersion, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(threadingModel, 0, NULL)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_clrClass)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_clrSurrogate)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(clsid, 0, &::SxspValidateGuidAttribute)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(name, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(runtimeVersion, 0, NULL)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_file_clrSurrogate)


BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_clrClass_progid)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_clrClass_progid)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_file_comInterfaceProxyStub)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(iid, 0, &::SxspValidateGuidAttribute)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(tlbid, 0, &::SxspValidateGuidAttribute)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(name, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(numMethods, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(baseInterface, 0, &::SxspValidateGuidAttribute)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(proxyStubClsid32, 0, &::SxspValidateGuidAttribute)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(threadingModel, 0, NULL)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_file_comInterfaceProxyStub)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_file_typelib)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(tlbid, 0, &::SxspValidateGuidAttribute)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(version, 0, NULL)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(helpdir, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(resourceid, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(flags, 0, NULL)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_file_typelib)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_file_windowClass)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(versioned, 0, &::SxspValidateBoolAttribute)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_file_windowClass)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_comInterfaceExternalProxyStub)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(iid, 0, &::SxspValidateGuidAttribute)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(name, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(proxyStubClsid32, 0, &::SxspValidateGuidAttribute)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(tlbid, 0, &::SxspValidateGuidAttribute)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(numMethods, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(baseInterface, 0, &::SxspValidateGuidAttribute)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_assembly_comInterfaceExternalProxyStub)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_configuration)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_configuration)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_configuration_windows)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_configuration_windows)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_configuration_windows_assemblyBinding)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_configuration_windows_assemblyBinding)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_configuration_windows_assemblyBinding_assemblyIdentity)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(name, 0, NULL)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(version, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(type, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(processorArchitecture,  0, &::SxspValidateProcessorArchitectureAttribute)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(publicKeyToken, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(language, 0, &::SxspValidateLanguageAttribute)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_configuration_windows_assemblyBinding_assemblyIdentity)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_configuration_windows_assemblyBinding_dependentAssembly_publisherPolicy)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(apply, 0, &::SxspValidateBoolAttribute)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_configuration_windows_assemblyBinding_dependentAssembly_publisherPolicy)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_configuration_windows_assemblyBinding_publisherPolicy)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(apply, 0, &::SxspValidateBoolAttribute)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_configuration_windows_assemblyBinding_publisherPolicy)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_configuration_windows_assemblyBinding_dependentAssembly)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_configuration_windows_assemblyBinding_dependentAssembly)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_configuration_windows_assemblyBinding_dependentAssembly_assemblyIdentity)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(name, 0, NULL)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(processorArchitecture,  0, &::SxspValidateProcessorArchitectureAttribute)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(type, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(publicKeyToken, 0, NULL)
    DEFINE_ELEMENT_NONS_OPTIONAL_ATTRIBUTE(language, 0, &::SxspValidateLanguageAttribute)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_configuration_windows_assemblyBinding_dependentAssembly_assemblyIdentity)

BEGIN_ELEMENT_LEGAL_ATTRIBUTES(doc_configuration_windows_assemblyBinding_dependentAssembly_bindingRedirect)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(oldVersion, 0, NULL)
    DEFINE_ELEMENT_NONS_REQUIRED_ATTRIBUTE(newVersion, 0, NULL)
END_ELEMENT_LEGAL_ATTRIBUTES(doc_configuration_windows_assemblyBinding_dependentAssembly_bindingRedirect)

static const struct
{
    CNodeFactory::XMLParseState m_xpsOld;
    DWORD m_dwType;
    PCWSTR m_pszNamespace;
    PCWSTR m_pszName;
    PCELEMENT_LEGAL_ATTRIBUTE m_prgLegalAttributes;
    UCHAR m_cchNamespace;           // We use UCHAR here just for greater data density.  Changing this and rebuilding
    UCHAR m_cchName;                // this module should work fine if you really need namespaces or names longer than
                                    // 255 characters.
    UCHAR m_cLegalAttributes;
    CNodeFactory::XMLParserWorkerFunctionPtr m_pfn;
    CNodeFactory::XMLParseState m_xpsNew;
} s_rgWorkers[] =
{
    DEFINE_TAG_WORKER_IGNOREALLOFTYPE(XML_WHITESPACE),
    DEFINE_TAG_WORKER_IGNOREALLOFTYPE(XML_COMMENT),
    DEFINE_TAG_WORKER_ELEMENT(doc, assembly),
    DEFINE_TAG_WORKER_ELEMENT(doc_assembly, assemblyIdentity),
    DEFINE_TAG_WORKER_ELEMENT_NOCB(doc_assembly, description),
    DEFINE_TAG_WORKER_ELEMENT(doc_assembly, noInherit),
    DEFINE_TAG_WORKER_ELEMENT(doc_assembly, noInheritable),
    DEFINE_TAG_WORKER_ELEMENT(doc_assembly, dependency),
    DEFINE_TAG_WORKER_ELEMENT(doc_assembly_dependency, dependentAssembly),
    DEFINE_TAG_WORKER_ELEMENT(doc_assembly_dependency_dependentAssembly, assemblyIdentity),
    DEFINE_TAG_WORKER_ELEMENT(doc_assembly_dependency_dependentAssembly, bindingRedirect),
    DEFINE_TAG_WORKER_ELEMENT_NOCB(doc_assembly, file),
    DEFINE_TAG_WORKER_ELEMENT_NOCB(doc_assembly_file, comClass),
    DEFINE_TAG_WORKER_ELEMENT_NOCB(doc_assembly_file_comClass, progid),
    DEFINE_TAG_WORKER_ELEMENT_NOCB(doc_assembly, clrClass),
    DEFINE_TAG_WORKER_ELEMENT_NOCB(doc_assembly_clrClass, progid),
    DEFINE_TAG_WORKER_ELEMENT_NOCB(doc_assembly_file, comInterfaceProxyStub),
    DEFINE_TAG_WORKER_ELEMENT_NOCB(doc_assembly_file, typelib),
    DEFINE_TAG_WORKER_ELEMENT_NOCB(doc_assembly_file, windowClass),
    DEFINE_TAG_WORKER_ELEMENT_NOCB(doc_assembly, clrSurrogate),
    DEFINE_TAG_WORKER_ELEMENT_NOCB(doc_assembly, comInterfaceExternalProxyStub),

    // All app config productions go here, just for neatness
    DEFINE_TAG_WORKER_ELEMENT_NONS(doc, configuration),
    DEFINE_TAG_WORKER_ELEMENT_NONS(doc_configuration, windows),
    DEFINE_TAG_WORKER_ELEMENT(doc_configuration_windows, assemblyBinding),
    DEFINE_TAG_WORKER_ELEMENT(doc_configuration_windows_assemblyBinding, assemblyIdentity),
    DEFINE_TAG_WORKER_ELEMENT(doc_configuration_windows_assemblyBinding, dependentAssembly),
    DEFINE_TAG_WORKER_ELEMENT(doc_configuration_windows_assemblyBinding, publisherPolicy),
    DEFINE_TAG_WORKER_ELEMENT(doc_configuration_windows_assemblyBinding_dependentAssembly, assemblyIdentity),
    DEFINE_TAG_WORKER_ELEMENT(doc_configuration_windows_assemblyBinding_dependentAssembly, bindingRedirect),
    DEFINE_TAG_WORKER_ELEMENT(doc_configuration_windows_assemblyBinding_dependentAssembly, publisherPolicy),
};

BOOL
SxspIsNamespaceDeclaration(XML_NODE_INFO *pNodeInfo)
{
    FN_TRACE();
    
    ASSERT(pNodeInfo->dwType == XML_ATTRIBUTE);

    //
    // ISSUE: jonwis 3/8/2002 - Could use FusionpCompareString rather than doing it the 'hard way'
    // 
    if (pNodeInfo->ulLen >= 5)
    { // "xmlns" : prefix for namespace declaration, default ns or non-default ns
        if ((pNodeInfo->pwcText[0] == L'x') &&
            (pNodeInfo->pwcText[1] == L'm') &&
            (pNodeInfo->pwcText[2] == L'l') &&
            (pNodeInfo->pwcText[3] == L'n') &&
            (pNodeInfo->pwcText[4] == L's'))
        {
            if (pNodeInfo->ulLen == 5) // like xmlns="", default ns declaration
                return TRUE;
            else
                if (pNodeInfo->pwcText[5] == L':')
                    return TRUE;
        }
    }

    return FALSE;
}
//In this function, two tasks:
// 1) verify PublicKey and StrongName
// 2) create AssemblyIdentity based on xmlnode array
// 3) for (name, processorArchitecure, version. langid) they would be unique with SXS_ASSEMBLY_MANIFEST_STD_NAMESPACE
// 4) if there are dup triples {nsURL, name, value), only one is count, this is done with SxsCreateAssemblyIdentity
BOOL
SxspCreateAssemblyIdentityFromIdentityElement(
    DWORD Flags,
    PCACTCTXCTB_PARSE_CONTEXT ParseContext,
    ULONG Type,
    PASSEMBLY_IDENTITY *AssemblyIdentityOut,
    DWORD cNumRecs,
    PCSXS_NODE_INFO prgNodeInfo
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    DWORD i;
    //
    // ISSUE: 3/9/2002 - Consider using a smart pointer here for AssemblyIdentity
    //      CSxsPointerWithNamedDestructor<ASSEMBLY_IDENTITY, ::SxsDestroyAssemblyIdentity> AssemblyIdentity;
    //
    // NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Leaking memory here if AssemblyIdentity isn't cleaned up (#3)
    PASSEMBLY_IDENTITY AssemblyIdentity = NULL;
    CStringBuffer buffValue;

    if (AssemblyIdentityOut != NULL)
         *AssemblyIdentityOut = NULL;

    PARAMETER_CHECK((Flags & ~(SXSP_CREATE_ASSEMBLY_IDENTITY_FROM_IDENTITY_TAG_FLAG_VERIFY_PUBLIC_KEY_IF_PRESENT)) == 0);
    PARAMETER_CHECK((Type == ASSEMBLY_IDENTITY_TYPE_DEFINITION) || (Type == ASSEMBLY_IDENTITY_TYPE_REFERENCE));
    PARAMETER_CHECK(AssemblyIdentityOut != NULL);
    PARAMETER_CHECK(prgNodeInfo != NULL);
    PARAMETER_CHECK(prgNodeInfo[0].Type == XML_ELEMENT);

    IFW32FALSE_EXIT(::SxsCreateAssemblyIdentity(0, Type, &AssemblyIdentity, 0, NULL));

    // NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Why not use a 'for' loop? (#4)
    i = 1;
    while (i<cNumRecs)
    {
        ULONG j;
        INTERNAL_ERROR_CHECK(prgNodeInfo[i].Type == XML_ATTRIBUTE);

        buffValue.Clear();

        //
        // ISSUE: jonwis 3/9/2002 - Consider moving this to a for() loop for optimization's sake:
        //          for (j = i + 1; ((j < cNumRec) && (prgNodeInfo[j].Type == XML_PCDATA)); j++)
        //
        j = i + 1;

        while ((j < cNumRecs) && (prgNodeInfo[j].Type == XML_PCDATA))
        {
            IFW32FALSE_EXIT(buffValue.Win32Append(prgNodeInfo[j].pszText, prgNodeInfo[j].cchText));

            j++;
        }

        // if this is a special attribute, we'll handle it ... specially.
        if ((prgNodeInfo[i].NamespaceStringBuf.Cch() == 0) &&
            (::FusionpCompareStrings(
                SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY,
                SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY_CCH,
                prgNodeInfo[i].pszText,
                prgNodeInfo[i].cchText,
                false) == 0))
        {// ignore publicKey if it appears in assembly identity
        }
        else
        {
            //
            // ISSUE: jonwis 3/9/2002 - Consider moving this to initialization rather than assignment
            //
            // NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Compiler is better than you are (#5)
            ASSEMBLY_IDENTITY_ATTRIBUTE Attribute;

            Attribute.Flags = 0;
            Attribute.NamespaceCch  = prgNodeInfo[i].NamespaceStringBuf.Cch();
            Attribute.Namespace     = prgNodeInfo[i].NamespaceStringBuf;
            Attribute.NameCch       = prgNodeInfo[i].cchText;
            Attribute.Name          = prgNodeInfo[i].pszText;
            Attribute.ValueCch      = buffValue.Cch();
            Attribute.Value         = buffValue;

            IFW32FALSE_EXIT(::SxsInsertAssemblyIdentityAttribute(0, AssemblyIdentity, &Attribute));
        }

        i = j;
    }

    *AssemblyIdentityOut = AssemblyIdentity;
    AssemblyIdentity = NULL;

    fSuccess = TRUE;
Exit:
    if (AssemblyIdentity != NULL)
        ::SxsDestroyAssemblyIdentity(AssemblyIdentity);
    return fSuccess;
}

//
// ISSUE: jonwis 3/9/2002 - Consider reordering initializers to read more like what's in the
//      class def for simpler reading.  Also consider using a smart-pointer object to manage
//      the m_Assembly, rather than doing our own management.  Same for the ASSEMBLY_IDENTITY
//      m_CurrentPolicyDependentAssemblyIdentity
//
// NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Odd ordering of initializers (#6)
CNodeFactory::CNodeFactory()
        : m_ActCtxGenCtx(NULL),
          m_Assembly(NULL),
          m_fFirstCreateNodeCall(true),
          m_cUnknownChildDepth(0),
          m_xpsParseState(eParsing_doc),
          m_fAssemblyFound(false),
          m_fIdentityFound(false),
          m_AssemblyContext(NULL),
          m_CurrentPolicyDependentAssemblyIdentity(NULL),
          m_CurrentPolicyStatement(NULL),
          m_IntuitedParseType(eActualParseType_Undetermined),
          m_pApplicationPolicyTable(NULL),          
          m_fNoInheritableFound(false)
{
    m_ParseContext.XMLElementDepth = 0;
    m_ParseContext.ElementPath = NULL;
    m_ParseContext.ElementPathCch = 0;
    m_ParseContext.ElementName = NULL;
    m_ParseContext.ElementPathCch = 0;
    m_ParseContext.ElementHash = 0;
}

CNodeFactory::~CNodeFactory()
{
    CSxsPreserveLastError ple;

    if ((m_CurrentPolicyStatement != NULL) &&
        (m_CurrentPolicyDependentAssemblyIdentity != NULL) &&
        (m_pApplicationPolicyTable != NULL))
    {
#if 1
        if (m_pApplicationPolicyTable->Find(m_buffCurrentApplicationPolicyIdentityKey, m_CurrentPolicyStatement))
            m_CurrentPolicyStatement = NULL;
#else
        CStringBuffer EncodedPolicyIdentity; 
        if (::SxspGenerateTextuallyEncodedPolicyIdentityFromAssemblyIdentity(
            SXSP_GENERATE_TEXTUALLY_ENCODED_POLICY_IDENTITY_FROM_ASSEMBLY_IDENTITY_FLAG_OMIT_ENTIRE_VERSION,
            m_CurrentPolicyDependentAssemblyIdentity,
            EncodedPolicyIdentity,
            NULL))
        {
            if (m_pApplicationPolicyTable->Find(EncodedPolicyIdentity, m_CurrentPolicyStatement))
            {
                m_CurrentPolicyStatement = NULL; // leave the cleanup work to outer destructor
            }
        }
#endif
    }

    FUSION_DELETE_SINGLETON(m_CurrentPolicyStatement);

    if (m_CurrentPolicyDependentAssemblyIdentity != NULL)
        ::SxsDestroyAssemblyIdentity(m_CurrentPolicyDependentAssemblyIdentity);

    if (m_Assembly != NULL)
    {
        m_Assembly->Release();
        m_Assembly = NULL;
    }

    ple.Restore();
}

BOOL
CNodeFactory::Initialize(
    PACTCTXGENCTX ActCtxGenCtx,
    PASSEMBLY Assembly,
    PACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext
    )
{
    FN_PROLOG_WIN32

    PARAMETER_CHECK(Assembly != NULL);

    IFW32FALSE_EXIT(m_XMLNamespaceManager.Initialize());

    m_ActCtxGenCtx = ActCtxGenCtx;

    Assembly->AddRef();
    if (m_Assembly != NULL)
        m_Assembly->Release();
    m_Assembly = Assembly;

    m_AssemblyContext = AssemblyContext;
    m_ParseContext.AssemblyContext = AssemblyContext;
    m_ParseContext.ErrorCallbacks.MissingRequiredAttribute = &CNodeFactory::ParseErrorCallback_MissingRequiredAttribute;
    m_ParseContext.ErrorCallbacks.AttributeNotAllowed = &CNodeFactory::ParseErrorCallback_AttributeNotAllowed;
    m_ParseContext.ErrorCallbacks.InvalidAttributeValue = &CNodeFactory::ParseErrorCallback_InvalidAttributeValue;
    m_ParseContext.SourceFilePathType = AssemblyContext->ManifestPathType;
    m_ParseContext.SourceFile = AssemblyContext->ManifestPath;
    m_ParseContext.SourceFileCch = AssemblyContext->ManifestPathCch;
    m_ParseContext.LineNumber = 0;

    FN_EPILOG
}

VOID
CNodeFactory::ResetParseState()
{
    m_fFirstCreateNodeCall = true;
    m_fAssemblyFound = false;
    m_fIdentityFound = false;
    m_fNoInheritableFound = false;

    FUSION_DELETE_SINGLETON(m_CurrentPolicyStatement);
    m_CurrentPolicyStatement = NULL;

    ::SxsDestroyAssemblyIdentity(m_CurrentPolicyDependentAssemblyIdentity);
    m_CurrentPolicyDependentAssemblyIdentity = NULL;    
}

HRESULT
CNodeFactory::QueryInterface(
    REFIID riid,
    LPVOID *ppvObj
    )
{
    FN_PROLOG_HR
    IUnknown *pIUnknown = NULL;

    if (ppvObj != NULL)
        *ppvObj = NULL;
    else
        ORIGINATE_HR_FAILURE_AND_EXIT(CNodeFactory::QueryInterface, E_POINTER);

    if (riid == __uuidof(this))
        pIUnknown = this;
    else if ((riid == IID_IUnknown) || (riid == IID_IXMLNodeFactory))
        pIUnknown = static_cast<IXMLNodeFactory *>(this);
    else
        ORIGINATE_HR_FAILURE_AND_EXIT(CNodeFactory::QueryInterface, E_NOINTERFACE);

    pIUnknown->AddRef();
    *ppvObj = pIUnknown;

    FN_EPILOG
}

HRESULT
CNodeFactory::NotifyEvent(
    IXMLNodeSource *pSource,
    XML_NODEFACTORY_EVENT iEvt
    )
{
    return NOERROR;
}

HRESULT
CNodeFactory::ConvertXMLNodeInfoToSXSNodeInfo(
	const XML_NODE_INFO *pNodeInfo,
	SXS_NODE_INFO &rSXSNodeInfo
	)
{
    //
    // ISSUE: jonwis 3/9/2002 - Consider filling out a private SXS_NODE_INFO and assigning it to
    //          the output SXSNodeInfo on success.
    //
    // NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Use a private SXS_NODE_INFO (#7)
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);

    INTERNAL_ERROR_CHECK(pNodeInfo != NULL);

    rSXSNodeInfo.Size = pNodeInfo->dwSize;
    rSXSNodeInfo.Type = pNodeInfo->dwType;

    switch (pNodeInfo->dwType)
    {
    case XML_ELEMENT:
        {
            SIZE_T cchNamespacePrefix;
            IFCOMFAILED_EXIT(
                m_XMLNamespaceManager.Map(
                    0,
                    pNodeInfo,
                    &rSXSNodeInfo.NamespaceStringBuf,
                    &cchNamespacePrefix));

            // +1 to skip colon
            rSXSNodeInfo.pszText = pNodeInfo->pwcText + ((cchNamespacePrefix != 0) ? (cchNamespacePrefix + 1) : 0);
            rSXSNodeInfo.cchText = pNodeInfo->ulLen - ((cchNamespacePrefix != 0) ? (cchNamespacePrefix + 1) : 0);

            break;
        }

    case XML_ATTRIBUTE:
        {
            SIZE_T cchNamespacePrefix;

            //
            // ISSUE: jonwis 3/9/2002 - I think this is questionable. Why are we allowing through
            //          attributes named "xmlns:" and "xmlns="?  Shouldn't these have been fixed earlier?
            //
            // ISSUE:2002-3-29:jonwis - I also think that this is a gross method of doing this work.  Calling
            //          wcslen on constants is a "bad thing."  There's got to be something better we
            //          could be doing here.
            // NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Wierd passthroughs of xmlns variants

            // if this is a namespace definition, ignore
            const PCWSTR pwcText = pNodeInfo->pwcText;
            if ((pwcText[0] == L'x') &&
                (pwcText[1] == L'm') &&
                (pwcText[2] == L'l') &&
                (pwcText[3] == L'n') &&
                (pwcText[4] == L's') &&
                ((pwcText[5] == L':') ||
                 (pwcText[5] == L'=')))
            {
                rSXSNodeInfo.pszText = pNodeInfo->pwcText;
                rSXSNodeInfo.cchText = pNodeInfo->ulLen;
            }
            else
            {
                IFCOMFAILED_EXIT(
                    m_XMLNamespaceManager.Map(
                        CXMLNamespaceManager::eMapFlag_DoNotApplyDefaultNamespace,
                        pNodeInfo,
                        &rSXSNodeInfo.NamespaceStringBuf,
                        &cchNamespacePrefix));

                // +1 to skip colon
                rSXSNodeInfo.pszText = pNodeInfo->pwcText + ((cchNamespacePrefix != 0) ? (cchNamespacePrefix + 1) : 0);
                rSXSNodeInfo.cchText = pNodeInfo->ulLen - ((cchNamespacePrefix != 0) ? (cchNamespacePrefix + 1) : 0);
            }
            break;
        }

    default:
        // Otherwise we'll assume there's no namespace processing to do...
        rSXSNodeInfo.NamespaceStringBuf.Clear();
        rSXSNodeInfo.pszText = pNodeInfo->pwcText;
        rSXSNodeInfo.cchText = pNodeInfo->ulLen;
        break;
    }

	FN_EPILOG
}

HRESULT
CNodeFactory::BeginChildren(
    IXMLNodeSource *pSource,
    XML_NODE_INFO *pNodeInfo
    )
{
    FN_PROLOG_HR

    ULONG i = 0;
    SXS_NODE_INFO SXSNodeInfo;

    IFCOMFAILED_EXIT(m_XMLNamespaceManager.OnBeginChildren(pSource, pNodeInfo));

    IFCOMFAILED_EXIT(this->ConvertXMLNodeInfoToSXSNodeInfo(pNodeInfo, SXSNodeInfo));
    for (i=0; i<m_ActCtxGenCtx->m_ContributorCount; i++)
    {
        IFW32FALSE_EXIT(
            m_ActCtxGenCtx->m_Contributors[i].Fire_BeginChildren(
                m_ActCtxGenCtx,
                m_AssemblyContext,
                &m_ParseContext,
                &SXSNodeInfo));
    }

    FN_EPILOG
}

HRESULT
CNodeFactory::EndChildren(
    IXMLNodeSource *pSource,
    BOOL Empty,
    XML_NODE_INFO *pNodeInfo
    )
{
    FN_PROLOG_HR
    ULONG i = 0;
    PWSTR Bang = NULL;
    SXS_NODE_INFO SXSNodeInfo;

    // Short-circuit PIs, XML-decls, whitespace and comments
    if ((pNodeInfo->dwType == XML_PI) ||
        (pNodeInfo->dwType == XML_XMLDECL) ||
        (pNodeInfo->dwType == XML_COMMENT) ||
        (pNodeInfo->dwType == XML_WHITESPACE))
    {
        FN_SUCCESSFUL_EXIT();

    }

    IFCOMFAILED_EXIT(m_XMLNamespaceManager.OnEndChildren(pSource, Empty, pNodeInfo));
    // We hit the end of something; if we're skipping stuff, we're one level back towards
    // paying attention.
    if (m_cUnknownChildDepth != 0)
    {
        m_cUnknownChildDepth--;
    }
    else
    {
        ULONG j;

        for (j=0; j<NUMBER_OF(s_rgWorkers); j++)
        {
            if (s_rgWorkers[j].m_xpsNew == m_xpsParseState)
            {
                m_xpsParseState = s_rgWorkers[j].m_xpsOld;
                break;
            }
        }

        if (j == NUMBER_OF(s_rgWorkers))
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: %s() called when we were not expecting it.  m_xpsParseState = %d\n", __FUNCTION__, m_xpsParseState);

            INTERNAL_ERROR_CHECK(FALSE);
            // Hey, how the heck did we get here?
        }

        // One time end-of-manifest checks...
        if (m_xpsParseState == eParsing_doc)
        {
            switch (m_ParseType)
            {
            default:
                INTERNAL_ERROR_CHECK(false);
                break;

            case XML_FILE_TYPE_COMPONENT_CONFIGURATION:
            case XML_FILE_TYPE_APPLICATION_CONFIGURATION:
                break;

            case XML_FILE_TYPE_MANIFEST:
                // If this is not the root assembly, this is not a noInherit actctx and the noInheritable
                // element was not found, issue an error.
                if (((m_AssemblyContext->Flags & ACTCTXCTB_ASSEMBLY_CONTEXT_IS_ROOT_ASSEMBLY) == 0) &&
                    m_ActCtxGenCtx->m_NoInherit &&
                    !m_fNoInheritableFound)
                {
                    this->LogParseError(MSG_SXS_NOINHERIT_REQUIRES_NOINHERITABLE);
                    ORIGINATE_WIN32_FAILURE_AND_EXIT(NoInheritRequiresNoInheritable, ERROR_SXS_MANIFEST_PARSE_ERROR);
                }
                break;
            }
        }
    }

    if (pNodeInfo->dwType != XML_XMLDECL)
    {
        IFCOMFAILED_EXIT(this->ConvertXMLNodeInfoToSXSNodeInfo(pNodeInfo, SXSNodeInfo));

        for (i=0; i<m_ActCtxGenCtx->m_ContributorCount; i++)
        {
            IFW32FALSE_EXIT(
                m_ActCtxGenCtx->m_Contributors[i].Fire_EndChildren(
                    m_ActCtxGenCtx,
                    m_AssemblyContext,
                    &m_ParseContext,
                    Empty,
                    &SXSNodeInfo));
        }

        INTERNAL_ERROR_CHECK(m_ParseContext.XMLElementDepth != 0);

        //
        // ISSUE: jonwis 3/9/2002 - Comment this better! It's hard to figure out what this is doing.
        //          It's apparently turning "foo!bar!bas" into "foo!bar", and "foo" into "". This
        //          could probably be done much better with some cleaning.
        //
        // NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Cleanup this string-fixer code (#9)
        m_ParseContext.XMLElementDepth--;
        Bang = wcsrchr(m_buffElementPath, L'!');

        INTERNAL_ERROR_CHECK(((Bang == NULL) == (m_ParseContext.XMLElementDepth == 0)));

        if (Bang != NULL)
        {
            m_buffElementPath.Left(Bang - m_buffElementPath);
            m_ParseContext.ElementPathCch = m_buffElementPath.Cch();
            m_ParseContext.ElementPath = m_buffElementPath;
            m_ParseContext.ElementName = wcsrchr(m_buffElementPath, L'!');
            if (m_ParseContext.ElementName == NULL)
            {
                m_ParseContext.ElementName = m_buffElementPath;
                m_ParseContext.ElementNameCch = m_buffElementPath.Cch();
            }
            else
            {
                m_ParseContext.ElementName++;
                m_ParseContext.ElementNameCch = m_buffElementPath.Cch() - (m_ParseContext.ElementName - m_buffElementPath);
            }

            IFW32FALSE_ORIGINATE_AND_EXIT(
                ::SxspHashString(
                    m_buffElementPath,
                    m_ParseContext.ElementPathCch,
                    &m_ParseContext.ElementHash,
                    false));
        }
        else
        {
            m_buffElementPath.Clear();
            m_ParseContext.ElementPath = NULL;
            m_ParseContext.ElementPathCch = 0;
            m_ParseContext.ElementName = NULL;
            m_ParseContext.ElementNameCch = 0;
            m_ParseContext.ElementHash = 0;
            m_ParseContext.XMLElementDepth = 0;
        }
    }

    FN_EPILOG
}

HRESULT
CNodeFactory::Error(
    IXMLNodeSource *pSource,
    HRESULT hrErrorCode,
    USHORT cNumRecs,
    XML_NODE_INFO **apNodeInfo
    )
{
    CSxsPreserveLastError ple;
    ::FusionpConvertCOMFailure(hrErrorCode);
    ::FusionpSetLastErrorFromHRESULT(hrErrorCode);
    this->LogParseError(MSG_SXS_WIN32_ERROR_MSG_WHEN_PARSING_MANIFEST, CEventLogLastError());
    ple.Restore();
    return NOERROR;
}

HRESULT
CNodeFactory::FirstCreateNodeCall(
    IXMLNodeSource *pSource,
    PVOID pNodeParent,
    USHORT NodeCount,
    const SXS_NODE_INFO *prgNodeInfo
    )
{
    FN_PROLOG_HR
    ULONG i = 0;
    bool fGotGoodManifestVersion = false;
    bool fGotAnyManifestVersion = false;

    // It's our first IXMLNodeFactory::CreateNode() call.  This had better
    // be an <ASSEMBLY MANIFESTVERSION="1.0" ...> deal.

    for (i=0; i<NodeCount; i++)
    {
        if (prgNodeInfo[i].Type == XML_ELEMENT)
        {
            INTERNAL_ERROR_CHECK(i == 0);

            switch (m_ParseType)
            {
            default:
                INTERNAL_ERROR_CHECK(false);
                break;

                //
                // ISSUE: jonwis 3/9/2002 - My goodness, this is gross.  Use FusionEqualStrings
                //          instead.  Doing memcmps between strings is totally bogus.
                //
                // NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Memcmping here is bogus
            case XML_FILE_TYPE_MANIFEST:
            case XML_FILE_TYPE_COMPONENT_CONFIGURATION:
                if ((prgNodeInfo[i].cchText != (NUMBER_OF(SXS_ASSEMBLY_MANIFEST_STD_ELEMENT_NAME_ASSEMBLY) - 1)) ||
                    (prgNodeInfo[i].NamespaceStringBuf.Cch() != (NUMBER_OF(SXS_ASSEMBLY_MANIFEST_STD_NAMESPACE) - 1)) ||
                    (memcmp(prgNodeInfo[i].pszText, SXS_ASSEMBLY_MANIFEST_STD_ELEMENT_NAME_ASSEMBLY, prgNodeInfo[i].cchText * sizeof(WCHAR)) != 0) ||
                    (memcmp(prgNodeInfo[i].NamespaceStringBuf, SXS_ASSEMBLY_MANIFEST_STD_NAMESPACE, prgNodeInfo[i].NamespaceStringBuf.Cch() * sizeof(WCHAR)) != 0))
                {
                    IFCOMFAILED_EXIT(this->LogParseError(MSG_SXS_MANIFEST_INCORRECT_ROOT_ELEMENT));
                }
                break;
                
                // NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Memcmping here is bogus
            case XML_FILE_TYPE_APPLICATION_CONFIGURATION:
                if ((prgNodeInfo[i].cchText != SXS_APPLICATION_CONFIGURATION_MANIFEST_STD_ELEMENT_NAME_CONFIGURATION_CCH) ||
                    (prgNodeInfo[i].NamespaceStringBuf.Cch() != 0) ||
                    (memcmp(prgNodeInfo[i].pszText, SXS_APPLICATION_CONFIGURATION_MANIFEST_STD_ELEMENT_NAME_CONFIGURATION,
                        SXS_APPLICATION_CONFIGURATION_MANIFEST_STD_ELEMENT_NAME_CONFIGURATION_CCH * sizeof(WCHAR)) != 0))
                {
                    IFCOMFAILED_EXIT(this->LogParseError(MSG_SXS_MANIFEST_INCORRECT_ROOT_ELEMENT));
                }
                break;
            }
        }
        else if (prgNodeInfo[i].Type == XML_ATTRIBUTE)
        {
            // NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Memcmping here is bogus
            if ((prgNodeInfo[i].cchText == (NUMBER_OF(SXS_ASSEMBLY_MANIFEST_STD_ATTRIBUTE_NAME_MANIFEST_VERSION) - 1)) &&
                (prgNodeInfo[i].NamespaceStringBuf.Cch() == 0) &&
                (memcmp(prgNodeInfo[i].pszText, SXS_ASSEMBLY_MANIFEST_STD_ATTRIBUTE_NAME_MANIFEST_VERSION, prgNodeInfo[i].cchText * sizeof(WCHAR)) == 0))
            {
                fGotAnyManifestVersion = true;

                ULONG j = i + 1;

                //
                // ISSUE: jonwis 3/9/2002 - Any reason these can't be a single if statement?
                // - Oh, an NO MEMCMPING STRINGS
                //
                // NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Memcmping here is bogus
                // NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Unrolled if (a&&b) is strange, but not wrong
                if (j < NodeCount)
                {
                    if (prgNodeInfo[j].Type == XML_PCDATA)
                    {
                        if (prgNodeInfo[j].cchText == 3)
                        {
                            if (memcmp(prgNodeInfo[j].pszText, L"1.0", prgNodeInfo[j].cchText * sizeof(WCHAR)) == 0)
                            {
                                fGotGoodManifestVersion = true;
                            }
                        }
                    }
                }
            }
        }
    }

    if ((m_ParseType == XML_FILE_TYPE_MANIFEST) ||
        (m_ParseType == XML_FILE_TYPE_COMPONENT_CONFIGURATION))
    {
        if (fGotAnyManifestVersion)
        {
            if (!fGotGoodManifestVersion)
                IFCOMFAILED_EXIT(this->LogParseError(MSG_SXS_MANIFEST_VERSION_ERROR));
        }
        else
            IFCOMFAILED_EXIT(this->LogParseError(MSG_SXS_MANIFEST_VERSION_MISSING));
    }

    m_Assembly->m_ManifestVersionMajor = 1;
    m_Assembly->m_ManifestVersionMinor = 0;

    FN_EPILOG
}

HRESULT
CNodeFactory::CreateNode(
    IXMLNodeSource *pSource,
    PVOID pNodeParent,
    USHORT NodeCount,
    XML_NODE_INFO **apNodeInfo
    )
{
    HRESULT hr = S_OK;
    FN_TRACE_HR(hr);

    ULONG i;
    //
    // ISSUE: jonwis 3/9/2002 - Consider making both of these smart-pointers to simplify cleanup
    //
    // NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Smart pointers would make cleanup cake
    PSXS_XML_NODE pXmlNode = NULL;
    PSXS_NODE_INFO pSXSNodeInfo = NULL;
    SIZE_T cchTemp;

    m_ParseContext.LineNumber = pSource->GetLineNumber();

    INTERNAL_ERROR_CHECK(NodeCount != 0);

#if DBG
    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_NODEFACTORY,
        "SXS.DLL: " __FUNCTION__ "() entered\n"
        "   m_ParseContext.XMLElementDepth = %lu\n",
        m_ParseContext.XMLElementDepth);

    for (i=0; i<NodeCount; i++)
        ::SxspDbgPrintXmlNodeInfo(FUSION_DBG_LEVEL_NODEFACTORY, apNodeInfo[i]);
#endif

    // Short-circuit PIs, XML-decls, whitespace and comments
    if ((apNodeInfo[0]->dwType == XML_PI) ||
        (apNodeInfo[0]->dwType == XML_XMLDECL) ||
        (apNodeInfo[0]->dwType == XML_COMMENT) ||
        (apNodeInfo[0]->dwType == XML_WHITESPACE))
    {
        FN_SUCCESSFUL_EXIT();
    }
    
    IFCOMFAILED_EXIT(m_XMLNamespaceManager.OnCreateNode(pSource, pNodeParent, NodeCount, apNodeInfo));

    IFALLOCFAILED_EXIT(pSXSNodeInfo = new SXS_NODE_INFO[NodeCount]);
    for (i=0; i<NodeCount; i++)
        IFCOMFAILED_EXIT(this->ConvertXMLNodeInfoToSXSNodeInfo(apNodeInfo[i], pSXSNodeInfo[i]));

    if (m_fFirstCreateNodeCall)
    {
        if ((apNodeInfo[0]->dwType == XML_COMMENT) ||
             (apNodeInfo[0]->dwType == XML_XMLDECL) ||
             (apNodeInfo[0]->dwType == XML_WHITESPACE))
        {
            hr = S_OK;
            goto Cont;
        }

        m_fFirstCreateNodeCall = FALSE;
        IFCOMFAILED_EXIT(this->FirstCreateNodeCall(pSource, pNodeParent, NodeCount, pSXSNodeInfo));
    }

Cont:
    if (m_cUnknownChildDepth == 0)
    {
        for (i=0; i<NUMBER_OF(s_rgWorkers); i++)
        {
            bool fTemp = false;

            if ((s_rgWorkers[i].m_xpsOld == eNotParsing) ||
                (m_xpsParseState == s_rgWorkers[i].m_xpsOld))
                fTemp = true;

            const bool fParseStateMatches = fTemp;

            fTemp = false;

            if (fParseStateMatches)
            {
                if (s_rgWorkers[i].m_dwType == apNodeInfo[0]->dwType)
                    fTemp = true;
            }

            const bool fTypeMatches = fTemp;

            fTemp = false;

            if (fTypeMatches)
            {
                if (s_rgWorkers[i].m_cchName == 0)
                    fTemp = true;
                else
                {
                    if (s_rgWorkers[i].m_cchNamespace == pSXSNodeInfo[0].NamespaceStringBuf.Cch())
                    {
                        if (s_rgWorkers[i].m_cchName == pSXSNodeInfo[0].cchText)
                        {
                            if (::FusionpCompareStrings(
                                    s_rgWorkers[i].m_pszNamespace,
                                    s_rgWorkers[i].m_cchNamespace,
                                    pSXSNodeInfo[0].NamespaceStringBuf,
                                    pSXSNodeInfo[0].NamespaceStringBuf.Cch(),
                                    false) == 0)
                            {
                                if (::FusionpCompareStrings(
                                        s_rgWorkers[i].m_pszName,
                                        s_rgWorkers[i].m_cchName,
                                        pSXSNodeInfo[0].pszText,
                                        pSXSNodeInfo[0].cchText,
                                        false) == 0)
                                {
                                    fTemp = true;
                                }
                            }
                        }
                    }
                }
            }

            if (fTemp)
            {
                m_xpsParseState = s_rgWorkers[i].m_xpsNew;

                IFW32FALSE_EXIT(
                    this->ValidateElementAttributes(
                        pSXSNodeInfo,
                        NodeCount,
                        s_rgWorkers[i].m_prgLegalAttributes,
                        s_rgWorkers[i].m_cLegalAttributes));

                if (s_rgWorkers[i].m_pfn != NULL)
                    IFW32FALSE_EXIT((this->*s_rgWorkers[i].m_pfn)(NodeCount, pSXSNodeInfo));
                break;
            }
        }

        if (i == NUMBER_OF(s_rgWorkers))
        {
            bool fEquals;

            // If we hit an unrecognized element and its namespace is the one we own, error!
            IFW32FALSE_EXIT(
                pSXSNodeInfo[0].NamespaceStringBuf.Win32Equals(
                    SXS_ASSEMBLY_MANIFEST_STD_NAMESPACE,
                    SXS_ASSEMBLY_MANIFEST_STD_NAMESPACE_CCH,
                    fEquals,
                    false));

            if (fEquals)
            {
                this->LogParseError(
                    MSG_SXS_MANIFEST_ELEMENT_USED_IN_INVALID_CONTEXT,
                    CUnicodeString(apNodeInfo[0]->pwcText, apNodeInfo[0]->ulLen),
                    CUnicodeString(m_ParseContext.ElementName, m_ParseContext.ElementNameCch));

                ORIGINATE_WIN32_FAILURE_AND_EXIT(ElementInInvalidContext, ERROR_SXS_MANIFEST_PARSE_ERROR);
            }

            // For an unknown child element, the built-in XML parsing should start to ignore the subtree at this point.
            if (apNodeInfo[0]->dwType == XML_ELEMENT)
                m_cUnknownChildDepth = 1;
        }
    }
    else
    {
        if ((NodeCount != 0) &&
            (apNodeInfo[0]->dwType == XML_ELEMENT))
        {
            // We're handling an unknown series of elements; increment the depth.
            m_cUnknownChildDepth++;
        }
    }

    // Fire the right callbacks for XML_ELEMENT, XML_PCDATA and XML_CDATA nodes:
    switch (apNodeInfo[0]->dwType)
    {
    case XML_ELEMENT:
#if defined(MSG_SXS_MANIFEST_PARSE_NO_INHERIT_CHILDREN_NOT_ALLOWED)
        if (m_cUnknownChildDepth != 0 && m_xpsParseState == eParsing_doc_assembly_noInherit)
        {
            ORIGINATE_HR_FAILURE_AND_EXIT(CNodeFactory::CreateNode, this->LogParseError(MSG_SXS_MANIFEST_PARSE_NO_INHERIT_CHILDREN_NOT_ALLOWED));
        }
#endif

        if (m_buffElementPath.Cch() != 0)
            IFW32FALSE_EXIT(m_buffElementPath.Win32Append(L"!", 1));

        cchTemp = m_buffElementPath.Cch();

        //
        // ISSUE: jonwis 3/9/2002 - Use mutli-append here, maybe?
        //
        // NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - A multi-append here will frobble the heap less
        if (pSXSNodeInfo[0].NamespaceStringBuf.Cch() != 0)
        {
            IFW32FALSE_EXIT(m_buffElementPath.Win32Append(pSXSNodeInfo[0].NamespaceStringBuf));
            IFW32FALSE_EXIT(m_buffElementPath.Win32Append(L"^", 1));
        }

        IFW32FALSE_EXIT(m_buffElementPath.Win32Append(pSXSNodeInfo[0].pszText, pSXSNodeInfo[0].cchText));

        m_ParseContext.ElementPathCch = m_buffElementPath.Cch();
        m_ParseContext.ElementPath = m_buffElementPath;
        m_ParseContext.ElementName = static_cast<PCWSTR>(m_buffElementPath) + cchTemp;
        m_ParseContext.ElementNameCch = m_buffElementPath.Cch() - cchTemp;

        IFW32FALSE_EXIT(::SxspHashString(m_buffElementPath, m_buffElementPath.Cch(), &m_ParseContext.ElementHash, true));

        m_ParseContext.XMLElementDepth++;

        //
        // ISSUE: jonwis 3/9/2002 - Maybe there needs to be a more general way of dispatching
        //          this sort of stuff off to the contributors, rather than having embedded
        //          loops everywhere.  Easier to read, easier to maintain (esp. if we end up
        //          doing some sort of filtering later.        
        //
        // NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Lots of places seem to do something similar, consider collapsing
        for (i=0; i<m_ActCtxGenCtx->m_ContributorCount; i++)
        {
            IFW32FALSE_EXIT(
                m_ActCtxGenCtx->m_Contributors[i].Fire_ElementParsed(
                        m_ActCtxGenCtx,
                        m_AssemblyContext,
                        &m_ParseContext,
                        NodeCount,
                        pSXSNodeInfo));
        }

        break;

    case XML_PCDATA:
        for (i=0; i<m_ActCtxGenCtx->m_ContributorCount; i++)
        {
            IFW32FALSE_EXIT(
                m_ActCtxGenCtx->m_Contributors[i].Fire_PCDATAParsed(
                        m_ActCtxGenCtx,
                        m_AssemblyContext,
                        &m_ParseContext,
                        apNodeInfo[0]->pwcText,
                        apNodeInfo[0]->ulLen));
        }

        break;

    case XML_CDATA:
        for (i=0; i<m_ActCtxGenCtx->m_ContributorCount; i++)
        {
            IFW32FALSE_EXIT(
                m_ActCtxGenCtx->m_Contributors[i].Fire_CDATAParsed(
                        m_ActCtxGenCtx,
                        m_AssemblyContext,
                        &m_ParseContext,
                        apNodeInfo[0]->pwcText,
                        apNodeInfo[0]->ulLen));
        }

        break;


    }

    hr = NOERROR;
Exit:
    if (pSXSNodeInfo != NULL)
        FUSION_DELETE_ARRAY(pSXSNodeInfo);

    if (pXmlNode != NULL)
        FUSION_DELETE_SINGLETON(pXmlNode);

    return hr;
}

BOOL
CNodeFactory::SetParseType(
    ULONG ParseType,
    ULONG PathType,
    const CBaseStringBuffer &Path,
    const FILETIME &rftLastWriteTime
    )
{
    FN_PROLOG_WIN32

    PARAMETER_CHECK(
        (ParseType == XML_FILE_TYPE_MANIFEST) ||
        (ParseType == XML_FILE_TYPE_APPLICATION_CONFIGURATION) ||
        (ParseType == XML_FILE_TYPE_COMPONENT_CONFIGURATION));

    PARAMETER_CHECK(PathType == ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE);

    IFW32FALSE_EXIT(m_buffCurrentFileName.Win32Assign(Path));

    m_ParseContext.SourceFilePathType = PathType;
    m_ParseContext.SourceFile = m_buffCurrentFileName;
    m_ParseContext.SourceFileCch = m_buffCurrentFileName.Cch();
    m_ParseContext.SourceFileLastWriteTime = rftLastWriteTime;

    m_ParseType = ParseType;

    FN_EPILOG
}

BOOL
CNodeFactory::XMLParser_Element_doc_assembly(
    USHORT cNumRecs,
    PCSXS_NODE_INFO prgNodeInfo
    )
{
    FN_PROLOG_WIN32
    ULONG i;

    ASSERT(cNumRecs != 0);
    ASSERT(prgNodeInfo != NULL);

    if (m_fAssemblyFound)
    {
        CUnicodeString s;
        PCWSTR ManifestPath;
        IFW32FALSE_EXIT(m_Assembly->GetManifestPath(&ManifestPath, NULL));
        s = ManifestPath;
        ORIGINATE_HR_FAILURE_AND_EXIT(CNodeFactory::XMLParser_Element_doc_assembly, this->LogParseError(MSG_SXS_MANIFEST_MULTIPLE_TOP_ASSEMBLY, &s));
    }

    m_fAssemblyFound = true;
    m_fMetadataSatelliteAlreadyFound = false;

    // Now let's tell all the contributors that we're about to begin a parsing session.
    for (i=0; i<m_ActCtxGenCtx->m_ContributorCount; i++)
    {
        IFW32FALSE_EXIT(m_ActCtxGenCtx->m_Contributors[i].Fire_ParseBeginning(
                    m_ActCtxGenCtx,
                    m_AssemblyContext,
                    0, // FileFlags
                    m_ParseType,
                    m_ParseContext.SourceFilePathType,
                    m_ParseContext.SourceFile,
                    m_ParseContext.SourceFileCch,
                    m_ParseContext.SourceFileLastWriteTime,
                    m_Assembly->m_ManifestVersionMajor,
                    m_Assembly->m_ManifestVersionMinor,
                    m_Assembly->m_MetadataSatelliteRosterIndex));
    }

    FN_EPILOG
}



BOOL
CNodeFactory::XMLParser_Element_doc_assembly_assemblyIdentity(
    USHORT cNumRecs,
    PCSXS_NODE_INFO prgNodeInfo
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    // NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Better cleanup in general here with smart pointers
    PASSEMBLY_IDENTITY AssemblyIdentity = NULL;
    const BOOL fGeneratingActCtx = (m_ActCtxGenCtx->m_ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT);
    ULONG i;
    DWORD dwValidateFlags = 0;

    if (m_fIdentityFound)
    {
        this->LogParseError(MSG_SXS_MULTIPLE_IDENTITY, CEventLogString(prgNodeInfo[0].pszText, prgNodeInfo[0].cchText));

        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: Manifest %ls has multiple identities\n", static_cast<PCWSTR>(m_buffCurrentFileName));
        ORIGINATE_WIN32_FAILURE_AND_EXIT(
            MultipleIdentities,
            ERROR_SXS_MANIFEST_PARSE_ERROR);
    }

    m_fIdentityFound = true;

    IFW32FALSE_EXIT(
        ::SxspCreateAssemblyIdentityFromIdentityElement(
            0,                                  // DWORD Flags,
            &m_ParseContext,
            ASSEMBLY_IDENTITY_TYPE_DEFINITION,  // ULONG Type,
            &AssemblyIdentity,                  // PASSEMBLY_IDENTITY *AssemblyIdentityOut,
            cNumRecs,
            prgNodeInfo));

    // If the identity that was created is a policy statement, then we
    // set the internal parse type to our special 'intuited' parse type
    // for later checks of missing attributes and whatnot.  This does
    // duplicate work in ValidateAssembly that does the same thing, but
    // we need to preemptively set this parse type before we go validating.
    
    // if (m_IntuitedParseType == eActualParseType_Undetermined)
    {
        BOOL fIsPolicy = FALSE;
        IFW32FALSE_EXIT(::SxspDetermineAssemblyType(AssemblyIdentity, fIsPolicy));

        if (fIsPolicy)
            m_IntuitedParseType = eActualParseType_PolicyManifest;
        else
            m_IntuitedParseType = eActualParseType_Undetermined;
    }

    if ((m_IntuitedParseType == eActualParseType_Manifest) ||
        (m_IntuitedParseType == eActualParseType_PolicyManifest) ||
        (m_ParseType == XML_FILE_TYPE_MANIFEST) ||
        (m_ParseType == XML_FILE_TYPE_COMPONENT_CONFIGURATION))
    {
        dwValidateFlags = eValidateIdentity_VersionRequired;
    }

    IFW32FALSE_EXIT(
        this->ValidateIdentity(
            dwValidateFlags,
            ASSEMBLY_IDENTITY_TYPE_DEFINITION,
            AssemblyIdentity));

    if (fGeneratingActCtx)
    {
        if (m_Assembly->IsRoot())
        {
            // If we're generating the actctx and this is the root assembly, it's possible
            // that we got to it by a filesystem path (e.g. private assembly) rather than 
            // an actual reference, so we need to fix up the assembly's identity information
            // appropriately.
            IFW32FALSE_EXIT(m_Assembly->m_ProbedAssemblyInformation.SetProbedIdentity(AssemblyIdentity));            
        }
        else
        {
            // If we're generating the actctx and this isn't the root assembly, we need to verify
            // that it's the right one.
            BOOL fEqual;
            IFW32FALSE_EXIT(
                ::SxsAreAssemblyIdentitiesEqual(
                    SXS_ARE_ASSEMBLY_IDENTITIES_EQUAL_FLAG_ALLOW_REF_TO_MATCH_DEF,
                    m_Assembly->GetAssemblyIdentity(),
                    AssemblyIdentity,
                    &fEqual));

            if (!fEqual)
            {
                ORIGINATE_HR_FAILURE_AND_EXIT(
                    CNodeFactory::XMLParser_Element_doc_assembly_assemblyIdentity,
                    this->LogParseError(MSG_SXS_COMPONENT_MANIFEST_PROBED_IDENTITY_MISMATCH));
                // LogParseError sets the last error appropriate to the message logged
            }
        }
    }

    if (m_IntuitedParseType == eActualParseType_PolicyManifest)
    {
        //
        // ISSUE: jonwis 3/11/2002 - Stomps on m_CurrentPolicyStatement if it was non-null.  Consider
        //          using INTERNAL_ERROR_CHECK to make sure it's NULL first.
        //
        // NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Track possible leaks here better
        IFALLOCFAILED_EXIT(m_CurrentPolicyStatement = new CPolicyStatement);
        IFW32FALSE_EXIT(m_CurrentPolicyStatement->Initialize());
    }

    // Tell everyone that we're sure who we are...
    for (i=0; i<m_ActCtxGenCtx->m_ContributorCount; i++)
    {
        IFW32FALSE_EXIT(
            m_ActCtxGenCtx->m_Contributors[i].Fire_IdentityDetermined(
                    m_ActCtxGenCtx,
                    m_AssemblyContext,
                    &m_ParseContext,
                    AssemblyIdentity));
    }

    // fix up assembly and assembly context so we know where to copy to
    // also save the manifest
    IFW32FALSE_EXIT(m_Assembly->m_ProbedAssemblyInformation.SetAssemblyIdentity(AssemblyIdentity));
    if (m_AssemblyContext->AssemblyIdentity != NULL)
        ::SxsDestroyAssemblyIdentity(const_cast<PASSEMBLY_IDENTITY>(m_AssemblyContext->AssemblyIdentity));

    m_AssemblyContext->AssemblyIdentity = AssemblyIdentity;
    AssemblyIdentity = NULL;

    fSuccess = TRUE;

Exit:
    if (AssemblyIdentity != NULL)
        ::SxsDestroyAssemblyIdentity(AssemblyIdentity);

    return fSuccess;
}

BOOL
CNodeFactory::XMLParser_Element_doc_assembly_noInherit(
    USHORT cNumRecs,
    PCSXS_NODE_INFO prgNodeInfo
    )
{
    FN_PROLOG_WIN32

    INTERNAL_ERROR_CHECK(
        (m_ParseType == XML_FILE_TYPE_MANIFEST) ||
        (m_ParseType == XML_FILE_TYPE_APPLICATION_CONFIGURATION) ||
        (m_ParseType == XML_FILE_TYPE_COMPONENT_CONFIGURATION));

    switch (m_ParseType)
    {
    case XML_FILE_TYPE_MANIFEST:
        if (cNumRecs != 1)
        {
            ORIGINATE_HR_FAILURE_AND_EXIT(
                CNodeFactory::XMLParser_Element_doc_assembly_noInherit,
                this->LogParseError(MSG_SXS_MANIFEST_PARSE_NO_INHERIT_ATTRIBUTES_NOT_ALLOWED));
        }
        
        if (m_ActCtxGenCtx->m_NoInherit)
        {
            ORIGINATE_HR_FAILURE_AND_EXIT(
                CNodeFactory::XMLParser_Element_doc_assembly_noInherit,
                this->LogParseError(MSG_SXS_MANIFEST_PARSE_MULTIPLE_NO_INHERIT));
        }

        if (m_fIdentityFound)
        {
            ORIGINATE_HR_FAILURE_AND_EXIT(
                CNodeFactory::XMLParser_Element_doc_assembly_noInherit,
                this->LogParseError(
                    MSG_SXS_MANIFEST_ELEMENT_MUST_OCCUR_BEFORE,
                    CEventLogString(L"noInherit"),
                    CEventLogString(L"assemblyIdentity")));
        }

        m_ActCtxGenCtx->m_NoInherit = true;
        break;

    case XML_FILE_TYPE_APPLICATION_CONFIGURATION:
        ORIGINATE_HR_FAILURE_AND_EXIT(
            CNodeFactory::XMLParser_Element_doc_assembly_noInherit, 
            this->LogParseError(MSG_SXS_POLICY_PARSE_NO_INHERIT_NOT_ALLOWED));
        break;

    default:
        INTERNAL_ERROR_CHECK(FALSE);
        break;
    }

    FN_EPILOG
}

BOOL
CNodeFactory::XMLParser_Element_doc_assembly_noInheritable(
    USHORT cNumRecs,
    PCSXS_NODE_INFO prgNodeInfo
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    INTERNAL_ERROR_CHECK(
        (m_ParseType == XML_FILE_TYPE_MANIFEST) ||
        (m_ParseType == XML_FILE_TYPE_APPLICATION_CONFIGURATION) ||
        (m_ParseType == XML_FILE_TYPE_COMPONENT_CONFIGURATION));

    switch (m_ParseType)
    {
    case XML_FILE_TYPE_MANIFEST:
        if (cNumRecs != 1)
        {
            this->LogParseError(MSG_SXS_MANIFEST_PARSE_NO_INHERIT_ATTRIBUTES_NOT_ALLOWED);
            goto Exit;
        }

        if (m_fNoInheritableFound)
        {
            this->LogParseError(MSG_SXS_MANIFEST_PARSE_MULTIPLE_NOINHERITABLE);
            goto Exit;
        }
        if (m_fIdentityFound)
        {
            this->LogParseError(
                MSG_SXS_MANIFEST_ELEMENT_MUST_OCCUR_BEFORE,
                CEventLogString(L"noInheritable"),
                CEventLogString(L"assemblyIdentity"));
            goto Exit;
        }


        m_fNoInheritableFound = true;

        break;

    case XML_FILE_TYPE_APPLICATION_CONFIGURATION:
    case XML_FILE_TYPE_COMPONENT_CONFIGURATION:
        this->LogParseError(MSG_SXS_POLICY_PARSE_NO_INHERIT_NOT_ALLOWED);
        goto Exit;

    default:
        ::FusionpSetLastWin32Error(ERROR_INTERNAL_ERROR);
        goto Exit;
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
CNodeFactory::XMLParser_Element_doc_assembly_dependency(
    USHORT cNumRecs,
    PCSXS_NODE_INFO prgNodeInfo
    )
{
    FN_PROLOG_WIN32
    bool fFound;
    SIZE_T cb;

    m_fIsDependencyOptional = false;
    m_fDependencyChildHit = false;
    m_fIsMetadataSatellite = false;

    IFW32FALSE_EXIT(
        ::SxspGetAttributeValue(
            0,
            &s_AttributeName_optional,
            prgNodeInfo,
            cNumRecs,
            &m_ParseContext,
            fFound,
            sizeof(m_fIsDependencyOptional),
            &m_fIsDependencyOptional,
            cb,
            &::SxspValidateBoolAttribute,
            0));

    if (!fFound)
        m_fIsDependencyOptional = false;

    FN_EPILOG
}

BOOL
CNodeFactory::XMLParser_Element_doc_assembly_dependency_dependentAssembly(
    USHORT cNumRecs,
    PCSXS_NODE_INFO prgNodeInfo
    )
{
    FN_PROLOG_WIN32
    bool fFound;
    SIZE_T cb;

    if (m_fDependencyChildHit == false)
    {
        m_fDependencyChildHit = true;
    }
    else
    {
        ORIGINATE_HR_FAILURE_AND_EXIT(
            CNodeFactory::XMLParser_Element_doc_assembly_dependency_dependentAssembly, 
            this->LogParseError(MSG_SXS_MANIFEST_MULTIPLE_DEPENDENTASSEMBLY_IN_DEPENDENCY));
    }

    m_fAssemblyIdentityChildOfDependenctAssemblyHit = false;

    IFW32FALSE_EXIT(
        ::SxspGetAttributeValue(
            0,
            &s_AttributeName_metadataSatellite,
            prgNodeInfo,
            cNumRecs,
            &m_ParseContext,
            fFound,
            sizeof(m_fIsMetadataSatellite),
            &m_fIsMetadataSatellite,
            cb,
            &::SxspValidateBoolAttribute,
            0));

    if (!fFound)
        m_fIsMetadataSatellite = false;

    FN_EPILOG
}

BOOL
CNodeFactory::XMLParser_Element_doc_assembly_dependency_dependentAssembly_bindingRedirect(
    USHORT cNumRecs,
    PCSXS_NODE_INFO prgNodeInfo
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    bool fFound;
    bool fValid;
    SIZE_T cb;
    CSmallStringBuffer buffOldVersion;
    CSmallStringBuffer buffNewVersion;

    INTERNAL_ERROR_CHECK(m_CurrentPolicyStatement != NULL);

    if (m_IntuitedParseType != eActualParseType_PolicyManifest)
    {
        this->LogParseError(MSG_SXS_BINDING_REDIRECTS_ONLY_IN_COMPONENT_CONFIGURATION);
        goto Exit;
    }

    IFW32FALSE_EXIT(
        ::SxspGetAttributeValue(
            SXSP_GET_ATTRIBUTE_VALUE_FLAG_REQUIRED_ATTRIBUTE,
            &s_AttributeName_oldVersion,
            prgNodeInfo,
            cNumRecs,
            &m_ParseContext,
            fFound,
            sizeof(buffOldVersion),
            &buffOldVersion,
            cb,
            NULL,
            0));
    INTERNAL_ERROR_CHECK(fFound);

    IFW32FALSE_EXIT(
        ::SxspGetAttributeValue(
            SXSP_GET_ATTRIBUTE_VALUE_FLAG_REQUIRED_ATTRIBUTE,
            &s_AttributeName_newVersion,
            prgNodeInfo,
            cNumRecs,
            &m_ParseContext,
            fFound,
            sizeof(buffNewVersion),
            &buffNewVersion,
            cb,
            NULL,
            0));
    INTERNAL_ERROR_CHECK(fFound);

    IFW32FALSE_EXIT(m_CurrentPolicyStatement->AddRedirect(buffOldVersion, buffNewVersion, fValid));

    if (!fValid)
    {
        this->LogParseError(MSG_SXS_BINDING_REDIRECT_MISSING_REQUIRED_ATTRIBUTES);
        goto Exit;
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
CNodeFactory::XMLParser_Element_doc_assembly_dependency_dependentAssembly_assemblyIdentity(
    USHORT cNumRecs,
    PCSXS_NODE_INFO prgNodeInfo
    )
{
    FN_PROLOG_WIN32
    CSmartPtrWithNamedDestructor<ASSEMBLY_IDENTITY, ::SxsDestroyAssemblyIdentity> pAssemblyIdentity;
    ULONG ParseType;

    ASSERT(cNumRecs != 0);
    ASSERT(prgNodeInfo != NULL);

    // We're either parsing a manifest or a policy file; what else??
    INTERNAL_ERROR_CHECK(
            (m_ParseType == XML_FILE_TYPE_MANIFEST) ||
            (m_ParseType == XML_FILE_TYPE_APPLICATION_CONFIGURATION) ||
            (m_ParseType == XML_FILE_TYPE_COMPONENT_CONFIGURATION));
    
    if (m_fAssemblyIdentityChildOfDependenctAssemblyHit == false)
        m_fAssemblyIdentityChildOfDependenctAssemblyHit = true;
    else
    {
        this->LogParseError(MSG_SXS_MANIFEST_MULTIPLE_ASSEMBLYIDENTITY_IN_DEPENDENCYASSEMBLY);
        goto Exit;
    }

    switch (m_IntuitedParseType)
    {
    case eActualParseType_Undetermined:
        ParseType = m_ParseType;
        break;
    case eActualParseType_PolicyManifest:
        ParseType = XML_FILE_TYPE_COMPONENT_CONFIGURATION;
        break;
    case eActualParseType_Manifest:
        ParseType = XML_FILE_TYPE_MANIFEST;
        break;
    default:
        INTERNAL_ERROR_CHECK(FALSE);
        ParseType = m_ParseType;
        break;
    }
    switch (ParseType)
    {
    case XML_FILE_TYPE_MANIFEST:
        IFW32FALSE_EXIT(
            ::SxspCreateAssemblyIdentityFromIdentityElement(
                0,
                &m_ParseContext,
                ASSEMBLY_IDENTITY_TYPE_REFERENCE,
                &pAssemblyIdentity,
                cNumRecs,
                prgNodeInfo));

        IFW32FALSE_EXIT(
            this->ValidateIdentity(
                eValidateIdentity_PoliciesNotAllowed| eValidateIdentity_VersionRequired,
                ASSEMBLY_IDENTITY_TYPE_REFERENCE,
                pAssemblyIdentity));

        //
        // If we're not installing, process the identity...
        //
        // Note that in a strange twist on refcounting, SxspEnqueueAssemblyReference does not
        // hold the reference, it clones it.  That's why we don't .Detach from the pAssemblyIdentity
        // smart pointer here.
        //
        if (m_ActCtxGenCtx->m_ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT)
            IFW32FALSE_EXIT(::SxspEnqueueAssemblyReference(m_ActCtxGenCtx, m_Assembly, pAssemblyIdentity, m_fIsDependencyOptional, m_fIsMetadataSatellite));

        break;

    case XML_FILE_TYPE_COMPONENT_CONFIGURATION:
        {
        BOOL fValidDependencyAssemblyIdentity = FALSE;
        PCWSTR pszName1 = NULL, pszName2 = NULL;
        SIZE_T cchName1 = 0, cchName2 = 0;

        if (m_CurrentPolicyDependentAssemblyIdentity != NULL)
        {
            this->LogParseError(MSG_SXS_COMPONENT_CONFIGURATION_MANIFESTS_MAY_ONLY_HAVE_ONE_DEPENDENCY);
            goto Exit;
        }

        IFW32FALSE_EXIT(
            ::SxspCreateAssemblyIdentityFromIdentityElement(
                0,
                &m_ParseContext,
                ASSEMBLY_IDENTITY_TYPE_REFERENCE,
                &pAssemblyIdentity,
                cNumRecs,
                prgNodeInfo));
        // check the name in dependency-assemblyidentity match with the name in assembly-assemblyidentity        
        IFW32FALSE_EXIT(
            ::SxspGetAssemblyIdentityAttributeValue(
                SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
                m_Assembly->GetAssemblyIdentity(),
                &s_IdentityAttribute_name,
                &pszName1,          // something in a format of "Policy.1212.1221.assemblyname"
                &cchName1));

        IFW32FALSE_EXIT(
            ::SxspGetAssemblyIdentityAttributeValue(
                SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
                pAssemblyIdentity,
                &s_IdentityAttribute_name,
                &pszName2,          // would be something as "assemblyname"
                &cchName2));

        if ((cchName1 > cchName2) && (cchName2 !=0))
        {
            if ( (*(pszName1 + (cchName1 - cchName2 -1)) == L'.') && (::FusionpCompareStrings(
                            pszName1 + (cchName1 - cchName2), cchName2,
                            pszName2, cchName2, FALSE // must be case-sensitive for values
                            ) == 0 ))  
            {                
                fValidDependencyAssemblyIdentity = TRUE;
            }
        }
    
        if (fValidDependencyAssemblyIdentity) 
        {
            IFW32FALSE_EXIT(
                this->ValidateIdentity(
                    eValidateIdentity_PoliciesNotAllowed | eValidateIdentity_VersionNotAllowed,
                    ASSEMBLY_IDENTITY_TYPE_REFERENCE,
                    pAssemblyIdentity));

            // We'll keep track of this so that we can recognize multiple dependentAssembly elements on installation
            // of policies.
            INTERNAL_ERROR_CHECK(m_CurrentPolicyDependentAssemblyIdentity == NULL);
            m_CurrentPolicyDependentAssemblyIdentity = pAssemblyIdentity.Detach();
        }
        else // print a message and ignore this entry
        {           
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_POLICY | FUSION_DBG_LEVEL_INFO,
                "SXS.DLL: unexpected assemblyidentity within dependency tag in component policy \"%ls\"\n",                
                m_buffCurrentFileName
                );
        }
        } // end of this case   

        break;

    default:
        // Internal error!
        INTERNAL_ERROR_CHECK(FALSE);
    }

    FN_EPILOG
}

BOOL
CNodeFactory::XMLParser_Element_doc_configuration(
    USHORT cNumRecs,
    PCSXS_NODE_INFO prgNodeInfo
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    ULONG i;

    ASSERT(cNumRecs != 0);
    ASSERT(prgNodeInfo != NULL);

    if (m_fAssemblyFound)
    {
        CUnicodeString s;
        PCWSTR ManifestPath;
        IFW32FALSE_EXIT(m_Assembly->GetManifestPath(&ManifestPath, NULL));
        s = ManifestPath;
        this->LogParseError(MSG_SXS_MANIFEST_MULTIPLE_TOP_ASSEMBLY, &s);
        goto Exit;
    }

    m_fAssemblyFound = true;

    m_fMetadataSatelliteAlreadyFound = false;

    // Now let's tell all the contributors that we're about to begin a parsing session.
    for (i=0; i<m_ActCtxGenCtx->m_ContributorCount; i++)
    {
        IFW32FALSE_EXIT(
            m_ActCtxGenCtx->m_Contributors[i].Fire_ParseBeginning(
                m_ActCtxGenCtx,
                m_AssemblyContext,
                0, // FileFlags
                m_ParseType,
                m_ParseContext.SourceFilePathType,
                m_ParseContext.SourceFile,
                m_ParseContext.SourceFileCch,
                m_ParseContext.SourceFileLastWriteTime,
                m_Assembly->m_ManifestVersionMajor,
                m_Assembly->m_ManifestVersionMinor,
                m_Assembly->m_MetadataSatelliteRosterIndex));
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
CNodeFactory::XMLParser_Element_doc_configuration_windows(
    USHORT cNumRecs,
    PCSXS_NODE_INFO prgNodeInfo
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    fSuccess = TRUE;
    // Exit:
    return fSuccess;
}

BOOL
CNodeFactory::XMLParser_Element_doc_configuration_windows_assemblyBinding(
    USHORT cNumRecs,
    PCSXS_NODE_INFO prgNodeInfo
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    fSuccess = TRUE;
    // Exit:
    return fSuccess;
}

BOOL
CNodeFactory::XMLParser_Element_doc_configuration_windows_assemblyBinding_assemblyIdentity(
    USHORT cNumRecs,
    PCSXS_NODE_INFO prgNodeInfo
    )
{
    FN_PROLOG_WIN32;
    CSmartPtrWithNamedDestructor<ASSEMBLY_IDENTITY, ::SxsDestroyAssemblyIdentity> pAssemblyIdentity;

    IFW32FALSE_EXIT(
        ::SxspCreateAssemblyIdentityFromIdentityElement(
            0,
            &m_ParseContext,
            ASSEMBLY_IDENTITY_TYPE_REFERENCE,
            &pAssemblyIdentity,
            cNumRecs,
            prgNodeInfo));

    IFW32FALSE_EXIT(
        this->ValidateIdentity(
            eValidateIdentity_PoliciesNotAllowed | eValidateIdentity_VersionRequired,
            ASSEMBLY_IDENTITY_TYPE_REFERENCE,
            pAssemblyIdentity));

    FN_EPILOG;
}

BOOL
CNodeFactory::XMLParser_Element_doc_configuration_windows_assemblyBinding_dependentAssembly(
    USHORT cNumRecs,
    PCSXS_NODE_INFO prgNodeInfo
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    fSuccess = TRUE;
    // Exit:
    return fSuccess;
}

BOOL
CNodeFactory::XMLParser_Element_doc_configuration_windows_assemblyBinding_dependentAssembly_assemblyIdentity(
    USHORT cNumRecs,
    PCSXS_NODE_INFO prgNodeInfo
    )
{
    FN_PROLOG_WIN32;
    CSmartPtrWithNamedDestructor<ASSEMBLY_IDENTITY, ::SxsDestroyAssemblyIdentity> pAssemblyIdentity;
    CSmartPtr<CPolicyStatement> pPolicyStatement;
    CPolicyStatement *pFoundPolicyStatement = NULL;

    IFW32FALSE_EXIT(
        ::SxspCreateAssemblyIdentityFromIdentityElement(
            0,
            &m_ParseContext,
            ASSEMBLY_IDENTITY_TYPE_REFERENCE,
            &pAssemblyIdentity,
            cNumRecs,
            prgNodeInfo));

    IFW32FALSE_EXIT(
        this->ValidateIdentity(
            eValidateIdentity_PoliciesNotAllowed | eValidateIdentity_VersionNotAllowed,
            ASSEMBLY_IDENTITY_TYPE_REFERENCE,
            pAssemblyIdentity));

    IFW32FALSE_EXIT(
        ::SxspGenerateTextuallyEncodedPolicyIdentityFromAssemblyIdentity(
            SXSP_GENERATE_TEXTUALLY_ENCODED_POLICY_IDENTITY_FROM_ASSEMBLY_IDENTITY_FLAG_OMIT_ENTIRE_VERSION,
            pAssemblyIdentity,
            m_buffCurrentApplicationPolicyIdentityKey,
            NULL));

    IFW32FALSE_EXIT(m_ActCtxGenCtx->m_ApplicationPolicyTable.Find(m_buffCurrentApplicationPolicyIdentityKey, pFoundPolicyStatement));
    if (pFoundPolicyStatement != NULL)
    {
        this->LogParseError(MSG_SXS_APPLICATION_CONFIGURATION_MANIFEST_MAY_ONLY_HAVE_ONE_DEPENDENTASSEMBLY_PER_IDENTITY);
        goto Exit;
    }


    IFALLOCFAILED_EXIT(pPolicyStatement.Win32Allocate(__FILE__, __LINE__));
    IFW32FALSE_EXIT(pPolicyStatement->Initialize());
    IFW32FALSE_EXIT(m_ActCtxGenCtx->m_ApplicationPolicyTable.Insert(m_buffCurrentApplicationPolicyIdentityKey, pPolicyStatement));

    // preset it to be global value about m_fApplyPublisherPolicy, 
    // and be reset in dependentAssembly_publisherPolicy function if present
    if ((this->m_ActCtxGenCtx->m_fAppApplyPublisherPolicy == SXS_PUBLISHER_POLICY_APPLY_YES) || (this->m_ActCtxGenCtx->m_fAppApplyPublisherPolicy == SXS_PUBLISHER_POLICY_APPLY_DEFAULT))
        pPolicyStatement->m_fApplyPublisherPolicy = true;
    else
        pPolicyStatement->m_fApplyPublisherPolicy = false;

    m_CurrentPolicyStatement = pPolicyStatement.Detach();

    if (m_CurrentPolicyDependentAssemblyIdentity != NULL)
    {
        ::SxsDestroyAssemblyIdentity(m_CurrentPolicyDependentAssemblyIdentity);
        m_CurrentPolicyDependentAssemblyIdentity = NULL;
    }

    m_CurrentPolicyDependentAssemblyIdentity = pAssemblyIdentity.Detach();

    FN_EPILOG;
}



BOOL
CNodeFactory::XMLParser_Element_doc_configuration_windows_assemblyBinding_dependentAssembly_publisherPolicy(
    USHORT cNumRecs, 
    PCSXS_NODE_INFO prgNodeInfo
    )
{
    FN_PROLOG_WIN32;
    
    if (m_CurrentPolicyStatement == NULL)
    {
        this->LogParseError(MSG_SXS_APPLICATION_CONFIGURATION_MANIFEST_DEPENDENTASSEMBLY_MISSING_IDENTITY);
        goto Exit;
    }

    if (this->m_ActCtxGenCtx->m_fAppApplyPublisherPolicy == SXS_PUBLISHER_POLICY_APPLY_NO)
        m_CurrentPolicyStatement->m_fApplyPublisherPolicy = false;
    else
    {
        bool fFound, fApplyPolicy;
        SIZE_T cb;

        IFW32FALSE_EXIT(
            ::SxspGetAttributeValue(
                0,
                &s_AttributeName_apply,
                prgNodeInfo,
                cNumRecs,
                &m_ParseContext,
                fFound,
                sizeof(fApplyPolicy),
                &fApplyPolicy,
                cb,
                SxspValidateBoolAttribute,
                0));

        INTERNAL_ERROR_CHECK(fFound); // if not found, syntax error in the manifest               
        
        if (fApplyPolicy == false)
        {
            //
            // if this tag appears, appcompat flags must be set, otherwise it is an error
            //
            if (!(this->m_ActCtxGenCtx->m_Flags & SXS_GENERATE_ACTCTX_APP_RUNNING_IN_SAFEMODE))
            {
                ::FusionpDbgPrintEx(
                    FUSION_DBG_LEVEL_ERROR,
                    "SXS.DLL: %s() app-level PublisherPolicy try to be set but there is no appcompat flags been set.\n", __FUNCTION__);
                ::SetLastError(ERROR_SXS_MANIFEST_PARSE_ERROR);
                goto Exit;
            }
        }
        m_CurrentPolicyStatement->m_fApplyPublisherPolicy = fApplyPolicy;
    }

    FN_EPILOG;
}


BOOL
CNodeFactory::XMLParser_Element_doc_configuration_windows_assemblyBinding_publisherPolicy(
    USHORT cNumRecs, 
    PCSXS_NODE_INFO prgNodeInfo
    )
{
    FN_PROLOG_WIN32;

    bool fFound, fApplyPolicy;
    SIZE_T cb;

    if (this->m_ActCtxGenCtx->m_fAppApplyPublisherPolicy != SXS_PUBLISHER_POLICY_APPLY_DEFAULT)
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %s() called but app-level PublisherPolicy has already been set.\n", __FUNCTION__);
        ::SetLastError(ERROR_SXS_MANIFEST_PARSE_ERROR);
        goto Exit;
    }

    IFW32FALSE_EXIT(
        ::SxspGetAttributeValue(
            0,
            &s_AttributeName_apply,
            prgNodeInfo,
            cNumRecs,
            &m_ParseContext,
            fFound,
            sizeof(fApplyPolicy),
            &fApplyPolicy,
            cb,
            SxspValidateBoolAttribute,
            0));

    INTERNAL_ERROR_CHECK(fFound);
    
    if (!fApplyPolicy)
    {
        //
        // if this tag set to be "no", appcompat flags must be set, otherwise it is an error
        //
        if (!(this->m_ActCtxGenCtx->m_Flags & SXS_GENERATE_ACTCTX_APP_RUNNING_IN_SAFEMODE))
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: %s() app-level PublisherPolicy try to be set but there is no appcompat flags been set.\n", __FUNCTION__);
            ::SetLastError(ERROR_SXS_MANIFEST_PARSE_ERROR);
            goto Exit;
        }

        this->m_ActCtxGenCtx->m_fAppApplyPublisherPolicy = SXS_PUBLISHER_POLICY_APPLY_NO;
    }
    else
        this->m_ActCtxGenCtx->m_fAppApplyPublisherPolicy = SXS_PUBLISHER_POLICY_APPLY_YES;    

    FN_EPILOG;
}

BOOL
CNodeFactory::XMLParser_Element_doc_configuration_windows_assemblyBinding_dependentAssembly_bindingRedirect(
    USHORT cNumRecs,
    PCSXS_NODE_INFO prgNodeInfo
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    CSmallStringBuffer buffOldVersion;
    CSmallStringBuffer buffNewVersion;
    bool fFound;
    bool fValid;
    SIZE_T cb;

    if (m_CurrentPolicyStatement == NULL)
    {
        this->LogParseError(MSG_SXS_APPLICATION_CONFIGURATION_MANIFEST_DEPENDENTASSEMBLY_MISSING_IDENTITY);
        goto Exit;
    }

    IFW32FALSE_EXIT(
        ::SxspGetAttributeValue(
            SXSP_GET_ATTRIBUTE_VALUE_FLAG_REQUIRED_ATTRIBUTE,
            &s_AttributeName_oldVersion,
            prgNodeInfo,
            cNumRecs,
            &m_ParseContext,
            fFound,
            sizeof(buffOldVersion),
            &buffOldVersion,
            cb,
            NULL,
            0));
    INTERNAL_ERROR_CHECK(fFound);

    IFW32FALSE_EXIT(
        ::SxspGetAttributeValue(
            SXSP_GET_ATTRIBUTE_VALUE_FLAG_REQUIRED_ATTRIBUTE,
            &s_AttributeName_newVersion,
            prgNodeInfo,
            cNumRecs,
            &m_ParseContext,
            fFound,
            sizeof(buffNewVersion),
            &buffNewVersion,
            cb,
            NULL,
            0));
    INTERNAL_ERROR_CHECK(fFound);

    // If either are not found, log an error
    if (!fFound)
    {
        this->LogParseError(MSG_SXS_BINDING_REDIRECT_MISSING_REQUIRED_ATTRIBUTES);
        goto Exit;
    }

    IFW32FALSE_EXIT(m_CurrentPolicyStatement->AddRedirect(buffOldVersion, buffNewVersion, fValid));
    if (! fValid)
    {       
        // log an error
        ::FusionpLogError(
            MSG_SXS_POLICY_VERSION_OVERLAP,
            CEventLogString(m_AssemblyContext->PolicyPath),
            CEventLogString(buffOldVersion),
            CEventLogString(buffNewVersion));

        ORIGINATE_WIN32_FAILURE_AND_EXIT(PolicyVersionOverlap, ERROR_SXS_MANIFEST_PARSE_ERROR);
    }


    fSuccess = TRUE;
Exit:
    return fSuccess;
}


//
// ISSUE: jonwis 3/11/2002 - Smells like dead code.  CPartialAssemblyVersion isn't constrcted anywhere,
//          and this function isn't called by anyone.
//
// NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Dead code, probably
BOOL
CNodeFactory::XMLParser_Parse_PartialAssemblyVersion(
    PVOID pvDatum,
    BOOL fAlreadyFound,
    CBaseStringBuffer &rbuff
    )
{
    return reinterpret_cast<CPartialAssemblyVersion *>(pvDatum)->Parse(rbuff, rbuff.Cch());
}


//
// ISSUE: jonwis 3/11/2002 - Same here... never called anywhere, dead code.
//
// NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Dead code, probably
BOOL
CNodeFactory::XMLParser_Parse_String(
    LPVOID pvDatum,
    BOOL fAlreadyFound,
    CBaseStringBuffer &rbuff)
{
    return ((CBaseStringBuffer *) pvDatum)->Win32Assign(rbuff);
}

//
// ISSUE: jonwis 3/11/2002 - More dead code.  Will the madness ever end??
//
// NTRAID#NTBUG9 - 572507 - jonwis - 2002/04/25 - Dead code probably
BOOL
CNodeFactory::ParseElementAttributes(
    USHORT cNumRecs,
    XML_NODE_INFO **prgpNodeInfo,
    SIZE_T cEntries,
    const AttributeMapEntry *prgEntries
    )
{
    FN_PROLOG_WIN32

    ULONG i, j;

    for (i=1; i<cNumRecs; i++)
    {
        // Skip things we don't understand.
        if (prgpNodeInfo[i]->dwType != XML_ATTRIBUTE)
            continue;

        for (j=0; j<cEntries; j++)
        {
            if (::FusionpEqualStrings(
                    prgEntries[j].m_pszAttributeName,
                    prgEntries[j].m_cchAttributeName,
                    prgpNodeInfo[i]->pwcText,
                    prgpNodeInfo[i]->ulLen,
                    false))
            {
                // Because attribute values may be multipart due to entity references,
                // we accumulate the attibute value into buffTemp to start, and then do
                // the parsing/whatever afterwards.
                CStringBuffer buffTemp;
                BOOL *pfIndicator = (BOOL *) (((ULONG_PTR) this) + prgEntries[j].m_offsetIndicator);

                while ((++i < cNumRecs) &&
                       (prgpNodeInfo[i]->dwType == XML_PCDATA))
                {
                    IFW32FALSE_EXIT(buffTemp.Win32Append(prgpNodeInfo[i]->pwcText, prgpNodeInfo[i]->ulLen));
                }

                // The outer for(;;) loop is going to increment i, so we need to back it up one
                // place...
                i--;

                // Call the appropriate value type handler function...
                if (prgEntries[j].m_pfn != NULL)
                {
                    IFW32FALSE_EXIT((this->*(prgEntries[j].m_pfn))(((LPBYTE) this) + prgEntries[j].m_offsetData, *pfIndicator, buffTemp));
                }

                *pfIndicator = TRUE;

                break;
            }
        }
    }

    FN_EPILOG
}

HRESULT
CNodeFactory::LogParseError(
    DWORD dwLastParseError,
    const UNICODE_STRING *p1,
    const UNICODE_STRING *p2,
    const UNICODE_STRING *p3,
    const UNICODE_STRING *p4,
    const UNICODE_STRING *p5,
    const UNICODE_STRING *p6,
    const UNICODE_STRING *p7,
    const UNICODE_STRING *p8,
    const UNICODE_STRING *p9,
    const UNICODE_STRING *p10,
    const UNICODE_STRING *p11,
    const UNICODE_STRING *p12,
    const UNICODE_STRING *p13,
    const UNICODE_STRING *p14,
    const UNICODE_STRING *p15,
    const UNICODE_STRING *p16,
    const UNICODE_STRING *p17,
    const UNICODE_STRING *p18,
    const UNICODE_STRING *p19,
    const UNICODE_STRING *p20
    )
{
    return
        ::FusionpLogParseError(
            m_ParseContext.SourceFile,
            m_ParseContext.SourceFileCch,
            m_ParseContext.LineNumber,
            dwLastParseError,
            p1, p2, p3, p4, p5,
            p6, p7, p8, p9, p10,
            p11, p12, p13, p14, p15,
            p16, p17, p18, p19, p20);
}

VOID
CNodeFactory::ParseErrorCallback_MissingRequiredAttribute(
    PCACTCTXCTB_PARSE_CONTEXT ParseContext,
    IN PCATTRIBUTE_NAME_DESCRIPTOR AttributeName
    )
{
    // CNodeFactory *pThis = (CNodeFactory *) ErrorContext;

    ::FusionpLogRequiredAttributeMissingParseError(
        ParseContext->SourceFile,
        ParseContext->SourceFileCch,
        ParseContext->LineNumber,
        ParseContext->ElementName,
        ParseContext->ElementNameCch,
        AttributeName->Name,
        AttributeName->NameCch);
}

VOID
CNodeFactory::ParseErrorCallback_InvalidAttributeValue(
    PCACTCTXCTB_PARSE_CONTEXT ParseContext,
    IN PCATTRIBUTE_NAME_DESCRIPTOR AttributeName
    )
{
    // CNodeFactory *pThis = (CNodeFactory *) ErrorContext;

    ::FusionpLogInvalidAttributeValueParseError(
        ParseContext->SourceFile,
        ParseContext->SourceFileCch,
        ParseContext->LineNumber,
        ParseContext->ElementName,
        ParseContext->ElementNameCch,
        AttributeName->Name,
        AttributeName->NameCch);
}

VOID
CNodeFactory::ParseErrorCallback_AttributeNotAllowed(
    PCACTCTXCTB_PARSE_CONTEXT ParseContext,
    IN PCATTRIBUTE_NAME_DESCRIPTOR AttributeName
    )
{
    // CNodeFactory *pThis = (CNodeFactory *) ErrorContext;

    ::FusionpLogAttributeNotAllowedParseError(
        ParseContext->SourceFile,
        ParseContext->SourceFileCch,
        ParseContext->LineNumber,
        ParseContext->ElementName,
        ParseContext->ElementNameCch,
        AttributeName->Name,
        AttributeName->NameCch);
}

VOID
SxspDbgPrintXmlNodeInfo(
    ULONG Level,
    XML_NODE_INFO *pNode
    )
{
    CUnicodeString s(pNode->pwcText, pNode->ulLen);

#if DBG_SXS
    ::FusionpDbgPrintEx(Level, "SXS.DLL: XML_NODE_INFO at %p\n", pNode);
    ::FusionpDbgPrintEx(Level, "    dwSize = %d\n", pNode->dwSize);
    ::FusionpDbgPrintEx(Level, "    dwType = %d (%s)\n", pNode->dwType, SxspFormatXmlNodeType(pNode->dwType));
    ::FusionpDbgPrintEx(Level, "    dwSubType = %d\n", pNode->dwSubType);
    ::FusionpDbgPrintEx(Level, "    fTerminal = %d\n", pNode->fTerminal);

    ::FusionpDbgPrintEx(Level, "    pwcText = %p (\"%wZ\")\n", pNode->pwcText, &s);
    ::FusionpDbgPrintEx(Level, "    ulLen = %d\n", pNode->ulLen);
    ::FusionpDbgPrintEx(Level, "    ulNsPrefixLen = %d\n", pNode->ulNsPrefixLen);
    ::FusionpDbgPrintEx(Level, "    pNode = %p\n", pNode->pNode);
    ::FusionpDbgPrintEx(Level, "    pReserved = %p\n", pNode->pReserved);
#else
    ::FusionpDbgPrintEx(Level, "SXS.DLL: XML_NODE_INFO at %p: \"%wZ\"\n", pNode, &s);
#endif
}


PCSTR
SxspFormatXmlNodeType(
    DWORD dwType
    )
{
    PCSTR Result = "Unknown";

#define HANDLE_NODE_TYPE(x) case static_cast<DWORD>(x): Result = #x; break;

    switch (dwType)
    {
        HANDLE_NODE_TYPE(XML_ELEMENT)
        HANDLE_NODE_TYPE(XML_ATTRIBUTE)
        HANDLE_NODE_TYPE(XML_PI)
        HANDLE_NODE_TYPE(XML_XMLDECL)
        HANDLE_NODE_TYPE(XML_DOCTYPE)
        HANDLE_NODE_TYPE(XML_DTDATTRIBUTE)
        HANDLE_NODE_TYPE(XML_ENTITYDECL)
        HANDLE_NODE_TYPE(XML_ELEMENTDECL)
        HANDLE_NODE_TYPE(XML_ATTLISTDECL)
        HANDLE_NODE_TYPE(XML_NOTATION)
        HANDLE_NODE_TYPE(XML_GROUP)
        HANDLE_NODE_TYPE(XML_INCLUDESECT)
        HANDLE_NODE_TYPE(XML_PCDATA)
        HANDLE_NODE_TYPE(XML_CDATA)
        HANDLE_NODE_TYPE(XML_IGNORESECT)
        HANDLE_NODE_TYPE(XML_COMMENT)
        HANDLE_NODE_TYPE(XML_ENTITYREF)
        HANDLE_NODE_TYPE(XML_WHITESPACE)
        HANDLE_NODE_TYPE(XML_NAME)
        HANDLE_NODE_TYPE(XML_NMTOKEN)
        HANDLE_NODE_TYPE(XML_STRING)
        HANDLE_NODE_TYPE(XML_PEREF)
        HANDLE_NODE_TYPE(XML_MODEL)
        HANDLE_NODE_TYPE(XML_ATTDEF)
        HANDLE_NODE_TYPE(XML_ATTTYPE)
        HANDLE_NODE_TYPE(XML_ATTPRESENCE)
        HANDLE_NODE_TYPE(XML_DTDSUBSET)
    }

    return Result;
}

BOOL
CNodeFactory::ValidateIdentity(
    DWORD Flags,
    ULONG Type,
    PCASSEMBLY_IDENTITY AssemblyIdentity
    )
{
    FN_PROLOG_WIN32
    PCWSTR pszTemp = NULL;
    SIZE_T cchTemp = 0;
    bool fSyntaxValid = false;
    bool fError = false;
    BOOL fIsPolicy;

    PARAMETER_CHECK((Flags & ~(
                            eValidateIdentity_VersionRequired |
                            eValidateIdentity_PoliciesNotAllowed |
                            eValidateIdentity_VersionNotAllowed)) == 0);
    PARAMETER_CHECK((Type == ASSEMBLY_IDENTITY_TYPE_DEFINITION) || (Type == ASSEMBLY_IDENTITY_TYPE_REFERENCE));
    PARAMETER_CHECK(AssemblyIdentity != NULL);

    //
    // only one of these flags is allowed
    //
    PARAMETER_CHECK(
        (Flags & (eValidateIdentity_VersionRequired | eValidateIdentity_VersionNotAllowed)) !=
                 (eValidateIdentity_VersionRequired | eValidateIdentity_VersionNotAllowed));

    //
    // Get the type of this assembly
    //
    IFW32FALSE_EXIT(::SxspDetermineAssemblyType(AssemblyIdentity, fIsPolicy));

    //
    // If it's policy, then make sure that policies are allowed.  Otherwise, fail out.
    //
    if (fIsPolicy)
    {
        m_AssemblyContext->Flags |= ACTCTXCTB_ASSEMBLY_CONTEXT_IS_SYSTEM_POLICY_INSTALLATION;

        if (Flags & eValidateIdentity_PoliciesNotAllowed)
        {
            CUnicodeString usType(pszTemp, cchTemp);

            fError = true;
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: Manifest \"%ls\" (line %d) has invalid type attribute \"%wZ\"; report to owner of \"%ls\"\n",
                m_ParseContext.SourceFile,
                m_ParseContext.LineNumber,
                &usType,
                m_ParseContext.SourceFile);
        }
    }

    IFW32FALSE_EXIT(
        ::SxspGetAssemblyIdentityAttributeValue(
            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
            AssemblyIdentity,
            &s_IdentityAttribute_name,
            &pszTemp,
            &cchTemp));

    if (cchTemp == 0)
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: Manifest \"%ls\" (line %d) is missing name attribute; report to owner of \"%ls\"\n",
            m_ParseContext.SourceFile,
            m_ParseContext.LineNumber,
            m_ParseContext.SourceFile);

        fError = true;
    }

    IFW32FALSE_EXIT(
        ::SxspGetAssemblyIdentityAttributeValue(
            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
            AssemblyIdentity,
            &s_IdentityAttribute_processorArchitecture,
            &pszTemp,
            &cchTemp));

    IFW32FALSE_EXIT(
        ::SxspGetAssemblyIdentityAttributeValue(
            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
            AssemblyIdentity,
            &s_IdentityAttribute_version,
            &pszTemp,
            &cchTemp));

    if (cchTemp != 0)
    {
        ASSEMBLY_VERSION av;

        IFW32FALSE_EXIT(CFusionParser::ParseVersion(av, pszTemp, cchTemp, fSyntaxValid));

        if (!fSyntaxValid)
        {
            ::FusionpLogInvalidAttributeValueParseError(
                m_ParseContext.SourceFile,
                m_ParseContext.SourceFileCch,
                m_ParseContext.LineNumber,
                m_ParseContext.ElementName,
                m_ParseContext.ElementNameCch,
                s_IdentityAttribute_version);

            ORIGINATE_WIN32_FAILURE_AND_EXIT(InvalidVersionNumber, ERROR_SXS_MANIFEST_PARSE_ERROR);
        }
    }

    if ((Flags & (eValidateIdentity_VersionNotAllowed | eValidateIdentity_VersionRequired)) != 0)
    {
        if ((Flags & eValidateIdentity_VersionNotAllowed) != 0 && cchTemp != 0)
        {
            fError = true;
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: Manifest \"%ls\" (line %d) has a version attribute where it may not appear; report to owner of \"%ls\"\n",
                m_ParseContext.SourceFile,
                m_ParseContext.LineNumber,
                m_ParseContext.SourceFile);
        }
        else if ((Flags & eValidateIdentity_VersionRequired) != 0 && cchTemp == 0)
        {
            fError = true;
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: Manifest \"%ls\" (line %d) is missing version attribute; report to owner of \"%ls\"\n",
                m_ParseContext.SourceFile,
                m_ParseContext.LineNumber,
                m_ParseContext.SourceFile);
        }
    }

    if (fError)
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: Manifest \"%ls\" is missing required attribute or contains disallowed attribute; report to owner of \"%ls\"\n",
            m_ParseContext.SourceFile,
            m_ParseContext.SourceFile);

        ORIGINATE_WIN32_FAILURE_AND_EXIT(InvalidIdentity, ERROR_SXS_MANIFEST_PARSE_ERROR);
    }

    FN_EPILOG
}

BOOL
CNodeFactory::ValidateElementAttributes(
    PCSXS_NODE_INFO prgNodes,
    SIZE_T cNodes,
    PCELEMENT_LEGAL_ATTRIBUTE prgAttributes,
    UCHAR cAttributes
    )
{
    FN_PROLOG_WIN32

    SIZE_T i;
    UCHAR j;
    UCHAR cRequiredAttributes, cRequiredAttributesFound;
    UCHAR rgRequiredAttributeFoundBitMask[8]; // 8 * 32 = 256
    BOOL fParseFailed = FALSE;

    PARAMETER_CHECK((cNodes == 0) || (prgNodes != NULL));
    PARAMETER_CHECK((cAttributes == 0) || (prgAttributes != NULL));

    cRequiredAttributes = 0;
    cRequiredAttributesFound = 0;

    for (i=0; i<cAttributes; i++)
        if (prgAttributes[i].m_dwFlags & ELEMENT_LEGAL_ATTRIBUTE_FLAG_REQUIRED)
            cRequiredAttributes++;

    rgRequiredAttributeFoundBitMask[0] = 0;
    rgRequiredAttributeFoundBitMask[1] = 0;
    rgRequiredAttributeFoundBitMask[2] = 0;
    rgRequiredAttributeFoundBitMask[3] = 0;
    rgRequiredAttributeFoundBitMask[4] = 0;
    rgRequiredAttributeFoundBitMask[5] = 0;
    rgRequiredAttributeFoundBitMask[6] = 0;
    rgRequiredAttributeFoundBitMask[7] = 0;

    for (i=0; i<cNodes; i++)
    {
        if (prgNodes[i].Type == SXS_ATTRIBUTE)
        {
            const SIZE_T cchText = prgNodes[i].cchText;
            const SIZE_T cchNamespace = prgNodes[i].NamespaceStringBuf.Cch();
            const PCWSTR pszText = prgNodes[i].pszText;

            // Ignore any attributes that start with xml
            if ((cchText >= 3) &&
                ((pszText[0] == L'x') || (pszText[0] == L'X')) &&
                ((pszText[1] == L'm') || (pszText[1] == L'M')) &&
                ((pszText[2] == L'l') || (pszText[2] == L'L')))
            {
                continue;
            }
            if (cchNamespace != 0 )
            {
                continue;
            }

            for (j=0; j<cAttributes; j++)
            {
                if ((prgAttributes[j].m_pName != NULL) &&
                    ::FusionpEqualStrings(prgNodes[i].NamespaceStringBuf, cchNamespace, prgAttributes[j].m_pName->Namespace, prgAttributes[j].m_pName->NamespaceCch, false) &&
                    ::FusionpEqualStrings(pszText, cchText, prgAttributes[j].m_pName->Name, prgAttributes[j].m_pName->NameCch, false))
                {
                    if (prgAttributes[j].m_pfnValidator != NULL)
                    {
                        CSmallStringBuffer buffValue;
                        bool fValid = false;
                        SIZE_T cb;
                        SIZE_T i2;

                        for (i2=i+1; i2<cNodes; i2++)
                        {
                            if (prgNodes[i2].Type == SXS_PCDATA)
                                IFW32FALSE_EXIT(buffValue.Win32Append(prgNodes[i2].pszText, prgNodes[i2].cchText));
                            else
                                break;
                        }

                        IFW32FALSE_EXIT(
                            (*prgAttributes[j].m_pfnValidator)(
                                prgAttributes[j].m_dwValidatorFlags,
                                buffValue,
                                fValid,
                                0,
                                NULL,
                                cb));

                        if (!fValid)
                        {
                            ::FusionpLogInvalidAttributeValueParseError(
                                m_ParseContext.SourceFile,
                                m_ParseContext.SourceFileCch,
                                m_ParseContext.LineNumber,
                                m_ParseContext.ElementName,
                                m_ParseContext.ElementNameCch,
                                prgAttributes[j].m_pName->Name,
                                prgAttributes[j].m_pName->NameCch);
                            
                            ORIGINATE_WIN32_FAILURE_AND_EXIT(InvalidAttributeValue, ERROR_SXS_MANIFEST_PARSE_ERROR);
                        }
                    }

                    if (prgAttributes[j].m_dwFlags & ELEMENT_LEGAL_ATTRIBUTE_FLAG_REQUIRED)
                    {
                        rgRequiredAttributeFoundBitMask[(j / 32)] |= (1 << (j % 32));
                        cRequiredAttributesFound++;
                    }

                    break;
                }
            }

            if (j == cAttributes)
            {
                // We found an illegal attribute!!
                ::FusionpLogAttributeNotAllowedParseError(
                    m_ParseContext.SourceFile,
                    m_ParseContext.SourceFileCch,
                    m_ParseContext.LineNumber,
                    prgNodes[0].pszText,
                    prgNodes[0].cchText,
                    prgNodes[i].pszText,
                    prgNodes[i].cchText);

                // We don't just go to exit here because we want to report all the bad attributes and missing attributes...
                fParseFailed = TRUE;
            }
        }
    }

    if (cRequiredAttributesFound != cRequiredAttributes)
    {
        for (j=0; j<cAttributes; j++)
        {
            if (prgAttributes[j].m_dwFlags & ELEMENT_LEGAL_ATTRIBUTE_FLAG_REQUIRED)
            {
                if ((rgRequiredAttributeFoundBitMask[(j / 32)] & (1 << (j % 32))) == 0)
                {
                    ::FusionpLogRequiredAttributeMissingParseError(
                        m_ParseContext.SourceFile,
                        m_ParseContext.SourceFileCch,
                        m_ParseContext.LineNumber,
                        prgNodes[0].pszText,
                        prgNodes[0].cchText,
                        prgAttributes[j].m_pName->Name,
                        prgAttributes[j].m_pName->NameCch);

                    fParseFailed = TRUE;
                }
            }
        }
    }

    if (fParseFailed)
        ORIGINATE_WIN32_FAILURE_AND_EXIT(ParseError, ERROR_SXS_MANIFEST_PARSE_ERROR);

    FN_EPILOG
}