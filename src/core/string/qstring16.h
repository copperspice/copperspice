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

#ifndef QSTRING16_H
#define QSTRING16_H

#include <cs_string.h>

#include <qchar32.h>
#include <qbytearray.h>

class QRegExp;
class QString16;

void cs_swapFunc(QString16 &a, QString16 &b);

class Q_CORE_EXPORT QString16 : public CsString::CsString_utf16
{
   public:
      using Iterator        = iterator;
      using ConstIterator   = const_iterator;

      using iterator        = CsString::CsString_utf16::iterator;
      using const_iterator  = CsString::CsString_utf16::const_iterator;

      QString16() = default;
      QString16(const QString16 &other) = default;
      QString16(QString16 &&other) = default;

      QString16(QChar32 c);
      QString16(size_type size, QChar32 c);

      QString16(const CsString::CsString_utf16 &other)
         : CsString::CsString_utf16(other)
      {
      }

      QString16(CsString::CsString_utf16 &&other)
         : CsString::CsString_utf16(std::move(other))
      {
      }

      template <typename Iterator>
      QString16(Iterator begin, Iterator end)
         : CsString::CsString_utf16(begin, end)
      { }

      ~QString16() = default;

};

Q_DECLARE_TYPEINFO(QString16, Q_MOVABLE_TYPE);      // broom - verify


#endif