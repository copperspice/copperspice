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

#include <qdeclarativeboundsignal_p.h>
#include <qmetaobjectbuilder_p.h>
#include <qdeclarativeengine_p.h>
#include <qdeclarativeexpression_p.h>
#include <qdeclarativecontext_p.h>
#include <qdeclarativemetatype_p.h>
#include <qdeclarative.h>
#include <qdeclarativecontext.h>
#include <qdeclarativeglobal_p.h>
#include <qdeclarativedebugtrace_p.h>

#include <QtCore/qstringbuilder.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

class QDeclarativeBoundSignalParameters : public QObject
{
   DECL_CS_OBJECT(QDeclarativeBoundSignalParameters)

 public:
   QDeclarativeBoundSignalParameters(const QMetaMethod &, QObject * = 0);
   ~QDeclarativeBoundSignalParameters();

   void setValues(void **);
   void clearValues();

 private:
   friend class MetaObject;
   int metaCall(QMetaObject::Call, int _id, void **);
   struct MetaObject : public QAbstractDynamicMetaObject {
      MetaObject(QDeclarativeBoundSignalParameters *b)
         : parent(b) {}

      int metaCall(QMetaObject::Call c, int id, void **a) {
         return parent->metaCall(c, id, a);
      }
      QDeclarativeBoundSignalParameters *parent;
   };

   int *types;
   void **values;
   QMetaObject *myMetaObject;
};

static int evaluateIdx = -1;

QDeclarativeAbstractBoundSignal::QDeclarativeAbstractBoundSignal(QObject *parent)
   : QObject(parent)
{
}

QDeclarativeAbstractBoundSignal::~QDeclarativeAbstractBoundSignal()
{
}

QDeclarativeBoundSignal::QDeclarativeBoundSignal(QObject *scope, const QMetaMethod &signal, QObject *parent)
   : m_expression(0), m_signal(signal), m_paramsValid(false), m_isEvaluating(false), m_params(0), m_scope(scope, this)
{
   init(parent);
}

QDeclarativeBoundSignal::QDeclarativeBoundSignal(QDeclarativeContext *ctxt, const QString &val,
      QObject *scope, const QMetaMethod &signal, QObject *parent)
   : m_expression(0), m_signal(signal), m_paramsValid(false), m_isEvaluating(false), m_params(0), m_scope(scope, this)
{
   init(parent);
   m_expression = new QDeclarativeExpression(ctxt, scope, val);
}

void QDeclarativeBoundSignal::init(QObject *parent)
{
   // This is thread safe.  Although it may be updated by two threads, they
   // will both set it to the same value - so the worst thing that can happen
   // is that they both do the work to figure it out.  Boo hoo.
   if (evaluateIdx == -1) {
      evaluateIdx = metaObject()->methodCount();
   }

   QDeclarative_setParent_noEvent(this, parent);
   QDeclarativePropertyPrivate::connect(m_scope, m_signal.methodIndex(), this, evaluateIdx);

   QDeclarativeData *const data = QDeclarativeData::get(m_scope, true);
   data->addBoundSignal(this);
}

QDeclarativeBoundSignal::~QDeclarativeBoundSignal()
{
   unregisterScopeObject();
   delete m_expression;
   m_expression = 0;
}

void QDeclarativeBoundSignal::disconnect()
{
   QMetaObject::disconnect(m_scope, m_signal.methodIndex(), this, evaluateIdx);
   QObjectPrivate *const priv = QObjectPrivate::get(m_scope);
   QVarLengthArray<char> signalSignature;
   QObjectPrivate::signalSignature(m_signal, &signalSignature);
   priv->disconnectNotify(signalSignature.constData());
}

int QDeclarativeBoundSignal::index() const
{
   return m_signal.methodIndex();
}

/*!
    Returns the signal expression.
*/
QDeclarativeExpression *QDeclarativeBoundSignal::expression() const
{
   return m_expression;
}

/*!
    Sets the signal expression to \a e.  Returns the current signal expression,
    or null if there is no signal expression.

    The QDeclarativeBoundSignal instance takes ownership of \a e.  The caller is
    assumes ownership of the returned QDeclarativeExpression.
*/
QDeclarativeExpression *QDeclarativeBoundSignal::setExpression(QDeclarativeExpression *e)
{
   QDeclarativeExpression *rv = m_expression;
   m_expression = e;
   if (m_expression) {
      m_expression->setNotifyOnValueChanged(false);
   }
   return rv;
}

QDeclarativeBoundSignal *QDeclarativeBoundSignal::cast(QObject *o)
{
   QDeclarativeAbstractBoundSignal *s = qobject_cast<QDeclarativeAbstractBoundSignal *>(o);
   return static_cast<QDeclarativeBoundSignal *>(s);
}

