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

#ifndef QSCRIPTENGINEAGENT_H
#define QSCRIPTENGINEAGENT_H

#include <qvariant.h>
#include <qscopedpointer.h>

class QScriptEngine;
class QScriptValue;
class QScriptEngineAgentPrivate;

class Q_SCRIPT_EXPORT QScriptEngineAgent
{
 public:
   enum Extension {
      DebuggerInvocationRequest
   };

   QScriptEngineAgent(QScriptEngine *engine);

   QScriptEngineAgent(const QScriptEngineAgent &) = delete;
   QScriptEngineAgent &operator=(const QScriptEngineAgent &) = delete;

   virtual ~QScriptEngineAgent();

   virtual void scriptLoad(qint64 id, const QString &program, const QString &fileName, int baseLineNumber);
   virtual void scriptUnload(qint64 id);

   virtual void contextPush();
   virtual void contextPop();

   virtual void functionEntry(qint64 scriptId);
   virtual void functionExit(qint64 scriptId, const QScriptValue &returnValue);

   virtual void positionChange(qint64 scriptId, int lineNumber, int columnNumber);

   virtual void exceptionThrow(qint64 scriptId, const QScriptValue &exception, bool hasHandler);
   virtual void exceptionCatch(qint64 scriptId, const QScriptValue &exception);

   virtual bool supportsExtension(Extension extension) const;
   virtual QVariant extension(Extension extension, const QVariant &argument = QVariant());

   QScriptEngine *engine() const;

 protected:
   QScriptEngineAgent(QScriptEngineAgentPrivate &dd, QScriptEngine *engine);
   QScopedPointer<QScriptEngineAgentPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QScriptEngineAgent)
};

#endif
