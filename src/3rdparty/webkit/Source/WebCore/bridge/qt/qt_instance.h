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

#ifndef qt_instance_h
#define qt_instance_h

#include "BridgeJSC.h"
#include "runtime_root.h"
#include <qscriptengine.h>
#include <qhash.h>
#include <qpointer.h>
#include <qset.h>

namespace JSC {

namespace Bindings {

class QtClass;
class QtField;
class QtRuntimeMetaMethod;

class QtInstance : public Instance {
public:
    ~QtInstance();

    virtual Class* getClass() const;
    virtual RuntimeObject* newRuntimeObject(ExecState*);

    virtual void begin();
    virtual void end();

    virtual JSValue valueOf(ExecState*) const;
    virtual JSValue defaultValue(ExecState*, PreferredPrimitiveType) const;

    void visitAggregate(SlotVisitor&);

    virtual JSValue getMethod(ExecState* exec, const Identifier& propertyName);
    virtual JSValue invokeMethod(ExecState*, RuntimeMethod*);

    virtual void getPropertyNames(ExecState*, PropertyNameArray&);

    JSValue stringValue(ExecState* exec) const;
    JSValue numberValue(ExecState* exec) const;
    JSValue booleanValue() const;

    QObject* getObject() const { return m_object; }
    QObject* hashKey() const { return m_hashkey; }

    static PassRefPtr<QtInstance> getQtInstance(QObject*, PassRefPtr<RootObject>, QScriptEngine::ValueOwnership ownership);

    virtual bool getOwnPropertySlot(JSObject*, ExecState*, const Identifier&, PropertySlot&);
    virtual void put(JSObject*, ExecState*, const Identifier&, JSValue, PutPropertySlot&);

    void removeCachedMethod(JSObject*);

    static QtInstance* getInstance(JSObject*);

private:
    static PassRefPtr<QtInstance> create(QObject *instance, PassRefPtr<RootObject> rootObject, QScriptEngine::ValueOwnership ownership)
    {
        return adoptRef(new QtInstance(instance, rootObject, ownership));
    }

    friend class QtClass;
    friend class QtField;

    QtInstance(QObject*, PassRefPtr<RootObject>, QScriptEngine::ValueOwnership ownership); // Factory produced only..
    mutable QtClass* m_class;
    QPointer<QObject> m_object;
    QObject* m_hashkey;

    mutable QHash<QString, WriteBarrier<JSObject> > m_methods;
    mutable QHash<QString, QtField*> m_fields;
    mutable WriteBarrier<QtRuntimeMetaMethod> m_defaultMethod;

    QScriptEngine::ValueOwnership m_ownership;
};

} // namespace Bindings

} // namespace JSC

#endif
