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

#ifndef QSCRIPTDEBUGGEREVENT_P_H
#define QSCRIPTDEBUGGEREVENT_P_H

#include <QtCore/qobjectdefs.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qhash.h>
#include <QtCore/qvariant.h>
#include <QtCore/qscopedpointer.h>

QT_BEGIN_NAMESPACE

class QDataStream;
class QScriptDebuggerValue;
class QScriptDebuggerEventPrivate;

class QScriptDebuggerEvent
{
 public:
   friend QDataStream &operator<<(QDataStream &, const QScriptDebuggerEvent &);
   friend QDataStream &operator>>(QDataStream &, QScriptDebuggerEvent &);

   enum Type {
      None,
      Interrupted,
      SteppingFinished,
      LocationReached,
      Breakpoint,
      Exception,
      Trace,
      InlineEvalFinished,
      DebuggerInvocationRequest,
      ForcedReturn,
      UserEvent = 1000,
      MaxUserEvent = 32767
   };

   enum Attribute {
      ScriptID,
      FileName,
      BreakpointID,
      LineNumber,
      ColumnNumber,
      Value,
      Message,
      IsNestedEvaluate,
      HasExceptionHandler,
      UserAttribute = 1000,
      MaxUserAttribute = 32767
   };

   QScriptDebuggerEvent();
   QScriptDebuggerEvent(Type type);
   QScriptDebuggerEvent(Type type, qint64 scriptId, int lineNumber, int columnNumber);
   QScriptDebuggerEvent(const QScriptDebuggerEvent &other);
   ~QScriptDebuggerEvent();

   Type type() const;

   QVariant attribute(Attribute attribute,
                      const QVariant &defaultValue = QVariant()) const;
   void setAttribute(Attribute attribute, const QVariant &value);
   QHash<Attribute, QVariant> attributes() const;

   qint64 scriptId() const;
   void setScriptId(qint64 id);
   QString fileName() const;
   void setFileName(const QString &fileName);
   int lineNumber() const;
   void setLineNumber(int lineNumber);
   int columnNumber() const;
   void setColumnNumber(int columnNumber);
   int breakpointId() const;
   void setBreakpointId(int id);
   QString message() const;
   void setMessage(const QString &message);
   QScriptDebuggerValue scriptValue() const;
   void setScriptValue(const QScriptDebuggerValue &value);
   void setNestedEvaluate(bool nested);
   bool isNestedEvaluate() const;
   void setHasExceptionHandler(bool hasHandler);
   bool hasExceptionHandler() const;

   QScriptDebuggerEvent &operator=(const QScriptDebuggerEvent &other);

   bool operator==(const QScriptDebuggerEvent &other) const;
   bool operator!=(const QScriptDebuggerEvent &other) const;

 private:
   QScopedPointer<QScriptDebuggerEventPrivate> d_ptr;

   Q_DECLARE_PRIVATE(QScriptDebuggerEvent)
};

QDataStream &operator<<(QDataStream &, const QScriptDebuggerEvent &);
QDataStream &operator>>(QDataStream &, QScriptDebuggerEvent &);

// helper class that's used to transport a debugger event through the Qt event loop
class QScriptDebuggerEventEvent : public QEvent
{
 public:
   QScriptDebuggerEventEvent(const QScriptDebuggerEvent &event)
      : QEvent(QEvent::Type(QEvent::User + 1)), m_event(event) {}
   ~QScriptDebuggerEventEvent() {}
   const QScriptDebuggerEvent &event() const {
      return m_event;
   }
 private:
   QScriptDebuggerEvent m_event;
};

QT_END_NAMESPACE

#endif
