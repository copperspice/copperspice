/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QSCRIPTDEBUGGERJOB_P_H
#define QSCRIPTDEBUGGERJOB_P_H

#include <QtCore/qobjectdefs.h>
#include <QtCore/qscopedpointer.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerJobSchedulerInterface;
class QScriptDebuggerValue;
class QScriptDebuggerJobPrivate;

class QScriptDebuggerJob
{
 public:
   QScriptDebuggerJob();
   virtual ~QScriptDebuggerJob();

   virtual void start() = 0;

   void finish();

   void hibernateUntilEvaluateFinished();
   virtual void evaluateFinished(const QScriptDebuggerValue &result);

 protected:
   QScriptDebuggerJob(QScriptDebuggerJobPrivate &dd);
   QScopedPointer<QScriptDebuggerJobPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QScriptDebuggerJob)
   Q_DISABLE_COPY(QScriptDebuggerJob)
};

QT_END_NAMESPACE

#endif
