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

#ifndef QSTRING8_H
#define QSTRING8_H

#include <cs_string.h>

#include <qchar.h>
#include <qbytearray.h>

class QRegExp;

using QChar32 = CsString::CsChar;

class Q_CORE_EXPORT QString8 : public CsString::CsString
{
   public:
      using Iterator      = iterator;
      using ConstIterator = const_iterator;

      QString8()= default;
      explicit QString8(const CsString::CsString &str);
      explicit QString8(CsString::CsString &&str);

      QString8(QChar32 c);
      QString8(int size, QChar32 c);

      QString8(const QString8 &str) = default;
      QString8(QString8 &&str) = default;

      ~QString8()
      { }

      // operators
      using CsString::CsString::operator=;
      using CsString::CsString::operator+=;

      QString8 &operator=(const QString8 &)  = default;
      QString8 &operator=(QString8 && other) = default;

      // methods
      QChar32 at(int i) const;
      int count() const;
      bool isEmpty() const;

      QString8 left(int numOfChars) const;

      QString8 &prepend(const QString8 &str);

      inline QString8 &prepend(QChar32 c) {
         insert(begin(), c);
         return *this;
      }

      QString8 &remove(int indexStart, int size);
      QString8 right(int numOfChars) const;

      const uint8_t *utf8() const;

   private:

};

Q_DECLARE_TYPEINFO(QString8, Q_MOVABLE_TYPE);

#endif