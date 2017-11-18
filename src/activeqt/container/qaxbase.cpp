/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the ActiveQt framework of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

//#define QAX_NO_CLASSINFO

#define QT_CHECK_STATE

#include "qaxobject.h"

#ifndef QT_NO_WIN_ACTIVEQT

#include <qfile.h>
#include <qwidget.h>

#include <quuid.h>
#include <qhash.h>
#include <qset.h>
#include <qpair.h>
#include <qmetaobject.h>
#include <qsettings.h>

#include <qmutex.h>
#include <qt_windows.h>
#include <ocidl.h>
#include <ctype.h>

#include "../shared/qaxtypes.h"

QT_BEGIN_NAMESPACE

/*
    \internal
    \class QAxMetaObject

    \brief The QAxMetaObject class stores extended information
*/
struct QAxMetaObject : public QMetaObject
{
    QAxMetaObject()
    {
        d.data = 0;
        d.stringdata = 0;
    }
    ~QAxMetaObject()
    {
        delete [] (int*)d.data;
        delete [] (char*)d.stringdata;
    }

    int numParameter(const QByteArray &prototype);
    QByteArray paramType(const QByteArray &signature, int index, bool *out = 0);
    QByteArray propertyType(const QByteArray &propertyName);
    void parsePrototype(const QByteArray &prototype);
    DISPID dispIDofName(const QByteArray &name, IDispatch *disp);

private:
    friend class MetaObjectGenerator;
    // save information about QAxEventSink connections, and connect when found in cache
    QList<QUuid> connectionInterfaces;
    // DISPID -> signal name
    QMap< QUuid, QMap<DISPID, QByteArray> > sigs;
    // DISPID -> property changed signal name
    QMap< QUuid, QMap<DISPID, QByteArray> > propsigs;
    // DISPID -> property name
    QMap< QUuid, QMap<DISPID, QByteArray> > props;

    // Prototype -> member info
    QHash<QByteArray, QList<QByteArray> > memberInfo;
    QMap<QByteArray, QByteArray> realPrototype;

    // DISPID cache
    QHash<QByteArray, DISPID> dispIDs;
};

void QAxMetaObject::parsePrototype(const QByteArray &prototype)
{
    QByteArray realProto = realPrototype.value(prototype, prototype);
    QByteArray parameters = realProto.mid(realProto.indexOf('(') + 1);
    parameters.truncate(parameters.length() - 1);

    if (parameters.isEmpty()) {
        memberInfo.insert(prototype, QList<QByteArray>());
    } else {
        QList<QByteArray> plist = parameters.split(',');
        memberInfo.insert(prototype, plist);
    }
}

inline QByteArray QAxMetaObject::propertyType(const QByteArray &propertyName)
{
    return realPrototype.value(propertyName);
}

int QAxMetaObject::numParameter(const QByteArray &prototype)
{
    if (!memberInfo.contains(prototype))
        parsePrototype(prototype);

    return memberInfo.value(prototype).count();
}

QByteArray QAxMetaObject::paramType(const QByteArray &prototype, int index, bool *out)
{
    if (!memberInfo.contains(prototype))
        parsePrototype(prototype);

    if (out)
        *out = false;

    QList<QByteArray> plist = memberInfo.value(prototype);
    if (index > plist.count() - 1)
        return QByteArray();

    QByteArray param(plist.at(index));
    if (param.isEmpty())
        return QByteArray();

    bool byRef = param.endsWith('&') || param.endsWith("**");
    if (byRef) {
        param.truncate(param.length() - 1);
        if (out)
            *out = true;
    }

    return param;
}

inline DISPID QAxMetaObject::dispIDofName(const QByteArray &name, IDispatch *disp)
{
    DISPID dispid = dispIDs.value(name, DISPID_UNKNOWN);
    if (dispid == DISPID_UNKNOWN) {
        // get the Dispatch ID from the object
        QString unicodeName = QLatin1String(name);
        OLECHAR *names = (wchar_t*)unicodeName.utf16();
        disp->GetIDsOfNames(IID_NULL, &names, 1, LOCALE_USER_DEFAULT, &dispid);
        if (dispid != DISPID_UNKNOWN)
            dispIDs.insert(name, dispid);
    }
    return dispid;
}


static QHash<QString, QAxMetaObject*> mo_cache;
static QHash<QUuid, QMap<QByteArray, QList<QPair<QByteArray, int> > > > enum_cache;
static int mo_cache_ref = 0;
static QMutex cache_mutex;


static const char *const type_conversion[][2] =
{
    { "float", "double"},
    { "short", "int"},
    { "char", "int"},
    { "QList<int>", "QVariantList" },
    { "QList<uint>", "QVariantList" },
    { "QList<double>", "QVariantList" },
    { "QList<bool>", "QVariantList" },
    { "QList<QDateTime>", "QVariantList" },
    { "QList<qint64>", "QVariantList" },
    { 0, 0 }
};

/*
    \internal
    \class QAxEventSink

    \brief The QAxEventSink class implements the event sink for all
	   IConnectionPoints implemented in the COM object.
*/

class QAxEventSink : public IDispatch, public IPropertyNotifySink
{
public:
    QAxEventSink(QAxBase *com)
        : cpoint(0), ciid(IID_NULL), combase(com), ref(1)
    {}
    virtual ~QAxEventSink()
    {
        Q_ASSERT(!cpoint);
    }

    QUuid connectionInterface() const
    {
        return ciid;
    }
    QMap<DISPID, QByteArray> signalMap() const
    {
        return sigs;
    }
    QMap<DISPID, QByteArray> propertyMap() const
    {
        return props;
    }
    QMap<DISPID, QByteArray> propSignalMap() const
    {
        return propsigs;
    }

    // add a connection
    void advise(IConnectionPoint *cp, IID iid)
    {
        cpoint = cp;
        cpoint->AddRef();
        ciid = iid;
        cpoint->Advise((IUnknown*)(IDispatch*)this, &cookie);
    }

    // disconnect from all connection points
    void unadvise()
    {
        combase = 0;
        if (cpoint) {
            cpoint->Unadvise(cookie);
            cpoint->Release();
            cpoint = 0;
        }
    }

    void addSignal(DISPID memid, const char *name)
    {
	QByteArray signalname = name;
	int pi = signalname.indexOf('(');
        int i = 0;
        while (type_conversion[i][0]) {
            int ti = pi;
            int len = int(strlen(type_conversion[i][0]));
            while ((ti = signalname.indexOf(type_conversion[i][0], ti)) != -1)
	        signalname.replace(ti, len, type_conversion[i][1]);
            ++i;
        }

        sigs.insert(memid, signalname);
        QMap<DISPID,QByteArray>::ConstIterator it;
        DISPID id = -1;
        for (it = propsigs.constBegin(); it!= propsigs.constEnd(); ++it) {
            if (it.value() == signalname) {
                id = it.key();
                break;
            }
        }
        if (id != -1)
            propsigs.remove(id);
    }
    void addProperty(DISPID propid, const char *name, const char *signal)
    {
        props.insert(propid, name);
        propsigs.insert(propid, signal);
    }

    // IUnknown
    unsigned long __stdcall AddRef()
    {
        return InterlockedIncrement(&ref);
    }
    unsigned long __stdcall Release()
    {
        LONG refCount = InterlockedDecrement(&ref);
        if (!refCount)
            delete this;

        return refCount;
    }
    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject)
    {
        *ppvObject = 0;
        if (riid == IID_IUnknown)
            *ppvObject = (IUnknown*)(IDispatch*)this;
        else if (riid == IID_IDispatch)
            *ppvObject = (IDispatch*)this;
        else if (riid == IID_IPropertyNotifySink)
            *ppvObject = (IPropertyNotifySink*)this;
        else if (ciid == riid)
            *ppvObject = (IDispatch*)this;
        else
            return E_NOINTERFACE;

        AddRef();
        return S_OK;
    }

    // IDispatch
    HRESULT __stdcall GetTypeInfoCount(unsigned int *) { return E_NOTIMPL; }
    HRESULT __stdcall GetTypeInfo(UINT, LCID, ITypeInfo **) { return E_NOTIMPL; }
    HRESULT __stdcall GetIDsOfNames(const _GUID &, wchar_t **, unsigned int, unsigned long, long *) { return E_NOTIMPL; }

    HRESULT __stdcall Invoke(DISPID dispIdMember,
                            REFIID riid,
                            LCID,
                            WORD wFlags,
                            DISPPARAMS *pDispParams,
                            VARIANT*,
                            EXCEPINFO*,
                            UINT*)
    {
        // verify input
        if (riid != IID_NULL)
            return DISP_E_UNKNOWNINTERFACE;
        if (!(wFlags & DISPATCH_METHOD))
            return DISP_E_MEMBERNOTFOUND;
        if (!combase)
            return E_UNEXPECTED;

        QByteArray signame = sigs.value(dispIdMember);
        if (signame.isEmpty())
            return DISP_E_MEMBERNOTFOUND;

        QObject *qobject = combase->qObject();
        if (qobject->signalsBlocked())
            return S_OK;

        QAxMetaObject *axmeta = combase->internalMetaObject();
        const QMetaObject *meta = combase->metaObject();

        int index = -1;
        // emit the generic signal "as is"
        if (signalHasReceivers(qobject, "signal(QString,int,void*)")) {
            index = meta->indexOfSignal("signal(QString,int,void*)");
            Q_ASSERT(index != -1);

            QString nameString = QLatin1String(signame);
            void *argv[] = {0, &nameString, &pDispParams->cArgs, &pDispParams->rgvarg};
            combase->qt_metacall(QMetaObject::InvokeMetaMethod, index, argv);
        }

        HRESULT hres = S_OK;

        // get the signal information from the metaobject
        index = -1;
        if (signalHasReceivers(qobject, signame)) {
            index = meta->indexOfSignal(signame);
            Q_ASSERT(index != -1);
            const QMetaMethod signal = meta->method(index);
            Q_ASSERT(signal.methodType() == QMetaMethod::Signal);
            Q_ASSERT(signame == signal.signature());
            // verify parameter count
            int pcount = axmeta->numParameter(signame);
            int argcount = pDispParams->cArgs;
            if (pcount > argcount)
                return DISP_E_PARAMNOTOPTIONAL;
            else if (pcount < argcount)
                return DISP_E_BADPARAMCOUNT;

            // setup parameters (no return values in signals)
            bool ok = true;
            void *static_argv[QAX_NUM_PARAMS + 1];
            void *static_argv_pointer[QAX_NUM_PARAMS + 1];
            QVariant static_varp[QAX_NUM_PARAMS + 1];

            void **argv = 0;
            void **argv_pointer = 0; // in case we need an additional level of indirection
            QVariant *varp = 0;

            if (pcount) {
                if (pcount <= QAX_NUM_PARAMS) {
                    argv = static_argv;
                    argv_pointer = static_argv_pointer;
                    varp = static_varp;
                } else {
                    argv = new void*[pcount + 1];
                    argv_pointer = new void*[pcount + 1];
                    varp = new QVariant[pcount + 1];
                }

                argv[0] = 0;
                argv_pointer[0] = 0;
            }

            int p;
            for (p = 0; p < pcount && ok; ++p) {
                // map the VARIANT to the void*
                QByteArray ptype = axmeta->paramType(signame, p);
                varp[p + 1] = VARIANTToQVariant(pDispParams->rgvarg[pcount - p - 1], ptype);
                argv_pointer[p + 1] = 0;
                if (varp[p + 1].isValid()) {
                    if (varp[p + 1].type() == QVariant::UserType) {
                        argv[p + 1] = varp[p + 1].data();
                    } else if (ptype == "QVariant") {
                        argv[p + 1] = varp + p + 1;
                    } else {
                        argv[p + 1] = const_cast<void*>(varp[p + 1].constData());
                        if (ptype.endsWith('*')) {
                            argv_pointer[p + 1] = argv[p + 1];
                            argv[p + 1] = argv_pointer + p + 1;
                        }
                    }
                } else if (ptype == "QVariant") {
                    argv[p + 1] = varp + p + 1;
                } else {
                    ok = false;
                }
            }

            if (ok) {
                // emit the generated signal if everything went well
                combase->qt_metacall(QMetaObject::InvokeMetaMethod, index, argv);
                // update the VARIANT for references and free memory
                for (p = 0; p < pcount; ++p) {
                    bool out;
                    QByteArray ptype = axmeta->paramType(signame, p, &out);
                    if (out) {
                        if (!QVariantToVARIANT(varp[p + 1], pDispParams->rgvarg[pcount - p - 1], ptype, out))
                            ok = false;
                    }
                }
            }

            if (argv != static_argv) {
                delete [] argv;
                delete [] argv_pointer;
                delete [] varp;
            }
            hres = ok ? S_OK : (ok ? DISP_E_MEMBERNOTFOUND : DISP_E_TYPEMISMATCH);
        }

        return hres;
    }

    QByteArray findProperty(DISPID dispID);

    // IPropertyNotifySink
    HRESULT __stdcall OnChanged(DISPID dispID)
    {
        // verify input
        if (dispID == DISPID_UNKNOWN || !combase)
            return S_OK;

        const QMetaObject *meta = combase->metaObject();
        if (!meta)
            return S_OK;

        QByteArray propname(findProperty(dispID));
        if (propname.isEmpty())
            return S_OK;

        QObject *qobject = combase->qObject();
        if (qobject->signalsBlocked())
            return S_OK;

        // emit the generic signal
        int index = meta->indexOfSignal("propertyChanged(QString)");
        if (index != -1) {
            QString propnameString = QString::fromLatin1(propname);
            void *argv[] = {0, &propnameString};
            combase->qt_metacall(QMetaObject::InvokeMetaMethod, index, argv);
        }

        QByteArray signame = propsigs.value(dispID);
        if (signame.isEmpty())
            return S_OK;

        index = meta->indexOfSignal(signame);
        if (index == -1) // bindable but not marked as bindable in typelib
            return S_OK;

        // get the signal information from the metaobject
        if (signalHasReceivers(qobject, signame)) {
            index = meta->indexOfSignal(signame);
            Q_ASSERT(index != -1);
            // setup parameters
            QVariant var = qobject->property(propname);
            if (!var.isValid())
                return S_OK;

            const QMetaProperty metaProp = meta->property(meta->indexOfProperty(propname));
            void *argv[] = {0, var.data()};
            if (metaProp.type() == QVariant::LastType)
                argv[1] = &var;

            // emit the "changed" signal
            combase->qt_metacall(QMetaObject::InvokeMetaMethod, index, argv);
        }
        return S_OK;
    }
    HRESULT __stdcall OnRequestEdit(DISPID dispID)
    {
        if (dispID == DISPID_UNKNOWN || !combase)
            return S_OK;

        QByteArray propname(findProperty(dispID));
        if (propname.isEmpty())
            return S_OK;

        return combase->propertyWritable(propname) ? S_OK : S_FALSE;
    }

    static bool signalHasReceivers(QObject *qobject, const char *signalName)
    {
        Q_ASSERT(qobject);
        return ((QAxObject*)qobject)->receivers(QByteArray::number(QSIGNAL_CODE) + signalName);
    }

    IConnectionPoint *cpoint;
    IID ciid;
    ULONG cookie;

    QMap<DISPID, QByteArray> sigs;
    QMap<DISPID, QByteArray> propsigs;
    QMap<DISPID, QByteArray> props;

    QAxBase *combase;
    LONG ref;
};

/*
    \internal
    \class QAxBasePrivate
*/

class QAxBasePrivate
{
public:
    QAxBasePrivate()
        : useEventSink(true), useMetaObject(true), useClassInfo(true),
        cachedMetaObject(false), initialized(false), tryCache(false),
        ptr(0), disp(0), metaobj(0)
    {
        // protect initialization
        QMutexLocker locker(&cache_mutex);
        mo_cache_ref++;

        qRegisterMetaType<IUnknown*>("IUnknown*", &ptr);
        qRegisterMetaType<IDispatch*>("IDispatch*", &disp);
    }

    ~QAxBasePrivate()
    {
        Q_ASSERT(!ptr);
        Q_ASSERT(!disp);

        // protect cleanup
        QMutexLocker locker(&cache_mutex);
        if (!--mo_cache_ref) {
            qDeleteAll(mo_cache);
            mo_cache.clear();
        }

        CoFreeUnusedLibraries();
    }

    inline IDispatch *dispatch() const
    {
        if (disp)
            return disp;

        if (ptr)
            ptr->QueryInterface(IID_IDispatch, (void**)&disp);
        return disp;
    }

    QString ctrl;

    QHash<QUuid, QAxEventSink*> eventSink;
    uint useEventSink	    :1;
    uint useMetaObject	    :1;
    uint useClassInfo	    :1;
    uint cachedMetaObject   :1;
    uint initialized	    :1;
    uint tryCache	    :1;

    IUnknown *ptr;
    mutable IDispatch *disp;

    QMap<QByteArray, bool> propWritable;

    inline QAxMetaObject *metaObject()
    {
        if (!metaobj)
            metaobj = new QAxMetaObject;
        return metaobj;
    }

    mutable QMap<QString, LONG> verbs;

    QAxMetaObject *metaobj;
};


