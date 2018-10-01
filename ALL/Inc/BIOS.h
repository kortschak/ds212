/********************* (C) COPYRIGHT 2017 e-Design Co.,Ltd. ********************
 Brief   : Low-level hardware configuration                     Author : bure
*******************************************************************************/
#ifndef __BIOS_H
#define __BIOS_H

#include "STM32F30x.h"

extern u16 AutoPwr_Cnt,AutoPwr_Cnt,PwrCnt,PD_Cnt,RunCnt, BeepCnt;
extern u16 Key_Wait_Cnt,Key_Repeat_Cnt;
extern u8  NumStr[20];


enum{LOW = 0, HIGH = 1, ANALOG = 2, PULSED = 4, SIMULTANEO = 5, INTERLEAVE = 6,};


// Function u32 Info(u8 Item) in Item Definition and return type description
//=====+========+========+========+========+========+========+========+========+
//Item:|Product Model|Hardware Version|MCU Model|LCD Model|LCD Model|FPGA Model|DFU Version|OEM Version|
//-----+--------+--------+--------+--------+--------+--------+--------+--------+
// RET:|   u8*  |   u8*  |   u8*  |   u8*  |   u8*  |   u8*  |   u8*  |   u8*  |
//-----+--------+--------+--------+--------+--------+--------+--------+--------+
enum   {  PROD,     SCH,     MCU,     LCD,     ADC,    FPGA,     DFU,     OEM,
//=====+========+========+========+========+========+========+========+========+
//Item:|U disk capacity|Sector size|Number of sectors|Product serial number|License|Power state|Input voltage|Input current|
//-----+--------+--------+--------+--------+--------+--------+--------+--------+
// RET:|   u8*  |   u16  |   u16  |   u32  |   u8   |   u16  | u16 mV | u16 mA |
//-----+--------+--------+--------+--------+--------+--------+--------+--------+
          DISK,   SECTOR,  AMOUNT,  DEV_SN,  LIC_OK,  P_INFO,  P_VUSB,  P_IUSB,
//=====+========+========+========+========+========+========+========+========+
//Item:|Power supply temperature|Battery voltage|Charge current|Discharge current|Supply voltage|Recharge power|Discharge power|Sine wave table|
//-----+--------+--------+--------+--------+--------+--------+--------+--------+
// RET:|  u8 C  | u16 mV | u16 mA | u16 mA | u16 mV |   u32  |   u32  |   u16* |
//-----+--------+--------+--------+--------+--------+--------+--------+--------+
         P_TEMP,  P_VBAT,  P_ICHG,  P_IBAT,  P_VAPS,  P_QNT1,  P_QNT2, SIN_TAB,
//=====+========+========+========+========+========+========+========+========+
//Item:|Triangle wave table|Sawtooth wave table|Sample count|Sample count|Input key|Horizontal coding|Vertical coding|
//-----+--------+--------+--------+--------+--------+--------+--------+--------+
// RET:|   u16* |   u16* |   u16  |   u16  |   u8   |   u16  |   u16  |
//-----+--------+--------+--------+--------+--------+--------+--------+--------+
        TRG_TAB, SAW_TAB, CHA_CNT, CHB_CNT,  KEY_IN,  H_ENCD , V_ENCD };
//=====+========+========+========+========+========+========+========+========+





// Function void Ctrl(u8 Item, u32 Val) in Item Definition and Val type description
//=====+========+========+========+========+========+========+========+========+
//Item:|System Configuration| LCD clear screen| LCD direction|Millisecond delay|Interrupt timing|Sound state|Sound volume|Backlight brightness|
//-----+--------+--------+--------+--------+--------+--------+--------+--------+
// Val:|   u8   |  Color |   0~3  |   mS   |   mS   |   0/1  |  0~99  |  0~99  |
//-----+--------+--------+--------+--------+--------+--------+--------+--------+
enum   {SYS_CFG, LCD_CLR, LCD_DIR, DELAYmS, SYS_INT, BUZZ_ST, BUZZVOL, B_LIGHT,
//=====+========+========+========+========+========+========+========+========+
//Item:|Output mode|Pulse frequency Division|Pulse period|Pulse width|Analog timing|Analog points|Analog data|Analog level|
//-----+--------+--------+--------+--------+--------+--------+--------+--------+
// Val:|   0~2  |   u16  |   u16  |   u16  |  0~99  |   u16  |  u16*  |   u16  |
//-----+--------+--------+--------+--------+--------+--------+--------+--------+
        OUT_MOD, OUT_PSC, OUT_ARR, OUT_WTH, DAC_TIM, OUT_CNT, OUT_BUF, OUT_VAN,
//=====+========+========+========+========+========+========+========+========+
//Item:|Power enable|DC1 power|LDO power|Power signal|Sampling status|Sampling timing|Sampling buffer|Sampling total|
//-----+--------+--------+--------+--------+--------+--------+--------+--------+
// Val:|   0~1  |   u8   |   u8   |   u8   |   0~1  |   u16  |  u16*  |   u16  |
//-----+--------+--------+--------+--------+--------+--------+--------+--------+
         PM_EN,   PM_VDC1, PM_LDO, PM_CTRL, SMPL_ST, SMPLTIM, SMPLBUF, SMPLNUM,
//=====+========+========+========+========+========+========+========+========+
//Item:| CHA range | CHB range | CHA offset | CHA offset | Sampling mode | Voltage switch |
//-----+--------+--------+--------+--------+--------+--------+--------+--------+
// Val:|   0~1  |   u8   | 0~4095 | 0~4095 |  0~1   |   0~1  |
//-----+--------+--------+--------+--------+--------+--------+--------+--------+
        AiRANGE, BiRANGE, AOFFSET, BOFFSET,SMPL_MODE,PWROFF};
