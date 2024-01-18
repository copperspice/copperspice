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
#include "qt_class.h"

#include "Identifier.h"
#include "qt_instance.h"
#include "qt_runtime.h"

#include <qdebug.h>
#include <qmetaobject.h>

namespace JSC {
namespace Bindings {

typedef HashMap<const QMetaObject*, QtClass *> ClassesByMetaObject;
static ClassesByMetaObject * classesByMetaObject = nullptr;

QtClass::QtClass(const QMetaObject *obj)
    : m_metaObject(obj)
{
}

QtClass::~QtClass()
{
}

QtClass * QtClass::classForObject(QObject *obj)
{
   if (! classesByMetaObject) {
      classesByMetaObject = new ClassesByMetaObject;
   }

   const QMetaObject* meta_obj = obj->metaObject();
   QtClass *myClass = classesByMetaObject->get(meta_obj);

   if (! myClass) {
      myClass = new QtClass(meta_obj);
      classesByMetaObject->set(meta_obj, myClass);
   }

   return myClass;
}

const char * QtClass::name() const
{
    return m_metaObject->className().constData();
}

// We use this to get at signals (so we can return a proper function object,
// and not get wrapped in RuntimeMethod). Also, use this for methods,
// so we can cache the object and return the same object for the same dentifier.

JSValue QtClass::fallbackObject(ExecState* exec, Instance* inst, const Identifier& identifier)
{
    QtInstance* qtinst = static_cast<QtInstance*>(inst);

    const UString &ustring = identifier.ustring();
    const QString name     = QString::fromUtf16(reinterpret_cast<const char16_t *>(ustring.characters()), ustring.length());

    // First see if we have a cache hit
    JSObject* val = qtinst->m_methods.value(name).get();

    if (val) {
        return val;
    }

    // none, create an entry
    const QString normal = QMetaObject::normalizedSignature(name);

    // See if there is an exact match
    int index = -1;

    if (normal.contains('(') && (index = m_metaObject->indexOfMethod(normal)) != -1) {

        QMetaMethod m = m_metaObject->method(index);

        if (m.access() != QMetaMethod::Private) {
            QtRuntimeMetaMethod* val = new (exec) QtRuntimeMetaMethod(exec, identifier,
                  static_cast<QtInstance*>(inst), index, normal, false);

            qtinst->m_methods.insert(name, WriteBarrier<JSObject>(exec->globalData(),
                  qtinst->createRuntimeObject(exec), val));

            return val;
        }
    }

    // Nope.. try a basename match
    const int count = m_metaObject->methodCount();

    for (index = count - 1; index >= 0; --index) {

        const QMetaMethod m = m_metaObject->method(index);

        if (m.access() == QMetaMethod::Private) {
            continue;
        }

        QString signature = m.methodSignature();
        QString::const_iterator iter = signature.indexOfFast('(');

        if (normal == QStringView(signature.begin(), iter)) {

            QtRuntimeMetaMethod* val = new (exec) QtRuntimeMetaMethod(exec, identifier,
                  static_cast<QtInstance*>(inst), index, normal, false);

            qtinst->m_methods.insert(name, WriteBarrier<JSObject>(exec->globalData(),
                  qtinst->createRuntimeObject(exec), val));

            return val;
        }
    }

    return jsUndefined();
}

// This functionality is handled by the fallback case above...
MethodList QtClass::methodsNamed(const Identifier&, Instance*) const
{
    return MethodList();
}

// may end up with a different search order than QtScript by not folding this code into the
//  fallbackMethod above, but Fields propagate out of the binding code

Field * QtClass::fieldNamed(const Identifier& identifier, Instance* instance) const
{
    // Check static properties first
    QtInstance* qtinst = static_cast<QtInstance*>(instance);

    QObject* obj = qtinst->getObject();
    const UString& ustring = identifier.ustring();

    const QString name = QString::fromUtf16(reinterpret_cast<const char16_t *>(ustring.characters()), ustring.length());

    // First check for a cached field
    QtField * f = qtinst->m_fields.value(name);

    if (obj) {
        if (f) {
            // only cache real metaproperties, but we do store the  other types so we can delete them later
            if (f->fieldType() == QtField::QtFieldType::MetaProperty) {
                return f;
            }

#ifndef QT_NO_PROPERTIES
            if (f->fieldType() == QtField::QtFieldType::DynamicProperty) {

                if (obj->dynamicPropertyNames().indexOf(name) >= 0) {
                    return f;
                }

                // Dynamic property that disappeared
                qtinst->m_fields.remove(name);
                delete f;
            }
#endif
            else {
                const QList<QObject *> &children = obj->children();
                const int count = children.size();

                for (int index = 0; index < count; ++index) {
                    QObject* child = children.at(index);

                    if (child->objectName() == name) {
                        return f;
                    }
                }

                // Did not find it, delete it from the cache
                qtinst->m_fields.remove(name);
                delete f;
            }
        }

        int index = m_metaObject->indexOfProperty(name);

        if (index >= 0) {
            const QMetaProperty prop = m_metaObject->property(index);

            if (prop.isScriptable(obj)) {
                f = new QtField(prop);
                qtinst->m_fields.insert(name, f);

                return f;
            }
        }

#ifndef QT_NO_PROPERTIES
        // Dynamic properties
        index = obj->dynamicPropertyNames().indexOf(name);

        if (index >= 0) {
            f = new QtField(name);
            qtinst->m_fields.insert(name, f);
            return f;
        }
#endif

        // Child objects
        const QList<QObject*> & children = obj->children();
        const int count = children.count();

        for (index = 0; index < count; ++index) {
            QObject* child = children.at(index);

            if (child->objectName() == name) {
                f = new QtField(child);
                qtinst->m_fields.insert(name, f);

                return f;
            }
        }

        // Nothing named this
        return 0;
    }

    // For compatibility with qtscript, cached methods don't cause errors until they are accessed,
    // so don't blindly create an error here

    if (qtinst->m_methods.contains(name)) {
        return 0;
    }

#ifndef QT_NO_PROPERTIES
    // deleted qobject, but can't throw an error from here (no exec)
    // create a fake QtField that will throw upon access

    if (! f) {
        f = new QtField(name);
        qtinst->m_fields.insert(name, f);
    }
#endif

    return f;
}

}
}