QByteArray QAxEventSink::findProperty(DISPID dispID)
{
    // look up in cache, and fall back to
    // type info for precompiled metaobjects
    QByteArray propname(props.value(dispID));

    if (!propname.isEmpty())
        return propname;

    IDispatch *dispatch = combase->d->dispatch();
    ITypeInfo *typeinfo = 0;
    if (dispatch)
        dispatch->GetTypeInfo(0, LOCALE_USER_DEFAULT, &typeinfo);
    if (!typeinfo)
        return propname;

    BSTR names;
    UINT cNames;
    typeinfo->GetNames(dispID, &names, 1, &cNames);
    if (cNames) {
        propname = QString::fromWCharArray(names).toLatin1();
        SysFreeString(names);
    }
    typeinfo->Release();

    QByteArray propsignal(propname + "Changed(");
    const QMetaObject *mo = combase->metaObject();
    int index = mo->indexOfProperty(propname);
    const QMetaProperty prop = mo->property(index);
    propsignal += prop.typeName();
    propsignal += ')';
    addProperty(dispID, propname, propsignal);

    return propname;
}

/*!
    \class QAxBase
    \brief The QAxBase class is an abstract class that provides an API
    to initialize and access a COM object.

    \inmodule QAxContainer

    QAxBase is an abstract class that cannot be used directly, and is
    instantiated through the subclasses QAxObject and QAxWidget. This
    class provides the API to access the COM object directly
    through its IUnknown implementation. If the COM object implements
    the IDispatch interface, the properties and methods of that object
    become available as Qt properties and slots.

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 0

    Properties exposed by the object's IDispatch implementation can
    be read and written through the property system provided by the
    Qt Object Model (both subclasses are QObjects, so you can use
    QObject::setProperty() and QObject::property()). Properties with
    multiple parameters are not supported.

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 1

    Write-functions for properties and other methods exposed by the
    object's IDispatch implementation can be called directly using
    dynamicCall(), or indirectly as slots connected to a signal.

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 2

    Outgoing events supported by the COM object are emitted as
    standard Qt signals.

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 3

    QAxBase transparently converts between COM data types and the
    equivalent Qt data types. Some COM types have no equivalent Qt data structure.

    Supported COM datatypes are listed in the first column of following table.
    The second column is the Qt type that can be used with the QObject property
    functions. The third column is the Qt type that is used in the prototype of
    generated signals and slots for in-parameters, and the last column is the Qt
    type that is used in the prototype of signals and slots for out-parameters.
    \table
    \header
    \i COM type
    \i Qt property
    \i in-parameter
    \i out-parameter
    \row
    \i VARIANT_BOOL
    \i bool
    \i bool
    \i bool&
    \row
    \i BSTR
    \i QString
    \i const QString&
    \i QString&
    \row
    \i char, short, int, long
    \i int
    \i int
    \i int&
    \row
    \i uchar, ushort, uint, ulong
    \i uint
    \i uint
    \i uint&
    \row
    \i float, double
    \i double
    \i double
    \i double&
    \row
    \i DATE
    \i QDateTime
    \i const QDateTime&
    \i QDateTime&
    \row
    \i CY
    \i qint64
    \i qint64
    \i qint64&
    \row
    \i OLE_COLOR
    \i QColor
    \i const QColor&
    \i QColor&
    \row
    \i SAFEARRAY(VARIANT)
    \i QList\<QVariant\>
    \i const QList\<QVariant\>&
    \i QList\<QVariant\>&
    \row
    \i SAFEARRAY(int), SAFEARRAY(double), SAFEARRAY(Date)
    \i QList\<QVariant\>
    \i const QList\<QVariant\>&
    \i QList\<QVariant\>&
    \row
    \i SAFEARRAY(BYTE)
    \i QByteArray
    \i const QByteArray&
    \i QByteArray&
    \row
    \i SAFEARRAY(BSTR)
    \i QStringList
    \i const QStringList&
    \i QStringList&
    \row
    \i VARIANT
    \i type-dependent
    \i const QVariant&
    \i QVariant&
    \row
    \i IFontDisp*
    \i QFont
    \i const QFont&
    \i QFont&
    \row
    \i IPictureDisp*
    \i QPixmap
    \i const QPixmap&
    \i QPixmap&
    \row
    \i IDispatch*
    \i QAxObject*
    \i \c QAxBase::asVariant()
    \i QAxObject* (return value)
    \row
    \i IUnknown*
    \i QAxObject*
    \i \c QAxBase::asVariant()
    \i QAxObject* (return value)
    \row
    \i SCODE, DECIMAL
    \i \e unsupported
    \i \e unsupported
    \i \e unsupported
    \row
    \i VARIANT* (Since Qt 4.5)
    \i \e unsupported
    \i \e QVariant&
    \i \e QVariant&
    \endtable

    Supported are also enumerations, and typedefs to supported types.

    To call the methods of a COM interface described by the following IDL

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 4

    use the QAxBase API like this:

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 5

    Note that the QList the object should fill has to be provided as an
    element in the parameter list of \l{QVariant}s.

    If you need to access properties or pass parameters of
    unsupported datatypes you must access the COM object directly
    through its \c IDispatch implementation or other interfaces.
    Those interfaces can be retrieved through queryInterface().

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 6

    To get the definition of the COM interfaces you will have to use the header
    files provided with the component you want to use. Some compilers can also
    import type libraries using the #import compiler directive. See the component
    documentation to find out which type libraries you have to import, and how to use
    them.

    If you need to react to events that pass parameters of unsupported
    datatypes you can use the generic signal that delivers the event
    data as provided by the COM event.

    \sa QAxObject, QAxWidget, QAxScript, {ActiveQt Framework}
*/

/*!
    \typedef QAxBase::PropertyBag

    A QMap<QString,QVariant> that can store properties as name:value pairs.
*/

/*!
    Creates a QAxBase object that wraps the COM object \a iface. If \a
    iface is 0 (the default), use setControl() to instantiate a COM
    object.
*/
QAxBase::QAxBase(IUnknown *iface)
{
    d = new QAxBasePrivate();
    d->ptr = iface;
    if (d->ptr) {
        d->ptr->AddRef();
        d->initialized = true;
    }
}

/*!
    Shuts down the COM object and destroys the QAxBase object.

    \sa clear()
*/
QAxBase::~QAxBase()
{
    clear();

    delete d;
    d = 0;
}

/*!
    \internal

    Used by subclasses generated with dumpcpp to balance reference count.
*/
void QAxBase::internalRelease()
{
    if (d->ptr)
        d->ptr->Release();
}

/*!
    \internal

    Used by subclasses generated with dumpcpp to implement cast-operators.
*/
void QAxBase::initializeFrom(QAxBase *that)
{
    if (d->ptr)
        return;

    d->ptr = that->d->ptr;
    if (d->ptr) {
        d->ptr->AddRef();
        d->initialized = true;
    }
}


QAxMetaObject *QAxBase::internalMetaObject() const
{
    return d->metaObject();
}

/*!
    \property QAxBase::control
    \brief the name of the COM object wrapped by this QAxBase object.

    Setting this property initializes the COM object. Any COM object
    previously set is shut down.

    The most efficient way to set this property is by using the
    registered component's UUID, e.g.

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 7

    The second fastest way is to use the registered control's class
    name (with or without version number), e.g.

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 8

    The slowest, but easiest way to use is to use the control's full
    name, e.g.

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 9

    It is also possible to initialize the object from a file, e.g.

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 10

    If the component's UUID is used the following patterns can be used
    to initialize the control on a remote machine, to initialize a
    licensed control or to connect to a running object:
    \list
    \i To initialize the control on a different machine use the following
    pattern:

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 11

    \i To initialize a licensed control use the following pattern:

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 12

    \i To connect to an already running object use the following pattern:

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 13

    \endlist
    The first two patterns can be combined, e.g. to initialize a licensed
    control on a remote machine:

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 14

    The control's read function always returns the control's UUID, if provided including the license
    key, and the name of the server, but not including the username, the domain or the password.
*/
bool QAxBase::setControl(const QString &c)
{
    if (c.toLower() == d->ctrl.toLower())
        return !d->ctrl.isEmpty();

    QString search = c;
    // don't waste time for DCOM requests
    int dcomIDIndex = search.indexOf(QLatin1String("/{"));
    if ((dcomIDIndex == -1 || dcomIDIndex != search.length()-39) && !search.endsWith(QLatin1String("}&"))) {
        QUuid uuid(search);
        if (uuid.isNull()) {
            CLSID clsid;
            HRESULT res = CLSIDFromProgID((wchar_t*)c.utf16(), &clsid);
            if (res == S_OK)
                search = QUuid(clsid).toString();
            else {
                QSettings controls(QLatin1String("HKEY_LOCAL_MACHINE\\Software\\Classes\\"), QSettings::NativeFormat);
                search = controls.value(c + QLatin1String("/CLSID/Default")).toString();
                if (search.isEmpty()) {
                    controls.beginGroup(QLatin1String("/CLSID"));
                    QStringList clsids = controls.childGroups();
                    for (QStringList::Iterator it = clsids.begin(); it != clsids.end(); ++it) {
                        QString clsid = *it;
                        QString name = controls.value(clsid + QLatin1String("/Default")).toString();
                        if (name == c) {
                            search = clsid;
                            break;
                        }
                    }
                    controls.endGroup();
                }
            }
        }
        if (search.isEmpty())
            search = c;
    }

    if (search.toLower() == d->ctrl.toLower())
        return !d->ctrl.isEmpty();

    clear();
    d->ctrl = search;

    d->tryCache = true;
    if (!initialize(&d->ptr))
        d->initialized = true;
    if (isNull()) {
        qWarning("QAxBase::setControl: requested control %s could not be instantiated", c.toLatin1().data());
        clear();
        return false;
    }
    return true;
}

QString QAxBase::control() const
{
    return d->ctrl;
}

/*!
    Disables the event sink implementation for this ActiveX container.
    If you don't intend to listen to the ActiveX control's events use
    this function to speed up the meta object generation.

    Some ActiveX controls might be unstable when connected to an event
    sink. To get OLE events you must use standard COM methods to
    register your own event sink. Use queryInterface() to get access
    to the raw COM object.

    Note that this function should be called immediately after
    construction of the object.
*/
void QAxBase::disableEventSink()
{
    d->useEventSink = false;
}

/*!
    Disables the meta object generation for this ActiveX container.
    This also disables the event sink and class info generation. If
    you don't intend to use the Qt meta object implementation call
    this function to speed up instantiation of the control. You will
    still be able to call the object through \l dynamicCall(), but
    signals, slots and properties will not be available with QObject
    APIs.

    Some ActiveX controls might be unstable when used with OLE
    automation. Use standard COM methods to use those controls through
    the COM interfaces provided by queryInterface().

    Note that this function must be called immediately after
    construction of the object.
*/
void QAxBase::disableMetaObject()
{
    d->useMetaObject	= false;
    d->useEventSink	= false;
    d->useClassInfo	= false;
}

/*!
    Disables the class info generation for this ActiveX container. If
    you don't require any class information about the ActiveX control
    use this function to speed up the meta object generation.

    Note that this function must be called immediately after
    construction of the object
*/
void QAxBase::disableClassInfo()
{
    d->useClassInfo = false;
}

/*!
    Disconnects and destroys the COM object.

    If you reimplement this function you must also reimplement the
    destructor to call clear(), and call this implementation at the
    end of your clear() function.
*/
void QAxBase::clear()
{
    QHash<QUuid, QAxEventSink*>::Iterator it =  d->eventSink.begin();
    while (it != d->eventSink.end()) {
        QAxEventSink *eventSink = it.value();
        ++it;
        if (eventSink) {
            eventSink->unadvise();
            eventSink->Release();
        }
    }
    d->eventSink.clear();
    if (d->disp) {
        d->disp->Release();
        d->disp = 0;
    }
    if (d->ptr) {
        d->ptr->Release();
        d->ptr = 0;
        d->initialized = false;
    }

    d->ctrl.clear();

    if (!d->cachedMetaObject)
        delete d->metaobj;
    d->metaobj = 0;
}

/*!
    \since 4.1

    Returns the list of verbs that the COM object can execute. If
    the object does not implement IOleObject, or does not support
    any verbs, then this function returns an empty stringlist.

    Note that the OLE default verbs (OLEIVERB_SHOW etc) are not
    included in the list.
*/
QStringList QAxBase::verbs() const
{
    if (!d->ptr)
        return QStringList();

    if (d->verbs.isEmpty()) {
        IOleObject *ole = 0;
        d->ptr->QueryInterface(IID_IOleObject, (void**)&ole);
        if (ole) {
            IEnumOLEVERB *enumVerbs = 0;
            ole->EnumVerbs(&enumVerbs);
            if (enumVerbs) {
                enumVerbs->Reset();
                ULONG c;
                OLEVERB verb;
                while (enumVerbs->Next(1, &verb, &c) == S_OK) {
                    if (!verb.lpszVerbName)
                        continue;
                    QString verbName = QString::fromWCharArray(verb.lpszVerbName);
                    if (!verbName.isEmpty())
                        d->verbs.insert(verbName, verb.lVerb);
                }
                enumVerbs->Release();
            }
            ole->Release();
        }
    }

    return d->verbs.keys();
}

/*!
    \internal
*/

long QAxBase::indexOfVerb(const QString &verb) const
{
    return d->verbs.value(verb);
}

/*!
    This virtual function is called by setControl() and creates the
    requested COM object. \a ptr is set to the object's IUnknown
    implementation. The function returns true if the object
    initialization succeeded; otherwise the function returns false.

    The default implementation interprets the string returned by
    control(), and calls initializeRemote(), initializeLicensed()
    or initializeActive() if the string matches the respective
    patterns. If control() is the name of an existing file,
    initializeFromFile() is called. If no pattern is matched, or
    if remote or licensed initialization fails, CoCreateInstance
    is used directly to create the object.

    See the \l control property documentation for details about
    supported patterns.

    The interface returned in \a ptr must be referenced exactly once
    when this function returns. The interface provided by e.g.
    CoCreateInstance is already referenced, and there is no need to
    reference it again.
*/
bool QAxBase::initialize(IUnknown **ptr)
{
    if (*ptr || control().isEmpty())
        return false;

    *ptr = 0;

    bool res = false;

    const QString ctrl(d->ctrl);
    if (ctrl.contains(QLatin1String("/{"))) // DCOM request
        res = initializeRemote(ptr);
    else if (ctrl.contains(QLatin1String("}:"))) // licensed control
        res = initializeLicensed(ptr);
    else if (ctrl.contains(QLatin1String("}&"))) // running object
        res = initializeActive(ptr);
    else if (QFile::exists(ctrl)) // existing file
        res = initializeFromFile(ptr);

    if (!res) { // standard
        HRESULT hres = CoCreateInstance(QUuid(ctrl), 0, CLSCTX_SERVER, IID_IUnknown, (void**)ptr);
        res = S_OK == hres;
#ifndef QT_NO_DEBUG
        if (!res)
            qErrnoWarning(hres, "CoCreateInstance failure");
#endif
    }

    return *ptr != 0;
}

/*!
    Creates an instance of a licensed control, and returns the IUnknown interface
    to the object in \a ptr. This functions returns true if successful, otherwise
    returns false.

    This function is called by initialize() if the control string contains the
    substring "}:". The license key needs to follow this substring.

    \sa initialize()
*/
bool QAxBase::initializeLicensed(IUnknown** ptr)
{
    int at = control().lastIndexOf(QLatin1String("}:"));

    QString clsid(control().left(at));
    QString key(control().mid(at+2));

    IClassFactory *factory = 0;
    CoGetClassObject(QUuid(clsid), CLSCTX_SERVER, 0, IID_IClassFactory, (void**)&factory);
    if (!factory)
        return false;
    initializeLicensedHelper(factory, key, ptr);
    factory->Release();

    return *ptr != 0;
}

/* \internal
    Called by initializeLicensed and initializedRemote to create an object
    via IClassFactory2.
*/
bool QAxBase::initializeLicensedHelper(void *f, const QString &key, IUnknown **ptr)
{
    IClassFactory *factory = (IClassFactory*)f;
    IClassFactory2 *factory2 = 0;
    factory->QueryInterface(IID_IClassFactory2, (void**)&factory2);
    if (factory2) {
        BSTR bkey = QStringToBSTR(key);
        HRESULT hres = factory2->CreateInstanceLic(0, 0, IID_IUnknown, bkey, (void**)ptr);
        SysFreeString(bkey);
#ifdef QT_DEBUG
        LICINFO licinfo;
        licinfo.cbLicInfo = sizeof(LICINFO);
        factory2->GetLicInfo(&licinfo);

        if (hres != S_OK) {
            SetLastError(hres);
            qErrnoWarning("CreateInstanceLic failed");
            if (!licinfo.fLicVerified) {
                qWarning("Wrong license key specified, and machine is not fully licensed.");
            } else if (licinfo.fRuntimeKeyAvail) {
                BSTR licenseKey;
                factory2->RequestLicKey(0, &licenseKey);
                QString qlicenseKey = QString::fromWCharArray(licenseKey);
                SysFreeString(licenseKey);
                qWarning("Use license key is '%s' to create object on unlicensed machine.",
                    qlicenseKey.toLatin1().constData());
            }
        } else if (licinfo.fLicVerified) {
            qWarning("Machine is fully licensed for '%s'", control().toLatin1().constData());
            if (licinfo.fRuntimeKeyAvail) {
                BSTR licenseKey;
                factory2->RequestLicKey(0, &licenseKey);
                QString qlicenseKey = QString::fromWCharArray(licenseKey);
                SysFreeString(licenseKey);

                if (qlicenseKey != key)
                    qWarning("Runtime license key is '%s'", qlicenseKey.toLatin1().constData());
            }
        }
#else
        Q_UNUSED(hres);
#endif
        factory2->Release();
    } else {  // give it a shot without license
        factory->CreateInstance(0, IID_IUnknown, (void**)ptr);
    }
    return *ptr != 0;
}


