/*++

Copyright (c) 2000 Microsoft Corporation

Module Name:

    renregfn.c

Abstract:

    Implements code-based registry rename.

Author:

    Jim Schmidt (jimschm) 15-Sep-2000

Revision History:

    <alias> <date> <comments>

--*/

//
// Includes
//

#include "pch.h"
#include "v1p.h"

#define DBG_RENREGFN    "RenRegFn"

//
// Strings
//

// None

//
// Constants
//

// None

//
// Macros
//

// None

//
// Types
//

typedef VOID(RENREGFNINIT)(MIG_PLATFORMTYPEID);
typedef RENREGFNINIT *PRENREGFNINIT;

typedef BOOL (STDMETHODCALLTYPE RENAMERULE)(
    IN      PCTSTR OldNode,
    IN      PCTSTR NewNode,
    IN      PCTSTR Leaf,
    IN      BOOL NoRestoreObject,
    OUT     PMIG_FILTEROUTPUT OutputData
    );
typedef RENAMERULE FAR *LPRENAMERULE;

//
// Globals
//

PTSTR g_DestIdentityGUID = NULL;
BOOL g_OERulesMigrated = FALSE;

//
// Macro expansion list
//

//
// DEFMAC(<script tag>, <enum callback>, <operation name>, <op init>, <operation callback>)
//
// It is assumed that <operation callback> is a tree filter (it can modify part of a path).
// The code does not currently support the contrary.
//

#define DEFAULT_ENUM        pDefaultRenRegFnQueueCallback
#define DEFAULT_INIT        pNulInit

#define RENAME_FUNCTIONS        \
    DEFMAC(ConvertOE4,          DEFAULT_ENUM, MOVE.ConvertOE4,          pConvertOE4Init,    pConvertOE4Move    ) \
    DEFMAC(ConvertOE4IAM,       DEFAULT_ENUM, MOVE.ConvertOE4IAM,       pConvertOE4Init,    pConvertOEIAMMove  ) \
    DEFMAC(ConvertOE5IAM,       DEFAULT_ENUM, MOVE.ConvertOE5IAM,       pConvertOE5IAMInit, pConvertOEIAMMove  ) \
    DEFMAC(ConvertOE5IdIAM,     DEFAULT_ENUM, MOVE.ConvertOE5IdIAM,     DEFAULT_INIT,       pConvertOEIdIAMMove) \
    DEFMAC(ConvertOE5MailRules, DEFAULT_ENUM, MOVE.ConvertOE5MailRules, DEFAULT_INIT,       pConvertOE5MailRulesMove) \
    DEFMAC(ConvertOE5NewsRules, DEFAULT_ENUM, MOVE.ConvertOE5NewsRules, DEFAULT_INIT,       pConvertOE5NewsRulesMove) \
    DEFMAC(ConvertOE5Block,     DEFAULT_ENUM, MOVE.ConvertOE5Block,     DEFAULT_INIT,       pConvertOE5BlockMove) \

//
// Private function prototypes
//

// None

//
// Macro expansion definition
//

//
// Declare special rename operation apply callback functions
//
#define DEFMAC(ifn,ec,opn,opi,opc) SGMENUMERATIONCALLBACK ec; RENREGFNINIT opi; OPMFILTERCALLBACK opc;
RENAME_FUNCTIONS
#undef DEFMAC

//
// This is the structure used for handling action functions
//
typedef struct {
    PCTSTR InfFunctionName;
    PSGMENUMERATIONCALLBACK EnumerationCallback;
    PCTSTR OperationName;
    MIG_OPERATIONID OperationId;
    PRENREGFNINIT OperationInit;
    POPMFILTERCALLBACK OperationCallback;
} RENAME_STRUCT, *PRENAME_STRUCT;

//
// Declare a global array of rename functions
//
#define DEFMAC(ifn,ec,opn,opi,opc) {TEXT("\\")TEXT(#ifn),ec,TEXT(#opn),0,opi,opc},
static RENAME_STRUCT g_RenameFunctions[] = {
                              RENAME_FUNCTIONS
                              {NULL, NULL, NULL, 0, NULL, NULL}
                              };
#undef DEFMAC

//
// Code
//

VOID
pNulInit (
    IN      MIG_PLATFORMTYPEID Platform
    )
{
}

