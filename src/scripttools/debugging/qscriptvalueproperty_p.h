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

#ifndef QSCRIPTVALUEPROPERTY_P_H
#define QSCRIPTVALUEPROPERTY_P_H

#include <QtCore/qobjectdefs.h>
#include <QtCore/qlist.h>
#include <qscopedpointer_p.h>
#include <QtScript/qscriptvalue.h>

QT_BEGIN_NAMESPACE

class QString;
class QScriptValuePropertyPrivate;

class QScriptValueProperty
{
 public:
   QScriptValueProperty();
   QScriptValueProperty(const QString &name,
                        const QScriptValue &value,
                        QScriptValue::PropertyFlags flags);
   QScriptValueProperty(const QScriptValueProperty &other);
   ~QScriptValueProperty();

   QScriptValueProperty &operator=(const QScriptValueProperty &other);

   QString name() const;
   QScriptValue value() const;
   QScriptValue::PropertyFlags flags() const;

   bool isValid() const;

 private:
   QScopedSharedPointer<QScriptValuePropertyPrivate> d_ptr;

   Q_DECLARE_PRIVATE(QScriptValueProperty)
};

typedef QList<QScriptValueProperty> QScriptValuePropertyList;

QT_END_NAMESPACE

#endif
