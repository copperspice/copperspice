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

#include <qdeclarativevmemetaobject_p.h>
#include <qdeclarative.h>
#include <qdeclarativerefcount_p.h>
#include <qdeclarativeexpression.h>
#include <qdeclarativeexpression_p.h>
#include <qdeclarativecontext_p.h>
#include <qdeclarativebinding_p.h>

Q_DECLARE_METATYPE(QScriptValue);

QT_BEGIN_NAMESPACE

class QDeclarativeVMEVariant
{
 public:
   inline QDeclarativeVMEVariant();
   inline ~QDeclarativeVMEVariant();

   inline const void *dataPtr() const;
   inline void *dataPtr();
   inline int dataType() const;

   inline QObject *asQObject();
   inline const QVariant &asQVariant();
   inline int asInt();
   inline bool asBool();
   inline double asDouble();
   inline const QString &asQString();
   inline const QUrl &asQUrl();
   inline const QColor &asQColor();
   inline const QTime &asQTime();
   inline const QDate &asQDate();
   inline const QDateTime &asQDateTime();
   inline const QScriptValue &asQScriptValue();

   inline void setValue(QObject *);
   inline void setValue(const QVariant &);
   inline void setValue(int);
   inline void setValue(bool);
   inline void setValue(double);
   inline void setValue(const QString &);
   inline void setValue(const QUrl &);
   inline void setValue(const QColor &);
   inline void setValue(const QTime &);
   inline void setValue(const QDate &);
   inline void setValue(const QDateTime &);
   inline void setValue(const QScriptValue &);
 private:
   int type;
   void *data[4]; // Large enough to hold all types

   inline void cleanup();
};

QDeclarativeVMEVariant::QDeclarativeVMEVariant()
   : type(QVariant::Invalid)
{
}

QDeclarativeVMEVariant::~QDeclarativeVMEVariant()
{
   cleanup();
}

void QDeclarativeVMEVariant::cleanup()
{
   if (type == QVariant::Invalid) {
   } else if (type == QMetaType::Int ||
              type == QMetaType::Bool ||
              type == QMetaType::Double) {
      type = QVariant::Invalid;
   } else if (type == QMetaType::QObjectStar) {
      ((QDeclarativeGuard<QObject> *)dataPtr())->~QDeclarativeGuard<QObject>();
      type = QVariant::Invalid;
   } else if (type == QMetaType::QString) {
      ((QString *)dataPtr())->~QString();
      type = QVariant::Invalid;
   } else if (type == QMetaType::QUrl) {
      ((QUrl *)dataPtr())->~QUrl();
      type = QVariant::Invalid;
   } else if (type == QMetaType::QColor) {
      ((QColor *)dataPtr())->~QColor();
      type = QVariant::Invalid;
   } else if (type == QMetaType::QTime) {
      ((QTime *)dataPtr())->~QTime();
      type = QVariant::Invalid;
   } else if (type == QMetaType::QDate) {
      ((QDate *)dataPtr())->~QDate();
      type = QVariant::Invalid;
   } else if (type == QMetaType::QDateTime) {
      ((QDateTime *)dataPtr())->~QDateTime();
      type = QVariant::Invalid;
   } else if (type == qMetaTypeId<QVariant>()) {
      ((QVariant *)dataPtr())->~QVariant();
      type = QVariant::Invalid;
   } else if (type == qMetaTypeId<QScriptValue>()) {
      ((QScriptValue *)dataPtr())->~QScriptValue();
      type = QVariant::Invalid;
   }

}

int QDeclarativeVMEVariant::dataType() const
{
   return type;
}

const void *QDeclarativeVMEVariant::dataPtr() const
{
   return &data;
}

void *QDeclarativeVMEVariant::dataPtr()
{
   return &data;
}

QObject *QDeclarativeVMEVariant::asQObject()
{
   if (type != QMetaType::QObjectStar) {
      setValue((QObject *)0);
   }

   return *(QDeclarativeGuard<QObject> *)(dataPtr());
}

const QVariant &QDeclarativeVMEVariant::asQVariant()
{
   if (type != QMetaType::QVariant) {
      setValue(QVariant());
   }

   return *(QVariant *)(dataPtr());
}

