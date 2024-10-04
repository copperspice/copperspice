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

#ifndef QCallTargetDescription_P_H
#define QCallTargetDescription_P_H

#include <QSharedData>
#include <qexpression_p.h>
#include <qcontainerfwd.h>

namespace QPatternist {

class CallSite;

class CallTargetDescription : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<CallTargetDescription> Ptr;
   typedef QList<Ptr> List;

   CallTargetDescription(const QXmlName &name);

   QXmlName name() const;

   static void checkCallsiteCircularity(CallTargetDescription::List &signList,
                                        const Expression::Ptr expr);
 private:
   static void checkArgumentsCircularity(CallTargetDescription::List &signList,
                                         const Expression::Ptr callsite);

   CallTargetDescription(const CallTargetDescription &) = delete;
   CallTargetDescription &operator=(const CallTargetDescription &) = delete;

   const QXmlName m_name;
};
}

#endif

