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
#include <qdatastream.h>
#include <qregexp.h>

QString8::QString8(QChar32 c)
   : CsString::CsString(1, c)
{
}

QString8::QString8(size_type size, QChar32 c)
   : CsString::CsString(size, c)
{
}

// methods
QChar32 QString8::at(size_type index) const
{
   return CsString::CsString::operator[](index);
}

void QString8::chop(size_type n)
{
   if (n > 0) {
      auto iter = end() - n;
      erase(iter, end());
   }
}

QString8 &QString8::fill(QChar32 c, size_type newSize)
{
   if (newSize > 0) {
      assign(newSize, c);
   } else {
      assign(size(), c);
   }

   return *this;
}

QString8 QString8::left(size_type numOfChars) const
{
   if (numOfChars < 0) {
      return *this;
   }

   return QString8(substr(0, numOfChars));
}

QString8 QString8::mid(size_type index, size_type numOfChars) const
{
   return substr(index, numOfChars);
}

bool QString8::isEmpty() const
{
   return empty();
}

QString8 &QString8::remove(size_type indexStart, size_type size)
{
   erase(indexStart, size);
   return *this;
}

QString8 QString8::repeated(size_type count) const
{
   if (count < 1 || empty()) {
      return QString8();
   }

   if (count == 1) {
      return *this;
   }

   QString8 retval;

   for (size_type i = 0; i < count; ++i )  {
      retval += *this;
   }

   return retval;
}

QString8 QString8::right(size_type numOfChars) const
{
   if (numOfChars < 0) {
      return *this;
   }

   auto iter = end() - numOfChars;

   return QString8(iter, end());
}

void QString8::truncate(size_type length)
{
   if (length < size()) {
      resize(length);
   }
}

// operators

#if ! defined(QT_NO_DATASTREAM)
   QDataStream &operator>>(QDataStream &out, QString8 &str)
   {
      // broom - not implemented
      return out;
   }

   QDataStream &operator<<(QDataStream &out, const QString8 &str)
   {
      // broom - not implemented
      return out;
   }
#endif


// functions
