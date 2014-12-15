/* Name: jprocess.c
 * Project: ARM fast JPEG-coder
 * Author: Dmitry Oparin aka Rst7/CBSIE (Modified by ILIASAM)
 * Creation Date: 1-Jun-2008
 * Copyright: (C)2008 by Rst7/CBSIE
 * License: GNU GPL v3 (see http://www.gnu.org/licenses/gpl-3.0.txt)
 */

#include "stdafx.h"
#include "params.h"
#include "jprocess.h"
#include "gray_jpg_test3.h"
#include <stdint.h>

#define LINE_WIDTH IMG_WIDTH

//coocox
//uint8_t outbytes0[20000] __attribute__ ((section(".ccm")));
//uint8_t outbytes1[20000] __attribute__ ((section(".ccm")));

#pragma location = ".ccmram"
uint8_t outbytes0[32000];
#pragma location = ".ccmram"
uint8_t outbytes1[32000];

uint8_t *write_pointer = (uint8_t*)&outbytes0;//сюда записываются закодированные jpeg данные (кодером)
uint8_t *read_pointer =  (uint8_t*)&outbytes1;//отсода данные считываются при передаче

extern uint8_t raw_image[240][320];

//Использовать *outbytes_p для вывода в outbytes
#define USE_OUTBYTES


#ifdef USE_OUTBYTES
//uint8_t outbytes[32768];
#else
//Сразу в железо, для AVR тут стоит #define OUTSYM(VAR) {while(!UCSRA_UDRE);UDR=VAR;}
volatile UINT8 _SYMBOL;
#define OUTSYM(VAR) _SYMBOL=(VAR)
#endif

//Использовать вместо деления на коэффициент q[i] - умножение на 1/q[i].
#define USE_MUL

#define abs(VAL) ((VAL)>=0?(VAL):-(VAL))
#define DIVIDE_(a,b)	((a)>=(b)?(a)/(b):0)

static DCTELEM dct_data[DCTSIZE*DCTSIZE];

static __flash UINT8 JHEADER_1[]=
{
  0xFF,0xD8, //SOI, Start of Image
  0xFF,0xDB, //DQT, Define quantization table(s)
  0x00,0x43, //Lq - length
  0x00      //Pq,Tq - Quantization table element precision/Quantization table destination identifier
};

typedef struct
{
  unsigned int q;
#ifdef USE_MUL
  unsigned int iq;
#endif
  DCTELEM *idx;
}ZQ;

static __flash UINT8 zigzag_index[] =
{
   0,  1,  8, 16,  9,  2,  3, 10,
  17, 24, 32, 25, 18, 11,  4,  5,
  12, 19, 26, 33, 40, 48, 41, 34,
  27, 20, 13,  6,  7, 14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36,
  29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46,
  53, 60, 61, 54, 47, 55, 62, 63
};

static ZQ z_q[64];

static __flash UINT16 aanscales[64] = {
  /* precomputed values scaled up by 14 bits */
  16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
  22725, 31521, 29692, 26722, 22725, 17855, 12299,  6270,
  21407, 29692, 27969, 25172, 21407, 16819, 11585,  5906,
  19266, 26722, 25172, 22654, 19266, 15137, 10426,  5315,
  16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
  12873, 17855, 16819, 15137, 12873, 10114,  6967,  3552,
  8867, 12299, 11585, 10426,  8867,  6967,  4799,  2446,
  4520,  6270,  5906,  5315,  4520,  3552,  2446,  1247
};

static __flash UINT8 JHEADER_Q[]=
{
  16, 11, 10, 16, 24, 40, 51, 61,
  12, 12, 14, 19, 26, 58, 60, 55,
  14, 13, 16, 24, 40, 57, 69, 56,
  14, 17, 22, 29, 51, 87, 80, 62,
  18, 22, 37, 56, 68, 109,103,77,
  24, 35, 55, 64, 81, 104,113,92,
  49, 64, 78, 87, 103,121,120,101,
  72, 92, 95, 98, 112,100,103,99
};

