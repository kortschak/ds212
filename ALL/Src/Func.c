/********************* (C) COPYRIGHT 2017 e-Design Co.,Ltd. ********************
 File Name : Function.c
 Version   : DS212                                        Author : bure & Kewei
*******************************************************************************/
#include "Version.h"
#include "Func.h"

u32 TestCnt = 0;

/*******************************************************************************
  16 bytes Big-endian mode data is converted to little-endian mode.
*******************************************************************************/
void Rev16(u16* pBuf)
{
  asm(" LDRH    R1, [R0] ");
  asm(" REV16   R1, R1 ");
  asm(" STRH    R1, [R0] ");
}
/*******************************************************************************
  32 bytes Big-endian mode data is converted to little-endian mode.
*******************************************************************************/
void Rev32(u32* pBuf)
{
  asm(" LDR     R1, [R0] ");
  asm(" REV     R1, R1 ");
  asm(" STR     R1, [R0] ");
}
/*******************************************************************************
  Calculate the yth power of x.
*******************************************************************************/
u32 Power(u8 x, u8 y)
{
  u32 m = x;

  if(y == 0) return 1;
  while (--y) m *= x;
  return m;
}
/*******************************************************************************
  Calculate the xth power of 10.
*******************************************************************************/
u32 Exp(u8 x)
{
  u32 m = 1;

  while(x--) m *= 10;
  return m;
}
/*******************************************************************************
  Find the starting address of the Idx string in the data area
*******************************************************************************/
u8* SeekStr(u8* ptr, u8 Idx)
{
  while(Idx--) while(*ptr++);
  return ptr;
}
/*******************************************************************************
 Value2Str: 32-bit to e-digit effective number character string + dimension string (structure is Unit[][6]) + mode
*******************************************************************************/
void Value2Str(u8 *p, s32 n, uc8 *pUnit, s8 e, u8 Mode)
{
  s16 i = 0;
  s32 m = n, c = 5;

  if(Mode == SIGN){
    if(n == 0) *p++ = ' ';
    if(n >  0) *p++ = '+';
    if(n <  0){*p++ = '-'; n = -n;}
  }else if(Mode == UNSIGN) *p++ = ' ';
    
  while(m >= 10){m /= 10; i++;} // Calculate the number of significant bits of n; place in i.
  if((i%3 == 2)&&(e == 2)) e++;
  m = n; i = 0;
  while(m >= 10){
    m /= 10;
    if(++i > e) c *= 10;        // The number of significant digits of n, calculate the rounding value if greater than e; place in i.
  }
  if(i >= e) n += c;            // Add rounding to n.
  m = n; i = 0;
  while(m >= 10){m /= 10; i++;} // Recalculate the number of significant digits of n; place in i.

  m = i/3;                      // Calculate dimension unit offset.
  while(e--){
    *p++ = '0'+ n/Exp(i);
    if(e &&(i%3 == 0)) *p++ = '.';
    n = (i < 0)? 0 : n%Exp(i);
    i--;
  }
  pUnit += 6*m;                 //
  do {*p++ = *pUnit;}
  while(*pUnit++);              // With dimensional character strings
}

/*******************************************************************************
 Two ASCII character Change to 1 Byte HEX data
*******************************************************************************/
u8 Str2Byte(u8 x,u8 y) // Double ASCII character to 1-byte binary number
{
  uc8 Hexcode[17]="0123456789ABCDEF";
  u8 i, Temp=0;

  if(x>='a' && x<='z')  x-=32;     // Lower case to capital
  if(y>='a' && y<='z')  y-=32;     // Lower case to capital
  for(i=0;i<16;i++){
    if(Hexcode[i]==x)  Temp+=i*16; // Convert the character to a high 4-digit hexadecimal value.
  }
  for(i=0;i<16;i++){
    if(Hexcode[i]==y)  Temp+=i;    // Convert the character to a lower 4-digit hexadecimal value.
  }
  return Temp;
}

