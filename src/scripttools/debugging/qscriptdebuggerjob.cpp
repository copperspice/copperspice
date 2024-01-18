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

#include "qscriptdebuggerjob_p.h"
#include "qscriptdebuggerjob_p_p.h"
#include "qscriptdebuggerjobschedulerinterface_p.h"

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

/*!
  \class QScriptDebuggerJob
  \since 4.5
  \internal

  \brief The QScriptDebuggerJob class is the base class of debugger jobs.

*/

QScriptDebuggerJobPrivate::QScriptDebuggerJobPrivate()
{
}

QScriptDebuggerJobPrivate::~QScriptDebuggerJobPrivate()
{
}

QScriptDebuggerJobPrivate *QScriptDebuggerJobPrivate::get(QScriptDebuggerJob *q)
{
   return q->d_func();
}

QScriptDebuggerJob::QScriptDebuggerJob()
   : d_ptr(new QScriptDebuggerJobPrivate)
{
   d_ptr->q_ptr = this;
   d_ptr->jobScheduler = 0;
}

QScriptDebuggerJob::QScriptDebuggerJob(QScriptDebuggerJobPrivate &dd)
   : d_ptr(&dd)
{
   d_ptr->q_ptr = this;
   d_ptr->jobScheduler = 0;
}

QScriptDebuggerJob::~QScriptDebuggerJob()
{
}

void QScriptDebuggerJob::finish()
{
   Q_D(QScriptDebuggerJob);
   Q_ASSERT(d->jobScheduler != 0);
   d->jobScheduler->finishJob(this);
}

void QScriptDebuggerJob::hibernateUntilEvaluateFinished()
{
   Q_D(QScriptDebuggerJob);
   Q_ASSERT(d->jobScheduler != 0);
   d->jobScheduler->hibernateUntilEvaluateFinished(this);
}

void QScriptDebuggerJob::evaluateFinished(const QScriptDebuggerValue &)
{
   Q_ASSERT_X(false, "QScriptDebuggerJob::evaluateFinished()",
              "implement if hibernateUntilEvaluateFinished() is called");
}

QT_END_NAMESPACE