static __flash UINT8 JHEADER_2[]=
{
  0xFF,0xC0, //SOF0, Start of frame, Baseline DCT
  0x00,0x11, //Lf - length
  0x08,      //P - precision
  (UINT8)(IMG_HEIGHT>>8),(UINT8)IMG_HEIGHT,  //Y - height
  (UINT8)(IMG_WIDTH>>8),(UINT8)IMG_WIDTH,  //X - width
  0x03,      //N - number of image components

  0x01,      //Ci - component identifier
  0x22,      //Hi,Vi - Horizontal/vertical sampling factor
  0x00,      //Tqi - Quantization table selector

  0x02,      //Ci - component identifier
  0x11,      //Hi,Vi - Horizontal/vertical sampling factor
  0x00,      //Tqi - Quantization table selector

  0x03,      //Ci - component identifier
  0x11,      //Hi,Vi - Horizontal/vertical sampling factor
  0x00,      //Tqi - Quantization table selector

  0xFF,0xC4, //DHT, Define Huffman table(s)
  0x00,0x1F, //length
  0x00,      //id - DC table 0
  0x00,0x01,0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,
  0xFF,0xC4, //DHT, Define Huffman tables(s)
  0x00,0xB5, //length
  0x10,      //id - AC table 0
  0x00,0x02,0x01,0x03,0x03,0x02,0x04,0x03,0x05,0x05,0x04,0x04,0x00,0x00,0x01,0x7D,
  0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61,0x07,
  0x22,0x71,0x14,0x32,0x81,0x91,0xA1,0x08,0x23,0x42,0xB1,0xC1,0x15,0x52,0xD1,0xF0,
  0x24,0x33,0x62,0x72,0x82,0x09,0x0A,0x16,0x17,0x18,0x19,0x1A,0x25,0x26,0x27,0x28,
  0x29,0x2A,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,
  0x4A,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,
  0x6A,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x83,0x84,0x85,0x86,0x87,0x88,0x89,
  0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,
  0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,
  0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xE1,0xE2,
  0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,
  0xF9,0xFA,
  0xFF,0xDA, //SOS, Start of scan
  0x00,0x0C, //Ls - length
  0x03,      //Ns - Number of image component in scan

  0x01,      //Csj - Scan component selector
  0x00,      //Tdj, Taj - DC/AC entropy coding table destination selector

  0x02,      //Csj - Scan component selector
  0x00,      //Tdj, Taj - DC/AC entropy coding table destination selector

  0x03,      //Csj - Scan component selector
  0x00,      //Tdj, Taj - DC/AC entropy coding table destination selector

  0x00,      //Ss - Start of spectral or predictor selection
  0x3F,      //Se - End of spectral selection
  0x00       //Ah, Al - Successive approximation bit position high/low or point transform
};

#ifdef USE_OUTBYTES
UINT8 *outbytes_p;
#endif
UREG bitstream_byte;
UREG bitstream_bit;
static int LastDC;

//#pragma inline=forced
static void out_jheader(UREG quality, unsigned char *start_wr_pointer)
{
  UINT8 __flash *p=JHEADER_1;
  int l=sizeof(JHEADER_1);
#ifdef USE_OUTBYTES
  //UINT8 *wp=outbytes;
  UINT8 *wp=start_wr_pointer;
#endif
  UREG i;
  UINT32 sq;
#ifdef USE_OUTBYTES
  do *wp++=*p++; while(--l);
#else
  do OUTSYM(*p++); while(--l);
#endif
  p=JHEADER_Q;
  i=0;
  do
  {
    UREG zi=zigzag_index[i];
    UREG q=JHEADER_Q[zi]/quality;
    if (q<2) q=2;
    z_q[i].idx=dct_data+zi;
    sq=(UINT32)aanscales[zi]*q;
    z_q[i].q=sq>>11;
#ifdef USE_MUL
    z_q[i].iq=((65536L<<11)+(sq>>1))/sq;
#endif
#ifdef USE_OUTBYTES
    *wp++=q;
#else
    OUTSYM(q);
#endif
    i++;
  }
  while(i<64);
  p=JHEADER_2;
  l=sizeof(JHEADER_2);
#ifdef USE_OUTBYTES
  do *wp++=*p++; while(--l);
  outbytes_p=wp;
#else
  do OUTSYM(*p++); while(--l);
#endif
}