UINT
pDefaultRenRegFnQueueCallback (
    IN      PCMIG_OBJECTENUMDATA Data,
    IN      ULONG_PTR CallerArg
    )
{
    PRENAME_STRUCT p = (PRENAME_STRUCT)CallerArg;

    IsmSetOperationOnObject (Data->ObjectTypeId, Data->ObjectName, p->OperationId, NULL, NULL);

    return CALLBACK_ENUM_CONTINUE;
}

PRENAME_STRUCT
pGetRenameStruct (
    IN      PCTSTR FunctionName
    )
{
    PRENAME_STRUCT p = g_RenameFunctions;
    INT i = 0;
    while (p->InfFunctionName != NULL) {
        if (StringIMatch (p->InfFunctionName, FunctionName)) {
            return p;
        }
        p++;
        i++;
    }
    return NULL;
}

VOID
InitSpecialRename (
    IN      MIG_PLATFORMTYPEID Platform
    )
{
    PRENAME_STRUCT p = g_RenameFunctions;

    while (p->InfFunctionName) {
        p->OperationId = IsmRegisterOperation (p->OperationName, FALSE);
        if (Platform == PLATFORM_DESTINATION) {
            IsmRegisterOperationFilterCallback (p->OperationId, p->OperationCallback, TRUE, TRUE, FALSE);
        }

        p->OperationInit(Platform);
        p++;
    }
}

VOID
TerminateSpecialRename (
    VOID
    )
{
    if (g_DestIdentityGUID) {
        FreeText (g_DestIdentityGUID);
    }
}

BOOL
AddSpecialRenameRule (
    IN      PCTSTR Pattern,
    IN      PCTSTR Function
)
{
    PRENAME_STRUCT functionStruct = NULL;
    BOOL result = FALSE;

    functionStruct = pGetRenameStruct (Function);

    if (functionStruct) {
        result = IsmHookEnumeration (
            g_RegType,
            Pattern,
            functionStruct->EnumerationCallback?
                functionStruct->EnumerationCallback:
                pDefaultRenRegFnQueueCallback,
            (ULONG_PTR)functionStruct,
            functionStruct->InfFunctionName
            );
    } else {
        LOG ((
            LOG_ERROR,
            (PCSTR) MSG_DATA_RENAME_BAD_FN,
            Function,
            Pattern
            ));
    }

    return result;
}

BOOL
pProcessDataRenameSection (
    IN      PINFSTRUCT InfStruct,
    IN      HINF InfHandle,
    IN      PCTSTR Section
    )
{
    PCTSTR pattern;
    ENCODEDSTRHANDLE encodedPattern = NULL;
    PCTSTR functionName;
    BOOL result = FALSE;

    __try {
        if (InfFindFirstLine (InfHandle, Section, NULL, InfStruct)) {
            do {

                if (IsmCheckCancel()) {
                    __leave;
                }

                pattern = InfGetStringField (InfStruct, 0);

                if (!pattern) {
                    continue;
                }
                encodedPattern = TurnRegStringIntoHandle (pattern, TRUE, NULL);

                functionName = InfGetStringField (InfStruct, 1);

                if (functionName) {
                    AddSpecialRenameRule(encodedPattern, functionName);
                } else {
                    LOG ((LOG_ERROR, (PCSTR) MSG_DATA_RENAME_NO_FN, pattern));
                }

                IsmDestroyObjectHandle (encodedPattern);
                encodedPattern = NULL;
            } while (InfFindNextLine (InfStruct));
        }

        result = TRUE;
    }
    __finally {
        InfCleanUpInfStruct (InfStruct);
    }

    return result;
}

BOOL
DoRegistrySpecialRename (
    IN      HINF InfHandle,
    IN      PCTSTR Section
    )
{
    PCTSTR osSpecificSection;
    BOOL b;
    INFSTRUCT is = INITINFSTRUCT_PMHANDLE;

    b = pProcessDataRenameSection (&is, InfHandle, Section);

    if (b) {
        osSpecificSection = GetMostSpecificSection (PLATFORM_SOURCE, &is, InfHandle, Section);

        if (osSpecificSection) {
            b = pProcessDataRenameSection (&is, InfHandle, osSpecificSection);
            FreeText (osSpecificSection);
        }
    }

    InfCleanUpInfStruct (&is);
    return b;
}


//
// Helpers below
//