/*!
    Connects to an active instance running on the current machine, and returns the
    IUnknown interface to the running object in \a ptr. This function returns true
    if successful, otherwise returns false.

    This function is called by initialize() if the control string contains the
    substring "}&".

    \sa initialize()
*/
bool QAxBase::initializeActive(IUnknown** ptr)
{
    int at = control().lastIndexOf(QLatin1String("}&"));
    QString clsid(control().left(at));

    GetActiveObject(QUuid(clsid), 0, ptr);

    return *ptr != 0;

}

#ifdef Q_CC_GNU
#   ifndef OLEPENDER_NONE
#   define OLERENDER_NONE 0
#   endif
#endif

/*!
    Creates the COM object handling the filename in the control property, and
    returns the IUnknown interface to the object in \a ptr. This function returns
    true if successful, otherwise returns false.

    This function is called by initialize() if the control string is the name of
    an existing file.

    \sa initialize()
*/
bool QAxBase::initializeFromFile(IUnknown** ptr)
{
    IStorage *storage = 0;
    ILockBytes * bytes = 0;
    HRESULT hres = ::CreateILockBytesOnHGlobal(0, TRUE, &bytes);
    hres = ::StgCreateDocfileOnILockBytes(bytes, STGM_SHARE_EXCLUSIVE|STGM_CREATE|STGM_READWRITE, 0, &storage);

    hres = OleCreateFromFile(CLSID_NULL, reinterpret_cast<const wchar_t*>(control().utf16()), IID_IUnknown, OLERENDER_NONE, 0, 0, storage, (void**)ptr);

    storage->Release();
    bytes->Release();

    return hres == S_OK;
}


// There seams to be a naming problem in mingw headers
#if defined(Q_CC_GNU) && !defined(COAUTHIDENTITY) && !defined(__MINGW64_VERSION_MAJOR)
#define COAUTHIDENTITY AUTH_IDENTITY
#endif


/*!
    Creates the instance on a remote server, and returns the IUnknown interface
    to the object in \a ptr. This function returns true if successful, otherwise
    returns false.

    This function is called by initialize() if the control string contains the
    substring "/{". The information about the remote machine needs to be provided
    in front of the substring.

    \sa initialize()
*/
bool QAxBase::initializeRemote(IUnknown** ptr)
{
    int at = control().lastIndexOf(QLatin1String("/{"));

    QString server(control().left(at));
    QString clsid(control().mid(at+1));

    QString user;
    QString domain;
    QString passwd;
    QString key;

    at = server.indexOf(QChar::fromLatin1('@'));
    if (at != -1) {
        user = server.left(at);
        server = server.mid(at+1);

        at = user.indexOf(QChar::fromLatin1(':'));
        if (at != -1) {
            passwd = user.mid(at+1);
            user = user.left(at);
        }
        at = user.indexOf(QChar::fromLatin1('/'));
        if (at != -1) {
            domain = user.left(at);
            user = user.mid(at+1);
        }
    }

    at = clsid.lastIndexOf(QLatin1String("}:"));
    if (at != -1) {
        key = clsid.mid(at+2);
        clsid = clsid.left(at);
    }

    d->ctrl = server + QChar::fromLatin1('/') + clsid;
    if (!key.isEmpty())
        d->ctrl = d->ctrl + QChar::fromLatin1(':') + key;

    COAUTHIDENTITY authIdentity;
    authIdentity.UserLength = user.length();
    authIdentity.User = authIdentity.UserLength ? (ushort*)user.utf16() : 0;
    authIdentity.DomainLength = domain.length();
    authIdentity.Domain = authIdentity.DomainLength ? (ushort*)domain.utf16() : 0;
    authIdentity.PasswordLength = passwd.length();
    authIdentity.Password = authIdentity.PasswordLength ? (ushort*)passwd.utf16() : 0;
    authIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

    COAUTHINFO authInfo;
    authInfo.dwAuthnSvc = RPC_C_AUTHN_WINNT;
    authInfo.dwAuthzSvc = RPC_C_AUTHZ_NONE;
    authInfo.pwszServerPrincName = 0;
    authInfo.dwAuthnLevel = RPC_C_AUTHN_LEVEL_DEFAULT;
    authInfo.dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
    authInfo.pAuthIdentityData = &authIdentity;
    authInfo.dwCapabilities = 0;

    COSERVERINFO serverInfo;
    serverInfo.dwReserved1 = 0;
    serverInfo.dwReserved2 = 0;
    serverInfo.pAuthInfo = &authInfo;
    serverInfo.pwszName = (wchar_t*)server.utf16();

    IClassFactory *factory = 0;
    HRESULT res = CoGetClassObject(QUuid(clsid), CLSCTX_REMOTE_SERVER, &serverInfo, IID_IClassFactory, (void**)&factory);
    if (factory) {
        if (!key.isEmpty())
            initializeLicensedHelper(factory, key, ptr);
        else
            res = factory->CreateInstance(0, IID_IUnknown, (void**)ptr);
        factory->Release();
    }
#ifndef QT_NO_DEBUG
    if (res != S_OK)
        qErrnoWarning(res, "initializeRemote Failed");
#endif

    return res == S_OK;
}

/*!
    Requests the interface \a uuid from the COM object and sets the
    value of \a iface to the provided interface, or to 0 if the
    requested interface could not be provided.

    Returns the result of the QueryInterface implementation of the COM object.

    \sa control
*/
long QAxBase::queryInterface(const QUuid &uuid, void **iface) const
{
    *iface = 0;
    if (!d->ptr) {
        ((QAxBase*)this)->initialize(&d->ptr);
        d->initialized = true;
    }

    if (d->ptr && !uuid.isNull())
        return d->ptr->QueryInterface(uuid, iface);

    return E_NOTIMPL;
}

class MetaObjectGenerator
{
public:
    MetaObjectGenerator(QAxBase *ax, QAxBasePrivate *dptr);
    MetaObjectGenerator(ITypeLib *typelib, ITypeInfo *typeinfo);
    ~MetaObjectGenerator();

    QMetaObject *metaObject(const QMetaObject *parentObject, const QByteArray &className = QByteArray());

    void readClassInfo();
    void readEnumInfo();
    void readInterfaceInfo();
    void readFuncsInfo(ITypeInfo *typeinfo, ushort nFuncs);
    void readVarsInfo(ITypeInfo *typeinfo, ushort nVars);
    void readEventInfo();
    void readEventInterface(ITypeInfo *eventinfo, IConnectionPoint *cpoint);

    inline void addClassInfo(const char *key, const char *value)
    {
        classinfo_list.insert(key, value);
    }

private:
    void init();


    QMetaObject *tryCache();

    QByteArray createPrototype(FUNCDESC *funcdesc, ITypeInfo *typeinfo, const QList<QByteArray> &names,
        QByteArray &type, QList<QByteArray> &parameters);

    QByteArray usertypeToString(const TYPEDESC &tdesc, ITypeInfo *info, const QByteArray &function);
    QByteArray guessTypes(const TYPEDESC &tdesc, ITypeInfo *info, const QByteArray &function);

    // ### from qmetaobject.cpp
    enum ProperyFlags  {
        Invalid			= 0x00000000,
        Readable		= 0x00000001,
        Writable		= 0x00000002,
        Resettable		= 0x00000004,
        EnumOrFlag		= 0x00000008,
        StdCppSet		= 0x00000100,
//        Override		= 0x00000200,
        Designable		= 0x00001000,
        ResolveDesignable	= 0x00002000,
        Scriptable		= 0x00004000,
        ResolveScriptable	= 0x00008000,
        Stored                  = 0x00010000,
        ResolveStored           = 0x00020000,
        Editable		= 0x00040000,
        ResolveEditable         = 0x00080000,
        User                    = 0x00100000,
        ResolveUser             = 0x00200000,
        // And our own - don't use the upper byte, as it's used for the property type
        RequestingEdit          = 0x00400000,
        Bindable                = 0x00800000
    };
    enum MemberFlags {
        AccessPrivate = 0x00,
        AccessProtected = 0x01,
        AccessPublic = 0x02,
        MemberMethod = 0x00,
        MemberSignal = 0x04,
        MemberSlot = 0x08,
        MemberCompatibility = 0x10,
        MemberCloned = 0x20,
        MemberScriptable = 0x40,
    };

    inline QList<QByteArray> paramList(const QByteArray &proto)
    {
        QByteArray prototype(proto);
        QByteArray parameters = prototype.mid(prototype.indexOf('(') + 1);
        parameters.truncate(parameters.length() - 1);

        QList<QByteArray> plist = parameters.split(',');
        return plist;
    }

    inline QByteArray replaceType(const QByteArray &type)
    {
        int i = 0;
        while (type_conversion[i][0]) {
            int len = int(strlen(type_conversion[i][0]));
            int ti;
            if ((ti = type.indexOf(type_conversion[i][0])) != -1) {
                QByteArray rtype(type);
                rtype.replace(ti, len, type_conversion[i][1]);
                return rtype;
            }
            ++i;
        }
        return type;
    }

    QByteArray replacePrototype(const QByteArray &prototype)
    {
        QByteArray proto(prototype);

        QList<QByteArray> plist = paramList(prototype);
        for (int p = 0; p < plist.count(); ++p) {
            QByteArray param(plist.at(p));
            if (param != replaceType(param)) {
                int type = 0;
                while (type_conversion[type][0]) {
                    int paren = proto.indexOf('(');
                    while ((paren = proto.indexOf(type_conversion[type][0])) != -1) {
                        proto.replace(paren, qstrlen(type_conversion[type][0]), type_conversion[type][1]);
                    }
                    ++type;
                }
                break;
            }
        }

        return proto;
    }

    QMap<QByteArray, QByteArray> classinfo_list;

    inline bool hasClassInfo(const char *key)
    {
        return classinfo_list.contains(key);
    }

    struct Method {
        Method() : flags(0)
        {}
        QByteArray type;
        QByteArray parameters;
        int flags;
        QByteArray realPrototype;
    };
    QMap<QByteArray, Method> signal_list;
    inline void addSignal(const QByteArray &prototype, const QByteArray &parameters)
    {
        QByteArray proto(replacePrototype(prototype));

        Method &signal = signal_list[proto];
        signal.type = 0;
        signal.parameters = parameters;
        signal.flags = QMetaMethod::Public | MemberSignal;
        if (proto != prototype)
            signal.realPrototype = prototype;
    }

    void addChangedSignal(const QByteArray &function, const QByteArray &type, int memid);

    inline bool hasSignal(const QByteArray &prototype)
    {
        return signal_list.contains(prototype);
    }

    QMap<QByteArray, Method> slot_list;
    inline void addSlot(const QByteArray &type, const QByteArray &prototype, const QByteArray &parameters, int flags = QMetaMethod::Public)
    {
        QByteArray proto = replacePrototype(prototype);

        Method &slot = slot_list[proto];
        slot.type = replaceType(type);
        slot.parameters = parameters;
        slot.flags = flags | MemberSlot;
        if (proto != prototype)
            slot.realPrototype = prototype;
    }

    void addSetterSlot(const QByteArray &property);

    inline bool hasSlot(const QByteArray &prototype)
    {
        return slot_list.contains(prototype);
    }

    struct Property {
        Property() : typeId(0)
        {}
        QByteArray type;
        uint typeId;
        QByteArray realType;
    };
    QMap<QByteArray, Property> property_list;
    void addProperty(const QByteArray &type, const QByteArray &name, uint flags)
    {
        QByteArray propertyType(type);
        if (propertyType.endsWith('&'))
            propertyType.chop(1);

        Property &prop = property_list[name];
        if (!propertyType.isEmpty() && propertyType != "HRESULT") {
            prop.type = replaceType(propertyType);
            if (prop.type != propertyType)
                prop.realType = propertyType;
        }
        if (flags & Writable)
            flags |= Stored;
        prop.typeId |= flags;
        QVariant::Type vartype = QVariant::nameToType(prop.type);
        switch(vartype) {
        case QVariant::Invalid:
        case QVariant::UserType:
            if (prop.type == "QVariant") {
                prop.typeId |= 0xff << 24;
                break;
            }
            if (QMetaType::type(prop.type) == -1)
                qWarning("QAxBase: Unsupported property type: %s", prop.type.data());
            break;
        default:
            prop.typeId |= vartype << 24;
            break;
        }
    }

    inline bool hasProperty(const QByteArray &name)
    {
        return property_list.contains(name);
    }

    inline QByteArray propertyType(const QByteArray &name)
    {
        return property_list.value(name).type;
    }

    QMap<QByteArray, QList<QPair<QByteArray, int> > > enum_list;
    inline void addEnumValue(const QByteArray &enumname, const QByteArray &key, int value)
    {
        enum_list[enumname].append(QPair<QByteArray, int>(key, value));
    }

    inline bool hasEnum(const QByteArray &enumname)
    {
        return enum_list.contains(enumname);
    }

    QAxBase *that;
    QAxBasePrivate *d;

    IDispatch *disp;
    ITypeInfo *dispInfo;
    ITypeInfo *classInfo;
    ITypeLib *typelib;
    QByteArray current_typelib;

    QSettings iidnames;
    QString cacheKey;
    QByteArray debugInfo;

    QUuid iid_propNotifySink;

    friend QMetaObject *qax_readClassInfo(ITypeLib *typeLib, ITypeInfo *classInfo, const QMetaObject *parentObject);
};

QMetaObject *qax_readEnumInfo(ITypeLib *typeLib, const QMetaObject *parentObject)
{
    MetaObjectGenerator generator(typeLib, 0);

    generator.readEnumInfo();
    return generator.metaObject(parentObject, "EnumInfo");
}

QMetaObject *qax_readInterfaceInfo(ITypeLib *typeLib, ITypeInfo *typeInfo, const QMetaObject *parentObject)
{
    MetaObjectGenerator generator(typeLib, typeInfo);

    QString className;
    BSTR bstr;
    if (S_OK != typeInfo->GetDocumentation(-1, &bstr, 0, 0, 0))
        return 0;

    className = QString::fromWCharArray(bstr);
    SysFreeString(bstr);

    generator.readEnumInfo();
    generator.readFuncsInfo(typeInfo, 0);
    generator.readVarsInfo(typeInfo, 0);

    return generator.metaObject(parentObject, className.toLatin1());
}

QMetaObject *qax_readClassInfo(ITypeLib *typeLib, ITypeInfo *classInfo, const QMetaObject *parentObject)
{
    MetaObjectGenerator generator(typeLib, 0);
    generator.addSignal("exception(int,QString,QString,QString)", "code,source,disc,help");
    generator.addSignal("propertyChanged(QString)", "name");

    QString className;
    BSTR bstr;
    if (S_OK != classInfo->GetDocumentation(-1, &bstr, 0, 0, 0))
        return 0;

    className = QString::fromWCharArray(bstr);
    SysFreeString(bstr);

    generator.readEnumInfo();

    TYPEATTR *typeattr;
    classInfo->GetTypeAttr(&typeattr);
    if (typeattr) {
        int nInterfaces = typeattr->cImplTypes;
        classInfo->ReleaseTypeAttr(typeattr);

        for (int index = 0; index < nInterfaces; ++index) {
            HREFTYPE refType;
            if (S_OK != classInfo->GetRefTypeOfImplType(index, &refType))
                continue;

            int flags = 0;
            classInfo->GetImplTypeFlags(index, &flags);
            if (flags & IMPLTYPEFLAG_FRESTRICTED)
                continue;

            ITypeInfo *interfaceInfo = 0;
            classInfo->GetRefTypeInfo(refType, &interfaceInfo);
            if (!interfaceInfo)
                continue;

            interfaceInfo->GetDocumentation(-1, &bstr, 0, 0, 0);
            QString interfaceName = QString::fromWCharArray(bstr);
            SysFreeString(bstr);
            QByteArray key;

            TYPEATTR *typeattr = 0;
            interfaceInfo->GetTypeAttr(&typeattr);

            if (flags & IMPLTYPEFLAG_FSOURCE) {
                if (typeattr && !(typeattr->wTypeFlags & TYPEFLAG_FHIDDEN))
                    key = "Event Interface " + QByteArray::number(index);
                generator.readEventInterface(interfaceInfo, 0);
            } else {
                if (typeattr && !(typeattr->wTypeFlags & TYPEFLAG_FHIDDEN))
                    key = "Interface " + QByteArray::number(index);
                generator.readFuncsInfo(interfaceInfo, 0);
                generator.readVarsInfo(interfaceInfo, 0);
            }
            if (!key.isEmpty())
                generator.addClassInfo(key.data(), interfaceName.toLatin1());

            if (typeattr)
                interfaceInfo->ReleaseTypeAttr(typeattr);
            interfaceInfo->Release();
        }
    }

    return generator.metaObject(parentObject, className.toLatin1());
}

MetaObjectGenerator::MetaObjectGenerator(QAxBase *ax, QAxBasePrivate *dptr)
: that(ax), d(dptr), disp(0), dispInfo(0), classInfo(0), typelib(0),
  iidnames(QLatin1String("HKEY_LOCAL_MACHINE\\Software\\Classes"), QSettings::NativeFormat)
{
    init();
}

