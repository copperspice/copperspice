/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

/********************************************************
**  Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
********************************************************/

#ifndef PHONON_PHONONDEFS_P_H
#define PHONON_PHONONDEFS_P_H

#include <QtCore/QMetaType>
#include <medianode_p.h>

#define K_D(Class) Class##Private *const d = k_func()

#define PHONON_CONCAT_HELPER_INTERNAL(x, y) x ## y
#define PHONON_CONCAT_HELPER(x, y) PHONON_CONCAT_HELPER_INTERNAL(x, y)

#define PHONON_PRIVATECLASS \
protected: \
    bool aboutToDeleteBackendObject() override; \
    void createBackendObject() override; \
    void setupBackendObject();

#define PHONON_PRIVATEABSTRACTCLASS \
protected: \
    bool aboutToDeleteBackendObject() override; \
    void setupBackendObject();

#define PHONON_ABSTRACTBASE_IMPL \
PHONON_CLASSNAME::PHONON_CLASSNAME(PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private) &dd, QObject *parent) \
    : QObject(parent), \
    MediaNode(dd) \
{ \
}

#define PHONON_OBJECT_IMPL \
PHONON_CLASSNAME::PHONON_CLASSNAME(QObject *parent) \
    : QObject(parent), \
    MediaNode(*new PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private)()) \
{ \
} \
void PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private)::createBackendObject() \
{ \
    if (m_backendObject) \
        return; \
    Q_Q(PHONON_CLASSNAME); \
    m_backendObject = Factory::PHONON_CONCAT_HELPER(create, PHONON_CLASSNAME)(q); \
    if (m_backendObject) { \
        setupBackendObject(); \
    } \
}

#define PHONON_HEIR_IMPL(parentclass) \
PHONON_CLASSNAME::PHONON_CLASSNAME(QObject *parent) \
    : parentclass(*new PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private), parent) \
{ \
} \
void PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private)::createBackendObject() \
{ \
    if (m_backendObject) \
        return; \
    Q_Q(PHONON_CLASSNAME); \
    m_backendObject = Factory::PHONON_CONCAT_HELPER(create, PHONON_CLASSNAME)(q); \
    if (m_backendObject) { \
        setupBackendObject(); \
    } \
}

#define BACKEND_GET(returnType, returnVar, methodName) \
QMetaObject::invokeMethod(d->m_backendObject, methodName, Qt::DirectConnection, Q_RETURN_ARG(returnType, returnVar))
#define BACKEND_GET1(returnType, returnVar, methodName, varType1, var1) \
QMetaObject::invokeMethod(d->m_backendObject, methodName, Qt::DirectConnection, Q_RETURN_ARG(returnType, returnVar), Q_ARG(varType1, var1))
#define BACKEND_GET2(returnType, returnVar, methodName, varType1, var1, varType2, var2) \
QMetaObject::invokeMethod(d->m_backendObject, methodName, Qt::DirectConnection, Q_RETURN_ARG(returnType, returnVar), Q_ARG(varType1, var1), Q_ARG(varType2, var2))
#define BACKEND_CALL(methodName) \
QMetaObject::invokeMethod(d->m_backendObject, methodName, Qt::DirectConnection)
#define BACKEND_CALL1(methodName, varType1, var1) \
QMetaObject::invokeMethod(d->m_backendObject, methodName, Qt::DirectConnection, Q_ARG(varType1, var1))
#define BACKEND_CALL2(methodName, varType1, var1, varType2, var2) \
QMetaObject::invokeMethod(d->m_backendObject, methodName, Qt::DirectConnection, Q_ARG(varType1, var1), Q_ARG(varType2, var2))

