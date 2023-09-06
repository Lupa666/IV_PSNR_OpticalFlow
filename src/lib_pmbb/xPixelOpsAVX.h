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

#if X_USE_AVX && X_AVX_ALL

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xPixelOpsAVX
{
public:
  //Image
  static void  Cvt          (uint16* restrict Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   );
  static void  Cvt          (uint8*  restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   );
  static void  Upsample     (uint16* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight);
  static void  CvtUpsample  (uint16* restrict Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight);
  static bool  CheckValues  (const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth);

  static void  Interleave   (uint16* restrict DstABCD, const uint16* SrcA, const uint16* SrcB, const uint16* SrcC, uint16 ValueD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height);
  static int32 CountNonZero (const uint16* Src, int32 SrcStride, int32 Width, int32 Height);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_USE_AVX && X_AVX_ALL