int QDeclarativeVMEVariant::asInt()
{
   if (type != QMetaType::Int) {
      setValue(int(0));
   }

   return *(int *)(dataPtr());
}

bool QDeclarativeVMEVariant::asBool()
{
   if (type != QMetaType::Bool) {
      setValue(bool(false));
   }

   return *(bool *)(dataPtr());
}

double QDeclarativeVMEVariant::asDouble()
{
   if (type != QMetaType::Double) {
      setValue(double(0));
   }

   return *(double *)(dataPtr());
}

const QString &QDeclarativeVMEVariant::asQString()
{
   if (type != QMetaType::QString) {
      setValue(QString());
   }

   return *(QString *)(dataPtr());
}

const QUrl &QDeclarativeVMEVariant::asQUrl()
{
   if (type != QMetaType::QUrl) {
      setValue(QUrl());
   }

   return *(QUrl *)(dataPtr());
}

const QColor &QDeclarativeVMEVariant::asQColor()
{
   if (type != QMetaType::QColor) {
      setValue(QColor());
   }

   return *(QColor *)(dataPtr());
}

const QTime &QDeclarativeVMEVariant::asQTime()
{
   if (type != QMetaType::QTime) {
      setValue(QTime());
   }

   return *(QTime *)(dataPtr());
}

const QDate &QDeclarativeVMEVariant::asQDate()
{
   if (type != QMetaType::QDate) {
      setValue(QDate());
   }

   return *(QDate *)(dataPtr());
}

const QDateTime &QDeclarativeVMEVariant::asQDateTime()
{
   if (type != QMetaType::QDateTime) {
      setValue(QDateTime());
   }

   return *(QDateTime *)(dataPtr());
}

const QScriptValue &QDeclarativeVMEVariant::asQScriptValue()
{
   if (type != qMetaTypeId<QScriptValue>()) {
      setValue(QScriptValue());
   }

   return *(QScriptValue *)(dataPtr());
}

void QDeclarativeVMEVariant::setValue(QObject *v)
{
   if (type != QMetaType::QObjectStar) {
      cleanup();
      type = QMetaType::QObjectStar;
      new (dataPtr()) QDeclarativeGuard<QObject>();
   }
   *(QDeclarativeGuard<QObject> *)(dataPtr()) = v;
}

void QDeclarativeVMEVariant::setValue(const QVariant &v)
{
   if (type != qMetaTypeId<QVariant>()) {
      cleanup();
      type = qMetaTypeId<QVariant>();
      new (dataPtr()) QVariant(v);
   } else {
      *(QVariant *)(dataPtr()) = v;
   }
}

void QDeclarativeVMEVariant::setValue(int v)
{
   if (type != QMetaType::Int) {
      cleanup();
      type = QMetaType::Int;
   }
   *(int *)(dataPtr()) = v;
}

void QDeclarativeVMEVariant::setValue(bool v)
{
   if (type != QMetaType::Bool) {
      cleanup();
      type = QMetaType::Bool;
   }
   *(bool *)(dataPtr()) = v;
}

void QDeclarativeVMEVariant::setValue(double v)
{
   if (type != QMetaType::Double) {
      cleanup();
      type = QMetaType::Double;
   }
   *(double *)(dataPtr()) = v;
}

void QDeclarativeVMEVariant::setValue(const QString &v)
{
   if (type != QMetaType::QString) {
      cleanup();
      type = QMetaType::QString;
      new (dataPtr()) QString(v);
   } else {
      *(QString *)(dataPtr()) = v;
   }
}

void QDeclarativeVMEVariant::setValue(const QUrl &v)
{
   if (type != QMetaType::QUrl) {
      cleanup();
      type = QMetaType::QUrl;
      new (dataPtr()) QUrl(v);
   } else {
      *(QUrl *)(dataPtr()) = v;
   }
}

void QDeclarativeVMEVariant::setValue(const QColor &v)
{
   if (type != QMetaType::QColor) {
      cleanup();
      type = QMetaType::QColor;
      new (dataPtr()) QColor(v);
   } else {
      *(QColor *)(dataPtr()) = v;
   }
}

void QDeclarativeVMEVariant::setValue(const QTime &v)
{
   if (type != QMetaType::QTime) {
      cleanup();
      type = QMetaType::QTime;
      new (dataPtr()) QTime(v);
   } else {
      *(QTime *)(dataPtr()) = v;
   }
}

