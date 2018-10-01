/********************* (C) COPYRIGHT 2017 e-Design Co.,Ltd. ********************
 File Name : Draw.h
 Version   : DS212                                                 Author : bure
*******************************************************************************/
#ifndef __DRAW_H
#define __DRAW_H

#include "STM32F30x.h"

//========== Waveform display window related definitions ===========

#define LCD_X1     0
#define LCD_X2     319
#define LCD_Y1     0
#define LCD_Y2     239
#define X_SIZE     300
#define Y_SIZE     200
#define X_BASE     1
#define Y_BASE     1
#define MIN_X      1
#define MIN_Y      19
#define MAX_X      (X_SIZE + MIN_X)
#define MAX_Y      (Y_SIZE + MIN_Y)
#define WIDTH      300
#define WIDTH_MINI 251

//================= Display method =================
#define PRN        0x00
#define INV        0x10   /* Bit4: 0/1 = NORM/INV   normal/contrast*/
#define SYMB       0x20   /* Bit5: O/1 = CHAR/SYMB  character/blocks*/
#define VOID       0x40   /* Bit6: 0/1 = REAL/VOID  solid line/dotted line*/

//============= Palette color definition ===============
#define CYN        0xFFE0  /* 0*/
#define PUR        0xF81F  /* 1*/
#define YEL        0x07FF  /* 2*/
#define GRN        0x07E0  /* 3*/
#define CYN_       0xBDE0  /* 4*/
#define PUR_       0xB817  /* 5*/
#define YEL_       0x05F7  /* 6*/
#define GRN_       0x05E0  /* 7*/
#define ORN        0x051F  /* 8*/
#define BLK        0x0000  /* 9*/
#define WHT        0xFFFF  /* 10*/
#define BLU        0xFC10  /* 11*/
#define RED        0x001F  /* 12*/
#define GRY        0x7BEF  /* 13*/
#define LGN        0x27E4  /* 14*/
#define DAR        0x39E7  /* 15*/
#define GRAY       0x7BEF
#define RED_       0x631F

// Parameter definition in window parameter table
//==============+======+======+======+======+======+======+======+======+======+
//Parameter marker positioning  | CH_A | CH_B | CH_C | VT line | V1 line | V2 line | T0 line | T1 line | T2 line |
//--------------+------+------+------+------+------+------+------+------+------+
enum            {  W1F,   W2F,   W3F,   VTF,   V1F,   V2F,   T0F,   T1F,   T2F,
//==============+======+======+======+======+======+======+======+======+======+
// Display variable positioning | A zero position | B zero position | C zero position | VT position | V1 position | V2 position | T0 position | T1 position | T2 position |
//--------------+------+------+------+------+------+------+------+------+------+
                  P1x2,  P2x2,  P3x2,  VTx2,  V1x2,  V2x2,  T0x1,  T1x1,  T2x1,
//==============+======+======+======+======+======+======+======+======+======+
// Display color location | A main color| B main color| C main color| VT color | V1 color | V2 color | T0 color | T1 color | T2 color |
//--------------+------+------+------+------+------+------+------+------+------+
                   W1C,   W2C,   W3C,   VTC,   V1C,   V2C,   T0C,   T1C,   T2C,
//==============+======+======+======+======+======+======+======+======+======+
// Display color location | A secondary color| B secondary color| C secondary color|      |      |      |      |      |      |
//--------------+------+------+------+------+------+------+------+------+------+
                  W1C_,  W2C_,  W3C_,   
//==============+======+======+======+======+======+======+======+======+======+
// Main window parameter positioning | x position | y position | x width |      |      |      |      |
//--------------+------+------+------+------+------+------+------+------+------+
                  M_X0,  M_Y0,  M_WX,                        
//==============+======+======+======+======+======+======+======+======+======+
// Sub-window parameter positioning | Logo | x position | x width | y position | y width |      |      |      |      |
//--------------+------+------+------+------+------+------+------+------+------+
                  POPF,  PXx1,  PWx1,  PYx2,  PHx2,                        
//==============+======+======+======+======+======+======+======+======+======+
//  Sub-window palette  |  #1  |  #2  |  #3  |  #4  |  #5  |  #6  |  #7  |      |
//--------------+------+------+------+------+------+------+------+------+------+
                  POP_CN, };