VOID
pConvertOE4Init (
    IN      MIG_PLATFORMTYPEID Platform
    )
{
    if (Platform == PLATFORM_DESTINATION &&
        IsmGetRealPlatform() == PLATFORM_DESTINATION &&
        IsmIsComponentSelected (S_OE_COMPONENT, 0) &&
        IsmIsEnvironmentFlagSet (PLATFORM_SOURCE, NULL, S_OE4_APPDETECT))
    {

        if (g_DestIdentityGUID != NULL) {
            // Already got it.. punt
            return;
        }

        // pull out the GUID from dest
        g_DestIdentityGUID = OEGetDefaultId (PLATFORM_DESTINATION);

        if (g_DestIdentityGUID == NULL)
        {
            // This is when we created a new user
            g_DestIdentityGUID = OECreateFirstIdentity();
        } else {
            // This is when applying to a user who never ran OE
            OEInitializeIdentity();
        }
    }
}

BOOL
WINAPI
pConvertOE4Move (
    IN      PCMIG_FILTERINPUT InputData,
    OUT     PMIG_FILTEROUTPUT OutputData,
    IN      BOOL NoRestoreObject,
    IN      PCMIG_BLOB SourceOperationData,             OPTIONAL
    IN      PCMIG_BLOB DestinationOperationData         OPTIONAL
    )
{
    PCTSTR srcNode = NULL;
    PCTSTR srcLeaf = NULL;
    PTSTR newNode = NULL;
    PTSTR ptr = NULL;
    MIG_OBJECTSTRINGHANDLE newName;

    if (g_DestIdentityGUID == NULL) {
        return FALSE;
    }

    IsmCreateObjectStringsFromHandle (InputData->CurrentObject.ObjectName, &srcNode, &srcLeaf);

    // srcNode should be "HKCU\Software\Microsoft\Outlook Express\..."

    if (!srcNode) {
        return FALSE;
    }

    ptr = (PTSTR)_tcsistr (srcNode, TEXT("\\Outlook Express"));
    if (!ptr) {
        return FALSE;
    }
    ptr += 16;

    newNode = JoinPathsInPoolEx ((
                    NULL,
                    TEXT("HKCU\\Identities"),
                    g_DestIdentityGUID,
                    TEXT("Software\\Microsoft\\Outlook Express\\5.0"),
                    ptr,
                    NULL
                    ));
    // newNode should be "HKCU\Identities\{GUID}\Software\Microsoft\Outlook Express\5.0\..."

    newName = IsmCreateObjectHandle (newNode, srcLeaf);
    FreePathString (newNode);
    IsmDestroyObjectString (srcLeaf);
    IsmDestroyObjectString (srcNode);

    OutputData->NewObject.ObjectName = newName;

    return TRUE;
}

VOID
pConvertOE5IAMInit (
    IN      MIG_PLATFORMTYPEID Platform
    )
{
    PTSTR srcAssocId;
    PTSTR newIdentity;

    if (Platform == PLATFORM_DESTINATION &&
        IsmGetRealPlatform() == PLATFORM_DESTINATION &&
        IsmIsComponentSelected (S_OE_COMPONENT, 0) &&
        IsmIsEnvironmentFlagSet (PLATFORM_SOURCE, NULL, S_OE5_APPDETECT))
    {
        // g_DestIdentityGUID should remain NULL if we do not want to remap the IAM tree
        // This is true when one of the following is true:
        // 1. Destination user profile has not been created yet
        // 2. When the IAM has not yet been initialized on the destination (assume that
        //    [AssociatedID] has not yet been written.. if this is not a valid assumption,
        //    compare to source's AssociatedID)
        // 3. The source's associated ID is being merged into the destination's associated ID

        if (g_DestIdentityGUID != NULL) {
            // Already got it.. punt
            return;
        }

        srcAssocId = OEGetAssociatedId (PLATFORM_SOURCE);
        if (srcAssocId) {
            newIdentity = OEGetRemappedId (srcAssocId);
            if (newIdentity) {
               // NOTE: OEIsIdentityAssociated checks to see if it's associated. If
               // the key does not exist, it automatically claims it and returns TRUE
               if (OEIsIdentityAssociated(newIdentity)) {
                   FreeText(newIdentity);
               } else {
                   g_DestIdentityGUID = newIdentity;
               }
            }
            FreeText(srcAssocId);
        }

        OEInitializeIdentity();
    }
}


