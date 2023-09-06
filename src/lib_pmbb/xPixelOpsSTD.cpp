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


#include "xPixelOpsSTD.h"
#include "xVec.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xPixelOpsSTD::Copy(uint16* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  for(int32 y = 0; y < Height; y++)
  {
    ::memcpy(Dst, Src, Width * sizeof(uint16));
    Src += SrcStride;
    Dst += DstStride;
  }
}
void xPixelOpsSTD::Cvt(uint16* restrict Dst, const uint8* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++) { Dst[x] = Src[x]; }
    Src += SrcStride;
    Dst += DstStride;
  }
}
void xPixelOpsSTD::Cvt(uint8* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++) { Dst[x] = (uint8)xClipU8<uint16>(Src[x]); }
    Src += SrcStride;
    Dst += DstStride;
  }
}
void xPixelOpsSTD::Upsample(uint16* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  uint16* restrict DstL0 = Dst;
  uint16* restrict DstL1 = Dst + DstStride;

  for(int32 y=0; y<DstHeight; y+=2)
  {
    for(int32 x=0; x<DstWidth; x+=2)
    {
      const uint16 S = Src[x>>1];
      DstL0[x  ] = S;
      DstL0[x+1] = S;
      DstL1[x  ] = S;
      DstL1[x+1] = S;
    }
    Src   += SrcStride;
    DstL0 += (DstStride << 1);
    DstL1 += (DstStride << 1);
  }
}
void xPixelOpsSTD::Downsample(uint16* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  const uint16* SrcL0 = Src;
  const uint16* SrcL1 = Src + SrcStride;

  for(int32 y=0; y<DstHeight; y++)
  {
    for(int32 x=0; x<DstWidth; x++)
    {
      const int32 SrcX = x << 1;
      int32 D = ((int32)SrcL0[SrcX] + (int32)SrcL0[SrcX + 1] + (int32)SrcL1[SrcX] + (int32)SrcL1[SrcX + 1] + 2) >> 2;
      Dst[x] = (uint16)D;
    }
    Dst   += DstStride;
    SrcL0 += (SrcStride << 1);
    SrcL1 += (SrcStride << 1);
  }
}
void xPixelOpsSTD::CvtUpsample(uint16* restrict Dst, const uint8* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  uint16* restrict DstL0 = Dst;
  uint16* restrict DstL1 = Dst + DstStride;

  for(int32 y=0; y<DstHeight; y+=2)
  {
    for(int32 x=0; x<DstWidth; x+=2)
    {
      uint16 S = Src[x>>1];
      DstL0[x  ] = S;
      DstL0[x+1] = S;
      DstL1[x  ] = S;
      DstL1[x+1] = S;
    }
    Src   += SrcStride;
    DstL0 += (DstStride << 1);
    DstL1 += (DstStride << 1);
  }
}
void xPixelOpsSTD::CvtDownsample(uint8* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  const uint16* SrcL0 = Src;
  const uint16* SrcL1 = Src + SrcStride;

  for(int32 y=0; y<DstHeight; y++)
  {
    for(int32 x=0; x<DstWidth; x++)
    {
      const int32 SrcX = x << 1;
      int32 D = ((int32)SrcL0[SrcX] + (int32)SrcL0[SrcX + 1] + (int32)SrcL1[SrcX] + (int32)SrcL1[SrcX + 1] + 2) >> 2;
      Dst[x] = (uint8)xClip<int32>(D, 0, 255);
    }
    Dst   += DstStride;
    const int32 SrcStrideMul2 = SrcStride << 1;
    SrcL0 += SrcStrideMul2;
    SrcL1 += SrcStrideMul2;
  }
}
bool xPixelOpsSTD::CheckValues(const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth)
{
  if(BitDepth == 16) { return true; }

  const int32 MaxValue = xBitDepth2MaxValue(BitDepth);
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++) { if(Src[x] > MaxValue) { return false; } }
    Src += SrcStride;
  }
  return true;
}
bool xPixelOpsSTD::FindBroken(const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth)
{
  const int32 MaxValue = xBitDepth2MaxValue(BitDepth);
  bool  Correct = true;
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++)
    {
      if(Src[x] > MaxValue) { fmt::printf("FILE BROKEN (y=%d, x=%d, VALUE=%d, Expected=[0-%d])\n", y, x, Src[x], MaxValue); Correct = false; }
    }
    Src += SrcStride;
  }
  return Correct;
}
void xPixelOpsSTD::ExtendMargin(uint16* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin)
{
  //left/right
  for(int32 y = 0; y < Height; y++)
  {
    uint16 Left  = Addr[0];
    uint16 Right = Addr[Width - 1];
    for(int32 x = 0; x < Margin; x++)
    {
      Addr[x - Margin] = Left;
      Addr[x + Width ] = Right;
    }
    Addr += Stride;
  }
  //below
  Addr -= (Stride + Margin);
  for(int32 y = 0; y < Margin; y++)
  {
    ::memcpy(Addr + (y + 1) * Stride, Addr, sizeof(uint16) * (Width + (Margin << 1)));
  }
  //above
  Addr -= ((Height - 1) * Stride);
  for(int32 y = 0; y < Margin; y++)
  {
    ::memcpy(Addr - (y + 1) * Stride, Addr, sizeof(uint16) * (Width + (Margin << 1)));
  }
}
void xPixelOpsSTD::ExtendMargin(flt32V2* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin)
{
    //left/right
    for (int32 y = 0; y < Height; y++)
    {
        flt32V2 Left = Addr[0];
        flt32V2 Right = Addr[Width - 1];
        for (int32 x = 0; x < Margin; x++)
        {
            Addr[x - Margin] = Left;
            Addr[x + Width] = Right;
        }
        Addr += Stride;
    }
    //below
    Addr -= (Stride + Margin);
    for (int32 y = 0; y < Margin; y++)
    {
        ::memcpy(Addr + (y + 1) * Stride, Addr, sizeof(flt32V2) * (Width + (Margin << 1)));
    }
    //above
    Addr -= ((Height - 1) * Stride);
    for (int32 y = 0; y < Margin; y++)
    {
        ::memcpy(Addr - (y + 1) * Stride, Addr, sizeof(flt32V2) * (Width + (Margin << 1)));
    }
}
void xPixelOpsSTD::Interleave(uint16* restrict DstABCD, const uint16* SrcA, const uint16* SrcB, const uint16* SrcC, uint16 ValueD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++)
    {
      DstABCD[(x<<2)+0] = SrcA[x];
      DstABCD[(x<<2)+1] = SrcB[x];
      DstABCD[(x<<2)+2] = SrcC[x];
      DstABCD[(x<<2)+3] = ValueD;
    }
    SrcA    += SrcStride;
    SrcB    += SrcStride;
    SrcC    += SrcStride;
    DstABCD += DstStride;
  }
}
int32 xPixelOpsSTD::CountNonZero(const uint16* Src, int32 SrcStride, int32 Width, int32 Height)
{
  int32 NumNonZero = 0;

  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++) { if(Src[x] != 0) { NumNonZero++; }
    }
    Src += SrcStride;
  }

  return NumNonZero;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
