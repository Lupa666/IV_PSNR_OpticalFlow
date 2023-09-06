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


#include "xDistortionSTD.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

int32 xDistortionSTD::CalcSD(const uint16* restrict Org, const uint16* restrict Dist, int32 Area)
{
  int32 SD = 0;
  for(int32 i=0; i < Area; i++) { SD += (int32)Org[i] - (int32)Dist[i]; }
  return SD;
}
int32 xDistortionSTD::CalcSD(const uint16* restrict Org, const uint16* restrict Dist, int32 OStride, int32 DStride, int32 Width, int32 Height)
{
  int32 SD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { SD += (int32)Org[x] - (int32)Dist[x]; }
    Org  += OStride;
    Dist += DStride;
  }
  return SD;
}
uint64 xDistortionSTD::CalcSSD(const uint16* restrict Org, const uint16* restrict Dist, int32 Area)
{
  uint64 SSD = 0;
  for(int32 i=0; i < Area; i++) { SSD += (uint64)xPow2(((int32)Org[i]) - ((int32)Dist[i])); }
  return SSD;
}
uint64 xDistortionSTD::CalcSSD(const flt32V2* restrict Org, const flt32V2* restrict Dist, int32 Area)
{
    uint64 SSD = 0;
    for (int32 i = 0; i < Area; i++) { 
        flt32V2 NewVec = (Org[i] - Dist[i]);
        SSD += (uint64)(xPow2(NewVec[0]) + xPow2(NewVec[1]));
    }
    return SSD;
}
uint64 xDistortionSTD::CalcSSD(const uint16* restrict Org, const uint16* restrict Dist, int32 OStride, int32 DStride, int32 Width, int32 Height)
{
  uint64 SSD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { SSD += (uint64)xPow2(((int32)Org[x]) - ((int32)Dist[x])); }
    Org  += OStride;
    Dist += DStride;
  }
  return SSD;
}

int64 xDistortionSTD::CalcWeightedSD(const uint16* restrict Org, const uint16* restrict Dist, const uint16* restrict Mask, int32 Area)
{
  int64 SD = 0;
  for(int32 i=0; i < Area; i++) { SD += ((int32)Org[i] - (int32)Dist[i]) * (int32)Mask[i]; }
  return SD;
}
int64 xDistortionSTD::CalcWeightedSD(const uint16* restrict Org, const uint16* restrict Dist, const uint16* restrict Mask, int32 OStride, int32 DStride, int32 MStride, int32 Width, int32 Height)
{
  int64 SD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { SD += ((int32)Org[x] - (int32)Dist[x]) * (int32)Mask[x]; }
    Org  += OStride;
    Dist += DStride;
    Mask += MStride;
  }
  return SD;
}
uint64 xDistortionSTD::CalcWeightedSSD(const uint16* restrict Org, const uint16* restrict Dist, const uint16* restrict Mask, int32 Area)
{
  uint64 SSD = 0;
  for(int32 i=0; i < Area; i++) { SSD += ((uint64)xPow2(((int32)Org[i]) - ((int32)Dist[i]))) * (uint64)Mask[i]; }
  return SSD;
}
uint64 xDistortionSTD::CalcWeightedSSD(const uint16* restrict Org, const uint16* restrict Dist, const uint16* restrict Mask, int32 OStride, int32 DStride, int32 MStride, int32 Width, int32 Height)
{
  uint64 SSD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { SSD += ((uint64)xPow2(((int32)Org[x]) - ((int32)Dist[x]))) * (uint64)Mask[x]; }
    Org  += OStride;
    Dist += DStride;
    Mask += MStride;
  }
  return SSD;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
