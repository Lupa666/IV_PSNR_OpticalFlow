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


#include "xCommonDefPMBB.h"
#include "xVec.h"

//portable implementation
#include "xPixelOpsSTD.h"

//SSE implementation
#if X_USE_SSE && X_SSE_ALL && __has_include("xPixelOpsSSE.h")
#define X_CAN_USE_SSE 1
#include "xPixelOpsSSE.h"
#else
#define X_CAN_USE_SSE 0
#endif

//AVX implementation
#if X_USE_AVX && X_AVX_ALL && __has_include("xPixelOpsAVX.h")
#define X_CAN_USE_AVX 1
#include "xPixelOpsAVX.h"
#else
#define X_CAN_USE_AVX 0
#endif


namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xPixelOps
{
public:
  //===============================================================================================================================================================================================================
  // Copy
  //===============================================================================================================================================================================================================
  template <typename PelType> static inline void Copy(PelType* Dst, const PelType* Src, int32 Area);
  template <typename PelType> static inline void Copy(PelType* Dst, const PelType* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height);
  template <typename PelType> static inline void CopyPart(PelType* Dst, const PelType* Src, int32 DstStride, int32 SrcStride, int32V2 DstCoord, int32V2 SrcCoord, int32V2 Size);


public:  
  static inline void Copy         (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsSTD::Copy         (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void Downsample   (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSTD::Downsample   (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void CvtDownsample(uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSTD::CvtDownsample(Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline bool FindBroken   (const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth) { return xPixelOpsSTD::FindBroken(Src, SrcStride, Width, Height, BitDepth); }
  static inline void ExtendMargin (uint16* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin) { xPixelOpsSTD::ExtendMargin(Addr, Stride, Width, Height, Margin); }
  static inline void ExtendMargin (flt32V2* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin) { xPixelOpsSTD::ExtendMargin(Addr, Stride, Width, Height, Margin); }

#if   X_CAN_USE_AVX
  
  static inline void  Cvt          (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsAVX::Cvt          (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void  Cvt          (uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsAVX::Cvt          (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void  Upsample     (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsAVX::Upsample     (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtUpsample  (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsAVX::CvtUpsample  (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  
  static inline bool  CheckValues  (const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth) { return xPixelOpsAVX::CheckValues(Src, SrcStride, Width, Height, BitDepth); }
  static inline void  Interleave   (uint16* DstABCD, const uint16* SrcA, const uint16* SrcB, const uint16* SrcC, uint16 ValueD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height) { xPixelOpsAVX::Interleave(DstABCD, SrcA, SrcB, SrcC, ValueD, DstStride, SrcStride, Width, Height); }

  static inline int32 CountNonZero (const uint16* Src, int32 SrcStride, int32 Width, int32 Height) { return xPixelOpsAVX::CountNonZero(Src, SrcStride, Width, Height); }

#elif X_CAN_USE_SSE

  static inline void  Cvt          (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsSSE::Cvt          (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void  Cvt          (uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsSSE::Cvt          (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void  Upsample     (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSSE::Upsample     (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtUpsample  (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSSE::CvtUpsample  (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  
  static inline bool  CheckValues  (const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth) { return xPixelOpsSSE::CheckValues(Src, SrcStride, Width, Height, BitDepth); }
  static inline void  Interleave   (uint16* DstABCD, const uint16* SrcA, const uint16* SrcB, const uint16* SrcC, uint16 ValueD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height) { xPixelOpsSSE::Interleave(DstABCD, SrcA, SrcB, SrcC, ValueD, DstStride, SrcStride, Width, Height); }

  static inline int32 CountNonZero (const uint16* Src, int32 SrcStride, int32 Width, int32 Height) { return xPixelOpsSSE::CountNonZero(Src, SrcStride, Width, Height); }

#else //X_CAN_USE_???

  static inline void  Cvt          (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsSTD::Cvt          (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void  Cvt          (uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsSTD::Cvt          (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void  Upsample     (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSTD::Upsample     (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtUpsample  (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSTD::CvtUpsample  (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  
  static inline bool  CheckValues  (const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth) { return xPixelOpsSTD::CheckValues(Src, SrcStride, Width, Height, BitDepth); }
  static inline void  Interleave   (uint16* DstABCD, const uint16* SrcA, const uint16* SrcB, const uint16* SrcC, uint16 ValueD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height) { xPixelOpsSTD::Interleave(DstABCD, SrcA, SrcB, SrcC, ValueD, DstStride, SrcStride, Width, Height); }

  static inline int32 CountNonZero (const uint16* Src, int32 SrcStride, int32 Width, int32 Height) { return xPixelOpsSTD::CountNonZero(Src, SrcStride, Width, Height); }

#endif //X_CAN_USE_???
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Copy
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template <typename PelType> void xPixelOps::Copy(PelType* Dst, const PelType* Src, int32 Area)
{
  ::memcpy(Dst, Src, Area * sizeof(PelType));
}
template <typename PelType> void xPixelOps::Copy(PelType* Dst, const PelType* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  for(int32 y = 0; y < Height; y++)
  {
    ::memcpy(Dst, Src, Width * sizeof(PelType));
    Src += SrcStride; Dst += DstStride;
  }
}
template <typename PelType> void xPixelOps::CopyPart(PelType* Dst, const PelType* Src, int32 DstStride, int32 SrcStride, int32V2 DstCoord, int32V2 SrcCoord, int32V2 Size)
{
  for(int32 y = 0; y < Size.getY(); y++)
  {
    const PelType* const SrcLine = Src + (SrcCoord.getY() + y) * SrcStride + SrcCoord.getX();
          PelType* const DstLine = Dst + (DstCoord.getY() + y) * DstStride + DstCoord.getX();
    ::memcpy(DstLine, SrcLine, Size.getX() * sizeof(PelType));
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