int QDeclarativeBoundSignal::qt_metacall(QMetaObject::Call c, int id, void **a)
{
   if (c == QMetaObject::InvokeMetaMethod && id == evaluateIdx) {
      if (!m_expression) {
         return -1;
      }
      if (QDeclarativeDebugService::isDebuggingEnabled()) {
         QDeclarativeDebugTrace::startRange(QDeclarativeDebugTrace::HandlingSignal);
         QDeclarativeDebugTrace::rangeData(QDeclarativeDebugTrace::HandlingSignal,
                                           QLatin1String(m_signal.signature()) % QLatin1String(": ") % m_expression->expression());
         QDeclarativeDebugTrace::rangeLocation(QDeclarativeDebugTrace::HandlingSignal, m_expression->sourceFile(),
                                               m_expression->lineNumber());
      }
      m_isEvaluating = true;
      if (!m_paramsValid) {
         if (!m_signal.parameterTypes().isEmpty()) {
            m_params = new QDeclarativeBoundSignalParameters(m_signal, this);
         }
         m_paramsValid = true;
      }

      if (m_params) {
         m_params->setValues(a);
      }
      if (m_expression && m_expression->engine()) {
         QDeclarativeExpressionPrivate::get(m_expression)->value(m_params);
         if (m_expression && m_expression->hasError()) {
            QDeclarativeEnginePrivate::warning(m_expression->engine(), m_expression->error());
         }
      }
      if (m_params) {
         m_params->clearValues();
      }
      m_isEvaluating = false;
      QDeclarativeDebugTrace::endRange(QDeclarativeDebugTrace::HandlingSignal);
      return -1;
   } else {
      return QObject::qt_metacall(c, id, a);
   }
}

void QDeclarativeBoundSignal::unregisterScopeObject()
{
   if (m_scope) {
      QDeclarativeData *const data = QDeclarativeData::get(m_scope, false);
      if (data) {
         data->removeBoundSignal(this);
      }
   }
}

QDeclarativeBoundSignalParameters::QDeclarativeBoundSignalParameters(const QMetaMethod &method,
      QObject *parent)
   : QObject(parent), types(0), values(0)
{
   MetaObject *mo = new MetaObject(this);

   // ### Optimize!
   QMetaObjectBuilder mob;
   mob.setSuperClass(&QDeclarativeBoundSignalParameters::staticMetaObject);
   mob.setClassName("QDeclarativeBoundSignalParameters");

   QList<QByteArray> paramTypes = method.parameterTypes();
   QList<QByteArray> paramNames = method.parameterNames();
   types = new int[paramTypes.count()];
   for (int ii = 0; ii < paramTypes.count(); ++ii) {
      const QByteArray &type = paramTypes.at(ii);
      const QByteArray &name = paramNames.at(ii);

      if (name.isEmpty() || type.isEmpty()) {
         types[ii] = 0;
         continue;
      }

      QVariant::Type t = (QVariant::Type)QMetaType::type(type.constData());
      if (QDeclarativeMetaType::isQObject(t)) {
         types[ii] = QMetaType::QObjectStar;
         QMetaPropertyBuilder prop = mob.addProperty(name, "QObject*");
         prop.setWritable(false);
      } else {
         QByteArray propType = type;
         if (t >= QVariant::UserType || t == QVariant::Invalid) {
            //copy of QDeclarativeObjectScriptClass::enumType()
            QByteArray scope;
            QByteArray name;
            int scopeIdx = propType.lastIndexOf("::");
            if (scopeIdx != -1) {
               scope = propType.left(scopeIdx);
               name = propType.mid(scopeIdx + 2);
            } else {
               name = propType;
            }
            const QMetaObject *meta;
            if (scope == "Qt") {
               meta = &QObject::staticQtMetaObject;
            } else {
               meta = parent->parent()->metaObject();   //### assumes parent->parent()
            }
            for (int i = meta->enumeratorCount() - 1; i >= 0; --i) {
               QMetaEnum m = meta->enumerator(i);
               if ((m.name() == name) && (scope.isEmpty() || (m.scope() == scope))) {
                  t = QVariant::Int;
                  propType = "int";
                  break;
               }
            }
         }
         if (QDeclarativeMetaType::canCopy(t)) {
            types[ii] = t;
            QMetaPropertyBuilder prop = mob.addProperty(name, propType);
            prop.setWritable(false);
         } else {
            types[ii] = 0x80000000 | t;
            QMetaPropertyBuilder prop = mob.addProperty(name, "QVariant");
            prop.setWritable(false);
         }
      }
   }
   myMetaObject = mob.toMetaObject();
   *static_cast<QMetaObject *>(mo) = *myMetaObject;

   d_ptr->metaObject = mo;
}

QDeclarativeBoundSignalParameters::~QDeclarativeBoundSignalParameters()
{
   delete [] types;
   qFree(myMetaObject);
}

void QDeclarativeBoundSignalParameters::setValues(void **v)
{
   values = v;
}

void QDeclarativeBoundSignalParameters::clearValues()
{
   values = 0;
}

int QDeclarativeBoundSignalParameters::metaCall(QMetaObject::Call c, int id, void **a)
{
   if (!values) {
      return -1;
   }

   if (c == QMetaObject::ReadProperty && id >= 1) {
      if (types[id - 1] & 0x80000000) {
         *((QVariant *)a[0]) = QVariant(types[id - 1] & 0x7FFFFFFF, values[id]);
      } else {
         QDeclarativeMetaType::copy(types[id - 1], a[0], values[id]);
      }
      return -1;
   } else {
      return qt_metacall(c, id, a);
   }
}

QT_END_NAMESPACE
