#pragma once
/* ############################################################################
The copyright in this software is being made available under the 3-clause BSD
License, included below. This software may be subject to other third party
and contributor rights, including patent rights, and no such rights are
granted under this license.

Author(s):
  * Jakub Stankowski, jakub.stankowski@put.poznan.pl,
    Poznan University of Technology, Poznań, Poland


Copyright (c) 2010-2021, Poznan University of Technology. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
############################################################################ */


//=============================================================================================================================================================================
// MSVC workaround
//=============================================================================================================================================================================
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

//=============================================================================================================================================================================
// base includes
//=============================================================================================================================================================================
#include <type_traits>
#include <algorithm>
#include <string>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cinttypes>
#include <cfloat>
#include <cassert>

#define FMT_HEADER_ONLY
#define FMT_USE_WINDOWS_H 0
#include "fmt/format.h"
#include "fmt/printf.h"
#include "fmt/ostream.h"
#undef FMT_USE_WINDOWS_H
#undef FMT_HEADER_ONLY

//=============================================================================================================================================================================
// Namespace
//=============================================================================================================================================================================
#ifndef PMBB_NAMESPACE
#define PMBB_NAMESPACE PMBB
#endif

//=============================================================================================================================================================================
// Compile time settings
//=============================================================================================================================================================================
#define USE_SIMD  1 // use SIMD (to be precise... use SSE 4.1 or AVX2) 

//=============================================================================================================================================================================
// Hard coded constrains
//=============================================================================================================================================================================
#define NOT_VALID  -1

//=============================================================================================================================================================================
// System section
//=============================================================================================================================================================================
#if (defined(WIN32) || defined(_WIN32) || defined(_WIN64))
#define X_SYSTEM_WINDOWS 1
#elif defined(__linux__)
#define X_SYSTEM_LINUX 1
#else
#define X_SYSTEM_UNKNOWN 0
#endif

//=============================================================================================================================================================================
// Compiler section
//=============================================================================================================================================================================
#if defined __INTEL_COMPILER
#define X_COMPILER_ICC 1
#define X_COMPILER_VER  __INTEL_COMPILER
#define X_COMPILER_NAME "Intel Compiler"
#elif defined __clang__
#define X_COMPILER_CLANG 1
#define X_COMPILER_VER  __clang_major__
#define X_COMPILER_NAME "Clang"
#elif defined __GNUC__
#define X_COMPILER_GCC 1
#define X_COMPILER_VER  __GNUC__
#define X_COMPILER_NAME "GNU Compiler Collection"
#elif defined _MSC_VER
#define X_COMPILER_MSVC 1
#define X_COMPILER_VER  _MSC_VER
#define X_COMPILER_NAME "MS Visual Studio"
#else
#define X_COMPILER_VER  
#define X_COMPILER_NAME "unknown"
#endif

//=============================================================================================================================================================================
// C99 restrict pointers support
//=============================================================================================================================================================================
#if defined(_MSC_VER) && _MSC_VER >= 1400 //MSVC
#define restrict __restrict
#elif defined(__GNUC__) && ((__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)) //GCC
#ifdef __cplusplus
#define restrict __restrict
#endif
#ifdef __STDC_VERSION__ //STANDARD_C_1990
#define restrict __restrict
#endif
#if (__STDC_VERSION__ >= 199409L) //STANDARD_C_1994
#define restrict __restrict
#endif
#if (__STDC_VERSION__ >= 199901L) //STANDARD_C_1999
//restrict defined
#endif
#elif defined(__STDC__) //STANDARD_C_1989
#define restrict
#endif

//=============================================================================================================================================================================
// SIMD section
//=============================================================================================================================================================================
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_AMD64) || defined(_M_IX86))
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif

//MSVC does not define __SSEn__ macros. Assuming all extensions present.
#if defined(_MSC_VER)
#define __SSE__    1
#define __SSE2__   1
#define __SSE3__   1
#define __SSSE3__  1
#define __SSE4_1__ 1
#define __SSE4_2__ 1
#endif

//SSE
//SSE - since Pentium III
#if defined(__SSE__) 
#define X_SSE1 __SSE__ 
#else
#define X_SSE1 0
#endif
//SSE2 - since Pentium 4 (Willamette, Northwood, Gallatin)
#if defined(__SSE2__) 
#define X_SSE2 __SSE2__ 
#else
#define X_SSE2 0
#endif
//SSE3 - since Pentium 4 (Prescott, Cedar Mill)
#if defined(__SSE3__) 
#define X_SSE3 __SSE3__ 
#else
#define X_SSE3 0
#endif
//SSSE3 - since Core 2 (Conroe, Merom)
#if defined(__SSSE3__) 
#define X_SSSE3 __SSSE3__ 
#else
#define X_SSSE3 0
#endif
//SSE4_1 - since Core 2 (Penryn, Wolfdale)
#if defined(__SSE4_1__) 
#define X_SSE4_1 __SSE4_1__ 
#else
#define X_SSE4_1 0
#endif
//SSE4_1 - since Core iX nnn (Nehalem, Westmere)
#if defined(__SSE4_2__) 
#define X_SSE4_2 __SSE4_2__ 
#else
#define X_SSE4_2 0
#endif
// all SSEs
#define X_SSE_ALL (X_SSE1 && X_SSE2 && X_SSE3 && X_SSSE3 && X_SSE4_1 && X_SSE4_2)
#define X_USE_SSE USE_SIMD

