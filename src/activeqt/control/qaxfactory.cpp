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

#include "qaxfactory.h"

#ifndef QT_NO_WIN_ACTIVEQT

#include <qfile.h>
#include <qfileinfo.h>
#include <qmetaobject.h>
#include <qsettings.h>
#include <qwidget.h>
#include <qt_windows.h>

QT_BEGIN_NAMESPACE

extern wchar_t qAxModuleFilename[MAX_PATH];

/*!
    \class QAxFactory
    \brief The QAxFactory class defines a factory for the creation of COM components.

    \inmodule QAxServer

    Implement this factory once in your COM server to provide information
    about the components the server can create. Subclass QAxFactory and implement
    the pure virtual functions in any implementation file (e.g. main.cpp), and export
    the factory using the \c QAXFACTORY_EXPORT() macro.

    \snippet doc/src/snippets/code/src_activeqt_control_qaxfactory.cpp 0

    If you use the \c Q_CLASSINFO() macro to provide the unique
    identifiers or other attributes for your class you can use the \c
    QAXFACTORY_BEGIN(), \c QAXCLASS() and \c QAXFACTORY_END() macros to
    expose one or more classes as COM objects.

    \snippet doc/src/snippets/code/src_activeqt_control_qaxfactory.cpp 1


    If your server supports just a single COM object, you can use
    a default factory implementation through the \c QAXFACTORY_DEFAULT() macro.

    \snippet doc/src/snippets/code/src_activeqt_control_qaxfactory.cpp 2

    Only one QAxFactory implementation may be instantiated and
    exported by an ActiveX server application. This instance is accessible
    through the global qAxFactory() function.

    A factory can also reimplement the registerClass() and
    unregisterClass() functions to set additional flags for an ActiveX
    control in the registry. To limit the number of methods or
    properties a widget class exposes from its parent classes
    reimplement exposeToSuperClass().

    \sa QAxAggregated, QAxBindable, {ActiveQt Framework}
*/

/*!
    Constructs a QAxFactory object that returns \a libid and \a appid
    in the implementation of the respective interface functions.
*/

QAxFactory::QAxFactory(const QUuid &libid, const QUuid &appid)
: typelib(libid), app(appid)
{
}

/*!
    Destroys the QAxFactory object.
*/
QAxFactory::~QAxFactory()
{
}

/*!
    \fn QUuid QAxFactory::typeLibID() const

    Reimplement this function to return the ActiveX server's type
    library identifier.
*/
QUuid QAxFactory::typeLibID() const
{
    return typelib;
}

/*!
    \fn QUuid QAxFactory::appID() const

    Reimplement this function to return the ActiveX server's
    application identifier.
*/
QUuid QAxFactory::appID() const
{
    return app;
}

/*!
    \fn QStringList QAxFactory::featureList() const

    Reimplement this function to return a list of the widgets (class
    names) supported by this factory.
*/

/*!
    \fn QObject *QAxFactory::createObject(const QString &key)

    Reimplement this function to return a new object for \a key, or 0 if
    this factory doesn't support the value of \a key.

    If the object returned is a QWidget it will be exposed as an ActiveX
    control, otherwise the returned object will be exposed as a simple COM
    object.
*/

/*!
    \fn const QMetaObject *QAxFactory::metaObject(const QString &key) const

    Reimplement this function to return the QMetaObject corresponding to
    \a key, or 0 if this factory doesn't support the value of \a key.
*/

/*!
    \fn bool QAxFactory::createObjectWrapper(QObject *object, IDispatch **wrapper)

    Reimplement this function to provide the COM object for \a object
    in \a wrapper. Return true if the function was successful; otherwise
    return false.

    The default implementation creates a generic automation wrapper based
    on the meta object information of \a object.
*/
// implementation in qaxserverbase.cpp

/*!
    Reimplement this function to return the class identifier for each
    \a key returned by the featureList() implementation, or an empty
    QUuid if this factory doesn't support the value of \a key.

    The default implementation interprets \a key as the class name,
    and returns the value of the Q_CLASSINFO() entry "ClassID".
*/
QUuid QAxFactory::classID(const QString &key) const
{
    const QMetaObject *mo = metaObject(key);
    if (!mo)
        return QUuid();
    QString id = QString::fromLatin1(mo->classInfo(mo->indexOfClassInfo("ClassID")).value());

    return QUuid(id);
}

