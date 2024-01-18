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

#ifndef QSCRIPTDEBUGGERVALUEPROPERTY_P_H
#define QSCRIPTDEBUGGERVALUEPROPERTY_P_H

#include <QtCore/qobjectdefs.h>
#include <QtCore/qlist.h>
#include <qscopedpointer_p.h>
#include <QtScript/qscriptvalue.h>

QT_BEGIN_NAMESPACE

class QDataStream;
class QString;
class QScriptDebuggerValue;
class QScriptDebuggerValuePropertyPrivate;

class QScriptDebuggerValueProperty
{
 public:
   QScriptDebuggerValueProperty();
   QScriptDebuggerValueProperty(const QString &name,
                                const QScriptDebuggerValue &value,
                                const QString &valueAsString,
                                QScriptValue::PropertyFlags flags);
   QScriptDebuggerValueProperty(const QScriptDebuggerValueProperty &other);
   ~QScriptDebuggerValueProperty();

   QScriptDebuggerValueProperty &operator=(const QScriptDebuggerValueProperty &other);

   QString name() const;
   QScriptDebuggerValue value() const;
   QString valueAsString() const;
   QScriptValue::PropertyFlags flags() const;

   bool isValid() const;

 private:
   QScopedSharedPointer<QScriptDebuggerValuePropertyPrivate> d_ptr;

   Q_DECLARE_PRIVATE(QScriptDebuggerValueProperty)
};

typedef QList<QScriptDebuggerValueProperty> QScriptDebuggerValuePropertyList;

QDataStream &operator<<(QDataStream &, const QScriptDebuggerValueProperty &);
QDataStream &operator>>(QDataStream &, QScriptDebuggerValueProperty &);

QT_END_NAMESPACE

#endif