//AVX & AVX2
//AVX - since Core iX 2nnn (Sandy Bridge)
#if defined(__AVX__) 
#define X_AVX1 __AVX__ 
#else
#define X_AVX1 0
#endif
//AVX2 - since Core iX 2nnn (Haswell), Ryzen 1xxx (Zen1)
#if defined(__AVX2__) 
#define X_AVX2 __AVX2__ 
#else
#define X_AVX2 0
#endif
//all AVXs
#define X_AVX_ALL (X_AVX1 && X_AVX2)
#define X_USE_AVX USE_SIMD


//=============================================================================================================================================================================
// Integers anf float types
//=============================================================================================================================================================================
typedef  int64_t   int64;
typedef  int32_t   int32;
typedef  int16_t   int16;
typedef  int8_t    int8;

typedef  uint64_t  uint64;
typedef  uint32_t  uint32;
typedef  uint16_t  uint16;
typedef  uint8_t   uint8;

typedef  uint8     byte;

typedef  float     flt32;
typedef  double    flt64;

typedef  uintptr_t uintPtr;
typedef  size_t    uintSize;

static const flt32 flt32_max = std::numeric_limits<flt32>::max    ();
static const flt32 flt32_min = std::numeric_limits<flt32>::lowest ();
static const flt32 flt32_eps = std::numeric_limits<flt32>::epsilon();

static const flt64 flt64_max = std::numeric_limits<flt64>::max    ();
static const flt64 flt64_min = std::numeric_limits<flt64>::lowest ();
static const flt64 flt64_eps = std::numeric_limits<flt64>::epsilon();

//=============================================================================================================================================================================
// Limits
//=============================================================================================================================================================================
static constexpr int64  int64_max  = INT64_MAX;
static constexpr int64  int64_min  = INT64_MIN;
static constexpr int32  int32_max  = INT32_MAX;
static constexpr int32  int32_min  = INT32_MIN;
static constexpr int16  int16_max  = INT16_MAX;
static constexpr int16  int16_min  = INT16_MIN;
static constexpr int8   int8_max   = INT8_MAX ;
static constexpr int8   int8_min   = INT8_MIN ;

static constexpr uint64 uint64_max = UINT64_MAX;
static constexpr uint64 uint64_min = 0         ;
static constexpr uint32 uint32_max = UINT32_MAX;
static constexpr uint32 uint32_min = 0         ;
static constexpr uint16 uint16_max = UINT16_MAX;
static constexpr uint16 uint16_min = 0         ;
static constexpr uint8  uint8_max  = UINT8_MAX ;
static constexpr uint8  uint8_min  = 0         ;

//=============================================================================================================================================================================
// Memmory and alignment section
//=============================================================================================================================================================================
static constexpr int32 xc_Log2MemSizePage = 12; //Memmory page size = 4kB
static constexpr int32 xc_MemSizePage     = (1<<xc_Log2MemSizePage);
static constexpr int32 xc_AlignmentPel    = xc_MemSizePage; //pel alignment

//Allocation with explicit alignment
#if defined(X_COMPILER_MSVC)
#define xAlignedMalloc(size, alignment) _aligned_malloc((size), (alignment))
#define xAlignedFree(memmory)           _aligned_free((memmory))
#else
#define xAlignedMalloc(size, alignment) aligned_alloc((alignment), (size))
#define xAlignedFree(memmory)           free((memmory))
#endif

//Aligned variables/objects
#if defined(X_COMPILER_MSVC)
#define xAligned(x) __declspec(align(x))
#elif defined(X_COMPILER_CLANG) || defined(X_COMPILER_GCC)
#define xAligned(x) __attribute__ ((aligned(x)))
#else
#error Unrecognized compiler
#endif

