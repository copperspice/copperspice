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

#ifndef QSCRIPTDEBUGGERBACKEND_P_H
#define QSCRIPTDEBUGGERBACKEND_P_H

#include <QtCore/qobjectdefs.h>
#include <QtCore/qpair.h>

#include <qscriptbreakpointdata_p.h>
#include <qscriptscriptdata_p.h>

QT_BEGIN_NAMESPACE

class QScriptContext;
class QScriptEngine;
class QScriptDebuggerCommandExecutor;
class QScriptDebuggerEvent;
class QScriptValue;
class QScriptValueIterator;
class QScriptObjectSnapshot;
class QStringList;

typedef QPair<QList<qint64>, QList<qint64> > QScriptScriptsDelta;
typedef QPair<QList<qint64>, QList<qint64> > QScriptContextsDelta;

class QScriptDebuggerBackendPrivate;
class QScriptDebuggerBackend
{
 public:
   QScriptDebuggerBackend();
   virtual ~QScriptDebuggerBackend();

   void attachTo(QScriptEngine *engine);
   void detach();

   QScriptEngine *engine() const;

   void stepInto(int count = 1);
   void stepOver(int count = 1);
   void stepOut();
   void continueEvalution();
   void interruptEvaluation();
   void runToLocation(const QString &fileName, int lineNumber);
   void runToLocation(qint64 scriptId, int lineNumber);
   void returnToCaller(int contextIndex, const QScriptValue &value);
   void evaluate(int contextIndex, const QString &program,
                 const QString &fileName, int lineNumber);

   int setBreakpoint(const QScriptBreakpointData &data);
   bool deleteBreakpoint(int id);
   void deleteAllBreakpoints();
   QScriptBreakpointData breakpointData(int id) const;
   bool setBreakpointData(int id, const QScriptBreakpointData &data);
   QScriptBreakpointMap breakpoints() const;

   QScriptScriptMap scripts() const;
   QScriptScriptData scriptData(qint64 id) const;
   void scriptsCheckpoint();
   QScriptScriptsDelta scriptsDelta() const;
   qint64 resolveScript(const QString &fileName) const;

   int contextCount() const;
   QScriptContext *context(int index) const;
   QStringList backtrace() const;
   QList<qint64> contextIds() const;
   QScriptContextsDelta contextsCheckpoint();

   int newScriptObjectSnapshot();
   QScriptObjectSnapshot *scriptObjectSnapshot(int id) const;
   void deleteScriptObjectSnapshot(int id);

   int newScriptValueIterator(const QScriptValue &object);
   QScriptValueIterator *scriptValueIterator(int id) const;
   void deleteScriptValueIterator(int id);

   QScriptValue traceFunction() const;
   QScriptValue assertFunction() const;
   QScriptValue fileNameFunction() const;
   QScriptValue lineNumberFunction() const;

   void doPendingEvaluate(bool postEvent);

   bool ignoreExceptions() const;
   void setIgnoreExceptions(bool ignore);

   QScriptDebuggerCommandExecutor *commandExecutor() const;
   void setCommandExecutor(QScriptDebuggerCommandExecutor *executor);

   virtual void resume() = 0;

 protected:
   virtual void event(const QScriptDebuggerEvent &event) = 0;

 protected:
   QScriptDebuggerBackend(QScriptDebuggerBackendPrivate &dd);
   QScopedPointer<QScriptDebuggerBackendPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QScriptDebuggerBackend)
   Q_DISABLE_COPY(QScriptDebuggerBackend)
};

QT_END_NAMESPACE

#endif