MetaObjectGenerator::MetaObjectGenerator(ITypeLib *tlib, ITypeInfo *tinfo)
: that(0), d(0), disp(0), dispInfo(tinfo), classInfo(0), typelib(tlib),
  iidnames(QLatin1String("HKEY_LOCAL_MACHINE\\Software\\Classes"), QSettings::NativeFormat)
{
    init();

    if (dispInfo)
        dispInfo->AddRef();
    if (typelib) {
        typelib->AddRef();
        BSTR bstr;
        typelib->GetDocumentation(-1, &bstr, 0, 0, 0);
        current_typelib = QString::fromWCharArray(bstr).toLatin1();
        SysFreeString(bstr);
    }
    readClassInfo();
}

void MetaObjectGenerator::init()
{
    if (d)
        disp = d->dispatch();

    iid_propNotifySink = IID_IPropertyNotifySink;

    addSignal("signal(QString,int,void*)", "name,argc,argv");
    addSignal("exception(int,QString,QString,QString)", "code,source,disc,help");
    addSignal("propertyChanged(QString)", "name");
    if (d || dispInfo) {
        addProperty("QString", "control", Readable|Writable|Designable|Scriptable|Stored|Editable|StdCppSet);
    }
}

MetaObjectGenerator::~MetaObjectGenerator()
{
    if (dispInfo) dispInfo->Release();
    if (classInfo) classInfo->Release();
    if (typelib) typelib->Release();
}

bool qax_dispatchEqualsIDispatch = true;
QList<QByteArray> qax_qualified_usertypes;

QByteArray MetaObjectGenerator::usertypeToString(const TYPEDESC &tdesc, ITypeInfo *info, const QByteArray &function)
{
    HREFTYPE usertype = tdesc.hreftype;
    if (tdesc.vt != VT_USERDEFINED)
        return 0;

    QByteArray typeName;
    ITypeInfo *usertypeinfo = 0;
    info->GetRefTypeInfo(usertype, &usertypeinfo);
    if (usertypeinfo) {
        ITypeLib *usertypelib = 0;
        UINT index;
        usertypeinfo->GetContainingTypeLib(&usertypelib, &index);
        if (usertypelib) {
            // get type library name
            BSTR typelibname = 0;
            usertypelib->GetDocumentation(-1, &typelibname, 0, 0, 0);
            QByteArray typeLibName = QString::fromWCharArray(typelibname).toLatin1();
            SysFreeString(typelibname);

            // get type name
            BSTR usertypename = 0;
            usertypelib->GetDocumentation(index, &usertypename, 0, 0, 0);
            QByteArray userTypeName = QString::fromWCharArray(usertypename).toLatin1();
            SysFreeString(usertypename);

            if (hasEnum(userTypeName)) // known enum?
                typeName = userTypeName;
            else if (userTypeName == "OLE_COLOR" || userTypeName == "VB_OLE_COLOR")
                typeName = "QColor";
            else if (userTypeName == "IFontDisp" || userTypeName == "IFontDisp*" || userTypeName == "IFont" || userTypeName == "IFont*")
                typeName = "QFont";
            else if (userTypeName == "Picture" || userTypeName == "Picture*")
                typeName = "QPixmap";

            if (typeName.isEmpty()) {
                TYPEATTR *typeattr = 0;
                usertypeinfo->GetTypeAttr(&typeattr);
                if (typeattr) {
                    switch(typeattr->typekind) {
                    case TKIND_ALIAS:
                        userTypeName = guessTypes(typeattr->tdescAlias, usertypeinfo, function);
                        break;
                    case TKIND_DISPATCH:
                    case TKIND_COCLASS:
                        if (qax_dispatchEqualsIDispatch) {
                            userTypeName = "IDispatch";
                        } else {
                            if (typeLibName != current_typelib)
                                userTypeName = typeLibName + "::" + userTypeName;
                            if (!qax_qualified_usertypes.contains(userTypeName))
                                qax_qualified_usertypes << userTypeName;
                        }
                        break;
                    case TKIND_ENUM:
                        if (typeLibName != current_typelib)
                            userTypeName = typeLibName + "::" + userTypeName;
                        if (!qax_qualified_usertypes.contains("enum " + userTypeName))
                            qax_qualified_usertypes << "enum " + userTypeName;
                        break;
                    case TKIND_INTERFACE:
                        if (typeLibName != current_typelib)
                            userTypeName = typeLibName + "::" + userTypeName;
                        if (!qax_qualified_usertypes.contains(userTypeName))
                            qax_qualified_usertypes << userTypeName;
                        break;
                    case TKIND_RECORD:
                        if (!qax_qualified_usertypes.contains("struct " + userTypeName))
                            qax_qualified_usertypes << "struct "+ userTypeName;
                        break;
                    default:
                        break;
                    }
                }

                usertypeinfo->ReleaseTypeAttr(typeattr);
                typeName = userTypeName;
            }
            usertypelib->Release();
        }
        usertypeinfo->Release();
    }

    return typeName;
}

#define VT_UNHANDLED(x) case VT_##x: qWarning("QAxBase: Unhandled type %s", #x); str = #x; break;

QByteArray MetaObjectGenerator::guessTypes(const TYPEDESC &tdesc, ITypeInfo *info, const QByteArray &function)
{
    QByteArray str;
    switch (tdesc.vt) {
    case VT_EMPTY:
    case VT_VOID:
        break;
    case VT_LPWSTR:
        str = "wchar_t *";
        break;
    case VT_BSTR:
        str = "QString";
        break;
    case VT_BOOL:
        str = "bool";
        break;
    case VT_I1:
        str = "char";
        break;
    case VT_I2:
        str = "short";
        break;
    case VT_I4:
    case VT_INT:
        str = "int";
        break;
    case VT_I8:
        str = "qint64";
        break;
    case VT_UI1:
    case VT_UI2:
    case VT_UI4:
    case VT_UINT:
        str = "uint";
        break;
    case VT_UI8:
        str = "quint64";
        break;
    case VT_CY:
        str = "qint64";
        break;
    case VT_R4:
        str = "float";
        break;
    case VT_R8:
        str = "double";
        break;
    case VT_DATE:
        str = "QDateTime";
        break;
    case VT_DISPATCH:
        str = "IDispatch*";
        break;
    case VT_VARIANT:
        str = "QVariant";
        break;
    case VT_UNKNOWN:
        str = "IUnknown*";
        break;
    case VT_HRESULT:
        str = "HRESULT";
        break;
    case VT_PTR:
        str = guessTypes(*tdesc.lptdesc, info, function);
        switch(tdesc.lptdesc->vt) {
        case VT_VOID:
            str = "void*";
            break;
        case VT_VARIANT:
        case VT_BSTR:
        case VT_I1:
        case VT_I2:
        case VT_I4:
        case VT_I8:
        case VT_UI1:
        case VT_UI2:
        case VT_UI4:
        case VT_UI8:
        case VT_BOOL:
        case VT_R4:
        case VT_R8:
        case VT_INT:
        case VT_UINT:
        case VT_CY:
            str += '&';
            break;
        case VT_PTR:
            if (str == "QFont" || str == "QPixmap") {
                str += '&';
                break;
            } else if (str == "void*") {
                str = "void **";
                break;
            }
            // FALLTHROUGH
        default:
            if (str == "QColor")
                str += '&';
            else if (str == "QDateTime")
                str += '&';
            else if (str == "QVariantList")
                str += '&';
            else if (str == "QByteArray")
                str += '&';
            else if (str == "QStringList")
                str += '&';
            else if (!str.isEmpty() && hasEnum(str))
                str += '&';
            else if (!str.isEmpty() && str != "QFont" && str != "QPixmap" && str != "QVariant")
                str += '*';
        }
        break;
    case VT_SAFEARRAY:
        switch(tdesc.lpadesc->tdescElem.vt) {
        // some shortcuts, and generic support for lists of QVariant-supported types
        case VT_UI1:
            str = "QByteArray";
            break;
        case VT_BSTR:
            str = "QStringList";
            break;
        case VT_VARIANT:
            str = "QVariantList";
            break;
        default:
            str = guessTypes(tdesc.lpadesc->tdescElem, info, function);
            if (!str.isEmpty())
                str = "QList<" + str + '>';
            break;
        }
        break;
    case VT_CARRAY:
        str = guessTypes(tdesc.lpadesc->tdescElem, info, function);
        if (!str.isEmpty()) {
            for (int index = 0; index < tdesc.lpadesc->cDims; ++index)
                str += '[' + QByteArray::number((int)tdesc.lpadesc->rgbounds[index].cElements) + ']';
        }
        break;
    case VT_USERDEFINED:
        str = usertypeToString(tdesc, info, function);
        break;

    VT_UNHANDLED(FILETIME);
    VT_UNHANDLED(BLOB);
    VT_UNHANDLED(ERROR);
    VT_UNHANDLED(DECIMAL);
    VT_UNHANDLED(LPSTR);
    default:
        break;
    }

    if (tdesc.vt & VT_BYREF)
        str += '&';

    str.replace("&*", "**");
    return str;
}

void MetaObjectGenerator::readClassInfo()
{
    // Read class information
    IProvideClassInfo *provideClassInfo = 0;
    if (d)
        d->ptr->QueryInterface(IID_IProvideClassInfo, (void**)&provideClassInfo);
    if (provideClassInfo) {
        provideClassInfo->GetClassInfo(&classInfo);
        TYPEATTR *typeattr = 0;
        if (classInfo)
            classInfo->GetTypeAttr(&typeattr);

        QString coClassID;
        if (typeattr) {
            QUuid clsid(typeattr->guid);
            coClassID = clsid.toString().toUpper();
#ifndef QAX_NO_CLASSINFO
            // UUID
            if (d->useClassInfo && !hasClassInfo("CoClass")) {
                QString coClassIDstr = iidnames.value(QLatin1String("/CLSID/") + coClassID + QLatin1String("/Default"), coClassID).toString();
                addClassInfo("CoClass", coClassIDstr.isEmpty() ? coClassID.toLatin1() : coClassIDstr.toLatin1());
                QByteArray version = QByteArray::number(typeattr->wMajorVerNum) + '.' + QByteArray::number(typeattr->wMinorVerNum);
                if (version != "0.0")
                    addClassInfo("Version", version);
            }
#endif
            classInfo->ReleaseTypeAttr(typeattr);
        }
        provideClassInfo->Release();
        provideClassInfo = 0;

        if (d->tryCache && !coClassID.isEmpty())
            cacheKey = QString::fromLatin1("%1$%2$%3$%4").arg(coClassID)
                .arg((int)d->useEventSink).arg((int)d->useClassInfo).arg((int)qax_dispatchEqualsIDispatch);
    }

    UINT index = 0;
    if (disp && !dispInfo)
        disp->GetTypeInfo(index, LOCALE_USER_DEFAULT, &dispInfo);

    if (dispInfo && !typelib)
        dispInfo->GetContainingTypeLib(&typelib, &index);

    if (!typelib) {
        QSettings controls(QLatin1String("HKEY_LOCAL_MACHINE\\Software"), QSettings::NativeFormat);
        QString tlid = controls.value(QLatin1String("/Classes/CLSID/") + that->control() + QLatin1String("/TypeLib/.")).toString();
        QString tlfile;
        if (!tlid.isEmpty()) {
            controls.beginGroup(QLatin1String("/Classes/TypeLib/") + tlid);
            QStringList versions = controls.childGroups();
            QStringList::Iterator vit = versions.begin();
            while (tlfile.isEmpty() && vit != versions.end()) {
                QString version = *vit;
                ++vit;
                tlfile = controls.value(QLatin1Char('/') + version + QLatin1String("/0/win32/.")).toString();
            }
            controls.endGroup();
        } else {
            tlfile = controls.value(QLatin1String("/Classes/CLSID/") + that->control() + QLatin1String("/InprocServer32/.")).toString();
            if (tlfile.isEmpty())
                tlfile = controls.value(QLatin1String("/Classes/CLSID/") + that->control() + QLatin1String("/LocalServer32/.")).toString();
        }
        if (!tlfile.isEmpty()) {
            LoadTypeLib((OLECHAR*)tlfile.utf16(), &typelib);
            if (!typelib) {
                tlfile = tlfile.left(tlfile.lastIndexOf(QLatin1Char('.'))) + QLatin1String(".tlb");
                LoadTypeLib((OLECHAR*)tlfile.utf16(), &typelib);
            }
            if (!typelib) {
                tlfile = tlfile.left(tlfile.lastIndexOf(QLatin1Char('.'))) + QLatin1String(".olb");
                LoadTypeLib((OLECHAR*)tlfile.utf16(), &typelib);
            }
        }
    }

    if (!classInfo && typelib && that)
        typelib->GetTypeInfoOfGuid(QUuid(that->control()), &classInfo);

    if (classInfo && !dispInfo) {
        TYPEATTR *classAttr;
        classInfo->GetTypeAttr(&classAttr);
        if (classAttr) {
            for (int i = 0; i < classAttr->cImplTypes; ++i) {
                int typeFlags = 0;
                classInfo->GetImplTypeFlags(i, &typeFlags);
                if (typeFlags & IMPLTYPEFLAG_FSOURCE)
                    continue;

                HREFTYPE hrefType;
                if (S_OK == classInfo->GetRefTypeOfImplType(i, &hrefType))
                    classInfo->GetRefTypeInfo(hrefType, &dispInfo);
                if (dispInfo) {
                    TYPEATTR *ifaceAttr;
                    dispInfo->GetTypeAttr(&ifaceAttr);
                    WORD typekind = ifaceAttr->typekind;
                    dispInfo->ReleaseTypeAttr(ifaceAttr);

                    if (typekind & TKIND_DISPATCH) {
                        break;
                    } else {
                        dispInfo->Release();
                        dispInfo = 0;
                    }
                }
            }
            classInfo->ReleaseTypeAttr(classAttr);
        }
    }

    if (!d || !dispInfo || !cacheKey.isEmpty() || !d->tryCache)
        return;

    TYPEATTR *typeattr = 0;
    dispInfo->GetTypeAttr(&typeattr);

    QString interfaceID;
    if (typeattr) {
        QUuid iid(typeattr->guid);
        interfaceID = iid.toString().toUpper();

        dispInfo->ReleaseTypeAttr(typeattr);
        // ### event interfaces!!
        if (!interfaceID.isEmpty())
            cacheKey = QString::fromLatin1("%1$%2$%3$%4").arg(interfaceID)
                .arg((int)d->useEventSink).arg((int)d->useClassInfo).arg((int)qax_dispatchEqualsIDispatch);
    }
}

void MetaObjectGenerator::readEnumInfo()
{
    if (!typelib)
        return;

    QUuid libUuid;

    if (d && d->tryCache) {
        TLIBATTR *libAttr = 0;
        typelib->GetLibAttr(&libAttr);
        if (libAttr) {
            libUuid = QUuid(libAttr->guid);
            typelib->ReleaseTLibAttr(libAttr);
            enum_list = enum_cache.value(libUuid);
            if (!enum_list.isEmpty())
                return;
        }
    }

    int valueindex = 0;
    QSet<QString> clashCheck;
    int clashIndex = 0;

    int enum_serial = 0;
    UINT index = typelib->GetTypeInfoCount();
    for (UINT i = 0; i < index; ++i) {
        TYPEKIND typekind;
        typelib->GetTypeInfoType(i, &typekind);
        if (typekind == TKIND_ENUM) {
            // Get the type information for the enum
            ITypeInfo *enuminfo = 0;
            typelib->GetTypeInfo(i, &enuminfo);
            if (!enuminfo)
                continue;

            // Get the name of the enumeration
            BSTR enumname;
            QByteArray enumName;
            if (typelib->GetDocumentation(i, &enumname, 0, 0, 0) == S_OK) {
                enumName = QString::fromWCharArray(enumname).toLatin1();
                SysFreeString(enumname);
            } else {
                enumName = "enum" + QByteArray::number(++enum_serial);
            }

            // Get the attributes of the enum type
            TYPEATTR *typeattr = 0;
            enuminfo->GetTypeAttr(&typeattr);
            if (typeattr) {
                // Get all values of the enumeration
                for (UINT vd = 0; vd < (UINT)typeattr->cVars; ++vd) {
                    VARDESC *vardesc = 0;
                    enuminfo->GetVarDesc(vd, &vardesc);
                    if (vardesc && vardesc->varkind == VAR_CONST) {
                        int value = vardesc->lpvarValue->lVal;
                        int memid = vardesc->memid;
                        // Get the name of the value
                        BSTR valuename;
                        QByteArray valueName;
                        UINT maxNamesOut;
                        enuminfo->GetNames(memid, &valuename, 1, &maxNamesOut);
                        if (maxNamesOut) {
                            valueName = QString::fromWCharArray(valuename).toLatin1();
                            SysFreeString(valuename);
                        } else {
                            valueName = "value" + QByteArray::number(valueindex++);
                        }

                        if (clashCheck.contains(QString::fromLatin1(valueName)))
                            valueName += QByteArray::number(++clashIndex);

                        clashCheck.insert(QString::fromLatin1(valueName));
                        addEnumValue(enumName, valueName, value);
                    }
                    enuminfo->ReleaseVarDesc(vardesc);
                }
            }
            enuminfo->ReleaseTypeAttr(typeattr);
            enuminfo->Release();
        }
    }

    if (!libUuid.isNull())
        enum_cache.insert(libUuid, enum_list);
}

void MetaObjectGenerator::addChangedSignal(const QByteArray &function, const QByteArray &type, int memid)
{
    QAxEventSink *eventSink = 0;
    if (d) {
        eventSink = d->eventSink.value(iid_propNotifySink);
        if (!eventSink && d->useEventSink) {
            eventSink = new QAxEventSink(that);
            d->eventSink.insert(iid_propNotifySink, eventSink);
        }
    }
    // generate changed signal
    QByteArray signalName(function);
    signalName += "Changed";
    QByteArray signalProto = signalName + '(' + replaceType(type) + ')';
    if (!hasSignal(signalProto))
        addSignal(signalProto, function);
    if (eventSink)
        eventSink->addProperty(memid, function, signalProto);
}