//return jpeg bytes number
//#pragma inline=forced
static unsigned int out_jtail(unsigned char *start_wr_pointer)
{
#ifdef USE_OUTBYTES
  UINT8 *wp=outbytes_p;
#endif
#ifdef USE_OUTBYTES    
  if (bitstream_bit<32)
  {
    UREG c;
    c=bitstream_byte>>24;
    if ((*wp++=c)==0xFF) *wp++=0;
    if (bitstream_bit<24)
    {
      c=bitstream_byte>>16;
      if ((*wp++=c)==0xFF) *wp++=0;
      if (bitstream_bit<16)
      {
	c=bitstream_byte>>8;
	if ((*wp++=c)==0xFF) *wp++=0;
	if (bitstream_bit<8)
	{
	  c=bitstream_byte;
	  if ((*wp++=c)==0xFF) *wp++=0;
	}
      }
    }
#else
    OUTSYM(bitstream_byte);
    if (bitstream_byte==0xFF) OUTSYM(0);
#endif
  }
#ifdef USE_OUTBYTES
  *wp++=0xFF;
  *wp++=0xD9;
  //return wp-outbytes;
  return wp-start_wr_pointer;
#else
  OUTSYM(0xFF);
  OUTSYM(0xD9);
  return 0;
#endif
}

static __flash UINT8 csize[256] =
{
  0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
};

//Fast bitstream write

#ifdef USE_OUTBYTES
#define _LOAD_BITSTREAM_CTRL(BYTE,BIT,POINTER) \
  UREG __bs_buf=BYTE; \
  UREG __bs_bit=BIT; \
//  UINT8 *__bs_bytep=POINTER;

#define _STORE_BITSTREAM_CTRL(BYTE,BIT,POINTER) \
  BYTE=__bs_buf; \
  BIT=__bs_bit; \
//  POINTER=__bs_bytep;

#define _WRITE_BITS_N(LEN,SYM) \
  { \
    if ((REG)(__bs_bit-=LEN)>0) \
    { \
      __bs_buf|=SYM<<__bs_bit; \
    } \
    else \
    { \
      UREG c; \
      __bs_buf|=SYM>>(__bs_bit=0-__bs_bit); \
      c=__bs_buf>>24; \
      if ((*__bs_bytep++=c)==0xFF) *__bs_bytep++=0; \
      c=__bs_buf>>16; \
      if ((*__bs_bytep++=c)==0xFF) *__bs_bytep++=0; \
      c=__bs_buf>>8; \
      if ((*__bs_bytep++=c)==0xFF) *__bs_bytep++=0; \
      c=__bs_buf>>0; \
      if ((*__bs_bytep++=c)==0xFF) *__bs_bytep++=0; \
      __bs_buf=SYM<<(__bs_bit=32-__bs_bit); \
    } \
  } \

#define _STORE_BITSTREAM_CTRL(BYTE,BIT,POINTER) \
  BYTE=__bs_buf; \
  BIT=__bs_bit; \
//  POINTER=__bs_bytep;

#endif


typedef struct
{
  UINT16 codelen;
  UINT16 code;
}HUFF_TABLE;

