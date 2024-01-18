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

#include <qdeclarativebinding_p.h>
#include <qdeclarativebinding_p_p.h>
#include <qdeclarative.h>
#include <qdeclarativecontext.h>
#include <qdeclarativeinfo.h>
#include <qdeclarativecontext_p.h>
#include <qdeclarativecompiler_p.h>
#include <qdeclarativedata_p.h>
#include <qdeclarativestringconverters_p.h>
#include <qdeclarativestate_p_p.h>
#include <qdeclarativedebugtrace_p.h>

#include <QVariant>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

QDeclarativeAbstractBinding::QDeclarativeAbstractBinding()
   : m_object(0), m_propertyIndex(-1), m_mePtr(0), m_prevBinding(0), m_nextBinding(0)
{
}

QDeclarativeAbstractBinding::~QDeclarativeAbstractBinding()
{
   Q_ASSERT(m_prevBinding == 0);
   Q_ASSERT(m_mePtr == 0);
}

/*!
Destroy the binding.  Use this instead of calling delete.

Bindings are free to implement their own memory management, so the delete operator is not
necessarily safe.  The default implementation clears the binding, removes it from the object
and calls delete.
*/
void QDeclarativeAbstractBinding::destroy(DestroyMode mode)
{
   if (mode == DisconnectBinding) {
      disconnect(QDeclarativeAbstractBinding::DisconnectOne);
   }

   removeFromObject();
   clear();

   delete this;
}

/*!
Add this binding to \a object.

This transfers ownership of the binding to the object, marks the object's property as
being bound.

However, it does not enable the binding itself or call update() on it.
*/
void QDeclarativeAbstractBinding::addToObject(QObject *object, int index)
{
   Q_ASSERT(object);

   if (m_object == object && m_propertyIndex == index) {
      return;
   }

   removeFromObject();

   Q_ASSERT(!m_prevBinding);

   m_object = object;
   m_propertyIndex = index;

   QDeclarativeData *data = QDeclarativeData::get(object, true);

   if (index & 0xFF000000) {
      // Value type

      int coreIndex = index & 0xFFFFFF;

      // Find the value type proxy (if there is one)
      QDeclarativeValueTypeProxyBinding *proxy = 0;
      if (data->hasBindingBit(coreIndex)) {
         QDeclarativeAbstractBinding *b = data->bindings;
         while (b && b->propertyIndex() != coreIndex) {
            b = b->m_nextBinding;
         }
         Q_ASSERT(b && b->bindingType() == QDeclarativeAbstractBinding::ValueTypeProxy);
         proxy = static_cast<QDeclarativeValueTypeProxyBinding *>(b);
      }

      if (!proxy) {
         proxy = new QDeclarativeValueTypeProxyBinding(object, coreIndex);
         proxy->addToObject(object, coreIndex);
      }

      m_nextBinding = proxy->m_bindings;
      if (m_nextBinding) {
         m_nextBinding->m_prevBinding = &m_nextBinding;
      }
      m_prevBinding = &proxy->m_bindings;
      proxy->m_bindings = this;

   } else {
      m_nextBinding = data->bindings;
      if (m_nextBinding) {
         m_nextBinding->m_prevBinding = &m_nextBinding;
      }
      m_prevBinding = &data->bindings;
      data->bindings = this;

      data->setBindingBit(m_object, index);
   }
}

/*!
Remove the binding from the object.
*/
void QDeclarativeAbstractBinding::removeFromObject()
{
   if (m_prevBinding) {
      int index = propertyIndex();

      *m_prevBinding = m_nextBinding;
      if (m_nextBinding) {
         m_nextBinding->m_prevBinding = m_prevBinding;
      }
      m_prevBinding = 0;
      m_nextBinding = 0;

      if (index & 0xFF000000) {
         // Value type - we don't remove the proxy from the object.  It will sit their happily
         // doing nothing until it is removed by a write, a binding change or it is reused
         // to hold more sub-bindings.
      } else if (m_object) {
         QDeclarativeData *data = QDeclarativeData::get(m_object, false);
         if (data) {
            data->clearBindingBit(index);
         }
      }

      m_object = 0;
      m_propertyIndex = -1;
   }
}

static void bindingDummyDeleter(QDeclarativeAbstractBinding *)
{
}

QDeclarativeAbstractBinding::Pointer QDeclarativeAbstractBinding::weakPointer()
{
   if (m_selfPointer.isNull()) {
      m_selfPointer = QSharedPointer<QDeclarativeAbstractBinding>(this, bindingDummyDeleter);
   }

   return m_selfPointer.toWeakRef();
}

