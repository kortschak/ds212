/********************* (C) COPYRIGHT 2017 e-Design Co.,Ltd. ********************
File Name : main.c
Version   : DS212                                                 Author : bure
*******************************************************************************/
#include "Version.h"
#include "Process.h"
#include "Drive.h"
#include "Func.h"
#include "Draw.h"
#include "Bios.h"
#include "Menu.h"
#include "Disk.h"
#include "LCD.h"
#include "FAT12.h"
#include "File.h"
#include "Math.h"

/*******************************************************************************


*******************************************************************************/

typedef void (*pFunc)(void);
void MSD_Disk_Config(void);
void Set_Licence(u16 x, u16 y);

//===============================APP version number======================================
u8  APP_VERSION[] = "V1.03";   // Must not exceed 12 characters.

u16 Key_Flag = 0;
u8  CalPop_Flag = 1, ResPop_Flag = 1;
u8  Menu_Temp[5], NumStr[20];
u16 FileInfo,     Label_Cnt;
u16 temp = 0;
u8  Channel = 0;

void main(void)
{
  //===============================System initialization===================================
  __Ctrl(SYS_CFG, RCC_DEV | TIM_DEV | GPIO_OPA | ADC_DAC | SPI );

 GPIO_SWD_NormalMode() ;  // Turn off SWD burner function

#if   defined (APP1)
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x8000);      // First Partition APP
#elif defined (APP2)
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x20000);     // Second partition APP
#endif
  SysTick_Config(SystemCoreClock/1000);                 // SysTick = 1mS
  __Ctrl(B_LIGHT, 50);                                  // Turn on LCD backlight 50%.

  __Ctrl(BUZZVOL, 50);                                  // Set the buzzer volume (0~100%).
  Beep(200);                                            // 200ms

  USB_MSD_Config();
  Init_Fat_Value();
  __Ctrl(SMPL_ST, DISABLE);
  __Ctrl(SMPL_ST, SIMULTANEO);
  __Ctrl(SMPLBUF, (u32)Smpl);
  __Ctrl(SMPLNUM, DEPTH[PopMenu1_Value[WIN_Depth]]);
  __Ctrl(SMPL_ST, ENABLE);

  //=============================Display boot prompt information page===========================
  SetColor(BLK, WHT);
  DispStr(0,      90, PRN, "                                        ");
  DispStr(0,      70, PRN, "                                        ");
#if   defined (APP1)
  DispStr(8,      90, PRN, "       Oscilloscope  APP1");
#elif defined (APP2)
  DispStr(8,      90, PRN, "       Oscilloscope  APP2");