static __flash HUFF_TABLE huff_dc[12]=
{
 {0x0002,0x0000}, {0x0003,0x0002}, {0x0003,0x0003}, {0x0003,0x0004}, {0x0003,0x0005}, {0x0003,0x0006}, {0x0004,0x000E}, {0x0005,0x001E},
 {0x0006,0x003E}, {0x0007,0x007E}, {0x0008,0x00FE}, {0x0009,0x01FE}
};
static __flash HUFF_TABLE huff_ac[256]=
{
 {0x0004,0x000A}, {0x0002,0x0000}, {0x0002,0x0001}, {0x0003,0x0004}, {0x0004,0x000B}, {0x0005,0x001A}, {0x0007,0x0078}, {0x0008,0x00F8},
 {0x000A,0x03F6}, {0x0010,0xFF82}, {0x0010,0xFF83}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000},
 {0x0000,0x0000}, {0x0004,0x000C}, {0x0005,0x001B}, {0x0007,0x0079}, {0x0009,0x01F6}, {0x000B,0x07F6}, {0x0010,0xFF84}, {0x0010,0xFF85},
 {0x0010,0xFF86}, {0x0010,0xFF87}, {0x0010,0xFF88}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000},
 {0x0000,0x0000}, {0x0005,0x001C}, {0x0008,0x00F9}, {0x000A,0x03F7}, {0x000C,0x0FF4}, {0x0010,0xFF89}, {0x0010,0xFF8A}, {0x0010,0xFF8B},
 {0x0010,0xFF8C}, {0x0010,0xFF8D}, {0x0010,0xFF8E}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000},
 {0x0000,0x0000}, {0x0006,0x003A}, {0x0009,0x01F7}, {0x000C,0x0FF5}, {0x0010,0xFF8F}, {0x0010,0xFF90}, {0x0010,0xFF91}, {0x0010,0xFF92},
 {0x0010,0xFF93}, {0x0010,0xFF94}, {0x0010,0xFF95}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000},
 {0x0000,0x0000}, {0x0006,0x003B}, {0x000A,0x03F8}, {0x0010,0xFF96}, {0x0010,0xFF97}, {0x0010,0xFF98}, {0x0010,0xFF99}, {0x0010,0xFF9A},
 {0x0010,0xFF9B}, {0x0010,0xFF9C}, {0x0010,0xFF9D}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000},
 {0x0000,0x0000}, {0x0007,0x007A}, {0x000B,0x07F7}, {0x0010,0xFF9E}, {0x0010,0xFF9F}, {0x0010,0xFFA0}, {0x0010,0xFFA1}, {0x0010,0xFFA2},
 {0x0010,0xFFA3}, {0x0010,0xFFA4}, {0x0010,0xFFA5}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000},
 {0x0000,0x0000}, {0x0007,0x007B}, {0x000C,0x0FF6}, {0x0010,0xFFA6}, {0x0010,0xFFA7}, {0x0010,0xFFA8}, {0x0010,0xFFA9}, {0x0010,0xFFAA},
 {0x0010,0xFFAB}, {0x0010,0xFFAC}, {0x0010,0xFFAD}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000},
 {0x0000,0x0000}, {0x0008,0x00FA}, {0x000C,0x0FF7}, {0x0010,0xFFAE}, {0x0010,0xFFAF}, {0x0010,0xFFB0}, {0x0010,0xFFB1}, {0x0010,0xFFB2},
 {0x0010,0xFFB3}, {0x0010,0xFFB4}, {0x0010,0xFFB5}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000},
 {0x0000,0x0000}, {0x0009,0x01F8}, {0x000F,0x7FC0}, {0x0010,0xFFB6}, {0x0010,0xFFB7}, {0x0010,0xFFB8}, {0x0010,0xFFB9}, {0x0010,0xFFBA},
 {0x0010,0xFFBB}, {0x0010,0xFFBC}, {0x0010,0xFFBD}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000},
 {0x0000,0x0000}, {0x0009,0x01F9}, {0x0010,0xFFBE}, {0x0010,0xFFBF}, {0x0010,0xFFC0}, {0x0010,0xFFC1}, {0x0010,0xFFC2}, {0x0010,0xFFC3},
 {0x0010,0xFFC4}, {0x0010,0xFFC5}, {0x0010,0xFFC6}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000},
 {0x0000,0x0000}, {0x0009,0x01FA}, {0x0010,0xFFC7}, {0x0010,0xFFC8}, {0x0010,0xFFC9}, {0x0010,0xFFCA}, {0x0010,0xFFCB}, {0x0010,0xFFCC},
 {0x0010,0xFFCD}, {0x0010,0xFFCE}, {0x0010,0xFFCF}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000},
 {0x0000,0x0000}, {0x000A,0x03F9}, {0x0010,0xFFD0}, {0x0010,0xFFD1}, {0x0010,0xFFD2}, {0x0010,0xFFD3}, {0x0010,0xFFD4}, {0x0010,0xFFD5},
 {0x0010,0xFFD6}, {0x0010,0xFFD7}, {0x0010,0xFFD8}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000},
 {0x0000,0x0000}, {0x000A,0x03FA}, {0x0010,0xFFD9}, {0x0010,0xFFDA}, {0x0010,0xFFDB}, {0x0010,0xFFDC}, {0x0010,0xFFDD}, {0x0010,0xFFDE},
 {0x0010,0xFFDF}, {0x0010,0xFFE0}, {0x0010,0xFFE1}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000},
 {0x0000,0x0000}, {0x000B,0x07F8}, {0x0010,0xFFE2}, {0x0010,0xFFE3}, {0x0010,0xFFE4}, {0x0010,0xFFE5}, {0x0010,0xFFE6}, {0x0010,0xFFE7},
 {0x0010,0xFFE8}, {0x0010,0xFFE9}, {0x0010,0xFFEA}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000},
 {0x0000,0x0000}, {0x0010,0xFFEB}, {0x0010,0xFFEC}, {0x0010,0xFFED}, {0x0010,0xFFEE}, {0x0010,0xFFEF}, {0x0010,0xFFF0}, {0x0010,0xFFF1},
 {0x0010,0xFFF2}, {0x0010,0xFFF3}, {0x0010,0xFFF4}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000},
 {0x000B,0x07F9}, {0x0010,0xFFF5}, {0x0010,0xFFF6}, {0x0010,0xFFF7}, {0x0010,0xFFF8}, {0x0010,0xFFF9}, {0x0010,0xFFFA}, {0x0010,0xFFFB},
 {0x0010,0xFFFC}, {0x0010,0xFFFD}, {0x0010,0xFFFE}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}, {0x0000,0x0000}
};

   
#define EncodeHuffman(HUFF,VAL)\
{\
  HUFF_TABLE __flash *__P=(HUFF)+(VAL);\
  UREG __len=__P->codelen;\
  UREG __sym=__P->code;\
  _WRITE_BITS_N(__len,__sym);\
}

