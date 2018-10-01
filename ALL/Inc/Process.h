/******************** (C) COPYRIGHT 2017 e-Design Co.,Ltd. *********************
 File Name : Process.h
 Version   : DS202 Ver 1.0                                         Author : bure
*******************************************************************************/
#ifndef __PROEESS_H
#define __PROEESS_H

#include "STM32F30x.h"
#define   PRE_SMPL    125

//==========+==========+==========+==========+==========+==========+==========+
// Current status | Sampling depth | Output mode | Output range | Output pulse width | File No. | Measurement 1  |
//----------+----------+----------+----------+----------+----------+----------+
enum        {   DPTH,      OUTM,      OUTF,      OUTW,     F_NUM,      VAL1,   
//----------+----------+----------+----------+----------+----------+----------+
//          | Measurement 2  | Window position | Pre-sampled value | Current title | Current label | Edit selection |
//----------+----------+----------+----------+----------+----------+----------+
                 VAL2,      VIEW,     PERS,      TITEL,     LABLE,     SELET,   
//----------+----------+----------+----------+----------+----------+----------+
//          | Synchronization status |      Battery voltage       |      Working current       |
//----------+----------+----------+----------+----------+----------+----------+
                HOLD,  };//     VBAT,     VBAT_,     IBAT,     IBAT_,  
//==========+==========+==========+==========+==========+==========+==========+
  
//==========+==========+==========+==========+==========+==========+==========+
// SYNC Options | Automatic trigger | Normal trigger | Single trigger | Scan trigger | Random trigger | Pause mode |
//----------+----------+----------+----------+----------+----------+----------+
enum        {   AUTO,      NORM,      SNGL,      SCAN,      NONE,  };
//==========+==========+==========+==========+==========+==========+==========+

//==========+==========+==========+==========+==========+==========+==========+
// TRIG Options | Rising trigger | Falling trigger | Greater than trigger | Less than trigger | Narrower than trigger | Wider than trigger |
//----------+----------+----------+----------+----------+----------+----------+
enum        {   RISE,      FALL,       G_T,       L_T,      THIN,      WIDE,  };
//==========+==========+==========+==========+==========+==========+==========+

//==========+==========+==========+==========+==========+==========+==========+
// OUTM Options | Analog output | Pulse output |          |          |          |          |
//----------+----------+----------+----------+----------+----------+----------+
enum        {   ANLOG,     PULSE,                                             };
//==========+==========+==========+==========+==========+==========+==========+

//==========+==========+==========+==========+==========+==========+==========+
// XiGN Options |  Gain=20 |  Gain=10 |  Gain=5  |  Gain=2  |  Gain=1  |          |
//----------+----------+----------+----------+----------+----------+----------+
enum        {    x20,       x10,        x5,        x2,        x1,             };
//==========+==========+==========+==========+==========+==========+==========+

//==========+==========+==========+==========+==========+==========+==========+
// VALx Options | Ai Effective value | Ai Peak-to-peak | Ai Maximum | Ai Minimum | Ai Mean | Ai Median |
//----------+----------+----------+----------+----------+----------+----------+
enum        {   aRMS,      aP_P,      aMIN,      aMAX,      aAVG,      aMID, 
//----------+----------+----------+----------+----------+----------+----------+
//          | Bi Effective value | Bi Peak-to-peak | Bi Maximum | Bi Minimum | Bi Mean | Bi Median |
//----------+----------+----------+----------+----------+----------+----------+
                bRMS,      bP_P,      bMIN,      bMAX,      bAVG,      bMID, 
//----------+----------+----------+----------+----------+----------+----------+
//          |  Potential difference  |  Time difference  |  Frequency  |           |
//----------+----------+----------+----------+----------+----------+----------+
                V1_2,      T1_2,      TWTH,             };
//==========+==========+==========+==========+==========+==========+==========+


extern u8   Norm_Clr, SNGL_Kflag;
extern u16  Smpl[];
extern uc8  GK[];
extern s16  *KpA, *KpB, *KgA, *KgB, Kpg[];
extern u8   Fit_TB;;
extern s32  PmaxA, PmaxB, PminA, PminB, PavgA, PavgB,PrmsA, PrmsB,PvppA,PvppB,PmidA, PmidB;
extern u32  HighA, HighB, LowA,  LowB,  EdgeA, EdgeB,PssqA, PssqB;
extern u8*  V_Buf;
extern u16  T0_PerCnt;
extern u8   FitWait_Cnt ;
extern s16  Trigger_k ,Ch1_Posi, Ch2_Posi,Ch3_Posi, Tri_Posi;

void Process(void);
void Zero_Align(void);
void Slope_Align(u8 Item);
void Set_Base(u8 Base);
void Auto_Fit(void);
void Fit(void);

#endif /*__PROEESS_H*/
/******************************** END OF FILE *********************************/