void MetaObjectGenerator::addSetterSlot(const QByteArray &property)
{
    QByteArray set;
    QByteArray prototype(property);
    if (isupper(prototype.at(0))) {
        set = "Set";
    } else {
        set = "set";
        prototype[0] = toupper(prototype[0]);
    }
    prototype = set + prototype + '(' + propertyType(property) + ')';
    if (!hasSlot(prototype))
        addSlot(0, prototype, property);
}

QByteArray MetaObjectGenerator::createPrototype(FUNCDESC *funcdesc, ITypeInfo *typeinfo, const QList<QByteArray> &names,
                                             QByteArray &type, QList<QByteArray> &parameters)
{
    QByteArray prototype;
    QByteArray function(names.at(0));
    const QByteArray hresult("HRESULT");
    // get function prototype
    type = guessTypes(funcdesc->elemdescFunc.tdesc, typeinfo, function);
    if ((type.isEmpty() || type == hresult) && funcdesc->invkind == INVOKE_PROPERTYPUT && funcdesc->lprgelemdescParam) {
        type = guessTypes(funcdesc->lprgelemdescParam->tdesc, typeinfo, function);
    }

    prototype = function + '(';
    if (funcdesc->invkind == INVOKE_FUNC && type == hresult)
        type = 0;

    int p;
    for (p = 1; p < names.count(); ++p) {
        // parameter
        QByteArray paramName = names.at(p);
        bool optional = p > (funcdesc->cParams - funcdesc->cParamsOpt);
        TYPEDESC tdesc = funcdesc->lprgelemdescParam[p-1].tdesc;
        PARAMDESC pdesc = funcdesc->lprgelemdescParam[p-1].paramdesc;

        QByteArray ptype = guessTypes(tdesc, typeinfo, function);
        if (pdesc.wParamFlags & PARAMFLAG_FRETVAL) {
            if (ptype.endsWith('&')) {
                ptype.truncate(ptype.length() - 1);
            } else if (ptype.endsWith("**")) {
                ptype.truncate(ptype.length() - 1);
            }
            type = ptype;
        } else {
            prototype += ptype;
            if (pdesc.wParamFlags & PARAMFLAG_FOUT && !ptype.endsWith('&') && !ptype.endsWith("**"))
                prototype += '&';
            if (optional || pdesc.wParamFlags & PARAMFLAG_FOPT)
                paramName += "=0";
            else if (pdesc.wParamFlags & PARAMFLAG_FHASDEFAULT) {
                // ### get the value from pdesc.pparamdescex
                paramName += "=0";
            }
            parameters << paramName;
        }
        if (p < funcdesc->cParams && !(pdesc.wParamFlags & PARAMFLAG_FRETVAL))
            prototype += ',';
    }

    if (!prototype.isEmpty()) {
        if (prototype.endsWith(',')) {
            if (funcdesc->invkind == INVOKE_PROPERTYPUT && p == funcdesc->cParams) {
                TYPEDESC tdesc = funcdesc->lprgelemdescParam[p-1].tdesc;
                QByteArray ptype = guessTypes(tdesc, typeinfo, function);
                prototype += ptype;
                prototype += ')';
                parameters << "rhs";
            } else {
                prototype[prototype.length()-1] = ')';
            }
        } else {
            prototype += ')';
        }
    }

    return prototype;
}

void MetaObjectGenerator::readFuncsInfo(ITypeInfo *typeinfo, ushort nFuncs)
{
    if (!nFuncs) {
        TYPEATTR *typeattr = 0;
        typeinfo->GetTypeAttr(&typeattr);
        if (typeattr) {
            nFuncs = typeattr->cFuncs;
            typeinfo->ReleaseTypeAttr(typeattr);
        }
    }

    // get information about all functions
    for (ushort fd = 0; fd < nFuncs ; ++fd) {
        FUNCDESC *funcdesc = 0;
        typeinfo->GetFuncDesc(fd, &funcdesc);
        if (!funcdesc)
            break;

        QByteArray function;
        QByteArray type;
        QByteArray prototype;
        QList<QByteArray> parameters;

        // parse function description
        BSTR bstrNames[256];
        UINT maxNames = 255;
        UINT maxNamesOut;
        typeinfo->GetNames(funcdesc->memid, (BSTR*)&bstrNames, maxNames, &maxNamesOut);
        QList<QByteArray> names;
        int p;
        for (p = 0; p < (int)maxNamesOut; ++p) {
            names << QString::fromWCharArray(bstrNames[p]).toLatin1();
            SysFreeString(bstrNames[p]);
        }

        // function name
        function = names.at(0);
        if ((maxNamesOut == 3 && function == "QueryInterface") ||
            (maxNamesOut == 1 && function == "AddRef") ||
            (maxNamesOut == 1 && function == "Release") ||
            (maxNamesOut == 9 && function == "Invoke") ||
            (maxNamesOut == 6 && function == "GetIDsOfNames") ||
            (maxNamesOut == 2 && function == "GetTypeInfoCount") ||
            (maxNamesOut == 4 && function == "GetTypeInfo")) {
            typeinfo->ReleaseFuncDesc(funcdesc);
            continue;
        }

        prototype = createPrototype(/*in*/ funcdesc, typeinfo, names, /*out*/type, parameters);

        // get type of function
        switch(funcdesc->invkind) {
        case INVOKE_PROPERTYGET: // property
        case INVOKE_PROPERTYPUT:
            if (funcdesc->cParams - funcdesc->cParamsOpt <= 1) {
                bool dontBreak = false;
                // getter with non-default-parameters -> fall through to function handling
                if (funcdesc->invkind == INVOKE_PROPERTYGET && parameters.count() && funcdesc->cParams - funcdesc->cParamsOpt) {
                    dontBreak = true;
                } else {
                    uint flags = Readable;
                    if (funcdesc->invkind != INVOKE_PROPERTYGET)
                        flags |= Writable;
                    if (!(funcdesc->wFuncFlags & (FUNCFLAG_FNONBROWSABLE | FUNCFLAG_FHIDDEN)))
                        flags |= Designable;
                    if (!(funcdesc->wFuncFlags & FUNCFLAG_FRESTRICTED))
                        flags |= Scriptable;
                    if (funcdesc->wFuncFlags & FUNCFLAG_FREQUESTEDIT)
                        flags |= RequestingEdit;
                    if (hasEnum(type))
                        flags |= EnumOrFlag;

                    if (funcdesc->wFuncFlags & FUNCFLAG_FBINDABLE && funcdesc->invkind == INVOKE_PROPERTYGET) {
                        addChangedSignal(function, type, funcdesc->memid);
                        flags |= Bindable;
                    }
                    // Don't generate code for properties without type
                    if (type.isEmpty())
                        break;
                    addProperty(type, function, flags);

                    // more parameters -> function handling
                    if (funcdesc->invkind == INVOKE_PROPERTYGET && funcdesc->cParams)
                        dontBreak = true;
                }

                if (!funcdesc->cParams) {
                    // don't generate slots for incomplete properties
                    if (type.isEmpty())
                        break;

                    // Done for getters
                    if (funcdesc->invkind == INVOKE_PROPERTYGET)
                        break;

                    // generate setter slot
                    if (funcdesc->invkind == INVOKE_PROPERTYPUT && hasProperty(function)) {
                        addSetterSlot(function);
                        break;
                    }
                } else if (funcdesc->invkind == INVOKE_PROPERTYPUT && hasProperty(function)) {
                    addSetterSlot(function);
                    // more parameters -> function handling
                    if (funcdesc->cParams > 1)
                        dontBreak = true;
                }
                if (!dontBreak)
                    break;
            }
            if (funcdesc->invkind == INVOKE_PROPERTYPUT) {
                // remove the typename guessed for property setters
                // its done only for setter's with more than one parameter.
                if (funcdesc->cParams - funcdesc->cParamsOpt > 1) {
                    type.clear();
                }
                QByteArray set;
                if (isupper(prototype.at(0))) {
                    set = "Set";
                } else {
                    set = "set";
                    prototype[0] = toupper(prototype[0]);
                }

                prototype = set + prototype;
            }
            // FALL THROUGH to support multi-variat properties
        case INVOKE_FUNC: // method
            {
                bool cloned = false;
                bool defargs;
                do {
                    QByteArray pnames;
                    for (p = 0; p < parameters.count(); ++p) {
                        pnames += parameters.at(p);
                        if (p < parameters.count() - 1)
                            pnames += ',';
                    }
                    defargs = pnames.contains("=0");
                    int flags = QMetaMethod::Public;
                    if (cloned)
                        flags |= QMetaMethod::Cloned << 4;
                    cloned |= defargs;
                    addSlot(type, prototype, pnames.replace("=0", ""), flags);

                    if (defargs) {
                        parameters.takeLast();
                        int lastParam = prototype.lastIndexOf(',');
                        if (lastParam == -1)
                            lastParam = prototype.indexOf('(') + 1;
                        prototype.truncate(lastParam);
                        prototype += ')';
                    }
                } while (defargs);
            }
            break;

        default:
            break;
        }

        typeinfo->ReleaseFuncDesc(funcdesc);
    }
}

void MetaObjectGenerator::readVarsInfo(ITypeInfo *typeinfo, ushort nVars)
{
    if (!nVars) {
        TYPEATTR *typeattr = 0;
        typeinfo->GetTypeAttr(&typeattr);
        if (typeattr) {
            nVars = typeattr->cVars;
            typeinfo->ReleaseTypeAttr(typeattr);
        }
    }

    // get information about all variables
    for (ushort vd = 0; vd < nVars; ++vd) {
        VARDESC *vardesc;
        typeinfo->GetVarDesc(vd, &vardesc);
        if (!vardesc)
            break;

        // no use if it's not a dispatched variable
        if (vardesc->varkind != VAR_DISPATCH) {
            typeinfo->ReleaseVarDesc(vardesc);
            continue;
        }

        // get variable name
        BSTR bstrName;
        UINT maxNames = 1;
        UINT maxNamesOut;
        typeinfo->GetNames(vardesc->memid, &bstrName, maxNames, &maxNamesOut);
        if (maxNamesOut != 1 || !bstrName) {
            typeinfo->ReleaseVarDesc(vardesc);
            continue;
        }
        QByteArray variableType;
        QByteArray variableName;
        uint flags = 0;

        variableName = QString::fromWCharArray(bstrName).toLatin1();
        SysFreeString(bstrName);

        // get variable type
        TYPEDESC typedesc = vardesc->elemdescVar.tdesc;
        variableType = guessTypes(typedesc, typeinfo, variableName);

        // generate meta property
        if (!hasProperty(variableName)) {
            flags = Readable;
            if (!(vardesc->wVarFlags & VARFLAG_FREADONLY))
                flags |= Writable;
            if (!(vardesc->wVarFlags & (VARFLAG_FNONBROWSABLE | VARFLAG_FHIDDEN)))
                flags |= Designable;
            if (!(vardesc->wVarFlags & VARFLAG_FRESTRICTED))
                flags |= Scriptable;
            if (vardesc->wVarFlags & VARFLAG_FREQUESTEDIT)
                flags |= RequestingEdit;
            if (hasEnum(variableType))
                flags |= EnumOrFlag;

            if (vardesc->wVarFlags & VARFLAG_FBINDABLE) {
                addChangedSignal(variableName, variableType, vardesc->memid);
                flags |= Bindable;
            }
            addProperty(variableType, variableName, flags);
        }

        // generate a set slot
        if (!(vardesc->wVarFlags & VARFLAG_FREADONLY))
            addSetterSlot(variableName);

        typeinfo->ReleaseVarDesc(vardesc);
    }
}

void MetaObjectGenerator::readInterfaceInfo()
{
    ITypeInfo *typeinfo = dispInfo;
    if (!typeinfo)
        return;
    typeinfo->AddRef();
    int interface_serial = 0;
    while (typeinfo) {
        ushort nFuncs = 0;
        ushort nVars = 0;
        ushort nImpl = 0;
        // get information about type
        TYPEATTR *typeattr;
        typeinfo->GetTypeAttr(&typeattr);
        bool interesting = true;
        if (typeattr) {
            // get number of functions, variables, and implemented interfaces
            nFuncs = typeattr->cFuncs;
            nVars = typeattr->cVars;
            nImpl = typeattr->cImplTypes;

            if ((typeattr->typekind == TKIND_DISPATCH || typeattr->typekind == TKIND_INTERFACE) &&
                (typeattr->guid != IID_IDispatch && typeattr->guid != IID_IUnknown)) {
#ifndef QAX_NO_CLASSINFO
                if (d && d->useClassInfo) {
                    // UUID
                    QUuid uuid(typeattr->guid);
                    QString uuidstr = uuid.toString().toUpper();
                    uuidstr = iidnames.value(QLatin1String("/Interface/") + uuidstr + QLatin1String("/Default"), uuidstr).toString();
                    addClassInfo("Interface " + QByteArray::number(++interface_serial), uuidstr.toLatin1());
                }
#endif
                typeinfo->ReleaseTypeAttr(typeattr);
            } else {
                interesting = false;
                typeinfo->ReleaseTypeAttr(typeattr);
            }
        }

        if (interesting) {
            readFuncsInfo(typeinfo, nFuncs);
            readVarsInfo(typeinfo, nVars);
        }

        if (!nImpl) {
            typeinfo->Release();
            typeinfo = 0;
            break;
        }

        // go up one base class
        HREFTYPE pRefType;
        typeinfo->GetRefTypeOfImplType(0, &pRefType);
        ITypeInfo *baseInfo = 0;
        typeinfo->GetRefTypeInfo(pRefType, &baseInfo);
        typeinfo->Release();
        if (typeinfo == baseInfo) { // IUnknown inherits IUnknown ???
            baseInfo->Release();
            typeinfo = 0;
            break;
        }
        typeinfo = baseInfo;
    }
}

void MetaObjectGenerator::readEventInterface(ITypeInfo *eventinfo, IConnectionPoint *cpoint)
{
    TYPEATTR *eventattr;
    eventinfo->GetTypeAttr(&eventattr);
    if (!eventattr)
        return;
    if (eventattr->typekind != TKIND_DISPATCH) {
        eventinfo->ReleaseTypeAttr(eventattr);
        return;
    }

    QAxEventSink *eventSink = 0;
    if (d) {
        IID conniid;
        cpoint->GetConnectionInterface(&conniid);
        eventSink = d->eventSink.value(QUuid(conniid));
        if (!eventSink) {
            eventSink = new QAxEventSink(that);
            d->eventSink.insert(QUuid(conniid), eventSink);
            eventSink->advise(cpoint, conniid);
        }
    }

    // get information about all event functions
    for (UINT fd = 0; fd < (UINT)eventattr->cFuncs; ++fd) {
        FUNCDESC *funcdesc;
        eventinfo->GetFuncDesc(fd, &funcdesc);
        if (!funcdesc)
            break;
        if (funcdesc->invkind != INVOKE_FUNC ||
            funcdesc->funckind != FUNC_DISPATCH) {
            eventinfo->ReleaseTypeAttr(eventattr);
            eventinfo->ReleaseFuncDesc(funcdesc);
            continue;
        }

        QByteArray function;
        QByteArray prototype;
        QList<QByteArray> parameters;

        // parse event function description
        BSTR bstrNames[256];
        UINT maxNames = 255;
        UINT maxNamesOut;
        eventinfo->GetNames(funcdesc->memid, (BSTR*)&bstrNames, maxNames, &maxNamesOut);
        QList<QByteArray> names;
        int p;
        for (p = 0; p < (int)maxNamesOut; ++p) {
            names << QString::fromWCharArray(bstrNames[p]).toLatin1();
            SysFreeString(bstrNames[p]);
        }

        // get event function prototype
        function = names.at(0);
        QByteArray type; // dummy - we don't care about return values for signals
        prototype = createPrototype(/*in*/ funcdesc, eventinfo, names, /*out*/type, parameters);
        if (!hasSignal(prototype)) {
            QByteArray pnames;
            for (p = 0; p < parameters.count(); ++p) {
                pnames += parameters.at(p);
                if (p < parameters.count() - 1)
                    pnames += ',';
            }
            addSignal(prototype, pnames);
        }
        if (eventSink)
            eventSink->addSignal(funcdesc->memid, prototype);

        eventinfo->ReleaseFuncDesc(funcdesc);
    }
    eventinfo->ReleaseTypeAttr(eventattr);
}