//-----------------------
//DCT

#define CONST_BITS  (8)
#define FIX_0_382683433  ((INT32)   98)		/* FIX(0.382683433) */
#define FIX_0_541196100  ((INT32)  139)		/* FIX(0.541196100) */
#define FIX_0_707106781  ((INT32)  181)		/* FIX(0.707106781) */
#define FIX_1_306562965  ((INT32)  334)		/* FIX(1.306562965) */
#define FIX_0_306562965  ((INT32)  (334-256))		/* FIX(1.306562965) */
#define DESCALE(x,n)  ((x)>>(n))

#define MULTIPLY(var,const)  ((DCTELEM) DESCALE((var) * (const), CONST_BITS))

/*
 * Perform the forward DCT on one block of samples.
 */

//#pragma inline=forced
static __z void dct_pass1(DCTELEM * dataptr, const UINT8 *inp)
{
  DCTELEM tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  DCTELEM tmp10, tmp11, tmp12, tmp13;
  DCTELEM z1, z2, z3, z4, z5, z11, z13;
  UREG ctr;
  
  /* Pass 1: process rows. */
  
  ctr=DCTSIZE;
  do
  {
    {
      DCTELEM l,r;
#define GET_tmp(ldisp,rdisp,left,right) \
  {l=inp[ldisp];r=inp[rdisp]; left=l+r-256; right=l-r;}
      GET_tmp(0,7,tmp0,tmp7);
      GET_tmp(1,6,tmp1,tmp6);
      GET_tmp(2,5,tmp2,tmp5);
      GET_tmp(3,4,tmp3,tmp4);
#undef GET_tmp
    }
    /*tmp0 = dataptr[0] + dataptr[7];
    tmp7 = dataptr[0] - dataptr[7];
    tmp1 = dataptr[1] + dataptr[6];
    tmp6 = dataptr[1] - dataptr[6];
    tmp2 = dataptr[2] + dataptr[5];
    tmp5 = dataptr[2] - dataptr[5];
    tmp3 = dataptr[3] + dataptr[4];
    tmp4 = dataptr[3] - dataptr[4];*/
    
    /* Even part */
    
    tmp10 = tmp0 + tmp3;	/* phase 2 */
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;
    
    dataptr[0] = tmp10 + tmp11; /* phase 3 */
    dataptr[4] = tmp10 - tmp11;
    
    z1 = MULTIPLY(tmp12 + tmp13, FIX_0_707106781); /* c4 */
    dataptr[2] = tmp13 + z1;	/* phase 5 */
    dataptr[6] = tmp13 - z1;
    
    /* Odd part */

    tmp10 = tmp4 + tmp5;	/* phase 2 */
    tmp11 = tmp5 + tmp6;
    tmp12 = tmp6 + tmp7;

    /* The rotator is modified from fig 4-8 to avoid extra negations. */
    z5 = MULTIPLY(tmp10 - tmp12, FIX_0_382683433); /* c6 */
    z2 = MULTIPLY(tmp10, FIX_0_541196100) + z5; /* c2-c6 */
    z4 = MULTIPLY(tmp12, FIX_1_306562965) + z5; /* c2+c6 */
//    z4 = MULTIPLY(tmp12, FIX_0_306562965) + tmp12 + z5; /* c2+c6 */
    z3 = MULTIPLY(tmp11, FIX_0_707106781); /* c4 */

    z11 = tmp7 + z3;		/* phase 5 */
    z13 = tmp7 - z3;

    dataptr[5] = z13 + z2;	/* phase 6 */
    dataptr[3] = z13 - z2;
    dataptr[1] = z11 + z4;
    dataptr[7] = z11 - z4;

    dataptr += DCTSIZE;		/* advance pointer to next row */
    //inp-=IMG_WIDTH; //минус - потому как bmp вверх ногами
    inp+=LINE_WIDTH; //для нормальных изображений
  }
  while(--ctr);
}

