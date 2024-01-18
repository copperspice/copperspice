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

#ifndef QSCRIPTDEBUGGERCONSOLEGLOBALOBJECT_P_H
#define QSCRIPTDEBUGGERCONSOLEGLOBALOBJECT_P_H

#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>
#include <qscriptdebuggerconsolecommandgroupdata_p.h>
#include <qscriptdebuggerconsolecommand_p.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerCommandSchedulerInterface;
class QScriptMessageHandlerInterface;
class QScriptDebuggerResponseHandlerInterface;
class QScriptDebuggerConsole;
class QScriptDebuggerValue;
class QScriptDebuggerCommand;
class QScriptBreakpointData;
class QScriptDebuggerConsoleGlobalObjectPrivate;

class QScriptDebuggerConsoleGlobalObject : public QObject
{
   SCRIPT_T_CS_OBJECT(QScriptDebuggerConsoleGlobalObject)

 public:
   QScriptDebuggerConsoleGlobalObject(QObject *parent = nullptr);
   ~QScriptDebuggerConsoleGlobalObject();

   QScriptDebuggerCommandSchedulerInterface *scheduler() const;
   void setScheduler(QScriptDebuggerCommandSchedulerInterface *scheduler);

   QScriptDebuggerResponseHandlerInterface *responseHandler() const;
   void setResponseHandler(QScriptDebuggerResponseHandlerInterface *responseHandler);

   QScriptMessageHandlerInterface *messageHandler() const;
   void setMessageHandler(QScriptMessageHandlerInterface *messageHandler);

   QScriptDebuggerConsole *console() const;
   void setConsole(QScriptDebuggerConsole *console);
 
   CS_SLOT_1(Public, int scheduleInterrupt())
   CS_SLOT_2(scheduleInterrupt)
   CS_SLOT_1(Public, int scheduleContinue())
   CS_SLOT_2(scheduleContinue)
   CS_SLOT_1(Public, int scheduleStepInto(int count = 1))
   CS_SLOT_2(scheduleStepInto)
   CS_SLOT_1(Public, int scheduleStepOver(int count = 1))
   CS_SLOT_2(scheduleStepOver)
   CS_SLOT_1(Public, int scheduleStepOut())
   CS_SLOT_2(scheduleStepOut)
   CS_SLOT_1(Public, int scheduleRunToLocation(const QString &fileName, int lineNumber))
   CS_SLOT_2(scheduleRunToLocation)
   CS_SLOT_1(Public, int scheduleRunToLocation(qint64 scriptId, int lineNumber))
   CS_SLOT_2(scheduleRunToLocation)
   CS_SLOT_1(Public, int scheduleForceReturn(int contextIndex, const QScriptDebuggerValue &value))
   CS_SLOT_2(scheduleForceReturn)

   CS_SLOT_1(Public, int scheduleSetBreakpoint(const QScriptBreakpointData &data))
   CS_SLOT_2(scheduleSetBreakpoint)
   CS_SLOT_1(Public, int scheduleDeleteBreakpoint(int id))
   CS_SLOT_2(scheduleDeleteBreakpoint)
   CS_SLOT_1(Public, int scheduleDeleteAllBreakpoints())
   CS_SLOT_2(scheduleDeleteAllBreakpoints)
   CS_SLOT_1(Public, int scheduleGetBreakpoints())
   CS_SLOT_2(scheduleGetBreakpoints)
   CS_SLOT_1(Public, int scheduleGetBreakpointData(int id))
   CS_SLOT_2(scheduleGetBreakpointData)
   CS_SLOT_1(Public, int scheduleSetBreakpointData(int id, const QScriptBreakpointData &data))
   CS_SLOT_2(scheduleSetBreakpointData)

   CS_SLOT_1(Public, int scheduleGetScripts())
   CS_SLOT_2(scheduleGetScripts)
   CS_SLOT_1(Public, int scheduleGetScriptData(qint64 id))
   CS_SLOT_2(scheduleGetScriptData)
   CS_SLOT_1(Public, int scheduleScriptsCheckpoint())
   CS_SLOT_2(scheduleScriptsCheckpoint)
   CS_SLOT_1(Public, int scheduleGetScriptsDelta())
   CS_SLOT_2(scheduleGetScriptsDelta)
   CS_SLOT_1(Public, int scheduleResolveScript(const QString &fileName))
   CS_SLOT_2(scheduleResolveScript)