void MetaObjectGenerator::readEventInfo()
{
    int event_serial = 0;
    IConnectionPointContainer *cpoints = 0;
    if (d && d->useEventSink)
        d->ptr->QueryInterface(IID_IConnectionPointContainer, (void**)&cpoints);
    if (cpoints) {
        // Get connection point enumerator
        IEnumConnectionPoints *epoints = 0;
        cpoints->EnumConnectionPoints(&epoints);
        if (epoints) {
            ULONG c = 1;
            IConnectionPoint *cpoint = 0;
            epoints->Reset();
            QList<QUuid> cpointlist;
            do {
                if (cpoint) cpoint->Release();
                cpoint = 0;
                HRESULT hr = epoints->Next(c, &cpoint, &c);
                if (!c || hr != S_OK)
                    break;

                IID conniid;
                cpoint->GetConnectionInterface(&conniid);
                // workaround for typelibrary bug of Word.Application
                QUuid connuuid(conniid);
                if (cpointlist.contains(connuuid))
                    break;

#ifndef QAX_NO_CLASSINFO
                if (d->useClassInfo) {
                    QString uuidstr = connuuid.toString().toUpper();
                    uuidstr = iidnames.value(QLatin1String("/Interface/") + uuidstr + QLatin1String("/Default"), uuidstr).toString();
                    addClassInfo("Event Interface " + QByteArray::number(++event_serial), uuidstr.toLatin1());
                }
#endif

                // get information about type
                if (conniid == IID_IPropertyNotifySink) {
                    // test whether property notify sink has been created already, and advise on it
                    QAxEventSink *eventSink = d->eventSink.value(iid_propNotifySink);
                    if (eventSink)
                        eventSink->advise(cpoint, conniid);
                    continue;
                }

                ITypeInfo *eventinfo = 0;
                if (typelib)
                    typelib->GetTypeInfoOfGuid(conniid, &eventinfo);

                if (eventinfo) {
                    // avoid recursion (see workaround above)
                    cpointlist.append(connuuid);

                    readEventInterface(eventinfo, cpoint);
                    eventinfo->Release();
                }
            } while (c);
            if (cpoint) cpoint->Release();
            epoints->Release();
        } else if (classInfo) { // no enumeration - search source interfaces and ask for those
	    TYPEATTR *typeattr = 0;
	    classInfo->GetTypeAttr(&typeattr);
            if (typeattr) {
                for (int i = 0; i < typeattr->cImplTypes; ++i) {
                    int flags = 0;
                    classInfo->GetImplTypeFlags(i, &flags);
                    if (!(flags & IMPLTYPEFLAG_FSOURCE))
                        continue;
                    HREFTYPE reference;
                    if (S_OK != classInfo->GetRefTypeOfImplType(i, &reference))
                        continue;
                    ITypeInfo *eventInfo = 0;
                    classInfo->GetRefTypeInfo(reference, &eventInfo);
                    if (!eventInfo)
                        continue;
                    TYPEATTR *eventattr = 0;
                    eventInfo->GetTypeAttr(&eventattr);
                    if (eventattr) {
                        IConnectionPoint *cpoint = 0;
                        cpoints->FindConnectionPoint(eventattr->guid, &cpoint);
                        if (cpoint) {
                            if (eventattr->guid == IID_IPropertyNotifySink) {
                                // test whether property notify sink has been created already, and advise on it
                                QAxEventSink *eventSink = d->eventSink.value(iid_propNotifySink);
                                if (eventSink)
                                    eventSink->advise(cpoint, eventattr->guid);
                                continue;
                            }

                            readEventInterface(eventInfo, cpoint);
                            cpoint->Release();
                        }
                        eventInfo->ReleaseTypeAttr(eventattr);
                    }
                    eventInfo->Release();
                }
                classInfo->ReleaseTypeAttr(typeattr);
            }
        }
        cpoints->Release();
    }
}

QMetaObject *MetaObjectGenerator::tryCache()
{
    if (!cacheKey.isEmpty()) {
        d->metaobj = mo_cache.value(cacheKey);
        if (d->metaobj) {
            d->cachedMetaObject = true;

            IConnectionPointContainer *cpoints = 0;
            d->ptr->QueryInterface(IID_IConnectionPointContainer, (void**)&cpoints);
            if (cpoints) {
                QList<QUuid>::ConstIterator it = d->metaobj->connectionInterfaces.begin();
                while (it != d->metaobj->connectionInterfaces.end()) {
                    QUuid iid = *it;
                    ++it;

                    IConnectionPoint *cpoint = 0;
                    cpoints->FindConnectionPoint(iid, &cpoint);
                    if (cpoint) {
                        QAxEventSink *sink = new QAxEventSink(that);
                        sink->advise(cpoint, iid);
                        d->eventSink.insert(iid, sink);
                        sink->sigs = d->metaobj->sigs.value(iid);
                        sink->props = d->metaobj->props.value(iid);
                        sink->propsigs = d->metaobj->propsigs.value(iid);
                        cpoint->Release();
                    }
                }
                cpoints->Release();
            }

            return d->metaobj;
        }
    }
    return 0;
}

QMetaObject *MetaObjectGenerator::metaObject(const QMetaObject *parentObject, const QByteArray &className)
{
    if (that) {
        readClassInfo();
        if (typelib) {
            BSTR bstr;
            typelib->GetDocumentation(-1, &bstr, 0, 0, 0);
            current_typelib = QString::fromWCharArray(bstr).toLatin1();
            SysFreeString(bstr);
        }
        if (d->tryCache && tryCache())
            return d->metaobj;
        readEnumInfo();
        readInterfaceInfo();
        readEventInfo();
    }

    current_typelib = QByteArray();

#ifndef QAX_NO_CLASSINFO
    if (!debugInfo.isEmpty() && d->useClassInfo)
        addClassInfo("debugInfo", debugInfo);
#endif

    QAxMetaObject *metaobj = new QAxMetaObject;

    // revision + classname + table + zero terminator
    uint int_data_size = 1+1+2+2+2+2+1;

    int_data_size += classinfo_list.count() * 2;
    int_data_size += signal_list.count() * 5;
    int_data_size += slot_list.count() * 5;
    int_data_size += property_list.count() * 3;
    int_data_size += enum_list.count() * 4;
    for (QMap<QByteArray, QList<QPair<QByteArray, int> > >::ConstIterator it = enum_list.begin();
    it != enum_list.end(); ++it) {
        int_data_size += (*it).count() * 2;
    }

    uint *int_data = new uint[int_data_size];
    int_data[0] = 1; // revision number
    int_data[1] = 0; // classname index
    int_data[2] = classinfo_list.count(); // num_classinfo
    int_data[3] = 10; // idx_classinfo
    int_data[4] = signal_list.count() + slot_list.count(); // num_methods
    int_data[5] = int_data[3] + int_data[2] * 2; // idx_signals
    int_data[6] = property_list.count(); // num_properties
    int_data[7] = int_data[5] + int_data[4] * 5; // idx_properties
    int_data[8] = enum_list.count(); // num_enums
    int_data[9] = int_data[7] + int_data[6] * 3; // idx_enums
    int_data[int_data_size - 1] = 0; // eod;

    char null('\0');
    // data + zero-terminator
    QByteArray stringdata = that ? QByteArray(that->className()) : className;
    stringdata += null;
    stringdata.reserve(8192);

    uint offset = int_data[3]; //idx_classinfo

    // each class info in form key\0value\0
    for (QMap<QByteArray, QByteArray>::ConstIterator it = classinfo_list.begin(); it != classinfo_list.end(); ++it) {
        QByteArray key(it.key());
        QByteArray value(it.value());
        int_data[offset++] = stringdata.length();
        stringdata += key;
        stringdata += null;
        int_data[offset++] = stringdata.length();
        stringdata += value;
        stringdata += null;
    }
    Q_ASSERT(offset == int_data[5]);

    // each signal in form prototype\0parameters\0type\0tag\0
    for (QMap<QByteArray, Method>::ConstIterator it = signal_list.begin(); it != signal_list.end(); ++it) {
        QByteArray prototype(QMetaObject::normalizedSignature(it.key()));
        QByteArray type(it.value().type);
        QByteArray parameters(it.value().parameters);
        if (!it.value().realPrototype.isEmpty())
            metaobj->realPrototype.insert(prototype, it.value().realPrototype);
        QByteArray tag;
        int flags = it.value().flags;

        int_data[offset++] = stringdata.length();
        stringdata += prototype;
        stringdata += null;
        int_data[offset++] = stringdata.length();
        stringdata += parameters;
        stringdata += null;
        int_data[offset++] = stringdata.length();
        stringdata += type;
        stringdata += null;
        int_data[offset++] = stringdata.length();
        stringdata += tag;
        stringdata += null;
        int_data[offset++] = flags;
    }

    // each slot in form prototype\0parameters\0type\0tag\0
    for (QMap<QByteArray, Method>::ConstIterator it = slot_list.begin(); it != slot_list.end(); ++it) {
        QByteArray prototype(QMetaObject::normalizedSignature(it.key()));
        QByteArray type(it.value().type);
        QByteArray parameters(it.value().parameters);
        if (!it.value().realPrototype.isEmpty())
            metaobj->realPrototype.insert(prototype, it.value().realPrototype);
        QByteArray tag;
        int flags = it.value().flags;

        int_data[offset++] = stringdata.length();
        stringdata += prototype;
        stringdata += null;
        int_data[offset++] = stringdata.length();
        stringdata += parameters;
        stringdata += null;
        int_data[offset++] = stringdata.length();
        stringdata += type;
        stringdata += null;
        int_data[offset++] = stringdata.length();
        stringdata += tag;
        stringdata += null;
        int_data[offset++] = flags;
    }
    Q_ASSERT(offset == int_data[7]);

    // each property in form name\0type\0
    for (QMap<QByteArray, Property>::ConstIterator it = property_list.begin(); it != property_list.end(); ++it) {
        QByteArray name(it.key());
        QByteArray type(it.value().type);
        QByteArray realType(it.value().realType);
        if (!realType.isEmpty() && realType != type)
            metaobj->realPrototype.insert(name, realType);
        uint flags = it.value().typeId;

        int_data[offset++] = stringdata.length();
        stringdata += name;
        stringdata += null;
        int_data[offset++] = stringdata.length();
        stringdata += type;
        stringdata += null;
        int_data[offset++] = flags;
    }
    Q_ASSERT(offset == int_data[9]);

    int value_offset = offset + enum_list.count() * 4;
    // each enum in form name\0
    for (QMap<QByteArray, QList<QPair<QByteArray, int> > >::ConstIterator it = enum_list.begin(); it != enum_list.end(); ++it) {
        QByteArray name(it.key());
        int flags = 0x0; // 0x1 for flag?
        int count = it.value().count();

        int_data[offset++] = stringdata.length();
        stringdata += name;
        stringdata += null;
        int_data[offset++] = flags;
        int_data[offset++] = count;
        int_data[offset++] = value_offset;
        value_offset += count * 2;
    }
    Q_ASSERT(offset == int_data[9] + enum_list.count() * 4);

    // each enum value in form key\0
    for (QMap<QByteArray, QList<QPair<QByteArray, int> > >::ConstIterator it = enum_list.begin(); it != enum_list.end(); ++it) {
        for (QList<QPair<QByteArray,int> >::ConstIterator it2 = it.value().begin(); it2 != it.value().end(); ++it2) {
            QByteArray key((*it2).first);
            int value = (*it2).second;
            int_data[offset++] = stringdata.length();
            stringdata += key;
            stringdata += null;
            int_data[offset++] = value;
        }
    }
    Q_ASSERT(offset == int_data_size-1);

    char *string_data = new char[stringdata.length()];
    memcpy(string_data, stringdata, stringdata.length());

    // put the metaobject together
    metaobj->d.data = int_data;
    metaobj->d.extradata = 0;
    metaobj->d.stringdata = string_data;
    metaobj->d.superdata = parentObject;

    if (d)
        d->metaobj = metaobj;

    if (!cacheKey.isEmpty()) {
        mo_cache.insert(cacheKey, d->metaobj);
        d->cachedMetaObject = true;
        for (QHash<QUuid, QAxEventSink*>::Iterator it = d->eventSink.begin(); it != d->eventSink.end(); ++it) {
            QAxEventSink *sink = it.value();
            if (sink) {
                QUuid ciid = sink->connectionInterface();

                d->metaobj->connectionInterfaces.append(ciid);
                d->metaobj->sigs.insert(ciid, sink->signalMap());
                d->metaobj->props.insert(ciid, sink->propertyMap());
                d->metaobj->propsigs.insert(ciid, sink->propSignalMap());
            }
        }
    }

    return metaobj;
}

static const uint qt_meta_data_QAxBase[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   10, // methods
       1,   25, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      24,    9,    8,    8, 0x05,
      55,   50,    8,    8, 0x05,
     102,   80,    8,    8, 0x05,

 // properties: name, type, flags
     149,  141, 0x0a095103,

       0        // eod
};

static const char qt_meta_stringdata_QAxBase[] = {
    "QAxBase\0\0name,argc,argv\0signal(QString,int,void*)\0name\0"
    "propertyChanged(QString)\0code,source,desc,help\0"
    "exception(int,QString,QString,QString)\0QString\0control\0"
};

static QMetaObject qaxobject_staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QAxBase,
        qt_meta_data_QAxBase, 0 }
};
static QMetaObject qaxwidget_staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QAxBase,
        qt_meta_data_QAxBase, 0 }
};

/*!
    \internal

    The metaobject is generated on the fly from the information
    provided by the IDispatch and ITypeInfo interface implementations
    in the COM object.
*/
const QMetaObject *QAxBase::metaObject() const
{
    if (d->metaobj)
        return d->metaobj;
    const QMetaObject* parentObject = parentMetaObject();

    if (!d->ptr && !d->initialized) {
        ((QAxBase*)this)->initialize(&d->ptr);
        d->initialized = true;
    }


    // only one thread at a time can generate meta objects
    QMutexLocker locker(&cache_mutex);

    // return the default meta object if not yet initialized
    if (!d->ptr || !d->useMetaObject) {
        if (qObject()->isWidgetType())
            return &qaxwidget_staticMetaObject;
        return &qaxobject_staticMetaObject;
    }
    MetaObjectGenerator generator((QAxBase*)this, d);
    return generator.metaObject(parentObject);
}

/*!
    \internal

    Connects to all event interfaces of the object.

    Called by the subclasses' connectNotify() reimplementations, so
    at this point the connection as actually been created already.
*/
void QAxBase::connectNotify()
{
    if (d->eventSink.count()) // already listening
        return;

    IEnumConnectionPoints *epoints = 0;
    if (d->ptr && d->useEventSink) {
        IConnectionPointContainer *cpoints = 0;
        d->ptr->QueryInterface(IID_IConnectionPointContainer, (void**)&cpoints);
        if (!cpoints)
            return;

        cpoints->EnumConnectionPoints(&epoints);
        cpoints->Release();
    }

    if (!epoints)
        return;

    UINT index;
    IDispatch *disp = d->dispatch();
    ITypeInfo *typeinfo = 0;
    ITypeLib  *typelib = 0;
    if (disp)
        disp->GetTypeInfo(0, LOCALE_USER_DEFAULT, &typeinfo);
    if (typeinfo)
        typeinfo->GetContainingTypeLib(&typelib, &index);

    if (!typelib) {
        epoints->Release();
        return;
    }

    MetaObjectGenerator generator(this, d);
    bool haveEnumInfo = false;

    ULONG c = 1;
    IConnectionPoint *cpoint = 0;
    epoints->Reset();
    do {
        if (cpoint) cpoint->Release();
        cpoint = 0;
        epoints->Next(c, &cpoint, &c);
        if (!c || !cpoint)
            break;

        IID conniid;
        cpoint->GetConnectionInterface(&conniid);
        // workaround for typelibrary bug of Word.Application
        QString connuuid(QUuid(conniid).toString());
        if (d->eventSink.contains(connuuid))
            break;

        // Get ITypeInfo for source-interface, and skip if not supporting IDispatch
        ITypeInfo *eventinfo = 0;
        typelib->GetTypeInfoOfGuid(conniid, &eventinfo);
        if (eventinfo) {
            TYPEATTR *eventAttr;
            eventinfo->GetTypeAttr(&eventAttr);
            if (!eventAttr) {
                eventinfo->Release();
                break;
            }

            TYPEKIND eventKind = eventAttr->typekind;
            eventinfo->ReleaseTypeAttr(eventAttr);
            if (eventKind != TKIND_DISPATCH) {
                eventinfo->Release();
                break;
            }
        }

        // always into the cache to avoid recoursion
        QAxEventSink *eventSink = eventinfo ? new QAxEventSink(this) : 0;
        d->eventSink.insert(connuuid, eventSink);

        if (!eventinfo)
            continue;

        // have to get type info to support signals with enum parameters
        if (!haveEnumInfo) {
            bool wasTryCache = d->tryCache;
            d->tryCache = true;
            generator.readClassInfo();
            generator.readEnumInfo();
            d->tryCache = wasTryCache;
            haveEnumInfo = true;
        }
        generator.readEventInterface(eventinfo, cpoint);
        eventSink->advise(cpoint, conniid);

        eventinfo->Release();
    } while (c);
    if (cpoint) cpoint->Release();
    epoints->Release();

    typelib->Release();

    // make sure we don't try again
    if (!d->eventSink.count())
        d->eventSink.insert(QString(), 0);
}

/*!
    \fn QString QAxBase::generateDocumentation()

    Returns a rich text string with documentation for the
    wrapped COM object. Dump the string to an HTML-file,
    or use it in e.g. a QTextBrowser widget.
*/