void QDeclarativeVMEVariant::setValue(const QDate &v)
{
   if (type != QMetaType::QDate) {
      cleanup();
      type = QMetaType::QDate;
      new (dataPtr()) QDate(v);
   } else {
      *(QDate *)(dataPtr()) = v;
   }
}

void QDeclarativeVMEVariant::setValue(const QDateTime &v)
{
   if (type != QMetaType::QDateTime) {
      cleanup();
      type = QMetaType::QDateTime;
      new (dataPtr()) QDateTime(v);
   } else {
      *(QDateTime *)(dataPtr()) = v;
   }
}

void QDeclarativeVMEVariant::setValue(const QScriptValue &v)
{
   if (type != qMetaTypeId<QScriptValue>()) {
      cleanup();
      type = qMetaTypeId<QScriptValue>();
      new (dataPtr()) QScriptValue(v);
   } else {
      *(QScriptValue *)(dataPtr()) = v;
   }
}

QDeclarativeVMEMetaObject::QDeclarativeVMEMetaObject(QObject *obj,
      const QMetaObject *other,
      const QDeclarativeVMEMetaData *meta,
      QDeclarativeCompiledData *cdata)
   : object(obj), compiledData(cdata), ctxt(QDeclarativeData::get(obj, true)->outerContext),
     metaData(meta), data(0), methods(0), parent(0)
{
   compiledData->addref();

   *static_cast<QMetaObject *>(this) = *other;
   this->d.superdata = obj->metaObject();

   QObjectPrivate *op = QObjectPrivate::get(obj);
   if (op->metaObject) {
      parent = static_cast<QAbstractDynamicMetaObject *>(op->metaObject);
   }
   op->metaObject = this;

   propOffset = QAbstractDynamicMetaObject::propertyOffset();
   methodOffset = QAbstractDynamicMetaObject::methodOffset();

   data = new QDeclarativeVMEVariant[metaData->propertyCount];

   aConnected.resize(metaData->aliasCount);
   int list_type = qMetaTypeId<QDeclarativeListProperty<QObject> >();

   // ### Optimize
   for (int ii = 0; ii < metaData->propertyCount; ++ii) {
      int t = (metaData->propertyData() + ii)->propertyType;
      if (t == list_type) {
         listProperties.append(List(methodOffset + ii));
         data[ii].setValue(listProperties.count() - 1);
      }
   }
}

QDeclarativeVMEMetaObject::~QDeclarativeVMEMetaObject()
{
   compiledData->release();
   delete parent;
   delete [] data;
   delete [] methods;
}

