
#ifndef MEM_MAP_H
#define MEM_MAP_H


#define OLD_FLASH_MAP       0

#define FLASH_XIP_ADDR                  0x00800000



#define APP_FLASH_LOAD_ADDR             (FLASH_XIP_ADDR+0x20000)
#define APP_FLASH_LOAD_SIZE             0x280000

#define BOOTLOADER_FLASH_LOAD_ADDR      (FLASH_XIP_ADDR+0x4000)
#define BOOTLOADER_FLASH_LOAD_SIZE      0x00012000

#if 1 //def QUECTEL_PLATFORM_SUPPORT  //louis@20200703: for hib raw flash write balance
#define FLASH_MEM_BACKUP_ADDR           (FLASH_XIP_ADDR+0x2CC000)
#define FLASH_MEM_BACKUP_SIZE           0x2000
#else
#define FLASH_MEM_BACKUP_ADDR           (FLASH_XIP_ADDR+0x34C000)
#define FLASH_MEM_BACKUP_SIZE           0x2000
#endif

#define FLASH_MEM_PLAT_INFO_ADDR        (FLASH_XIP_ADDR+0x3BB000)
#define FLASH_MEM_PLAT_INFO_SIZE        0x1000
#define FLASH_MEM_PLAT_INFO_NONXIP_ADDR (FLASH_MEM_PLAT_INFO_ADDR - FLASH_XIP_ADDR)

///////////////NV RAM AREA///////////////////////
#define NVRAM_FACTORY_PHYSICAL_BASE     0x16000
#define NVRAM_PHYSICAL_BASE             0x3A4000

#if 1 //def QUECTEL_PLATFORM_SUPPORT  //louis@20200519: adjust the partition of flash
#define FLASH_APP_REGION_START          0x2A0000
#define FLASH_APP_REGION_LEN            0x30000
#define FLASH_APP_REGION_END            0x2D0000

#define FLASH_FOTA_REGION_START         0x2D0000
#define FLASH_FOTA_REGION_LEN           0x80000
#define FLASH_FOTA_REGION_END           0x350000
#else
#define FLASH_FOTA_REGION_START         0x2A0000
#define FLASH_FOTA_REGION_LEN           0x80000
#define FLASH_FOTA_REGION_END           0x320000
#endif

/////////////////////////////////////////////////

////////////////FS AREA//////////////////////////
#define FLASH_FS_REGION_OFFSET          0x350000
#define FLASH_FS_REGION_END             0x3A4000
#define FLASH_FS_REGION_SIZE            (FLASH_FS_REGION_END-FLASH_FS_REGION_OFFSET)      // 336KB
/////////////////////////////////////////////////


////////////////EC EXCEPTION AREA////////////////
#define EC_EXCEPTION_FLASH_BASE         0x3BC000
#define EC_ASSERT_BLOCK_NUMBS           (68)        // (16KB+256KB) / 4KB

#define EC_EXCEPTION_RAM_BASE           (0x000000)

#define EC_RAM_START_ADDR               EC_EXCEPTION_RAM_BASE
#define EC_RAM_TOTAL_SIZE               (0x44000)                       // (16KB+256KB)
#define EC_RAM_END_ADDR                 (EC_RAM_TOTAL_SIZE+EC_RAM_START_ADDR)

#define EC_RESET_REASON_ADDR			(EC_RAM_END_ADDR-0x24)		   // 0x43FDC
#define EC_ASSERT_PC_ADDR               (EC_RAM_END_ADDR-0x20)         // 0x43FE0
#define EC_ASSERT_LR_ADDR               (EC_RAM_END_ADDR-0x1C)         // 0x43FE4
#define EC_EXCEPTION_MAGIC_ADDR         (EC_RAM_END_ADDR-0x18)         // 0x43FE8
#define EC_EXCEP_STORE_RAM_ADDR         (EC_RAM_END_ADDR-0x10)         // 0x43FF0

/////////////////////////////////////////////////
#if 1 //def QUECTEL_PLATFORM_SUPPORT
#define HEAP_MEM_MAX_START_ADDR             0x36000
#else
#define HEAP_MEM_MAX_START_ADDR             0x33000 // heap should at least occupy 0x33000--0x41000
#endif 

#define HEAP_MEM_MAX_END_ADDR               0x40800
#endif