//#pragma inline=forced
__z_x void dct_pass2(DCTELEM * dataptr)
{
  DCTELEM tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  DCTELEM tmp10, tmp11, tmp12, tmp13;
  DCTELEM z1, z2, z3, z4, z5, z11, z13;
//  DCTELEM *datap2;
  UREG ctr;

  /* Pass 2: process columns. */

  ctr=DCTSIZE;
  do
  {
    tmp0 = dataptr[DCTSIZE*0] + dataptr[DCTSIZE*7];
    tmp7 = dataptr[DCTSIZE*0] - dataptr[DCTSIZE*7];
    tmp1 = dataptr[DCTSIZE*1] + dataptr[DCTSIZE*6];
    tmp6 = dataptr[DCTSIZE*1] - dataptr[DCTSIZE*6];
    tmp2 = dataptr[DCTSIZE*2] + dataptr[DCTSIZE*5];
    tmp5 = dataptr[DCTSIZE*2] - dataptr[DCTSIZE*5];
    tmp3 = dataptr[DCTSIZE*3] + dataptr[DCTSIZE*4];
    tmp4 = dataptr[DCTSIZE*3] - dataptr[DCTSIZE*4];
    
    
    /* Even part */
    
    tmp10 = tmp0 + tmp3;	/* phase 2 */
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;
    
    dataptr[DCTSIZE*0] = tmp10 + tmp11; /* phase 3 */
    dataptr[DCTSIZE*4] = tmp10 - tmp11;
    z1 = MULTIPLY(tmp12 + tmp13, FIX_0_707106781); /* c4 */
    dataptr[DCTSIZE*2] = tmp13 + z1; /* phase 5 */
    dataptr[DCTSIZE*6] = tmp13 - z1;
    
    /* Odd part */

    tmp10 = tmp4 + tmp5;	/* phase 2 */
    tmp11 = tmp5 + tmp6;
    tmp12 = tmp6 + tmp7;

    /* The rotator is modified from fig 4-8 to avoid extra negations. */
    z5 = MULTIPLY(tmp10 - tmp12, FIX_0_382683433); /* c6 */
    z2 = MULTIPLY(tmp10, FIX_0_541196100) + z5; /* c2-c6 */
    z4 = MULTIPLY(tmp12, FIX_1_306562965) + z5; /* c2+c6 */
//    z4 = MULTIPLY(tmp12, FIX_0_306562965) + tmp12 + z5; /* c2+c6 */
    z3 = MULTIPLY(tmp11, FIX_0_707106781); /* c4 */

    z11 = tmp7 + z3;		/* phase 5 */
    z13 = tmp7 - z3;
    
    dataptr[DCTSIZE*5] = z13 + z2; /* phase 6 */
    dataptr[DCTSIZE*3] = z13 - z2;
    dataptr[DCTSIZE*1] = z11 + z4;
    dataptr[DCTSIZE*7] = z11 - z4;

    dataptr++;			/* advance pointer to next column */
//    datap2++;
  }
  while(--ctr);
}

