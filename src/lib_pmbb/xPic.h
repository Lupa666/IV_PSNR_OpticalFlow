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

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xPicCommon
//===============================================================================================================================================================================================================
class xPicCommon
{
public:
  static constexpr int32 c_MaxNumCmps = 4; //maximum number of components - do non change
  static constexpr int32 c_DefNumCmps = 3;
  static constexpr int32 c_DefMargin  = 4;

protected:
  int32   m_Width            = NOT_VALID;
  int32   m_Height           = NOT_VALID;
  int32   m_Margin           = NOT_VALID;
  int32   m_Stride           = NOT_VALID;
  int32   m_BitDepth         = NOT_VALID;
  int32   m_NumCmps          = NOT_VALID;

  int32   m_BytesPerSample   = NOT_VALID;
  int32   m_BuffCmpNumPels   = NOT_VALID; //size of picture buffer (including margin) in number of PELs
  int32   m_BuffCmpNumBytes  = NOT_VALID; //size of picture buffer (including margin) in bytes

  int64   m_POC              = NOT_VALID;
  int64   m_Timestamp        = NOT_VALID;

  bool    m_IsMarginExtended = false;

protected:
  void xInit  (int32V2 Size, int32 BitDepth, int32 Margin, int32 NumCmps);
  void xInit  (int32V2 Size, int32 BitDepth, int32 Margin, int32 NumCmps, int32 PelTypeSize);
  void xUnInit();

public:
  //inter-buffer compatibility functions
  inline bool isSameSize      (int32 Width, int32 Height              ) const { return (m_Width == Width && m_Height == Height); }
  inline bool isSameMargin    (int32 Margin                           ) const { return (m_Margin == Margin); }
  inline bool isSameSizeMargin(int32 Width, int32 Height, int32 Margin) const { return isSameSize(Width, Height) && isSameMargin(Margin); }
  inline bool isSameBitDepth  (int32 BitDepth                         ) const { return m_BitDepth == BitDepth; }
  inline bool isSameNumCmps   (int32 NumCmps                          ) const { return m_NumCmps == NumCmps; }

  inline bool isSameSize      (const xPicCommon* Pic) const { return isSameSize(Pic->m_Width, Pic->m_Height); }
  inline bool isSameMargin    (const xPicCommon* Pic) const { return isSameMargin(Pic->getMargin()); }
  inline bool isSameSizeMargin(const xPicCommon* Pic) const { return isSameSize(Pic) && isSameMargin(Pic); }
  inline bool isSameBitDepth  (const xPicCommon* Pic) const { return isSameBitDepth(Pic->m_BitDepth); }
  inline bool isSameNumCmps   (const xPicCommon* Pic) const { return isSameNumCmps(Pic->m_NumCmps); }
  inline bool isCompatible    (const xPicCommon* Pic) const { return isSameSizeMargin(Pic) && isSameBitDepth(Pic) && isSameNumCmps(Pic); }

  //parameters
  inline int32V2 getSize    () const { return int32V2(m_Width, m_Height); }
  inline int32   getWidth   () const { return m_Width           ; }
  inline int32   getHeight  () const { return m_Height          ; }
  inline int32   getArea    () const { return m_Width * m_Height; }
  inline int32   getMargin  () const { return m_Margin          ; }
  inline int32   getBitDepth() const { return m_BitDepth        ; }
  inline int32   getNumCmps () const { return m_NumCmps         ; }

  //time
  inline void  setPOC      (int64 POC      )       { m_POC = POC; }
  inline int64 getPOC      (               ) const { return m_POC; }
  inline void  setTimestamp(int64 Timestamp)       { m_Timestamp = Timestamp; }
  inline int64 getTimestamp(               ) const { return m_Timestamp; }

  //data type/range helpers
  inline uint16  getMaxPelValue() const { return (uint16)xBitDepth2MaxValue(m_BitDepth); }
  inline uint16  getMidPelValue() const { return (uint16)xBitDepth2MidValue(m_BitDepth); }
  inline uint16  getMinPelValue() const { return (uint16)0;                              }
};

//===============================================================================================================================================================================================================
// xPicP - planar
//===============================================================================================================================================================================================================
class xPicP : public xPicCommon
{
protected:
  uint16* m_Buffer[c_MaxNumCmps] = { nullptr, nullptr, nullptr, nullptr }; //picture buffer
  uint16* m_Origin[c_MaxNumCmps] = { nullptr, nullptr, nullptr, nullptr }; //pel origin, pel access -> m_PelOrg[y*m_PelStride + x]

public:
  //general functions
  xPicP () { };
  xPicP (int32V2 Size, int32 BitDepth, int32 Margin = c_DefMargin) { create(Size, BitDepth, Margin); }
  ~xPicP() { destroy(); }