BOOL
pConcatRuleIndex (
    IN      PCTSTR Node,
    IN      PCTSTR SearchStr
    )
{
    MIG_OBJECTSTRINGHANDLE objectName;
    MIG_CONTENT objectContent;
    PTSTR tmpNode;
    PTSTR ptr;
    TCHAR number[5];
    PTSTR newStr;

    // Node looks like "HKR\Identities\{GUID}\Software\Microsoft\Outlook Express\5.0\Rules\News\008\"
    // SearchStr looks like "\News\"

    tmpNode = DuplicateText(Node);
    if (tmpNode) {
        ptr = (PTSTR)_tcsistr (tmpNode, SearchStr);
        if (ptr) {
            ptr += TcharCount(SearchStr);
            StringCopyTcharCount(number, ptr, 4);
            number[4] = 0;
            *ptr = 0;

            // number should now look like "008"

            objectName = IsmCreateObjectHandle (tmpNode, TEXT("Order"));
            if (IsmAcquireObject(g_RegType | PLATFORM_DESTINATION,
                                 objectName,
                                 &objectContent)) {
                if (IsValidRegSz(&objectContent)) {

                    // Does this index already exist?
                    if (!_tcsistr ((PCTSTR)objectContent.MemoryContent.ContentBytes, number)) {

                        // Tack this onto the end of the data, separated by a space
                        newStr = IsmGetMemory (objectContent.MemoryContent.ContentSize + sizeof (TCHAR) + ByteCount (number));
                        StringCopy(newStr, (PCTSTR)objectContent.MemoryContent.ContentBytes);
                        StringCat(newStr, TEXT(" "));
                        StringCat(newStr, number);

                        IsmReleaseMemory(objectContent.MemoryContent.ContentBytes);
                        objectContent.MemoryContent.ContentSize = SizeOfString(newStr);
                        objectContent.MemoryContent.ContentBytes = (PCBYTE)newStr;

                        IsmReplacePhysicalObject (g_RegType, objectName, &objectContent);
                    }
                }
                IsmReleaseObject(&objectContent);
            }
            IsmDestroyObjectHandle(objectName);
        }
        FreeText(tmpNode);
    }
    return TRUE;
}

