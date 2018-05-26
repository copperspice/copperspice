/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
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

#include <qcore_mac_p.h>
#include <new>

QString QCFString::toQString(CFStringRef str)
{
   return QString::fromCFString(str);
}

QCFString::operator QString() const
{
   if (string.isEmpty() && type) {
      const_cast<QCFString *>(this)->string = toQString(type);
   }

   return string;
}

CFStringRef QCFString::toCFStringRef(const QString &string)
{
   return string.toCFString();
}

QCFString::operator CFStringRef() const
{
   if (! type) {
      const_cast<QCFString *>(this)->type = CFStringCreateWithBytesNoCopy(kCFAllocatorDefault,
                  reinterpret_cast<const UInt8 *>(string.constData()), string.size_storage(),
                  kCFStringEncodingUTF8, false, kCFAllocatorNull);
   }

   return type;
}