//-----------------------

#define MUL_U16(a,b) ((unsigned int)(((UINT32)(a)*(b))>>16))
//#define MUL_U16(a,b) ( (unsigned int)( ( ((UINT32)(a)*(b))+0x8000 )>>16 ) ) //more contrast?

//Quanization and Zigzag
//Encode and write data
//#pragma inline=forced
__x_z UINT8 *z_and_q(UREG __bs_buf, UREG __bs_bit, UINT8 *__bs_bytep, const unsigned int *zqp, UINT8 is_chroma)
{
  //Encode DC
  {
    unsigned int cofac;
    int diff,coef;
    UREG s;
    unsigned int __q=*zqp++;
#ifdef USE_MUL
    unsigned int __iq=*zqp++;
#endif
    coef=*((DCTELEM*)(*zqp++));
    if (coef<0)
    {
      unsigned int tmp;
      tmp = -coef;
      tmp += __q>>1;	/* for rounding */
#ifdef USE_MUL
      coef = -(tmp>__q?MUL_U16(tmp, __iq):0);
#else
      coef = -DIVIDE_(tmp, __q);
#endif
    }
    else
    {
      unsigned int tmp;
      tmp = coef;
      tmp += __q>>1;	/* for rounding */
#ifdef USE_MUL
      coef = tmp>__q?MUL_U16(tmp,__iq):0;
#else
      coef = DIVIDE_(tmp, __q);
#endif
    }
    if (is_chroma == 0)
    {
    	diff = coef - LastDC;
    	LastDC = coef;                /* Do DPCM */
    }
    else {diff = coef;}


    cofac = abs(diff);
    if (cofac < 256)
    {
      s = csize[cofac];          /* Find true size */
    }
    else
    {
      cofac = cofac >> 8;
      s = csize[cofac] + 8;
    }
    EncodeHuffman(huff_dc,s);              /* Encode size */
    if (s)
    {
      if (diff<0)             /* Follow by significant bits */
      {
	diff--;
	diff&=(1<<s)-1; 
      }
      _WRITE_BITS_N(s,diff);
    }
  }
  //Encode AC
  {
    unsigned int cofac;
    UREG k,r,ssss,ii;
    DCTELEM data;
    r=0;
    k=BLOCKSIZE-1;
  FAST_LOOP:
    do
    {
      unsigned int __q=*zqp++;
#ifdef USE_MUL
      unsigned int __iq=*zqp++;
#endif
      data=*((DCTELEM*)(*zqp++));
      if (data<0)
      {
	unsigned int tmp;
	tmp = -data;
	tmp += __q>>1;	/* for rounding */
#ifdef USE_MUL
	if (tmp<=__q) goto FAST_ZERO;
	data = -(cofac=MUL_U16(tmp, __iq));
#else
	data = -(cofac=DIVIDE_(tmp, __q));
#endif
      }
      else
      {
	unsigned int tmp;
	tmp = data;
	tmp += __q>>1;	/* for rounding */
#ifdef USE_MUL
	if (tmp<=__q) goto FAST_ZERO;
	data = cofac=MUL_U16(tmp, __iq);
#else
	data = cofac = DIVIDE_(tmp, __q);
#endif
      }
      //cofac = abs(data);             /* Find absolute size */
      if (!cofac)                /* Check for zeroes */
      {
      FAST_ZERO:
	r++;                           /* Increment run-length of zeroes */
	if (--k) goto FAST_LOOP;
	_WRITE_BITS_N(4,0x000A); //0 -> code 0x000A, size 4
	break;
      }
      else
      {
	if (cofac < 256)
	{
	  ssss = csize[cofac];
	}
	else
	{
	  cofac = cofac >> 8;
	  ssss = csize[cofac] + 8;
	}
	//while(r > 15)                 /* If run-length > 15, time for  */
	goto L1;
#pragma diag_suppress=Pe128
	do
	{                           /* Run-length extension */
	  _WRITE_BITS_N(11,0x07F9); //240 -> code 0x7F9, size 11
	  r -= 16;
	L1:;
	}
	while(r > 15);
#pragma diag_default=Pe128	
	ii = 16*r + ssss;              /* Now we can find code byte */
	r = 0;
	EncodeHuffman(huff_ac,ii);             /* Encode RLE code */
	if (data<0)             /* Follow by significant bits */
	{
	  data--;
	  data&=(1<<ssss)-1; 
	}
	_WRITE_BITS_N(ssss,data);
      }
    }
    while(--k);
  }
  _STORE_BITSTREAM_CTRL(bitstream_byte,bitstream_bit,outbytes_p);
  return(__bs_bytep);
}