#define pBACKEND_GET(returnType, returnVar, methodName) \
QMetaObject::invokeMethod(m_backendObject, methodName, Qt::DirectConnection, Q_RETURN_ARG(returnType, returnVar))
#define pBACKEND_GET1(returnType, returnVar, methodName, varType1, var1) \
QMetaObject::invokeMethod(m_backendObject, methodName, Qt::DirectConnection, Q_RETURN_ARG(returnType, returnVar), Q_ARG(varType1, var1))
#define pBACKEND_GET2(returnType, returnVar, methodName, varType1, var1, varType2, var2) \
QMetaObject::invokeMethod(m_backendObject, methodName, Qt::DirectConnection, Q_RETURN_ARG(returnType, returnVar), Q_ARG(varType1, var1), Q_ARG(varType2, var2))
#define pBACKEND_CALL(methodName) \
QMetaObject::invokeMethod(m_backendObject, methodName, Qt::DirectConnection)
#define pBACKEND_CALL1(methodName, varType1, var1) \
QMetaObject::invokeMethod(m_backendObject, methodName, Qt::DirectConnection, Q_ARG(varType1, var1))
#define pBACKEND_CALL2(methodName, varType1, var1, varType2, var2) \
QMetaObject::invokeMethod(m_backendObject, methodName, Qt::DirectConnection, Q_ARG(varType1, var1), Q_ARG(varType2, var2))

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace
    {
        class NoIface;

        /// All template arguments are valid
        template<typename T> struct IsValid { enum { Result = true }; };

        /// except NoIface
        template<> struct IsValid<NoIface> { enum { Result = false }; };

        template<class T> inline T my_cast(QObject *o) { return qobject_cast<T>(o); }
        template<class T> inline T my_cast(const QObject *o) { return qobject_cast<T>(o); }

        template<> inline NoIface *my_cast<NoIface *>(QObject *) { return 0; }
        template<> inline NoIface *my_cast<NoIface *>(const QObject *) { return 0; }
    }

    template<class T0, class T1 = NoIface, class T2 = NoIface>
    class Iface
    {
    public:
        static inline T0 *cast(MediaNodePrivate *const d)
        {
            if (IsValid<T1>::Result) {
                T0 *ret;
                if (IsValid<T2>::Result) {
                    ret = reinterpret_cast<T0 *>(my_cast<T2 *>(d->m_backendObject));
                    if (ret) return ret;
                }
                ret = reinterpret_cast<T0 *>(my_cast<T1 *>(d->m_backendObject));
                if (ret) return ret;
            }
            return qobject_cast<T0 *>(d->m_backendObject);
        }

        static inline const T0 *cast(const MediaNodePrivate *const d)
        {
            if (IsValid<T1>::Result) {
                const T0 *ret;
                if (IsValid<T2>::Result) {
                    ret = reinterpret_cast<const T0 *>(my_cast<T2 *>(d->m_backendObject));
                    if (ret) return ret;
                }
                ret = reinterpret_cast<const T0 *>(my_cast<T1 *>(d->m_backendObject));
                if (ret) return ret;
            }
            return qobject_cast<T0 *>(d->m_backendObject);
        }

        inline Iface(MediaNodePrivate *const d) : iface(cast(d)) {}
        inline operator       T0 *()        { return iface; }
        inline operator const T0 *()  const { return iface; }
        inline       T0 *operator->()       { Q_ASSERT(iface); return iface; }
        inline const T0 *operator->() const { Q_ASSERT(iface); return iface; }
    private:
        T0 *const iface;
    };

    template<class T0, class T1 = NoIface, class T2 = NoIface>
    class ConstIface
    {
    public:
        inline ConstIface(const MediaNodePrivate *const d) : iface(Iface<T0, T1, T2>::cast(d)) {}
        inline operator const T0 *() const { return iface; }
        inline const T0 *operator->() const { Q_ASSERT(iface); return iface; }
    private:
        const T0 *const iface;
    };
} // namespace Phonon

QT_END_NAMESPACE

#define INTERFACE_CALL(function) \
Iface<PHONON_INTERFACENAME >::cast(d)->function

#define pINTERFACE_CALL(function) \
Iface<PHONON_INTERFACENAME >::cast(this)->function

#define PHONON_GETTER(rettype, name, retdefault) \
rettype PHONON_CLASSNAME::name() const \
{ \
    const PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private) *d = k_func(); \
    if (!d->m_backendObject) \
        return retdefault; \
    rettype ret; \
    BACKEND_GET(rettype, ret, #name); \
    return ret; \
}

#define PHONON_INTERFACE_GETTER(rettype, name, retdefault) \
rettype PHONON_CLASSNAME::name() const \
{ \
    const PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private) *d = k_func(); \
    if (!d->m_backendObject) \
        return retdefault; \
    return Iface<PHONON_INTERFACENAME >::cast(d)->name(); \
}

#define PHONON_GETTER1(rettype, name, retdefault, argtype1, argvar1) \
rettype PHONON_CLASSNAME::name(argtype1 argvar1) const \
{ \
    const PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private) *d = k_func(); \
    if (!d->m_backendObject) \
        return retdefault; \
    rettype ret; \
    BACKEND_GET1(rettype, ret, #name, const QObject *, argvar1->k_ptr->backendObject()); \
    return ret; \
}

#define PHONON_INTERFACE_GETTER1(rettype, name, retdefault, argtype1, argvar1) \
rettype PHONON_CLASSNAME::name(argtype1 argvar1) const \
{ \
    const PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private) *d = k_func(); \
    if (!d->m_backendObject) \
        return retdefault; \
    return Iface<PHONON_INTERFACENAME >::cast(d)->name(argvar1->k_ptr->backendObject()); \
}

#define PHONON_SETTER(functionname, privatevar, argtype1) \
void PHONON_CLASSNAME::functionname(argtype1 x) \
{ \
    PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private) *d = k_func(); \
    d->privatevar = x; \
    if (k_ptr->backendObject()) { \
        BACKEND_CALL1(#functionname, argtype1, x); \
    } \
}

#define PHONON_INTERFACE_SETTER(functionname, privatevar, argtype1) \
void PHONON_CLASSNAME::functionname(argtype1 x) \
{ \
    PHONON_CONCAT_HELPER(PHONON_CLASSNAME, Private) *d = k_func(); \
    d->privatevar = x; \
    if (k_ptr->backendObject()) { \
        Iface<PHONON_INTERFACENAME >::cast(d)->functionname(x); \
    } \
}

#ifndef METATYPE_QLIST_INT_DEFINED
#define METATYPE_QLIST_INT_DEFINED
// Want this exactly once, see phonondefs_p.h kcm/outputdevicechoice.cpp
Q_DECLARE_METATYPE(QList<int>)
#endif

#endif // PHONONDEFS_P_H
