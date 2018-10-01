;/******************** (C) COPYRIGHT 2017 e-Design Co.,Ltd. ********************
; File Name : Display.s
; Version   : For DS212 Ver 1.0 With STM32F303VC                  Author : bure
;******************************************************************************/
    RSEG VIEW:CODE(3)
    PRESERVE8

    IMPORT    __SetPosi
    IMPORT    __SendPixels

BACKGROUND    = 0x0000         ;// Background color
GRID_COLOR    = 0x7BEF         ;// Background grid line color
WR            = 0x020          ;// Gpio Pin 5
P_HID         = 0x01
L_HID         = 0x02
D_HID         = 0x01
W_HID         = 0x04
                               ;//    0 ~  409: LCD Col Buffer 
P_TAB         =  410           ;//  410 ~  499: ParamTab 
A_BUF         =  500           ;//  500 ~  799: Wave Track#1 Buffer
B_BUF         =  800           ;//  800 ~ 1099: Wave Track#2_Buffer 
C_BUF         = 1100           ;// 1100 ~ 1399: Wave Track#3_Buffer 
P_BUF         = 1400           ;// 1400 ~ 8191: Pop Buffer


CH_A          = P_TAB+2*0      ;// Wave Track#1 Flag
CH_B          = P_TAB+2*1      ;// Wave Track#2 Flag
CH_C          = P_TAB+2*2      ;// Wave Track#3 Flag
CCHA          = P_TAB+2*18     ;// Wave Track#1 Color
CCHB          = P_TAB+2*19     ;// Wave Track#2 Color
CCHC          = P_TAB+2*20     ;// Wave Track#3 Color
M_X0          = P_TAB+2*30     ;// Starting position of waveform display window X0
M_Y0          = P_TAB+2*31     ;// Starting position of waveform display window Y0
M_WX          = P_TAB+2*32     ;// Horizontal width of waveform display window WX
POPF          = P_TAB+2*33     ;// Pop Flag
PXx1          = P_TAB+2*34     ;// Pop X Position
PWx1          = P_TAB+2*35     ;// Pop Width
PYx2          = P_TAB+2*36     ;// Pop Y Position *2
PHx2          = P_TAB+2*37     ;// Pop Hight *2

    EXPORT  Align00
    EXPORT  Align01
    EXPORT  Align02
    EXPORT  Align03
    EXPORT  Align04
    EXPORT  Align05
    EXPORT  Align06
    EXPORT  Align07
    EXPORT  Align08
    EXPORT  Align09
    EXPORT  Align10
    EXPORT  Align11

;//=============================================================================
;//                  "Window waveforms display" related assembly language subroutines
;//=============================================================================
;// void __DrawWindow(u32 VRAM_Addr)
;//=============================================================================
    EXPORT  __DrawWindow
    NOP.W
    NOP.W
