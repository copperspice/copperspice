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

#ifndef QJSDEBUGGERAGENT_P_H
#define QJSDEBUGGERAGENT_P_H

#include <QtScript/qscriptengineagent.h>
#include <QtCore/qset.h>

QT_BEGIN_NAMESPACE
class QScriptValue;
class QDeclarativeEngine;
QT_END_NAMESPACE

QT_BEGIN_NAMESPACE

class QJSDebuggerAgentPrivate;

enum JSDebuggerState {
   NoState,
   SteppingIntoState,
   SteppingOverState,
   SteppingOutState,
   StoppedState
};

enum JSCoverageMessage {
   CoverageLocation,
   CoverageScriptLoad,
   CoveragePosChange,
   CoverageFuncEntry,
   CoverageFuncExit,
   CoverageComplete,

   CoverageMaximumMessage
};

struct JSAgentWatchData {
   QByteArray exp;
   QByteArray name;
   QByteArray value;
   QByteArray type;
   bool hasChildren;
   quint64 objectId;
};

inline QDataStream &operator<<(QDataStream &s, const JSAgentWatchData &data)
{
   return s << data.exp << data.name << data.value
          << data.type << data.hasChildren << data.objectId;
}

inline QDataStream &operator>>(QDataStream &s, JSAgentWatchData &data)
{
   return s >> data.exp >> data.name >> data.value
          >> data.type >> data.hasChildren >> data.objectId;
}

struct JSAgentStackData {
   QByteArray functionName;
   QByteArray fileUrl;
   qint32 lineNumber;
};

inline QDataStream &operator<<(QDataStream &s, const JSAgentStackData &data)
{
   return s << data.functionName << data.fileUrl << data.lineNumber;
}

inline QDataStream &operator>>(QDataStream &s, JSAgentStackData &data)
{
   return s >> data.functionName >> data.fileUrl >> data.lineNumber;
}

struct JSAgentBreakpointData {
   QByteArray functionName;
   QByteArray fileUrl;
   qint32 lineNumber;
};

typedef QSet<JSAgentBreakpointData> JSAgentBreakpoints;

inline QDataStream &operator<<(QDataStream &s, const JSAgentBreakpointData &data)
{
   return s << data.functionName << data.fileUrl << data.lineNumber;
}

inline QDataStream &operator>>(QDataStream &s, JSAgentBreakpointData &data)
{
   return s >> data.functionName >> data.fileUrl >> data.lineNumber;
}

inline bool operator==(const JSAgentBreakpointData &b1, const JSAgentBreakpointData &b2)
{
   return b1.lineNumber == b2.lineNumber && b1.fileUrl == b2.fileUrl;
}

inline uint qHash(const JSAgentBreakpointData &b)
{
   return b.lineNumber ^ qHash(b.fileUrl);
}


class QJSDebuggerAgent : public QObject, public QScriptEngineAgent
{
   DECL_CS_OBJECT(QJSDebuggerAgent)

 public:
   QJSDebuggerAgent(QScriptEngine *engine, QObject *parent = nullptr);
   QJSDebuggerAgent(QDeclarativeEngine *engine, QObject *parent = nullptr);
   ~QJSDebuggerAgent();

   bool isInitialized() const;

   void setBreakpoints(const JSAgentBreakpoints &);
   void setWatchExpressions(const QStringList &);

   void stepOver();
   void stepInto();
   void stepOut();
   void continueExecution();
   void setCoverageEnabled(bool enabled);

   JSAgentWatchData executeExpression(const QString &expr);
   QList<JSAgentWatchData> expandObjectById(quint64 objectId);
   QList<JSAgentWatchData> locals();
   QList<JSAgentWatchData> localsAtFrame(int frameId);
   QList<JSAgentStackData> backtrace();
   QList<JSAgentWatchData> watches();
   void setProperty(qint64 objectId,
                    const QString &property,
                    const QString &value);

   // reimplemented
   void scriptLoad(qint64 id, const QString &program,
                   const QString &fileName, int baseLineNumber);
   void scriptUnload(qint64 id);

   void contextPush();
   void contextPop();

   void functionEntry(qint64 scriptId);
   void functionExit(qint64 scriptId,
                     const QScriptValue &returnValue);

   void positionChange(qint64 scriptId,
                       int lineNumber, int columnNumber);

   void exceptionThrow(qint64 scriptId,
                       const QScriptValue &exception,
                       bool hasHandler);
   void exceptionCatch(qint64 scriptId,
                       const QScriptValue &exception);

   bool supportsExtension(Extension extension) const;
   QVariant extension(Extension extension,
                      const QVariant &argument = QVariant());

 public:
   DECL_CS_SIGNAL_1(Public, void stopped(bool becauseOfException, const QString &exception))
   DECL_CS_SIGNAL_2(stopped, becauseOfException, exception)

 private:
   friend class QJSDebuggerAgentPrivate;
   QJSDebuggerAgentPrivate *d;
};

QT_END_NAMESPACE

#endif // QJSDEBUGGERAGENT_P_H