/*******************************************************************************
 u16ToDec4Str: Unsigned 16-bit binary digits to 4-digit decimal string with spaces before valid digits.
*******************************************************************************/
void u16ToDec4Str(u8 *p, u16 n)
{
  if(n/10000){
    *p++ = 'O';
    *p++ = 'v';
    *p++ = 'e';
    *p++ = 'r';
    *p   = 0;
    return;
  }
  *p++ = '0'+n/1000;
  n %= 1000;
  *p++ = '0'+n/100;
  n %= 100;
  *p++ = '0'+n/10;
  n %= 10;
  *p++ = '0'+n;
  *p = 0;
  if(p[-4] == '0'){
    p[-4] = ' ';
    if(p[-3] == '0'){
      p[-3] = ' ';
      if(p[-2] == '0') p[-2] = ' ';
    }
  }
}
/*******************************************************************************
 u16ToDec5Str: Unsigned 16-bit binary number to 5-digit decimal string
*******************************************************************************/
void u16ToDec5Str(u8 *p, u16 n)
{
  *p++ = '0'+n/10000;
  n %= 10000;
  *p++ = '0'+n/1000;
  n %= 1000;
  *p++ = '0'+n/100;
  n %= 100;
  *p++ = '0'+n/10;
  n %= 10;
  *p++ = '0'+n;
  *p = 0;
}
void s16ToDec5Str(u8 *p, s16 n)
{
  if(n >= 0) *p++ = '+';
  else      {*p++ = '-'; n = -n;}
  u16ToDec5Str(p, n);
}
/*******************************************************************************
 u8ToDec3: Change Byte to 3 decimal number string
*******************************************************************************/
void u8ToDec3Str(u8 *p, u8 n)
{
    *p++ = '0'+n/100;
    n %= 100;
    *p++ = '0'+n/10;
    n %= 10;
    *p++ = '0'+n;
    *p = 0;
}
/*******************************************************************************
 s8ToPercen: Change sign char to +(-)x.xx string
*******************************************************************************/
void s8ToPercen(u8 *p, s8 n)
{
    if(n >= 0)  *p++ = '+';
    else {
      *p++ = '-';
      n = -n;
    }
    *p++ = '0'+n/100;
    n %= 100;
    *p++ = '.';
    *p++ = '0'+n/10;
    n %= 10;
    *p++ = '0'+n;
    *p = 0;
}
/*******************************************************************************
 u8ToDec2: Change Byte to 2 decimal number string
*******************************************************************************/
void u8ToDec2Str(u8 *p, u8 n)
{
//    *p++ = '0'+n/100;
    n %= 100;
    *p++ = '0'+n/10;
    n %= 10;
    *p++ = '0'+n;
    *p = 0;
}
/*******************************************************************************
 Char2Hex: Change Byte to 2 hex number string
*******************************************************************************/
void Char2Hex(u8 *p, u8 n)
{
    if(n/16>9) *p++ = 'A'+(n/16-10);
    else       *p++ = '0'+n/16;
    n %= 16;
    if(n>9)    *p++ = 'A'+(n-10);
    else       *p++ = '0'+n;
    *p = 0;
}
/*******************************************************************************
 Half2Hex: Change 2Bytes to 4 hex number string
*******************************************************************************/
void Half2Hex(u8 *p, u16 n)
{
    if(n/0x1000 >9) *p++ = 'A'+(n/0x1000-10);
    else            *p++ = '0'+ n/0x1000;
    n %= 0x1000;
    if(n/0x100 >9)  *p++ = 'A'+(n/0x100-10);
    else            *p++ = '0'+ n/0x100;
    n %= 0x100;
    if(n/0x10 >9)   *p++ = 'A'+(n/0x10-10);
    else            *p++ = '0'+ n/0x10;
    n %= 0x10;
    if(n >9)        *p++ = 'A'+(n-10);
    else            *p++ = '0'+n;
    *p = 0;
}
/*******************************************************************************
 Word2Hex: Change 4 Bytes to 8 hex number string
*******************************************************************************/
void Word2Hex(u8 *p, u32 n)
{
  s8 i, k;

  for(i=28; i>=0; i-=4){
    k = ((n >> i)& 0xF);
    if(k > 9) *p++ = 'A'+ k-10;
    else      *p++ = '0'+ k;
  }
  *p = 0;

}
/*******************************************************************************
 Int_sqrt: unsigned int square root
*******************************************************************************/
u16 Sqrt32(u32 n)
{ u32 k;
  if ( n == 0 ) return 0;
  k = 2*Sqrt32(n/4)+1;
  if ( k*k > n ) return k-1;
  else return k;
}
/********************************* END OF FILE ********************************/
