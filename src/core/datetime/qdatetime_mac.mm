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

#include <qdatetime.h>

#import <Foundation/Foundation.h>

QDateTime QDateTime::fromCFDate(CFDateRef date)
{
   if (! date) {
      return QDateTime();
   }

   return QDateTime::fromMSecsSinceEpoch(static_cast<qint64>((CFDateGetAbsoluteTime(date)
         + kCFAbsoluteTimeIntervalSince1970) * 1000));
}

CFDateRef QDateTime::toCFDate() const
{
   return CFDateCreate(kCFAllocatorDefault, (static_cast<CFAbsoluteTime>(toMSecsSinceEpoch())
         / 1000) - kCFAbsoluteTimeIntervalSince1970);
}

QDateTime QDateTime::fromNSDate(const NSDate *date)
{
   if (! date) {
      return QDateTime();
   }

   return QDateTime::fromMSecsSinceEpoch(static_cast<qint64>([date timeIntervalSince1970] * 1000));
}

NSDate *QDateTime::toNSDate() const
{
   return [NSDate dateWithTimeIntervalSince1970:static_cast<NSTimeInterval>(toMSecsSinceEpoch()) / 1000];
}
