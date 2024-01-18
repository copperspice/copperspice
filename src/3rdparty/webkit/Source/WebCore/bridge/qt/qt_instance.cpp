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

#include "config.h"
#include "qt_instance.h"

#include "Error.h"
#include "JSDOMBinding.h"
#include "JSGlobalObject.h"
#include "JSLock.h"
#include "ObjectPrototype.h"
#include "PropertyNameArray.h"
#include "qt_class.h"
#include "qt_runtime.h"
#include "runtime_object.h"
#include "runtime/FunctionPrototype.h"

#include <qalgorithms.h>
#include <qdebug.h>
#include <qhash.h>
#include <qmultihash.h>
#include <qmetaobject.h>
#include <qwebelement.h>

namespace JSC {
namespace Bindings {

// Cache QtInstances
typedef QMultiHash<void*, QtInstance*> QObjectInstanceMap;
static QObjectInstanceMap cachedInstances;

// Derived RuntimeObject
class QtRuntimeObject : public RuntimeObject {
public:
    QtRuntimeObject(ExecState*, JSGlobalObject*, PassRefPtr<Instance>);

    static const ClassInfo s_info;

    virtual void visitChildren(SlotVisitor& visitor)
    {
        RuntimeObject::visitChildren(visitor);
        QtInstance* instance = static_cast<QtInstance*>(getInternalInstance());
        if (instance)
            instance->visitAggregate(visitor);
    }