/*!
    Reimplement this function to return the interface identifier for
    each \a key returned by the featureList() implementation, or an
    empty QUuid if this factory doesn't support the value of \a key.

    The default implementation interprets \a key as the class name,
    and returns the value of the Q_CLASSINFO() entry "InterfaceID".
*/
QUuid QAxFactory::interfaceID(const QString &key) const
{
    const QMetaObject *mo = metaObject(key);
    if (!mo)
        return QUuid();
    QString id = QString::fromLatin1(mo->classInfo(mo->indexOfClassInfo("InterfaceID")).value());

    return QUuid(id);
}

/*!
    Reimplement this function to return the identifier of the event
    interface for each \a key returned by the featureList()
    implementation, or an empty QUuid if this factory doesn't support
    the value of \a key.

    The default implementation interprets \a key as the class name,
    and returns the value of the Q_CLASSINFO() entry "EventsID".
*/
QUuid QAxFactory::eventsID(const QString &key) const
{
    const QMetaObject *mo = metaObject(key);
    if (!mo)
        return QUuid();
    QString id = QString::fromLatin1(mo->classInfo(mo->indexOfClassInfo("EventsID")).value());

    return QUuid(id);
}

/*!
    Registers additional values for the class \a key in the system
    registry using the \a settings object. The standard values have
    already been registered by the framework, but additional values,
    e.g. implemented categories, can be added in an implementation of
    this function.

    \snippet doc/src/snippets/code/src_activeqt_control_qaxfactory.cpp 3

    If you reimplement this function you must also reimplement
    unregisterClass() to remove the additional registry values.

    \sa QSettings
*/
void QAxFactory::registerClass(const QString &key, QSettings *settings) const
{
    Q_UNUSED(key);
    Q_UNUSED(settings)
}

/*!
    Unregisters any additional values for the class \a key from the
    system registry using the \a settings object.

    \snippet doc/src/snippets/code/src_activeqt_control_qaxfactory.cpp 4

    \sa registerClass(), QSettings
*/
void QAxFactory::unregisterClass(const QString &key, QSettings *settings) const
{
    Q_UNUSED(key);
    Q_UNUSED(settings)
}

/*!
    Reimplement this function to return true if \a licenseKey is a valid
    license for the class \a key, or if the current machine is licensed.

    The default implementation returns true if the class \a key is
    not licensed (ie. no \c Q_CLASSINFO() attribute "LicenseKey"), or
    if  \a licenseKey matches the value of the "LicenseKey"
    attribute, or if the machine is licensed through a .LIC file with
    the same filename as this COM server.
*/
bool QAxFactory::validateLicenseKey(const QString &key, const QString &licenseKey) const
{
    const QMetaObject *mo = metaObject(key);
    if (!mo)
        return true;

    QString classKey = QString::fromLatin1(mo->classInfo(mo->indexOfClassInfo("LicenseKey")).value());
    if (classKey.isEmpty())
        return true;

    if (licenseKey.isEmpty()) {
        QString licFile(QString::fromWCharArray(qAxModuleFilename));
        int lastDot = licFile.lastIndexOf(QLatin1Char('.'));
        licFile = licFile.left(lastDot) + QLatin1String(".lic");
        if (QFile::exists(licFile))
            return true;
        return false;
    }
    return licenseKey == classKey;
}

/*!
    Reimplement this function to return the name of the super class of
    \a key up to which methods and properties should be exposed by the
    ActiveX control.

    The default implementation interprets \a key as the class name,
    and returns the value of the \c Q_CLASSINFO() entry
    "ToSuperClass". If no such value is set the null-string is
    returned, and the functions  and properties of all the super
    classes including QWidget will be  exposed.

    To only expose the functions and properties of the class itself,
    reimplement this function to return \a key.
*/
QString QAxFactory::exposeToSuperClass(const QString &key) const
{
    const QMetaObject *mo = metaObject(key);
    if (!mo)
        return QString();
    return QString::fromLatin1(mo->classInfo(mo->indexOfClassInfo("ToSuperClass")).value());
}