int QDeclarativeVMEMetaObject::metaCall(QMetaObject::Call c, int _id, void **a)
{
   int id = _id;
   if (c == QMetaObject::WriteProperty) {
      int flags = *reinterpret_cast<int *>(a[3]);
      if (!(flags & QDeclarativePropertyPrivate::BypassInterceptor)
            && !aInterceptors.isEmpty()
            && aInterceptors.testBit(id)) {
         QPair<int, QDeclarativePropertyValueInterceptor *> pair = interceptors.value(id);
         int valueIndex = pair.first;
         QDeclarativePropertyValueInterceptor *vi = pair.second;
         int type = property(id).userType();

         if (type != QVariant::Invalid) {
            if (valueIndex != -1) {
               QDeclarativeEnginePrivate *ep = ctxt ? QDeclarativeEnginePrivate::get(ctxt->engine) : 0;
               QDeclarativeValueType *valueType = 0;
               if (ep) {
                  valueType = ep->valueTypes[type];
               } else {
                  valueType = QDeclarativeValueTypeFactory::valueType(type);
               }
               Q_ASSERT(valueType);

               valueType->setValue(QVariant(type, a[0]));
               QMetaProperty valueProp = valueType->metaObject()->property(valueIndex);
               vi->write(valueProp.read(valueType));

               if (!ep) {
                  delete valueType;
               }
               return -1;
            } else {
               vi->write(QVariant(type, a[0]));
               return -1;
            }
         }
      }
   }
   if (c == QMetaObject::ReadProperty || c == QMetaObject::WriteProperty) {
      if (id >= propOffset) {
         id -= propOffset;

         if (id < metaData->propertyCount) {
            int t = (metaData->propertyData() + id)->propertyType;
            bool needActivate = false;

            if (t == -1) {

               if (c == QMetaObject::ReadProperty) {
                  *reinterpret_cast<QVariant *>(a[0]) = readVarPropertyAsVariant(id);
               } else if (c == QMetaObject::WriteProperty) {
                  writeVarProperty(id, *reinterpret_cast<QVariant *>(a[0]));
               }

            } else {

               if (c == QMetaObject::ReadProperty) {
                  switch (t) {
                     case QVariant::Int:
                        *reinterpret_cast<int *>(a[0]) = data[id].asInt();
                        break;
                     case QVariant::Bool:
                        *reinterpret_cast<bool *>(a[0]) = data[id].asBool();
                        break;
                     case QVariant::Double:
                        *reinterpret_cast<double *>(a[0]) = data[id].asDouble();
                        break;
                     case QVariant::String:
                        *reinterpret_cast<QString *>(a[0]) = data[id].asQString();
                        break;
                     case QVariant::Url:
                        *reinterpret_cast<QUrl *>(a[0]) = data[id].asQUrl();
                        break;
                     case QVariant::Color:
                        *reinterpret_cast<QColor *>(a[0]) = data[id].asQColor();
                        break;
                     case QVariant::Date:
                        *reinterpret_cast<QDate *>(a[0]) = data[id].asQDate();
                        break;
                     case QVariant::DateTime:
                        *reinterpret_cast<QDateTime *>(a[0]) = data[id].asQDateTime();
                        break;
                     case QMetaType::QObjectStar:
                        *reinterpret_cast<QObject **>(a[0]) = data[id].asQObject();
                        break;
                     default:
                        break;
                  }
                  if (t == qMetaTypeId<QDeclarativeListProperty<QObject> >()) {
                     int listIndex = data[id].asInt();
                     const List *list = &listProperties.at(listIndex);
                     *reinterpret_cast<QDeclarativeListProperty<QObject> *>(a[0]) =
                        QDeclarativeListProperty<QObject>(object, (void *)list,
                                                          list_append, list_count, list_at,
                                                          list_clear);
                  }

               } else if (c == QMetaObject::WriteProperty) {

                  switch (t) {
                     case QVariant::Int:
                        needActivate = *reinterpret_cast<int *>(a[0]) != data[id].asInt();
                        data[id].setValue(*reinterpret_cast<int *>(a[0]));
                        break;
                     case QVariant::Bool:
                        needActivate = *reinterpret_cast<bool *>(a[0]) != data[id].asBool();
                        data[id].setValue(*reinterpret_cast<bool *>(a[0]));
                        break;
                     case QVariant::Double:
                        needActivate = *reinterpret_cast<double *>(a[0]) != data[id].asDouble();
                        data[id].setValue(*reinterpret_cast<double *>(a[0]));
                        break;
                     case QVariant::String:
                        needActivate = *reinterpret_cast<QString *>(a[0]) != data[id].asQString();
                        data[id].setValue(*reinterpret_cast<QString *>(a[0]));
                        break;
                     case QVariant::Url:
                        needActivate = *reinterpret_cast<QUrl *>(a[0]) != data[id].asQUrl();
                        data[id].setValue(*reinterpret_cast<QUrl *>(a[0]));
                        break;
                     case QVariant::Color:
                        needActivate = *reinterpret_cast<QColor *>(a[0]) != data[id].asQColor();
                        data[id].setValue(*reinterpret_cast<QColor *>(a[0]));
                        break;
                     case QVariant::Date:
                        needActivate = *reinterpret_cast<QDate *>(a[0]) != data[id].asQDate();
                        data[id].setValue(*reinterpret_cast<QDate *>(a[0]));
                        break;
                     case QVariant::DateTime:
                        needActivate = *reinterpret_cast<QDateTime *>(a[0]) != data[id].asQDateTime();
                        data[id].setValue(*reinterpret_cast<QDateTime *>(a[0]));
                        break;
                     case QMetaType::QObjectStar:
                        needActivate = *reinterpret_cast<QObject **>(a[0]) != data[id].asQObject();
                        data[id].setValue(*reinterpret_cast<QObject **>(a[0]));
                        break;
                     default:
                        break;
                  }
               }

            }

            if (c == QMetaObject::WriteProperty && needActivate) {
               activate(object, methodOffset + id, 0);
            }

            return -1;
         }

         id -= metaData->propertyCount;

         if (id < metaData->aliasCount) {

            QDeclarativeVMEMetaData::AliasData *d = metaData->aliasData() + id;

            if (d->flags & QML_ALIAS_FLAG_PTR && c == QMetaObject::ReadProperty) {
               *reinterpret_cast<void **>(a[0]) = 0;
            }

            if (!ctxt) {
               return -1;
            }

            QDeclarativeContext *context = ctxt->asQDeclarativeContext();
            QDeclarativeContextPrivate *ctxtPriv = QDeclarativeContextPrivate::get(context);

            QObject *target = ctxtPriv->data->idValues[d->contextIdx].data();
            if (!target) {
               return -1;
            }

            connectAlias(id);

            if (d->isObjectAlias()) {
               *reinterpret_cast<QObject **>(a[0]) = target;
               return -1;
            }

            // Remove binding (if any) on write
            if (c == QMetaObject::WriteProperty) {
               int flags = *reinterpret_cast<int *>(a[3]);
               if (flags & QDeclarativePropertyPrivate::RemoveBindingOnAliasWrite) {
                  QDeclarativeData *targetData = QDeclarativeData::get(target);
                  if (targetData && targetData->hasBindingBit(d->propertyIndex())) {
                     QDeclarativeAbstractBinding *binding = QDeclarativePropertyPrivate::setBinding(target, d->propertyIndex(),
                                                            d->isValueTypeAlias() ? d->valueTypeIndex() : -1, 0);
                     if (binding) {
                        binding->destroy();
                     }
                  }
               }
            }

            if (d->isValueTypeAlias()) {
               // Value type property
               QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(ctxt->engine);

               QDeclarativeValueType *valueType = ep->valueTypes[d->valueType()];
               Q_ASSERT(valueType);

               valueType->read(target, d->propertyIndex());
               int rv = QMetaObject::metacall(valueType, c, d->valueTypeIndex(), a);

               if (c == QMetaObject::WriteProperty) {
                  valueType->write(target, d->propertyIndex(), 0x00);
               }

               return rv;

            } else {
               return QMetaObject::metacall(target, c, d->propertyIndex(), a);
            }

         }
         return -1;

      }

   } else if (c == QMetaObject::InvokeMetaMethod) {

      if (id >= methodOffset) {

         id -= methodOffset;
         int plainSignals = metaData->signalCount + metaData->propertyCount +
                            metaData->aliasCount;
         if (id < plainSignals) {
            QMetaObject::activate(object, _id, a);
            return -1;
         }

         id -= plainSignals;

         if (id < metaData->methodCount) {
            if (!ctxt->engine) {
               return -1;   // We can't run the method
            }

            QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(ctxt->engine);

            QScriptValue function = method(id);

            QScriptValueList args;
            QDeclarativeVMEMetaData::MethodData *data = metaData->methodData() + id;
            if (data->parameterCount) {
               for (int ii = 0; ii < data->parameterCount; ++ii) {
                  args << ep->scriptValueFromVariant(*(QVariant *)a[ii + 1]);
               }
            }
            QScriptValue rv = function.call(ep->objectClass->newQObject(object), args);

            if (a[0]) {
               *reinterpret_cast<QVariant *>(a[0]) = ep->scriptValueToVariant(rv);
            }

            return -1;
         }
         return -1;
      }
   }

   if (parent) {
      return parent->metaCall(c, _id, a);
   } else {
      return object->qt_metacall(c, _id, a);
   }
}