static bool checkHRESULT(HRESULT hres, EXCEPINFO *exc, QAxBase *that, const QString &name, uint argerr)
{
    switch(hres) {
    case S_OK:
        return true;
    case DISP_E_BADPARAMCOUNT:
        qWarning("QAxBase: Error calling IDispatch member %s: Bad parameter count", name.toLatin1().data());
        return false;
    case DISP_E_BADVARTYPE:
        qWarning("QAxBase: Error calling IDispatch member %s: Bad variant type", name.toLatin1().data());
        return false;
    case DISP_E_EXCEPTION:
        {
            bool printWarning = true;
            unsigned short code = -1;
            QString source, desc, help;
            const QMetaObject *mo = that->metaObject();
            int exceptionSignal = mo->indexOfSignal("exception(int,QString,QString,QString)");
            if (exceptionSignal >= 0) {
                if (exc->pfnDeferredFillIn)
                    exc->pfnDeferredFillIn(exc);

                code = exc->wCode ? exc->wCode : exc->scode;
                source = QString::fromWCharArray(exc->bstrSource);
                desc = QString::fromWCharArray(exc->bstrDescription);
                help = QString::fromWCharArray(exc->bstrHelpFile);
                uint helpContext = exc->dwHelpContext;

                if (helpContext && !help.isEmpty())
                    help += QString::fromLatin1(" [%1]").arg(helpContext);

                if (QAxEventSink::signalHasReceivers(that->qObject(), "exception(int,QString,QString,QString)")) {
                    void *argv[] = {0, &code, &source, &desc, &help};
                    that->qt_metacall(QMetaObject::InvokeMetaMethod, exceptionSignal, argv);
                    printWarning = false;
                }
            }
            if (printWarning) {
                qWarning("QAxBase: Error calling IDispatch member %s: Exception thrown by server", name.toLatin1().data());
                qWarning("             Code       : %d", code);
                qWarning("             Source     : %s", source.toLatin1().data());
                qWarning("             Description: %s", desc.toLatin1().data());
                qWarning("             Help       : %s", help.toLatin1().data());
                qWarning("         Connect to the exception(int,QString,QString,QString) signal to catch this exception");
            }
        }
        return false;
    case DISP_E_MEMBERNOTFOUND:
        qWarning("QAxBase: Error calling IDispatch member %s: Member not found", name.toLatin1().data());
        return false;
    case DISP_E_NONAMEDARGS:
        qWarning("QAxBase: Error calling IDispatch member %s: No named arguments", name.toLatin1().data());
        return false;
    case DISP_E_OVERFLOW:
        qWarning("QAxBase: Error calling IDispatch member %s: Overflow", name.toLatin1().data());
        return false;
    case DISP_E_PARAMNOTFOUND:
        qWarning("QAxBase: Error calling IDispatch member %s: Parameter %d not found", name.toLatin1().data(), argerr);
        return false;
    case DISP_E_TYPEMISMATCH:
        qWarning("QAxBase: Error calling IDispatch member %s: Type mismatch in parameter %d", name.toLatin1().data(), argerr);
        return false;
    case DISP_E_UNKNOWNINTERFACE:
        qWarning("QAxBase: Error calling IDispatch member %s: Unknown interface", name.toLatin1().data());
        return false;
    case DISP_E_UNKNOWNLCID:
        qWarning("QAxBase: Error calling IDispatch member %s: Unknown locale ID", name.toLatin1().data());
        return false;
    case DISP_E_PARAMNOTOPTIONAL:
        qWarning("QAxBase: Error calling IDispatch member %s: Non-optional parameter missing", name.toLatin1().data());
        return false;
    default:
        qWarning("QAxBase: Error calling IDispatch member %s: Unknown error", name.toLatin1().data());
        return false;
    }
}

/*!
    \internal
*/
int QAxBase::internalProperty(QMetaObject::Call call, int index, void **v)
{
    const QMetaObject *mo = metaObject();
    const QMetaProperty prop = mo->property(index + mo->propertyOffset());
    QByteArray propname = prop.name();

    // hardcoded control property
    if (propname == "control") {
        switch(call) {
        case QMetaObject::ReadProperty:
            *(QString*)*v = control();
            break;
        case QMetaObject::WriteProperty:
            setControl(*(QString*)*v);
            break;
        case QMetaObject::ResetProperty:
            clear();
            break;
        default:
            break;
        }
        return index - mo->propertyCount();
    }

    // get the IDispatch
    if (!d->ptr || !prop.isValid())
        return index;
    IDispatch *disp = d->dispatch();
    if (!disp)
        return index;

    DISPID dispid = d->metaObject()->dispIDofName(propname, disp);
    if (dispid == DISPID_UNKNOWN)
        return index;

    Q_ASSERT(d->metaobj);
    // property found, so everthing that goes wrong now should not bother the caller
    index -= mo->propertyCount();

    VARIANTARG arg;
    VariantInit(&arg);
    DISPPARAMS params;
    EXCEPINFO excepinfo;
    memset(&excepinfo, 0, sizeof(excepinfo));
    UINT argerr = 0;
    HRESULT hres = E_FAIL;

    QByteArray proptype(prop.typeName());
    switch (call) {
    case QMetaObject::ReadProperty:
        {
            params.cArgs = 0;
            params.cNamedArgs = 0;
            params.rgdispidNamedArgs = 0;
            params.rgvarg = 0;

            hres = disp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &arg, &excepinfo, 0);

            // map result VARIANTARG to void*
            uint type = QVariant::Int;
            if (!prop.isEnumType())
                type = prop.type();
            QVariantToVoidStar(VARIANTToQVariant(arg, proptype, type), *v, proptype, type);
            if ((arg.vt != VT_DISPATCH && arg.vt != VT_UNKNOWN) || type == QVariant::Pixmap || type == QVariant::Font)
                clearVARIANT(&arg);
        }
        break;

    case QMetaObject::WriteProperty:
        {
            QVariant::Type t = (QVariant::Type)prop.type();

            DISPID dispidNamed = DISPID_PROPERTYPUT;
            params.cArgs = 1;
            params.cNamedArgs = 1;
            params.rgdispidNamedArgs = &dispidNamed;
            params.rgvarg = &arg;

            arg.vt = VT_ERROR;
            arg.scode = DISP_E_TYPEMISMATCH;

            // map void* to VARIANTARG via QVariant
            QVariant qvar;
            if (prop.isEnumType()) {
                qvar = *(int*)v[0];
                proptype = 0;
            } else {
                if (t == QVariant::LastType) {
                    qvar = *(QVariant*)v[0];
                    proptype = 0;
                } else if (t == QVariant::UserType) {
                    qvar = QVariant(qRegisterMetaType<void*>(prop.typeName()), (void**)v[0]);
//                    qvar.setValue(*(void**)v[0], prop.typeName());
                } else {
                    proptype = d->metaObject()->propertyType(propname);
                    qvar = QVariant(t, v[0]);
                }
            }

            QVariantToVARIANT(qvar, arg, proptype);
            if (arg.vt == VT_EMPTY || arg.vt == VT_ERROR) {
                qWarning("QAxBase::setProperty: Unhandled property type %s", prop.typeName());
                break;
            }
        }
        hres = disp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &params, 0, &excepinfo, &argerr);
        clearVARIANT(&arg);
        break;

    default:
        break;
    }

    checkHRESULT(hres, &excepinfo, this, QLatin1String(propname), argerr);
    return index;
}

int QAxBase::internalInvoke(QMetaObject::Call call, int index, void **v)
{
    Q_ASSERT(call == QMetaObject::InvokeMetaMethod);
    Q_UNUSED(call);

    // get the IDispatch
    IDispatch *disp = d->dispatch();
    if (!disp)
        return index;

    const QMetaObject *mo = metaObject();
    // get the slot information
    const QMetaMethod slot = mo->method(index + mo->methodOffset());
    Q_ASSERT(slot.methodType() == QMetaMethod::Slot);

    QByteArray signature(slot.signature());
    QByteArray slotname(signature);
    slotname.truncate(slotname.indexOf('('));

    // Get the Dispatch ID of the method to be called
    bool isProperty = false;
    DISPID dispid = d->metaObject()->dispIDofName(slotname, disp);

    Q_ASSERT(d->metaobj);

    if (dispid == DISPID_UNKNOWN && slotname.toLower().startsWith("set")) {
        // see if we are calling a property set function as a slot
        slotname = slotname.right(slotname.length() - 3);
        dispid = d->metaobj->dispIDofName(slotname, disp);
        isProperty = true;
    }
    if (dispid == DISPID_UNKNOWN)
        return index;

    // slot found, so everthing that goes wrong now should not bother the caller
    index -= mo->methodCount();

    // setup the parameters
    DISPPARAMS params;
    DISPID dispidNamed = DISPID_PROPERTYPUT;
    params.cArgs = d->metaobj->numParameter(signature);
    params.cNamedArgs = isProperty ? 1 : 0;
    params.rgdispidNamedArgs = isProperty ? &dispidNamed : 0;
    params.rgvarg = 0;
    VARIANTARG static_rgvarg[QAX_NUM_PARAMS];
    if (params.cArgs) {
        if (params.cArgs <= QAX_NUM_PARAMS)
            params.rgvarg = static_rgvarg;
        else
            params.rgvarg = new VARIANTARG[params.cArgs];
    }

    int p;
    for (p = 0; p < (int)params.cArgs; ++p) {
        bool out;
        QByteArray type = d->metaobj->paramType(signature, p, &out);
        QVariant::Type vt = QVariant::nameToType(type);
        QVariant qvar;
        if (vt != QVariant::UserType)
            qvar = QVariant(vt, v[p + 1]);

        if (!qvar.isValid()) {
            if (type == "IDispatch*")
                qvar.setValue(*(IDispatch**)v[p+1]);
            else if (type == "IUnknown*")
                qvar.setValue(*(IUnknown**)v[p+1]);
            else if (type == "QVariant")
                qvar = *(QVariant*)v[p + 1];
            else if (mo->indexOfEnumerator(type) != -1)
                qvar = *(int*)v[p + 1];
            else
                qvar = QVariant(QMetaType::type(type), v[p + 1]);
        }

        QVariantToVARIANT(qvar, params.rgvarg[params.cArgs - p - 1], type, out);
    }

    // call the method
    VARIANT ret;
    VariantInit(&ret);
    UINT argerr = 0;
    HRESULT hres = E_FAIL;
    EXCEPINFO excepinfo;
    memset(&excepinfo, 0, sizeof(excepinfo));

    WORD wFlags = isProperty ? DISPATCH_PROPERTYPUT : DISPATCH_METHOD | DISPATCH_PROPERTYGET;
    hres = disp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, wFlags, &params, &ret, &excepinfo, &argerr);

    // get return value
    if (hres == S_OK && ret.vt != VT_EMPTY)
        QVariantToVoidStar(VARIANTToQVariant(ret, slot.typeName()), v[0], slot.typeName());

    // update out parameters
    for (p = 0; p < (int)params.cArgs; ++p) {
        bool out;
        QByteArray ptype = d->metaobj->paramType(signature, p, &out);
        if (out)
            QVariantToVoidStar(VARIANTToQVariant(params.rgvarg[params.cArgs - p - 1], ptype), v[p+1], ptype);
    }
    // clean up
    for (p = 0; p < (int)params.cArgs; ++p)
        clearVARIANT(params.rgvarg+p);
    if (params.rgvarg != static_rgvarg)
        delete [] params.rgvarg;

    checkHRESULT(hres, &excepinfo, this, QString::fromLatin1(slotname), params.cArgs-argerr-1);
    return index;
}

/*!
    \internal
*/
int QAxBase::qt_metacall(QMetaObject::Call call, int id, void **v)
{
    const QMetaObject *mo = metaObject();
    if (isNull() && mo->property(id + mo->propertyOffset()).name() != QByteArray("control")) {
        qWarning("QAxBase::qt_metacall: Object is not initialized, or initialization failed");
        return id;
    }

    switch(call) {
    case QMetaObject::InvokeMetaMethod:
        switch (mo->method(id + mo->methodOffset()).methodType()) {
        case QMetaMethod::Signal:
            QMetaObject::activate(qObject(), mo, id, v);
            id -= mo->methodCount();
            break;
        case QMetaMethod::Method:
        case QMetaMethod::Slot:
            id = internalInvoke(call, id, v);
            break;
        default:
            break;
        }
        break;
    case QMetaObject::ReadProperty:
    case QMetaObject::WriteProperty:
    case QMetaObject::ResetProperty:
        id = internalProperty(call, id, v);
        break;
    case QMetaObject::QueryPropertyScriptable:
    case QMetaObject::QueryPropertyDesignable:
    case QMetaObject::QueryPropertyStored:
    case QMetaObject::QueryPropertyEditable:
    case QMetaObject::QueryPropertyUser:
        id -= mo->propertyCount();
        break;
    default:
        break;
    }
    Q_ASSERT(id < 0);
    return id;
}

#ifdef QT_CHECK_STATE
static void qax_noSuchFunction(int disptype, const QByteArray &name, const QByteArray &function, const QAxBase *that)
{
    const QMetaObject *metaObject = that->metaObject();
    const char *coclass = metaObject->classInfo(metaObject->indexOfClassInfo("CoClass")).value();

    if (disptype == DISPATCH_METHOD) {
        qWarning("QAxBase::dynamicCallHelper: %s: No such method in %s [%s]", name.data(), that->control().toLatin1().data(), coclass ? coclass: "unknown");
        qWarning("\tCandidates are:");
        for (int i = 0; i < metaObject->methodCount(); ++i) {
            const QMetaMethod slot(metaObject->method(i));
            if (slot.methodType() != QMetaMethod::Slot)
                continue;
            QByteArray signature = slot.signature();
            if (signature.toLower().startsWith(function.toLower()))
                qWarning("\t\t%s", signature.data());
        }
    } else {
        qWarning("QAxBase::dynamicCallHelper: %s: No such property in %s [%s]", name.data(), that->control().toLatin1().data(), coclass ? coclass: "unknown");
        if (!function.isEmpty()) {
            qWarning("\tCandidates are:");
            char f0 = function.toLower().at(0);
            for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i) {
                QByteArray signature(metaObject->property(i).name());
                if (!signature.isEmpty() && signature.toLower().at(0) == f0)
                    qWarning("\t\t%s", signature.data());
            }
        }
    }
}
#endif

/*!
    \internal

    \a name is already normalized?
*/
bool QAxBase::dynamicCallHelper(const char *name, void *inout, QList<QVariant> &vars, QByteArray &type)
{
    if (isNull()) {
        qWarning("QAxBase::dynamicCallHelper: Object is not initialized, or initialization failed");
        return false;
    }

    IDispatch *disp = d->dispatch();
    if (!disp) {
        qWarning("QAxBase::dynamicCallHelper: Object does not support automation");
        return false;
    }

    const QMetaObject *mo = metaObject();
    d->metaObject();
    Q_ASSERT(d->metaobj);

    int varc = vars.count();

    QByteArray normFunction = QMetaObject::normalizedSignature(name);
    QByteArray function(normFunction);
    VARIANT staticarg[QAX_NUM_PARAMS];
    VARIANT *arg = 0;
    VARIANTARG *res = (VARIANTARG*)inout;

    unsigned short disptype;

    int id = -1;
    bool parse = false;

    if (function.contains('(')) {
        disptype = DISPATCH_METHOD | DISPATCH_PROPERTYGET;
        if (d->useMetaObject)
            id = mo->indexOfSlot(function);
        if (id >= 0) {
            const QMetaMethod slot = mo->method(id);
            Q_ASSERT(slot.methodType() == QMetaMethod::Slot);
            function = slot.signature();
            type = slot.typeName();
        }
        function.truncate(function.indexOf('('));
        parse = !varc && normFunction.length() > function.length() + 2;
        if (parse) {
            QString args = QLatin1String(normFunction);
            args = args.mid(function.length() + 1);
            // parse argument string int list of arguments
            QString curArg;
            const QChar *c = args.unicode();
            int index = 0;
            bool inString = false;
            bool inEscape = false;
            while (index < (int)args.length()) {
                QChar cc = *c;
                ++c;
                ++index;
                switch(cc.toLatin1()) {
                case 'n':
                    if (inEscape)
                        cc = QLatin1Char('\n');
                    break;
                case 'r':
                    if (inEscape)
                        cc = QLatin1Char('\r');
                    break;
                case 't':
                    if (inEscape)
                        cc = QLatin1Char('\t');
                    break;
                case '\\':
                    if (!inEscape && inString) {
                        inEscape = true;
                        continue;
                    }
                    break;
                case '"':
                    if (!inEscape) {
                        inString = !inString;
                        curArg += cc;
                        continue;
                    }
                    break;
                case ' ':
                    if (!inString && curArg.isEmpty())
                        continue;
                    break;
                case ',':
                case ')':
                    if (inString)
                        break;
                    curArg = curArg.trimmed();
                    if (curArg.at(0) == QLatin1Char('\"') && curArg.at(curArg.length()-1) == QLatin1Char('\"')) {
                        vars << curArg.mid(1, curArg.length() - 2);
                    } else {
                        bool isNumber = false;
                        bool isDouble = false;
                        int number = curArg.toInt(&isNumber);
                        double dbl = curArg.toDouble(&isDouble);
                        if (isNumber) {
                            vars << number;
                        } else if (isDouble) {
                            vars << dbl;
                        } else {
                            bool isEnum = false;
                            for (int enumIndex = 0; enumIndex < mo->enumeratorCount(); ++enumIndex) {
                                QMetaEnum metaEnum =mo->enumerator(enumIndex);
                                int value = metaEnum.keyToValue(curArg.toLatin1());
                                if (value != -1 && !QByteArray(metaEnum.valueToKey(value)).isEmpty()) {
                                    vars << value;
                                    isEnum = true;
                                    break;
                                }
                            }
                            if (!isEnum)
                                vars << curArg;
                        }
                    }
                    curArg.clear();
                    continue;
                default:
                    break;
                }
                inEscape = false;
                curArg += cc;
            }

            varc = vars.count();
        }
    } else {
        if (d->useMetaObject)
            id = mo->indexOfProperty(normFunction);

        if (id >= 0) {
            const QMetaProperty prop =mo->property(id);
            type = prop.typeName();
        }
        if (varc == 1) {
            res = 0;
            disptype = DISPATCH_PROPERTYPUT;
        } else {
            disptype = DISPATCH_PROPERTYGET;
        }
    }
    if (varc) {
        varc = qMin(varc, d->metaobj->numParameter(normFunction));
        arg = varc <= QAX_NUM_PARAMS ? staticarg : new VARIANT[varc];
        for (int i = 0; i < varc; ++i) {
            QVariant var(vars.at(i));
            VariantInit(arg + (varc - i - 1));
            bool out = false;
            QByteArray paramType;
            if (disptype == DISPATCH_PROPERTYPUT)
                paramType = type;
            else if (parse || disptype == DISPATCH_PROPERTYGET)
                paramType = 0;
            else
                paramType = d->metaobj->paramType(normFunction, i, &out);

            if ((!parse && d->useMetaObject && var.type() == QVariant::String) || var.type() == QVariant::ByteArray) {
                int enumIndex =mo->indexOfEnumerator(paramType);
                if (enumIndex != -1) {
                    QMetaEnum metaEnum =mo->enumerator(enumIndex);
                    QVariantToVARIANT(metaEnum.keyToValue(var.toByteArray()), arg[varc - i - 1], "int", out);
                }
            }

            if (arg[varc - i - 1].vt == VT_EMPTY)
                QVariantToVARIANT(var, arg[varc - i - 1], paramType, out);
        }
    }

    DISPID dispid = d->metaobj->dispIDofName(function, disp);
    if (dispid == DISPID_UNKNOWN && function.toLower().startsWith("set")) {
        function = function.mid(3);
        dispid = d->metaobj->dispIDofName(function, disp);
        disptype = DISPATCH_PROPERTYPUT;
    }

    if (dispid == DISPID_UNKNOWN) {
#ifdef QT_CHECK_STATE
        qax_noSuchFunction(disptype, normFunction, function, this);
#endif
        return false;
    }

    DISPPARAMS params;
    DISPID dispidNamed = DISPID_PROPERTYPUT;

    params.cArgs = varc;
    params.cNamedArgs = (disptype == DISPATCH_PROPERTYPUT) ? 1 : 0;
    params.rgdispidNamedArgs = (disptype == DISPATCH_PROPERTYPUT) ? &dispidNamed : 0;
    params.rgvarg = arg;
    EXCEPINFO excepinfo;
    memset(&excepinfo, 0, sizeof(excepinfo));
    UINT argerr = 0;

    HRESULT hres = disp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, disptype, &params, res, &excepinfo, &argerr);

    if (disptype == (DISPATCH_METHOD|DISPATCH_PROPERTYGET) && hres == S_OK && varc) {
        for (int i = 0; i < varc; ++i)
            if (arg[varc-i-1].vt & VT_BYREF) // update out-parameters
                vars[i] = VARIANTToQVariant(arg[varc-i-1], vars.at(i).typeName());
    }

    // clean up
    for (int i = 0; i < varc; ++i)
        clearVARIANT(params.rgvarg+i);
    if (arg && arg != staticarg)
        delete[] arg;

    return checkHRESULT(hres, &excepinfo, this, QLatin1String(function), varc-argerr-1);
}


