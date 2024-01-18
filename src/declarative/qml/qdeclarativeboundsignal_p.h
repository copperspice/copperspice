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

#ifndef QDECLARATIVEBOUNDSIGNAL_P_H
#define QDECLARATIVEBOUNDSIGNAL_P_H

#include <qdeclarativeexpression.h>
#include <qdeclarativeguard_p.h>
#include <QtCore/qmetaobject.h>

QT_BEGIN_NAMESPACE

class QDeclarativeAbstractBoundSignal : public QObject
{
   DECL_CS_OBJECT(QDeclarativeAbstractBoundSignal)

 public:
   QDeclarativeAbstractBoundSignal(QObject *parent = nullptr);
   virtual ~QDeclarativeAbstractBoundSignal() = 0;
   virtual void disconnect() = 0;

 protected :
   DECL_CS_SLOT_1(Protected, virtual void unregisterScopeObject())
   DECL_CS_SLOT_2(unregisterScopeObject)
};

class QDeclarativeBoundSignalParameters;

class QDeclarativeBoundSignal : public QDeclarativeAbstractBoundSignal
{
 public:
   QDeclarativeBoundSignal(QObject *scope, const QMetaMethod &signal, QObject *parent);
   QDeclarativeBoundSignal(QDeclarativeContext *ctxt, const QString &val, QObject *scope,
                           const QMetaMethod &signal, QObject *parent);
   virtual ~QDeclarativeBoundSignal();

   void disconnect();

   int index() const;

   QDeclarativeExpression *expression() const;
   QDeclarativeExpression *setExpression(QDeclarativeExpression *);

   bool isEvaluating() const {
      return m_isEvaluating;
   }

   static QDeclarativeBoundSignal *cast(QObject *);

 protected:
   void unregisterScopeObject();
   virtual int qt_metacall(QMetaObject::Call c, int id, void **a);

 private:
   class ScopeGuard : public QDeclarativeGuard<QObject>
   {
    public:
      ScopeGuard(QObject *object, QDeclarativeBoundSignal *signal)
         : QDeclarativeGuard<QObject>(object), m_signal(signal) {
      }

      void objectDestroyed(QObject *obj) {
         Q_UNUSED(obj);
         m_signal->unregisterScopeObject();
      }

    private:
      QDeclarativeBoundSignal *m_signal;
   };

   void init(QObject *parent);

   QDeclarativeExpression *m_expression;
   QMetaMethod m_signal;
   bool m_paramsValid : 1;
   bool m_isEvaluating : 1;
   QDeclarativeBoundSignalParameters *m_params;
   ScopeGuard m_scope;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEBOUNDSIGNAL_P_H
