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

#include "qaxobject.h"

#ifndef QT_NO_WIN_ACTIVEQT

#include <quuid.h>
#include <qmetaobject.h>
#include <qstringlist.h>

#include <windows.h>

QT_BEGIN_NAMESPACE

/*!
    Creates an empty COM object and propagates \a parent to the
    QObject constructor. To initialize the object, call \link
    QAxBase::setControl() setControl \endlink.
*/
QAxObject::QAxObject(QObject *parent)
   : QObject(parent)
{
}

/*!
    Creates a QAxObject that wraps the COM object \a c. \a parent is
    propagated to the QObject constructor.

    \sa setControl()
*/
QAxObject::QAxObject(const QString &c, QObject *parent)
   : QObject(parent)
{
    setControl(c);
}

/*!
    Creates a QAxObject that wraps the COM object referenced by \a
    iface. \a parent is propagated to the QObject constructor.
*/
QAxObject::QAxObject(IUnknown *iface, QObject *parent)
   : QObject(parent), QAxBase(iface)
{
}

/*!
    Releases the COM object and destroys the QAxObject,
    cleaning up all allocated resources.
*/
QAxObject::~QAxObject()
{
    clear();
}

/*!
    \internal
*/
const QMetaObject *QAxObject::metaObject() const
{
    return QAxBase::metaObject();
}

/*!
    \internal
*/
const QMetaObject *QAxObject::parentMetaObject() const
{
    return &QObject::staticMetaObject;
}

/*!
    \internal
*/
void *QAxObject::qt_metacast(const char *cname)
{
    if (!qstrcmp(cname, "QAxObject")) return (void*)this;
    if (!qstrcmp(cname, "QAxBase")) return (QAxBase*)this;
    return QObject::qt_metacast(cname);
}

/*!
    \internal
*/
const char *QAxObject::className() const
{
    return "QAxObject";
}

/*!
    \internal
*/
int QAxObject::qt_metacall(QMetaObject::Call call, int id, void **v)
{
    id = QObject::qt_metacall(call, id, v);
    if (id < 0)
        return id;
    return QAxBase::qt_metacall(call, id, v);
}

/*!
    \fn QObject *QAxObject::qObject() const
    \internal
*/

/*!
    \reimp
*/
void QAxObject::connectNotify(const char *)
{
    QAxBase::connectNotify();
}

/*!
    \since 4.1

    Requests the COM object to perform the action \a verb. The
    possible verbs are returned by verbs().

    The function returns true if the object could perform the action, otherwise returns false.
*/
bool QAxObject::doVerb(const QString &verb)
{
    if (!verbs().contains(verb))
        return false;
    IOleObject *ole = 0;
    queryInterface(IID_IOleObject, (void**)&ole);
    if (!ole)
        return false;

    LONG index = indexOfVerb(verb);

    HRESULT hres = ole->DoVerb(index, 0, 0, 0, 0, 0);

    ole->Release();

    return hres == S_OK;
}

QT_END_NAMESPACE
#endif // QT_NO_WIN_ACTIVEQT
