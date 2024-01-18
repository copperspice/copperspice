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

#include "qscriptdebuggerconsolecommandjob_p.h"
#include "qscriptdebuggerconsolecommandjob_p_p.h"

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

QScriptDebuggerConsoleCommandJobPrivate::QScriptDebuggerConsoleCommandJobPrivate()
{
}

QScriptDebuggerConsoleCommandJobPrivate::~QScriptDebuggerConsoleCommandJobPrivate()
{
}

QScriptDebuggerConsoleCommandJob::QScriptDebuggerConsoleCommandJob(
   QScriptDebuggerConsole *console,
   QScriptMessageHandlerInterface *messageHandler,
   QScriptDebuggerCommandSchedulerInterface *scheduler)
   : QScriptDebuggerCommandSchedulerJob(*new QScriptDebuggerConsoleCommandJobPrivate,
                                        scheduler)
{
   Q_D(QScriptDebuggerConsoleCommandJob);
   d->console = console;
   d->messageHandler = messageHandler;
}

QScriptDebuggerConsoleCommandJob::~QScriptDebuggerConsoleCommandJob()
{
}

QScriptDebuggerConsoleCommandJob::QScriptDebuggerConsoleCommandJob(
   QScriptDebuggerConsoleCommandJobPrivate &dd,
   QScriptDebuggerConsole *console,
   QScriptMessageHandlerInterface *messageHandler,
   QScriptDebuggerCommandSchedulerInterface *scheduler)
   : QScriptDebuggerCommandSchedulerJob(dd, scheduler)
{
   Q_D(QScriptDebuggerConsoleCommandJob);
   d->console = console;
   d->messageHandler = messageHandler;
}

QT_END_NAMESPACE