#endif
  DispStr(8+26*8, 90, PRN,                            APP_VERSION);
  DispStr(8,      70, PRN, "        System Initializing...       ");
  __Ctrl(DELAYmS, 500);

  ADC_StartConversion(ADC1);
  ADC_StartConversion(ADC3);
  ADC_StartConversion(ADC2);
  ADC_StartConversion(ADC4);
  memset(VRAM_PTR+TR1_pBUF, ~0,    900);
  memcpy(VRAM_PTR+TAB_PTR,  PARAM,  90);

  //=============================First write firmware auto-calibration===========================
  Read_CalFlag();
  /**/
  if(Cal_Flag == 1){
    Cal_Flag = 0;
    SetColor(BLK, WHT);
    DispStr(8, 90, PRN, "                                        ");
    DispStr(8, 70, PRN, "                                        ");
    DispStr(8, 90, PRN, "      Run the calibration program...    ");
    DispStr(8, 70, PRN, "        Please wait a few seconds       ");
    Zero_Align();                              // Vertical displacement zero correction.
    Restore_OrigVal();                         // Reset parameters.
    Save_Param();                              // Save parameters.
    Save_Kpg();
  }

  //=============================Main interface display=====================================
  Read_Kpg();                                   // Read calibration parameters.
  Load_Param();                                 // Read saved parameters from U disk.
  File_Num();                                   // Read file number.
  ClrScrn(DAR);                                 // Background clear screen.
  menu.menu_flag = 1;                           // Main menu
  menu.mflag  |= UPD;                           // Menu options
  menu.iflag  |= UPD;                           // Sub-menu
  Label_Flag  |= UPD;                           // Cursor
  Show_Title();                                 // Display
  Show_Menu();                                  // Main menu and options.
  Update_Proc_All();                            // Refresh.
  Update_Label();                               // Cursor display.
  Print_dT_Info(INV);
  Print_dV_Info(INV);
  Battery_Show();
  MenuCnt = 5000;                               // Main menu first standby time.
  PD_Cnt      = PopMenu3_Value[SYS_Standy]*Unit;   // Screen standby time.
  AutoPwr_Cnt = PopMenu3_Value[SYS_PowerOff]*Unit; // Automatic shutdown time.
  if(PopMenu1_Value[TRI_Fit])Key_S_Time = 300;
  else                       Key_S_Time = 0;

  //========================Check the Licence is correct then close the DEMO window=======================
  if(__Info(LIC_OK) == 1){
    PopType &= ~DEMO_POP;
    ClosePop();
  }
  else Demo_Pop();

  Keys_Detect();                                // Debounce.
  KeyIn=0;

  //===================================Main cycle program===============================
  while(1){
    //=================Reading ADC data=====================

    if(!__Info(LIC_OK)){// Shutdown test KEY_R KEY_S.
      if(((~GPIOB->IDR)& 0x0020)&&((~GPIOB->IDR)& 0x0040))
        __Ctrl(PWROFF, ENABLE);
    }
    //====================Standby=======================
    if(((PopMenu3_Value[SYS_Standy]!=0) && (PD_Cnt == 0))){
      __Ctrl(B_LIGHT,1);                   //Turn off the backlight
      StdBy_Flag = 1;
    }
    //==================Automatic shut-down=====================
    if((PopMenu3_Value[SYS_PowerOff] != 0) && (AutoPwr_Cnt == 0) && (__Info(P_VUSB) == 0)){
      Beep(500);
      __Ctrl(DELAYmS, 500);                // 0.5s buzzer sounds before shutting down.
      __Ctrl(PWROFF,  ENABLE);              // Power off.
    }
    else if(__Info(P_VUSB))                 // Do not automatic shut-down when USB is charging
      AutoPwr_Cnt = PopMenu3_Value[SYS_PowerOff]*Unit;

    //if(menu.menu_flag == 1)Show_Menu();
    if((PopType & DEMO_POP)&&!(PopType&(DAILOG_POP|PWR_POP|LIST_POP|FILE_POP)))
      MovePop();                           // Not unlocked so show the Demo window.
    if(About_Flag == 0){                   // Do not refresh the waveform window when About is displayed.
      Process();
      __DrawWindow(VRAM_PTR);
    }

    Keys_Detect();                         // Key scan.

    if(KeyIn) Key_Flag = 1;
    else      Key_Flag = 0;

    if(About_Flag == 1){                   // When displaying About, only the "M" key and screenshots are valid.
      Key_Flag = 0;
      if((KeyIn == K_M)||(KeyIn == R_HOLD))
        Key_Flag = 1;

    }

    if(StdBy_Flag == 1){
      if(KeyIn)
        Key_Flag = 1;
      else
        Key_Flag = 0;
    }

    if(KeyIn && Key_Flag){

      //==========There are keys to reset standby and automatic shutdown time===============
      if(((PopMenu3_Value[SYS_Standy] != 0) &&(PD_Cnt == 0)) || (StdBy_Key == 1)){
        __Ctrl(B_LIGHT, PopMenu3_Value[SYS_BKLight]*10);
        StdBy_Flag  = 0;
        StdBy_Key   = 0;
        KeyIn = 0;
      }
      PD_Cnt      = PopMenu3_Value[SYS_Standy]*Unit;   // Standby time
      AutoPwr_Cnt = PopMenu3_Value[SYS_PowerOff]*Unit; // Automatic shutdown time

      //=======================Key operation=====================
      switch (KeyIn){
        //-------------Mechanical keys----------------

      case R_HOLD:                       // Long press RUN button, screenshot shortcut button.
        Beep(50);
        __Ctrl(DELAYmS, 100);
        Beep(50);
        FileInfo = Save_Bmp(PopMenu3_Value[SAVE_Bmp]);
        DispFileInfo(FileInfo);

        SetColor(DAR, ORN);
        Print_dT_Info(INV);                // Display T1-T2.
        Update[T1F] &=~ UPD ;
        break;

      case M_HOLD:                         // Hold
        if(menu.menu_flag == 1)
        {
          if(PopType & (FILE_POP)){
            ClosePop();
            menu.current = Menu_Temp[0];
            menu.menu_index[menu.current] = Menu_Temp[1];
            break;
          }
          else if(!(PopType & ( DAILOG_POP))){
            Menu_Temp[0] = menu.current;
            Menu_Temp[1] = menu.menu_index[menu.current];

            ClosePop();
            PopCnt = POP_TIME;              // Set pop-up auto off timer 5000ms
            menu.current = Option;
            menu.menu_index[menu.current] = 0;
            Cur_PopItem = 1;
            Show_PopMenu(Cur_PopItem);      // Pop up submenu _Pop.
            List_Pop();
          }
        }
        break;

      case S_HOLD:
        if(!(PopType & (LIST_POP|DAILOG_POP|FILE_POP))){

          if(menu.menu_flag){                          // Hide menu window.
            MenuCnt = 0;
            menu.menu_flag = 0;
            ParamTab[M_WX] = 300;
            Clear_Label_R(DAR);                        // Erase right column.
          }
          else {                                       // Pop up window.
            if(__Info(LIC_OK) == 1);                   // Demo resets when switching windows.
            else if((ParamTab[PXx1]+ParamTab[PWx1]) >= (WIDTH_MINI+1))Demo_Pop();
            ParamTab[M_WX] = WIDTH_MINI;
            menu.menu_flag = 1;
            Show_Menu();
            menu.mflag |= UPD;
          }
        }
        break;

      case K_RUN:                     // RUN key.
        {                             // Do pause, run.
          if(Status == STOP) {
            Status &= ~STOP;
            if(PopMenu1_Value[TRI_Sync] == SNGL)ADC_Start();
            if(PopMenu1_Value[TRI_Sync] == NORM)ADC_Start();
            Norm_Clr = 1;
            SNGL_Kflag = 1;
            Update_Proc_All();
          }
          else  {
            Status  |=  STOP;
            Ch1_Posi = PopMenu1_Value[CH1_Posi];
            Ch2_Posi = PopMenu1_Value[CH2_Posi];
            Ch3_Posi = PopMenu1_Value[CH3_Posi];
          }
          Update_Status();
        }

        break;

      case K_S:
        if((PopType & PWR_POP)){          // Shutdown window.
        }
        else if((PopType & FILE_POP)&&(menu.current == Option)  // File submenu window.
                &&(menu.menu_index[menu.current] == 0)){
                  FileInfo = 1;
                  if(Cur_PopItem == SAVE_PAM) {
                    menu.current = Menu_Temp[0];
                    menu.menu_index[menu.current] = Menu_Temp[1];
                    Save_Param();
                    FileInfo = 0;
                    DispFileInfo(FileInfo);
                    menu.current = Option;
                    menu.menu_index[menu.current] = 0;
                    Show_PopMenu(Cur_PopItem);
                    break;
                  }
                  else if(Cur_PopItem == SAVE_BMP) {
                    ClosePop();
                    __DrawWindow(VRAM_PTR);
                    FileInfo = Save_Bmp(PopMenu3_Value[SAVE_Bmp]);
                    List_Pop();
                  }
                  else if(Cur_PopItem == SAVE_DAT) {
                    FileInfo = Save_Dat(PopMenu3_Value[SAVE_Dat]);
                  }
                  else if(Cur_PopItem == SAVE_BUF) {
                    FileInfo = Save_Buf(PopMenu3_Value[SAVE_Buf]);
                  }
                  else if(Cur_PopItem == SAVE_CSV) {
                    FileInfo = Save_Csv(PopMenu3_Value[SAVE_Csv]);
                  }
                  else if(Cur_PopItem == LOAD_DAT) {
                    FileInfo = Load_Dat(PopMenu3_Value[LOAD_Dat]);
                  }
                  else if(Cur_PopItem == LOAD_BUT) {
                    FileInfo = Load_Buf(PopMenu3_Value[LOAD_Buf]);
                  }
                  else if(Cur_PopItem == SAVE_SVG) {
                    FileInfo = Save_Svg(PopMenu3_Value[SAVE_Svg]);
                    menu.current = Option;              // Pop up File window.
                    menu.menu_index[menu.current] = 0;
                    Show_PopMenu(Cur_PopItem);
                    List_Pop();
                  }

                  Show_PopMenu(Cur_PopItem);
                  Show_Title();
                  DispFileInfo(FileInfo);
                  if(PopType & DAILOG_POP)  ClosePop();
                  break;
                }

        else if((menu.current == Option) && (menu.menu_index[menu.current] == 3)
                && (PopType & (LIST_POP |DAILOG_POP))){   // CAL calibration options.
                  if(Cur_PopItem == CAL_ZERO) {
                    if(CalPop_Flag == 1){
                      Dialog_Pop("Auto Calibration?");
                      PopCnt = POP_TIME;
                      CalPop_Flag = 0;
                      break;
                    }
                    if(CalPop_Flag == 0){
                      if(PopType & DAILOG_POP){          // DAILOG_POP dialog selection.
                        Save_Kpg();
                        ClosePop();
                        CalPop_Flag = 1;
                      }
                      else if(PopType & LIST_POP){       // MENU_POP dialog selection.
                        if(Cur_PopItem == CAL_ZERO) {
                          ClosePop();
                          Tips_Pop("Waiting for Calibration ...");
                          __DrawWindow(VRAM_PTR);        // Refresh the interface.
                          Zero_Align();
                          Update_Proc_All();
                          ClosePop();
                          Dialog_CalPop("Cal completed,Save data?",48,110,32,26*6);
                          PopCnt = POP_TIME;
                        }
                      }
                    }
                  }
                  else if(Cur_PopItem == RES_DATA) {
                    if(ResPop_Flag ==1){
                      Dialog_Pop("  Restore Data ?");
                      PopCnt = POP_TIME;
                      ResPop_Flag = 0;
                      break;
                    }
                    if(ResPop_Flag == 0){
                      if(PopType & DAILOG_POP){
                        menu.current = Oscillo;
                        menu.menu_index[Oscillo] = 0;
                        Save_Param();
                        ClosePop();
                        menu.mflag |= UPD;               // Menu options.
                        Show_Menu();                     // Main menu and options.
                        ResPop_Flag = 1;
                      }
                      else if(PopType & LIST_POP){
                        Restore_OrigVal();
                        menu.current = Option;
                        menu.menu_index[Option] = 3;
                        Show_Title();                   // Diplay.
                        Show_Menu();                    // Main menu and options.
                        Update_Proc_All();              // Refresh.
                        ClosePop();
                        Dialog_CalPop(" Restored,Save Setting?", 90, 50, 32, 26*6);
                        PopCnt = POP_TIME;
                      }
                    }
                  }
                  break;
                }
        else if(!(PopType & LIST_POP)){
          if((menu.menu_flag == 1)){
            ParamTab[M_WX] = 251;
            //if(menu.menu_flag == 1){
              if(menu.current >= MENU_MAX)menu.current = 0;
              else menu.current++;
            //}
            //menu.menu_flag = 1;
            Show_Menu();
            menu.mflag |= UPD;
            MenuCnt = 6000;
          }
          else{
            PopMenu1_Value[TRI_Ch] = !PopMenu1_Value[TRI_Ch] ;
            Update[VTF] |= UPD;
            Label_Flag |= UPD;
          }
        }


        break;

      case KEY_DOUBLE_M:
        Beep(50);
        __Ctrl(DELAYmS, 100);
        Beep(50);
        if(PopMenu1_Value[TRI_Fit]){
          Auto_Fit();
          menu.iflag |= UPD;
        }
        break;

      case K_M:

        if(menu.menu_flag){
          if(PopType & PWR_POP){           // In the shutdown window, press the power button to close the window.
            PopType &= ~PWR_POP;
            ClosePop();
          }

          else if(!(PopType & (LIST_POP|DAILOG_POP|FILE_POP))){
            // Open child window when there is no child window.
            PopCnt = POP_TIME;             // Set pop-up auto off timer 5000ms
            Cur_PopItem = 1;               // Sub-window default option is first option.
            Show_PopMenu(Cur_PopItem);     // Popup submenu _Pop
            if(PopType & FILE_POP){        // If the file management sub-window pops up, record the current page and current options.
              Menu_Temp[0] = menu.current;
              Menu_Temp[1] = menu.menu_index[menu.current];
            }
            if((menu.menu_index[menu.current] != 5)||(menu.current == 0))
              List_Pop();                  //Battery voltage and About do not pop up.
          }

          else if(PopType & (LIST_POP|DAILOG_POP|FILE_POP)){
            // Close child window when there are child windows.
            if(PopType & FILE_POP){        // File Management sub-window, restores the current page and options when opened.
              menu.current = Menu_Temp[0];
              menu.menu_index[menu.current] = Menu_Temp[1];
            }
            ClosePop();
            CalPop_Flag  = 1;             // Auto_Cal?
            ResPop_Flag  = 1;             // Restore?
            Windows_Flag = 0;             // Close windows.
            Update_Windows();
          }
        }
        else{
          if(Channel == CH1_Vol){
            Channel = CH2_Vol;
            CHA_Col = DAR;
            CHB_Col = RED_;
          }
          else{
            Channel = CH1_Vol;
            CHA_Col = RED_;
            CHB_Col = DAR;
          }
        }
        break;

      case K_UP:
        if(menu.menu_flag == 0){
          if(PopMenu1_Value[Channel]<Popmenu1_Limit1[Channel])
            PopMenu1_Value[Channel]++;
          break;
        }

        if((PopType & LIST_POP)|| (Windows_Pop == 1)){   // Submenu Pop Selection.
          if((menu.current == Option) && (menu.menu_index[menu.current] == 1)
             && (PopMenu3_Value[WAVE_Type] > 0)){
               if(Cur_PopItem <= 1)           // menu_key_chosen
               {
                 if(PopMenu3_Value[SYS_PosiCyc])Cur_PopItem = Cur_Limit-1 ;  // DUTY does not loop during analog output.
               }else Cur_PopItem--;
             }else{
               if(Cur_PopItem <= 1){           // menu_key_chosen
                 if(PopMenu3_Value[SYS_PosiCyc])Cur_PopItem = Cur_Limit ;
               }
               else Cur_PopItem--;
             }
          Show_PopMenu(Cur_PopItem);
          menu.mflag &= ~UPD;
          menu.iflag &= ~UPD;
      CalPop_Flag = 1;
          ResPop_Flag = 1;
        }
        else if(PopType & FILE_POP){         //File management pop-up button selection.
          menu.current = Option;
          menu.menu_index[menu.current] = 0;
          if(Cur_PopItem <= 1){               // menu_key_chosen
            if(PopMenu3_Value[SYS_PosiCyc])Cur_PopItem = Cur_Limit ;
          }
          else Cur_PopItem--;
          Show_PopMenu(Cur_PopItem);
          menu.mflag &= ~UPD;
          menu.iflag &= ~UPD;
        }
        else if(!(PopType & (DAILOG_POP | PWR_POP))){      // Main menu selection.
          if(menu.menu_index[menu.current] <= 0){          // menu_key_chosen
            if(PopMenu3_Value[SYS_PosiCyc])menu.menu_index[menu.current] = Menu_Limit[menu.current]-1;
          }
          else
            menu.menu_index[menu.current]--;
          menu.iflag |= UPD;
        }
        break;

      case K_DOWN:
        if(menu.menu_flag == 0){
          if(PopMenu1_Value[Channel]>PopMenu1_Limit2[Channel])
          PopMenu1_Value[Channel]--;
          break;
        }

        if((PopType & LIST_POP)|| (Windows_Pop == 1)){
          if((menu.current == Option)&&
             (menu.menu_index[menu.current] == 1)
               &&(PopMenu3_Value[WAVE_Type] > 0)){
                 if(Cur_PopItem >= Cur_Limit-1){           // menu_key_chosen
                   if(PopMenu3_Value[SYS_PosiCyc])Cur_PopItem = 1 ;
                 }
                 else Cur_PopItem++;

               }else{
                 if(Cur_PopItem >= Cur_Limit  ){           // menu_key_chosen
                   if(PopMenu3_Value[SYS_PosiCyc])Cur_PopItem = 1;
                 }
                 else
                   Cur_PopItem++;
               }
          Show_PopMenu(Cur_PopItem);
          menu.mflag &= ~UPD;
          menu.iflag &= ~UPD;
          CalPop_Flag = 1;
          ResPop_Flag = 1;
        }
        else if(PopType & FILE_POP){                      // File Management pop-up key.
          menu.current = Option;
          menu.menu_index[menu.current] = 0;
          if(Cur_PopItem >= Cur_Limit  ){                  // menu_key_chosen
            if(PopMenu3_Value[SYS_PosiCyc])Cur_PopItem = 1;
          }
          else
            Cur_PopItem++;
          Show_PopMenu(Cur_PopItem);
          menu.mflag &= ~UPD;
          menu.iflag &= ~UPD;
        }
        else if(!(PopType & (DAILOG_POP | PWR_POP))){
          if(menu.menu_index[menu.current] >= Menu_Limit[menu.current]-1){
            if(PopMenu3_Value[SYS_PosiCyc])menu.menu_index[menu.current] = 0; //  menu_key_chosen
          }
          else
            menu.menu_index[menu.current]++;
          menu.iflag |= UPD;
        }
        break;

      case K_LEFT:
        if(menu.menu_flag == 0){
          temp = PopMenu1_Value[TIM_Base];
          if(PopMenu1_Value[TIM_Base]>PopMenu1_Limit2[TIM_Base])
            PopMenu1_Value[TIM_Base]--;
          if((temp==2)&&(PopMenu1_Value[TIM_Base]==1))__Ctrl(SMPL_MODE, INTERLEAVE);
          break;
        }

        if(PopType & FILE_POP) {                          // File Management pop-up key.
          PMenu_Proc(dec, Cur_PopItem, 0);
          menu.mflag &= ~UPD;
          menu.iflag &= ~UPD;
        }
        else {
          if((PopType & LIST_POP)|| (Windows_Pop == 1)) {

            PMenu_Proc(dec, Cur_PopItem, 0);
            CalPop_Flag = 1;
            ResPop_Flag = 1;
            if(menu.current == Oscillo){
              while(__Info(KEY_IN)==0x80){
                PMenu_Proc(dec, Cur_PopItem, 0);
                CalPop_Flag = 1;
                ResPop_Flag = 1;
                Process();
                __DrawWindow(VRAM_PTR);
                Update_Label();
              }
            }
          }
          else if(!(PopType & (DAILOG_POP | PWR_POP)))  Item_Proc(dec);
        }
        break;

      case K_RIGHT:
        if(menu.menu_flag == 0){
          temp = PopMenu1_Value[TIM_Base];
          if(PopMenu1_Value[TIM_Base]<Popmenu1_Limit1[TIM_Base])
            PopMenu1_Value[TIM_Base]++;
          if((temp==1)&&(PopMenu1_Value[TIM_Base]==2))__Ctrl(SMPL_MODE, SIMULTANEO);
          break;
        }

        if(PopType & FILE_POP) {                      // File Management pop-up key.
          PMenu_Proc(add, Cur_PopItem, 0);
          menu.mflag &= ~UPD;
          menu.iflag &= ~UPD;
        }
        else{
          if((PopType & LIST_POP) || (Windows_Pop == 1)) {
            PMenu_Proc(add, Cur_PopItem, 0);
            CalPop_Flag = 1;
            ResPop_Flag = 1;
            if(menu.current == Oscillo){
              while(__Info(KEY_IN)==0x80){
                PMenu_Proc(add, Cur_PopItem, 0);
                CalPop_Flag = 1;
                ResPop_Flag = 1;
                Process();
                __DrawWindow(VRAM_PTR);
                Update_Label();
              }
            }
          }
          else if(!(PopType & (DAILOG_POP | PWR_POP)))   Item_Proc(add);
        }
        break;

      } //----switch end-----
      Beep(50);

      KeyIn = 0;

      if(menu.menu_flag == 1)Show_Menu();    // Refresh when there is a menu bar.
      else {                                 // Refresh without menu bar.
        Update_Proc_All();
        Show_Title();
      }
      Update_Label();
    } //---Key_In end------


    if(Bat_Vol() < 3200) // Battery less than 3.2V: automatic shutdown.
    {
      Battery = 0;
      Battery_update();
      Beep(500);
      __Ctrl(DELAYmS, 500);                // 0.5s buzzer sounds before shutting down.
      __Ctrl(PWROFF, ENABLE);
    }

    if(About_Flag == 0){                             // Measured data timing refresh.
      if((Label_Cnt == 50)){
        Label_Cnt = 0;
        Label_Flag |= UPD;
        Update_Label();
        Print_dT_Info(INV);
        Print_dV_Info(INV);
        Battery_update();
        if((menu.menu_flag == 1) && (menu.current == Measure))Show_Measure();
      }else Label_Cnt++;
    }
  }
}

/******************************** END OF FILE *********************************/
