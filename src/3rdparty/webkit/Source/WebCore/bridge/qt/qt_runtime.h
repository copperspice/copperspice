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

#ifndef BINDINGS_QT_RUNTIME_H_
#define BINDINGS_QT_RUNTIME_H_

#include "BridgeJSC.h"
#include "Completion.h"
#include "Strong.h"
#include "runtime_method.h"

#include <qbytearray.h>
#include <qmetaobject.h>
#include <qpointer.h>
#include <qvariant.h>

namespace JSC {
namespace Bindings {

class QtInstance;

class QtField : public Field {

public:
    enum QtFieldType {
        MetaProperty,
        DynamicProperty,
        ChildObject
    };

    QtField(const QMetaProperty &p)
        : m_type(QtFieldType::MetaProperty), m_property(p)
        {}

    QtField(const QString &b)
        : m_type(DynamicProperty), m_dynamicProperty(b)
        {}

    QtField(QObject *child)
        : m_type(ChildObject), m_childObject(child)
        {}

    virtual JSValue valueFromInstance(ExecState*, const Instance*) const;
    virtual void setValueToInstance(ExecState*, const Instance*, JSValue) const;
    QString name() const;
    QtFieldType fieldType() const {return m_type;}

private:
    QtFieldType m_type;
    QString m_dynamicProperty;
    QMetaProperty m_property;
    QPointer<QObject> m_childObject;
};


class QtMethod : public Method
{
public:
    QtMethod(const QMetaObject *mo, int i, const QByteArray &ident, int numParameters)
        : m_metaObject(mo),
          m_index(i),
          m_identifier(ident),
          m_nParams(numParameters)
        { }

    virtual const char* name() const { return m_identifier.constData(); }
    virtual int numParameters() const { return m_nParams; }

private:
    friend class QtInstance;
    const QMetaObject *m_metaObject;
    int m_index;
    QByteArray m_identifier;
    int m_nParams;
};


template <typename T> class QtArray : public Array
{
public:
    QtArray(QList<T> list, QVariant::Type type, PassRefPtr<RootObject>);
    virtual ~QtArray();

    RootObject* rootObject() const;

    virtual void setValueAt(ExecState*, unsigned index, JSValue) const;
    virtual JSValue valueAt(ExecState*, unsigned index) const;
    virtual unsigned int getLength() const {return m_length;}

private:
    mutable QList<T> m_list; // setValueAt is const!
    unsigned int m_length;
    QVariant::Type m_type;
};

// Based on RuntimeMethod

// Extra data classes (to avoid the CELL_SIZE limit on JS objects)

class QtRuntimeMethodData {
    public:
        virtual ~QtRuntimeMethodData();
        RefPtr<QtInstance> m_instance;
};

class QtRuntimeConnectionMethod;

class QtRuntimeMetaMethodData : public QtRuntimeMethodData {
    public:
        ~QtRuntimeMetaMethodData();

        QString m_signature;
        bool m_allowPrivate;
        int m_index;
        WriteBarrier<QtRuntimeConnectionMethod> m_connect;
        WriteBarrier<QtRuntimeConnectionMethod> m_disconnect;
};

class QtRuntimeConnectionMethodData : public QtRuntimeMethodData {
    public:
        ~QtRuntimeConnectionMethodData();

        QString m_signature;
        int m_index;
        bool m_isConnect;
};

// Common base class (doesn't really do anything interesting)
class QtRuntimeMethod : public InternalFunction {
public:
    virtual ~QtRuntimeMethod();

    static const ClassInfo s_info;

    static FunctionPrototype* createPrototype(ExecState*, JSGlobalObject* globalObject)
    {
        return globalObject->functionPrototype();
    }