void QDeclarativeAbstractBinding::clear()
{
   if (m_mePtr) {
      *m_mePtr = 0;
      m_mePtr = 0;
   }
}

QString QDeclarativeAbstractBinding::expression() const
{
   return QLatin1String("<Unknown>");
}

QObject *QDeclarativeAbstractBinding::object() const
{
   return m_object;
}

int QDeclarativeAbstractBinding::propertyIndex() const
{
   return m_propertyIndex;
}

void QDeclarativeAbstractBinding::setEnabled(bool enabled, QDeclarativePropertyPrivate::WriteFlags flags)
{
   if (enabled) {
      update(flags);
   }
}

QDeclarativeBinding::Identifier QDeclarativeBinding::Invalid = -1;

void QDeclarativeBindingPrivate::refresh()
{
   Q_Q(QDeclarativeBinding);
   q->update();
}

QDeclarativeBindingPrivate::QDeclarativeBindingPrivate()
   : updating(false), enabled(false), deleted(0)
{
}

QDeclarativeBindingPrivate::~QDeclarativeBindingPrivate()
{
   if (deleted) {
      *deleted = true;
   }
}

QDeclarativeBinding::QDeclarativeBinding(void *data, QDeclarativeRefCount *rc, QObject *obj,
      QDeclarativeContextData *ctxt, const QString &url, int lineNumber,
      QObject *parent)
   : QDeclarativeExpression(ctxt, data, rc, obj, url, lineNumber, *new QDeclarativeBindingPrivate)
{
   setParent(parent);
   setNotifyOnValueChanged(true);
}

QDeclarativeBinding *
QDeclarativeBinding::createBinding(Identifier id, QObject *obj, QDeclarativeContext *ctxt,
                                   const QString &url, int lineNumber, QObject *parent)
{
   if (id < 0) {
      return 0;
   }

   Q_ASSERT(ctxt);
   QDeclarativeContextData *ctxtdata = QDeclarativeContextData::get(ctxt);

   QDeclarativeEnginePrivate *engine = QDeclarativeEnginePrivate::get(ctxtdata->engine);
   QDeclarativeCompiledData *cdata = 0;
   QDeclarativeTypeData *typeData = 0;
   if (!ctxtdata->url.isEmpty()) {
      typeData = engine->typeLoader.get(ctxtdata->url);
      cdata = typeData->compiledData();
   }
   QDeclarativeBinding *rv = cdata ? new QDeclarativeBinding((void *)cdata->datas.at(id).constData(), cdata, obj, ctxtdata,
                             url, lineNumber, parent) : 0;
   if (cdata) {
      cdata->release();
   }
   if (typeData) {
      typeData->release();
   }
   return rv;
}

QDeclarativeBinding::QDeclarativeBinding(const QString &str, QObject *obj, QDeclarativeContext *ctxt,
      QObject *parent)
   : QDeclarativeExpression(QDeclarativeContextData::get(ctxt), obj, str, *new QDeclarativeBindingPrivate)
{
   setParent(parent);
   setNotifyOnValueChanged(true);
}

QDeclarativeBinding::QDeclarativeBinding(const QString &str, QObject *obj, QDeclarativeContextData *ctxt,
      QObject *parent)
   : QDeclarativeExpression(ctxt, obj, str, *new QDeclarativeBindingPrivate)
{
   setParent(parent);
   setNotifyOnValueChanged(true);
}

QDeclarativeBinding::QDeclarativeBinding(const QScriptValue &func, QObject *obj, QDeclarativeContextData *ctxt,
      QObject *parent)
   : QDeclarativeExpression(ctxt, obj, func, *new QDeclarativeBindingPrivate)
{
   setParent(parent);
   setNotifyOnValueChanged(true);
}

QDeclarativeBinding::~QDeclarativeBinding()
{
}

void QDeclarativeBinding::setTarget(const QDeclarativeProperty &prop)
{
   Q_D(QDeclarativeBinding);
   d->property = prop;

   update();
}

QDeclarativeProperty QDeclarativeBinding::property() const
{
   Q_D(const QDeclarativeBinding);
   return d->property;
}

void QDeclarativeBinding::setEvaluateFlags(EvaluateFlags flags)
{
   Q_D(QDeclarativeBinding);
   d->setEvaluateFlags(QDeclarativeQtScriptExpression::EvaluateFlags(static_cast<int>(flags)));
}