//=====+========+========+========+========+========+========+========+========+


// Function void Ctrl(SYS_CFG, Val) in  Val Defined device type description
//=====+=================+=================+=================+=================+
// Val:|   RCC initialization    |  Timers initialization  |   GPIO & OpAmp  |   TouchPad IRQ  |
//-----+-----------------+-----------------+-----------------+-----------------+
enum   {  RCC_DEV = 0x01,   TIM_DEV = 0x02,  GPIO_OPA = 0x04,   CTP_DEV = 0x08,
//-----+-----------------+-----------------+-----------------+-----------------+
//     |    I2C initialization   |  ADC DAC initialization |    LCD initialization   |    MSD initialization   |
//-----+-----------------+-----------------+-----------------+-----------------+
            SPI = 0x10,   ADC_DAC = 0x20,   LCD_DEV = 0x40,  USB_DISK = 0x80,};
//=====+=================+=================+=================+=================+


// Function void Ctrl(AiRANGE/BiRANGE, AC+LV+ACT) in Val Defined channel characteristics description
//======+======================+=======================+=======================+
// Val: |      AC/DC coupling      |     Low/high voltage range     |     Grounding/activation     |
//------+----------------------+-----------------------+-----------------------+
enum    { AC = 0x00, DC = 0x02,  LV = 0x00,  HV = 0x01, GND = 0x00, ACT = 0x04,};
//======+======================+=======================+=======================+

//------------------------------ SCI related macro definitions --------------------------------

#define SCI_CLK_HIGH()    GPIOA->BSRR = USB_DN
#define SCI_DIO_HIGH()    GPIOA->BSRR = USB_DP

#define SCI_CLK_LOW()     GPIOA->BRR  = USB_DN
#define SCI_DIO_LOW()     GPIOA->BRR  = USB_DP

#define SCI_DIO_ST        GPIOA->IDR & USB_DP

//----------------------- Ext Flash Control related macro definitions -----------------------------

#define ExtFlash_CS_HIGH()  GPIOB->BSRR = SPI_CS
#define ExtFlash_CS_LOW()   GPIOB->BRR  = SPI_CS
#define ExtFlash_RST_HIGH() GPIOD->BSRR = SPI_RST
#define ExtFlash_RST_LOW()  GPIOD->BRR  = SPI_RST


void Ctrl(u8 Device, u32 Value);
u32  __Info(u8 Item);
void __Ctrl(u8 Device, u32 Value);
void __ExtFlashSecWr(u8* pBuf, u32 WrAddr);
void __ExtFlashDataRd(u8* pBuf, u32 RdAddr, u16 Len);
u16  __I2C_Write(u8 DevID, u8 Addr, u8 Data);
u16  __I2C_Read (u8 DevID, u8 Addr, u8* pBuf, u16 n);
void __SetBlock(u16 x1, u16 y1, u16 x2, u16 y2);     // LCD Set Block
void __SetPosi(u16 x0, u16 y0);                      // LCD Set Posision
void __SetPixel(u16 Color);                          // LCD Set Pixel
void __SendPixels(u16* pBuf, u16 n);                 // LCD Send Pixels
u16  __ReadPixel(void);                              // LCD Read Pixel
u16  __Font_8x14(u8 Code, u16 Row);
void __Disp_Logo(u16 x0, u16 y0);
void __Disp_Str(u16 x, u16 y, u16 Color, u8 *s);
u8   __FLASH_Prog(u32 Addr, u16 Data);
void __FLASH_Erase(u32 Addr);
void __Disp_OEM(void);



#endif
/********************************* END OF FILE ********************************/