//==============+======+======+======+======+======+======+======+======+======+

//=========== Display window flag definition =============

#define SHOW        0x00       /* All Show*/
#define D_HID       0x01       /* Endp Hide*/
#define L_HID       0x02       /* Line Hide*/
#define W_HID       0x04       /* Wave Hide*/
#define B_HID       0x06       /* Line & Wave Hide*/
#define A_HID       0x07       /* End Dot & Line & Wave Hide*/

//=========== Pop-up subwindow mark definition =============

#define P_HID       0x01       /* Pop Hide*/
#define LIST_POP    0x02       /**/
#define DAILOG_POP  0x04       /**/
#define DEMO_POP    0x08       /**/
#define PWR_POP     0x10
#define MENU_POP    0x20
#define FILE_POP    0x40
#define COVER       1
#define TRNSI       0

//=========== Pop-up subwindow auto close time =============
#define   POP_TIME   5000 /* ms */

//============== Display parameter definition ==================

#define CCM_ADDR   0x10000000
#define POP_SIZE   6692
#define POP_BUF    1400
#define TR1_SIZE   300
#define TR2_SIZE   300
#define TR3_SIZE   300
#define TR1_pBUF   500
#define TR2_pBUF   800
#define TR3_pBUF   1100
#define TAB_PTR    410

// Row_Buf:0~407     Pam_Tab:410~499    TR1_Buf:500~799
// TR2_Buf:800~1099  TR3_Buf:1100~1399  Pop_Buf:1400~8191
extern uc16 PARAM[];
extern uc8  DEMO[];
extern u8   PopType;  
extern u16* ParamTab;
extern u16  Background, Foreground, Sx, Sy; //  Nx, Ny,
extern uc16 CHAR10x14[],CHAR8x14[],CHAR8x9[];
extern uc8  CHAR6x8[];

void __DrawWindow(u8* VRAM_Addr);
void PopColor(u16 Board_Color, u16 Text_Color);
void SetColor(u16 Board_Color, u16 Text_Color);
void PopPixel(u8 ColorNum);
void OpenPop(u8 PopX, u8 PopY, u8 PopW, u8 PopH, u8 Mode);
void PopChar(u8 Mode, u8 Code);
void Pop_STR(u16 Nx0, u16 Ny0, u8 Mode, u8 *Str);
void DispChar(u8 Mode, u8 Code);
void DispStr(u16 x0,  u16 y0,  u8 Mode, u8 *Str);
void DispStr10x14(u16 x0,  u16 y0,  u8 Mode, u8 *Str);
void DispChar10x14(u8 Mode, u8 Code);
void PrintStr(u8 Mode, u8 *Str);
void PrintStr_Cur(u8 Mode, u8 *Str);
void List_Pop(void);
void ClosePop(void);
void MovePop(void);
void Dialog_Pop(u8* Str);
void PwrOff_Pop(void);
void Tips_Pop(u8* Str);
void Demo_Pop(void);
void DispStr8x9(u16 x0, u16 y0, u8 Mode, u8 *Str);
void DispChar8x9(u8 Mode, u8 Code);
u16  Get_TAB_8x9(u8 Code, u16 Row);
u16  Get_TAB_6x8(u8 Code, u16 Row);
u16  Get_TAB_10x14(u8 Code, u16 Row);
void DispStr6x8(u16 x0, u16 y0, u8 Mode, u8 *Str);
void DispChar6x8(u8 Mode, u8 Code);
void PrintStr6x8(u8 Mode, u8 *Str);
void Pop_STR6x8(u16 Nx0, u16 Ny0, u8 Mode, u8 *Str);
void PopChar6x8(u8 Mode, u8 Code);
void Power_Pop(u8* Str, u16 X_Posi, u16 Y_Posi, u16 High, u16 Width );
void Dialog_CalPop(u8* Str, u16 X_Posi, u16 Y_Posi, u16 High, u16 Width );
void Product_Pop(u8* Str, u16 X_Posi, u16 Y_Posi, u16 High, u16 Width );
void About_Pop(u8* Str, u16 X_Posi, u16 Y_Posi, u16 High, u16 Width );
void Clr_WaveWin(u16 Col);
void Clr_WavePosi(u16 Col);

#endif
/********************************* END OF FILE ********************************/
