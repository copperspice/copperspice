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

#ifndef QSCRIPTDEBUGGERCONSOLECOMMAND_P_H
#define QSCRIPTDEBUGGERCONSOLECOMMAND_P_H

#include <QtCore/qobjectdefs.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class QString;
class QStringList;
class QScriptDebuggerConsole;
class QScriptDebuggerConsoleCommandJob;
class QScriptMessageHandlerInterface;
class QScriptDebuggerCommandSchedulerInterface;
class QScriptDebuggerConsoleCommandPrivate;

class QScriptDebuggerConsoleCommand
{
 public:
   QScriptDebuggerConsoleCommand();
   virtual ~QScriptDebuggerConsoleCommand();

   virtual QString name() const = 0;
   virtual QString group() const = 0;
   virtual QString shortDescription() const = 0;
   virtual QString longDescription() const = 0;
   virtual QStringList seeAlso() const;
   virtual QStringList aliases() const;

   virtual QStringList argumentTypes() const;

   virtual QStringList subCommands() const;
   virtual QScriptDebuggerConsoleCommandJob *createJob(
      const QStringList &arguments,
      QScriptDebuggerConsole *console,
      QScriptMessageHandlerInterface *messageHandler,
      QScriptDebuggerCommandSchedulerInterface *scheduler) = 0;

 protected:
   QScriptDebuggerConsoleCommand(QScriptDebuggerConsoleCommandPrivate &dd);
   QScopedPointer<QScriptDebuggerConsoleCommandPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QScriptDebuggerConsoleCommand)
   Q_DISABLE_COPY(QScriptDebuggerConsoleCommand)
};

typedef QList<QScriptDebuggerConsoleCommand *> QScriptDebuggerConsoleCommandList;

QT_END_NAMESPACE

#endif