/*!
    Reimplement this function to return true if the ActiveX control \a key
    should be a top level window, e.g. a dialog. The default implementation
    returns false.
*/
bool QAxFactory::stayTopLevel(const QString &key) const
{
    return false;
}

/*!
    Reimplement this function to return true if the ActiveX control
    \a key should support the standard ActiveX events
    \list
    \i Click
    \i DblClick
    \i KeyDown
    \i KeyPress
    \i KeyUp
    \i MouseDown
    \i MouseUp
    \i MouseMove
    \endlist

    The default implementation interprets \a key as the class name,
    and returns true if the value of the \c Q_CLASSINFO() entry
    "StockEvents" is "yes". Otherwise this function returns false.
*/
bool QAxFactory::hasStockEvents(const QString &key) const
{
    const QMetaObject *mo = metaObject(key);
    if (!mo)
        return false;
    return QString::fromLatin1(mo->classInfo(mo->indexOfClassInfo("StockEvents")).value()) == QLatin1String("yes");
}


extern bool qAxIsServer;

/*!
    Returns true if the application has been started (by COM) as an ActiveX
    server, otherwise returns false.

    \snippet doc/src/snippets/code/src_activeqt_control_qaxfactory.cpp 5
*/

bool QAxFactory::isServer()
{
    return qAxIsServer;
}

extern wchar_t qAxModuleFilename[MAX_PATH];

/*!
    Returns the directory that contains the server binary.

    For out-of-process servers this is the same as
    QApplication::applicationDirPath(). For in-process servers
    that function returns the directory that contains the hosting
    application.
*/
QString QAxFactory::serverDirPath()
{
    return QFileInfo(QString::fromWCharArray(qAxModuleFilename)).absolutePath();
}

/*!
    Returns the file path of the server binary.

    For out-of-process servers this is the same as
    QApplication::applicationFilePath(). For in-process servers
    that function returns the file path of the hosting application.
*/
QString QAxFactory::serverFilePath()
{
    return QString::fromWCharArray(qAxModuleFilename);
}

/*!
    Reimplement this function to return true if the server is
    running as a persistent service (e.g. an NT service) and should
    not terminate even when all objects provided have been released.

    The default implementation returns false.
*/
bool QAxFactory::isService() const
{
    return false;
}

/*!
    \enum QAxFactory::ServerType

    This enum specifies the different types of servers that can be
    started with startServer.

    \value SingleInstance The server process can create only one instance of each
    exported class. COM starts a new process for each request. This is typically
    used in servers that export only one creatable class.
    \value MultipleInstances The server can create multiple instances of
    each exported class. This is the default. All instances will live in the same
    thread, and will share static resources.
*/

/*!
    \fn bool QAxFactory::startServer(ServerType type);

    Starts the COM server with \a type and returns true if successful,
    otherwise returns false.

    Calling this function if the server is already running (or for an
    in-process server) does nothing and returns true.

    The server is started automatically with \a type set to \c MultipleInstances
    if the server executable has been started with the \c -activex
    command line parameter. To switch to SingleInstance, call 
    
    \snippet doc/src/snippets/code/src_activeqt_control_qaxfactory.cpp 6

    in your own main() entry point function.
*/

/*!
    \fn bool QAxFactory::stopServer();

    Stops the COM server and returns true if successful, otherwise
    returns false.

    Calling this function if the server is not running (or for an
    in-process server) does nothing and returns true.

    Stopping the server will not invalidate existing objects, but no
    new objects can be created from the existing server process. Usually
    COM will start a new server process if additional objects are requested.

    The server is stopped automatically when the main() function returns.
*/

class ActiveObject : public QObject
{
public:
    ActiveObject(QObject *parent, QAxFactory *factory);
    ~ActiveObject();

    IDispatch *wrapper;
    DWORD cookie;
};

ActiveObject::ActiveObject(QObject *parent, QAxFactory *factory)
: QObject(parent), wrapper(0), cookie(0)
{
    QLatin1String key(parent->metaObject()->className());

    factory->createObjectWrapper(parent, &wrapper);
    if (wrapper)
        RegisterActiveObject(wrapper, QUuid(factory->classID(key)), ACTIVEOBJECT_STRONG, &cookie);
}

ActiveObject::~ActiveObject()
{
    if (cookie)
        RevokeActiveObject(cookie, 0);
    if (wrapper)
        wrapper->Release();
}

