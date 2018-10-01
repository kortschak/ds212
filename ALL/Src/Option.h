;/********************* (C) COPYRIGHT 2017 e-Design Co.,Ltd. *******************
; Option.h                                                        Author : bure
;******************************************************************************/
;/*--------------------------- APP type compilation options ------------------------------*/
  #define APP1
//#define APP2
 /*At the same time to modify the starting address of the icf file, APP1->0x8000,APP2->0x20000*/
;/*--------------------------- Machine type compilation options ------------------------------*/
  #define TYPE_DS212

;/*--------------------------- OEM type compilation options ------------------------------*/
  #define GENERAL

;/*--------------------------- LCD type compilation options ------------------------------*/
  #define LCD_ILI_9341
  //#define LCD_ST_7781

;/*--------------------------- LCD interface type options ------------------------------*/
  #define LCD_GPIO

;/*--------------------------- U disk type compilation options ------------------------------*/
  #define FLASH_DISK_8M

;/*********************************  END OF FILE  *****************************/
