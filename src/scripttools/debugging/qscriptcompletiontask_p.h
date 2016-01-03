/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QSCRIPTCOMPLETIONTASK_P_H
#define QSCRIPTCOMPLETIONTASK_P_H

#include <qscriptcompletiontaskinterface_p.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerCommandSchedulerInterface;
class QScriptDebuggerJobSchedulerInterface;
class QScriptDebuggerConsole;
class QScriptCompletionTaskPrivate;

class QScriptCompletionTask : public QScriptCompletionTaskInterface
{
   SCRIPT_T_CS_OBJECT(QScriptCompletionTask)

 public:
   QScriptCompletionTask(
      const QString &contents, int cursorPosition, int frameIndex,
      QScriptDebuggerCommandSchedulerInterface *commandScheduler,
      QScriptDebuggerJobSchedulerInterface *jobScheduler,
      QScriptDebuggerConsole *console,
      QObject *parent = 0);
   ~QScriptCompletionTask();

   void start();

 private:
   Q_DECLARE_PRIVATE(QScriptCompletionTask)
   Q_DISABLE_COPY(QScriptCompletionTask)
};

QT_END_NAMESPACE

#endif
