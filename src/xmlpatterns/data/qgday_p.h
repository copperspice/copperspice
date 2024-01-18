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

#ifndef QGDay_P_H
#define QGDay_P_H

#include <qabstractdatetime_p.h>

namespace QPatternist {
class GDay : public AbstractDateTime
{

 public:
   typedef AtomicValue::Ptr Ptr;

   static GDay::Ptr fromLexical(const QString &string);
   static GDay::Ptr fromDateTime(const QDateTime &dt);

   ItemType::Ptr type() const override;
   QString stringValue() const override;

 protected:
   friend class CommonValues;

   GDay(const QDateTime &dateTime);
};
}

#endif