  void   create (int32V2 Size, int32 BitDepth, int32 Margin = c_DefMargin);
  void   create (const xPicP *Ref) { create(Ref->getSize(), Ref->getBitDepth(), Ref->getMargin()); }
  void   destroy();

  void   clear ();
  void   copy  (const xPicP* Src            );
  void   copy  (const xPicP* Src, eCmp CmpId) { assert(isCompatible(Src)); xMemcpyX(m_Buffer[(int32)CmpId], Src->m_Buffer[(int32)CmpId], m_BuffCmpNumPels); }
  void   set   (uint16 Value                );
  void   set   (uint16 Value    , eCmp CmpId) { xMemsetX<uint16>(m_Buffer[(int32)CmpId], Value, m_BuffCmpNumPels); }
  bool   check (const std::string& Name);
  void   extend();

  //access picture data
  inline int32         getStride(                            ) const { return m_Stride              ; }
  inline int32         getPitch (                            ) const { return 1                     ; }  
  inline uint16*       getAddr  (                  eCmp CmpId)       { return m_Origin[(int32)CmpId]; }
  inline const uint16* getAddr  (                  eCmp CmpId) const { return m_Origin[(int32)CmpId]; }
  inline int32         getOffset(int32V2 Position            ) const { return Position.getY() * m_Stride + Position.getX(); }
  inline uint16*       getAddr  (int32V2 Position, eCmp CmpId)       { return getAddr(CmpId) + getOffset(Position); }
  inline const uint16* getAddr  (int32V2 Position, eCmp CmpId) const { return getAddr(CmpId) + getOffset(Position); }
  //slow pel access
  inline uint16&       accessPel(int32V2 Position, eCmp CmpId)       { return *(getAddr(CmpId) + getOffset(Position)); }
  inline const uint16& accessPel(int32V2 Position, eCmp CmpId) const { return *(getAddr(CmpId) + getOffset(Position)); }
  inline uint16&       accessPel(int32   Offset  , eCmp CmpId)       { return *(getAddr(CmpId) + Offset); }
  inline const uint16& accessPel(int32   Offset  , eCmp CmpId) const { return *(getAddr(CmpId) + Offset); }

  //low level buffer modification / access - dangerous
  inline int32         getBuffNumPels(          ) const { return m_BuffCmpNumPels ; }
  inline int32         getBuffSize   (          ) const { return m_BuffCmpNumBytes; }
  inline uint16*       getBuffer     (eCmp CmpId)       { return m_Buffer[(int32)CmpId]; }
  inline const uint16* getBuffer     (eCmp CmpId) const { return m_Buffer[(int32)CmpId]; }
         bool          bindBuffer    (uint16*  Buffer, eCmp CmpId);
         uint16*       unbindBuffer  (                 eCmp CmpId);
         bool          swapBuffer    (uint16*& Buffer, eCmp CmpId);
         bool          swapBuffer    (xPicP* TheOther, eCmp CmpId);
         bool          swapBuffers   (xPicP* TheOther);
};

//===============================================================================================================================================================================================================
// xPicI - interleaved
//===============================================================================================================================================================================================================
class xPicI : public xPicCommon
{
protected:
  uint16* m_Buffer = nullptr;
  uint16* m_Origin = nullptr;

public:
  //general functions
  xPicI () { };
  xPicI (int32V2 Size, int32 BitDepth, int32 Margin = c_DefMargin) { create(Size, BitDepth, Margin); }
  ~xPicI() { destroy(); }

  void   create (int32V2 Size, int32 BitDepth, int32 Margin = c_DefMargin);
  void   create (const xPicI* Ref) { create(Ref->getSize(), Ref->getBitDepth(), Ref->getMargin()); }
  void   destroy();

  //convertion
  void rearrangeFromPlanar(const xPicP* Planar);
  void rearrangeToPlanar  (      xPicP* Planar);

public:
  //interleaved access
  //inline int32   getStride() const { return m_Stride<<2; }
  //inline int32   getPitch () const { return 4; }

  //vector access
  inline int32           getStride(                ) const { return m_Stride; }
  inline int32           getPitch (                ) const { return 1; }
  inline int32           getOffset(int32V2 Position) const { return (Position.getY() * m_Stride + Position.getX()); }
  inline uint16V4*       getAddr  (                )       { return (uint16V4*)m_Origin; }
  inline const uint16V4* getAddr  (                ) const { return (uint16V4*)m_Origin; }
  inline uint16V4*       getBuffer(                )       { return (uint16V4*)m_Buffer; }
  inline const uint16V4* getBuffer(                ) const { return (uint16V4*)m_Buffer; }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