BOOL
pRenameEx
(
    IN      PCTSTR OldNode,
    IN      PCTSTR NewNode,
    IN      PCTSTR Leaf,
    IN      BOOL NoRestoreObject,
    IN      PCTSTR PrevKey,
    IN      PCTSTR FormatStr,
    OUT     PMIG_FILTEROUTPUT OutputData,
    IN      BOOL ZeroBase
    )
{
    BOOL result = FALSE;
    PTSTR patternNode;
    PTSTR tmpNode;
    PTSTR ptr;
    PTSTR searchStr;
    DWORD prevKeyLen;
    DWORD keySize = 1;
    MIG_FILTERINPUT filterInput;
    MIG_FILTEROUTPUT filterOutput;
    MIG_BLOB migBlob;
    MIG_BLOB zeroBaseBlob;
    PTSTR filteredNode = NULL;

    // This function basically manually generates a RenRegEx rule and runs the filter
    // Then for each base key it updates the associated [Order] key to add the new number, if needed

    // Rule keys are something like:
    //  HKR\Identities\{GUID}\Software\Microsoft\Outlook Express\5.0\Rules\Mail\000\*
    //  HKR\Identities\{GUID}\Software\Microsoft\Outlook Express\5.0\Rules\News\008\*
    //  HKR\Identities\{GUID}\Software\Microsoft\Outlook Express\5.0\Block Senders\Mail\Criteria\0AF\*
    // PrevKey is the string preceding the numbered key at the end. i.e. "Mail", "Criteria", etc

    tmpNode = DuplicateText(NewNode);
    if (tmpNode) {
        prevKeyLen = TcharCount(PrevKey);
        searchStr = AllocText(prevKeyLen + 3);
        if (searchStr) {
            _stprintf(searchStr, TEXT("\\%s\\"), PrevKey);

            ptr = (PTSTR)_tcsistr (tmpNode, searchStr);
            if (ptr) {
                ptr += (prevKeyLen + 2); // Advance to next portion
                *ptr = 0;
                ptr = _tcsinc(ptr);
                while (*ptr && *ptr != TEXT('\\')) {
                    ptr = _tcsinc(ptr);
                    keySize++;
                }

                patternNode = AllocText (CharCount (tmpNode) + (CharCount (FormatStr) - keySize));
                if (patternNode) {
                    StringCopy(patternNode, tmpNode);
                    StringCat(patternNode, FormatStr);
                    StringCat(patternNode, ptr);

                    filterInput.OriginalObject.ObjectTypeId = g_RegType;
                    filterInput.OriginalObject.ObjectName = IsmCreateObjectHandle (OldNode, NULL);
                    filterInput.CurrentObject.ObjectTypeId = g_RegType;
                    filterInput.CurrentObject.ObjectName = IsmCreateObjectHandle (OldNode, NULL);
                    migBlob.Type = BLOBTYPE_STRING;
                    migBlob.String = IsmCreateObjectHandle (patternNode, NULL);

                    if (ZeroBase) {
                        zeroBaseBlob.Type = BLOBTYPE_BINARY;
                        zeroBaseBlob.BinarySize = sizeof(PCBYTE);
                        zeroBaseBlob.BinaryData = (PCBYTE)TRUE;

                        FilterRenameExFilter (&filterInput, &filterOutput, NoRestoreObject, &zeroBaseBlob, &migBlob);
                    } else {
                        FilterRenameExFilter (&filterInput, &filterOutput, NoRestoreObject, NULL, &migBlob);
                    }

                    IsmDestroyObjectHandle (migBlob.String);
                    IsmDestroyObjectHandle (filterInput.CurrentObject.ObjectName);
                    IsmDestroyObjectHandle (filterInput.OriginalObject.ObjectName);

                    IsmCreateObjectStringsFromHandle (filterOutput.NewObject.ObjectName, &filteredNode, NULL);
                    IsmDestroyObjectHandle (filterOutput.NewObject.ObjectName);

                    OutputData->NewObject.ObjectName = IsmCreateObjectHandle (filteredNode, Leaf);

                    // If this is the root numeric key, then update the index
                    if (0 == *ptr) {
                        pConcatRuleIndex(filteredNode, searchStr);
                    }
                    FreeText (filteredNode);

                    FreeText(patternNode);
                }
            } else {
                OutputData->NewObject.ObjectName = IsmCreateObjectHandle (tmpNode, Leaf);
            }
            FreeText(searchStr);
        } else {
            OutputData->NewObject.ObjectName = IsmCreateObjectHandle (tmpNode, Leaf);
        }
        FreeText(tmpNode);
    }
    return TRUE;
}

BOOL
pRenameNewsRule
(
    IN      PCTSTR OldNode,
    IN      PCTSTR NewNode,
    IN      PCTSTR Leaf,
    IN      BOOL NoRestoreObject,
    OUT     PMIG_FILTEROUTPUT OutputData
    )
{
    return pRenameEx(OldNode, NewNode, Leaf, NoRestoreObject, TEXT("News"), TEXT("<%03x>"), OutputData, TRUE);
}

BOOL
pRenameMailRule
(
    IN      PCTSTR OldNode,
    IN      PCTSTR NewNode,
    IN      PCTSTR Leaf,
    IN      BOOL NoRestoreObject,
    OUT     PMIG_FILTEROUTPUT OutputData
    )
{
    return pRenameEx(OldNode, NewNode, Leaf, NoRestoreObject, TEXT("Mail"), TEXT("<%03x>"), OutputData, TRUE);
}

BOOL
pRenameBlockRule
(
    IN      PCTSTR OldNode,
    IN      PCTSTR NewNode,
    IN      PCTSTR Leaf,
    IN      BOOL NoRestoreObject,
    OUT     PMIG_FILTEROUTPUT OutputData
    )
{
    return pRenameEx(OldNode, NewNode, Leaf, NoRestoreObject, TEXT("Criteria"), TEXT("<%03x>"), OutputData, TRUE);
}



BOOL
pRenameAccount
(
    IN      PCTSTR OldNode,
    IN      PCTSTR NewNode,
    IN      PCTSTR Leaf,
    IN      BOOL NoRestoreObject,
    OUT     PMIG_FILTEROUTPUT OutputData
    )
{
    return pRenameEx(OldNode, NewNode, Leaf, NoRestoreObject, TEXT("Accounts"), TEXT("<%08d>"), OutputData, FALSE);
}