/*!
    Calls the COM object's method \a function, passing the
    parameters \a var1, \a var1, \a var2, \a var3, \a var4, \a var5,
    \a var6, \a var7 and \a var8, and returns the value returned by
    the method, or an invalid QVariant if the method does not return
    a value or when the function call failed.

    If \a function is a method of the object the string must be provided
    as the full prototype, for example as it would be written in a
    QObject::connect() call.

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 15

    Alternatively a function can be called passing the parameters embedded
    in the string, e.g. above function can also be invoked using

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 16

    All parameters are passed as strings; it depends on the control whether
    they are interpreted correctly, and is slower than using the prototype
    with correctly typed parameters.

    If \a function is a property the string has to be the name of the
    property. The property setter is called when \a var1 is a valid QVariant,
    otherwise the getter is called.

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 17

    Note that it is faster to get and set properties using
    QObject::property() and QObject::setProperty().

    dynamicCall() can also be used to call objects with a
    \l{QAxBase::disableMetaObject()}{disabled metaobject} wrapper,
    which can improve performance significantely, esp. when calling many
    different objects of different types during an automation process.
    ActiveQt will then however not validate parameters.

    It is only possible to call functions through dynamicCall() that
    have parameters or return values of datatypes supported by
    QVariant. See the QAxBase class documentation for a list of
    supported and unsupported datatypes. If you want to call functions
    that have unsupported datatypes in the parameter list, use
    queryInterface() to retrieve the appropriate COM interface, and
    use the function directly.

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 18

    This is also more efficient.
*/
QVariant QAxBase::dynamicCall(const char *function,
                              const QVariant &var1,
                              const QVariant &var2,
                              const QVariant &var3,
                              const QVariant &var4,
                              const QVariant &var5,
                              const QVariant &var6,
                              const QVariant &var7,
                              const QVariant &var8)
{
    QList<QVariant> vars;
    QVariant var = var1;
    int argc = 1;
    while(var.isValid()) {
        vars << var;
        switch(++argc) {
        case 2: var = var2; break;
        case 3: var = var3; break;
        case 4: var = var4; break;
        case 5: var = var5; break;
        case 6: var = var6; break;
        case 7: var = var7; break;
        case 8: var = var8; break;
        default:var = QVariant(); break;
        }
    }

    return dynamicCall(function, vars);
}

/*!
    \overload

    Calls the COM object's method \a function, passing the
    parameters in \a vars, and returns the value returned by
    the method. If the method does not return a value or when
    the function call failed this function returns an invalid
    QVariant object.

    The QVariant objects in \a vars are updated when the method has
    out-parameters.
*/
QVariant QAxBase::dynamicCall(const char *function, QList<QVariant> &vars)
{
    VARIANTARG res;
    VariantInit(&res);

    QByteArray rettype;
    if (!dynamicCallHelper(function, &res, vars, rettype))
        return QVariant();

    QVariant qvar = VARIANTToQVariant(res, rettype);
    if ((res.vt != VT_DISPATCH && res.vt != VT_UNKNOWN) || qvar.type() == QVariant::Pixmap || qvar.type() == QVariant::Font)
        clearVARIANT(&res);

    return qvar;
}

/*!
    Returns a pointer to a QAxObject wrapping the COM object provided
    by the method or property \a name, passing passing the parameters
    \a var1, \a var1, \a var2, \a var3, \a var4, \a var5, \a var6,
    \a var7 and \a var8.

    If \a name is provided by a method the string must include the
    full function prototype.

    If \a name is a property the string must be the name of the property,
    and \a var1, ... \a var8 are ignored.

    The returned QAxObject is a child of this object (which is either of
    type QAxObject or QAxWidget), and is deleted when this object is
    deleted. It is however safe to delete the returned object yourself,
    and you should do so when you iterate over lists of subobjects.

    COM enabled applications usually have an object model publishing
    certain elements of the application as dispatch interfaces. Use
    this method to navigate the hierarchy of the object model, e.g.

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 19
*/
QAxObject *QAxBase::querySubObject(const char *name,
                                   const QVariant &var1,
                                   const QVariant &var2,
                                   const QVariant &var3,
                                   const QVariant &var4,
                                   const QVariant &var5,
                                   const QVariant &var6,
                                   const QVariant &var7,
                                   const QVariant &var8)
{
    QList<QVariant> vars;
    QVariant var = var1;
    int argc = 1;
    while(var.isValid()) {
        vars << var;
        switch(++argc) {
        case 2: var = var2; break;
        case 3: var = var3; break;
        case 4: var = var4; break;
        case 5: var = var5; break;
        case 6: var = var6; break;
        case 7: var = var7; break;
        case 8: var = var8; break;
        default:var = QVariant(); break;
        }
    }

    return querySubObject(name, vars);
}

/*!
    \overload

    The QVariant objects in \a vars are updated when the method has
    out-parameters.
*/
QAxObject *QAxBase::querySubObject(const char *name, QList<QVariant> &vars)
{
    QAxObject *object = 0;
    VARIANTARG res;
    VariantInit(&res);

    QByteArray rettype;
    if (!dynamicCallHelper(name, &res, vars, rettype))
        return 0;

    switch (res.vt) {
    case VT_DISPATCH:
        if (res.pdispVal) {
            if (rettype.isEmpty() || rettype == "IDispatch*" || rettype == "QVariant") {
                object = new QAxObject(res.pdispVal, qObject());
            } else if (QMetaType::type(rettype)) {
                QVariant qvar = VARIANTToQVariant(res, rettype, 0);
                object = *(QAxObject**)qvar.constData();
//                qVariantGet(qvar, object, rettype);
                res.pdispVal->AddRef();
            }
            if (object)
                ((QAxBase*)object)->d->tryCache = true;
        }
        break;
    case VT_UNKNOWN:
        if (res.punkVal) {
            if (rettype.isEmpty() || rettype == "IUnknown*") {
                object = new QAxObject(res.punkVal, qObject());
            } else if (QMetaType::type(rettype)) {
                QVariant qvar = VARIANTToQVariant(res, rettype, 0);
                object = *(QAxObject**)qvar.constData();
//                qVariantGet(qvar, object, rettype);
                res.punkVal->AddRef();
            }
            if (object)
                ((QAxBase*)object)->d->tryCache = true;
        }
        break;
    case VT_EMPTY:
#ifdef QT_CHECK_STATE
        {
            const char *coclass = metaObject()->classInfo(metaObject()->indexOfClassInfo("CoClass")).value();
            qWarning("QAxBase::querySubObject: %s: Error calling function or property in %s (%s)"
                , name, control().toLatin1().data(), coclass ? coclass: "unknown");
        }
#endif
        break;
    default:
#ifdef QT_CHECK_STATE
        {
            const char *coclass = metaObject()->classInfo(metaObject()->indexOfClassInfo("CoClass")).value();
            qWarning("QAxBase::querySubObject: %s: Method or property is not of interface type in %s (%s)"
                , name, control().toLatin1().data(), coclass ? coclass: "unknown");
        }
#endif
        break;
    }

    clearVARIANT(&res);
    return object;
}

class QtPropertyBag : public IPropertyBag
{
public:
    QtPropertyBag() :ref(0) {}
    virtual ~QtPropertyBag() {}

    HRESULT __stdcall QueryInterface(REFIID iid, LPVOID *iface)
    {
        *iface = 0;
        if (iid == IID_IUnknown)
            *iface = this;
        else if (iid == IID_IPropertyBag)
            *iface = this;
        else
            return E_NOINTERFACE;

        AddRef();
        return S_OK;
    }
    unsigned long __stdcall AddRef()
    {
        return InterlockedIncrement(&ref);
    }
    unsigned long __stdcall Release()
    {
        LONG refCount = InterlockedDecrement(&ref);
        if (!refCount)
            delete this;

        return refCount;
    }

    HRESULT __stdcall Read(LPCOLESTR name, VARIANT *var, IErrorLog *)
    {
        if (!var)
            return E_POINTER;

        QString property = QString::fromWCharArray(name);
        QVariant qvar = map.value(property);
        QVariantToVARIANT(qvar, *var);
        return S_OK;
    }
    HRESULT __stdcall Write(LPCOLESTR name, VARIANT *var)
    {
        if (!var)
            return E_POINTER;
        QString property = QString::fromWCharArray(name);
        QVariant qvar = VARIANTToQVariant(*var, 0);
        map[property] = qvar;

        return S_OK;
    }

    QAxBase::PropertyBag map;

private:
    LONG ref;
};

/*!
    Returns a name:value map of all the properties exposed by the COM
    object.

    This is more efficient than getting multiple properties
    individually if the COM object supports property bags.

    \warning It is not guaranteed that the property bag implementation
    of the COM object returns all properties, or that the properties
    returned are the same as those available through the IDispatch
    interface.
*/
QAxBase::PropertyBag QAxBase::propertyBag() const
{
    PropertyBag result;

    if (!d->ptr && !d->initialized) {
        ((QAxBase*)this)->initialize(&d->ptr);
        d->initialized = true;
    }

    if (isNull())
        return result;
    IPersistPropertyBag *persist = 0;
    d->ptr->QueryInterface(IID_IPersistPropertyBag, (void**)&persist);
    if (persist) {
        QtPropertyBag *pbag = new QtPropertyBag();
        pbag->AddRef();
        persist->Save(pbag, false, true);
        result = pbag->map;
        pbag->Release();
        persist->Release();
        return result;
    } else {
        const QMetaObject *mo = metaObject();
        for (int p = mo->propertyOffset(); p < mo->propertyCount(); ++p) {
            const QMetaProperty property = mo->property(p);
            QVariant var = qObject()->property(property.name());
            result.insert(QLatin1String(property.name()), var);
        }
    }
    return result;
}

/*!
    Sets the properties of the COM object to the corresponding values
    in \a bag.

    \warning
    You should only set property bags that have been returned by the
    propertyBag function, as it cannot be guaranteed that the property
    bag implementation of the COM object supports the same properties
    that are available through the IDispatch interface.

    \sa propertyBag()
*/
void QAxBase::setPropertyBag(const PropertyBag &bag)
{
    if (!d->ptr && !d->initialized) {
        initialize(&d->ptr);
        d->initialized = true;
    }

    if (isNull())
        return;
    IPersistPropertyBag *persist = 0;
    d->ptr->QueryInterface(IID_IPersistPropertyBag, (void**)&persist);
    if (persist) {
        QtPropertyBag *pbag = new QtPropertyBag();
        pbag->map = bag;
        pbag->AddRef();
        persist->Load(pbag, 0);
        pbag->Release();
        persist->Release();
    } else {
        const QMetaObject *mo = metaObject();
        for (int p = mo->propertyOffset(); p < mo->propertyCount(); ++p) {
            const QMetaProperty property = mo->property(p);
            QVariant var = bag.value(QLatin1String(property.name()));
            qObject()->setProperty(property.name(), var);
        }
    }
}

/*!
    Returns true if the property \a prop is writable; otherwise
    returns false. By default, all properties are writable.

    \warning
    Depending on the control implementation this setting might be
    ignored for some properties.

    \sa setPropertyWritable(), propertyChanged()
*/
bool QAxBase::propertyWritable(const char *prop) const
{
    return d->propWritable.value(prop, true);
}

/*!
    Sets the property \a prop to writable if \a ok is true, otherwise
    sets \a prop to be read-only. By default, all properties are
    writable.

    \warning
    Depending on the control implementation this setting might be
    ignored for some properties.

    \sa propertyWritable(), propertyChanged()
*/
void QAxBase::setPropertyWritable(const char *prop, bool ok)
{
    d->propWritable[prop] = ok;
}

/*!
    Returns true if there is no COM object loaded by this wrapper;
    otherwise return false.

    \sa control
*/
bool QAxBase::isNull() const
{
    return !d->ptr;
}

/*!
    Returns a QVariant that wraps the COM object. The variant can
    then be used as a parameter in e.g. dynamicCall().
*/
QVariant QAxBase::asVariant() const
{
    if (!d->ptr && !d->initialized) {
        ((QAxBase*)this)->initialize(&d->ptr);
        d->initialized = true;
    }

    QVariant qvar;
    QByteArray cn(className());
    if (cn == "QAxObject" || cn == "QAxWidget" || cn == "QAxBase") {
        if (d->dispatch())
            qvar.setValue(d->dispatch());
        else if (d->ptr)
            qvar.setValue(d->ptr);
    } else {
        cn = cn.mid(cn.lastIndexOf(':') + 1);
        QObject *object = qObject();
        if (QMetaType::type(cn))
            qvar = QVariant(qRegisterMetaType<QObject*>(cn + '*'), &object);
//            qvar.setValue(qObject(), cn + '*');
    }

    return qvar;
}

// internal function that creates a QAxObject from an iface
// used by type-conversion code (types.cpp)
void *qax_createObjectWrapper(int metaType, IUnknown *iface)
{
    if (!iface)
        return 0;

    QAxObject *object = (QAxObject*)QMetaType::construct(metaType, 0);
    QAxBasePrivate *d = object->d;

    d->ptr = iface;
    d->initialized = true;

    // no release, since no addref

    return object;
}

/*!
    \fn void QAxBase::signal(const QString &name, int argc, void *argv)

    This generic signal gets emitted when the COM object issues the
    event \a name. \a argc is the number of parameters provided by the
    event (DISPPARAMS.cArgs), and \a argv is the pointer to the
    parameter values (DISPPARAMS.rgvarg). Note that the order of parameter
    values is turned around, ie. the last element of the array is the first
    parameter in the function.

    \snippet doc/src/snippets/code/src_activeqt_container_qaxbase.cpp 20

    Use this signal if the event has parameters of unsupported data
    types. Otherwise, connect directly to the signal \a name.
*/

/*!
    \fn void QAxBase::propertyChanged(const QString &name)

    If the COM object supports property notification, this signal gets
    emitted when the property called \a name is changed.
*/

/*!
    \fn void QAxBase::exception(int code, const QString &source, const QString &desc, const QString &help)

    This signal is emitted when the COM object throws an exception while called using the OLE automation
    interface IDispatch. \a code, \a source, \a desc and \a help provide information about the exception as
    provided by the COM server and can be used to provide useful feedback to the end user. \a help includes
    the help file, and the help context ID in brackets, e.g. "filename [id]".
*/

/*!
    \fn QObject *QAxBase::qObject() const
    \internal
*/

/*!
    \fn const char *QAxBase::className() const
    \internal
*/

QT_END_NAMESPACE
#endif //QT_NO_WIN_ACTIVEQT