/*!
    Registers the QObject \a object with COM as a running object, and returns true if
    the registration succeeded, otherwise returns false. The object is unregistered
    automatically when it is destroyed.

    This function should only be called if the application has been started by the user
    (i.e. not by COM to respond to a request), and only for one object, usually the
    toplevel object of the application's object hierarchy.

    This function does nothing and returns false if the object's class info for
    "RegisterObject" is not set to "yes", or if the server is an in-process server.
*/
bool QAxFactory::registerActiveObject(QObject *object)
{
    if (qstricmp(object->metaObject()->classInfo(object->metaObject()->indexOfClassInfo("RegisterObject")).value(), "yes"))
        return false;

    if (!QString::fromWCharArray(qAxModuleFilename).toLower().endsWith(QLatin1String(".exe")))
	return false;

    ActiveObject *active = new ActiveObject(object, qAxFactory());
    if (!active->wrapper || !active->cookie) {
        delete active;
        return false;
    }
    return true;
}

/*!
    \macro QAXFACTORY_DEFAULT(Class, ClassID, InterfaceID, EventID, LibID, AppID)
    \relates QAxFactory

    This macro can be used to export a single QObject subclass \a Class a this
    COM server through an implicitly declared QAxFactory implementation.

    This macro exports the class \a Class as a COM coclass with the CLSID \a ClassID.
    The properties and slots will be declared through a COM interface with the IID
    \a InterfaceID, and signals will be declared through a COM event interface with
    the IID \a EventID. All declarations will be in a type library with the id \a LibID,
    and if the server is an executable server then it will have the application id
    \a AppID.

    \snippet doc/src/snippets/code/src_activeqt_control_qaxfactory.cpp 7

    \sa QAXFACTORY_EXPORT(), QAXFACTORY_BEGIN()
*/

/*!
    \macro QAXFACTORY_EXPORT(Class, LibID, AppID)
    \relates QAxFactory

    This macro can be used to export a QAxFactory implementation \a Class from
    a COM server. All declarations will be in a type library with the id \a LibID,
    and if the server is an executable server then it will have the application id
    \a AppID.

    \snippet doc/src/snippets/code/src_activeqt_control_qaxfactory.cpp 8

    \sa QAXFACTORY_BEGIN()
*/

/*!
    \macro QAXFACTORY_BEGIN(IDTypeLib, IDApp)
    \relates QAxFactory

    This macro can be used to export multiple QObject classes through an
    implicitly declared QAxFactory implementation. All QObject classes have to
    declare the ClassID, InterfaceID and EventsID (if applicable) through the
    Q_CLASSINFO() macro. All declarations will be in a type library with the id
    \a IDTypeLib, and if the server is an executable server then it will have the
    application id \a IDApp.

    This macro needs to be used together with the QAXCLASS(), QAXTYPE()
    and QAXFACTORY_END() macros.

    \snippet doc/src/snippets/code/src_activeqt_control_qaxfactory.cpp 9
*/

/*!
    \macro QAXCLASS(Class)
    \relates QAxFactory

    This macro adds a creatable COM class \a Class to the QAxFactory declared
    with the QAXFACTORY_BEGIN() macro.

    \sa QAXFACTORY_BEGIN(), QAXTYPE(), QAXFACTORY_END(), Q_CLASSINFO()
*/

/*!
    \macro QAXTYPE(Class)
    \relates QAxFactory

    This macro adds a non-creatable COM class \a Class to the QAxFactory
    declared with the QAXFACTORY_BEGIN(). The class \a Class can be used
    in APIs of other COM classes exported through QAXTYPE() or QAXCLASS().

    Instances of type \a Class can only be retrieved using APIs of already
    instantiated objects.

    \sa QAXFACTORY_BEGIN(), QAXCLASS(), QAXFACTORY_END(), Q_CLASSINFO()
*/

/*!
    \macro QAXFACTORY_END()
    \relates QAxFactory

    Completes the QAxFactory declaration started with the QAXFACTORY_BEGIN()
    macro.

    \sa QAXFACTORY_BEGIN(), QAXCLASS(), QAXTYPE()
*/

QT_END_NAMESPACE
#endif // QT_NO_WIN_ACTIVEQT
