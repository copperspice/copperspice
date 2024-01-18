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

#ifndef QSCRIPTDEBUGGERRESPONSE_P_H
#define QSCRIPTDEBUGGERRESPONSE_P_H

#include <QtCore/qobjectdefs.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qmap.h>
#include <QtCore/qvariant.h>

#include <qscriptbreakpointdata_p.h>
#include <qscriptscriptdata_p.h>
#include <qscriptdebuggervalue_p.h>
#include <qscriptdebuggervalueproperty_p.h>

QT_BEGIN_NAMESPACE

class QDataStream;
class QScriptContextInfo;
class QScriptDebuggerResponsePrivate;

class QScriptDebuggerResponse
{
 public:
   friend QDataStream &operator<<(QDataStream &, const QScriptDebuggerResponse &);
   friend QDataStream &operator>>(QDataStream &, QScriptDebuggerResponse &);

   enum Error {
      NoError,
      InvalidContextIndex,
      InvalidArgumentIndex,
      InvalidScriptID,
      InvalidBreakpointID,
      UserError = 1000,
      MaxUserError = 32767
   };

   QScriptDebuggerResponse();
   QScriptDebuggerResponse(const QScriptDebuggerResponse &other);
   ~QScriptDebuggerResponse();

   Error error() const;
   void setError(Error error);

   QVariant result() const;
   void setResult(const QVariant &value);

   void setResult(int value);
   void setResult(const QString &value);
   void setResult(const QScriptBreakpointData &data);
   void setResult(const QScriptBreakpointMap &breakpoints);
   void setResult(const QScriptScriptMap &scripts);
   void setResult(const QScriptScriptData &data);
   void setResult(const QScriptDebuggerValue &value);
   void setResult(const QScriptDebuggerValueList &value);
   void setResult(const QScriptDebuggerValuePropertyList &props);
   void setResult(const QScriptContextInfo &info);

   int resultAsInt() const;
   qint64 resultAsLongLong() const;
   QString resultAsString() const;
   QScriptBreakpointData resultAsBreakpointData() const;
   QScriptBreakpointMap resultAsBreakpoints() const;
   QScriptScriptMap resultAsScripts() const;
   QScriptScriptData resultAsScriptData() const;
   QScriptDebuggerValue resultAsScriptValue() const;
   QScriptDebuggerValueList resultAsScriptValueList() const;
   QScriptDebuggerValuePropertyList resultAsScriptValuePropertyList() const;
   QScriptContextInfo resultAsContextInfo() const;

   bool async() const;
   void setAsync(bool async);

   QScriptDebuggerResponse &operator=(const QScriptDebuggerResponse &other);

   bool operator==(const QScriptDebuggerResponse &other) const;
   bool operator!=(const QScriptDebuggerResponse &other) const;

 private:
   QScopedPointer<QScriptDebuggerResponsePrivate> d_ptr;

   Q_DECLARE_PRIVATE(QScriptDebuggerResponse)
};

QDataStream &operator<<(QDataStream &, const QScriptDebuggerResponse &);
QDataStream &operator>>(QDataStream &, QScriptDebuggerResponse &);

QT_END_NAMESPACE

#endif