BOOL
WINAPI
pConvertOE5RulesMove (
    IN      PCMIG_FILTERINPUT InputData,
    OUT     PMIG_FILTEROUTPUT OutputData,
    IN      BOOL NoRestoreObject,
    IN      LPRENAMERULE fnRename
    )
{
    PCTSTR srcNode = NULL;
    PCTSTR srcLeaf = NULL;
    PTSTR newNode = NULL;
    PTSTR tmpText;
    TCHAR *endId;
    TCHAR *srcIdentity;
    PTSTR newIdentity;

    // Move tree and Merge account name
    // This function just changes the {GUID} to the new value, then calls fnRename to update the
    // numeric portion of the keyname

    IsmCreateObjectStringsFromHandle (InputData->CurrentObject.ObjectName, &srcNode, &srcLeaf);
    if (srcNode) {
        // srcNode should be "HKCU\Identities\{GUID}\Software\Microsoft\Outlook Express\Rules\Mail\..."
        tmpText = DuplicateText(srcNode);
        if (tmpText) {
            srcIdentity = _tcschr(tmpText, TEXT('{'));
            if (srcIdentity) {
                endId = _tcschr(srcIdentity, TEXT('\\'));
                if (endId) {
                    *endId = 0;
                    endId = _tcsinc(endId);

                    // endId should be "Software\Microsoft\Outlook Express\Rules\Mail\..."
                    // srcIdentity should be "{GUID}"

                    newIdentity = OEGetRemappedId (srcIdentity);
                    if (newIdentity) {
                        newNode = JoinPathsInPoolEx ((
                                        NULL,
                                        TEXT("HKCU\\Identities"),
                                        newIdentity,
                                        endId,
                                        NULL
                                        ));

                        if (newNode) {
                            if (srcLeaf &&
                                !g_OERulesMigrated &&
                                !StringIMatch(srcLeaf, TEXT("Version"))) {
                                g_OERulesMigrated = TRUE;
                            }
                            fnRename(srcNode, newNode, srcLeaf, NoRestoreObject, OutputData);
                            FreePathString (newNode);
                        }
                        FreeText(newIdentity);
                    }
                }
            }
            FreeText(tmpText);
        }
        IsmDestroyObjectString (srcNode);
    }
    IsmDestroyObjectString (srcLeaf);

    return TRUE;
}

BOOL
WINAPI
pConvertOE5NewsRulesMove (
    IN      PCMIG_FILTERINPUT InputData,
    OUT     PMIG_FILTEROUTPUT OutputData,
    IN      BOOL NoRestoreObject,
    IN      PCMIG_BLOB SourceOperationData,             OPTIONAL
    IN      PCMIG_BLOB DestinationOperationData         OPTIONAL
    )
{
    return pConvertOE5RulesMove(InputData, OutputData, NoRestoreObject, pRenameNewsRule);
}

BOOL
WINAPI
pConvertOE5MailRulesMove (
    IN      PCMIG_FILTERINPUT InputData,
    OUT     PMIG_FILTEROUTPUT OutputData,
    IN      BOOL NoRestoreObject,
    IN      PCMIG_BLOB SourceOperationData,             OPTIONAL
    IN      PCMIG_BLOB DestinationOperationData         OPTIONAL
    )
{
    return pConvertOE5RulesMove(InputData, OutputData, NoRestoreObject, pRenameMailRule);
}

BOOL
WINAPI
pConvertOE5BlockMove (
    IN      PCMIG_FILTERINPUT InputData,
    OUT     PMIG_FILTEROUTPUT OutputData,
    IN      BOOL NoRestoreObject,
    IN      PCMIG_BLOB SourceOperationData,             OPTIONAL
    IN      PCMIG_BLOB DestinationOperationData         OPTIONAL
    )
{
    return pConvertOE5RulesMove(InputData, OutputData, NoRestoreObject, pRenameBlockRule);
}

