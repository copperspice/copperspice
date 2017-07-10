/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QCHAR32_H
#define QCHAR32_H

#include <qglobal.h>
#include <cs_char.h>
#include <cs_string.h>

class Q_CORE_EXPORT QChar32 : public CsString::CsChar
{
   public:
      enum SpecialCharacter {
         Null                       = 0x0000,
         Tabulation                 = 0x0009,
         LineFeed                   = 0x000a,
         CarriageReturn             = 0x000d,
         Space                      = 0x0020,
         Nbsp                       = 0x00a0,
         ReplacementCharacter       = 0xfffd,
         ObjectReplacementCharacter = 0xfffc,
         ByteOrderMark              = 0xfeff,
         ByteOrderSwapped           = 0xfffe,
         ParagraphSeparator         = 0x2029,
         LineSeparator              = 0x2028,
         LastValidCodePoint         = 0x10ffff
      };

      enum Category {
         Mark_NonSpacing,          //   Mn
         Mark_SpacingCombining,    //   Mc
         Mark_Enclosing,           //   Me

         Number_DecimalDigit,      //   Nd
         Number_Letter,            //   Nl
         Number_Other,             //   No

         Separator_Space,          //   Zs
         Separator_Line,           //   Zl
         Separator_Paragraph,      //   Zp

         Other_Control,            //   Cc
         Other_Format,             //   Cf
         Other_Surrogate,          //   Cs
         Other_PrivateUse,         //   Co
         Other_NotAssigned,        //   Cn

         Letter_Uppercase,         //   Lu
         Letter_Lowercase,         //   Ll
         Letter_Titlecase,         //   Lt
         Letter_Modifier,          //   Lm
         Letter_Other,             //   Lo

         Punctuation_Connector,    //   Pc
         Punctuation_Dash,         //   Pd
         Punctuation_Open,         //   Ps
         Punctuation_Close,        //   Pe
         Punctuation_InitialQuote, //   Pi
         Punctuation_FinalQuote,   //   Pf
         Punctuation_Other,        //   Po

         Symbol_Math,              //   Sm
         Symbol_Currency,          //   Sc
         Symbol_Modifier,          //   Sk
         Symbol_Other,             //   So
      };

      QChar32() = default;

      template <typename T = int>
      explicit QChar32(char c)
         : CsString::CsChar(c)
      { }

      QChar32(char32_t c)
         : CsString::CsChar(c)
      { }

      QChar32(int c)
         : CsString::CsChar(c)
      { }

      QChar32(CsString::CsChar c)
         : CsString::CsChar(c)
      { }

      QChar32(SpecialCharacter c)
         : CsString::CsChar(static_cast<char32_t>(c))
      { }

      ~QChar32() = default;

      // methods
      static QChar32 fromLatin1(char c) {
         return QChar32(static_cast<char32_t>(c));
      }

      bool isNull() const {
         return unicode() == 0;
      }

      char toLatin1() const {
         uint32_t tmp = unicode();

         if (tmp < 256) {
            return tmp;
         }

         return 0;
      }

      uint32_t unicode() const {
         return CsString::CsChar::unicode();
      }

};

inline bool operator==(QChar32 c1, QChar32 c2)
{
   return c1.unicode() == c2.unicode();
}

inline bool operator!=(QChar32 c1, QChar32 c2)
{
   return c1.unicode() != c2.unicode();
}

inline bool operator<=(QChar32 c1, QChar32 c2)
{
   return c1.unicode() <= c2.unicode();
}

inline bool operator>=(QChar32 c1, QChar32 c2)
{
   return c1.unicode() >= c2.unicode();
}

inline bool operator<(QChar32 c1, QChar32 c2)
{
   return c1.unicode() < c2.unicode();
}

inline bool operator>(QChar32 c1, QChar32 c2)
{
   return c1.unicode() > c2.unicode();
}


#endif
