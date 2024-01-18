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

#ifndef QDECLARATIVEEXPRESSION_P_H
#define QDECLARATIVEEXPRESSION_P_H

#include <qdeclarativeexpression.h>
#include <qdeclarativeengine_p.h>
#include <qdeclarativeguard_p.h>
#include <QtScript/qscriptvalue.h>

QT_BEGIN_NAMESPACE

class QDeclarativeAbstractExpression
{
 public:
   QDeclarativeAbstractExpression();
   virtual ~QDeclarativeAbstractExpression();

   bool isValid() const;

   QDeclarativeContextData *context() const;
   void setContext(QDeclarativeContextData *);

   virtual void refresh();

 private:
   friend class QDeclarativeContext;
   friend class QDeclarativeContextData;
   friend class QDeclarativeContextPrivate;
   QDeclarativeContextData *m_context;
   QDeclarativeAbstractExpression **m_prevExpression;
   QDeclarativeAbstractExpression  *m_nextExpression;
};

class QDeclarativeDelayedError
{
 public:
   inline QDeclarativeDelayedError() : nextError(0), prevError(0) {}
   inline ~QDeclarativeDelayedError() {
      removeError();
   }

   QDeclarativeError error;

   bool addError(QDeclarativeEnginePrivate *);

   inline void removeError() {
      if (!prevError) {
         return;
      }
      if (nextError) {
         nextError->prevError = prevError;
      }
      *prevError = nextError;
      nextError = 0;
      prevError = 0;
   }

 private:
   QDeclarativeDelayedError  *nextError;
   QDeclarativeDelayedError **prevError;
};

class QDeclarativeQtScriptExpression : public QDeclarativeAbstractExpression,
   public QDeclarativeDelayedError
{
 public:
   enum Mode { SharedContext, ExplicitContext };

   enum EvaluateFlag { RequiresThisObject = 0x01 };
   using EvaluateFlags = QFlags<EvaluateFlag>;

   QDeclarativeQtScriptExpression();
   virtual ~QDeclarativeQtScriptExpression();

   QDeclarativeRefCount *dataRef;

   QString expression;

   Mode expressionFunctionMode;
   QScriptValue expressionFunction;

   QScriptValue expressionContext; // Only used in ExplicitContext
   QObject *scopeObject;           // Only used in SharedContext

   bool notifyOnValueChange() const;
   void setNotifyOnValueChange(bool);
   void resetNotifyOnChange();
   void setNotifyObject(QObject *, int );

   void setEvaluateFlags(EvaluateFlags flags);
   EvaluateFlags evaluateFlags() const;

   QScriptValue scriptValue(QObject *secondaryScope, bool *isUndefined);

   class DeleteWatcher
   {
    public:
      inline DeleteWatcher(QDeclarativeQtScriptExpression *data);
      inline ~DeleteWatcher();
      inline bool wasDeleted() const;
    private:
      bool *m_wasDeleted;
      bool m_wasDeletedStorage;
      QDeclarativeQtScriptExpression *m_d;
   };

 private:
   void clearGuards();
   QScriptValue eval(QObject *secondaryScope, bool *isUndefined);
   void updateGuards(const QPODVector<QDeclarativeEnginePrivate::CapturedProperty, 16> &properties);

   bool trackChange;

   QDeclarativeNotifierEndpoint *guardList;
   int guardListLength;

   QObject *guardObject;
   int guardObjectNotifyIndex;
   bool *deleted;

   EvaluateFlags evalFlags;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDeclarativeQtScriptExpression::EvaluateFlags)


class QDeclarativeExpression;
class QString;
class QDeclarativeExpressionPrivate : public QDeclarativeQtScriptExpression
{
   Q_DECLARE_PUBLIC(QDeclarativeExpression)

 public:
   QDeclarativeExpressionPrivate();
   ~QDeclarativeExpressionPrivate();

   void init(QDeclarativeContextData *, const QString &, QObject *);
   void init(QDeclarativeContextData *, const QScriptValue &, QObject *);
   void init(QDeclarativeContextData *, void *, QDeclarativeRefCount *, QObject *, const QString &, int);

   QVariant value(QObject *secondaryScope = 0, bool *isUndefined = 0);
   QScriptValue scriptValue(QObject *secondaryScope = 0, bool *isUndefined = 0);

   static QDeclarativeExpressionPrivate *get(QDeclarativeExpression *expr) {
      return static_cast<QDeclarativeExpressionPrivate *>(QObjectPrivate::get(expr));
   }
   static QDeclarativeExpression *get(QDeclarativeExpressionPrivate *expr) {
      return expr->q_func();
   }

   void _q_notify();
   virtual void emitValueChanged();

   static void exceptionToError(QScriptEngine *, QDeclarativeError &);
   static QScriptValue evalInObjectScope(QDeclarativeContextData *, QObject *, const QString &, const QString &,
                                         int, QScriptValue *);
   static QScriptValue evalInObjectScope(QDeclarativeContextData *, QObject *, const QScriptProgram &,
                                         QScriptValue *);

   bool expressionFunctionValid: 1;

   QString url; // This is a QString for a reason.  QUrls are slooooooow...
   int line;
   QByteArray name; //function name, hint for the debugger
};

QDeclarativeQtScriptExpression::DeleteWatcher::DeleteWatcher(QDeclarativeQtScriptExpression *data)
   : m_wasDeletedStorage(false), m_d(data)
{
   if (!m_d->deleted) {
      m_d->deleted = &m_wasDeletedStorage;
   }
   m_wasDeleted = m_d->deleted;
}

QDeclarativeQtScriptExpression::DeleteWatcher::~DeleteWatcher()
{
   if (false == *m_wasDeleted && m_wasDeleted == m_d->deleted) {
      m_d->deleted = 0;
   }
}

bool QDeclarativeQtScriptExpression::DeleteWatcher::wasDeleted() const
{
   return *m_wasDeleted;
}

QT_END_NAMESPACE

#endif // QDECLARATIVEEXPRESSION_P_H