    static Structure* createStructure(JSGlobalData& globalData, JSValue prototype)
    {
        return Structure::create(globalData, prototype, TypeInfo(ObjectType,  StructureFlags), AnonymousSlotCount, &s_info);
    }

protected:
    static const unsigned StructureFlags = OverridesGetOwnPropertySlot | OverridesGetPropertyNames | InternalFunction::StructureFlags | OverridesVisitChildren;

    QtRuntimeMethodData *d_func() const {return d_ptr;}
    QtRuntimeMethod(QtRuntimeMethodData *dd, ExecState *exec, const Identifier &n, PassRefPtr<QtInstance> inst);
    QtRuntimeMethodData *d_ptr;
};

class QtRuntimeMetaMethod : public QtRuntimeMethod
{
public:
    QtRuntimeMetaMethod(ExecState *exec, const Identifier &n, PassRefPtr<QtInstance> inst, int index,
                  const QString &signature, bool allowPrivate);

    virtual bool getOwnPropertySlot(ExecState *, const Identifier&, PropertySlot&);
    virtual bool getOwnPropertyDescriptor(ExecState*, const Identifier&, PropertyDescriptor&);
    virtual void getOwnPropertyNames(ExecState*, PropertyNameArray&, EnumerationMode mode = ExcludeDontEnumProperties);

    virtual void visitChildren(SlotVisitor&);

protected:
    QtRuntimeMetaMethodData* d_func() const {return reinterpret_cast<QtRuntimeMetaMethodData*>(d_ptr);}

private:
    virtual CallType getCallData(CallData&);
    static EncodedJSValue JSC_HOST_CALL call(ExecState* exec);
    static JSValue lengthGetter(ExecState*, JSValue, const Identifier&);
    static JSValue connectGetter(ExecState*, JSValue, const Identifier&);
    static JSValue disconnectGetter(ExecState*, JSValue, const Identifier&);
};

class QtConnectionObject;
class QtRuntimeConnectionMethod : public QtRuntimeMethod
{
public:
    QtRuntimeConnectionMethod(ExecState *exec, const Identifier &n, bool isConnect, PassRefPtr<QtInstance> inst,
                  int index, const QString &signature );

    virtual bool getOwnPropertySlot(ExecState *, const Identifier&, PropertySlot&);
    virtual bool getOwnPropertyDescriptor(ExecState*, const Identifier&, PropertyDescriptor&);
    virtual void getOwnPropertyNames(ExecState*, PropertyNameArray&, EnumerationMode mode = ExcludeDontEnumProperties);

protected:
    QtRuntimeConnectionMethodData* d_func() const {return reinterpret_cast<QtRuntimeConnectionMethodData*>(d_ptr);}

private:
    virtual CallType getCallData(CallData&);
    static EncodedJSValue JSC_HOST_CALL call(ExecState* exec);
    static JSValue lengthGetter(ExecState*, JSValue, const Identifier&);
    static QMultiMap<QObject *, QtConnectionObject *> connections;
    friend class QtConnectionObject;
};

class QtConnectionObject: public QObject
{
   WEB_CS_OBJECT(QtConnectionObject)

public:
    QtConnectionObject(JSGlobalData&, PassRefPtr<QtInstance> instance, int signalIndex, JSObject* thisObject, JSObject* funcObject);
    ~QtConnectionObject();

    bool match(QObject *sender, int signalIndex, JSObject* thisObject, JSObject *funcObject);

    WEB_CS_SLOT_1(Public, void execute(void **argv));
    WEB_CS_SLOT_2(execute);

private:
    RefPtr<QtInstance> m_instance;
    int m_signalIndex;
    QObject* m_originalObject; // only used as a key, not dereferenced
    Strong<JSObject> m_thisObject;
    Strong<JSObject> m_funcObject;
};

QVariant convertValueToQVariant(ExecState* exec, JSValue value, QVariant::Type hint, int *distance);
JSValue convertQVariantToValue(ExecState* exec, PassRefPtr<RootObject> root, const QVariant& variant);

} // namespace Bindings
} // namespace JSC

#endif
