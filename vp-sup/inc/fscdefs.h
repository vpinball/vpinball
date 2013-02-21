/*
    File:       fscdefs.h

    Copyright:  c 1988-1990 by Apple Computer, Inc., all rights reserved.

*/


#define true 1
#define false 0

#define ONEFIX      ( 1L << 16 )
#define ONEFRAC     ( 1L << 30 )
#define ONEHALFFIX  0x8000L
#define ONEVECSHIFT 16
#define HALFVECDIV  (1L << (ONEVECSHIFT-1))

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef long int32;
typedef unsigned long uint32;

typedef short FUnit;
typedef unsigned short uFUnit;

typedef long Fixed;
typedef long Fract;

#ifndef F26Dot6
#define F26Dot6 long
#endif

#ifndef boolean
#define boolean int
#endif

#ifndef CONST
#define CONST const
#endif

#ifndef FAR
#define FAR
#endif

#ifndef NEAR 
#define NEAR 
#endif 

#ifndef TMP_CONV
#define TMP_CONV
#endif 

#ifndef FS_MAC_PASCAL
#define FS_MAC_PASCAL
#endif 

#ifndef FS_PC_PASCAL
#define FS_PC_PASCAL
#endif 

#ifndef FS_MAC_TRAP
#define FS_MAC_TRAP(a)
#endif 

typedef struct {
    Fixed       transform[3][3];
} transMatrix;

typedef struct {
    Fixed       x, y;
} vectorType;

/* Private Data Types */
typedef struct {
    int16 xMin;
    int16 yMin;
    int16 xMax;
    int16 yMax;
} BBOX;

#ifndef SHORTMUL
#define SHORTMUL(a,b)   (int32)((int32)(a) * (b))
#endif

#ifndef SHORTDIV
#define SHORTDIV(a,b)   (int32)((int32)(a) / (b))
#endif

#ifdef FSCFG_BIG_ENDIAN	/* target byte order matches Motorola 68000 */
 #define SWAPL(a)        (a)
 #define SWAPW(a)        (a)
 #define SWAPWINC(a)     (*(a)++)
#else
 /* Portable code to extract a short or a long from a 2- or 4-byte buffer */
 /* which was encoded using Motorola 68000 (TrueType "native") byte order. */
 #define FS_2BYTE(p)  ( ((unsigned short)((p)[0]) << 8) |  (p)[1])
 #define FS_4BYTE(p)  ( FS_2BYTE((p)+2) | ( (FS_2BYTE(p)+0L) << 16) )
#endif

#ifndef SWAPW
#define SWAPW(a)        ((short) FS_2BYTE( (unsigned char FAR*)(&a) ))
#endif

#ifndef SWAPL
#define SWAPL(a)        ((long) FS_4BYTE( (unsigned char FAR*)(&a) ))
#endif

#ifndef SWAPWINC
#define SWAPWINC(a)     SWAPW(*(a)); a++	/* Do NOT parenthesize! */
#endif

#ifndef LoopCount
#define LoopCount int16      /* short gives us a Motorola DBF */
#endif

#ifndef ArrayIndex
#define ArrayIndex int32     /* avoids EXT.L on Motorola */
#endif

typedef void (*voidFunc) ();
typedef void FAR * voidPtr;
typedef void (*ReleaseSFNTFunc) (voidPtr);
typedef void FAR * (*GetSFNTFunc) (long, long, long);


#ifndef MEMSET
#define MEMSET(dst, value, size)    memset(dst, value, (size_t)(size))
#define FS_NEED_STRING_DOT_H
#endif

#ifndef MEMCPY
#define MEMCPY(dst, src, size)      memcpy(dst, src, (size_t)(size))
#ifndef FS_NEED_STRING_DOT_H
#define FS_NEED_STRING_DOT_H
#endif
#endif

#ifdef FS_NEED_STRING_DOT_H
#undef FS_NEED_STRING_DOT_H
#include <string.h>
#endif

#ifndef FS_UNUSED_PARAMETER
#define FS_UNUSED_PARAMETER(a)		(a)	/* Silence some warnings */
#endif
