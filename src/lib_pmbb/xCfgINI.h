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
#include "xString.h"
#include <vector>
#include <map>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xCfgINI
{
  enum class eResult
  {
    INVALID = NOT_VALID,
    UNKNOWN = 0,
    Success,
    Error,
    Failure
  };

public:
  using stringVx = std::vector<std::string>;
  using flt32Vx  = std::vector<flt32      >;
  using flt64Vx  = std::vector<flt64      >;
  using int64Vx  = std::vector<int64      >;
  using int32Vx  = std::vector<int32      >;
  using boolVx   = std::vector<bool       >;

  using tSectionNames = std::vector<std::string>;

public:
  class xCmdParam
  {
  protected:
    std::string m_CmdName;
    std::string m_SectionName;
    std::string m_ParamName;
    bool        m_IsFlag;
    std::string m_FlagValue;
    std::string m_Comment;

  public:
    xCmdParam() { m_IsFlag = false; };
    xCmdParam(const std::string& CmdName, const std::string& SectionName, const std::string& ParamName                              ) { m_CmdName = CmdName; m_SectionName = SectionName; m_ParamName = ParamName; m_IsFlag = false; m_FlagValue = ""       ; m_Comment = ""; }
    xCmdParam(const std::string& CmdName, const std::string& SectionName, const std::string& ParamName, const std::string& FlagValue) { m_CmdName = CmdName; m_SectionName = SectionName; m_ParamName = ParamName; m_IsFlag = true ; m_FlagValue = FlagValue; m_Comment = ""; }

  public:
    inline void               setCmdName     (const std::string& CmdName    )       { m_CmdName = CmdName;        }
    inline const std::string& getCmdName     (                              ) const { return m_CmdName;           } 
    inline void               setSectionName (const std::string& SectionName)       { m_SectionName = SectionName;}
    inline const std::string& getSectionName (                              ) const { return m_SectionName;       } 
    inline void               setParamName   (const std::string& ParamName  )       { m_ParamName = ParamName;    }
    inline const std::string& getParamName   (                              ) const { return m_ParamName;         } 
    inline void               setIsFlag      (bool               IsFlag     )       { m_IsFlag = IsFlag;          }
    inline bool               getIsFlag      (                              ) const { return m_IsFlag;            } 
    inline void               setFlagValue   (const std::string& FlagValue  )       { m_FlagValue = FlagValue;    }
    inline const std::string& getFlagValue   (                              ) const { return m_FlagValue;         }
  };

  class xParam
  {
  protected:
    std::string              m_Name;
    std::string              m_Comment;
    std::vector<std::string> m_Args;

  public:
    xParam(const std::string&     Name) { m_Name = Name; }
    xParam(const std::string_view Name) { m_Name = Name; }

    const std::string& getName   (                              ) const { return m_Name; }
    void               setComment(const std::string&     Comment)       { m_Comment = Comment; }
    void               setComment(const std::string_view Comment)       { m_Comment = Comment; }
    const std::string& getComment(                              ) const { return m_Comment; }

    void     clearArgs ()       { m_Args.clear(); }
    uintSize getNumArgs() const { return m_Args.size(); }
    void     addArg    (const std::string& Arg) { m_Args.push_back(Arg); }
    void     addArg    (std::string&&      Arg) { m_Args.push_back(std::move(Arg)); }
    void     addArg    (std::string_view   Arg) { m_Args.emplace_back(Arg); }

    const std::string& getArg(uint32 ArgIdx, const std::string& Default) const { return m_Args.size()>ArgIdx ? m_Args[ArgIdx] : Default; }
    flt64              getArg(uint32 ArgIdx, flt64              Default) const { return m_Args.size()>ArgIdx ? xStringToXXX(m_Args[ArgIdx], Default) : Default; }
    flt32              getArg(uint32 ArgIdx, flt32              Default) const { return m_Args.size()>ArgIdx ? xStringToXXX(m_Args[ArgIdx], Default) : Default; }
    int64              getArg(uint32 ArgIdx, int64              Default) const { return m_Args.size()>ArgIdx ? xStringToXXX(m_Args[ArgIdx], Default) : Default; }
    int32              getArg(uint32 ArgIdx, int32              Default) const { return m_Args.size()>ArgIdx ? xStringToXXX(m_Args[ArgIdx], Default) : Default; }
    bool               getArg(uint32 ArgIdx, bool               Default) const { return m_Args.size()>ArgIdx ? xStringToXXX(m_Args[ArgIdx], Default) : Default; }

    const std::vector<std::string>& getArgs(             ) const { return m_Args; }
    std::vector<flt32>              getArgs(flt32 Default) const { return xVecOfStringToVecOfXXX(m_Args, Default); }
    std::vector<flt64>              getArgs(flt64 Default) const { return xVecOfStringToVecOfXXX(m_Args, Default); }
    std::vector<int64>              getArgs(int64 Default) const { return xVecOfStringToVecOfXXX(m_Args, Default); }
    std::vector<int32>              getArgs(int32 Default) const { return xVecOfStringToVecOfXXX(m_Args, Default); }
    std::vector<bool >              getArgs(bool  Default) const { return xVecOfStringToVecOfXXX(m_Args, Default); }

  protected:
    template <class XXX> static XXX xStringToXXX(const std::string& Str, XXX Default)
    {
      if(Str.length() > 0) { std::istringstream InStringStream(Str, std::istringstream::in); InStringStream >> Default; }
      return Default;
    }
    template <class XXX> static std::vector<XXX>xVecOfStringToVecOfXXX(const std::vector<std::string>& VecStr, XXX Default)
    { 
      std::vector<XXX> VecXXX;
      std::transform(VecStr.cbegin(), VecStr.cend(), std::back_inserter(VecXXX), [&](const std::string& Str) -> XXX { return xStringToXXX<XXX>(Str, Default); });
      return VecXXX;
    }
  };

  class xSection
  {
  public:
    using tParams = std::map<std::string, xParam>;

  protected:
    std::string m_Name;
    std::string m_Comment;
    tParams     m_Params;

  public:
    xSection(const std::string&     Name) { m_Name = Name; }
    xSection(const std::string_view Name) { m_Name = Name; }
    void clear() { m_Comment.clear(); m_Params.clear(); }

    const std::string& getName   (                              ) const { return m_Name; }
    void               setComment(const std::string&     Comment)       { m_Comment = Comment; }
    void               setComment(const std::string_view Comment)       { m_Comment = Comment; }
    const std::string& getComment(                              ) const { return m_Comment; }

    //params
    void             clearParams (                            )       { m_Params.clear(); }
    uintSize         getNumParams(                            ) const { return m_Params.size(); }
    void             assignParam (const xParam& Param         )       { m_Params.insert_or_assign(Param.getName(), Param); }
    void             assignParam (xParam&&      Param         )       { m_Params.insert_or_assign(Param.getName(), std::move(Param)); }
    tParams&         getParams   (                            )       { return m_Params; }
    const tParams&   getParams   (                            ) const { return m_Params; }
    bool             findParam   (const std::string& ParamName) const { return (m_Params.find(ParamName) != m_Params.end()); }
    xParam&          getParam    (const std::string& ParamName)       { return m_Params.at(ParamName); }
    const xParam&    getParam    (const std::string& ParamName) const { return m_Params.at(ParamName); }

    //params - direct access
    const std::string& getParam1stArg(const std::string& ParamName, const std::string& Default) const { return findParam(ParamName) ? getParam(ParamName).getArg(0, Default) : Default; }
    flt64              getParam1stArg(const std::string& ParamName, flt64              Default) const { return findParam(ParamName) ? getParam(ParamName).getArg(0, Default) : Default; }
    flt32              getParam1stArg(const std::string& ParamName, flt32              Default) const { return findParam(ParamName) ? getParam(ParamName).getArg(0, Default) : Default; }
    int64              getParam1stArg(const std::string& ParamName, int64              Default) const { return findParam(ParamName) ? getParam(ParamName).getArg(0, Default) : Default; }
    int32              getParam1stArg(const std::string& ParamName, int32              Default) const { return findParam(ParamName) ? getParam(ParamName).getArg(0, Default) : Default; }
    bool               getParam1stArg(const std::string& ParamName, bool               Default) const { return findParam(ParamName) ? getParam(ParamName).getArg(0, Default) : Default; }

    stringVx getParamArgs(const std::string& ParamName               ) const { return findParam(ParamName) ? getParam(ParamName).getArgs(       ) : stringVx(); }
    flt32Vx  getParamArgs(const std::string& ParamName, flt32 Default) const { return findParam(ParamName) ? getParam(ParamName).getArgs(Default) : flt32Vx (); }
    flt64Vx  getParamArgs(const std::string& ParamName, flt64 Default) const { return findParam(ParamName) ? getParam(ParamName).getArgs(Default) : flt64Vx (); }
    int64Vx  getParamArgs(const std::string& ParamName, int64 Default) const { return findParam(ParamName) ? getParam(ParamName).getArgs(Default) : int64Vx (); }
    int32Vx  getParamArgs(const std::string& ParamName, int32 Default) const { return findParam(ParamName) ? getParam(ParamName).getArgs(Default) : int32Vx (); }
    boolVx   getParamArgs(const std::string& ParamName, bool  Default) const { return findParam(ParamName) ? getParam(ParamName).getArgs(Default) : boolVx  (); }
    
    template <typename XXX> std::vector<XXX> getParamArgs(const std::string& ParamName, XXX Default) const { return findParam(ParamName) ? getParam(ParamName).getArgs(Default) : std::vector<XXX>()    ; }
  };

  class xRootSection final : public xSection
  {
  public:
    using tSections = std::map<std::string, xSection>;    

  protected:
    tSections m_SubSections;

  public:
    xRootSection() : xSection(std::string_view("root")) {}
    void clear() { m_Comment.clear(); m_Params.clear(); m_SubSections.clear();  }

    //subsections
    void             clearSections (                              )       { m_SubSections.clear(); }
    uintSize         getNumSections(                              ) const { return m_SubSections.size(); }
    void             addSection    (const std::string& SectionName)       { if(!findSection(SectionName)) { m_SubSections.emplace(std::make_pair(SectionName, SectionName)); } }
    tSections&       getSections   (                              )       { return m_SubSections; }
    const tSections& getSections   (                              ) const { return m_SubSections; }
    bool             findSection   (const std::string& SectionName) const { return (m_SubSections.find(SectionName) != m_SubSections.end()); }
    xSection&        getSection    (const std::string& SectionName)       { return m_SubSections.at(SectionName); }
    const xSection&  getSection    (const std::string& SectionName) const { return m_SubSections.at(SectionName); }
    xSection*        getSectionPtr (const std::string& SectionName)       { return &m_SubSections.at(SectionName); }
    const xSection*  getSectionPtr (const std::string& SectionName) const { return &m_SubSections.at(SectionName); }
    
    tSectionNames    getSectionsNames() const
    {
      std::vector<std::string> SectionsNames;
      std::transform(m_SubSections.begin(), m_SubSections.end(), std::inserter(SectionsNames, SectionsNames.end()), [](auto pair) { return pair.first; });
      return SectionsNames;
    }
  };

  class xParser
  {
  protected:
    static constexpr bool     c_AllowMultiline        = true;
    static constexpr bool     c_AllowEmptyArgs        = true;
    static constexpr bool     c_AllowUTF8_BOM         = true;
    static constexpr uintSize c_ExpectedMaxLineLength = 1024;
    static constexpr uintSize c_ExpectedMaxNameLength = 256;

  protected:
    static constexpr char c_CmdToken    = '-';
    static constexpr char c_SectionBeg  = '[';
    static constexpr char c_SectionEnd  = ']';
    static constexpr char c_QuoteMarkA  = '"';
    static constexpr char c_QuoteMarkB  = '\'';
    static constexpr char c_AssignmentA = '=';
    static constexpr char c_AssignmentB = ':';
    static constexpr char c_Separator   = ',';
    static constexpr char c_CommentA    = '#';
    static constexpr char c_CommentB    = ';';

    static inline bool xIsSectionBeg(int32 c) { return (c==c_SectionBeg); }
    static inline bool xIsSectionEnd(int32 c) { return (c==c_SectionEnd); }
    static inline bool xIsSection   (int32 c) { return (xIsSectionBeg(c)  || xIsSectionEnd(c)); }

    static inline bool xIsQuoteMark (int32 c) { return (c==c_QuoteMarkA   || c==c_QuoteMarkB ); }
    static inline bool xIsAssignment(int32 c) { return (c==c_AssignmentA  || c==c_AssignmentB); }
    static inline bool xIsSeparator (int32 c) { return (c==c_Separator); }

    static inline bool xIsComment   (int32 c) { return (c==c_CommentA || c==c_CommentB); }

    static inline bool xIsNameBeg   (int32 c) { return (!xString::xIsBlank(c) && !xString::xIsEndl(c) && !xIsAssignment(c) && !xIsSection(c) && !xIsSeparator(c) && !xIsComment(c)); }
    static inline bool xIsName      (int32 c) { return (                         !xString::xIsEndl(c) && !xIsAssignment(c) && !xIsSection(c) && !xIsSeparator(c) && !xIsComment(c)); }

    static inline uintSize xCountBlank(std::string_view S) { for(uintSize C = 0; C < S.length(); C++) { if(!xString::xIsBlank(S[C])) { return C; } } return std::string_view::npos; }

  protected:
    bool               m_AllowUnknownCmdParams = true;
    xRootSection       m_RootSection;
    bool               m_ConfigFileFoundInCmd;
    std::stringstream  m_ConfigStream;
    std::string        m_CurrLine;
    uint32             m_CurrLineNum;
    std::string        m_LastSectionName;
    std::string        m_LastParamName;
    int32              m_LastParamIndent;
    std::string        m_ParserLog;

    std::map<std::string, xCmdParam> m_CmdParams;

  public:
    xParser();
    void        addCommandlineParam (xCmdParam CmdParam) { m_CmdParams.insert_or_assign(CmdParam.getCmdName(), CmdParam); }
    void        setUnknownCmdParams (bool AllowUnknownCmdParams) { m_AllowUnknownCmdParams = AllowUnknownCmdParams; }
    bool        loadFromCommandline (int argc, char* argv[], const std::string& CfgTokenShort = "-c", const std::string& CfgTokenLong = "--config");
    bool        loadFromFile        (const std::string& FileName);
    bool        loadFromString      (const std::string& Buffer  );
    std::string printConfig         (                           );
    void        clear               (                           ) { m_RootSection.clear(); m_CmdParams.clear(); }
    bool        getCfgFileFoundInCmd(                           ) { return m_ConfigFileFoundInCmd; }

  protected:
    eResult     xParseSection    (                               std::string_view  LineView);
    eResult     xParseSectionName(std::string_view& SectionName, std::string_view& LineView);
    eResult     xParseParam      (                               std::string_view  LineView);
    eResult     xParseParamCont  (                               std::string_view  LineView);
    eResult     xParseParamName  (std::string_view& ParamName,   std::string_view& LineView);
    eResult     xParseParamArg   (std::string_view& ParamArg ,   std::string_view& LineView);

    std::string xStoreSection    (const xSection& Section);
    std::string xStoreParam      (const xParam&   Param  );

  public:
    xRootSection&       getRootSection()       { return m_RootSection; }
    const xRootSection& getRootSection() const { return m_RootSection; }
 
    bool            findSection   (const std::string& SectionName) const { return m_RootSection.findSection(SectionName); }
    xSection&       getSection    (const std::string& SectionName)       { return m_RootSection.getSection (SectionName); }
    const xSection& getSection    (const std::string& SectionName) const { return m_RootSection.getSection (SectionName); }
    tSectionNames   getSectionsNames() { return m_RootSection.getSectionsNames(); }

    bool        findParam     (const std::string& ParamName) { return m_RootSection.findParam(ParamName); } //operates on root section
    xParam      getParam      (const std::string& ParamName) { return m_RootSection.getParam (ParamName); } //operates on root section

    std::string getParam1stArg(const std::string& ParamName, const std::string& Default) const { return m_RootSection.getParam1stArg(ParamName, Default); } //operates on root section
    flt64       getParam1stArg(const std::string& ParamName, flt64              Default) const { return m_RootSection.getParam1stArg(ParamName, Default); } //operates on root section
    flt32       getParam1stArg(const std::string& ParamName, flt32              Default) const { return m_RootSection.getParam1stArg(ParamName, Default); } //operates on root section
    int64       getParam1stArg(const std::string& ParamName, int64              Default) const { return m_RootSection.getParam1stArg(ParamName, Default); } //operates on root section
    int32       getParam1stArg(const std::string& ParamName, int32              Default) const { return m_RootSection.getParam1stArg(ParamName, Default); } //operates on root section
  //bool        getParam1stArg(const std::string& ParamName, bool               Default) const { return m_RootSection.getParam1stArg(ParamName, Default); } //operates on root section
  
    std::string getParam1stArg(const std::string& SectionName, const std::string& ParamName, const std::string& Default) const { return findSection(SectionName) ? getSection(SectionName).getParam1stArg(ParamName, Default) : Default; } //operates on root section
    flt64       getParam1stArg(const std::string& SectionName, const std::string& ParamName, flt64              Default) const { return findSection(SectionName) ? getSection(SectionName).getParam1stArg(ParamName, Default) : Default; } //operates on root section
    flt32       getParam1stArg(const std::string& SectionName, const std::string& ParamName, flt32              Default) const { return findSection(SectionName) ? getSection(SectionName).getParam1stArg(ParamName, Default) : Default; } //operates on root section
    int64       getParam1stArg(const std::string& SectionName, const std::string& ParamName, int64              Default) const { return findSection(SectionName) ? getSection(SectionName).getParam1stArg(ParamName, Default) : Default; } //operates on root section
    int32       getParam1stArg(const std::string& SectionName, const std::string& ParamName, int32              Default) const { return findSection(SectionName) ? getSection(SectionName).getParam1stArg(ParamName, Default) : Default; } //operates on root section
    bool        getParam1stArg(const std::string& SectionName, const std::string& ParamName, bool               Default) const { return findSection(SectionName) ? getSection(SectionName).getParam1stArg(ParamName, Default) : Default; } //operates on root section

    stringVx    getParamArgs(const std::string& ParamName) const { return m_RootSection.getParamArgs(ParamName); } //operates on root section
  };

public:
  static void printCommandlineArgs(int argc, char* argv[]);
  static void printErrorMessage   (const std::string& ErrorMessage, const std::string_view HelpString);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