//=============================================================================================================================================================================
// Basic ops
//=============================================================================================================================================================================
template <class XXX> static inline XXX xMin     (XXX a, XXX b              ) { return std::min(a, b);            }
template <class XXX> static inline XXX xMax     (XXX a, XXX b              ) { return std::max(a, b);            }
template <class XXX> static inline XXX xMin     (XXX a, XXX b, XXX c       ) { return xMin(xMin(a,b),c);         }
template <class XXX> static inline XXX xMax     (XXX a, XXX b, XXX c       ) { return xMax(xMax(a,b),c);         }
template <class XXX> static inline XXX xMin     (XXX a, XXX b, XXX c, XXX d) { return xMin(xMin(a,b),xMin(c,d)); }
template <class XXX> static inline XXX xMax     (XXX a, XXX b, XXX c, XXX d) { return xMax(xMax(a,b),xMax(c,d)); }

template <class XXX> static inline XXX xClip    (XXX x, XXX min, XXX max) { return xMax(min, xMin(x, max)); }
template <class XXX> static inline XXX xClipU   (XXX x, XXX max         ) { return xMax((XXX)0, xMin(x,max));}
template <class XXX> static inline XXX xClipU8  (XXX x                  ) { return xMax((XXX)0,xMin(x,(XXX)255));}
template <class XXX> static inline XXX xClipS8  (XXX x                  ) { return xMax((XXX)-128,xMin(x,(XXX)127));}
template <class XXX> static inline XXX xClipU16 (XXX x                  ) { return xMax((XXX)0,xMin(x,(XXX)65536));}
template <class XXX> static inline XXX xClipS16 (XXX x                  ) { return xMax((XXX)-32768,xMin(x,(XXX)32767));}

template <class XXX> static inline XXX xAbs     (XXX a) { return (XXX)std::abs(a); }
template <class XXX> static inline XXX xPow2    (XXX x) { return x * x; }

template <class XXX> static inline XXX xBitDepth2MidValue(XXX BitDepth) { return (1 << (BitDepth - 1)); }
template <class XXX> static inline XXX xBitDepth2MaxValue(XXX BitDepth) { return ((1 << BitDepth) - 1); }
template <class XXX> static inline XXX xBitDepth2NumLevel(XXX BitDepth) { return (1 << BitDepth); }

template <class XXX> static inline XXX xLog2SizeToSize(XXX Log2Size) { return (1<<Log2Size); } 
template <class XXX> static inline XXX xLog2SizeToArea(XXX Log2Size) { return (1<<((Log2Size)<<1)); } 

template <class XXX> static inline XXX xNumUnitsCoveringLength(XXX Length, XXX Log2UnitSzize) { return ((Length - 1) >> Log2UnitSzize) + 1; }

template <class XXX> static inline XXX xRoundCntrToNearestMultiple(XXX Value, XXX Log2Multiple) { return (((Value + ((1 << Log2Multiple) >> 1)) >> Log2Multiple) << Log2Multiple); } //positive integer only
template <class XXX> static inline XXX xRoundUpToNearestMultiple  (XXX Value, XXX Log2Multiple) { return (((Value + ((1 << Log2Multiple) -  1)) >> Log2Multiple) << Log2Multiple); } //positive integer only
template <class XXX> static inline XXX xRoundDownToNearestMultiple(XXX Value, XXX Log2Multiple) { return (( Value                               >> Log2Multiple) << Log2Multiple); } //positive integer only

//=============================================================================================================================================================================
// flt32/64 to int32 rounding
//=============================================================================================================================================================================
#if X_USE_SSE && X_SSE2
static inline int32 xRoundFlt32ToInt32(flt32 Flt) { return _mm_cvtss_si32(_mm_set_ss(Flt)); }
static inline int32 xRoundFlt64ToInt32(flt64 Flt) { return _mm_cvtsd_si32(_mm_set_sd(Flt)); }
#else
static inline int32 xRoundFlt32ToInt32(flt32 Flt) { return (int32)(std::round(Flt)); }
static inline int32 xRoundFlt64ToInt32(flt64 Flt) { return (int32)(std::round(Flt)); }
#endif
template <class XXX> static inline int32 xRoundFltToInt32(XXX Flt);
template <> inline int32 xRoundFltToInt32(flt32 Flt) { return xRoundFlt32ToInt32 (Flt); }
template <> inline int32 xRoundFltToInt32(flt64 Flt) { return xRoundFlt64ToInt32(Flt); }

//=============================================================================================================================================================================
// flt32/64 madness
//=============================================================================================================================================================================
//https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
//https://stackoverflow.com/questions/17333/what-is-the-most-effective-way-for-float-and-double-comparison

template<typename XXX> static bool xIsApproximatelyEqual(XXX a, XXX b, XXX Tolerance = std::numeric_limits<XXX>::epsilon())
{
  XXX Diff = std::fabs(a - b); if(Diff <= Tolerance || Diff < std::fmax(std::fabs(a), std::fabs(b)) * Tolerance) { return true; }
  return false;
}
template<typename XXX> static inline bool xIsApproximatelyZero(XXX a, XXX Tolerance = std::numeric_limits<XXX>::epsilon()) { return (std::fabs(a) <= Tolerance); }
template<typename XXX> static inline bool xIsApproximatelyOne(XXX a, XXX Tolerance = std::numeric_limits<XXX>::epsilon()) { return (std::fabs(a - (XXX)1.0) <= Tolerance); }

