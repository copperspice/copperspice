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

#include <qstring.h>

#import <Foundation/Foundation.h>

QString8 QString8::fromCFString(CFStringRef str)
{
   if (! str) {
      return QString();
   }

   CFIndex length = CFStringGetLength(str);
   if (length == 0) {
      return QString();
   }

   const UniChar *chars = CFStringGetCharactersPtr(str);
   if (chars) {
      return QString::fromUtf16(reinterpret_cast<const char16_t *>(chars), length);
   }

   std::vector<UniChar> tmp(length);
   CFStringGetCharacters(str, CFRangeMake(0, length), &tmp[0]);

   return QString::fromUtf16(reinterpret_cast<const char16_t *>(&tmp[0]), length);
}

CFStringRef QString8::toCFString() const
{
   return CFStringCreateWithBytes(kCFAllocatorDefault,
                  reinterpret_cast<const UInt8 *>(this->constData()), this->size_storage(), kCFStringEncodingUTF8, false);
}

QString8 QString8::fromNSString(const NSString *str)
{
   if (! str) {
      return QString();
   }

   return QString::fromUtf8([str UTF8String]);
}

NSString *QString8::toNSString() const
{
   return [[NSString alloc] initWithBytes: constData() length:size_storage() encoding:NSUTF8StringEncoding];
}

