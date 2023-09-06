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


#include "xCfgINI.h"
#include "xFile.h"
#include <fstream>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xCfgINI::xParser
//===============================================================================================================================================================================================================
xCfgINI::xParser::xParser()
{
  m_ConfigFileFoundInCmd = false;
  m_CurrLine       .reserve(c_ExpectedMaxLineLength);
  m_LastSectionName.reserve(c_ExpectedMaxNameLength);
  m_LastParamName  .reserve(c_ExpectedMaxNameLength);
}
bool xCfgINI::xParser::loadFromCommandline(int argc, char* argv[], const std::string& CfgTokenShort, const std::string& CfgTokenLong)
{
  if(argc == 1) { m_ParserLog += fmt::sprintf("cfg: no commandline arguments!\n"); return false; }

  for(int i = 1; i < argc; i++)
  {
    std::string OneArg = argv[i];

    if(OneArg[0] == c_CmdToken && OneArg.length()>1) //begins from '-'
    {
      //config file
      if(((!CfgTokenShort.empty() && OneArg == CfgTokenShort) || (!CfgTokenLong.empty() && OneArg == CfgTokenLong)) && ++i < argc)
      {
        m_ConfigFileFoundInCmd = true;
        std::string CfgFileName = argv[i];
        bool LoadedCorectly = loadFromFile(CfgFileName);
        if(!LoadedCorectly) { return false; }
      }
      //commandline param
      else if(!m_CmdParams.empty() && m_CmdParams.find(OneArg) != m_CmdParams.end())
      {
        xCmdParam& CmdParam = m_CmdParams[OneArg];
        xParam Param(CmdParam.getParamName());
        if(CmdParam.getIsFlag()) //flag only
        { 
          Param.addArg(CmdParam.getFlagValue());
        } 
        else //flag only
        {
          if(++i < argc)
          { 
            std::string Arg = argv[i];
            Param.addArg(Arg);
          }
          else
          { 
            return false;
          }
        } //flag only

        Param.setComment(fmt::sprintf("%c generated based on commandline param", c_CommentA));
        if(CmdParam.getSectionName().empty() || CmdParam.getSectionName()=="root")
        {
          m_RootSection.assignParam(std::move(Param));
        }
        else
        {
          if(m_RootSection.findSection(CmdParam.getSectionName())) //section alredy exist
          {
            m_RootSection.getSection(CmdParam.getSectionName()).assignParam(std::move(Param));
          }
          else
          {
            m_RootSection.addSection(CmdParam.getSectionName());
            m_RootSection.getSection(CmdParam.getSectionName()).assignParam(std::move(Param));
            m_RootSection.getSection(CmdParam.getSectionName()).setComment(fmt::sprintf("%c generated based on commandline param", c_CommentA));
          }
        }
      }
      else
      {
        //unknown param
        if(!m_AllowUnknownCmdParams) { return false; }
      }
    }
    else
    {
      //its no param
      return false;
    }
  }
  return true;
}
bool xCfgINI::xParser::loadFromFile(const std::string& FileName)
{
  if(!xFile::exist(FileName)) { return false; }

  std::ifstream File(FileName.c_str(), std::ifstream::in);
  if(File.good() && File.is_open())
  {
    std::string Buffer((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());
    File.close();
    loadFromString(Buffer);
    return true;
  }
  return false;
}
bool xCfgINI::xParser::loadFromString(const std::string& Buffer)
{
  m_ConfigStream.clear();
  m_ConfigStream.str  (Buffer);
  m_CurrLineNum = 0;
  m_LastSectionName.clear();
  m_LastParamName  .clear();
  m_LastParamIndent = NOT_VALID;

  while(!std::getline(m_ConfigStream, m_CurrLine).eof())
  {
    eResult RetVal = eResult::UNKNOWN;
    m_CurrLineNum++;
    if(m_CurrLine.empty()) { continue; }

    std::string_view LineView(m_CurrLine);
    if(c_AllowUTF8_BOM && m_CurrLineNum==1 && LineView.length()>=3 && (unsigned char)(LineView[0]) == 0xEF && (unsigned char)(LineView[1]) == 0xBB && (unsigned char)(LineView[2]) == 0xBF) //C++20 --> starts_with
    {
      LineView.remove_prefix(3);
    }
    if(LineView.empty()) { continue; }

    LineView = xString::stripR(LineView);
    if(LineView.empty()) { continue; }
    int32 stripL = (int32)xCountBlank(LineView);
    if(LineView.empty()) { continue; }

    int32 Token = LineView[stripL];
    if(xIsComment(Token))//comment line --> nothing to do here
    {

    }
    else if(c_AllowMultiline && m_LastParamIndent != NOT_VALID && stripL > m_LastParamIndent && !m_LastParamName.empty()) //Non-black line with leading whitespace, treat as continuation of previous name's value
    {
      RetVal = xParseParamCont(LineView);
    }
    else if(xIsSectionBeg(Token)) //section line [section]
    {
      RetVal = xParseSection(LineView);
      m_LastParamIndent = NOT_VALID;
    }
    else if(xIsNameBeg(Token) || xIsQuoteMark(Token)) //new param line
    {
      RetVal = xParseParam(LineView);
      m_LastParamIndent = stripL;
    }
    else //unknown
    {
      RetVal = eResult::Failure;
    }

    //errors
    if(RetVal == eResult::Error)
    {
      if(xIsSectionBeg(Token)) { return false; }
    }
    if(RetVal == eResult::Failure)
    {
      return false;
    }
  }
  return true;
}
std::string xCfgINI::xParser::printConfig()
{
  std::string Result;

  Result += xStoreSection(m_RootSection);

  for(auto const& [SectionName, Section] : m_RootSection.getSections())
  {
    Result += xStoreSection(Section);
  }
  return Result;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

xCfgINI::eResult xCfgINI::xParser::xParseSection(std::string_view LineView)
{
  LineView = xString::stripL(LineView);

  //section begin
  if(LineView.length() < 3)            { return eResult::Error; }
  if(!xIsSectionBeg(LineView.front())) { return eResult::Error; }
  LineView.remove_prefix(1);
  LineView = xString::stripL(LineView);
  //section name
  std::string_view SectionName;
  eResult RetVal = xParseSectionName(SectionName, LineView);
  if(RetVal != eResult::Success) { return eResult::Error; }
  //section end
  if(LineView.empty())                 { return eResult::Error; }
  if(!xIsSectionEnd(LineView.front())) { return eResult::Error; }
  LineView.remove_prefix(1);

  m_LastSectionName = SectionName;
  if(!m_RootSection.findSection(m_LastSectionName))
  {
    m_RootSection.addSection(m_LastSectionName);
  }  

  LineView = xString::stripL(LineView);
  if(!LineView.empty() && xIsComment(LineView.front())) //comment
  {
    xSection& CurrSection = m_RootSection.getSection(m_LastSectionName);
    CurrSection.setComment(xString::stripR(LineView.substr(1)));
  }

  return eResult::Success;
}
xCfgINI::eResult xCfgINI::xParser::xParseSectionName(std::string_view& SectionName, std::string_view& LineView)
{
  if(!xIsNameBeg(LineView.front())) { return eResult::Error; }
  if(LineView.length() < 2)         { return eResult::Error; }
  uintSize SectionEnd = LineView.find(c_SectionEnd);
  if(SectionEnd == 0 || SectionEnd == std::string_view::npos) { return eResult::Error; }
  SectionName = xString::stripR(LineView.substr(0, SectionEnd));
  LineView.remove_prefix(SectionEnd);
  return eResult::Success;
}

xCfgINI::eResult xCfgINI::xParser::xParseParam(std::string_view LineView)
{
  LineView = xString::stripL(LineView);

  std::string_view ParamName;
  eResult RetValName = xParseParamName(ParamName, LineView);
  if(RetValName != eResult::Success) { return eResult::Error; }
  xParam Param(ParamName);
  m_LastParamName = ParamName;
  
  uintSize AssignmentBeg = xMin(LineView.find(c_AssignmentA), LineView.find(c_AssignmentB));
  if constexpr(!c_AllowEmptyArgs)
  {
    if(AssignmentBeg == std::string_view::npos) { return eResult::Error; }
  }
  
  if(AssignmentBeg != std::string_view::npos)
  {
    LineView.remove_prefix(AssignmentBeg+1);
    LineView = xString::stripL(LineView);
    if(LineView.length() < 1) { return eResult::Error; }

    if(xString::xIsEndl(LineView.front()) || xIsAssignment(LineView.front()) || xIsComment(LineView.front()) || xIsSection(LineView.front())) { return eResult::Error; };
    do
    {
      std::string_view ParamArg;
      eResult RetValArg = xParseParamArg(ParamArg, LineView);
      if(RetValArg != eResult::Success) { return eResult::Error; }
      if(!ParamArg.empty()) { Param.addArg(ParamArg); }
      LineView = xString::stripL(LineView);
    }
    while(LineView.length() > 0 && !xString::xIsEndl(LineView.front()) && !xIsComment(LineView.front()));
  }

  if(!LineView.empty() && xIsComment(LineView.front())) //comment
  {
    Param.setComment(xString::stripR(LineView.substr(1)));
  }

  xSection* Section = &m_RootSection;
  if(!m_LastSectionName.empty()) 
  {
    if(!m_RootSection.findSection(m_LastSectionName)) { return eResult::Error; }    
    Section = &m_RootSection.getSection(m_LastSectionName);
  }
  Section->assignParam(std::move(Param));
  return eResult::Success;
}
xCfgINI::eResult xCfgINI::xParser::xParseParamCont(std::string_view LineView)
{
  xSection* Section = &m_RootSection;
  if(!m_LastSectionName.empty()) 
  {
    if(!m_RootSection.findSection(m_LastSectionName)) { return eResult::Error; }    
    Section = &m_RootSection.getSection(m_LastSectionName);
  }

  if(m_LastParamName.empty()             ) { return eResult::Error; }
  if(!Section->findParam(m_LastParamName)) { return eResult::Error; }
  xParam& Param = Section->getParam(m_LastParamName);

  LineView = xString::stripL(LineView);
  
  if(xString::xIsEndl(LineView.front()) || xIsAssignment(LineView.front()) || xIsComment(LineView.front()) || xIsSection(LineView.front())) { return eResult::Error; };
  do
  {
    std::string_view ParamArg;
    eResult RetVal = xParseParamArg(ParamArg, LineView);
    if(RetVal != eResult::Success) { return eResult::Error; }
    if(!ParamArg.empty()) { Param.addArg(ParamArg); }
    LineView = xString::stripL(LineView);
  }
  while(LineView.length() > 0 && !xString::xIsEndl(LineView.front()) && !xIsComment(LineView.front()));
  return eResult::Success;
}

xCfgINI::eResult xCfgINI::xParser::xParseParamName(std::string_view& ParamName, std::string_view& LineView)
{
  if(!xIsNameBeg(LineView.front())) { return eResult::Error; }
  if(LineView.length() == 0)        { return eResult::Error; }

  uintSize NameLength = 1;
  while(NameLength < LineView.length())
  {
    if(!xIsName(LineView[NameLength])) { break; }
    NameLength++;
  }

  ParamName = xString::stripR(LineView.substr(0, NameLength));
  LineView.remove_prefix(NameLength);
  return eResult::Success;
}
xCfgINI::eResult xCfgINI::xParser::xParseParamArg(std::string_view& ParamArg, std::string_view& LineView)
{
  if(LineView.length() == 0) { return eResult::Error; }

  if(xIsQuoteMark(LineView.front()))
  {
    const char QuoteMark = LineView.front();
    LineView.remove_prefix(1);
    uintSize   QuoteEnd  = LineView.find(QuoteMark);
    if(QuoteEnd == std::string_view::npos) { return eResult::Error; }
    ParamArg = LineView.substr(0, QuoteEnd);
    LineView.remove_prefix(QuoteEnd+1);
  }
  else
  {  
    uintSize ArgLength = 1;
    while(ArgLength < LineView.length())
    {
      if(!xIsName(LineView[ArgLength])) { break; }
      ArgLength++;
    }
    if(ArgLength < LineView.length() && (xIsSection(LineView[ArgLength]) || xIsAssignment(LineView[ArgLength]))) { return eResult::Error; }
    ParamArg = xString::stripR(LineView.substr(0, ArgLength));
    LineView.remove_prefix(ArgLength);
    LineView = xString::stripL(LineView);
    if(!LineView.empty() && xIsSeparator(LineView.front())) { LineView.remove_prefix(1); }
  } 

  return eResult::Success;
}
std::string xCfgINI::xParser::xStoreSection(const xSection& Section)
{
  std::string Result;
  if(Section.getName() != "root")
  {
    Result += c_SectionBeg + Section.getName() + c_SectionEnd;
    if(!Section.getComment().empty()) { Result += " # " + Section.getComment(); }
    Result += "\n";
  }
  for(auto const& [ParamName, Param] : Section.getParams())
  {
    Result += xStoreParam(Param);
  }
  return Result;
}
std::string xCfgINI::xParser::xStoreParam(const xParam& Param)
{
  std::string Result;

  Result += Param.getName() + " " + c_AssignmentA + " ";
  const std::vector<std::string>& Args = Param.getArgs();
  for(const auto& Arg : Args) { Result += Arg + ", "; }
  if(!Param.getComment().empty()) { Result += " # " + Param.getComment(); }
  Result += "\n";
  return Result;
}

//===============================================================================================================================================================================================================
// xCfgINI
//===============================================================================================================================================================================================================
void xCfgINI::printCommandlineArgs(int argc, char* argv[])
{
  std::string CommandlineArgs = fmt::sprintf("argc     = %d\n", argc);
  for(int32 i=0; i<argc; i++) { CommandlineArgs += fmt::sprintf("argv[%02d] = %s\n", i, argv[i]); }
  CommandlineArgs += fmt::sprintf("\n");
  fmt::printf(CommandlineArgs);
}
void xCfgINI::printErrorMessage(const std::string& ErrorMessage, const std::string_view HelpString)
{
  fmt::fprintf(stdout, ErrorMessage + "\n");
  fmt::fprintf(stdout, HelpString);
  fmt::fprintf(stderr, ErrorMessage);
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