    static Structure* createStructure(JSGlobalData& globalData, JSValue prototype)
    {
        return Structure::create(globalData, prototype, TypeInfo(ObjectType,  StructureFlags), AnonymousSlotCount, &s_info);
    }

protected:
    static const unsigned StructureFlags = RuntimeObject::StructureFlags | OverridesVisitChildren;
};

const ClassInfo QtRuntimeObject::s_info = { "QtRuntimeObject", &RuntimeObject::s_info, 0, 0 };

QtRuntimeObject::QtRuntimeObject(ExecState* exec, JSGlobalObject* globalObject, PassRefPtr<Instance> instance)
    : RuntimeObject(exec, globalObject, WebCore::deprecatedGetDOMStructure<QtRuntimeObject>(exec), instance)
{
}

// QtInstance
QtInstance::QtInstance(QObject* o, PassRefPtr<RootObject> rootObject, QScriptEngine::ValueOwnership ownership)
    : Instance(rootObject), m_class(0), m_object(o), m_hashkey(o), m_ownership(ownership)
{
}

QtInstance::~QtInstance()
{
    JSLock lock(SilenceAssertionsOnly);

    cachedInstances.remove(m_hashkey);

    // clean up (unprotect from gc) the JSValues we've created
    m_methods.clear();

    qDeleteAll(m_fields);
    m_fields.clear();

    if (m_object) {
        switch (m_ownership) {
        case QScriptEngine::QtOwnership:
            break;
        case QScriptEngine::AutoOwnership:
            if (m_object->parent())
                break;
            // fall through!
        case QScriptEngine::ScriptOwnership:
            delete m_object;
            break;
        }
    }
}

PassRefPtr<QtInstance> QtInstance::getQtInstance(QObject* o, PassRefPtr<RootObject> rootObject, QScriptEngine::ValueOwnership ownership)
{
    JSLock lock(SilenceAssertionsOnly);

    for (QtInstance *instance : cachedInstances.values(o))
        if (instance->rootObject() == rootObject) {
            // The garbage collector removes instances, but it may happen that the wrapped
            // QObject dies before the gc kicks in. To handle that case we have to do an additional
            // check if to see if the instance's wrapped object is still alive. If it isn't, then
            // we have to create a new wrapper.
            if (!instance->getObject())
                cachedInstances.remove(instance->hashKey());
            else
                return instance;
        }

    RefPtr<QtInstance> ret = QtInstance::create(o, rootObject, ownership);
    cachedInstances.insert(o, ret.get());

    return ret.release();
}

bool QtInstance::getOwnPropertySlot(JSObject* object, ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return object->JSObject::getOwnPropertySlot(exec, propertyName, slot);
}

void QtInstance::put(JSObject* object, ExecState* exec, const Identifier& propertyName, JSValue value, PutPropertySlot& slot)
{
    object->JSObject::put(exec, propertyName, value, slot);
}

void QtInstance::removeCachedMethod(JSObject* method)
{
    if (m_defaultMethod.get() == method)
        m_defaultMethod.clear();

    for (QHash<QString, WriteBarrier<JSObject> >::iterator it = m_methods.begin(), end = m_methods.end(); it != end; ++it) {
        if (it.value().get() == method) {
            m_methods.erase(it);
            return;
        }
    }
}

QtInstance* QtInstance::getInstance(JSObject* object)
{
    if (!object)
        return 0;

    if (!object->inherits(&QtRuntimeObject::s_info))
        return 0;

    return static_cast<QtInstance*>(static_cast<RuntimeObject*>(object)->getInternalInstance());
}

Class* QtInstance::getClass() const
{
    if (!m_class) {
        if (!m_object)
            return 0;
        m_class = QtClass::classForObject(m_object);
    }
    return m_class;
}

RuntimeObject* QtInstance::newRuntimeObject(ExecState* exec)
{
    JSLock lock(SilenceAssertionsOnly);
    m_methods.clear();
    return new (exec) QtRuntimeObject(exec, exec->lexicalGlobalObject(), this);
}

void QtInstance::visitAggregate(SlotVisitor& visitor)
{
    if (m_defaultMethod)
        visitor.append(&m_defaultMethod);

    for (QHash<QString, WriteBarrier<JSObject> >::iterator it = m_methods.begin(), end = m_methods.end(); it != end; ++it)
        visitor.append(&it.value());
}

void QtInstance::begin()
{
    // Do nothing.
}

void QtInstance::end()
{
    // Do nothing.
}

void QtInstance::getPropertyNames(ExecState *exec, PropertyNameArray &array)
{
    // This is the enumerable properties, so put:
    // properties
    // dynamic properties
    // slots
    QObject* obj = getObject();

    if (obj) {
        const QMetaObject* meta = obj->metaObject();

        int i;
        for (i = 0; i < meta->propertyCount(); i++) {
            QMetaProperty prop = meta->property(i);

            if (prop.isScriptable()) {
                array.add(Identifier(exec, prop.name().constData()));
            }
        }

#ifndef QT_NO_PROPERTIES
        QList<QString> dynProps = obj->dynamicPropertyNames();

        for (const QString &ba : dynProps) {
            array.add(Identifier(exec, ba.constData()));
        }
#endif

        const int methodCount = meta->methodCount();

        for (i = 0; i < methodCount; i++) {
            QMetaMethod method = meta->method(i);

            if (method.access() != QMetaMethod::Private)
                array.add(Identifier(exec, method.methodSignature().constData()));
        }
    }
}

JSValue QtInstance::getMethod(ExecState* exec, const Identifier& propertyName)
{
    if (!getClass())
        return jsNull();
    MethodList methodList = m_class->methodsNamed(propertyName, this);
    return new (exec) RuntimeMethod(exec, exec->lexicalGlobalObject(), WebCore::deprecatedGetDOMStructure<RuntimeMethod>(exec), propertyName, methodList);
}

JSValue QtInstance::invokeMethod(ExecState*, RuntimeMethod*)
{
    // Implemented via fallbackMethod & QtRuntimeMetaMethod::callAsFunction
    return jsUndefined();
}

JSValue QtInstance::defaultValue(ExecState* exec, PreferredPrimitiveType hint) const
{
    if (hint == PreferString)
        return stringValue(exec);
    if (hint == PreferNumber)
        return numberValue(exec);
    return valueOf(exec);
}

JSValue QtInstance::stringValue(ExecState* exec) const
{
    QObject* obj = getObject();

    if (! obj) {
      return jsNull();
    }

    //see if there is a toString defined
    QByteArray buf;

    bool useDefault = true;
    getClass();

    if (m_class) {
      QString ret;

      if ( QMetaObject::invokeMethod(obj, "toString", Q_RETURN_ARG(QString, ret)) ) {
         buf = ret.toLatin1().constData();
         useDefault = false;
      }
    }

    if (useDefault) {
        // there is no "toString" method
        const QMetaObject* metaObj = obj->metaObject();

        QString name = obj->objectName();
        QString str  = QString::fromUtf8("%0(name = \"%1\")").formatArg(metaObj->className()).formatArg(name);

        buf = str.toLatin1();
    }

    return jsString(exec, buf.constData());
}

JSValue QtInstance::numberValue(ExecState*) const
{
    return jsNumber(0);
}

JSValue QtInstance::booleanValue() const
{
    // ECMA 9.2
    return jsBoolean(getObject());
}

JSValue QtInstance::valueOf(ExecState* exec) const
{
    return stringValue(exec);
}

// In qt_runtime.cpp
JSValue convertQVariantToValue(ExecState*, PassRefPtr<RootObject> root, const QVariant& variant);
QVariant convertValueToQVariant(ExecState*, JSValue, QVariant::Type hint, int *distance);

QString QtField::name() const
{
    if (m_type == QtFieldType::MetaProperty) {
        return m_property.name();
    }

    if (m_type == ChildObject && m_childObject) {
        return m_childObject->objectName();
    }


#ifndef QT_NO_PROPERTIES
    if (m_type == DynamicProperty) {
        return m_dynamicProperty;
    }
#endif

    return QString(); // deleted child object
}

JSValue QtField::valueFromInstance(ExecState* exec, const Instance* inst) const
{
    const QtInstance* instance = static_cast<const QtInstance*>(inst);
    QObject* obj = instance->getObject();

    if (obj) {
        QVariant val;

        if (m_type == QtFieldType::MetaProperty) {

            if (m_property.isReadable())
                val = m_property.read(obj);
            else
                return jsUndefined();


        } else if (m_type == ChildObject)
            val = QVariant::fromValue((QObject*) m_childObject);

#ifndef QT_NO_PROPERTIES
        else if (m_type == DynamicProperty)
            val = obj->property(m_dynamicProperty);
#endif
        return convertQVariantToValue(exec, inst->rootObject(), val);
    }

    QString msg = QString("Can not access member `%1' of deleted QObject").formatArg(name());
    return throwError(exec, createError(exec, msg.toLatin1().constData()));
}

void QtField::setValueToInstance(ExecState* exec, const Instance* inst, JSValue aValue) const
{
    if (m_type == ChildObject) // unable to set to a named child
        return;

    const QtInstance* instance = static_cast<const QtInstance*>(inst);
    QObject* obj = instance->getObject();

    if (obj) {
        uint argtype = QVariant::Void;

        if (m_type == QtFieldType::MetaProperty) {
            argtype = QVariant::nameToType(m_property.typeName());
        }

        // dynamic properties just get any QVariant
        QVariant val = convertValueToQVariant(exec, aValue, static_cast<QVariant::Type>(argtype), 0);

        if (m_type == QtFieldType::MetaProperty) {
            if (m_property.isWritable())
                m_property.write(obj, val);
        }

#ifndef QT_NO_PROPERTIES
        else if (m_type == DynamicProperty)
            obj->setProperty(m_dynamicProperty, val);
#endif

    } else {
        QString msg = QString("Can not access member `%1' of deleted QObject").formatArg(name());
        throwError(exec, createError(exec, msg.toLatin1().constData()));
    }
}


}
}
