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

#include <qstring.h>
#include <qstring16.h>

#import <Foundation/Foundation.h>

// QString 8

QString8 QString8::fromCFString(CFStringRef str)
{
   if (! str) {
      return QString8();
   }

   CFIndex length = CFStringGetLength(str);
   if (length == 0) {
      return QString8();
   }

   const UniChar *chars = CFStringGetCharactersPtr(str);
   if (chars) {
      return QString8::fromUtf16(reinterpret_cast<const char16_t *>(chars), length);
   }

   std::vector<UniChar> tmp(length);
   CFStringGetCharacters(str, CFRangeMake(0, length), &tmp[0]);

   return QString8::fromUtf16(reinterpret_cast<const char16_t *>(&tmp[0]), length);
}

CFStringRef QString8::toCFString() const
{
   return CFStringCreateWithBytes(kCFAllocatorDefault,
               reinterpret_cast<const UInt8 *>(this->constData()), this->size_storage(), kCFStringEncodingUTF8, false);
}

QString8 QString8::fromNSString(const NSString *str)
{
   if (! str) {
      return QString8();
   }

   return QString8::fromUtf8([str UTF8String]);
}

NSString *QString8::toNSString() const
{
   return [[NSString alloc] initWithBytes: constData() length:size_storage() encoding:NSUTF8StringEncoding];
}

// QString 16

QString16 QString16::fromCFString(CFStringRef str)
{
   if (! str) {
      return QString16();
   }

   CFIndex length = CFStringGetLength(str);
   if (length == 0) {
      return QString16();
   }

   const UniChar *chars = CFStringGetCharactersPtr(str);
   if (chars) {
      return QString16::fromUtf16(reinterpret_cast<const char16_t *>(chars), length);
   }

   std::vector<UniChar> tmp(length);
   CFStringGetCharacters(str, CFRangeMake(0, length), &tmp[0]);

   return QString16::fromUtf16(reinterpret_cast<const char16_t *>(&tmp[0]), length);
}

CFStringRef QString16::toCFString() const
{
   return CFStringCreateWithCharacters(kCFAllocatorDefault,
                  reinterpret_cast<const UniChar *>(this->constData()), this->size_storage());
}

QString16 QString16::fromNSString(const NSString *str)
{
   if (! str) {
      return QString16();
   }

   return QString16::fromUtf8([str UTF8String]);
}

NSString *QString16::toNSString() const
{
   return [[NSString alloc] initWithCharacters: reinterpret_cast<const unichar *>(constData()) length:size_storage()];
}

