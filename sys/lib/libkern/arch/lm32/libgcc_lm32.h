/* Integer arithmetic support for Lattice Mico32.
   Contributed by Jon Beniston <jon@beniston.com> 
   
   Copyright (C) 2009 Free Software Foundation, Inc.

   This file is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 3, or (at your option) any
   later version.
   
   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.
   
   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>. */

#ifndef LIBGCC_LM32_H
#define LIBGCC_LM32_H 

/* Types.  */

typedef unsigned char UQItype __attribute__ ((mode (QI)));
typedef long SItype __attribute__ ((mode (SI)));
typedef unsigned long USItype __attribute__ ((mode (SI)));
typedef int DItype __attribute__ ((mode (DI)));
typedef unsigned int UDItype __attribute__ ((mode (DI)));

#define Wtype SItype
#define UWtype USItype
#define HWtype SItype
#define UHWtype USItype
#define DWtype DItype
#define UDWtype UDItype
/* Prototypes.  */

USItype __mulsi3 (USItype a, USItype b);
USItype __udivmodsi4 (USItype num, USItype den, int modwanted);
SItype __divsi3 (SItype a, SItype b);
SItype __modsi3 (SItype a, SItype b);
USItype __udivsi3 (USItype a, USItype b);
USItype __umodsi3 (USItype a, USItype b);
DWtype __muldi3 (DWtype u, DWtype v);
//DWtype __moddi3 (DWtype u, DWtype v);
//UDWtype __udivmoddi4 (UDWtype n, UDWtype d, UDWtype *rp);

// big endian
struct DWstruct {
Wtype high, low;
};

typedef union
{
  struct DWstruct s;
  DWtype ll;
} DWunion;

#define W_TYPE_SIZE 32
#define __ll_B ((UWtype) 1 << (W_TYPE_SIZE / 2))
#define __ll_lowpart(t) ((UWtype) (t) & (__ll_B - 1))
#define __ll_highpart(t) ((UWtype) (t) >> (W_TYPE_SIZE / 2))

#define umul_ppmm(w1, w0, u, v) \
do { \
UWtype __x0, __x1, __x2, __x3; \
UHWtype __ul, __vl, __uh, __vh; \
\
__ul = __ll_lowpart (u); \
__uh = __ll_highpart (u); \
__vl = __ll_lowpart (v); \
__vh = __ll_highpart (v); \
\
__x0 = (UWtype) __ul * __vl; \
__x1 = (UWtype) __ul * __vh; \
__x2 = (UWtype) __uh * __vl; \
__x3 = (UWtype) __uh * __vh; \
\
__x1 += __ll_highpart (__x0);/* this can't give carry */\
__x1 += __x2; /* but this indeed can */\
if (__x1 < __x2) /* did we get it? */\
__x3 += __ll_B; /* yes, add it in the proper pos. */\
\
(w1) = __x3 + __ll_highpart (__x1); \
(w0) = __ll_lowpart (__x1) * __ll_B + __ll_lowpart (__x0); \
} while (0)

#define __umulsidi3(u, v) \
({DWunion __w; \
umul_ppmm (__w.s.high, __w.s.low, u, v); \
__w.ll; })

#endif /* LIBGCC_LM32_H */