QScriptValue QDeclarativeVMEMetaObject::method(int index)
{
   if (!methods) {
      methods = new QScriptValue[metaData->methodCount];
   }

   if (!methods[index].isValid()) {
      QDeclarativeVMEMetaData::MethodData *data = metaData->methodData() + index;

      const QChar *body =
         (const QChar *)(((const char *)metaData) + data->bodyOffset);

      QString code = QString::fromRawData(body, data->bodyLength);

      // XXX Use QScriptProgram
      // XXX We should evaluate all methods in a single big script block to
      // improve the call time between dynamic methods defined on the same
      // object
      methods[index] = QDeclarativeExpressionPrivate::evalInObjectScope(ctxt, object, code, ctxt->url.toString(),
                       data->lineNumber, 0);
   }

   return methods[index];
}

QScriptValue QDeclarativeVMEMetaObject::readVarProperty(int id)
{
   if (data[id].dataType() == qMetaTypeId<QScriptValue>()) {
      return data[id].asQScriptValue();
   } else if (data[id].dataType() == QMetaType::QObjectStar) {
      return QDeclarativeEnginePrivate::get(ctxt->engine)->objectClass->newQObject(data[id].asQObject());
   } else {
      return QDeclarativeEnginePrivate::get(ctxt->engine)->scriptValueFromVariant(data[id].asQVariant());
   }
}

