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

#include <qcore_mac_p.h>
#include <new>

QString QCFString::toQString(CFStringRef str)
{
   return QString::fromCFString(str);
}

CFStringRef QCFString::toCFStringRef(const QString &str)
{
   return str.toCFString();
}

QString QCFString::toQString() const
{
   if (m_string.isEmpty() && m_type) {
      const_cast<QCFString *>(this)->m_string = toQString(m_type);
   }

   return m_string;
}

CFStringRef QCFString::toCFStringRef() const
{
   if (! m_type) {
      const_cast<QCFString *>(this)->m_type = CFStringCreateWithBytesNoCopy(kCFAllocatorDefault,
                  reinterpret_cast<const UInt8 *>(m_string.constData()), m_string.size_storage(),
                  kCFStringEncodingUTF8, false, kCFAllocatorNull);
   }

   return m_type;
}

