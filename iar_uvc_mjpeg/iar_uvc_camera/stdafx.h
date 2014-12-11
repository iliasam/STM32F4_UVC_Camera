/* Name: stdafx.h
 * Project: ARM fast JPEG-coder
 * Author: Dmitry Oparin aka Rst7/CBSIE
 * Creation Date: 1-Jun-2008
 * Copyright: (C)2008 by Rst7/CBSIE
 * License: GNU GPL v3 (see http://www.gnu.org/licenses/gpl-3.0.txt)
 */

#ifndef STDAFX_H
#define STDAFX_H


#define __flash const
#define __x
#define __z
#define __x_z
#define __z_x


//#include <pgmspace.h> /* compiler extra options must be added "--string_literals_in_flash" */

/*****************************************************************************
Types definitions
******************************************************************************/

typedef unsigned int UREG;
typedef signed int REG;

/* byte */
typedef char CHAR;
typedef signed char SCHAR;
typedef unsigned char UCHAR;
typedef SCHAR INT8;
typedef UCHAR UINT8;
typedef SCHAR SINT8;
#define MAXUINT8	((UINT8)(-1))

/* word */
typedef short INT16;
typedef signed short SINT16;
typedef unsigned short UINT16;
#define MAXUINT16	((UINT16)(-1))

/* long word */
typedef long INT32;
typedef signed long SINT32;
typedef unsigned long UINT32;
#define MAXUINT32	((UINT32)(-1))

/* utils */
/*#define MAKEUINT16(byte_h, byte_l)	((UINT16)(((UINT16)(byte_h) << 8)|(UINT16)(byte_l)))
#define HIBYTE(word)				((UCHAR)((UINT16)(word) >> 8))
#define LOBYTE(word)				((UCHAR)((UINT16)(word) & 0xff))

#define MAKEUINT32(word_h, word_l)	((UINT32)(((UINT32)(word_h) << 16)|(UINT32)(word_l)))
#define HIWORD(longv)				((UINT16)((UINT32)(longv) >> 16))
#define LOWORD(longv)				((UINT16)((UINT32)(longv) & 0xffff))*/

#define SWAP16(n16) ((((UINT16)(n16))<<8)|(((UINT16)(n16))>>8))
/*#define SWAP32(n32)					(MAKEUINT32(SWAP16(LOWORD(n32)), SWAP16(HIWORD(n32))))*/

#endif//STDAFX_H

