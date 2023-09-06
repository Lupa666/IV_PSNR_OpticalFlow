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


#include "xPic.h"
#include "xPixelOps.h"
#include <cassert>
#include <cstring>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xPicCommon
//===============================================================================================================================================================================================================
void xPicCommon::xInit(int32V2 Size, int32 BitDepth, int32 Margin, int32 NumCmps)
{
  assert(Size.getX()>0 && Size.getY()>0 && Margin>=0 && NumCmps>0 && NumCmps<=c_MaxNumCmps);

  m_Width    = Size.getX();
  m_Height   = Size.getY();
  m_Margin   = Margin;
  m_Stride   = m_Width + (m_Margin << 1);
  m_BitDepth = BitDepth;
  m_NumCmps  = NumCmps;

  m_BytesPerSample  = m_BitDepth <= 8 ? 1 : 2;
  m_BuffCmpNumPels  = (m_Width + (m_Margin << 1)) * (m_Height + (m_Margin << 1));
  m_BuffCmpNumBytes = m_BuffCmpNumPels * sizeof(uint16);

  m_IsMarginExtended = false;
}
void xPicCommon::xUnInit()
{
  m_Width            = NOT_VALID;
  m_Height           = NOT_VALID;
  m_Margin           = NOT_VALID;
  m_Stride           = NOT_VALID;
  m_BitDepth         = NOT_VALID;
  m_NumCmps          = NOT_VALID;

  m_BytesPerSample   = NOT_VALID;
  m_BuffCmpNumPels   = NOT_VALID;
  m_BuffCmpNumBytes  = NOT_VALID;

  m_POC              = NOT_VALID;
  m_Timestamp        = NOT_VALID;

  m_IsMarginExtended = false;
}

