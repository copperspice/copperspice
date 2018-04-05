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

#include <qstring8.h>

#import <Foundation/Foundation.h>

QString QString::fromCFString(CFStringRef string)
{
    if (! string) {
        return QString();
    }

    CFIndex length = CFStringGetLength(string);

    // Fast path: CFStringGetCharactersPtr does not copy but may
    // return null for any and no reason.
    const UniChar *chars = CFStringGetCharactersPtr(string);

    if (chars) {
        return QString(reinterpret_cast<const QChar *>(chars), length);
    }

    QString ret(length, Qt::Uninitialized);

    CFStringGetCharacters(string, CFRangeMake(0, length), reinterpret_cast<UniChar *>(ret.data()));
    return ret;
}

CFStringRef QString::toCFString() const
{
    return CFStringCreateWithCharacters(0, reinterpret_cast<const UniChar *>(unicode()), length());
}

QString QString::fromNSString(const NSString *string)
{
    if (! string) {
        return QString();
   }

   QString qstring;
   qstring.resize([string length]);

   [string getCharacters: reinterpret_cast<unichar*>(qstring.data()) range: NSMakeRange(0, [string length])];
   return qstring;
}

NSString *QString::toNSString() const
{
    return [NSString stringWithCharacters: reinterpret_cast<const UniChar*>(unicode()) length: length()];
}