QVariant QDeclarativeVMEMetaObject::readVarPropertyAsVariant(int id)
{
   if (data[id].dataType() == qMetaTypeId<QScriptValue>()) {
      return QDeclarativeEnginePrivate::get(ctxt->engine)->scriptValueToVariant(data[id].asQScriptValue());
   } else if (data[id].dataType() == QMetaType::QObjectStar) {
      return QVariant::fromValue(data[id].asQObject());
   } else {
      return data[id].asQVariant();
   }
}

void QDeclarativeVMEMetaObject::writeVarProperty(int id, const QScriptValue &value)
{
   data[id].setValue(value);
   activate(object, methodOffset + id, 0);
}

void QDeclarativeVMEMetaObject::writeVarProperty(int id, const QVariant &value)
{
   bool needActivate = false;
   if (value.userType() == QMetaType::QObjectStar) {
      QObject *o = qvariant_cast<QObject *>(value);
      needActivate = (data[id].dataType() != QMetaType::QObjectStar || data[id].asQObject() != o);
      data[id].setValue(qvariant_cast<QObject *>(value));
   } else {
      needActivate = (data[id].dataType() != qMetaTypeId<QVariant>() ||
                      data[id].asQVariant().userType() != value.userType() ||
                      data[id].asQVariant() != value);
      data[id].setValue(value);
   }
   if (needActivate) {
      activate(object, methodOffset + id, 0);
   }
}

void QDeclarativeVMEMetaObject::listChanged(int id)
{
   activate(object, methodOffset + id, 0);
}

void QDeclarativeVMEMetaObject::list_append(QDeclarativeListProperty<QObject> *prop, QObject *o)
{
   List *list = static_cast<List *>(prop->data);
   list->append(o);
   QMetaObject::activate(prop->object, list->notifyIndex, 0);
}

int QDeclarativeVMEMetaObject::list_count(QDeclarativeListProperty<QObject> *prop)
{
   return static_cast<List *>(prop->data)->count();
}

QObject *QDeclarativeVMEMetaObject::list_at(QDeclarativeListProperty<QObject> *prop, int index)
{
   return static_cast<List *>(prop->data)->at(index);
}

void QDeclarativeVMEMetaObject::list_clear(QDeclarativeListProperty<QObject> *prop)
{
   List *list = static_cast<List *>(prop->data);
   list->clear();
   QMetaObject::activate(prop->object, list->notifyIndex, 0);
}

void QDeclarativeVMEMetaObject::registerInterceptor(int index, int valueIndex,
      QDeclarativePropertyValueInterceptor *interceptor)
{
   if (aInterceptors.isEmpty()) {
      aInterceptors.resize(propertyCount() + metaData->propertyCount);
   }
   aInterceptors.setBit(index);
   interceptors.insert(index, qMakePair(valueIndex, interceptor));
}

int QDeclarativeVMEMetaObject::vmeMethodLineNumber(int index)
{
   if (index < methodOffset) {
      Q_ASSERT(parent);
      return static_cast<QDeclarativeVMEMetaObject *>(parent)->vmeMethodLineNumber(index);
   }

   int plainSignals = metaData->signalCount + metaData->propertyCount + metaData->aliasCount;
   Q_ASSERT(index >= (methodOffset + plainSignals) && index < (methodOffset + plainSignals + metaData->methodCount));

   int rawIndex = index - methodOffset - plainSignals;

   QDeclarativeVMEMetaData::MethodData *data = metaData->methodData() + rawIndex;
   return data->lineNumber;
}