//=============================================================================================================================================================================
// Fast iteger Log2 using bsr (Bit Scan Reverse) x86/x64 instructions, undefined for Value == 0 (same as log2())
//=============================================================================================================================================================================
#if defined(X_COMPILER_MSVC)
static inline uint32 xFastLog2(uint32 Value) { unsigned long Log2; _BitScanReverse  (&Log2, (uint32)Value); return Log2; }
static inline uint64 xFastLog2(uint64 Value) { unsigned long Log2; _BitScanReverse64(&Log2, (uint64)Value); return Log2; }
#elif (X_COMPILER_GCC || X_COMPILER_CLANG)
static inline uint32 xFastLog2(uint32 Value) { return 31 - __builtin_clz(Value); }
static inline uint64 xFastLog2(uint64 Value) { return 63 - __builtin_clzll(Value); }
#else
#error Unrecognized compiler
#endif

//=============================================================================================================================================================================
// type safe memset & memcpy
//=============================================================================================================================================================================
template <class XXX> static inline void xMemsetX(XXX* Dst, const XXX  Val, uint32 Count) { if constexpr(sizeof(XXX) == 1) { std::memset(Dst, Val, Count); } else { for(uint32 i = 0; i < Count; i++) Dst[i] = Val; } }
template <class XXX> static inline void xMemcpyX(XXX* Dst, const XXX* Src, uint32 Count) { std::memcpy(Dst, Src, Count*sizeof(XXX)); }

//=============================================================================================================================================================================
// time is money
//=============================================================================================================================================================================
using tClock      = std::chrono::high_resolution_clock       ;
using tTimePoint  = tClock::time_point                       ;
using tDuration   = tClock::duration                         ;
using tDurationMS = std::chrono::duration<double, std::milli>;
using tDurationS  = std::chrono::duration<double            >;

//=============================================================================================================================================================================
// Math constants
//=============================================================================================================================================================================
template<class XXX> constexpr XXX xc_Pi       = XXX( 3.14159265358979323846L ); // pi
template<class XXX> constexpr XXX xc_2Pi      = XXX( 6.28318530717958647692L ); // 2*pi
template<class XXX> constexpr XXX xc_4Pi      = XXX(12.56637061435917295384L ); // 4*pi
template<class XXX> constexpr XXX xc_PiDiv2   = XXX( 1.57079632679489661923L ); // pi/2
template<class XXX> constexpr XXX xc_1DivPi   = XXX( 0.318309886183790671538L); // 1/pi
template<class XXX> constexpr XXX xc_1Div2Pi  = XXX( 0.15915494309189533577L ); // 1/(2*pi)
template<class XXX> constexpr XXX xc_DegToRad = xc_Pi<XXX> / XXX(180);
template<class XXX> constexpr XXX xc_RadToDeg = XXX(180) / xc_Pi<XXX>;

//=============================================================================================================================================================================
// Multiple and remainder
//=============================================================================================================================================================================
static constexpr uint32 c_MultipleMask4    = 0xFFFFFFFC;
static constexpr uint32 c_MultipleMask8    = 0xFFFFFFF8;
static constexpr uint32 c_MultipleMask16   = 0xFFFFFFF0;
static constexpr uint32 c_MultipleMask32   = 0xFFFFFFE0;
static constexpr uint32 c_MultipleMask64   = 0xFFFFFFC0;
static constexpr uint32 c_MultipleMask128  = 0xFFFFFF80;

static constexpr uint32 c_RemainderMask4   = 0x00000003;
static constexpr uint32 c_RemainderMask8   = 0x00000007;
static constexpr uint32 c_RemainderMask16  = 0x0000000F;
static constexpr uint32 c_RemainderMask32  = 0x0000001F;
static constexpr uint32 c_RemainderMask64  = 0x0000003F;
static constexpr uint32 c_RemainderMask128 = 0x0000007F;

//=============================================================================================================================================================================
// Common enums
//=============================================================================================================================================================================
enum class eCmp : int32
{
  INVALID = NOT_VALID,

  //generic
  C0 = 0,
  C1 = 1,
  C2 = 2,
  C3 = 3,

  //Y Cb Cr
  LM = 0,     //Luma      (Y)
  CB = 1,     //Chroma Cb (U)
  CR = 2,     //Chroma Cr (V)

  //R G B
  R = 0,
  G = 1,
  B = 2,
};

//=============================================================================================================================================================================