__DrawWindow
    PUSH    {R4-R12,LR}
    NOP.W                      ;// R0: VRAM starting pointer
    LDRH    R1,  [R0, #M_WX]   ;// The horizontal start position of the popup window.
    MOV     R2,  #0            ;// Column count initial value
    LDRH    R11, [R0, #PXx1]   ;// The horizontal start position of the popup window.
    LDRH    R12, [R0, #PWx1]   ;// The horizontal width of the popup window
    ADD     R12, R11, R12      ;// The horizontal end of the popup window
    ADD     R10,  R0,  #P_BUF  ;// Pop-up window buffer pointer initial value

;//----------- Painting background ----------//
Draw_Loop
    CMP     R2,  #0
    ITT     EQ
    BLEQ    Buld_0             ;// Create background data for out-of-row column buffers.
    BEQ     Draw_Wave
    ADDS    R3,  R1, #2        ;// WIDTH+2
    CMP     R2,  R3
    ITT     EQ
    BLEQ    Buld_0             ;// Create background data for out-of-row column buffers.
    BEQ     Draw_Wave

    CMP     R2,  #1
    ITT     EQ
    BLEQ    Buld_1             ;// Create background data for edge column buffers.
    BEQ     Draw_Wave
    ADDS    R3,  R1, #1        ;// WIDTH+1
    CMP     R2,  R3
    ITT     EQ
    BLEQ    Buld_1             ;// Create background data for edge column buffers.
    BEQ     Draw_Wave

    SUBS    R3,  R2, #1
    MOVS    R6,  #25
    UDIV    R5,  R3, R6                
    MULS    R5,  R5, R6
    SUBS    R5,  R3, R5
    ITT     EQ
    BLEQ    Buld_4             ;// Create background data for grid column buffers.
    BEQ     Draw_Wave

    MOVS    R6,  #5
    UDIV    R5,  R3, R6
    MULS    R5,  R5, R6
    SUBS    R5,  R3, R5
    ITT     EQ
    BLEQ    Buld_3             ;// Create background data for the grid column buffer.
    BEQ     Draw_Wave
    BL      Buld_2             ;// Create background data for the geography column buffer.

;//--------- Draw waveform curve --------//
Draw_Wave
    CMP     R2,  #3            ;// From 3~299
    BCC     Horizontal
    CMP     R2,  R1            ;// WIDTH
    BCS     Horizontal

    LDRH    R3,  [R0, #CH_A]   ;// Take the blanking flag of the CH_A waveform.
    TST     R3,  #W_HID
    ITTT    EQ
    MOVEQ   R3,  #CCHA         ;// R3 = CH_A waveform color table offset.
    ADDEQ   R4,  R0, #A_BUF
    BLEQ    Draw_Analog

    LDRH    R3,  [R0, #CH_B]   ;// Take CH_B waveform blanking flag.
    TST     R3,  #W_HID
    ITTT    EQ
    MOVEQ   R3,  #CCHB         ;// R3 = CH_B waveform color table offset.
    ADDEQ   R4,  R0, #B_BUF
    BLEQ    Draw_Analog

    LDRH    R3,  [R0, #CH_C]   ;// Take CH_C waveform blanking flag.
    TST     R3,  #W_HID
    ITTT    EQ
    MOVEQ   R3,  #CCHC         ;// R3 = CH_C waveform color table offset.
    ADDEQ   R4,  R0, #C_BUF
    BLEQ    Draw_Analog

;//------- Draw horizontal direction cursor ------//
Horizontal
    CMP     R2,  #0
    ITT     EQ
    BLEQ    Cursor_0           ;// Draw the cursor endpoint on the outer edge of the column.
    BEQ     Vertical
    ADDS    R3,  R1, #2        ;// WIDTH+2
    CMP     R2,  R3             
    ITT     EQ
    BLEQ    Cursor_0           ;// Draw the cursor endpoint on the outer edge of the column.
    BEQ     Vertical

    CMP     R2,  #1
    ITT     EQ
    BLEQ    Cursor_1           ;// Draw edge column cursor endpoints.
    BEQ     Vertical
    ADDS    R3,  R1, #1        ;// WIDTH+1
    CMP     R2,  R3             
    ITT     EQ
    BLEQ    Cursor_1           ;// Draw edge column cursor endpoints.
    BEQ     Vertical

    CMP     R2,  #2
    ITT     EQ
    BLEQ    Cursor_2           ;// Draw cursor endpoints on inner edge columns.
    BEQ     Vertical
    CMP     R2,  R1            ;// WIDTH 
    IT      EQ
    BLEQ    Cursor_2           ;// Draw cursor endpoints on inner edge columns.
    BEQ     Vertical
    BL      Cursor_3           ;// Cursor lines for the remaining columns.

;//------- Draw vertical cursors ------//
Vertical
    BL      Cursor_4

;//--------- Draw a pop-up window --------//

    LDRH    R3,  [R0, #POPF]   ;// Take the pop-up window blanking flag.
    TST     R3,  #P_HID
    BNE     Send
    CMP     R2,  R11           ;// Conditional pop-up window column processing begins.
    BLT     Send
    CMP     R2,  R12           ;// Conditional pop-up window column processing ends.
    IT      LT
    BLLT    Draw_Pop           ;// Column count in pop-up window.

;//--------- Display column data --------//
Send
    BL      Send_LCD           ;// Transfer a column of data from the buffer to LCD.
    ADDS    R3,  R1, #2        ;// WIDTH+2
    CMP     R2,  R3             
    ITT     NE
    ADDNE   R2,  R2, #1
    BNE     Draw_Loop          ;// Process the next column of display data.

    POP     {R4-R12,PC}
;//=============================================================================
; Draw_Analog(R2:Col, R3:ColorNum, R4:pDat)   Draw analog waveform curve  Used: R3-R7
;//=============================================================================
    NOP.W
    NOP.W
    NOP
Draw_Analog
    ADD     R4,  R4, R2
    LDRB    R5,  [R4]          ;// Take the current column waveform data, n1.
    LDRB    R4,  [R4, #-1]     ;// Take the previous column waveform data, n0
Analog0
    CMP     R4,  #199          ;// Truncate at the top R4 >= 200
    IT      HI
    BXHI    LR
    CMP     R4,  #0            ;// Truncate at the bottom R4 = 0
    IT      EQ
    BXEQ    LR

    CMP     R5,  R4
    ITTEE   CS                 ;// R5 = | n1 - n0 |
    MOVCS   R6,  R4
    SUBCS   R5,  R5, R4
    MOVCC   R6,  R5            ;// Put n1, n0 minimum result into R6.
    SUBCC   R5,  R4, R5

    CMP     R6,  #198          ;// Blanking if starting point is beyond the upper boundary (R6 > 198).
    IT      HI
    BXHI    LR
    ADDS    R4,  R5, R6
    CMP     R4,  #198          ;// Limiting if end point is beyond the upper boundary (R4 > 198).
    IT      HI
    RSBHI   R5,  R6, #198
    BGT     Analog2

    CMP     R4,  #1            ;// Blanking if end point is beyond the low boundary (R4 <= 1).
    IT      LS
    BXLS    LR
    CMP     R6,  #2            ;// Limiting if starting point is beyond the low boundary (R6 <= 2).
    ITTT    LS
    MOVLS   R6,  #2
    SUBLS   R5,  R4, #2
    BLS     Analog2

    CMP     R5,  #0            ;// Horizontal line bold
    ITT     EQ
    SUBEQ   R6,  R6, #1
    ADDEQ   R5,  R5, #2

Analog2
    CMP     R5,  #20           ;// Choose the color.
    IT      GE
    ADDGE   R3,  R3, #18       ;// Select low brightness color group.
    LDRH    R3,  [R0, R3]

    LSL     R6,  R6, #1
    ADD     R6,  R0, R6        ;// Determine the display position.
Align00    
Analog3
    STRH    R3,  [R6], #2      ;// Draw waveform points.
    SUBS    R5,  R5, #1
    BCS     Analog3
    BX      LR
;//=============================================================================
; Cursor_4(R1:pTab, R2:Col)// Draw the cursor endpoint on the outer edge of the column  Used: R3-R8
;//=============================================================================
    NOP.W
Cursor_4
    MOVS    R3,  #P_TAB+6*2    ;// 6-8 items are vertical cursors.
Cursor40
    MOV     R4,  R0
    LDRH    R5,  [R0, R3]      ;// Take the cursor variable blanking flag.
    TST     R5,  #D_HID
    BNE     Cursor49           ;// Is cursor blanking?
Cursor41
    ADDS    R3,  R3, #9*2
    LDRH    R5,  [R0, R3]      ;// Take the position of cursor variable.
    ADDS    R3,  R3, #9*2
    LDRH    R6,  [R0, R3]      ;// Take the display color of cursor variable.
    SUBS    R3,  R3, #18*2     ;// Restore variable table pointer.

    SUBS    R8,  R5, #2
    CMP     R2,  R8            ;// Draw left outer edge cursor endpoint.
    BNE     Cursor42
    STRH    R6,  [R4]          ;// Draw left lower edge.
    ADDS    R4,  R4, #404
    STRH    R6,  [R4]          ;// Draw left outer edge.
    B       Cursor49
Cursor42
    ADDS    R8,  R8, #1
    CMP     R2,  R8            ;// Draw left line cursor endpoint.
    BNE     Cursor43
    STRH    R6,  [R4], #2      ;// Draw left lower edge.
    STRH    R6,  [R4]          ;// Draw the lower left line.
    ADDS    R4,  R4, #400
    STRH    R6,  [R4], #2      ;// Draw the top left line.
    STRH    R6,  [R4]          ;// Draw left outer edge.
    B       Cursor49
Cursor43
    ADDS    R8,  R8, #1
    CMP     R2,  R8            ;// Draw cursor vertices and cursor lines.
    BNE     Cursor45
    STRH    R6,  [R4], #2      ;// Draw along the outer line.
    STRH    R6,  [R4], #2      ;// Draw lower edge.
    STRH    R6,  [R4]          ;// Draw inner line.
    ADDS    R4,  R4, #396
    STRH    R6,  [R4], #2      ;// Draw on the inside.
    STRH    R6,  [R4], #2      ;// Draw line.
    STRH    R6,  [R4]          ;// Draw along the line.

    LDRH    R5,  [R0, R3]      ;// Take the cursor variable blanking flag.
    TST     R5,  #2
    BNE     Cursor45           ;// Is the cursor line blanking?
    MOVS    R4,  R0
    ADDS    R7,  R4, #400
Align01    
Cursor44
    STRH    R6,  [R4], #8      ;// Draw cursor line.
    CMP     R7,  R4
    BCS     Cursor44
    B       Cursor49
Cursor45
    ADDS    R8,  R8, #1
    CMP     R2,  R8            ;// Draw the right line cursor end point.
    BNE     Cursor46
    STRH    R6,  [R4], #2      ;// Draw right lower edge.
    STRH    R6,  [R4]          ;// Draw the lower right line.
    ADDS    R4,  R4, #400
    STRH    R6,  [R4], #2      ;// Draw upper right line.
    STRH    R6,  [R4]          ;// Draw upper right edge.
    B       Cursor49
Cursor46
    ADDS    R8,  R8, #1
    CMP     R2,  R8            ;// Draw right outer edge cursor end point.
    BNE     Cursor49
    STRH    R6,  [R4]          ;// Draw right lower edge.
    ADDS    R4,  R4, #404
    STRH    R6,  [R4]          ;// Draw upper right edge.
Cursor49
    ADDS    R3,  R3, #1*2
    CMP     R3,  #P_TAB+9*2    ;//10
    BNE     Cursor40
    BX      LR
;//=============================================================================
; Cursor_3(R1:pTab, R2:Col)// Draw the remaining cursor lines  Used: R3-R6
;//=============================================================================
    NOP
Align02    
Cursor_3
    MOVS    R3,  #P_TAB+5*2    ;// 0-5 items are horizontal cursors.
    MOVS    R4,  R0
Cursor31
    LDRH    R5,  [R0, R3]      ;// Take the cursor variable blanking flag.
    TST     R5,  #L_HID
    BNE     Cursor32           ;// Is the cursor line blanking?
    SUBS    R5,  R2, #1
    ANDS    R5,  R5, #3
    BNE     Cursor32           ;// Loop if cursor line is the dotted line position.
    ADDS    R3,  R3, #9*2
    LDRH    R5,  [R0, R3]      ;// Take the display position of the cursor variable.
    ADDS    R4,  R0, R5
    ADDS    R3,  R3, #9*2
    LDRH    R6,  [R0, R3]      ;// Take the display color of this cursor variable.
    STRH    R6,  [R4]          ;// Draw cursor line.
    SUBS    R3,  R3, #18*2     ;// Restore variable table pointer.
Cursor32
    SUBS    R3,  R3, #1*2
    CMP     R3,  #P_TAB     
    BPL     Cursor31           ;// Process the next cursor endpoint.
    BX      LR
;//=============================================================================
; Cursor_0(R1:pTab, R2:Col)// Draw outer edge column cursor endpoints  Used: R3-R6
;//=============================================================================
    NOP
Align03
Cursor_0
    MOVS    R3,  #P_TAB+5*2    ;// 0-5 items are horizontal cursors.
    MOVS    R4,  R0
Cursor01
    LDRH    R5,  [R0, R3]      ;// Take the cursor variable blanking flag.
    TST     R5,  #1
    BNE     Cursor02           ;// Is cursor vertex blanking?
    ADDS    R3,  R3, #9*2
    LDRH    R5,  [R0, R3]      ;// Take the display position of the cursor variable.
    ADDS    R4,  R0, R5
    ADDS    R3,  R3, #9*2
    LDRH    R6,  [R0, R3]      ;// Take the display color of this cursor variable.
    SUB     R4,  R4, #4
    STRH    R6,  [R4], #2
    STRH    R6,  [R4], #2
    STRH    R6,  [R4], #2      ;// Draw outer edge column cursor endpoints.
    STRH    R6,  [R4], #2
    STRH    R6,  [R4], #2
    SUBS    R3,  R3, #18*2     ;// Restore variable table pointer.
Cursor02
    SUBS    R3,  R3, #1*2
    CMP     R3,  #P_TAB       
    BPL     Cursor01           ;// Process the next cursor endpoint.
    BX      LR
;//=============================================================================
; Cursor_1(R1:pTab, R2:Col)// Draw edge column cursor endpoints  Used: R3-R6
;//=============================================================================
    NOP.W
Align04
Cursor_1
    MOVS    R3,  #P_TAB+5*2    ;// 0-5 items are horizontal cursors.
    MOVS    R4,  R0
Cursor11
    LDRH    R5,  [R0, R3]      ;// Take the cursor variable blanking flag.
    TST     R5,  #1
    BNE     Cursor12           ;// Is cursor vertex blanking?
    ADDS    R3,  R3, #9*2
    LDRH    R5,  [R0, R3]      ;// Take the display position of the cursor variable.
    ADDS    R4,  R0, R5
    ADDS    R3,  R3, #9*2
    LDRH    R6,  [R0, R3]      ;// Take the display color of this cursor variable.
    SUBS    R4,  R4, #2
    STRH    R6,  [R4], #2
    STRH    R6,  [R4], #2      ;// Draw edge column cursor endpoints/
    STRH    R6,  [R4], #2
    SUBS    R3,  R3, #18*2     ;// Restore variable table pointer.
Cursor12
    SUBS    R3,  R3, #1*2
    CMP     R3,  #P_TAB     
    BPL     Cursor11           ;// Process the next cursor endpoint.
    BX      LR
;//=============================================================================
; Cursor_2(R1:pTab, R2:Col)// Draw inner edge column cursor endpoints  Used: R3-R6
;//=============================================================================
    NOP.W
    NOP.W
    NOP.W
    NOP
Align05
Cursor_2
    MOVS    R3,  #P_TAB+5*2    ;// 0-5 items are horizontal cursors.
    MOVS    R4,  R0
Cursor21
    LDRH    R5,  [R0, R3]      ;// Take the cursor variable blanking flag.
    TST     R5,  #1
    BNE     Cursor22           ;// Is cursor vertex blanking?
    ADDS    R3,  R3, #9*2
    LDRH    R5,  [R0, R3]      ;// Take the display position of the cursor variable.
    ADDS    R4,  R0, R5
    ADDS    R3,  R3, #9*2
    LDRH    R6,  [R0, R3]      ;// Take the display color of this cursor variable.
    STRH    R6,  [R4]          ;// Draw inner edge column cursor endpoints.
    SUBS    R3,  R3, #18*2     ;// Restore variable table pointer.
Cursor22
    SUBS    R3,  R3, #1*2
    CMP     R3,  #P_TAB   
    BPL     Cursor21           ;// Process the next cursor endpoint.
    BX      LR
;//=============================================================================
;// R0:pDat, R1:pTab, R2:Col, R3:Var, R4:pBuf, R5:Cnt, R6:Tmp,
;//=============================================================================
; void Fill_Base(R3 = u32 Color)// Fill the background colour of column buffer RET: R4=R0+2   Used: R3-R5
;//=============================================================================
    NOP
    EXPORT  Fill_Base
Fill_Base
    MOV.W   R4,  R0
    MOV.W   R5,  #102          ;// 1 + 202/2 lines of 404 bytes.
Align06    
Fill_Loop_0
    STR     R3,  [R4], #4      ;// Pointer plus 4 after transfer is completed.
    SUBS    R5,  #1
    BNE     Fill_Loop_0
    ADD     R4,  R0, #2        ;// Pointer alignment.
    MOV     R3,  #GRID_COLOR   ;// Preloaded grid color values.
    BX      LR
;//=============================================================================
; Draw_Pop(R0:pWork) // Draw a pop-up window                                  Used: R3-R8
;//=============================================================================
    NOP.W
    NOP.W
    NOP.W
    NOP
Align07
Draw_Pop
    LDRH    R5,  [R0, #PYx2]   ;// Take the vertical start position of the pop-up window.
    ADDS    R5,   R0,  R5
    LDRH    R6,  [R0, #PHx2]   ;// Take the vertical height of the pop-up window.
    ADD     R7,   R0, #PHx2    ;// HYx2+2 is a storage color table pointer, CPTR.
Pop_Loop
    LDRH    R4,  [R10], #2     ;// Take Pop data (2 bytes, 4 points in total).
    ANDS    R3,  R4,  #0x0E
    ITT     NE                 ;// Skipped if transparent color.
    LDRHNE  R3,  [R7, R3]      ;// Check the table to take 1-7 colors.
    STRHNE  R3,  [R5]          ;// Picture 1.
    ADDS    R5,  R5,  #2
    LSR     R4,  R4,  #4
    ANDS    R3,  R4,  #0x0E
    ITT     NE                 ;// Skipped if transparent color.
    LDRHNE  R3,  [R7, R3]      ;// Check the table to take 1-7 colors.
    STRHNE  R3,  [R5]          ;// Picture 2.
    ADDS    R5,  R5,  #2
    LSR     R4,  R4,  #4
    ANDS    R3,  R4,  #0x0E
    ITT     NE                 ;// Skipped if transparent color.
    LDRHNE  R3,  [R7, R3]      ;// Check the table to take 1-7 colors.
    STRHNE  R3,  [R5]          ;// Picture 3.
    ADDS    R5,  R5,  #2
    LSR     R4,  R4,  #4
    ANDS    R3,  R4,  #0x0E
    ITT     NE                 ;// Skipped if transparent color.
    LDRHNE  R3,  [R7, R3]      ;// Check the table to take 1-7 colors.
    STRHNE  R3,  [R5]          ;// Picture 4.
    ADDS    R5,  R5, #2
    SUBS    R6,  R6, #8
    BNE     Pop_Loop
    BX      LR                 ;// Complete the current column display.
    NOP
;//=============================================================================
; void Buld_0(R4 = u16* pCol)   // Create background data for out-of-row column buffers Used: R3-R5
;//=============================================================================
Buld_0
    MOV     R3,  #BACKGROUND   ;// background color
    B       Fill_Base
;//=============================================================================
; void Buld_2(R4 = u16* pCol)   // Create background data for grid inner column buffer Used: R3-R6
;//=============================================================================
Buld_2
    MOV     R6,  LR
    MOV     R3,  #BACKGROUND   ;// background color
    BL      Fill_Base
    STRH    R3,  [R4, #400]    ;// upper line
    STRH    R3,  [R4]          ;// lower line
    BX      R6
;//=============================================================================
; void Buld_4(R4 = u16* pCol)   // Create background data for grid column buffers
;//=============================================================================
    NOP.W
    NOP
Buld_4
    MOV     R6,  LR
    MOV     R3,  #BACKGROUND   ;// background color
    BL      Fill_Base
    MOVS    R5,  #41           ;// 41 points  P_TAB
Align08    
Loop7
    STRH    R3, [R4], #5*2     ;// Draw 1 grid point for every 5 lines.
    SUBS    R5,  R5,  #1
    BNE     Loop7
    BX      R6
;//=============================================================================
; void Buld_3(R4 = u16* pCol)   // Create background data for the grid column buffer Used: R3-R6
;//=============================================================================
    NOP.W
    NOP.W
    NOP
Buld_3
    MOV     R6,  LR
    MOV     R3,  #BACKGROUND   ;// background color
    BL      Fill_Base
    MOVS    R5,  #9            ;// 9 grid points
Align09    
Loop3
    STRH    R3, [R4], #50      ;// Draw 1 grid point for every 25 lines.
    SUBS    R5,  R5,  #1
    BNE     Loop3
    BX      R6
;//=============================================================================
; void Buld_1(R4 = u16* pCol)   // Create background data for edge column buffers Used: R3-R6
;//=============================================================================
Buld_1
    MOV     R6,  LR
    MOV.W   R3,  #GRID_COLOR
    MOVT    R3,  #GRID_COLOR   ;// To improve transmission efficiency, take 32bit color
    BL      Fill_Base          ;// RET: R4=R0+2
    MOV     R3,  #BACKGROUND   ;// background color
    STRH    R3,  [R4, #402]            
    STRH    R3,  [R4, #-2]     ;// Leave blank on the bottom line.
    BX      R6
;//=============================================================================
; void __Mem32Fill(u32* pMem, u32 Data, u32 n)
;//=============================================================================
    NOP.W
    NOP.W
    NOP
    EXPORT  __Mem32Fill
Align10    
__Mem32Fill
    STR     R1, [R0], #4
    SUBS    R2, R2, #1  
    BNE     __Mem32Fill  
    BX      LR         
;//=============================================================================
; void Send_LCD(u16* pBuf, u16 Row) // Send a column of buffer data to LCD     Used: R3-R8
;//=============================================================================
    NOP.W
    NOP
    EXPORT  Send_LCD
Send_LCD
    MOVS    R5,  R0
    PUSH    {R0-R3, LR}
    LDRH    R1, [R0, #M_Y0]
    LDRH    R0, [R0, #M_X0]
    ADDS    R0,  R0, R2
    BL      __SetPosi
    MOVS    R0, #0x0C00
    MOVT    R0, #0x4800        ;// Port D Base 0x48000C00
    MOVS    R1, #WR 
    MOVS    R2, #203           ;// 1+202
Align11    
Loop9    
    LDRH    R3, [R5], #2
    STRH    R3, [R0,  #0x414]  ;// Port E ODR
    STRH    R1, [R0,  #0x028]  ;// Port D BRR
    SUBS    R2,  R2,  #1  
    STRH    R1, [R0,  #0x018]  ;// Port D BSRR
    BNE     Loop9
    POP     {R0-R3, PC}
;//=============================================================================
   END

;******************************* END OF FILE ***********************************