QDeclarativeBinding::EvaluateFlags QDeclarativeBinding::evaluateFlags() const
{
   Q_D(const QDeclarativeBinding);
   return QDeclarativeBinding::EvaluateFlags(static_cast<int>(d->evaluateFlags()));
}


class QDeclarativeBindingProfiler
{
 public:
   QDeclarativeBindingProfiler(QDeclarativeBinding *bind) {
      QDeclarativeDebugTrace::startRange(QDeclarativeDebugTrace::Binding);
      QDeclarativeDebugTrace::rangeData(QDeclarativeDebugTrace::Binding, bind->expression());
      QDeclarativeDebugTrace::rangeLocation(QDeclarativeDebugTrace::Binding, bind->sourceFile(), bind->lineNumber());
   }

   ~QDeclarativeBindingProfiler() {
      QDeclarativeDebugTrace::endRange(QDeclarativeDebugTrace::Binding);
   }
};

void QDeclarativeBinding::update(QDeclarativePropertyPrivate::WriteFlags flags)
{
   Q_D(QDeclarativeBinding);

   if (!d->enabled || !d->context() || !d->context()->isValid()) {
      return;
   }

   if (!d->updating) {
      QDeclarativeBindingProfiler prof(this);
      d->updating = true;
      bool wasDeleted = false;
      d->deleted = &wasDeleted;

      if (d->property.propertyType() == qMetaTypeId<QDeclarativeBinding *>()) {

         int idx = d->property.index();
         Q_ASSERT(idx != -1);

         QDeclarativeBinding *t = this;
         int status = -1;
         void *a[] = { &t, 0, &status, &flags };
         QMetaObject::metacall(d->property.object(),
                               QMetaObject::WriteProperty,
                               idx, a);

         if (wasDeleted) {
            return;
         }

      } else {
         QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(d->context()->engine);

         bool isUndefined = false;
         QVariant value;

         QScriptValue scriptValue = d->scriptValue(0, &isUndefined);
         if (wasDeleted) {
            return;
         }

         if (d->property.propertyTypeCategory() == QDeclarativeProperty::List) {
            value = ep->scriptValueToVariant(scriptValue, qMetaTypeId<QList<QObject *> >());
         } else if (scriptValue.isNull() &&
                    d->property.propertyTypeCategory() == QDeclarativeProperty::Object) {
            value = QVariant::fromValue((QObject *)0);
         } else {
            value = ep->scriptValueToVariant(scriptValue, d->property.propertyType());
            if (value.userType() == QMetaType::QObjectStar && !qvariant_cast<QObject *>(value)) {
               // If the object is null, we extract the predicted type.  While this isn't
               // 100% reliable, in many cases it gives us better error messages if we
               // assign this null-object to an incompatible property
               int type = ep->objectClass->objectType(scriptValue);
               QObject *o = 0;
               value = QVariant(type, (void *)&o);
            }
         }


         if (d->error.isValid()) {

         } else if (isUndefined && d->property.isResettable()) {

            d->property.reset();

         } else if (isUndefined && d->property.propertyType() == qMetaTypeId<QVariant>()) {

            QDeclarativePropertyPrivate::write(d->property, QVariant(), flags);

         } else if (isUndefined) {

            QUrl url = QUrl(d->url);
            int line = d->line;
            if (url.isEmpty()) {
               url = QUrl(QLatin1String("<Unknown File>"));
            }

            d->error.setUrl(url);
            d->error.setLine(line);
            d->error.setColumn(-1);
            d->error.setDescription(QLatin1String("Unable to assign [undefined] to ") +
                                    QLatin1String(QMetaType::typeName(d->property.propertyType())) +
                                    QLatin1String(" ") + d->property.name());

         } else if (!scriptValue.isRegExp() && scriptValue.isFunction()) {

            QUrl url = QUrl(d->url);
            int line = d->line;
            if (url.isEmpty()) {
               url = QUrl(QLatin1String("<Unknown File>"));
            }

            d->error.setUrl(url);
            d->error.setLine(line);
            d->error.setColumn(-1);
            d->error.setDescription(QLatin1String("Unable to assign a function to a property."));

         } else if (d->property.object() &&
                    !QDeclarativePropertyPrivate::write(d->property, value, flags)) {

            if (wasDeleted) {
               return;
            }

            QUrl url = QUrl(d->url);
            int line = d->line;
            if (url.isEmpty()) {
               url = QUrl(QLatin1String("<Unknown File>"));
            }

            const char *valueType = 0;
            if (value.userType() == QVariant::Invalid) {
               valueType = "null";
            } else {
               valueType = QMetaType::typeName(value.userType());
            }

            d->error.setUrl(url);
            d->error.setLine(line);
            d->error.setColumn(-1);
            d->error.setDescription(QLatin1String("Unable to assign ") +
                                    QLatin1String(valueType) +
                                    QLatin1String(" to ") +
                                    QLatin1String(QMetaType::typeName(d->property.propertyType())));
         }

         if (wasDeleted) {
            return;
         }

         if (d->error.isValid()) {
            if (!d->addError(ep)) {
               ep->warning(this->error());
            }
         } else {
            d->removeError();
         }
      }

      d->updating = false;
      d->deleted = 0;
   } else {
      qmlInfo(d->property.object()) << tr("Binding loop detected for property \"%1\"").arg(d->property.name());
   }
}

