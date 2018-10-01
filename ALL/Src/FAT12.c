/********************* (C) COPYRIGHT 2017 e-Design Co.,Ltd. ********************
 File Name : FAT12.c  
 Version   : DS212                                                Author : bure
*******************************************************************************/
#include <string.h>
#include "FAT12.h"
#include "BIOS.h"
#include "Func.h"
#include "Flash.h"


typedef struct
{
  u32 FAT1_BASE;          // FAT1 area start address
  u32 FAT2_BASE;          // FAT2 area start address
  u32 ROOT_BASE;          // Root directory start address
  u32 FILE_BASE;          // File area start address
  u32 FAT_LEN;
  u32 SEC_LEN;            // Sector length
  u32 FAT_END;            // End of link
  u8  FAT1_SEC;           // Number of FAT1 sectors
  u8  FAT2_SEC;
}FAT_InitTypeDef;

u8 Clash;
uc8 DiskDevInfo_8M[]={"8MB Internal"};
uc8 McuType[]={"STM32F103VC"};

FAT_InitTypeDef FAT_V;

int Init_Fat_Value(void)
{
  FAT_V.FAT1_BASE=FAT1_BASE_8M; 
  FAT_V.FAT2_BASE=FAT2_BASE_8M; 
  FAT_V.ROOT_BASE=ROOT_BASE_8M;  
  FAT_V.FILE_BASE=FILE_BASE_8M;      
  FAT_V.FAT1_SEC=FAT1_SEC_8M;
  FAT_V.FAT2_SEC=FAT1_SEC_8M;
  FAT_V.SEC_LEN = SEC_LEN_8M;    
  FAT_V.FAT_LEN = FAT_LEN_8M; 
  FAT_V.FAT_END = FAT_END_8M;
  
  return 0;
}
/*******************************************************************************
 Read disk page (256 Bytes) - Contains USB read and write conflicts after reread.
*******************************************************************************/
u8 ReadDiskData(u8* pBuffer, u32 ReadAddr, u16 Lenght)
{
  u8 n = 0;
  
  while(1){
    Clash = 0;
    ExtFlashDataRd(pBuffer, ReadAddr, Lenght);
    if(n++ > 6) return SEC_ERR;     // Timeout error return.
    if(Clash == 0) return OK;       // Return without collision.
  }
}
/*******************************************************************************
 Write disk page (256 Bytes) - Contains USB read and write conflicts rewritten.
*******************************************************************************/
u8 ProgDiskPage(u8* pBuffer, u32 ProgAddr)
{                         
  u8   n = 0; 
  
  while(1){
    Clash = 0;
    ExtFlashSecWr(pBuffer, ProgAddr);
    if(n++ > 6) return SEC_ERR;     // Timeout error return.
    if(Clash == 0) return OK;       // Return without collision.
  }
} 
/*******************************************************************************
 Finds the next linked cluster number and returns. The current cluster number is stored in the pointer +1 position.
*******************************************************************************/
u8 NextCluster(u16* pCluster)
{
  u16 FatNum;
  u32 Addr ;
  
  Addr=FAT_V.FAT1_BASE +(*pCluster + *pCluster/2);
  
  *(pCluster+1)= *pCluster;                                   // Save the previous cluster number.
  *pCluster = 0;
  if((*(pCluster+1) >=FAT_V.FAT_END)||(*(pCluster+1)< 2)) return SEC_ERR;
  if(ReadDiskData((u8*)&FatNum, Addr, 2)!= OK) return SEC_ERR;
  *pCluster= (*(pCluster+1) & 1)?(FatNum >>4):(FatNum & 0xFFF);//Point to the next cluster number.
  return OK; 
}
/*******************************************************************************
 Read file sector (512 Bytes), return pointer to the next cluster number. The current cluster number is stored in the pointer +1 position.
*******************************************************************************/
u8 ReadFileSec(u8* pBuffer, u16* pCluster)
{
  u32 ReadAddr =FAT_V.FILE_BASE + FAT_V.SEC_LEN*(*pCluster-2);

  if(ReadDiskData(pBuffer, ReadAddr, FAT_V.SEC_LEN)!=OK) return SEC_ERR; 
  if(NextCluster(pCluster)!=0) return FAT_ERR;                 // Get the next cluster number.
  return OK;
} 
/*******************************************************************************
 Write file sector (512/4096 Bytes), fill in the current FAT table and return the next cluster number found.
*******************************************************************************/
u8 ProgFileSec(u8* pBuffer, u16* pCluster)
{
  u16 Tmp;
  u32 ProgAddr = FAT_V.FILE_BASE + FAT_V.SEC_LEN*(*pCluster-2);
  if(ProgDiskPage(pBuffer, ProgAddr)!= OK) return SEC_ERR; 
  
  if(NextCluster(pCluster)!=0) return FAT_ERR;                 // Get the next cluster number.
  Tmp = *(pCluster+1);
  if(*pCluster == 0){
    *pCluster = Tmp;
    if(SeekBlank (pBuffer, pCluster )!= OK) return OVER;
    if(SetCluster(pBuffer, pCluster )!= OK) return SEC_ERR;
  }
  return OK;
}
/*******************************************************************************
 Find the free cluster number. When it returns, the pointer points to the next free cluster number. The current cluster number is saved in the position of pointer +1.
*******************************************************************************/
u8 SeekBlank(u8* pBuffer, u16* pCluster)
{
  u16  Tmp;
  u8   Buffer[2];
  u8   Tmp_Flag = 1;

  *(pCluster+1)= *pCluster;                                    // Save current cluster number.

  for(*pCluster=0; (*pCluster)<4095; (*pCluster)++){
    if(ReadDiskData(Buffer, FAT_V.FAT1_BASE +(*pCluster)+(*pCluster)/2, 2)!= 0)
      return SEC_ERR;
    Tmp = ((*pCluster)& 1)?((*(u16*)Buffer)>>4):((*(u16*)Buffer)& 0xFFF);
    
    if((Tmp == 0)&&(Tmp_Flag == 0)&&(((*pCluster))!= *(pCluster+1))) {
      Tmp_Flag = 1;
      return OK;
    }
    if((Tmp == 0)&&(Tmp_Flag == 1))  {
      *(pCluster+2) = *pCluster;
      Tmp_Flag = 0;
    } 
  }
  return OK;
}         
/*******************************************************************************
 Writes the next cluster number to the FAT table's current cluster link bit. When it returns, the pointer points to the next cluster number. Pointer +1 is the current cluster number.
*******************************************************************************/
u8 SetCluster(u8* pBuffer, u16* pCluster)
{
  u16  Offset, i, k;
  u32  SecAddr;

  i = *(pCluster+1);                    // Extract the current cluster number.
  k = *pCluster;                        // Extract the next cluster number.
  Offset = i+ i/2;
  SecAddr = FAT_V.FAT1_BASE +(Offset & 0xF000 );
  Offset &= 0x0FFF;
  if(ReadDiskData(pBuffer, SecAddr, FAT_V.SEC_LEN)!= 0) return SEC_ERR; 
  if(i & 1){
    pBuffer[Offset  ]=(pBuffer[Offset]& 0x0F)+((k <<4)& 0xF0);
    pBuffer[Offset+1]= k >>4;
  } else {
    pBuffer[Offset  ]= k & 0xFF;
    pBuffer[Offset+1]=(pBuffer[Offset+1]& 0xF0)+((k>>8)& 0x0F);
  }
  
  if(ProgDiskPage(pBuffer, SecAddr)!= 0) return SEC_ERR;
  return OK;
}
/*******************************************************************************
 Read Mode Open File: Returns the first cluster number and directory entry address of the file or 0 cluster number and the first blank directory entry address.
*******************************************************************************/
u8 OpenFileRd(u8* pBuffer, u8* pFileName, u16* pCluster, u32* pDirAddr)
{
  u16 i, n;

  *pCluster = 0;
  for(*pDirAddr=FAT_V.ROOT_BASE; *pDirAddr<FAT_V.FILE_BASE; ){
    if(ReadDiskData(pBuffer, *pDirAddr,FAT_V.SEC_LEN)!= OK) return SEC_ERR;
    for(n=0; n<FAT_V.SEC_LEN; n+=32){   
      for(i=0; i<11; i++){
        if(pBuffer[n + i]!= 0){
          if(pBuffer[n + i]!= pFileName[i]) break;
          if(i == 10){                             // Find the file name.
            *pCluster = *(u16*)(pBuffer + n + 0x1A); // The first cluster number of the file.
            return OK;         
          }
        } else return NEW;               // Returns after the first blank directory entry.
      }
      *pDirAddr += 32;
    }
  }
  return OVER;
}
/*******************************************************************************
 Write mode to open the file: return the file first cluster number and directory entry address.
*******************************************************************************/
u8 OpenFileWr(u8* pBuffer, u8* pFileName, u16* pCluster, u32* pDirAddr)
{
  u32 i, n,offset;
  
  i = OpenFileRd(pBuffer, pFileName, pCluster, pDirAddr);
  if(i != NEW) return i;
  else{                                                    // The current item is a blank directory item.
    if(SeekBlank(pBuffer, pCluster)!= OK) return OVER;     // If the FAT table is full, return overflow.
    n =*pDirAddr & 0xFFF;
    offset=*pDirAddr-n;
    if(ReadDiskData(pBuffer,offset, FAT_V.SEC_LEN)!= OK) return SEC_ERR; 
    for(i=0; i<11; i++) pBuffer[n+i]= pFileName[i];      // Create new directory entry offset +
    *(u16*)(pBuffer + n + 0x1A)= *pCluster;
    if(ProgDiskPage(pBuffer,offset)!= OK) return SEC_ERR;
    return OK;
  }
}                
/*******************************************************************************
 Close the file, write the terminator to the FAT table, write the file length to the directory entry, copy FAT1 to FAT2.
*******************************************************************************/
u8 CloseFile(u8* pBuffer, u32 Lenght, u16* pCluster, u32* pDirAddr)
{
  u32 n;
  n=0xFFF;
  *pCluster = 0xFFF;
  SetCluster(pBuffer, pCluster); 

  if(ReadDiskData(pBuffer, (*pDirAddr &(~n)), FAT_V.SEC_LEN)!= OK) return SEC_ERR;
  *(u8* )(pBuffer +(*pDirAddr & n)+ 0x0B)= 0x20;
  *(u32*)(pBuffer +(*pDirAddr & n)+ 0x1C)= Lenght;
  if(ProgDiskPage(pBuffer, (*pDirAddr &(~n)))!= OK) return SEC_ERR;
  if(ReadDiskData(pBuffer, FAT_V.FAT1_BASE, FAT_V.SEC_LEN)!= OK) return SEC_ERR;
  if(ProgDiskPage(pBuffer, FAT_V.FAT2_BASE     )!= OK) return SEC_ERR; 
  
  return OK;
}