   CS_SLOT_1(Public, int scheduleGetBacktrace())
   CS_SLOT_2(scheduleGetBacktrace)
   CS_SLOT_1(Public, int scheduleGetThisObject(int contextIndex))
   CS_SLOT_2(scheduleGetThisObject)
   CS_SLOT_1(Public, int scheduleGetActivationObject(int contextIndex))
   CS_SLOT_2(scheduleGetActivationObject)
   CS_SLOT_1(Public, int scheduleGetContextCount())
   CS_SLOT_2(scheduleGetContextCount)
   CS_SLOT_1(Public, int scheduleGetContextInfo(int contextIndex))
   CS_SLOT_2(scheduleGetContextInfo)

   CS_SLOT_1(Public, int scheduleNewScriptValueIterator(const QScriptDebuggerValue &object))
   CS_SLOT_2(scheduleNewScriptValueIterator)
   CS_SLOT_1(Public, int scheduleGetPropertiesByIterator(int id, int count))
   CS_SLOT_2(scheduleGetPropertiesByIterator)
   CS_SLOT_1(Public, int scheduleDeleteScriptValueIterator(int id))
   CS_SLOT_2(scheduleDeleteScriptValueIterator)

   CS_SLOT_1(Public, int scheduleEvaluate(int contextIndex, const QString &program, const QString &fileName = QString(),
                                          intlineNumber = 1))
   CS_SLOT_2(scheduleEvaluate)

   CS_SLOT_1(Public, int scheduleScriptValueToString(const QScriptDebuggerValue &value))
   CS_SLOT_2(scheduleScriptValueToString)

   CS_SLOT_1(Public, int scheduleClearExceptions())
   CS_SLOT_2(scheduleClearExceptions)

   CS_SLOT_1(Public, int scheduleCommand(const QScriptDebuggerCommand &command))
   CS_SLOT_2(scheduleCommand)

   // message handler
   CS_SLOT_1(Public, void message(const QString &text, const QString &fileName = QString(), intlineNumber = -1,
                                  intcolumnNumber = -1))
   CS_SLOT_2(message)
   CS_SLOT_1(Public, void warning(const QString &text, const QString &fileName = QString(), intlineNumber = -1,
                                  intcolumnNumber = -1))
   CS_SLOT_2(warning)
   CS_SLOT_1(Public, void error(const QString &text, const QString &fileName = QString(), intlineNumber = -1,
                                intcolumnNumber = -1))
   CS_SLOT_2(error)

   // console state
   CS_SLOT_1(Public, int getCurrentFrameIndex())
   CS_SLOT_2(getCurrentFrameIndex)
   CS_SLOT_1(Public, void setCurrentFrameIndex(int index))
   CS_SLOT_2(setCurrentFrameIndex)
   CS_SLOT_1(Public, qint64 getCurrentScriptId())
   CS_SLOT_2(getCurrentScriptId)
   CS_SLOT_1(Public, void setCurrentScriptId(qint64 id))
   CS_SLOT_2(setCurrentScriptId)
   CS_SLOT_1(Public, qint64 getSessionId())
   CS_SLOT_2(getSessionId)
   CS_SLOT_1(Public, int getCurrentLineNumber())
   CS_SLOT_2(getCurrentLineNumber)
   CS_SLOT_1(Public, void setCurrentLineNumber(int lineNumber))
   CS_SLOT_2(setCurrentLineNumber)

   // command introspection
   CS_SLOT_1(Public, QScriptDebuggerConsoleCommandGroupMap getCommandGroups())
   CS_SLOT_2(getCommandGroups)
   CS_SLOT_1(Public, QScriptDebuggerConsoleCommand *findCommand(const QString &command)const)
   CS_SLOT_2(findCommand)
   CS_SLOT_1(Public, QScriptDebuggerConsoleCommandList getCommandsInGroup(const QString &name)const)
   CS_SLOT_2(getCommandsInGroup)
   CS_SLOT_1(Public, QStringList getCommandCompletions(const QString &prefix)const)
   CS_SLOT_2(getCommandCompletions)

   CS_SLOT_1(Public, bool checkSyntax(const QString &program))
   CS_SLOT_2(checkSyntax)

   CS_SLOT_1(Public, void setEvaluateAction(int action))
   CS_SLOT_2(setEvaluateAction)

 private:
   Q_DECLARE_PRIVATE(QScriptDebuggerConsoleGlobalObject)
   Q_DISABLE_COPY(QScriptDebuggerConsoleGlobalObject)
};

QT_END_NAMESPACE

#endif