void QDeclarativeBindingPrivate::emitValueChanged()
{
   Q_Q(QDeclarativeBinding);
   q->update();
}

void QDeclarativeBinding::setEnabled(bool e, QDeclarativePropertyPrivate::WriteFlags flags)
{
   Q_D(QDeclarativeBinding);
   d->enabled = e;
   setNotifyOnValueChanged(e);

   if (e) {
      update(flags);
   }
}

bool QDeclarativeBinding::enabled() const
{
   Q_D(const QDeclarativeBinding);

   return d->enabled;
}

QString QDeclarativeBinding::expression() const
{
   return QDeclarativeExpression::expression();
}

void QDeclarativeBinding::disconnect(DisconnectMode disconnectMode)
{
   Q_UNUSED(disconnectMode);
   setNotifyOnValueChanged(false);
}

QDeclarativeValueTypeProxyBinding::QDeclarativeValueTypeProxyBinding(QObject *o, int index)
   : m_object(o), m_index(index), m_bindings(0)
{
}

QDeclarativeValueTypeProxyBinding::~QDeclarativeValueTypeProxyBinding()
{
   while (m_bindings) {
      QDeclarativeAbstractBinding *binding = m_bindings;
      binding->setEnabled(false, 0);
      binding->destroy();
   }
}

void QDeclarativeValueTypeProxyBinding::setEnabled(bool e, QDeclarativePropertyPrivate::WriteFlags flags)
{
   if (e) {
      QDeclarativeAbstractBinding *bindings = m_bindings;
      recursiveEnable(bindings, flags);
   } else {
      QDeclarativeAbstractBinding *bindings = m_bindings;
      recursiveDisable(bindings);
   }
}

void QDeclarativeValueTypeProxyBinding::recursiveEnable(QDeclarativeAbstractBinding *b,
      QDeclarativePropertyPrivate::WriteFlags flags)
{
   if (!b) {
      return;
   }

   recursiveEnable(b->m_nextBinding, flags);

   if (b) {
      b->setEnabled(true, flags);
   }
}

void QDeclarativeValueTypeProxyBinding::recursiveDisable(QDeclarativeAbstractBinding *b)
{
   if (!b) {
      return;
   }

   recursiveDisable(b->m_nextBinding);

   if (b) {
      b->setEnabled(false, 0);
   }
}

void QDeclarativeValueTypeProxyBinding::update(QDeclarativePropertyPrivate::WriteFlags)
{
}

void QDeclarativeValueTypeProxyBinding::disconnect(DisconnectMode disconnectMode)
{
   Q_UNUSED(disconnectMode);
   // Nothing to do
}

QDeclarativeAbstractBinding *QDeclarativeValueTypeProxyBinding::binding(int propertyIndex)
{
   QDeclarativeAbstractBinding *binding = m_bindings;

   while (binding && binding->propertyIndex() != propertyIndex) {
      binding = binding->m_nextBinding;
   }

   return binding;
}

/*!
Removes a collection of bindings, corresponding to the set bits in \a mask.
*/
void QDeclarativeValueTypeProxyBinding::removeBindings(quint32 mask)
{
   QDeclarativeAbstractBinding *binding = m_bindings;
   while (binding) {
      if (mask & (1 << (binding->propertyIndex() >> 24))) {
         QDeclarativeAbstractBinding *remove = binding;
         binding = remove->m_nextBinding;
         *remove->m_prevBinding = remove->m_nextBinding;
         if (remove->m_nextBinding) {
            remove->m_nextBinding->m_prevBinding = remove->m_prevBinding;
         }
         remove->m_prevBinding = 0;
         remove->m_nextBinding = 0;
         remove->destroy();
      } else {
         binding = binding->m_nextBinding;
      }
   }
}

QT_END_NAMESPACE
