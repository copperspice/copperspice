/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include "qinteger_p.h"

#include "qtocodepointsiterator_p.h"

using namespace QPatternist;

ToCodepointsIterator::ToCodepointsIterator(const QString &string)
   : m_string(string),
     m_len(string.length()),
     m_position(0)
{
   Q_ASSERT(!string.isEmpty());
}

Item ToCodepointsIterator::next()
{
   if (m_position == -1) {
      return Item();
   }

   ++m_position;
   if (m_position > m_len) {
      m_position = -1;
      m_current.reset();
      return m_current;
   }

   m_current = Integer::fromValue(m_string.at(m_position - 1).unicode());
   return m_current;
}

xsInteger ToCodepointsIterator::count()
{
   return m_len;
}

Item ToCodepointsIterator::current() const
{
   return m_current;
}

xsInteger ToCodepointsIterator::position() const
{
   return m_position;
}

Item::Iterator::Ptr ToCodepointsIterator::copy() const
{
   return Item::Iterator::Ptr(new ToCodepointsIterator(m_string));
}

