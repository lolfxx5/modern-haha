/*++

Copyright (C) 1996-1999 Microsoft Corporation

Module Name:

    IMAGEHELP.H

Abstract:

    Re-director for imagehlp.dll.

History:

--*/

#ifndef ESPUTIL_IMAGEHLP_H
#define ESPUTIL_IMAGEHLP_H




class LTAPIENTRY CImageHelp
{
public:
	CImageHelp();
	BOOL ImagehlpAvailable(void);
			
	BOOL EnumerateLoadedModules(HANDLE, PENUMLOADED_MODULES_CALLBACK, void *);
	PIMAGE_NT_HEADERS ImageNtHeader(LPVOID);

	LPAPI_VERSION ImagehlpApiVersion(void);
	BOOL StackWalk(
			DWORD                             MachineType,
			HANDLE                            hProcess,
			HANDLE                            hThread,
			LPSTACKFRAME                      StackFrame,
			LPVOID                            ContextRecord,
			PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
			PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine,
			PGET_MODULE_BASE_ROUTINE          GetModuleBaseRoutine,
			PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
		);

	BOOL SymGetModuleInfo(
			IN  HANDLE              hProcess,
			IN  DWORD               dwAddr,
			OUT PIMAGEHLP_MODULE    ModuleInfo
		);
	LPVOID SymFunctionTableAccess(
			HANDLE  hProcess,
			DWORD   AddrBase
		);

	BOOL SymGetSymFromAddr(
			IN  HANDLE              hProcess,
			IN  DWORD               dwAddr,
			OUT PDWORD              pdwDisplacement,
			OUT PIMAGEHLP_SYMBOL    Symbol
		);

	BOOL SymInitialize(
			IN HANDLE   hProcess,
			IN LPSTR    UserSearchPath,
			IN BOOL     fInvadeProcess
		);

	BOOL SymUnDName(
			IN  PIMAGEHLP_SYMBOL sym,               // Symbol to undecorate
			OUT LPSTR            UnDecName,         // Buffer to store undecorated name in
			IN  DWORD            UnDecNameLength    // Size of the buffer
		);

	DWORD SymLoadModule(
			IN  HANDLE          hProcess,
			IN  HANDLE          hFile,
			IN  PSTR            ImageName,
			IN  PSTR            ModuleName,
			IN  DWORD           BaseOfDll,
			IN  DWORD           SizeOfDll
		);
	DWORD UnDecorateSymbolName(
			LPSTR    DecoratedName,         // Name to undecorate
			LPSTR    UnDecoratedName,       // If NULL, it will be allocated
			DWORD    UndecoratedLength,     // The maximym length
			DWORD    Flags                  // See IMAGEHLP.H
		);

	DWORD SymGetOptions(void);
	DWORD SymSetOptions(DWORD);
	
	PIMAGE_NT_HEADERS CheckSumMappedFile(
	    LPVOID BaseAddress,
	    DWORD FileLength,
	    LPDWORD HeaderSum,
	    LPDWORD CheckSum
	    );

	BOOL MakeSureDirectoryPathExists(const TCHAR *);
	
private:

	void LoadImageHelp(void);
	
};

#endif
