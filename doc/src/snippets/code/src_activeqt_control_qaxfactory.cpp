/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the documentation of the Qt Toolkit.
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

//! [0]
QStringList ActiveQtFactory::featureList() const
{
    QStringList list;
    list << "ActiveX1";
    list << "ActiveX2";
    return list;
}

QObject *ActiveQtFactory::createObject(const QString &key)
{
    if (key == "ActiveX1")
        return new ActiveX1(parent);
    if (key == "ActiveX2")
        return new ActiveX2(parent);
    return 0;
}

const QMetaObject *ActiveQtFactory::metaObject(const QString &key) const
{
    if (key == "ActiveX1")
        return &ActiveX1::staticMetaObject;
    if (key == "ActiveX2")
        return &ActiveX2::staticMetaObject;
}

QUuid ActiveQtFactory::classID(const QString &key) const
{
    if (key == "ActiveX1")
        return "{01234567-89AB-CDEF-0123-456789ABCDEF}";
    ...
    return QUuid();
}

QUuid ActiveQtFactory::interfaceID(const QString &key) const
{
    if (key == "ActiveX1")
        return "{01234567-89AB-CDEF-0123-456789ABCDEF}";
    ...
    return QUuid();
}

QUuid ActiveQtFactory::eventsID(const QString &key) const
{
    if (key == "ActiveX1")
        return "{01234567-89AB-CDEF-0123-456789ABCDEF}";
    ...
    return QUuid();
}

QAXFACTORY_EXPORT(
    ActiveQtFactory,			      // factory class
    "{01234567-89AB-CDEF-0123-456789ABCDEF}", // type library ID
    "{01234567-89AB-CDEF-0123-456789ABCDEF}"  // application ID
)
//! [0]


//! [1]
QAXFACTORY_BEGIN(
    "{01234567-89AB-CDEF-0123-456789ABCDEF}", // type library ID
    "{01234567-89AB-CDEF-0123-456789ABCDEF}"  // application ID
)
    QAXCLASS(Class1)
    QAXCLASS(Class2)
QAXFACTORY_END()
//! [1]


//! [2]
#include <qapplication.h>
#include <qaxfactory.h>

#include "theactivex.h"

QAXFACTORY_DEFAULT(
    TheActiveX,				  // widget class
    "{01234567-89AB-CDEF-0123-456789ABCDEF}", // class ID
    "{01234567-89AB-CDEF-0123-456789ABCDEF}", // interface ID
    "{01234567-89AB-CDEF-0123-456789ABCDEF}", // event interface ID
    "{01234567-89AB-CDEF-0123-456789ABCDEF}", // type library ID
    "{01234567-89AB-CDEF-0123-456789ABCDEF}"  // application ID
)
//! [2]


//! [3]
settings->setValue("/CLSID/" + classID(key)
                   + "/Implemented Categories/"
                   + "/{00000000-0000-0000-000000000000}/.",
                   QString());
//! [3]


//! [4]
settings->remove("/CLSID/" + classID(key)
                 + "/Implemented Categories"
                 + "/{00000000-0000-0000-000000000000}/.");
//! [4]


//! [5]
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    if (!QAxFactory::isServer()) {
        // initialize for stand-alone execution
    }
    return app.exec();
}
//! [5]


//! [6]
if (QAxFactory::isServer()) {
    QAxFactory::stopServer();
    QAxFactory::startServer(QAxFactory::SingleInstance);
}
//! [6]


//! [7]
#include <qaxfactory.h>

#include "theactivex.h"

QAXFACTORY_DEFAULT(
    TheActiveX,				  // widget class
    "{01234567-89AB-CDEF-0123-456789ABCDEF}", // class ID
    "{01234567-89AB-CDEF-0123-456789ABCDEF}", // interface ID
    "{01234567-89AB-CDEF-0123-456789ABCDEF}", // event interface ID
    "{01234567-89AB-CDEF-0123-456789ABCDEF}", // type library ID
    "{01234567-89AB-CDEF-0123-456789ABCDEF}"  // application ID
)
//! [7]


//! [8]
QAXFACTORY_EXPORT(
    MyFactory,			              // factory class
    "{01234567-89AB-CDEF-0123-456789ABCDEF}", // type library ID
    "{01234567-89AB-CDEF-0123-456789ABCDEF}"  // application ID
)
//! [8]


//! [9]
QAXFACTORY_BEGIN(
    "{01234567-89AB-CDEF-0123-456789ABCDEF}", // type library ID
    "{01234567-89AB-CDEF-0123-456789ABCDEF}"  // application ID
)
    QAXCLASS(Class1)
    QAXCLASS(Class2)
QAXFACTORY_END()
//! [9]
