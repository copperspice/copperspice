/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QSCRIPTCONTEXTINFO_H
#define QSCRIPTCONTEXTINFO_H

#include <QtCore/qlist.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qsharedpointer.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_DATASTREAM
class QDataStream;
#endif

class QScriptContext;
class QScriptContextInfoPrivate;

class Q_SCRIPT_EXPORT QScriptContextInfo
{

 public:
#ifndef QT_NO_DATASTREAM
   friend Q_SCRIPT_EXPORT QDataStream &operator<<(QDataStream &, const QScriptContextInfo &);
   friend Q_SCRIPT_EXPORT QDataStream &operator>>(QDataStream &, QScriptContextInfo &);
#endif

   enum FunctionType {
      ScriptFunction,
      QtFunction,
      QtPropertyFunction,
      NativeFunction
   };

   QScriptContextInfo(const QScriptContext *context);
   QScriptContextInfo(const QScriptContextInfo &other);
   QScriptContextInfo();
   ~QScriptContextInfo();

   QScriptContextInfo &operator=(const QScriptContextInfo &other);

   bool isNull() const;

   qint64 scriptId() const;
   QString fileName() const;
   int lineNumber() const;

#ifdef QT_DEPRECATED
   QT_DEPRECATED int columnNumber() const;
#endif

   QString functionName() const;
   FunctionType functionType() const;

   QStringList functionParameterNames() const;

   int functionStartLineNumber() const;
   int functionEndLineNumber() const;

   int functionMetaIndex() const;

   bool operator==(const QScriptContextInfo &other) const;
   bool operator!=(const QScriptContextInfo &other) const;

 private:
   QExplicitlySharedDataPointer<QScriptContextInfoPrivate> d_ptr;

   Q_DECLARE_PRIVATE(QScriptContextInfo)
};

typedef QList<QScriptContextInfo> QScriptContextInfoList;

#ifndef QT_NO_DATASTREAM
Q_SCRIPT_EXPORT QDataStream &operator<<(QDataStream &, const QScriptContextInfo &);
Q_SCRIPT_EXPORT QDataStream &operator>>(QDataStream &, QScriptContextInfo &);
#endif

QT_END_NAMESPACE

#endif