QScriptValue QDeclarativeVMEMetaObject::vmeMethod(int index)
{
   if (index < methodOffset) {
      Q_ASSERT(parent);
      return static_cast<QDeclarativeVMEMetaObject *>(parent)->vmeMethod(index);
   }
   int plainSignals = metaData->signalCount + metaData->propertyCount + metaData->aliasCount;
   Q_ASSERT(index >= (methodOffset + plainSignals) && index < (methodOffset + plainSignals + metaData->methodCount));
   return method(index - methodOffset - plainSignals);
}

void QDeclarativeVMEMetaObject::setVmeMethod(int index, const QScriptValue &value)
{
   if (index < methodOffset) {
      Q_ASSERT(parent);
      return static_cast<QDeclarativeVMEMetaObject *>(parent)->setVmeMethod(index, value);
   }
   int plainSignals = metaData->signalCount + metaData->propertyCount + metaData->aliasCount;
   Q_ASSERT(index >= (methodOffset + plainSignals) && index < (methodOffset + plainSignals + metaData->methodCount));

   if (!methods) {
      methods = new QScriptValue[metaData->methodCount];
   }
   methods[index - methodOffset - plainSignals] = value;
}

QScriptValue QDeclarativeVMEMetaObject::vmeProperty(int index)
{
   if (index < propOffset) {
      Q_ASSERT(parent);
      return static_cast<QDeclarativeVMEMetaObject *>(parent)->vmeProperty(index);
   }
   return readVarProperty(index - propOffset);
}

void QDeclarativeVMEMetaObject::setVMEProperty(int index, const QScriptValue &v)
{
   if (index < propOffset) {
      Q_ASSERT(parent);
      static_cast<QDeclarativeVMEMetaObject *>(parent)->setVMEProperty(index, v);
   }
   return writeVarProperty(index - propOffset, v);
}

bool QDeclarativeVMEMetaObject::aliasTarget(int index, QObject **target, int *coreIndex, int *valueTypeIndex) const
{
   Q_ASSERT(index >= propOffset + metaData->propertyCount);

   *target = 0;
   *coreIndex = -1;
   *valueTypeIndex = -1;

   if (!ctxt) {
      return false;
   }

   QDeclarativeVMEMetaData::AliasData *d = metaData->aliasData() + (index - propOffset - metaData->propertyCount);
   QDeclarativeContext *context = ctxt->asQDeclarativeContext();
   QDeclarativeContextPrivate *ctxtPriv = QDeclarativeContextPrivate::get(context);

   *target = ctxtPriv->data->idValues[d->contextIdx].data();
   if (!*target) {
      return false;
   }

   if (d->isObjectAlias()) {
   } else if (d->isValueTypeAlias()) {
      *coreIndex = d->propertyIndex();
      *valueTypeIndex = d->valueTypeIndex();
   } else {
      *coreIndex = d->propertyIndex();
   }

   return true;
}

void QDeclarativeVMEMetaObject::connectAlias(int aliasId)
{
   if (!aConnected.testBit(aliasId)) {
      aConnected.setBit(aliasId);

      QDeclarativeContext *context = ctxt->asQDeclarativeContext();
      QDeclarativeContextPrivate *ctxtPriv = QDeclarativeContextPrivate::get(context);

      QDeclarativeVMEMetaData::AliasData *d = metaData->aliasData() + aliasId;

      QObject *target = ctxtPriv->data->idValues[d->contextIdx].data();
      if (!target) {
         return;
      }

      int sigIdx = methodOffset + aliasId + metaData->propertyCount;
      QMetaObject::connect(context, d->contextIdx + ctxtPriv->notifyIndex, object, sigIdx);

      if (!d->isObjectAlias()) {
         QMetaProperty prop = target->metaObject()->property(d->propertyIndex());
         if (prop.hasNotifySignal()) {
            QDeclarativePropertyPrivate::connect(target, prop.notifySignalIndex(), object, sigIdx);
         }
      }
   }
}

void QDeclarativeVMEMetaObject::connectAliasSignal(int index)
{
   int aliasId = (index - methodOffset) - metaData->propertyCount;
   if (aliasId < 0 || aliasId >= metaData->aliasCount) {
      return;
   }

   connectAlias(aliasId);
}

QT_END_NAMESPACE
