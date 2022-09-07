#if !defined( _SECTOR_IO_ )

#define _SECTOR_IO_

#include "io.hxx"
#include "hmem.hxx"
#include "secrun.hxx"

DECLARE_CLASS( SECTOR_IO );

class SECTOR_IO : public IO_OBJECT {

    public:

        NONVIRTUAL
        SECTOR_IO(
            ) { _drive = NULL; };

        VIRTUAL
        BOOLEAN
        Setup(
            IN  PMEM                Mem,
            IN  PLOG_IO_DP_DRIVE    Drive,
            IN  HANDLE              Application,
            IN  HWND                WindowHandle,
            OUT PBOOLEAN            Error
            );

        VIRTUAL
        BOOLEAN
        Read(
            OUT PULONG              pError
            );

        VIRTUAL
        BOOLEAN
        Write(
            );

        VIRTUAL
        PVOID
        GetBuf(
            OUT PULONG  Size    DEFAULT NULL
            );

        VIRTUAL
        PTCHAR
        GetHeaderText(
            );

    private:

        PLOG_IO_DP_DRIVE    _drive;
        SECRUN              _secrun;
        TCHAR               _header_text[64];

};


#endif
