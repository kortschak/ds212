/********************* (C) COPYRIGHT 2017 e-Design Co.,Ltd. ********************
 File Name : FAT12.h  
 Version   : DS212                                                Author : bure
*******************************************************************************/
#ifndef __FAT12_H
#define __FAT12_H
#include "STM32F30x.h"

//-------FLASH-----W25Q64BV--------------------------------------------------//
#define FILE_BASE_8M    0x7000     /* File area start address */
#define ROOT_BASE_8M    0x3000     /* Root directory start address */
#define FAT_LEN_8M      0x1000 
#define FAT1_BASE_8M    0x1000     /* FAT1 area start address */
#define FAT2_BASE_8M    0x2000     /* FAT2 area start address */ 
#define SEC_LEN_8M      0x1000     /* Sector length */
#define FAT1_SEC_8M     0x1        /* FAT1 sectors */
#define FAT2_SEC_8M     0x1        /* FAT2 sectors */
#define FAT_END_8M      0x7FF      /* End of link */

#define OK           0             /* Operation completed */
#define SEC_ERR      1             /* Sector read/write errors */
#define FAT_ERR      2             /* FAT table read/write errors */
#define OVER         3             /* Operation overflow */
#define NEW          4             /* Blank/new directory item */
#define SUM_ERR      6             /* Checksum error */

#define VER_ERR      1             /* Wrong version */
#define NO_FILE      2             /* File does not exist */
#define FILE_RW_ERR  3             /* Sector read error */
#define DISK_RW_ERR  4             /* Disk error */

#define OW           0             /* Or write (data changed from 0 to 1) */
#define RW           1             /* Rewrite */

extern u8 Clash;

u8   ReadFileSec(u8* Buffer, u16* Cluster);
u8   ReadDiskData(u8* pBuffer, u32 ReadAddr, u16 Lenght);
u8   NextCluster(u16* Cluster);
u8   ProgFileSec(u8* Buffer, u16* Cluster);
u8   ProgDiskPage(u8* Buffer, u32 ProgAddr);
u8   SeekBlank(u8* Buffer, u16* Cluster);
u8   SetCluster(u8* Buffer, u16* Cluster);
u8   OpenFileRd(u8* Buffer, u8* FileName, u16* Cluster, u32* pDirAddr);
u8   OpenFileWr(u8* Buffer, u8* FileName, u16* Cluster, u32* pDirAddr);
u8   CloseFile(u8* Buffer, u32 Lenght, u16* Cluster, u32* pDirAddr);
void ExtFlash_PageWrite(u8* pBuffer, u32 WriteAddr, u8 Mode);
int  Init_Fat_Value(void);
#endif
/********************************* END OF FILE ********************************/
