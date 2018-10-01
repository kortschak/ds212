/******************** (C) COPYRIGHT 2017 e-Design Co.,Ltd. *********************
Project Name: DS212
* File Name:  Files.c                                               Author: xie
*******************************************************************************/
#include <stdlib.h>
#include "Version.h"
#include "string.h"
#include "STM32F30x.h"
#include "stm32f30x_flash.h"
#include "Draw.h"
#include "Func.h"
#include "File.h"
#include "FAT12.h"
#include "Menu.h"
#include "Process.h"
#include "Flash.h"
#include "Func.h"
#include "Drive.h"
#include "Bios.h"
#include "Lcd.h"

#define Page_Address    0x08007800
#define Kpg_Address     0x08007800  /* Save calibrated Kpg[] zero offset and gain coefficient. */

u8   Cal_Flag   = 1;
u8   Rest_Flag  = 0;
u8   Save_Flag  = 0;
u8   About_Flag = 0;
u8   KpgSave_Len = 32;
u8*  NamePtr;
u16  BmpNum[8];
u16  BMP_Ag_Color[16];
u16  Addr = 0, Length = 0;
u8   Num[4];
u64  Svg_Cnt = 0;
u32  Clk_Cnt = 0;
uc8  DAT_Var = 20;

uc8  BmpHead[54]   = {0X42, 0X4D, 0X76, 0X96, 0X00, 0X00, 0X00, 0X00,
                      0X00, 0X00, 0X76, 0X00, 0X00, 0X00, 0X28, 0X00,
                      0X00, 0X00, 0X40, 0X01, 0X00, 0X00, 0XF0, 0X00,
                      0X00, 0X00, 0X01, 0X00, 0X04, 0X00, 0X00, 0X00,
                      0X00, 0X00, 0X82, 0X0B, 0X00, 0X00, 0X12, 0X0b,
                      0X00, 0X00, 0X12, 0X0b, 0X00, 0X00, 0X00, 0X00,
                      0X00, 0X00, 0X00, 0X00, 0X00, 0X00};

uc16 CLK_TAB[44] ={
   0x000,0x000,0x000,0x000,0x070,0x3FF,0x070,0x000,0x000,0x000,0x000,
   0x000,0x002,0x004,0x008,0x070,0x070,0x070,0x080,0x100,0x200,0x000,
   0x020,0x020,0x020,0x020,0x070,0x070,0x070,0x020,0x020,0x020,0x020,
   0x000,0x200,0x100,0x080,0x070,0x070,0x070,0x008,0x004,0x002,0x000};

uc16 BMP_Color[16] = {WHT,  CYN, CYN_,  YEL,  YEL_, PUR, PUR_, GRN,
                      GRN_, GRY, ORN, BLU, RED, BLK, LGN,  DAR};

u8   FileBuff[920];
u8   F_Sector[128];
u16  RsvdSecCnt, SectorSize, FAT_Size, FileLen;
u16  SecPerClus, DirFliePtr, DirSecNum, FAT1_Addr, FAT_Ptr;
u32  DiskStart, Hidden_Sec, Root_Addr, ClusterNum, File_Addr;
u32* FilePtr;

extern u8 DiskBuf[4096];
extern u8  Menu_Temp[5];

/*******************************************************************************
 Color_Num: Find the corresponding color palette number of the current color
*******************************************************************************/
u8 Color_Num(u16 Color)
{
  if(Color == WHT)                    return 0;
  else if((Color & CYN  )== CYN  )    return 1;
  else if((Color & CYN_ )== CYN_ )    return 2;
  else if((Color & YEL  )== YEL  )    return 3;
  else if((Color & YEL_ )== YEL_ )    return 4;
  else if((Color & PUR  )== PUR  )    return 5;
  else if((Color & PUR_ )== PUR_ )    return 6;
  else if((Color & GRN  )== GRN  )    return 7;
  else if((Color & GRN_ )== GRN_ )    return 8;
  else if((Color & GRY  )== GRY  )    return 9;
  else if((Color & ORN  )== ORN  )    return 10;
  else if((Color & BLU  )== BLU  )    return 11;
  else if((Color & RED  )== RED  )    return 12;
  else if((Color & BLK  )== BLK  )    return 13;
  else if((Color & LGN  )== LGN  )    return 14;
  else                                return 15;
}
/*******************************************************************************
 Open the file with the specified extension - Input: File extension - Return value: 0x00 = Success
*******************************************************************************/
void Make_Filename(s16 FileNo,unsigned char* FileName)
{
  u8 Num[4];
  u8ToDec3Str(Num, FileNo);
#if   defined (APP1)
  FileName[4] = Num[0];
#elif defined (APP2)
  FileName[4] = '2';
#endif
  FileName[5] = Num[1];
  FileName[6] = Num[2];
}

