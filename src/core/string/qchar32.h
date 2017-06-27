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
      QChar32() = default;

      template <typename T = int>
      QChar32(char c)
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

      ~QChar32() = default;
};

#endif