BOOL
WINAPI
pConvertOEIdIAMMove (
    IN      PCMIG_FILTERINPUT InputData,
    OUT     PMIG_FILTEROUTPUT OutputData,
    IN      BOOL NoRestoreObject,
    IN      PCMIG_BLOB SourceOperationData,             OPTIONAL
    IN      PCMIG_BLOB DestinationOperationData         OPTIONAL
    )
{
    PCTSTR srcNode = NULL;
    PCTSTR srcLeaf = NULL;
    PTSTR newNode = NULL;
    PTSTR tmpText;
    TCHAR *endId;
    TCHAR *srcIdentity;
    PTSTR newIdentity;

    // Move tree and Merge account name

    // This moves the tree in one of the following ways:
    // 1. From Identities into HKCU\SoftwareMicrosoft\Internet Account Manager\
    // 2. From one Identity into another Identity (for merging case)
    // 3. From one Identity into the same Identity (nop)
    // In all cases, then call pRenameAccount to update the name (name is actually a DWORD index value)

    IsmCreateObjectStringsFromHandle (InputData->CurrentObject.ObjectName, &srcNode, &srcLeaf);
    if (srcNode) {
        // srcNode should be "HKCU\Identities\{GUID}\Software\Microsoft\Internet Account Manager\..."
        tmpText = DuplicateText(srcNode);
        if (tmpText) {
            srcIdentity = _tcschr(tmpText, TEXT('{'));
            if (srcIdentity) {
                endId = _tcschr(srcIdentity, TEXT('\\'));
                if (endId) {
                    *endId = 0;
                    endId = _tcsinc(endId);

                    // endId should be "Software\Microsoft\Internet Account Manager\..."
                    // srcIdentity should be "{GUID}"

                    newIdentity = OEGetRemappedId (srcIdentity);
                    if (newIdentity) {
                        if (OEIsIdentityAssociated (newIdentity)) {
                            newNode = JoinPaths (TEXT("HKCU"), endId);
                            // newNode should be "HKCU\Software\Microsoft\Internet Account Manager\..."
                        } else {
                            newNode = JoinPathsInPoolEx ((
                                            NULL,
                                            TEXT("HKCU\\Identities"),
                                            newIdentity,
                                            endId,
                                            NULL
                                            ));
                        }

                        if (newNode) {
                            pRenameAccount(srcNode,
                                           newNode,
                                           srcLeaf,
                                           NoRestoreObject,
                                           OutputData);
                            FreePathString (newNode);
                        }
                        FreeText(newIdentity);
                    }
                }
            }
            FreeText(tmpText);
        }
        IsmDestroyObjectString (srcNode);
    }
    IsmDestroyObjectString (srcLeaf);

    return TRUE;
}

BOOL
WINAPI
pConvertOEIAMMove (
    IN      PCMIG_FILTERINPUT InputData,
    OUT     PMIG_FILTEROUTPUT OutputData,
    IN      BOOL NoRestoreObject,
    IN      PCMIG_BLOB SourceOperationData,             OPTIONAL
    IN      PCMIG_BLOB DestinationOperationData         OPTIONAL
    )
{
    PCTSTR srcNode = NULL;
    PCTSTR srcLeaf = NULL;
    PTSTR newNode = NULL;
    PTSTR filteredNode = NULL;
    PTSTR ptr = NULL;

    // Move tree and Merge account name

    // This moves the tree in one of the following ways:
    // 1. From HKCU\SoftwareMicrosoft\Internet Account Manager\ into an Identity
    // 2. From HKCU\SoftwareMicrosoft\Internet Account Manager\ into the same location (nop)
    // In all cases, then call pRenameAccount to update the name (name is actually a DWORD index value)

    // This might move the accounts from HKCU\Software\Microsoft\Internet Account Manager into an identity

    IsmCreateObjectStringsFromHandle (InputData->CurrentObject.ObjectName, &srcNode, &srcLeaf);
    if (srcNode) {
        // srcNode should be "HKCU\Software\Microsoft\Internet Account Manager\..."
        if (g_DestIdentityGUID != NULL &&
            !OEIsIdentityAssociated (g_DestIdentityGUID)) {

            ptr = _tcschr (srcNode, TEXT('\\'));
            if (ptr) {
                newNode = AllocText (TcharCount (srcNode) + TcharCount (g_DestIdentityGUID) + 13);
                StringCopy (newNode, TEXT("HKCU\\Identities\\"));    // +12
                StringCat (newNode, g_DestIdentityGUID);
                StringCat (newNode, ptr);

                // newNode should be "HKCU\Identities\{GUID}\Software\Microsoft\Internet Account Manager\..."
            }
        } else {
            newNode = DuplicateText (srcNode);
        }

        if (newNode) {
            pRenameAccount(srcNode,
                           newNode,
                           srcLeaf,
                           NoRestoreObject,
                           OutputData);
            FreeText (newNode);
        }
        IsmDestroyObjectString (srcNode);
    }
    IsmDestroyObjectString (srcLeaf);

    return TRUE;
}
