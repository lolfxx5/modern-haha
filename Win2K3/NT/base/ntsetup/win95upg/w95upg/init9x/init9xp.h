#include "master.h"
#include "master9x.h"

extern READ_DISK_SECTORS_PROC ReadDiskSectors;


#define FAT_BOOT_SECTOR_SIZE 512
#define FAT_BOOT_SECTOR_COUNT 1
#define FAT_STARTING_SECTOR 0