/*******************************************************************************
 Save_Dat: Save the raw data of the current display image - Input : file number - Return value 0x00 = success
*******************************************************************************/
u8 Save_Dat(s16 FileNo)
{
  u8  pFileName[12]="FILE    DAT";
  u16 i;
  u16 Rvalue=DISK_RW_ERR ;
  u16 pCluster[3];
  u32 pDirAddr[1];
  Make_Filename(FileNo, pFileName);
  memset(DiskBuf,0,4096);
  if(OpenFileWr(DiskBuf, pFileName, pCluster, pDirAddr)!=OK) return Rvalue;
  DiskBuf[0]  = PopMenu1_Value[CH1_Posi];
  DiskBuf[1]  = PopMenu1_Value[CH2_Posi];
  DiskBuf[2]  = PopMenu1_Value[CH3_Posi];
  DiskBuf[3]  = PopMenu1_Value[TIM_Base];
  DiskBuf[4]  = PopMenu1_Value[CH1_Vol];
  DiskBuf[5]  = PopMenu1_Value[CH1_Coupl];
  DiskBuf[6]  = PopMenu1_Value[CH2_Vol];
  DiskBuf[7]  = PopMenu1_Value[CH2_Coupl];
  DiskBuf[8]  = PopMenu1_Value[CH1_Probe];
  DiskBuf[9]  = PopMenu1_Value[CH2_Probe];
  DiskBuf[10] = PopMenu1_Value[TRI_Mode];

  for(i=0; i<3; i++){
    memcpy(DiskBuf+DAT_Var+300*i, V_Buf+TR1_pBUF+300*i, 300);
  }
  if(ProgFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR; // Write data.
  if(CloseFile(DiskBuf, 512*2, pCluster, pDirAddr)!= OK) return FILE_RW_ERR;
  if(PopMenu3_Value[SAVE_Dat]<99)PopMenu3_Value[SAVE_Dat]++;
  return OK;
}
/*******************************************************************************
 Load_Dat: Load the saved raw image of the saved screen image - Input: File number - Return value: 0x00 = Success
*******************************************************************************/
u8 Load_Dat(s16 FileNo)
{
  u8 pFileName[12]="FILE    DAT";
  u16 pCluster[3];
  u32 pDirAddr[1];
  Make_Filename(FileNo, pFileName);
  if((OpenFileRd(DiskBuf, pFileName, pCluster, pDirAddr))!= OK)
    return NO_FILE;
  if(ReadFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR;
  memcpy(FileBuff, DiskBuf, 920);
  PopMenu1_Value[TIM_Base]   = DiskBuf[3];
  PopMenu1_Value[CH1_Vol]    = DiskBuf[4];
  PopMenu1_Value[CH1_Coupl]  = DiskBuf[5];
  PopMenu1_Value[CH2_Vol]    = DiskBuf[6];
  PopMenu1_Value[CH2_Coupl]  = DiskBuf[7];
  PopMenu1_Value[CH1_Probe]  = DiskBuf[8];
  PopMenu1_Value[CH2_Probe]  = DiskBuf[9];
  PopMenu1_Value[TRI_Mode]   = DiskBuf[10];

  Update_Proc_All();
  Show_Title();
  if(Menu_Temp[0] == Oscillo){
    Show_Item_One(CH1);
    Show_Item_One(CH2);
    Show_Item_One(TIMEBASE);
    Show_Item_One(TRIGGER);
  }
  if(PopMenu3_Value[LOAD_Dat]<99)PopMenu3_Value[LOAD_Dat]++;
  return OK;
}

/*******************************************************************************
 Save_Bmp: Save the current screen display image as BMP format - Input: File number - Return value: 0x00 = success
*******************************************************************************/
u8 Save_Bmp(s16 FileNo)
{
  u8  pFileName[12]="IMG_    BMP";
  u16 pCluster[3];
  u32 pDirAddr[1];
  u32 Rvalue=DISK_RW_ERR;
  s16 x, y, i=54, j,ColorH,ColorL ;
  u16 k=0, l=0;
  u32 length=4096;

  __USB_Port(DISABLE);
  Make_Filename(FileNo, pFileName);
  if(OpenFileWr(DiskBuf, pFileName, pCluster, pDirAddr)!=OK) return Rvalue;
  memcpy(DiskBuf, BmpHead, 54);
  i = 0x0036;                                     // Palette storage start address.
  for(j=0; j<16; ++j){
    DiskBuf[j*4 +i+0]=(BMP_Color[j] & 0xF800)>>8;  // Bule
    DiskBuf[j*4 +i+1]=(BMP_Color[j] & 0x07E0)>>3;  // Green&
    DiskBuf[j*4 +i+2]=(BMP_Color[j] & 0x001F)<<3;  // Red
    DiskBuf[j*4 +i+3]= 0;                          // Alpha
  }
  i = 0x0076;                                     // Image data storage start address.
  for(y=0; y<240; y++){
    for(x=0; x<320 ; x+=2){
      __SetPosi(x, y);
      ColorH = __ReadPixel();
      __SetPosi(x+1, y);
      ColorL = __ReadPixel();
      DiskBuf[i] =(Color_Num(ColorH)<<4)+ Color_Num(ColorL);
      i++;
      if(i>=length){
        i=0;
        if(ProgFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR; // Write data.
        if(l==0)PrintClk(168,2,(k++ >>1)& 3);         // Progress indication.
        l++;
        if(l>=2)l=0;
      }
    }
  }
  if(i!=0){
      if(ProgFileSec(DiskBuf, pCluster)!= OK){List_Pop();return FILE_RW_ERR;}
    }
  if(CloseFile(DiskBuf, 76*512, pCluster, pDirAddr)!= OK) return FILE_RW_ERR;
  if(PopMenu3_Value[SAVE_Bmp]<99)PopMenu3_Value[SAVE_Bmp]++;
  __USB_Port(ENABLE);
  return OK;
}

/*******************************************************************************
 Open_Bmp: Display image as BMP on screen - Input: File number - Return value: 0x00=success
*******************************************************************************/
u8 Open_Bmp(u8 FileNum)
{
    u8  Filename[12] = "ABOUT_F BMP";
    u16 width,high;
    u16 i,j, x=10, y=20;
    u16 pCluster[3];
    u32 pDirAddr[1];

    if(OpenFileRd(DiskBuf, Filename, pCluster, pDirAddr)!=OK) {
      About_Pop(" About.file is Error ", 105, 20,16, 23*6 );
      About_Flag = 0;
      return FILE_RW_ERR;
    }
     if(ReadFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR;
     i = 0x0036;                              // Palette storage start address.
     for(j=0; j<16; j++){
       BMP_Ag_Color[j]=((DiskBuf[j*4 +i+0]<<8)&0xF800)
                      |((DiskBuf[j*4 +i+1]<<3)&0x07E0)
                      |((DiskBuf[j*4 +i+2]>>3)&0x001F);
     }
     width = DiskBuf[0x12];
     high =  DiskBuf[0x16];
     if((width == 250) &&(high == 200) ){
       Clear_Label_L(DAR);                   // Clear left column channel small icon.
       Draw_RECT(CYN, 7, 20, 203, 254, 2 );  // Draw an empty rectangle.
       i = 0x0076;                           // Image data storage start address.
       for(y=22; y<199+22; y++){
         for(x=9; x<256+9 ; x+=2){
           if(x>249+9);
           else{
             __SetPosi(x, y);
             __SetPixel(BMP_Ag_Color[DiskBuf[i]>>4]);
             __SetPosi(x+1, y);
             __SetPixel(BMP_Ag_Color[DiskBuf[i]-(DiskBuf[i]>>4)*16]);
           }
           i++;
           if(i>=4096){
             if(ReadFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR;
             i=0;
           }
         }
       }

     }else {
       About_Pop(" About.Bmp is Error ", 105, 20,16, 22*6 );
       About_Flag = 0;
     }
  return 0;
}

/*******************************************************************************
 Save_Buf: Saves the buffer of the acquisition data to BUF format - Input: File number - Return value: 0x00 = Success
*******************************************************************************/
u8 Save_Buf(s16 FileNo)
{
  u8 pFileName[12]="DATA    BUF";
  u16 pCluster[3];
  u32 pDirAddr[1];
  u32 i=0, Rvalue=DISK_RW_ERR;
  u16 *p;
  u8  l=0;
  u16 n,k;

  Make_Filename(FileNo, pFileName);
  memset(DiskBuf,0,4096);
  if(OpenFileWr(DiskBuf, pFileName, pCluster, pDirAddr)!=OK) return Rvalue;
  n=4; k=4096;


   p   = (u16*)&DiskBuf;
  *p++ = PopMenu1_Value[CH1_Posi];
  *p++ = PopMenu1_Value[CH2_Posi];
  *p++ = PopMenu1_Value[CH3_Posi];
  *p++ = PopMenu1_Value[TIM_Base];
  *p++ = PopMenu1_Value[CH1_Vol];
  *p++ = PopMenu1_Value[CH1_Coupl];
  *p++ = PopMenu1_Value[CH2_Vol];
  *p++ = PopMenu1_Value[CH2_Coupl];
  *p++ = PopMenu1_Value[CH1_Probe];
  *p++ = PopMenu1_Value[CH2_Probe];
  *p++ = PopMenu1_Value[TRI_Sync];
  *p++ = PopMenu1_Value[TRI_Mode];
  *p++ = PopMenu1_Value[TRI_Ch];
  *p++ = PopMenu1_Value[WIN_Depth];

  if(ProgFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR; // Write data.

  memset(DiskBuf,0,k);

  if(PopMenu1_Value[WIN_Depth]==3) n = 8;

  for(i=0; i<n; i++){
    if(PopMenu1_Value[WIN_Depth]==3)
    {
      if(i<4)memcpy(DiskBuf,&(Smpl[i*k/2]),k);
      else   memcpy(DiskBuf,&(Smpl[(i-4)*k/2+4096*2]),k);
    }else{
      if(i<2)memcpy(DiskBuf,&(Smpl[i*k/2]),k);
      else   memcpy(DiskBuf,&(Smpl[i*k/2+4096]),k);
    }
    if(ProgFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR; // Write data.
    PrintClk(264,2,(l++ >>1) & 3);                              // Progress indication.
  }


  if(PopMenu1_Value[WIN_Depth]==3) {
    if(CloseFile(DiskBuf, 33*2*512, pCluster, pDirAddr)!= OK) return FILE_RW_ERR;
  }else{
    if(CloseFile(DiskBuf, 34*512, pCluster, pDirAddr)!= OK) return FILE_RW_ERR;
  }

  if(PopMenu3_Value[SAVE_Buf]<99)PopMenu3_Value[SAVE_Buf]++;
  return OK;
}

/*******************************************************************************
 Load_Buf: Load the saved acquisition data buffer - Input: File number - Return value: 0x00 = success
*******************************************************************************/
u8 Load_Buf(s16 FileNo)
{
  u8 pFileName[12]="DATA    BUF";
  u16 pCluster[3];
  u32 pDirAddr[1];
  u16 *ptr;
  u32 i;
  u32 n,k;

  Make_Filename(FileNo, pFileName);
  if( (OpenFileRd(DiskBuf, pFileName, pCluster, pDirAddr))!= OK)
    return NO_FILE;
  n=4;k=4096;

   if(ReadFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR;
  ptr=(u16*)F_Sector;                               // Save the data.
  *ptr++=0xaa55;
  *ptr++=Smpl[0];
  *ptr++=Smpl[1];
  *ptr++=PopMenu1_Value[CH1_Vol];
  *ptr++=PopMenu1_Value[CH2_Vol];
  *ptr++=PopMenu1_Value[WIN_Depth];
  *ptr++=PopMenu1_Value[TIM_Base];
  ptr =(u16*)DiskBuf;                              // Read original parameters.
  PopMenu1_Value[CH1_Posi]  = *ptr++;
  PopMenu1_Value[CH2_Posi]  = *ptr++;
  PopMenu1_Value[CH3_Posi]  = *ptr++;
  PopMenu1_Value[TIM_Base]  = *ptr++;
  PopMenu1_Value[CH1_Vol]   = *ptr++;
  PopMenu1_Value[CH1_Coupl] = *ptr++;
  PopMenu1_Value[CH2_Vol]   = *ptr++;
  PopMenu1_Value[CH2_Coupl] = *ptr++;
  PopMenu1_Value[CH1_Probe] = *ptr++;
  PopMenu1_Value[CH2_Probe] = *ptr++;
  PopMenu1_Value[TRI_Sync]  = *ptr++;
  PopMenu1_Value[TRI_Mode]  = *ptr++;
  PopMenu1_Value[TRI_Ch]    = *ptr++;
  PopMenu1_Value[WIN_Depth] = *ptr++;

  if(PopMenu1_Value[WIN_Depth]==3) n = 8;
  for(i=0;i<n;i++){
    if(ReadFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR;
    if(PopMenu1_Value[WIN_Depth]==3)
    {
      if(i<4)memcpy(&(Smpl[i*k/2]),DiskBuf,k);
      else   memcpy(&(Smpl[(i-4)*k/2+4096*2]),DiskBuf,k);
    }else{
      if(i<2)memcpy(&(Smpl[i*k/2]),DiskBuf,k);
      else   memcpy(&(Smpl[i*k/2+4096]),DiskBuf,k);
    }
  }



  Status |=  STOP;
  Update_Status();
  Update_Proc_All();
  Show_Title();                                 // Display
  if(Menu_Temp[0] == Oscillo){
    Show_Item_One(CH1);
    Show_Item_One(CH2);
    Show_Item_One(TIMEBASE);
    Show_Item_One(TRIGGER);
  }
  if(PopMenu3_Value[LOAD_Buf]<99)PopMenu3_Value[LOAD_Buf]++;
  return 0;
}
/*******************************************************************************
 make_Vertical
*******************************************************************************/
void make_Vertical(u8 CHn, u8 TRACK, u8* buf, u8* len)
{
  u8 i=0;
  u8* ptr;
  if(CHn == CH1){
   if(PopMenu1_Value[CH1_Probe] == 0)
        ptr = (u8*)&Vol_Str[PopMenu1_Value[TRACK]];
   else ptr = (u8*)&Vol_10X[PopMenu1_Value[TRACK]];
  }
  else if(CHn == CH2){
   if(PopMenu1_Value[CH2_Probe] == 0)
        ptr = (u8*)&Vol_Str[PopMenu1_Value[TRACK]];
   else ptr = (u8*)&Vol_10X[PopMenu1_Value[TRACK]];
  }

  while(*ptr != 0){
    if(*ptr == 0x21)  buf[i] = 0x20;
    else              buf[i] = *ptr;
    ptr++;
    i++;
  };
  buf[i]   = 0x2c;
  buf[i+1] = 0;
  *len = i+1;
}
/*******************************************************************************
 Save_Csv: Saves the capture data buffer in CSV format - Input: File number - Return value: 0x00 = Success
*******************************************************************************/
u8 Save_Csv(s16 FileNo)
{
  s32 Ak = (KgA[KindA+(StateA?1:0)]*4)/GK[GainA];
  s32 Bk = (KgB[KindB+(StateB?1:0)]*4)/GK[GainB]; // 8192~409

  u8 pFileName[12]="DATA    CSV";
  u16 pCluster[3];
  u32 pDirAddr[1];
  u32 i,j,k=0,l=0,Rvalue=DISK_RW_ERR,length;
  u8 Num[4],track[4];
  s16 temp;
  u8 count;

  length=4096;

  Make_Filename(FileNo, pFileName);
  if(OpenFileWr(DiskBuf, pFileName, pCluster, pDirAddr)!=OK) return Rvalue;
  memcpy(DiskBuf,"CH1",3);
  make_Vertical(CH1, CH1_Vol, &DiskBuf[3],&count);
  k=3+count;
  memcpy(&DiskBuf[k],"CH2",3);
  make_Vertical(CH2, CH2_Vol, &DiskBuf[k+3],&count);
  k=k+3+count;
  memset(&DiskBuf[k], 0x20, 5);
  memcpy(&DiskBuf[k], TimeBase_Str[PopMenu1_Value[TIM_Base]], 5);
  k += 5;
  memcpy(&DiskBuf[k], "\r\n", 2);
  k += 2;

  if(PopMenu1_Value[CH1_Probe] == 0)
    memcpy(&DiskBuf[k],"Probe:X1 ",9);
  else
    memcpy(&DiskBuf[k],"Probe:X10",9);
  k += 9;
  DiskBuf[k++] = 0x2c;
  if(PopMenu1_Value[CH2_Probe] == 0)
    memcpy(&DiskBuf[k],"Probe:X1 ",9);
  else
    memcpy(&DiskBuf[k],"Probe:X10",9);
  k += 9;
  DiskBuf[k++] = 0x2c;

  memcpy(&DiskBuf[k], "\r\n", 2);
  k += 2;


  for(i=0; i<DEPTH[PopMenu1_Value[WIN_Depth]]; i++){
    temp = ((((Smpl[i])-2048)*Ak)>>12)+100;          // Calculate the base value of the current waveform point.
    if(temp > 0){
      if(temp > 200)  track[0] = 199;
      else            track[0] = temp;
    } else            track[0] = 0;
    temp = ((((Smpl[i+8192])-2048)*Bk)>>12)+100;     // Calculate the base value of the current waveform point.
    if(temp > 0){
      if(temp > 200)  track[1] = 199;
      else            track[1] = temp;
    } else            track[1] = 0;

   for(j=0; j<2; j++){
    u8ToDec3Str(Num,track[j]);
    for(count=0; count<3; count++){
      if(Num[count] == 0) break;
      DiskBuf[k++] = Num[count];
      if(k >= length){
        if(ProgFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR; // Write data.
        PrintClk(264,2,(l++ >>1) & 3);                              // Progress indication.
        k = 0;
      }
    }
    DiskBuf[k++] = 0x2c;
    if(k >= length){
      if(ProgFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR; // Write data.
      PrintClk(264,2,(l++ >>1)& 3);                               // Progress indication.
      k = 0;
    }
   }
    DiskBuf[k++] = 0x0d;
    if(k >= length){
      if(ProgFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR; // Write data.
      PrintClk(264,2,(l++ >>1)& 3);                               // Progress indication.
      k = 0;
    }
    DiskBuf[k++] = 0x0a;
    if(k >= length){
      if(ProgFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR; // Write data.
      PrintClk(264,2,(l++ >>1)& 3);                               // Progress indication.
      k = 0;
    }
  }
  if(k != 0){
    DiskBuf[k++]=0x0d;
    DiskBuf[k++]=0x0a;
    memset(&DiskBuf[k],0x20,(length-k));
    k=0;
    if(ProgFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR; // Write data.
    PrintClk(264,2,(l++ >>1)& 3);                               // Progress indication.
  }
  if(CloseFile(DiskBuf, l*length, pCluster, pDirAddr)!= OK)
    return DISK_RW_ERR;
  if(PopMenu3_Value[SAVE_Csv]<99)PopMenu3_Value[SAVE_Csv]++;
  return OK;
}

/*******************************************************************************
 Save_Svg: Saves the buffer of the acquisition data in BUF format - Input: File number - Return value: 0x00 = Success
*******************************************************************************/
u8 Save_Svg(s16 FileNo)
{
  u8 pFileName[12]="DATA    SVG";
  u16 pCluster[3];
  u32 pDirAddr[1];
  u16 track[4];
  u32 Rvalue = DISK_RW_ERR;
  u32 l = 0;
  u16 Buf_len = 4096;
  u16 i,x, y;
  s16 temp;
  u8  *str;
  s32 Ak = (KgA[KindA+(StateA?1:0)]*4)/GK[GainA];
  s32 Bk = (KgB[KindB+(StateB?1:0)]*4)/GK[GainB]; // 8192~409

  Svg_Cnt = 0;
  Addr = 0, Length = 0;
  str = (u8*)&FileBuff;
  __USB_Port(DISABLE);
  memset(FileBuff,0,910);
  memset(DiskBuf,0,Buf_len);
  Make_Filename(FileNo, pFileName);
  if(OpenFileWr(DiskBuf, pFileName, pCluster, pDirAddr)!=OK) return Rvalue;
  // SVG document has some basic properties of SVG, specify an external DTD.
  str = "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n";
  WriteStr(str, pCluster);
  str = "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\" \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\n";
  WriteStr(str, pCluster);
  str = "<svg width=\"";
  WriteStr(str, pCluster);
  WriteVar(DEPTH[PopMenu1_Value[WIN_Depth]], pCluster);
  str = "\" height=\"200\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" onload=\"init(evt)\" onmousemove=\"msMove(evt)\">\n";
  WriteStr(str, pCluster);
  // Background frame size color.
  str = "<path stroke=\"none\" fill=\"black\" d=\"M0,0 H";
  WriteStr(str, pCluster);
  WriteVar(DEPTH[PopMenu1_Value[WIN_Depth]], pCluster);
  str = " V200 H0 V0\"/>\n";
  WriteStr(str, pCluster);
  // Current window background.
  str = "<path stroke=\"none\" fill=\"#400040\" d=\"M";  // purple
  WriteStr(str, pCluster);
  WriteVar(PopMenu1_Value[WIN_Posi], pCluster);
  str = ",0 H";
  WriteStr(str, pCluster);
  WriteVar(PopMenu1_Value[WIN_Posi], pCluster);
  str = " V200 H";
  WriteStr(str, pCluster);
  WriteVar(PopMenu1_Value[WIN_Posi]+300, pCluster);
  str = "  V0\"/>\n";
  WriteStr(str, pCluster);
  // Background grid.
  for (y=0; y<=200; y+=25){
    //<path stroke="#404040" d="M0,0 H4096"/>
    str = "<path stroke=\"#404040\" d=\"M0,";
    WriteStr(str, pCluster);
    WriteVar(y, pCluster);
    str = " H";
    WriteStr(str, pCluster);
    WriteVar(DEPTH[PopMenu1_Value[WIN_Depth]], pCluster);
    str = "\"/>\n";
    WriteStr(str, pCluster);
  }
  for (x=0; x<=DEPTH[PopMenu1_Value[WIN_Depth]]; x+=25){
    //<path stroke="#404040" d="M0,0 V256"/>
    str = "<path stroke=\"#404040\" d=\"M";
    WriteStr(str, pCluster);
    WriteVar(x, pCluster);
    str = ",0 V200\"/>\n";
    WriteStr(str, pCluster);
  }
  // Channel A data.
  str = "<path stroke=\"cyan\" fill=\"none\" stroke-width=\"1\" d=\"";
  WriteStr(str, pCluster);
  for (i=0; i<=DEPTH[PopMenu1_Value[WIN_Depth]]; i++){
      temp = ((((Smpl[i])-2048)*Ak)>>12)+100;
    if(temp > 0){
      if(temp > 200)  track[0] = 199;
      else            track[0] = temp;
    } else            track[0] = 0;
    track[0] =  200- track[0];     //
    if(i==0)str = "M";
    else    str = " L";
    WriteStr(str, pCluster);
    WriteVar(i, pCluster);
    str = ",";
    WriteStr(str, pCluster);
    WriteVar(track[0], pCluster);
  }
  str = "\"/>\n";
  WriteStr(str, pCluster);
  // Channel B data....
  str = "<path stroke=\"yellow\" fill=\"none\" stroke-width=\"1\" d=\"";
  WriteStr(str, pCluster);
  for (i=0; i<=DEPTH[PopMenu1_Value[WIN_Depth]]; i++){
      temp = ((((Smpl[i+8192])-2048)*Bk)>>12)+100;
    if(temp > 0){
      if(temp > 200)  track[1] = 199;
      else            track[1] = temp;
    } else            track[1] = 0;
    track[1] =  200- track[1];     //
    if(i==0)str = "M";
    else    str = " L";
    WriteStr(str, pCluster);
    WriteVar(i, pCluster);
    str = ",";
    WriteStr(str, pCluster);
    WriteVar(track[1], pCluster);
  }
  str = "\"/>\n";
  WriteStr(str, pCluster);
  // Right-end range/timebase data
  str = "<text font-family=\"Arial\" x=\"638\" y=\"170\" fill=\"cyan\" font-size=\"18\" text-anchor=\"end\" id=\"CH1s\">CH1: ";
  WriteStr(str, pCluster);
  if(PopMenu1_Value[CH1_Probe])str = (u8*)Vol_10X[PopMenu1_Value[CH1_Vol]];
  else                         str = (u8*)Vol_Str[PopMenu1_Value[CH1_Vol]];
  WriteStr(str, pCluster);
  str = "/div ";
  WriteStr(str, pCluster);
  str = "(";                                // (
  WriteStr(str, pCluster);
  if(PopMenu1_Value[CH1_Probe])str = "X10"; // X10
  else                         str = "X1";  // X1
  WriteStr(str, pCluster);
  str = ") ";                               // )
  WriteStr(str, pCluster);
  str = (u8*)TimeBase_Str[PopMenu1_Value[TIM_Base]]; // Timebase
  WriteStr(str, pCluster);
  str = "/div</text>\n";
  WriteStr(str, pCluster);
  str = "<text font-family=\"Arial\" x=\"2\" y=\"170\" fill=\"cyan\" font-size=\"18\" text-anchor=\"start\" id=\"CH1\"> </text>\n";
  WriteStr(str, pCluster);

  str = "<text font-family=\"Arial\" x=\"638\" y=\"195\" fill=\"yellow\" font-size=\"18\" text-anchor=\"end\" id=\"CH2s\">CH2: ";
  WriteStr(str, pCluster);
  if(PopMenu1_Value[CH2_Probe])str = (u8*)Vol_10X[PopMenu1_Value[CH2_Vol]];
  else                         str = (u8*)Vol_Str[PopMenu1_Value[CH2_Vol]];
  WriteStr(str, pCluster);
  str = "/div ";
  WriteStr(str, pCluster);
  str = "(";                                // (
  WriteStr(str, pCluster);
  if(PopMenu1_Value[CH2_Probe])str = "X10"; // X10
  else                         str = "X1";  // X1
  WriteStr(str, pCluster);
  str = ") ";                               // )
  WriteStr(str, pCluster);
  str = (u8*)TimeBase_Str[PopMenu1_Value[TIM_Base]]; //Timebase
  WriteStr(str, pCluster);
  str = "/div</text>\n";
  WriteStr(str, pCluster);
  str = "<text font-family=\"Arial\" x=\"2\" y=\"195\" fill=\"yellow\" font-size=\"18\" text-anchor=\"start\" id=\"CH2\"> </text>\n";
  WriteStr(str, pCluster);
  //===============================
  // Variable.
  s16 Posi1 = 199-PopMenu1_Value[CH1_Posi];
  s16 Posi2 = 199-PopMenu1_Value[CH2_Posi];
  //===============================
  str = "<script type=\"text/ecmascript\"><![CDATA[\n";
  WriteStr(str, pCluster);
  str = "function init(evt) { if ( window.svgDocument == null ) svgDocument = evt.target.ownerDocument; }\n";
  WriteStr(str, pCluster);
  str = "function fmtT(val, div) {\n";
  WriteStr(str, pCluster);
  str = "if (div > 10000) return Math.floor(val*div/25/1000) + \" ms\";\n";
  WriteStr(str, pCluster);
  str = "if (div > 250) return Math.floor(val*div/25)/1000 + \" ms\";\n";
  WriteStr(str, pCluster);
  str = "return Math.floor(val*div*1000/25)/1000 + \" us\";\n";
  WriteStr(str, pCluster);
  str = "}\n";
  WriteStr(str, pCluster);
  str = "function fmtU(val, div, shift) { return Math.floor((shift-val)*div/25) + \" mV\"; }\n";
  WriteStr(str, pCluster);
  str = "function msMove (evt)\n";
  WriteStr(str, pCluster);
  str = "{\n";
  WriteStr(str, pCluster);
  //str = "var x = evt.clientX;\n";
  str = "var x = evt.pageX;\n";
  WriteStr(str, pCluster);
  str = "var y = evt.clientY;\n";
  WriteStr(str, pCluster);
  str = "if (x < 0 || y < 0 || x >= ";
  WriteStr(str, pCluster);
  WriteVar(DEPTH[PopMenu1_Value[WIN_Depth]], pCluster);
  str = "|| y >= 200)\n";
  WriteStr(str, pCluster);
  str = "return;\n";
  WriteStr(str, pCluster);
  str = "var hscroll = (document.all ? document.scrollLeft : window.pageXOffset);\n";
  WriteStr(str, pCluster);
  str = "var width = window.innerWidth;\n";
  WriteStr(str, pCluster);
  str = "\n";
  WriteStr(str, pCluster);
  str = "svgDocument.getElementById(\"CH1\").setAttribute(\"x\", hscroll+20);\n";
  WriteStr(str, pCluster);
  str = "svgDocument.getElementById(\"CH2\").setAttribute(\"x\", hscroll+20);\n";
  WriteStr(str, pCluster);
  str = "svgDocument.getElementById(\"CH1s\").setAttribute(\"x\", hscroll+width-20);\n";
  WriteStr(str, pCluster);
  str = "svgDocument.getElementById(\"CH2s\").setAttribute(\"x\", hscroll+width-20);\n";
  WriteStr(str, pCluster);
  str = "\n";
  WriteStr(str, pCluster);
  //==============================
  str = "svgDocument.getElementById(\"CH1\").firstChild.data = \"CH1: \"+fmtU(y, ";
  WriteStr(str, pCluster);
  //WriteVar(fScale1, pCluster);        //
  if(PopMenu1_Value[CH1_Probe])str = (u8*)VScale_Str_10X[PopMenu1_Value[CH1_Vol]];
  else str = (u8*)VScale_Str[PopMenu1_Value[CH1_Vol]];
  WriteStr(str, pCluster);
  str = ", ";
  WriteStr(str, pCluster);
  WriteVar((u16)Posi1, pCluster); //
  str = ") + \" \" + fmtT(x, ";
  WriteStr(str, pCluster);
  str = (u8*)TScale_Str[PopMenu1_Value[TIM_Base]]; // Timebase
  WriteStr(str, pCluster);
  //WriteVar(fTime, pCluster);         //
  str = ");\n";
  WriteStr(str, pCluster);

  str = "svgDocument.getElementById(\"CH2\").firstChild.data = \"CH2: \"+fmtU(y, ";
  WriteStr(str, pCluster);
  //WriteVar(fScale2, pCluster);        //
  if(PopMenu1_Value[CH2_Probe])str = (u8*)VScale_Str_10X[PopMenu1_Value[CH2_Vol]];
  else str = (u8*)VScale_Str[PopMenu1_Value[CH2_Vol]];
  WriteStr(str, pCluster);
  str = ", ";
  WriteStr(str, pCluster);
  WriteVar((u16)Posi2, pCluster); //
  str = ") + \" \" + fmtT(x, ";
  WriteStr(str, pCluster);
  //WriteVar(fTime, pCluster);         //
  str = (u8*)TScale_Str[PopMenu1_Value[TIM_Base]]; // Timebase
  WriteStr(str, pCluster);
  str = ");\n";
  WriteStr(str, pCluster);

  //==============================
  str = "}\n";
  WriteStr(str, pCluster);
  str = "]]></script>\n";
  WriteStr(str, pCluster);
  str = "</svg>\n";
  WriteStr(str, pCluster);

  if(ProgFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR; // Write data.
  PrintClk(264,2,(l++ >>1)& 3);                               // Progress indication.

  if(CloseFile(DiskBuf, Svg_Cnt, pCluster, pDirAddr)!= OK)
    return DISK_RW_ERR;
  if(PopMenu3_Value[SAVE_Csv]<99)PopMenu3_Value[SAVE_Svg]++;
  __USB_Port(ENABLE);
  memset(FileBuff,0,910);
  return OK;
}
/*******************************************************************************
 WriteStr
*******************************************************************************/
void WriteStr(u8* Str, u16* pClu)
  {
    u16 temp_addr,temp_len;
    u16 Buf_len = 4096;

    Length = strlen((char const*)Str);
    memcpy(&DiskBuf[Addr], Str, Length);
    Addr = Addr + Length;
    Svg_Cnt = Svg_Cnt+ Length;
    if(Addr >=Buf_len){
      if(ProgFileSec(DiskBuf, pClu)!= OK) {}; // Write data.
      PrintClk(264,2,(Clk_Cnt++ >>1)& 3);     // Progress indication.
      temp_len = Addr -Buf_len;
      temp_addr = Length - temp_len;
      Addr = 0;
      memset(DiskBuf,0,Buf_len);
      memset(FileBuff,0,910);
      memcpy(&DiskBuf[Addr],Str + temp_addr, temp_len);
      Addr = Addr + temp_len;
      Length = 0;
    }
  }
/*******************************************************************************
 WriteVar
*******************************************************************************/
void WriteVar(u16 Var, u16* pClu)
  {
    u16 temp_addr,temp_len;
    u16 Buf_len = 4096;

    memset(Num, 0, 4);
    u16ToDec4Str(Num,Var);
    Length = strlen((char const*)Num);
    memcpy(&DiskBuf[Addr], Num, Length);
    Addr = Addr + Length;
    Svg_Cnt = Svg_Cnt+ Length;
    if(Addr >=Buf_len){
      if(ProgFileSec(DiskBuf, pClu)!= OK) {}; // Write data.
      //PrintClk(264,2,(Clk_Cnt++ >>1)& 3);     // Progress indication.
      temp_len = Addr -Buf_len;
      temp_addr = Length - temp_len;
      Addr = 0;
      memset(DiskBuf,0,Buf_len);
      memset(FileBuff,0,910);
      memcpy(&DiskBuf[Addr],Num + temp_addr, temp_len);
      Addr = Addr + temp_len;
      Length = 0;
    }
  }
/*******************************************************************************
 WriteVar_u64
*******************************************************************************/
void WriteVar_u64(u64 Var, u16* pClu)
  {
    u16 temp_addr,temp_len;
    u16 Buf_len = 4096;

    memset(Num, 0, 4);
    u16ToDec4Str(Num,Var);
    Length = strlen((char const*)Num);
    memcpy(&DiskBuf[Addr], Num, Length);
    Addr = Addr + Length;
    Svg_Cnt = Svg_Cnt+ Length;
    if(Addr >=Buf_len){
      if(ProgFileSec(DiskBuf, pClu)!= OK) {}; // Write data.
      temp_len = Addr -Buf_len;
      temp_addr = Length - temp_len;
      Addr = 0;
      memset(DiskBuf,0,Buf_len);
      memset(FileBuff,0,910);
      memcpy(&DiskBuf[Addr],Num + temp_addr, temp_len);
      Addr = Addr + temp_len;
      Length = 0;
    }
  }
/*******************************************************************************
 Save_Parameter: Save current working parameters to internal Flash - Return: 0=Success
*******************************************************************************/
u8 Save_Parameter(void)
{
  unsigned short i, j;
  u16* ptr;

  FLASH_Unlock();
  j=FLASH_ErasePage(Page_Address);
  if(j== FLASH_COMPLETE){
    ptr=(u16*)&DiskBuf;
    *ptr++=0x0300;                              // Ver:3.00
    *ptr++=menu.current;
    *ptr++=Status;
    for(i=0;i<10;i++) *ptr++=menu.menu_index[i];
    for(i=0;i<PopMenu1_Len;i++) *ptr++=PopMenu1_Value[i];
    for(i=0;i<PopMenu2_Len;i++) *ptr++=PopMenu2_Value[i];
    for(i=0;i<PopMenu3_Len;i++) *ptr++=PopMenu3_Value[i];
    for(i=0;i<8;i++)  *ptr++=Kpg[i];             // 88+3
    *ptr++=0xaa55;
    ptr=(u16*)&DiskBuf;
    for(i=0; i<256; i+=2){                       // 128 SHORT
      j= FLASH_ProgramHalfWord(Page_Address+i, *ptr++);
      if(j!= FLASH_COMPLETE) break;
    }
  }
  FLASH_Lock();
  return j;
}

/*******************************************************************************
 Read_Parameter: read out the parameter field from the internal FLASH
*******************************************************************************/
void Read_Parameter(void)
{
  u8 i;
  u16* ptr;
  ptr=(u16*)(Page_Address+89*2);
  if(*ptr!=0xaa55) return ;
  ptr=(u16*)Page_Address;
  if(*ptr++!=0x0300) return ;
  menu.current=*ptr++;
  ptr++;
  for(i=0;i<10;i++) menu.menu_index[i]=*ptr++;
  for(i=0;i<PopMenu1_Len;i++) PopMenu1_Value[i] =*ptr++;
  for(i=0;i<PopMenu2_Len;i++) PopMenu2_Value[i] =*ptr++;
  for(i=0;i<PopMenu3_Len;i++) PopMenu3_Value[i] =*ptr++;

  return ;
}
/*******************************************************************************
 Save_Kpg: Save Calibration Parameters - Return: 0=Success
*******************************************************************************/
u8 Save_Kpg(void)
{
  unsigned short i, j;
  u16* ptr;

  FLASH_Unlock();
  j=FLASH_ErasePage(Kpg_Address);
  if(j== FLASH_COMPLETE){
    ptr=(u16*)&DiskBuf;
    *ptr++=0x0300;                              // Ver:3.00
    *ptr++=Cal_Flag;
    for(i=0;i<12;i++)  *ptr++=Kpg[i];
    ptr=(u16*)&DiskBuf;
    *(ptr+KpgSave_Len)=0xaa55;
    ptr=(u16*)&DiskBuf;
    for(i=0; i<256; i+=2){                      // 128 SHORT
      j= FLASH_ProgramHalfWord(Kpg_Address+i, *ptr++);
      if(j!= FLASH_COMPLETE) break;
    }
  }
  FLASH_Lock();
  return j;
}
/*******************************************************************************
 Read_Kpg: Read calibration parameters from FLASH
*******************************************************************************/
void Read_Kpg(void)
{
  u8 i;
  u16* ptr;
  ptr=(u16*)(Kpg_Address+KpgSave_Len*2);
  if(*ptr!=0xaa55) return ;
  ptr=(u16*)Kpg_Address;
  if(*ptr++!=0x0300) return ;
  Cal_Flag = *ptr++;
  for(i=0;i< 6;i++) Kpg[i] = *ptr++;  // CH_A,CH_B zero offset
  return ;
}
/*******************************************************************************
 Read_Kpg: Read calibration parameters from FLASH
*******************************************************************************/
void Read_CalFlag(void)
{
  u16* ptr;
  ptr=(u16*)(Kpg_Address+KpgSave_Len*2);
  if(*ptr!=0xaa55) return ;
  ptr=(u16*)Kpg_Address;
  if(*ptr++!=0x0300) return ;
  Cal_Flag = *ptr++;
  return ;
}

/*******************************************************************************
 Print_Clk: Progress indication
*******************************************************************************/
void PrintClk(u16 x0, u16 y0, u8 Phase)
{
  u16 i, j, b;
  x0=272;
  SetColor(DAR, WHT);
  DispStr(x0-20, 2, PRN, "          ");
  for(i=0; i<11; ++i){
    b = CLK_TAB[Phase *11 + i];
    for(j=0; j<11; ++j){
      __SetPosi((x0+j), (y0+1+i));
      if((b >> j)& 0x001)
        __SetPixel(RED);
      else
        __SetPixel(Background);
    }
  }
}
/*******************************************************************************
 Save_Parameter: Save current working parameters to U disk - Return: 0=Success
*******************************************************************************/
u8 Save_Param(void)
{
  u8  Sum = 0, Filename[12], Versions = 0x06;
  u16 i, Tmp[2];
  u16* ptr =(u16*)DiskBuf;

  u16 pCluster[3];
  u32 pDirAddr[1];

  Word2Hex(Filename, __Info(DEV_SN));
  //Filename[8] = 'P'; Filename[9] = 'A'; Filename[10] = 'R';
#if   defined (APP1)
  Filename[8] = 'P'; Filename[9] = 'A'; Filename[10] = 'R';
#elif defined (APP2)
  Filename[8] = 'P'; Filename[9] = 'A'; Filename[10] = '2';
#endif

  switch (OpenFileRd(DiskBuf, Filename, pCluster, pDirAddr)){
  case OK:                                                     // The original WPT file exists
    Tmp[0] = *pCluster;
    //Filename[8] = 'B'; Filename[9] = 'A'; Filename[10] = 'K';  // Convert to BAK file.
#if   defined (APP1)
  Filename[8] = 'B'; Filename[9] = 'A'; Filename[10] = 'K';  // Convert to BAK file.
#elif defined (APP2)
  Filename[8] = 'B'; Filename[9] = 'A'; Filename[10] = '2';  // Convert to BAK file.
#endif
    if(OpenFileWr(DiskBuf, Filename, pCluster, pDirAddr)!= OK)
      return DISK_RW_ERR;
    if(ReadFileSec(DiskBuf, Tmp     )!= OK) return FILE_RW_ERR;
    if(ProgFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR; // Save BAK file.
    if(CloseFile(DiskBuf, 512, pCluster, pDirAddr)!= OK)
      return FILE_RW_ERR;
  case NEW:                                                    // Original WPT file does not exist.
    //Filename[8] = 'P'; Filename[9] = 'A'; Filename[10] = 'R';  // Create WPT file.
#if   defined (APP1)
  Filename[8] = 'P'; Filename[9] = 'A'; Filename[10] = 'R';  // Create WPT file.
#elif defined (APP2)
  Filename[8] = 'P'; Filename[9] = 'A'; Filename[10] = '2';  // Create WPT file.
#endif
    if(OpenFileWr(DiskBuf, Filename, pCluster, pDirAddr)!= OK)
      return DISK_RW_ERR;
    memset(DiskBuf, 0, 512);
    *ptr++ =(menu.current <<8)+ Versions;           // Save parameter table version number and current page.
    *ptr++=Status;
    for(i=0;i<10;i++) *ptr++=menu.menu_index[i];
    for(i=0;i<PopMenu1_Len;i++) *ptr++=PopMenu1_Value[i];
    for(i=0;i<PopMenu2_Len;i++) *ptr++=PopMenu2_Value[i];
    for(i=0;i<PopMenu3_Len;i++) *ptr++=PopMenu3_Value[i];

    for(i=0; i<511; i++)  Sum += DiskBuf[i];  // Calculate the parameter table checksum.
    DiskBuf[511] = (~Sum)+ 1;
    if(ProgFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR; // Write data.
    if(CloseFile(DiskBuf, 512, pCluster, pDirAddr)!= OK)
      return FILE_RW_ERR;

    Save_Flag = 1; // Save parameter flag.

    return OK;
    default:  return FILE_RW_ERR;
  }
}
/*******************************************************************************
 Save_One_Param: Save current working parameters to USB flash drive - Return: 0=Success
*******************************************************************************/
u8 Save_One_Param(u8 Page, u8 Num)
{
  u8   Sum = 0, Filename[12], Versions = 0x06;
  u16  i,j,Tmp[2];
  u16* ptr =(u16*)DiskBuf;
  s16  Temp[12];

  u16 pCluster[3];
  u32 pDirAddr[1];


  switch(Page){                           // Read previously saved parameters.
    case Oscillo:
      Temp[0] = PopMenu1_Value[Num];
      if(Load_Param())Restore_OrigVal();  // If the parameters are saved, they are read without restoring the factory parameters.
      else PopMenu1_Value[Num] = Temp[0]; // Save parameters to be saved separately.
      break;
    case Measure:
      Temp[0] = PopMenu2_Value[Num];
      if(Load_Param())Restore_OrigVal();
      else PopMenu2_Value[Num] = Temp[0];
      break;
    case Option:
      for(j=0;j<7;j++){                   // Save 7 parameters.
        //Temp[0] = PopMenu3_Value[Num];  // Save a parameter.
        Temp[j] = PopMenu3_Value[Num+j];
      }
      if(Load_Param())Restore_OrigVal();
      else {
        for(j=0;j<7;j++){
         //PopMenu3_Value[Num] = Temp[0];
          PopMenu3_Value[Num+j] = Temp[j];
        }
         // When restoring to the factory without saving, the Bmp sequence number is the number before the restoration..
        if((Rest_Flag==1) && (Save_Flag==0))
          for(j=0;j<7;j++){ // Save 7 parameters.
            PopMenu3_Value[SAVE_Bmp+j] = BmpNum[j];
          }
      }
      break;
  }

  Word2Hex(Filename, __Info(DEV_SN));
  //Filename[8] = 'P'; Filename[9] = 'A'; Filename[10] = 'R';
#if   defined (APP1)
  Filename[8] = 'P'; Filename[9] = 'A'; Filename[10] = 'R';
#elif defined (APP2)
  Filename[8] = 'P'; Filename[9] = 'A'; Filename[10] = '2';
#endif
  switch (OpenFileRd(DiskBuf, Filename, pCluster, pDirAddr)){
  case OK:                                                     // The original WPT file exists.
    Tmp[0] = *pCluster;
    //Filename[8] = 'B'; Filename[9] = 'A'; Filename[10] = 'K';  // Convert to BAK file.
#if   defined (APP1)
    Filename[8] = 'B'; Filename[9] = 'A'; Filename[10] = 'K';  // Convert to BAK file.
#elif defined (APP2)
    Filename[8] = 'B'; Filename[9] = 'A'; Filename[10] = '2';  // Convert to BAK file.
#endif
    if(OpenFileWr(DiskBuf, Filename, pCluster, pDirAddr)!= OK)
      return DISK_RW_ERR;
    if(ReadFileSec(DiskBuf, Tmp     )!= OK) return FILE_RW_ERR;
    if(ProgFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR;  // Save BAK file.
    if(CloseFile(DiskBuf, 512, pCluster, pDirAddr)!= OK)
      return FILE_RW_ERR;/**/
  case NEW:                                                   // Original WPT file does not exist.
    //Filename[8] = 'P'; Filename[9] = 'A'; Filename[10] = 'R'; // Create WPT file.
#if   defined (APP1)
    Filename[8] = 'P'; Filename[9] = 'A'; Filename[10] = 'R';
#elif defined (APP2)
    Filename[8] = 'P'; Filename[9] = 'A'; Filename[10] = '2';
#endif
    if(OpenFileWr(DiskBuf, Filename, pCluster, pDirAddr)!= OK)
      return DISK_RW_ERR;
    memset(DiskBuf, 0, 512);

    *ptr++ =(menu.current <<8)+ Versions;           // Save parameter table version number and current page.
    *ptr++=Status;
    for(i=0;i<10;i++) *ptr++=menu.menu_index[i];
    for(i=0;i<PopMenu1_Len;i++) *ptr++=PopMenu1_Value[i];
    for(i=0;i<PopMenu2_Len;i++) *ptr++=PopMenu2_Value[i];
    for(i=0;i<PopMenu3_Len;i++) *ptr++=PopMenu3_Value[i];

    for(i=0; i<511; i++)  Sum += DiskBuf[i];  // Calculate the parameter table checksum.
    DiskBuf[511] = (~Sum)+ 1;
    if(ProgFileSec(DiskBuf, pCluster)!= OK)
      return FILE_RW_ERR; // Write data.
    if(CloseFile(DiskBuf, 512, pCluster, pDirAddr)!= OK)
      return FILE_RW_ERR;
    return OK;
    default:  return FILE_RW_ERR;
  }
}
/*******************************************************************************
 Load_Parameter: Work parameters before loading From USB drive - Return: 0=Success
*******************************************************************************/
u8 Load_Param(void)
{
  u8  Sum = 0, Filename[12], Versions = 0x06;
  u16 i,rc;
  u16* ptr =(u16*)DiskBuf;

  u16 pCluster[3];
  u32 pDirAddr[1];

  Word2Hex(Filename, __Info(DEV_SN));
  //Filename[8] = 'P'; Filename[9] = 'A'; Filename[10] = 'R'; Filename[11] = 0;
#if   defined (APP1)
  Filename[8] = 'P'; Filename[9] = 'A'; Filename[10] = 'R';
#elif defined (APP2)
  Filename[8] = 'P'; Filename[9] = 'A'; Filename[10] = '2';
#endif
  rc = OpenFileRd(DiskBuf, Filename, pCluster, pDirAddr);
  if(rc != OK) return FILE_RW_ERR;

  else
  {
    if(ReadFileSec(DiskBuf, pCluster)!= OK) return FILE_RW_ERR;
    if(Versions !=(*ptr & 0xFF)) return VER_ERR;  // Version error return.
    for(i=0; i<512; ++i) Sum += DiskBuf[i];
    if(Sum != 0) return SUM_ERR;                  // Checksum error return.

    menu.current =(*ptr++ >>8);                   // Current title priot to loading.
    ptr++;
    for(i=0;i<10;i++) menu.menu_index[i]=*ptr++;
    for(i=0;i<PopMenu1_Len;i++) PopMenu1_Value[i] =*ptr++;
    for(i=0;i<PopMenu2_Len;i++) PopMenu2_Value[i] =*ptr++;
    for(i=0;i<PopMenu3_Len;i++) PopMenu3_Value[i] =*ptr++;

  return rc;
 }
}

/*******************************************************************************
 Restore_OrigVal Restore factory parameters
*******************************************************************************/
void Restore_OrigVal(void)
{
    u8 i,j;
    Rest_Flag = 1;
    for(j=0;j<7;j++){
      BmpNum[j] = PopMenu3_Value[SAVE_Bmp+j];
    }
    menu.current = Oscillo;
    for(i=0;i<3;i++)  menu.menu_index[i]=0;
    for(i=0;i<PopMenu1_Len;i++) PopMenu1_Value[i] =PopMenu1_CValue[i];
    for(i=0;i<PopMenu2_Len;i++) PopMenu2_Value[i] =PopMenu2_CValue[i];
    for(i=0;i<PopMenu3_Len;i++) PopMenu3_Value[i] =PopMenu3_CValue[i];
}
/*******************************************************************************
 Load_File_Num(u8 Tpye) - Input: File Type - Return Value: File Number
*******************************************************************************/
s16 Load_File_Num(u8 Tpye)
{
  s16 FileNo=0;
  u8  pFileName[12]="FILE       ";
  u16 pCluster[3];
  u32 pDirAddr[1];

  switch(Tpye)
  {
  case SAVE_BMP:
    memset(DiskBuf,0,4096);
    memcpy(pFileName,"IMG_    BMP",12);
    Make_Filename(FileNo, pFileName);
    while(OpenFileRd(DiskBuf, pFileName, pCluster, pDirAddr)==OK) {
      FileNo++;
      Make_Filename(FileNo, pFileName);
    }
    break;
  case SAVE_DAT:
    memset(DiskBuf,0,4096);
    memcpy(pFileName,"FILE    DAT",12);
    Make_Filename(FileNo, pFileName);
    while(OpenFileRd(DiskBuf, pFileName, pCluster, pDirAddr)==OK) {
      FileNo++;
      Make_Filename(FileNo, pFileName);
    }
    break;
  case SAVE_BUF:
    memset(DiskBuf,0,4096);
    memcpy(pFileName,"DATA    BUF",12);
    Make_Filename(FileNo, pFileName);
    while(OpenFileRd(DiskBuf, pFileName, pCluster, pDirAddr)==OK) {
      FileNo++;
      Make_Filename(FileNo, pFileName);
    }
    break;
  case SAVE_CSV:
    memset(DiskBuf,0,4096);
    memcpy(pFileName,"DATA    CSV",12);
    Make_Filename(FileNo, pFileName);
    while(OpenFileRd(DiskBuf, pFileName, pCluster, pDirAddr)==OK) {
      FileNo++;
      Make_Filename(FileNo, pFileName);
    }
    break;
  case SAVE_SVG:
    memset(DiskBuf,0,4096);
    memcpy(pFileName,"DATA    SVG",12);
    Make_Filename(FileNo, pFileName);
    while(OpenFileRd(DiskBuf, pFileName, pCluster, pDirAddr)==OK) {
      FileNo++;
      Make_Filename(FileNo, pFileName);
    }
    break;
  }
  return FileNo;
}
/*******************************************************************************
 File_Num(void) Input: File Type - Return Value: File Number
*******************************************************************************/
void File_Num(void)
{
  PopMenu3_Value[SAVE_BMP-1] =  Load_File_Num(SAVE_BMP);
  PopMenu3_Value[SAVE_DAT-1] =  Load_File_Num(SAVE_DAT);
  PopMenu3_Value[SAVE_BUF-1] =  Load_File_Num(SAVE_BUF);
  PopMenu3_Value[SAVE_CSV-1] =  Load_File_Num(SAVE_CSV);
  PopMenu3_Value[SAVE_SVG-1] =  Load_File_Num(SAVE_SVG);
}
/******************************** END OF FILE *********************************/

