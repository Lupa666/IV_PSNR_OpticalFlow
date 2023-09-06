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
#include "xDistortionSTD.h"

//SSE implementation
#if X_USE_SSE && X_SSE_ALL && __has_include("xDistortionSSE.h")
#define X_CAN_USE_SSE 1
#include "xDistortionSSE.h"
#else
#define X_CAN_USE_SSE 0
#endif

//AVX implementation
#if X_USE_AVX && X_AVX_ALL && __has_include("xDistortionAVX.h")
#define X_CAN_USE_AVX 1
#include "xDistortionAVX.h"
#else
#define X_CAN_USE_AVX 0
#endif


namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xDistortion
{
public:
#if   X_CAN_USE_AVX

  static inline  int32 CalcSD (const uint16* Org, const uint16* Dist,                               int32 Area               ) { return xDistortionAVX::CalcSD (Org, Dist,                   Area          ); }
  static inline  int32 CalcSD (const uint16* Org, const uint16* Dist, int32 OStride, int32 DStride, int32 Width, int32 Height) { return xDistortionAVX::CalcSD (Org, Dist, OStride, DStride, Width,  Height); }
  static inline uint64 CalcSSD(const uint16* Org, const uint16* Dist,                               int32 Area               ) { return xDistortionAVX::CalcSSD(Org, Dist,                   Area          ); }
  static inline uint64 CalcSSD(const uint16* Org, const uint16* Dist, int32 OStride, int32 DStride, int32 Width, int32 Height) { return xDistortionAVX::CalcSSD(Org, Dist, OStride, DStride, Width,  Height); }

#elif X_CAN_USE_SSE

  static inline  int32 CalcSD (const uint16* Org, const uint16* Dist,                               int32 Area               ) { return xDistortionSSE::CalcSD (Org, Dist,                   Area          ); }
  static inline  int32 CalcSD (const uint16* Org, const uint16* Dist, int32 OStride, int32 DStride, int32 Width, int32 Height) { return xDistortionSSE::CalcSD (Org, Dist, OStride, DStride, Width,  Height); }
  static inline uint64 CalcSSD(const uint16* Org, const uint16* Dist,                               int32 Area               ) { return xDistortionSSE::CalcSSD(Org, Dist,                   Area          ); }
  static inline uint64 CalcSSD(const uint16* Org, const uint16* Dist, int32 OStride, int32 DStride, int32 Width, int32 Height) { return xDistortionSSE::CalcSSD(Org, Dist, OStride, DStride, Width,  Height); }

#else //X_CAN_USE_???

  static inline  int32 CalcSD (const uint16* Org, const uint16* Dist,                               int32 Area               ) { return xDistortionSTD::CalcSD (Org, Dist,                   Area          ); }
  static inline  int32 CalcSD (const uint16* Org, const uint16* Dist, int32 OStride, int32 DStride, int32 Width, int32 Height) { return xDistortionSTD::CalcSD (Org, Dist, OStride, DStride, Width,  Height); }
  static inline uint64 CalcSSD(const uint16* Org, const uint16* Dist,                               int32 Area               ) { return xDistortionSTD::CalcSSD(Org, Dist,                   Area          ); }
  static inline uint64 CalcSSD(const uint16* Org, const uint16* Dist, int32 OStride, int32 DStride, int32 Width, int32 Height) { return xDistortionSTD::CalcSSD(Org, Dist, OStride, DStride, Width,  Height); }

#endif //X_CAN_USE_???

  static inline  int64 CalcWeightedSD (const uint16* Org, const uint16* Dist, const uint16* Mask,                                              int32 Area               ) { return xDistortionSTD::CalcWeightedSD (Org, Dist, Mask,                            Area          ); }
  static inline  int64 CalcWeightedSD (const uint16* Org, const uint16* Dist, const uint16* Mask, int32 OStride, int32 DStride, int32 MStride, int32 Width, int32 Height) { return xDistortionSTD::CalcWeightedSD (Org, Dist, Mask, OStride, DStride, MStride, Width,  Height); }
  static inline uint64 CalcWeightedSSD(const uint16* Org, const uint16* Dist, const uint16* Mask,                                              int32 Area               ) { return xDistortionSTD::CalcWeightedSSD(Org, Dist, Mask,                            Area          ); }
  static inline uint64 CalcWeightedSSD(const uint16* Org, const uint16* Dist, const uint16* Mask, int32 OStride, int32 DStride, int32 MStride, int32 Width, int32 Height) { return xDistortionSTD::CalcWeightedSSD(Org, Dist, Mask, OStride, DStride, MStride, Width,  Height); }

};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