//===============================================================================================================================================================================================================
// xPicP 
//===============================================================================================================================================================================================================
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// xPicP - general functions
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void xPicP::create(int32V2 Size, int32 BitDepth, int32 Margin)
{
  xInit(Size, BitDepth, Margin, c_DefNumCmps);

  for(int32 c = 0; c < m_NumCmps; c++)
  {
    m_Buffer[c] = (uint16*)xAlignedMalloc(m_BuffCmpNumBytes, xc_AlignmentPel);
    m_Origin[c] = m_Buffer[c] + (m_Margin * m_Stride) + m_Margin;
  }  
}
void xPicP::destroy()
{
  for(int32 c = 0; c < m_NumCmps; c++)
  {
    if(m_Buffer[c] != nullptr) { xAlignedFree(m_Buffer[c]); m_Buffer[c] = nullptr; }
    m_Origin[c] = nullptr;
  }
  xUnInit();
}
void xPicP::clear()
{
  for(int32 c=0; c < m_NumCmps; c++) { memset(m_Buffer[c], 0, m_BuffCmpNumBytes); }
  m_POC              = NOT_VALID;
  m_Timestamp        = NOT_VALID;
  m_IsMarginExtended = false;
}
void xPicP::copy(const xPicP* Src)
{
  assert(Src!=nullptr && isCompatible(Src));
  for(int32 c=0; c < m_NumCmps; c++) { memcpy(m_Buffer[c], Src->m_Buffer[c], m_BuffCmpNumBytes); }
  m_IsMarginExtended = Src->m_IsMarginExtended;
}
void xPicP::set(uint16 Value)
{
  for(int32 c = 0; c < m_NumCmps; c++) { set(Value, (eCmp)c); }
  m_IsMarginExtended = true;
}
bool xPicP::check(const std::string& Name)
{
  boolV4 Correct = xMakeVec4(true);
  for(int32 CmpIdx = 0; CmpIdx < m_NumCmps; CmpIdx++)
  { 
    Correct[CmpIdx] = xPixelOps::CheckValues(m_Origin[CmpIdx], m_Stride, m_Width, m_Height, m_BitDepth);
  }

  for(int32 CmpIdx = 0; CmpIdx < m_NumCmps; CmpIdx++)
  {
    if(!Correct[CmpIdx])
    {
      fmt::printf("FILE BROKEN " + Name + " (CMP=%d)\n", CmpIdx);
      xPixelOps::FindBroken(m_Origin[CmpIdx], m_Stride, m_Width, m_Height, m_BitDepth);
      return false;
    }
  }

  return true;
}
void xPicP::extend()
{
  for(int32 CmpIdx = 0; CmpIdx < m_NumCmps; CmpIdx++) { xPixelOps::ExtendMargin(m_Origin[CmpIdx], m_Stride, m_Width, m_Height, m_Margin); }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//low level buffer modification / access - dangerous
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool xPicP::bindBuffer(uint16* Buffer, eCmp CmpId)
{
  assert(Buffer!=nullptr); if(m_Buffer[(int32)CmpId]) { return false; }
  m_Buffer[(int32)CmpId] = Buffer;
  m_Origin[(int32)CmpId] = m_Buffer[(int32)CmpId] + (m_Margin * m_Stride) + m_Margin;
  return true;
}
uint16* xPicP::unbindBuffer(eCmp CmpId)
{
  if(m_Buffer[(int32)CmpId]==nullptr) { return nullptr; }
  uint16* Tmp = m_Buffer[(int32)CmpId];
  m_Buffer[(int32)CmpId] = nullptr;
  m_Origin[(int32)CmpId] = nullptr;
  return Tmp;
}
bool xPicP::swapBuffer(uint16*& Buffer, eCmp CmpId)
{
  assert(Buffer!=nullptr); if(m_Buffer[(int32)CmpId]==nullptr) { return false; }
  std::swap(m_Buffer[(int32)CmpId], Buffer);
  m_Origin[(int32)CmpId] = m_Buffer[(int32)CmpId] + (m_Margin * m_Stride) + m_Margin;
  return true;
}
bool xPicP::swapBuffer(xPicP* TheOther, eCmp CmpId)
{
  assert(TheOther != nullptr); if(TheOther==nullptr || !isCompatible(TheOther)) { return false; }
  std::swap(this->m_Buffer[(int32)CmpId], TheOther->m_Buffer[(int32)CmpId]);
  this    ->m_Origin[(int32)CmpId] = this    ->m_Buffer[(int32)CmpId] + (this    ->m_Margin * this    ->m_Stride) + this    ->m_Margin;
  TheOther->m_Origin[(int32)CmpId] = TheOther->m_Buffer[(int32)CmpId] + (TheOther->m_Margin * TheOther->m_Stride) + TheOther->m_Margin;
  return true;
}
bool xPicP::swapBuffers(xPicP* TheOther)
{
  for(int32 c = 0; c < m_NumCmps; c++)
  { 
    bool Result = swapBuffer(TheOther, (eCmp)c);
    if(!Result) { return false; }
  }
  return true;
}


//===============================================================================================================================================================================================================
// xPicI
//===============================================================================================================================================================================================================
void xPicI::create(int32V2 Size, int32 BitDepth, int32 Margin)
{
  xInit(Size, BitDepth, Margin, c_DefNumCmps);

  m_Buffer = (uint16*)xAlignedMalloc(m_BuffCmpNumBytes * c_MaxNumCmps, xc_AlignmentPel);
  m_Origin = m_Buffer + (m_Margin * (m_Stride << 2)) + (m_Margin << 2);
}
void xPicI::destroy()
{
  xAlignedFree(m_Buffer); m_Buffer = nullptr;
  m_Origin = nullptr;

  xUnInit();
}
void xPicI::rearrangeFromPlanar(const xPicP* Planar)
{
  assert(isCompatible(Planar));
  const int32 ExtWidth  = m_Width  + (m_Margin << 1);
  const int32 ExtHeight = m_Height + (m_Margin << 1);
  xPixelOps::Interleave(m_Buffer, Planar->getBuffer(eCmp::C0), Planar->getBuffer(eCmp::C1), Planar->getBuffer(eCmp::C2), 0, m_Stride * c_MaxNumCmps, Planar->getStride(), ExtWidth, ExtHeight);
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
