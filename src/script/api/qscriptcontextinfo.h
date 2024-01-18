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

#ifndef QSCRIPTCONTEXTINFO_H
#define QSCRIPTCONTEXTINFO_H

#include <qlist.h>
#include <qstringlist.h>
#include <qsharedpointer.h>

class QDataStream;
class QScriptContext;
class QScriptContextInfoPrivate;

class Q_SCRIPT_EXPORT QScriptContextInfo
{

 public:
   friend Q_SCRIPT_EXPORT QDataStream &operator<<(QDataStream &, const QScriptContextInfo &);
   friend Q_SCRIPT_EXPORT QDataStream &operator>>(QDataStream &, QScriptContextInfo &);

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
   int columnNumber() const;

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

Q_SCRIPT_EXPORT QDataStream &operator<<(QDataStream &, const QScriptContextInfo &);
Q_SCRIPT_EXPORT QDataStream &operator>>(QDataStream &, QScriptContextInfo &);

#endif