void process_quadro_block(UINT8 *start_pointer)//pointer - upper left pixel
{
	uint8_t i;

	//Process DCT
    dct_pass1(dct_data, (UINT8*)(start_pointer));//upper left
	dct_pass2(dct_data);
    outbytes_p=z_and_q(bitstream_byte,bitstream_bit,outbytes_p,(unsigned int *)z_q,0);

    dct_pass1(dct_data, (UINT8*)(start_pointer + DCTSIZE));//upper right
	dct_pass2(dct_data);
    outbytes_p=z_and_q(bitstream_byte,bitstream_bit,outbytes_p,(unsigned int *)z_q,0);

    dct_pass1(dct_data, (UINT8*)(start_pointer + DCTSIZE*LINE_WIDTH));//lower left
	dct_pass2(dct_data);
    outbytes_p=z_and_q(bitstream_byte,bitstream_bit,outbytes_p,(unsigned int *)z_q,0);

    dct_pass1(dct_data, (UINT8*)(start_pointer + DCTSIZE*LINE_WIDTH + DCTSIZE));//lower right
	dct_pass2(dct_data);
    outbytes_p=z_and_q(bitstream_byte,bitstream_bit,outbytes_p,(unsigned int *)z_q,0);

    for (i=0;i<(DCTSIZE*DCTSIZE);i++){
      dct_data[i] = 0;
      asm("nop");//do not remove - protection from optimization
    }//clean for chroma
    outbytes_p=z_and_q(bitstream_byte,bitstream_bit,outbytes_p,(unsigned int *)z_q,1);//cb
    outbytes_p=z_and_q(bitstream_byte,bitstream_bit,outbytes_p,(unsigned int *)z_q,1);//cr
}


unsigned int jprocess(void)
{
  UREG ycount=0;
  out_jheader(3,write_pointer);//3 - quality
  LastDC=0;
  bitstream_byte=0;
  bitstream_bit=32;

  do
  {
    UREG xcount=0;
    do
    { 
      process_quadro_block((UINT8*)(inBMP2+0x436 + xcount*DCTSIZE + ycount*IMG_WIDTH));//pointer - upper left pixel
      //process_quadro_block((UINT8*)((uint8_t*)&raw_image + xcount*DCTSIZE + ycount*IMG_WIDTH));//pointer - upper left pixel
      //Next blocks by X
      xcount = xcount+2;
    }
    while(xcount<(IMG_WIDTH/DCTSIZE));
    //Next block by Y
  }
  while((ycount+=(DCTSIZE*2))<(IMG_HEIGHT));

  return out_jtail(write_pointer);//start poiter нужен для вычисления размера изображения
}


void switch_buffers(void)
{
  if (write_pointer == ((uint8_t*)&outbytes0))
  {
    write_pointer = outbytes1;
    read_pointer = outbytes0;
  }
  else
  {
    write_pointer = outbytes0;
    read_pointer = outbytes1;
  }
}
