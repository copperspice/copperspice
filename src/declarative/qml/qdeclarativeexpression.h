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

#ifndef QDECLARATIVEEXPRESSION_H
#define QDECLARATIVEEXPRESSION_H

#include <QtDeclarative/qdeclarativeerror.h>
#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QString;
class QDeclarativeRefCount;
class QDeclarativeEngine;
class QDeclarativeContext;
class QDeclarativeExpressionPrivate;
class QDeclarativeContextData;
class QScriptValue;

class Q_DECLARATIVE_EXPORT QDeclarativeExpression : public QObject
{
   DECL_CS_OBJECT(QDeclarativeExpression)

 public:
   QDeclarativeExpression();
   QDeclarativeExpression(QDeclarativeContext *, QObject *, const QString &, QObject * = 0);
   virtual ~QDeclarativeExpression();

   QDeclarativeEngine *engine() const;
   QDeclarativeContext *context() const;

   QString expression() const;
   void setExpression(const QString &);

   bool notifyOnValueChanged() const;
   void setNotifyOnValueChanged(bool);

   QString sourceFile() const;
   int lineNumber() const;
   void setSourceLocation(const QString &fileName, int line);

   QObject *scopeObject() const;

   bool hasError() const;
   void clearError();
   QDeclarativeError error() const;

   QVariant evaluate(bool *valueIsUndefined = 0);

   DECL_CS_SIGNAL_1(Public, void valueChanged())
   DECL_CS_SIGNAL_2(valueChanged)

 protected:
   QDeclarativeExpression(QDeclarativeContextData *, QObject *, const QString &, QDeclarativeExpressionPrivate &dd);
   QDeclarativeExpression(QDeclarativeContextData *, QObject *, const QScriptValue &, QDeclarativeExpressionPrivate &dd);

   QDeclarativeExpression(QDeclarativeContextData *, void *, QDeclarativeRefCount *rc,
                          QObject *me, const QString &, int, QDeclarativeExpressionPrivate &dd);

 private:
   QDeclarativeExpression(QDeclarativeContextData *, QObject *, const QString &);

   Q_DISABLE_COPY(QDeclarativeExpression)
   Q_DECLARE_PRIVATE(QDeclarativeExpression)

   DECL_CS_SLOT_1(Private, void _q_notify())
   DECL_CS_SLOT_2(_q_notify)

   friend class QDeclarativeDebugger;
   friend class QDeclarativeContext;
   friend class QDeclarativeVME;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEEXPRESSION_H

