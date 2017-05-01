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

#include <qstring8.h>
#include <qregexp.h>

QString8::QString8(QChar32 c)
   : CsString::CsString(1, c)
{
}

QString8::QString8(int size, QChar32 c)
   : CsString::CsString(size, c)
{
}

QString8::QString8(const CsString::CsString &str)
   : CsString::CsString(str)
{
}

QString8::QString8(CsString::CsString &&str)
   : CsString::CsString(std::move(str))
{
}

// operators


// methods
QChar32 QString8::at(int i) const
{
   return CsString::CsString::operator[](i);
}

int QString8::count() const 
{
   return size();
}

QString8 QString8::left(int numOfChars) const
{
   if (numOfChars < 0) {
      return *this;
   }

   return QString8(substr(0, numOfChars));
}

bool QString8::isEmpty() const
{
   return empty();
}

QString8 &QString8::prepend(const QString8 &str) 
{
   insert(0, str);
   return *this;
}

QString8 &QString8::remove(int indexStart, int size)
{
   erase(indexStart, size);  
   return *this;
}

QString8 QString8::right(int numOfChars) const
{
   if (numOfChars < 0) {
      return *this;
   }

   int size = this->size();

   return QString8(substr(size - numOfChars, numOfChars));
}

const uint8_t *QString8::utf8() const
{
   return constData();
}

