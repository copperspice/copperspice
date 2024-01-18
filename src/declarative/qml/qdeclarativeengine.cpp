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

#include "private/qdeclarativeengine_p.h"
#include "qdeclarativeengine.h"

#include "private/qdeclarativeboundsignal_p.h"
#include "private/qdeclarativecontext_p.h"
#include "private/qdeclarativecompiler_p.h"
#include "private/qdeclarativeglobalscriptclass_p.h"
#include "qdeclarative.h"
#include "qdeclarativecontext.h"
#include "qdeclarativeexpression.h"
#include "qdeclarativecomponent.h"
#include "private/qdeclarativebinding_p_p.h"
#include "private/qdeclarativevme_p.h"
#include "private/qdeclarativeenginedebugservice_p.h"
#include "private/qdeclarativestringconverters_p.h"
#include "private/qdeclarativexmlhttprequest_p.h"
#include "private/qdeclarativesqldatabase_p.h"
#include "private/qdeclarativetypenamescriptclass_p.h"
#include "private/qdeclarativelistscriptclass_p.h"
#include "qdeclarativescriptstring.h"
#include "private/qdeclarativeglobal_p.h"
#include "private/qdeclarativeworkerscript_p.h"
#include "private/qdeclarativecomponent_p.h"
#include "qdeclarativenetworkaccessmanagerfactory.h"
#include "qdeclarativeimageprovider.h"
#include "private/qdeclarativedirparser_p.h"
#include "qdeclarativeextensioninterface.h"
#include "private/qdeclarativelist_p.h"
#include "private/qdeclarativetypenamecache_p.h"
#include "private/qdeclarativeinclude_p.h"
#include "private/qdeclarativenotifier_p.h"
#include "private/qdeclarativedebugtrace_p.h"
#include "private/qdeclarativeapplication_p.h"
#include "private/qjsdebugservice_p.h"

#include <QtCore/qmetaobject.h>
#include <QScriptClass>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QDesktopServices>
#include <QTimer>
#include <QList>
#include <QPair>
#include <QDebug>
#include <QMetaObject>
#include <QStack>
#include <QMap>
#include <QPluginLoader>
#include <QtGui/qapplication.h>
#include <QtGui/qfontdatabase.h>
#include <QtCore/qlibraryinfo.h>
#include <QtCore/qthreadstorage.h>
#include <QtCore/qthread.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdir.h>
#include <QtCore/qmutex.h>
#include <QtGui/qcolor.h>
#include <QtGui/qvector3d.h>
#include <QtGui/qsound.h>
#include <QtCore/qcryptographichash.h>

#include <private/qscriptdeclarativeclass_p.h>
#include <private/qdeclarativeitemsmodule_p.h>
#include <private/qdeclarativeutilmodule_p.h>

#ifdef Q_OS_WIN // for %APPDATA%
#include <qt_windows.h>
#include <qlibrary.h>
#include <windows.h>

#define CSIDL_APPDATA		0x001a	// <username>\Application Data
#endif

Q_DECLARE_METATYPE(QDeclarativeProperty)

QT_BEGIN_NAMESPACE

/*!
  \qmlclass QtObject QObject
  \ingroup qml-utility-elements
  \since 4.7
  \brief The QtObject element is the most basic element in QML.

  The QtObject element is a non-visual element which contains only the
  objectName property.

  It can be useful to create a QtObject if you need an extremely
  lightweight element to enclose a set of custom properties:

  \snippet doc/src/snippets/declarative/qtobject.qml 0

  It can also be useful for C++ integration, as it is just a plain
  QObject. See the QObject documentation for further details.
*/
/*!
  \qmlproperty string QML:QtObject::objectName
  This property holds the QObject::objectName for this specific object instance.

  This allows a C++ application to locate an item within a QML component
  using the QObject::findChild() method. For example, the following C++
  application locates the child \l Rectangle item and dynamically changes its
  \c color value:

    \qml
    // MyRect.qml

    import QtQuick 1.0

    Item {
        width: 200; height: 200

        Rectangle {
            anchors.fill: parent
            color: "red"
            objectName: "myRect"
        }
    }
    \endqml

    \code
    // main.cpp

    QDeclarativeView view;
    view.setSource(QUrl::fromLocalFile("MyRect.qml"));
    view.show();

    QDeclarativeItem *item = view.rootObject()->findChild<QDeclarativeItem*>("myRect");
    if (item)
        item->setProperty("color", QColor(Qt::yellow));
    \endcode
*/

struct StaticQtMetaObject : public QObject {
   static const QMetaObject *get() {
      return &static_cast<StaticQtMetaObject *> (0)->staticQtMetaObject;
   }
};

static bool qt_QmlQtModule_registered = false;
bool QDeclarativeEnginePrivate::qml_debugging_enabled = false;

void QDeclarativeEnginePrivate::defineModule()
{
   qmlRegisterType<QDeclarativeComponent>("QtQuick", 1, 0, "Component");
   qmlRegisterType<QObject>("QtQuick", 1, 0, "QtObject");
   qmlRegisterType<QDeclarativeWorkerScript>("QtQuick", 1, 0, "WorkerScript");

   qmlRegisterType<QDeclarativeBinding>();
}

void QDeclarativeEnginePrivate::defineModuleCompat()
{
   qmlRegisterType<QDeclarativeComponent>("Qt", 4, 7, "Component");
   qmlRegisterType<QObject>("Qt", 4, 7, "QtObject");
   qmlRegisterType<QDeclarativeWorkerScript>("Qt", 4, 7, "WorkerScript");
}

/*!
\qmlclass QML:Qt QDeclarativeEnginePrivate
  \ingroup qml-utility-elements
\brief The QML global Qt object provides useful enums and functions from Qt.

\keyword QmlGlobalQtObject

\brief The \c Qt object provides useful enums and functions from Qt, for use in all QML files.

The \c Qt object is a global object with utility functions, properties and enums.

It is not instantiable; to use it, call the members of the global \c Qt object directly.
For example:

\qml
import QtQuick 1.0

Text {
    color: Qt.rgba(1, 0, 0, 1)
    text: Qt.md5("hello, world")
}
\endqml


\section1 Enums

The Qt object contains the enums available in the \l {Qt Namespace}. For example, you can access
the \l Qt::LeftButton and \l Qt::RightButton enum values as \c Qt.LeftButton and \c Qt.RightButton.


\section1 Types
The Qt object also contains helper functions for creating objects of specific
data types. This is primarily useful when setting the properties of an item
when the property has one of the following types:

\list
\o \c color - use \l{QML:Qt::rgba()}{Qt.rgba()}, \l{QML:Qt::hsla()}{Qt.hsla()}, \l{QML:Qt::darker()}{Qt.darker()}, \l{QML:Qt::lighter()}{Qt.lighter()} or \l{QML:Qt::tint()}{Qt.tint()}
\o \c rect - use \l{QML:Qt::rect()}{Qt.rect()}
\o \c point - use \l{QML:Qt::point()}{Qt.point()}
\o \c size - use \l{QML:Qt::size()}{Qt.size()}
\o \c vector3d - use \l{QML:Qt::vector3d()}{Qt.vector3d()}
\endlist

There are also string based constructors for these types. See \l{qdeclarativebasictypes.html}{QML Basic Types} for more information.

\section1 Date/Time Formatters

The Qt object contains several functions for formatting QDateTime, QDate and QTime values.

\list
    \o \l{QML:Qt::formatDateTime}{string Qt.formatDateTime(datetime date, variant format)}
    \o \l{QML:Qt::formatDate}{string Qt.formatDate(datetime date, variant format)}
    \o \l{QML:Qt::formatTime}{string Qt.formatTime(datetime date, variant format)}
\endlist

The format specification is described at \l{QML:Qt::formatDateTime}{Qt.formatDateTime}.


\section1 Dynamic Object Creation
The following functions on the global object allow you to dynamically create QML
items from files or strings. See \l{Dynamic Object Management in QML} for an overview
of their use.

\list
    \o \l{QML:Qt::createComponent()}{object Qt.createComponent(url)}
    \o \l{QML:Qt::createQmlObject()}{object Qt.createQmlObject(string qml, object parent, string filepath)}
\endlist
*/


/*!
    \qmlproperty object QML:Qt::application
    \since QtQuick 1.1

    The \c application object provides access to global application state
    properties shared by many QML components.

    Its properties are:

    \table
    \row
    \o \c application.active
    \o
    This read-only property indicates whether the application is the top-most and focused
    application, and the user is able to interact with the application. The property
    is false when the application is in the background, the device keylock or screen
    saver is active, the screen backlight is turned off, or the global system dialog
    is being displayed on top of the application. It can be used for stopping and
    pausing animations, timers and active processing of data in order to save device
    battery power and free device memory and processor load when the application is not
    active.

    \row
    \o \c application.layoutDirection
    \o
    This read-only property can be used to query the default layout direction of the
    application. On system start-up, the default layout direction depends on the
    application's language. The property has a value of \c Qt.RightToLeft in locales
    where text and graphic elements are read from right to left, and \c Qt.LeftToRight
    where the reading direction flows from left to right. You can bind to this
    property to customize your application layouts to support both layout directions.

    Possible values are:

    \list
    \o Qt.LeftToRight - Text and graphics elements should be positioned
                        from left to right.
    \o Qt.RightToLeft - Text and graphics elements should be positioned
                        from right to left.
    \endlist
    \endtable

    The following example uses the \c application object to indicate
    whether the application is currently active:

    \snippet doc/src/snippets/declarative/application.qml document

*/


/*!
\qmlmethod object Qt::include(string url, jsobject callback)

Includes another JavaScript file. This method can only be used from within JavaScript files,
and not regular QML files.

This imports all functions from \a url into the current script's namespace.

Qt.include() returns an object that describes the status of the operation.  The object has
a single property, \c {status}, that is set to one of the following values:

\table
\header \o Symbol \o Value \o Description
\row \o result.OK \o 0 \o The include completed successfully.
\row \o result.LOADING \o 1 \o Data is being loaded from the network.
\row \o result.NETWORK_ERROR \o 2 \o A network error occurred while fetching the url.
\row \o result.EXCEPTION \o 3 \o A JavaScript exception occurred while executing the included code.
An additional \c exception property will be set in this case.
\endtable

The \c status property will be updated as the operation progresses.

If provided, \a callback is invoked when the operation completes.  The callback is passed
the same object as is returned from the Qt.include() call.
*/
// Qt.include() is implemented in qdeclarativeinclude.cpp


QDeclarativeEnginePrivate::QDeclarativeEnginePrivate(QDeclarativeEngine *e)
   : captureProperties(false), rootContext(0), isDebugging(false),
     outputWarningsToStdErr(true), contextClass(0), sharedContext(0), sharedScope(0),
     objectClass(0), valueTypeClass(0), globalClass(0), cleanup(0), erroredBindings(0),
     inProgressCreations(0), scriptEngine(this), workerScriptEngine(0), componentAttached(0),
     inBeginCreate(false), networkAccessManager(0), networkAccessManagerFactory(0),
     typeLoader(e), importDatabase(e), uniqueId(1)
{
   if (!qt_QmlQtModule_registered) {
      qt_QmlQtModule_registered = true;
      QDeclarativeEnginePrivate::defineModule();
      QDeclarativeItemModule::defineModule();
      QDeclarativeValueTypeFactory::registerValueTypes();
      QDeclarativeUtilModule::defineModule();
   }
   globalClass = new QDeclarativeGlobalScriptClass(&scriptEngine);
}

/*!
  \qmlmethod url Qt::resolvedUrl(url url)
  Returns \a url resolved relative to the URL of the caller.
*/
QUrl QDeclarativeScriptEngine::resolvedUrl(QScriptContext *context, const QUrl &url)
{
   if (p) {
      QDeclarativeContextData *ctxt = p->getContext(context);
      if (ctxt) {
         return ctxt->resolvedUrl(url);
      } else {
         return p->getUrl(context).resolved(url);
      }
   }
   return baseUrl.resolved(url);
}

QDeclarativeScriptEngine::QDeclarativeScriptEngine(QDeclarativeEnginePrivate *priv)
   : p(priv), sqlQueryClass(0), namedNodeMapClass(0), nodeListClass(0)
{
   // Note that all documentation for stuff put on the global object goes in
   // doc/src/declarative/globalobject.qdoc

   bool mainthread = priv != 0;

   QScriptValue qtObject =
      newQMetaObject(StaticQtMetaObject::get());
   globalObject().setProperty(QLatin1String("Qt"), qtObject);

#ifndef QT_NO_DESKTOPSERVICES
   offlineStoragePath = QDesktopServices::storageLocation(QDesktopServices::DataLocation).replace(QLatin1Char('/'),
                        QDir::separator())
                        + QDir::separator() + QLatin1String("QML")
                        + QDir::separator() + QLatin1String("OfflineStorage");
#endif

#ifndef QT_NO_XMLSTREAMREADER
   qt_add_qmlxmlhttprequest(this);
#endif
   qt_add_qmlsqldatabase(this);
   // XXX A Multimedia "Qt.Sound" class also needs to be made available,
   // XXX but we don't want a dependency in that cirection.
   // XXX When the above a done some better way, that way should also be
   // XXX used to add Qt.Sound class.

   //types
   if (mainthread) {
      qtObject.setProperty(QLatin1String("include"), newFunction(QDeclarativeInclude::include, 2));
   } else {
      qtObject.setProperty(QLatin1String("include"), newFunction(QDeclarativeInclude::worker_include, 2));
   }

   qtObject.setProperty(QLatin1String("isQtObject"), newFunction(QDeclarativeEnginePrivate::isQtObject, 1));
   qtObject.setProperty(QLatin1String("rgba"), newFunction(QDeclarativeEnginePrivate::rgba, 4));
   qtObject.setProperty(QLatin1String("hsla"), newFunction(QDeclarativeEnginePrivate::hsla, 4));
   qtObject.setProperty(QLatin1String("rect"), newFunction(QDeclarativeEnginePrivate::rect, 4));
   qtObject.setProperty(QLatin1String("point"), newFunction(QDeclarativeEnginePrivate::point, 2));
   qtObject.setProperty(QLatin1String("size"), newFunction(QDeclarativeEnginePrivate::size, 2));
   qtObject.setProperty(QLatin1String("vector3d"), newFunction(QDeclarativeEnginePrivate::vector3d, 3));

   if (mainthread) {
      //color helpers
      qtObject.setProperty(QLatin1String("lighter"), newFunction(QDeclarativeEnginePrivate::lighter, 1));
      qtObject.setProperty(QLatin1String("darker"), newFunction(QDeclarativeEnginePrivate::darker, 1));
      qtObject.setProperty(QLatin1String("tint"), newFunction(QDeclarativeEnginePrivate::tint, 2));
   }

#ifndef QT_NO_DATESTRING
   //date/time formatting
   qtObject.setProperty(QLatin1String("formatDate"), newFunction(QDeclarativeEnginePrivate::formatDate, 2));
   qtObject.setProperty(QLatin1String("formatTime"), newFunction(QDeclarativeEnginePrivate::formatTime, 2));
   qtObject.setProperty(QLatin1String("formatDateTime"), newFunction(QDeclarativeEnginePrivate::formatDateTime, 2));
#endif

   //misc methods
   qtObject.setProperty(QLatin1String("openUrlExternally"), newFunction(QDeclarativeEnginePrivate::desktopOpenUrl, 1));
   qtObject.setProperty(QLatin1String("fontFamilies"), newFunction(QDeclarativeEnginePrivate::fontFamilies, 0));
   qtObject.setProperty(QLatin1String("md5"), newFunction(QDeclarativeEnginePrivate::md5, 1));
   qtObject.setProperty(QLatin1String("btoa"), newFunction(QDeclarativeEnginePrivate::btoa, 1));
   qtObject.setProperty(QLatin1String("atob"), newFunction(QDeclarativeEnginePrivate::atob, 1));
   qtObject.setProperty(QLatin1String("quit"), newFunction(QDeclarativeEnginePrivate::quit, 0));
   qtObject.setProperty(QLatin1String("resolvedUrl"), newFunction(QDeclarativeScriptEngine::resolvedUrl, 1));

   if (mainthread) {
      qtObject.setProperty(QLatin1String("createQmlObject"),
                           newFunction(QDeclarativeEnginePrivate::createQmlObject, 1));
      qtObject.setProperty(QLatin1String("createComponent"),
                           newFunction(QDeclarativeEnginePrivate::createComponent, 1));
   }

   //firebug/webkit compat
   QScriptValue consoleObject = newObject();
   consoleObject.setProperty(QLatin1String("log"), newFunction(QDeclarativeEnginePrivate::consoleLog, 1));
   consoleObject.setProperty(QLatin1String("debug"), newFunction(QDeclarativeEnginePrivate::consoleLog, 1));
   globalObject().setProperty(QLatin1String("console"), consoleObject);

   // translation functions need to be installed
   // before the global script class is constructed (QTBUG-6437)
   installTranslatorFunctions();
}

QDeclarativeScriptEngine::~QDeclarativeScriptEngine()
{
   delete sqlQueryClass;
   delete nodeListClass;
   delete namedNodeMapClass;
}

QScriptValue QDeclarativeScriptEngine::resolvedUrl(QScriptContext *ctxt, QScriptEngine *engine)
{
   QString arg = ctxt->argument(0).toString();
   QUrl r = QDeclarativeScriptEngine::get(engine)->resolvedUrl(ctxt, QUrl(arg));
   return QScriptValue(r.toString());
}

QNetworkAccessManager *QDeclarativeScriptEngine::networkAccessManager()
{
   return p->getNetworkAccessManager();
}

QDeclarativeEnginePrivate::~QDeclarativeEnginePrivate()
{
   Q_ASSERT(inProgressCreations == 0);
   Q_ASSERT(bindValues.isEmpty());
   Q_ASSERT(parserStatus.isEmpty());

   while (cleanup) {
      QDeclarativeCleanup *c = cleanup;
      cleanup = c->next;
      if (cleanup) {
         cleanup->prev = &cleanup;
      }
      c->next = 0;
      c->prev = 0;
      c->clear();
   }

   delete rootContext;
   rootContext = 0;
   delete contextClass;
   contextClass = 0;
   delete objectClass;
   objectClass = 0;
   delete valueTypeClass;
   valueTypeClass = 0;
   delete typeNameClass;
   typeNameClass = 0;
   delete listClass;
   listClass = 0;
   delete globalClass;
   globalClass = 0;

   for (QHash<int, QDeclarativeCompiledData *>::ConstIterator iter = m_compositeTypes.constBegin();
         iter != m_compositeTypes.constEnd(); ++iter) {
      (*iter)->release();
   }
   for (QHash<const QMetaObject *, QDeclarativePropertyCache *>::Iterator iter = propertyCache.begin();
         iter != propertyCache.end(); ++iter) {
      (*iter)->release();
   }
   for (QHash<QPair<QDeclarativeType *, int>, QDeclarativePropertyCache *>::Iterator iter = typePropertyCache.begin();
         iter != typePropertyCache.end(); ++iter) {
      (*iter)->release();
   }

}

void QDeclarativeEnginePrivate::clear(SimpleList<QDeclarativeAbstractBinding> &bvs)
{
   bvs.clear();
}

void QDeclarativeEnginePrivate::clear(SimpleList<QDeclarativeParserStatus> &pss)
{
   for (int ii = 0; ii < pss.count; ++ii) {
      QDeclarativeParserStatus *ps = pss.at(ii);
      if (ps) {
         ps->d = 0;
      }
   }
   pss.clear();
}

void QDeclarativePrivate::qdeclarativeelement_destructor(QObject *o)
{
   QObjectPrivate *p = QObjectPrivate::get(o);
   if (p->declarativeData) {
      QDeclarativeData *d = static_cast<QDeclarativeData *>(p->declarativeData);
      if (d->ownContext && d->context) {
         d->context->destroy();
         d->context = 0;
      }

      // Disconnect the notifiers now - during object destruction this would be too late, since
      // the disconnect call wouldn't be able to call disconnectNotify(), as it isn't possible to
      // get the metaobject anymore.
      d->disconnectNotifiers();
   }
}

void QDeclarativeData::destroyed(CSAbstractDeclarativeData *d, QObject *o)
{
   static_cast<QDeclarativeData *>(d)->destroyed(o);
}

void QDeclarativeData::parentChanged(CSAbstractDeclarativeData *d, QObject *o, QObject *p)
{
   static_cast<QDeclarativeData *>(d)->parentChanged(o, p);
}

void QDeclarativeEnginePrivate::init()
{
   Q_Q(QDeclarativeEngine);
   qRegisterMetaType<QVariant>("QVariant");
   qRegisterMetaType<QDeclarativeScriptString>("QDeclarativeScriptString");
   qRegisterMetaType<QScriptValue>("QScriptValue");
   qRegisterMetaType<QDeclarativeComponent::Status>("QDeclarativeComponent::Status");

   QDeclarativeData::init();

   contextClass = new QDeclarativeContextScriptClass(q);
   objectClass = new QDeclarativeObjectScriptClass(q);
   valueTypeClass = new QDeclarativeValueTypeScriptClass(q);
   typeNameClass = new QDeclarativeTypeNameScriptClass(q);
   listClass = new QDeclarativeListScriptClass(q);
   rootContext = new QDeclarativeContext(q, true);

   QScriptValue applicationObject = objectClass->newQObject(new QDeclarativeApplication(q));
   scriptEngine.globalObject().property(QLatin1String("Qt")).setProperty(QLatin1String("application"), applicationObject);

   if (QCoreApplication::instance()->thread() == q->thread() &&
         QDeclarativeEngineDebugService::isDebuggingEnabled()) {
      isDebugging = true;
      QDeclarativeEngineDebugService::instance()->addEngine(q);
      QJSDebugService::instance()->addEngine(q);
   }
}

QDeclarativeWorkerScriptEngine *QDeclarativeEnginePrivate::getWorkerScriptEngine()
{
   Q_Q(QDeclarativeEngine);
   if (!workerScriptEngine) {
      workerScriptEngine = new QDeclarativeWorkerScriptEngine(q);
   }
   return workerScriptEngine;
}

/*!
  \class QDeclarativeEngine
  \since 4.7
  \brief The QDeclarativeEngine class provides an environment for instantiating QML components.
  \mainclass

  Each QML component is instantiated in a QDeclarativeContext.
  QDeclarativeContext's are essential for passing data to QML
  components.  In QML, contexts are arranged hierarchically and this
  hierarchy is managed by the QDeclarativeEngine.

  Prior to creating any QML components, an application must have
  created a QDeclarativeEngine to gain access to a QML context.  The
  following example shows how to create a simple Text item.

  \code
  QDeclarativeEngine engine;
  QDeclarativeComponent component(&engine);
  component.setData("import QtQuick 1.0\nText { text: \"Hello world!\" }", QUrl());
  QDeclarativeItem *item = qobject_cast<QDeclarativeItem *>(component.create());

  //add item to view, etc
  ...
  \endcode

  In this case, the Text item will be created in the engine's
  \l {QDeclarativeEngine::rootContext()}{root context}.

  \sa QDeclarativeComponent QDeclarativeContext
*/

/*!
  Create a new QDeclarativeEngine with the given \a parent.
*/
QDeclarativeEngine::QDeclarativeEngine(QObject *parent)
   : QObject(parent), d_ptr(new QDeclarativeEnginePrivate(this))
{
   Q_D(QDeclarativeEngine);
   d->init();
}

/*!
  Destroys the QDeclarativeEngine.

  Any QDeclarativeContext's created on this engine will be
  invalidated, but not destroyed (unless they are parented to the
  QDeclarativeEngine object).
*/
QDeclarativeEngine::~QDeclarativeEngine()
{
   Q_D(QDeclarativeEngine);
   if (d->isDebugging) {
      QDeclarativeEngineDebugService::instance()->remEngine(this);
      QJSDebugService::instance()->removeEngine(this);
   }
}

/*! \fn void QDeclarativeEngine::quit()
    This signal is emitted when the QML loaded by the engine would like to quit.
 */

/*! \fn void QDeclarativeEngine::warnings(const QList<QDeclarativeError> &warnings)
    This signal is emitted when \a warnings messages are generated by QML.
 */

/*!
  Clears the engine's internal component cache.

  Normally the QDeclarativeEngine caches components loaded from qml
  files.  This method clears this cache and forces the component to be
  reloaded.
 */
void QDeclarativeEngine::clearComponentCache()
{
   Q_D(QDeclarativeEngine);
   d->typeLoader.clearCache();
}

/*!
  Returns the engine's root context.

  The root context is automatically created by the QDeclarativeEngine.
  Data that should be available to all QML component instances
  instantiated by the engine should be put in the root context.

  Additional data that should only be available to a subset of
  component instances should be added to sub-contexts parented to the
  root context.
*/
QDeclarativeContext *QDeclarativeEngine::rootContext() const
{
   Q_D(const QDeclarativeEngine);
   return d->rootContext;
}

/*!
  Sets the \a factory to use for creating QNetworkAccessManager(s).

  QNetworkAccessManager is used for all network access by QML.  By
  implementing a factory it is possible to create custom
  QNetworkAccessManager with specialized caching, proxy and cookie
  support.

  The factory must be set before executing the engine.
*/
void QDeclarativeEngine::setNetworkAccessManagerFactory(QDeclarativeNetworkAccessManagerFactory *factory)
{
   Q_D(QDeclarativeEngine);
   QMutexLocker locker(&d->mutex);
   d->networkAccessManagerFactory = factory;
}

/*!
  Returns the current QDeclarativeNetworkAccessManagerFactory.

  \sa setNetworkAccessManagerFactory()
*/
QDeclarativeNetworkAccessManagerFactory *QDeclarativeEngine::networkAccessManagerFactory() const
{
   Q_D(const QDeclarativeEngine);
   return d->networkAccessManagerFactory;
}

QNetworkAccessManager *QDeclarativeEnginePrivate::createNetworkAccessManager(QObject *parent) const
{
   QMutexLocker locker(&mutex);
   QNetworkAccessManager *nam;
   if (networkAccessManagerFactory) {
      nam = networkAccessManagerFactory->create(parent);
   } else {
      nam = new QNetworkAccessManager(parent);
   }

   return nam;
}

QNetworkAccessManager *QDeclarativeEnginePrivate::getNetworkAccessManager() const
{
   Q_Q(const QDeclarativeEngine);
   if (!networkAccessManager) {
      networkAccessManager = createNetworkAccessManager(const_cast<QDeclarativeEngine *>(q));
   }
   return networkAccessManager;
}

/*!
  Returns a common QNetworkAccessManager which can be used by any QML
  element instantiated by this engine.

  If a QDeclarativeNetworkAccessManagerFactory has been set and a
  QNetworkAccessManager has not yet been created, the
  QDeclarativeNetworkAccessManagerFactory will be used to create the
  QNetworkAccessManager; otherwise the returned QNetworkAccessManager
  will have no proxy or cache set.

  \sa setNetworkAccessManagerFactory()
*/
QNetworkAccessManager *QDeclarativeEngine::networkAccessManager() const
{
   Q_D(const QDeclarativeEngine);
   return d->getNetworkAccessManager();
}

/*!

  Sets the \a provider to use for images requested via the \e
  image: url scheme, with host \a providerId. The QDeclarativeEngine
  takes ownership of \a provider.

  Image providers enable support for pixmap and threaded image
  requests. See the QDeclarativeImageProvider documentation for details on
  implementing and using image providers.

  All required image providers should be added to the engine before any
  QML sources files are loaded.

  \sa removeImageProvider()
*/
void QDeclarativeEngine::addImageProvider(const QString &providerId, QDeclarativeImageProvider *provider)
{
   Q_D(QDeclarativeEngine);
   QMutexLocker locker(&d->mutex);
   d->imageProviders.insert(providerId.toLower(), QSharedPointer<QDeclarativeImageProvider>(provider));
}

/*!
  Returns the QDeclarativeImageProvider set for \a providerId.
*/
QDeclarativeImageProvider *QDeclarativeEngine::imageProvider(const QString &providerId) const
{
   Q_D(const QDeclarativeEngine);
   QMutexLocker locker(&d->mutex);
   return d->imageProviders.value(providerId).data();
}

/*!
  Removes the QDeclarativeImageProvider for \a providerId.

  Returns the provider if it was found; otherwise returns 0.

  \sa addImageProvider()
*/
void QDeclarativeEngine::removeImageProvider(const QString &providerId)
{
   Q_D(QDeclarativeEngine);
   QMutexLocker locker(&d->mutex);
   d->imageProviders.take(providerId);
}

QDeclarativeImageProvider::ImageType QDeclarativeEnginePrivate::getImageProviderType(const QUrl &url)
{
   QMutexLocker locker(&mutex);
   QSharedPointer<QDeclarativeImageProvider> provider = imageProviders.value(url.host());
   locker.unlock();
   if (provider) {
      return provider->imageType();
   }
   return static_cast<QDeclarativeImageProvider::ImageType>(-1);
}

QImage QDeclarativeEnginePrivate::getImageFromProvider(const QUrl &url, QSize *size, const QSize &req_size)
{
   QMutexLocker locker(&mutex);
   QImage image;
   QSharedPointer<QDeclarativeImageProvider> provider = imageProviders.value(url.host());
   locker.unlock();
   if (provider) {
      QString imageId = url.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority).mid(1);
      image = provider->requestImage(imageId, size, req_size);
   }
   return image;
}

QPixmap QDeclarativeEnginePrivate::getPixmapFromProvider(const QUrl &url, QSize *size, const QSize &req_size)
{
   QMutexLocker locker(&mutex);
   QPixmap pixmap;
   QSharedPointer<QDeclarativeImageProvider> provider = imageProviders.value(url.host());
   locker.unlock();
   if (provider) {
      QString imageId = url.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority).mid(1);
      pixmap = provider->requestPixmap(imageId, size, req_size);
   }
   return pixmap;
}

/*!
  Return the base URL for this engine.  The base URL is only used to
  resolve components when a relative URL is passed to the
  QDeclarativeComponent constructor.

  If a base URL has not been explicitly set, this method returns the
  application's current working directory.

  \sa setBaseUrl()
*/
QUrl QDeclarativeEngine::baseUrl() const
{
   Q_D(const QDeclarativeEngine);
   if (d->baseUrl.isEmpty()) {
      return QUrl::fromLocalFile(QDir::currentPath() + QDir::separator());
   } else {
      return d->baseUrl;
   }
}

/*!
  Set the  base URL for this engine to \a url.

  \sa baseUrl()
*/
void QDeclarativeEngine::setBaseUrl(const QUrl &url)
{
   Q_D(QDeclarativeEngine);
   d->baseUrl = url;
}

/*!
  Returns true if warning messages will be output to stderr in addition
  to being emitted by the warnings() signal, otherwise false.

  The default value is true.
*/
bool QDeclarativeEngine::outputWarningsToStandardError() const
{
   Q_D(const QDeclarativeEngine);
   return d->outputWarningsToStdErr;
}

/*!
  Set whether warning messages will be output to stderr to \a enabled.

  If \a enabled is true, any warning messages generated by QML will be
  output to stderr and emitted by the warnings() signal.  If \a enabled
  is false, on the warnings() signal will be emitted.  This allows
  applications to handle warning output themselves.

  The default value is true.
*/
void QDeclarativeEngine::setOutputWarningsToStandardError(bool enabled)
{
   Q_D(QDeclarativeEngine);
   d->outputWarningsToStdErr = enabled;
}

/*!
  Returns the QDeclarativeContext for the \a object, or 0 if no
  context has been set.

  When the QDeclarativeEngine instantiates a QObject, the context is
  set automatically.
  */
QDeclarativeContext *QDeclarativeEngine::contextForObject(const QObject *object)
{
   if (!object) {
      return 0;
   }

   QObjectPrivate *priv = QObjectPrivate::get(const_cast<QObject *>(object));

   QDeclarativeData *data =
      static_cast<QDeclarativeData *>(priv->declarativeData);

   if (!data) {
      return 0;
   } else if (data->outerContext) {
      return data->outerContext->asQDeclarativeContext();
   } else {
      return 0;
   }
}

/*!
  Sets the QDeclarativeContext for the \a object to \a context.
  If the \a object already has a context, a warning is
  output, but the context is not changed.

  When the QDeclarativeEngine instantiates a QObject, the context is
  set automatically.
 */
void QDeclarativeEngine::setContextForObject(QObject *object, QDeclarativeContext *context)
{
   if (!object || !context) {
      return;
   }

   QDeclarativeData *data = QDeclarativeData::get(object, true);
   if (data->context) {
      qWarning("QDeclarativeEngine::setContextForObject(): Object already has a QDeclarativeContext");
      return;
   }

   QDeclarativeContextData *contextData = QDeclarativeContextData::get(context);
   contextData->addObject(object);
}

/*!
  \enum QDeclarativeEngine::ObjectOwnership

  Ownership controls whether or not QML automatically destroys the
  QObject when the object is garbage collected by the JavaScript
  engine.  The two ownership options are:

  \value CppOwnership The object is owned by C++ code, and will
  never be deleted by QML.  The JavaScript destroy() method cannot be
  used on objects with CppOwnership.  This option is similar to
  QScriptEngine::QtOwnership.

  \value JavaScriptOwnership The object is owned by JavaScript.
  When the object is returned to QML as the return value of a method
  call or property access, QML will delete the object if there are no
  remaining JavaScript references to it and it has no
  QObject::parent().  This option is similar to
  QScriptEngine::ScriptOwnership.

  Generally an application doesn't need to set an object's ownership
  explicitly.  QML uses a heuristic to set the default object
  ownership.  By default, an object that is created by QML has
  JavaScriptOwnership.  The exception to this are the root objects
  created by calling QDeclarativeCompnent::create() or
  QDeclarativeComponent::beginCreate() which have CppOwnership by
  default.  The ownership of these root-level objects is considered to
  have been transferred to the C++ caller.

  Objects not-created by QML have CppOwnership by default.  The
  exception to this is objects returned from a C++ method call.  The
  ownership of these objects is passed to JavaScript.

  Calling setObjectOwnership() overrides the default ownership
  heuristic used by QML.
*/

/*!
  Sets the \a ownership of \a object.
*/
void QDeclarativeEngine::setObjectOwnership(QObject *object, ObjectOwnership ownership)
{
   if (!object) {
      return;
   }

   QDeclarativeData *ddata = QDeclarativeData::get(object, true);
   if (!ddata) {
      return;
   }

   ddata->indestructible = (ownership == CppOwnership) ? true : false;
   ddata->explicitIndestructibleSet = true;
}

/*!
  Returns the ownership of \a object.
*/
QDeclarativeEngine::ObjectOwnership QDeclarativeEngine::objectOwnership(QObject *object)
{
   if (!object) {
      return CppOwnership;
   }

   QDeclarativeData *ddata = QDeclarativeData::get(object, false);
   if (!ddata) {
      return CppOwnership;
   } else {
      return ddata->indestructible ? CppOwnership : JavaScriptOwnership;
   }
}

void qmlExecuteDeferred(QObject *object)
{
   QDeclarativeData *data = QDeclarativeData::get(object);

   if (data && data->deferredComponent) {
      if (QDeclarativeDebugService::isDebuggingEnabled()) {
         QDeclarativeDebugTrace::startRange(QDeclarativeDebugTrace::Creating);
         QDeclarativeType *type = QDeclarativeMetaType::qmlType(object->metaObject());
         QString typeName = type ? QLatin1String(type->qmlTypeName()) : QString::fromLatin1(object->metaObject()->className());
         QDeclarativeDebugTrace::rangeData(QDeclarativeDebugTrace::Creating, typeName);
         if (data->outerContext) {
            QDeclarativeDebugTrace::rangeLocation(QDeclarativeDebugTrace::Creating, data->outerContext->url, data->lineNumber);
         }
      }
      QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(data->context->engine);

      QDeclarativeComponentPrivate::ConstructionState state;
      QDeclarativeComponentPrivate::beginDeferred(ep, object, &state);

      data->deferredComponent->release();
      data->deferredComponent = 0;

      QDeclarativeComponentPrivate::complete(ep, &state);
      QDeclarativeDebugTrace::endRange(QDeclarativeDebugTrace::Creating);
   }
}

QDeclarativeContext *qmlContext(const QObject *obj)
{
   return QDeclarativeEngine::contextForObject(obj);
}

QDeclarativeEngine *qmlEngine(const QObject *obj)
{
   QDeclarativeContext *context = QDeclarativeEngine::contextForObject(obj);
   return context ? context->engine() : 0;
}

QObject *qmlAttachedPropertiesObjectById(int id, const QObject *object, bool create)
{
   QDeclarativeData *data = QDeclarativeData::get(object);
   if (!data) {
      return 0;   // Attached properties are only on objects created by QML
   }

   QObject *rv = data->hasExtendedData() ? data->attachedProperties()->value(id) : 0;
   if (rv || !create) {
      return rv;
   }

   QDeclarativeAttachedPropertiesFunc pf = QDeclarativeMetaType::attachedPropertiesFuncById(id);
   if (!pf) {
      return 0;
   }

   rv = pf(const_cast<QObject *>(object));

   if (rv) {
      data->attachedProperties()->insert(id, rv);
   }

   return rv;
}

QObject *qmlAttachedPropertiesObject(int *idCache, const QObject *object,
                                     const QMetaObject *attachedMetaObject, bool create)
{
   if (*idCache == -1) {
      *idCache = QDeclarativeMetaType::attachedPropertiesFuncId(attachedMetaObject);
   }

   if (*idCache == -1 || !object) {
      return 0;
   }

   return qmlAttachedPropertiesObjectById(*idCache, object, create);
}

QDeclarativeDebuggingEnabler::QDeclarativeDebuggingEnabler()
{
#ifndef QDECLARATIVE_NO_DEBUG_PROTOCOL
   if (!QDeclarativeEnginePrivate::qml_debugging_enabled) {
      qDebug("Qml debugging is enabled. Only use this in a safe environment!");
   }
   QDeclarativeEnginePrivate::qml_debugging_enabled = true;
#endif
}


class QDeclarativeDataExtended
{
 public:
   QDeclarativeDataExtended();
   ~QDeclarativeDataExtended();

   QHash<int, QObject *> attachedProperties;
   QDeclarativeNotifier objectNameNotifier;
   QList<QDeclarativeAbstractBoundSignal *> boundSignals;
};

QDeclarativeDataExtended::QDeclarativeDataExtended()
{
}

QDeclarativeDataExtended::~QDeclarativeDataExtended()
{
}

QDeclarativeNotifier *QDeclarativeData::objectNameNotifier() const
{
   if (!extendedData) {
      extendedData = new QDeclarativeDataExtended;
   }
   return &extendedData->objectNameNotifier;
}

QHash<int, QObject *> *QDeclarativeData::attachedProperties() const
{
   if (!extendedData) {
      extendedData = new QDeclarativeDataExtended;
   }
   return &extendedData->attachedProperties;
}

void QDeclarativeData::addBoundSignal(QDeclarativeAbstractBoundSignal *signal)
{
   if (!extendedData) {
      extendedData = new QDeclarativeDataExtended;
   }
   extendedData->boundSignals.append(signal);
}

void QDeclarativeData::removeBoundSignal(QDeclarativeAbstractBoundSignal *signal)
{
   if (extendedData) {
      extendedData->boundSignals.removeAll(signal);
   }
}

void QDeclarativeData::disconnectNotifiers()
{
   QDeclarativeAbstractBinding *binding = bindings;
   while (binding) {
      binding->disconnect(QDeclarativeAbstractBinding::DisconnectAll);
      binding = binding->m_nextBinding;
   }

   if (extendedData) {
      for(QDeclarativeAbstractBoundSignal * signal : extendedData->boundSignals){
         signal->disconnect();
      }
   }
}

void QDeclarativeData::destroyed(QObject *object)
{
   if (deferredComponent) {
      deferredComponent->release();
   }

   if (nextContextObject) {
      nextContextObject->prevContextObject = prevContextObject;
   }
   if (prevContextObject) {
      *prevContextObject = nextContextObject;
   }

   QDeclarativeAbstractBinding *binding = bindings;
   while (binding) {
      QDeclarativeAbstractBinding *next = binding->m_nextBinding;
      binding->m_prevBinding = 0;
      binding->m_nextBinding = 0;
      binding->destroy(QDeclarativeAbstractBinding::KeepBindingConnected);
      binding = next;
   }

   if (bindingBits) {
      free(bindingBits);
   }

   if (propertyCache) {
      propertyCache->release();
   }

   if (ownContext && context) {
      context->destroy();
   }

   while (guards) {
      QDeclarativeGuard<QObject> *guard = static_cast<QDeclarativeGuard<QObject> *>(guards);
      *guard = (QObject *)0;
      guard->objectDestroyed(object);
   }

   if (scriptValue) {
      delete scriptValue;
   }

   if (extendedData) {
      delete extendedData;
   }

   if (ownMemory) {
      delete this;
   }
}

void QDeclarativeData::parentChanged(QObject *, QObject *parent)
{
   if (!parent && scriptValue) {
      delete scriptValue;
      scriptValue = 0;
   }
}

void QDeclarativeData::objectNameChanged(QObject *)
{
   if (extendedData) {
      objectNameNotifier()->notify();
   }
}

bool QDeclarativeData::hasBindingBit(int bit) const
{
   if (bindingBitsSize > bit) {
      return bindingBits[bit / 32] & (1 << (bit % 32));
   } else {
      return false;
   }
}

void QDeclarativeData::clearBindingBit(int bit)
{
   if (bindingBitsSize > bit) {
      bindingBits[bit / 32] &= ~(1 << (bit % 32));
   }
}

void QDeclarativeData::setBindingBit(QObject *obj, int bit)
{
   if (bindingBitsSize <= bit) {
      int props = obj->metaObject()->propertyCount();
      Q_ASSERT(bit < props);

      int arraySize = (props + 31) / 32;
      int oldArraySize = bindingBitsSize / 32;

      bindingBits = (quint32 *)q_check_ptr(realloc(bindingBits,
                                           arraySize * sizeof(quint32)));

      memset(bindingBits + oldArraySize,
             0x00,
             sizeof(quint32) * (arraySize - oldArraySize));

      bindingBitsSize = arraySize * 32;
   }

   bindingBits[bit / 32] |= (1 << (bit % 32));
}

/*!
    Creates a QScriptValue allowing you to use \a object in QML script.
    \a engine is the QDeclarativeEngine it is to be created in.

    The QScriptValue returned is a QtScript Object, not a QtScript QObject, due
    to the special needs of QML requiring more functionality than a standard
    QtScript QObject.
*/
QScriptValue QDeclarativeEnginePrivate::qmlScriptObject(QObject *object,
      QDeclarativeEngine *engine)
{
   QDeclarativeEnginePrivate *enginePriv = QDeclarativeEnginePrivate::get(engine);
   return enginePriv->objectClass->newQObject(object);
}

/*!
    Returns the QDeclarativeContext for the executing QScript \a ctxt.
*/
QDeclarativeContextData *QDeclarativeEnginePrivate::getContext(QScriptContext *ctxt)
{
   QScriptValue scopeNode = QScriptDeclarativeClass::scopeChainValue(ctxt, -3);
   Q_ASSERT(scopeNode.isValid());
   Q_ASSERT(QScriptDeclarativeClass::scriptClass(scopeNode) == contextClass);
   return contextClass->contextFromValue(scopeNode);
}

/*!
    Returns the QUrl associated with the script \a ctxt for the case that there is
    no QDeclarativeContext.
*/
QUrl QDeclarativeEnginePrivate::getUrl(QScriptContext *ctxt)
{
   QScriptValue scopeNode = QScriptDeclarativeClass::scopeChainValue(ctxt, -3);
   Q_ASSERT(scopeNode.isValid());
   Q_ASSERT(QScriptDeclarativeClass::scriptClass(scopeNode) == contextClass);
   return contextClass->urlFromValue(scopeNode);
}

QString QDeclarativeEnginePrivate::urlToLocalFileOrQrc(const QUrl &url)
{
   if (url.scheme().compare(QLatin1String("qrc"), Qt::CaseInsensitive) == 0) {
      if (url.authority().isEmpty()) {
         return QLatin1Char(':') + url.path();
      }
      return QString();
   }
   return url.toLocalFile();
}

/*!
\qmlmethod object Qt::createComponent(url)

Returns a \l Component object created using the QML file at the specified \a url,
or \c null if an empty string was given.

The returned component's \l Component::status property indicates whether the
component was successfully created. If the status is \c Component.Error,
see \l Component::errorString() for an error description.

Call \l {Component::createObject()}{Component.createObject()} on the returned
component to create an object instance of the component.

For example:

\snippet doc/src/snippets/declarative/createComponent-simple.qml 0

See \l {Dynamic Object Management in QML} for more information on using this function.

To create a QML object from an arbitrary string of QML (instead of a file),
use \l{QML:Qt::createQmlObject()}{Qt.createQmlObject()}.
*/

QScriptValue QDeclarativeEnginePrivate::createComponent(QScriptContext *ctxt, QScriptEngine *engine)
{
   QDeclarativeEnginePrivate *activeEnginePriv =
      static_cast<QDeclarativeScriptEngine *>(engine)->p;
   QDeclarativeEngine *activeEngine = activeEnginePriv->q_func();

   if (ctxt->argumentCount() != 1) {
      return ctxt->throwError(QLatin1String("Qt.createComponent(): Invalid arguments"));
   } else {

      QString arg = ctxt->argument(0).toString();
      if (arg.isEmpty()) {
         return engine->nullValue();
      }
      QUrl url = QDeclarativeScriptEngine::get(engine)->resolvedUrl(ctxt, QUrl(arg));
      QDeclarativeContextData *context = activeEnginePriv->getContext(ctxt);
      QDeclarativeComponent *c = new QDeclarativeComponent(activeEngine, url, activeEngine);
      QDeclarativeComponentPrivate::get(c)->creationContext = context;
      QDeclarativeData::get(c, true)->setImplicitDestructible();
      return activeEnginePriv->objectClass->newQObject(c, qMetaTypeId<QDeclarativeComponent *>());
   }
}

/*!
\qmlmethod object Qt::createQmlObject(string qml, object parent, string filepath)

Returns a new object created from the given \a string of QML which will have the specified \a parent,
or \c null if there was an error in creating the object.

If \a filepath is specified, it will be used for error reporting for the created object.

Example (where \c parentItem is the id of an existing QML item):

\snippet doc/src/snippets/declarative/createQmlObject.qml 0

In the case of an error, a QtScript Error object is thrown. This object has an additional property,
\c qmlErrors, which is an array of the errors encountered.
Each object in this array has the members \c lineNumber, \c columnNumber, \c fileName and \c message.
For example, if the above snippet had misspelled color as 'colro' then the array would contain an object like the following:
{ "lineNumber" : 1, "columnNumber" : 32, "fileName" : "dynamicSnippet1", "message" : "Cannot assign to non-existent property \"colro\""}.

Note that this function returns immediately, and therefore may not work if
the \a qml string loads new components (that is, external QML files that have not yet been loaded).
If this is the case, consider using \l{QML:Qt::createComponent()}{Qt.createComponent()} instead.

See \l {Dynamic Object Management in QML} for more information on using this function.
*/

QScriptValue QDeclarativeEnginePrivate::createQmlObject(QScriptContext *ctxt, QScriptEngine *engine)
{
   QDeclarativeEnginePrivate *activeEnginePriv =
      static_cast<QDeclarativeScriptEngine *>(engine)->p;
   QDeclarativeEngine *activeEngine = activeEnginePriv->q_func();

   if (ctxt->argumentCount() < 2 || ctxt->argumentCount() > 3) {
      return ctxt->throwError(QLatin1String("Qt.createQmlObject(): Invalid arguments"));
   }

   QDeclarativeContextData *context = activeEnginePriv->getContext(ctxt);
   Q_ASSERT(context);

   QString qml = ctxt->argument(0).toString();
   if (qml.isEmpty()) {
      return engine->nullValue();
   }

   QUrl url;
   if (ctxt->argumentCount() > 2) {
      url = QUrl(ctxt->argument(2).toString());
   } else {
      url = QUrl(QLatin1String("inline"));
   }

   if (url.isValid() && url.isRelative()) {
      url = context->resolvedUrl(url);
   }

   QObject *parentArg = activeEnginePriv->objectClass->toQObject(ctxt->argument(1));
   if (!parentArg) {
      return ctxt->throwError(QLatin1String("Qt.createQmlObject(): Missing parent object"));
   }

   QDeclarativeComponent component(activeEngine);
   component.setData(qml.toUtf8(), url);

   if (component.isError()) {
      QList<QDeclarativeError> errors = component.errors();
      QString errstr = QLatin1String("Qt.createQmlObject() failed to create object: ");
      QScriptValue arr = ctxt->engine()->newArray(errors.length());
      int i = 0;
      foreach (const QDeclarativeError & error, errors) {
         errstr += QLatin1String("    ") + error.toString() + QLatin1String("\n");
         QScriptValue qmlErrObject = ctxt->engine()->newObject();
         qmlErrObject.setProperty(QLatin1String("lineNumber"), QScriptValue(error.line()));
         qmlErrObject.setProperty(QLatin1String("columnNumber"), QScriptValue(error.column()));
         qmlErrObject.setProperty(QLatin1String("fileName"), QScriptValue(error.url().toString()));
         qmlErrObject.setProperty(QLatin1String("message"), QScriptValue(error.description()));
         arr.setProperty(i++, qmlErrObject);
      }
      QScriptValue err = ctxt->throwError(errstr);
      err.setProperty(QLatin1String("qmlErrors"), arr);
      return err;
   }

   if (!component.isReady()) {
      return ctxt->throwError(QLatin1String("Qt.createQmlObject(): Component is not ready"));
   }

   QObject *obj = component.beginCreate(context->asQDeclarativeContext());
   if (obj) {
      QDeclarativeData::get(obj, true)->setImplicitDestructible();
   }
   component.completeCreate();

   if (component.isError()) {
      QList<QDeclarativeError> errors = component.errors();
      QString errstr = QLatin1String("Qt.createQmlObject() failed to create object: ");
      QScriptValue arr = ctxt->engine()->newArray(errors.length());
      int i = 0;
      foreach (const QDeclarativeError & error, errors) {
         errstr += QLatin1String("    ") + error.toString() + QLatin1String("\n");
         QScriptValue qmlErrObject = ctxt->engine()->newObject();
         qmlErrObject.setProperty(QLatin1String("lineNumber"), QScriptValue(error.line()));
         qmlErrObject.setProperty(QLatin1String("columnNumber"), QScriptValue(error.column()));
         qmlErrObject.setProperty(QLatin1String("fileName"), QScriptValue(error.url().toString()));
         qmlErrObject.setProperty(QLatin1String("message"), QScriptValue(error.description()));
         arr.setProperty(i++, qmlErrObject);
      }
      QScriptValue err = ctxt->throwError(errstr);
      err.setProperty(QLatin1String("qmlErrors"), arr);
      return err;
   }

   Q_ASSERT(obj);

   obj->setParent(parentArg);

   QList<QDeclarativePrivate::AutoParentFunction> functions = QDeclarativeMetaType::parentFunctions();
   for (int ii = 0; ii < functions.count(); ++ii) {
      if (QDeclarativePrivate::Parented == functions.at(ii)(obj, parentArg)) {
         break;
      }
   }

   QDeclarativeData::get(obj, true)->setImplicitDestructible();
   return activeEnginePriv->objectClass->newQObject(obj, QMetaType::QObjectStar);
}

/*!
\qmlmethod bool Qt::isQtObject(object)
Returns true if \c object is a valid reference to a Qt or QML object, otherwise false.
*/
QScriptValue QDeclarativeEnginePrivate::isQtObject(QScriptContext *ctxt, QScriptEngine *engine)
{
   if (ctxt->argumentCount() == 0) {
      return QScriptValue(engine, false);
   }

   return QScriptValue(engine, 0 != ctxt->argument(0).toQObject());
}

/*!
\qmlmethod Qt::vector3d(real x, real y, real z)
Returns a Vector3D with the specified \c x, \c y and \c z.
*/
QScriptValue QDeclarativeEnginePrivate::vector3d(QScriptContext *ctxt, QScriptEngine *engine)
{
   if (ctxt->argumentCount() != 3) {
      return ctxt->throwError(QLatin1String("Qt.vector(): Invalid arguments"));
   }
   qsreal x = ctxt->argument(0).toNumber();
   qsreal y = ctxt->argument(1).toNumber();
   qsreal z = ctxt->argument(2).toNumber();
   return QDeclarativeEnginePrivate::get(engine)->scriptValueFromVariant(QVariant::fromValue(QVector3D(x, y, z)));
}

/*!
\qmlmethod string Qt::formatDate(datetime date, variant format)

Returns a string representation of \c date, optionally formatted according
to \c format.

The \a date parameter may be a JavaScript \c Date object, a \l{date}{date}
property, a QDate, or QDateTime value. The \a format parameter may be any of
the possible format values as described for
\l{QML:Qt::formatDateTime()}{Qt.formatDateTime()}.

If \a format is not specified, \a date is formatted using
\l {Qt::DefaultLocaleShortDate}{Qt.DefaultLocaleShortDate}.
*/
#ifndef QT_NO_DATESTRING
QScriptValue QDeclarativeEnginePrivate::formatDate(QScriptContext *ctxt, QScriptEngine *engine)
{
   int argCount = ctxt->argumentCount();
   if (argCount == 0 || argCount > 2) {
      return ctxt->throwError(QLatin1String("Qt.formatDate(): Invalid arguments"));
   }

   QDate date = ctxt->argument(0).toDateTime().date();
   Qt::DateFormat enumFormat = Qt::DefaultLocaleShortDate;
   if (argCount == 2) {
      QScriptValue formatArg = ctxt->argument(1);
      if (formatArg.isString()) {
         QString format = formatArg.toString();
         return engine->newVariant(QVariant::fromValue(date.toString(format)));
      } else if (formatArg.isNumber()) {
         enumFormat = Qt::DateFormat(formatArg.toUInt32());
      } else {
         return ctxt->throwError(QLatin1String("Qt.formatDate(): Invalid date format"));
      }
   }
   return engine->newVariant(QVariant::fromValue(date.toString(enumFormat)));
}

/*!
\qmlmethod string Qt::formatTime(datetime time, variant format)

Returns a string representation of \c time, optionally formatted according to
\c format.

The \a time parameter may be a JavaScript \c Date object, a QTime, or QDateTime
value. The \a format parameter may be any of the possible format values as
described for \l{QML:Qt::formatDateTime()}{Qt.formatDateTime()}.

If \a format is not specified, \a time is formatted using
\l {Qt::DefaultLocaleShortDate}{Qt.DefaultLocaleShortDate}.
*/
QScriptValue QDeclarativeEnginePrivate::formatTime(QScriptContext *ctxt, QScriptEngine *engine)
{
   int argCount = ctxt->argumentCount();
   if (argCount == 0 || argCount > 2) {
      return ctxt->throwError(QLatin1String("Qt.formatTime(): Invalid arguments"));
   }

   QTime time;
   QScriptValue sv = ctxt->argument(0);
   if (sv.isDate()) {
      time = sv.toDateTime().time();
   } else if (sv.toVariant().type() == QVariant::Time) {
      time = sv.toVariant().toTime();
   }

   Qt::DateFormat enumFormat = Qt::DefaultLocaleShortDate;
   if (argCount == 2) {
      QScriptValue formatArg = ctxt->argument(1);
      if (formatArg.isString()) {
         QString format = formatArg.toString();
         return engine->newVariant(QVariant::fromValue(time.toString(format)));
      } else if (formatArg.isNumber()) {
         enumFormat = Qt::DateFormat(formatArg.toUInt32());
      } else {
         return ctxt->throwError(QLatin1String("Qt.formatTime(): Invalid time format"));
      }
   }
   return engine->newVariant(QVariant::fromValue(time.toString(enumFormat)));
}

/*!
\qmlmethod string Qt::formatDateTime(datetime dateTime, variant format)

Returns a string representation of \c datetime, optionally formatted according to
\c format.

The \a date parameter may be a JavaScript \c Date object, a \l{date}{date}
property, a QDate, QTime, or QDateTime value.

If \a format is not provided, \a dateTime is formatted using
\l {Qt::DefaultLocaleShortDate}{Qt.DefaultLocaleShortDate}. Otherwise,
\a format should be either.

\list
\o One of the Qt::DateFormat enumeration values, such as
   \c Qt.DefaultLocaleShortDate or \c Qt.ISODate
\o A string that specifies the format of the returned string, as detailed below.
\endlist

If \a format specifies a format string, it should use the following expressions
to specify the date:

    \table
    \header \i Expression \i Output
    \row \i d \i the day as number without a leading zero (1 to 31)
    \row \i dd \i the day as number with a leading zero (01 to 31)
    \row \i ddd
            \i the abbreviated localized day name (e.g. 'Mon' to 'Sun').
            Uses QDate::shortDayName().
    \row \i dddd
            \i the long localized day name (e.g. 'Monday' to 'Qt::Sunday').
            Uses QDate::longDayName().
    \row \i M \i the month as number without a leading zero (1-12)
    \row \i MM \i the month as number with a leading zero (01-12)
    \row \i MMM
            \i the abbreviated localized month name (e.g. 'Jan' to 'Dec').
            Uses QDate::shortMonthName().
    \row \i MMMM
            \i the long localized month name (e.g. 'January' to 'December').
            Uses QDate::longMonthName().
    \row \i yy \i the year as two digit number (00-99)
    \row \i yyyy \i the year as four digit number
    \endtable

In addition the following expressions can be used to specify the time:

    \table
    \header \i Expression \i Output
    \row \i h
         \i the hour without a leading zero (0 to 23 or 1 to 12 if AM/PM display)
    \row \i hh
         \i the hour with a leading zero (00 to 23 or 01 to 12 if AM/PM display)
    \row \i m \i the minute without a leading zero (0 to 59)
    \row \i mm \i the minute with a leading zero (00 to 59)
    \row \i s \i the second without a leading zero (0 to 59)
    \row \i ss \i the second with a leading zero (00 to 59)
    \row \i z \i the milliseconds without leading zeroes (0 to 999)
    \row \i zzz \i the milliseconds with leading zeroes (000 to 999)
    \row \i AP
            \i use AM/PM display. \e AP will be replaced by either "AM" or "PM".
    \row \i ap
            \i use am/pm display. \e ap will be replaced by either "am" or "pm".
    \endtable

    All other input characters will be ignored. Any sequence of characters that
    are enclosed in single quotes will be treated as text and not be used as an
    expression. Two consecutive single quotes ("''") are replaced by a single quote
    in the output.

For example, if the following date/time value was specified:

    \code
    // 21 May 2001 14:13:09
    var dateTime = new Date(2001, 5, 21, 14, 13, 09)
    \endcode

This \a dateTime value could be passed to \c Qt.formatDateTime(),
\l {QML:Qt::formatDate()}{Qt.formatDate()} or \l {QML:Qt::formatTime()}{Qt.formatTime()}
with the \a format values below to produce the following results:

    \table
    \header \i Format \i Result
    \row \i "dd.MM.yyyy"      \i 21.05.2001
    \row \i "ddd MMMM d yy"   \i Tue May 21 01
    \row \i "hh:mm:ss.zzz"    \i 14:13:09.042
    \row \i "h:m:s ap"        \i 2:13:9 pm
    \endtable
*/
QScriptValue QDeclarativeEnginePrivate::formatDateTime(QScriptContext *ctxt, QScriptEngine *engine)
{
   int argCount = ctxt->argumentCount();
   if (argCount == 0 || argCount > 2) {
      return ctxt->throwError(QLatin1String("Qt.formatDateTime(): Invalid arguments"));
   }

   QDateTime date = ctxt->argument(0).toDateTime();
   Qt::DateFormat enumFormat = Qt::DefaultLocaleShortDate;
   if (argCount == 2) {
      QScriptValue formatArg = ctxt->argument(1);
      if (formatArg.isString()) {
         QString format = formatArg.toString();
         return engine->newVariant(QVariant::fromValue(date.toString(format)));
      } else if (formatArg.isNumber()) {
         enumFormat = Qt::DateFormat(formatArg.toUInt32());
      } else {
         return ctxt->throwError(QLatin1String("Qt.formatDateTime(): Invalid datetime format"));
      }
   }
   return engine->newVariant(QVariant::fromValue(date.toString(enumFormat)));
}
#endif // QT_NO_DATESTRING

/*!
\qmlmethod color Qt::rgba(real red, real green, real blue, real alpha)

Returns a color with the specified \c red, \c green, \c blue and \c alpha components.
All components should be in the range 0-1 inclusive.
*/
QScriptValue QDeclarativeEnginePrivate::rgba(QScriptContext *ctxt, QScriptEngine *engine)
{
   int argCount = ctxt->argumentCount();
   if (argCount < 3 || argCount > 4) {
      return ctxt->throwError(QLatin1String("Qt.rgba(): Invalid arguments"));
   }
   qsreal r = ctxt->argument(0).toNumber();
   qsreal g = ctxt->argument(1).toNumber();
   qsreal b = ctxt->argument(2).toNumber();
   qsreal a = (argCount == 4) ? ctxt->argument(3).toNumber() : 1;

   if (r < 0.0) {
      r = 0.0;
   }
   if (r > 1.0) {
      r = 1.0;
   }
   if (g < 0.0) {
      g = 0.0;
   }
   if (g > 1.0) {
      g = 1.0;
   }
   if (b < 0.0) {
      b = 0.0;
   }
   if (b > 1.0) {
      b = 1.0;
   }
   if (a < 0.0) {
      a = 0.0;
   }
   if (a > 1.0) {
      a = 1.0;
   }

   return engine->toScriptValue(QVariant::fromValue(QColor::fromRgbF(r, g, b, a)));
}

/*!
\qmlmethod color Qt::hsla(real hue, real saturation, real lightness, real alpha)

Returns a color with the specified \c hue, \c saturation, \c lightness and \c alpha components.
All components should be in the range 0-1 inclusive.
*/
QScriptValue QDeclarativeEnginePrivate::hsla(QScriptContext *ctxt, QScriptEngine *engine)
{
   int argCount = ctxt->argumentCount();
   if (argCount < 3 || argCount > 4) {
      return ctxt->throwError(QLatin1String("Qt.hsla(): Invalid arguments"));
   }
   qsreal h = ctxt->argument(0).toNumber();
   qsreal s = ctxt->argument(1).toNumber();
   qsreal l = ctxt->argument(2).toNumber();
   qsreal a = (argCount == 4) ? ctxt->argument(3).toNumber() : 1;

   if (h < 0.0) {
      h = 0.0;
   }
   if (h > 1.0) {
      h = 1.0;
   }
   if (s < 0.0) {
      s = 0.0;
   }
   if (s > 1.0) {
      s = 1.0;
   }
   if (l < 0.0) {
      l = 0.0;
   }
   if (l > 1.0) {
      l = 1.0;
   }
   if (a < 0.0) {
      a = 0.0;
   }
   if (a > 1.0) {
      a = 1.0;
   }

   return engine->toScriptValue(QVariant::fromValue(QColor::fromHslF(h, s, l, a)));
}

/*!
\qmlmethod rect Qt::rect(int x, int y, int width, int height)

Returns a \c rect with the top-left corner at \c x, \c y and the specified \c width and \c height.

The returned object has \c x, \c y, \c width and \c height attributes with the given values.
*/
QScriptValue QDeclarativeEnginePrivate::rect(QScriptContext *ctxt, QScriptEngine *engine)
{
   if (ctxt->argumentCount() != 4) {
      return ctxt->throwError(QLatin1String("Qt.rect(): Invalid arguments"));
   }

   qsreal x = ctxt->argument(0).toNumber();
   qsreal y = ctxt->argument(1).toNumber();
   qsreal w = ctxt->argument(2).toNumber();
   qsreal h = ctxt->argument(3).toNumber();

   return QDeclarativeEnginePrivate::get(engine)->scriptValueFromVariant(QVariant::fromValue(QRectF(x, y, w, h)));
}

/*!
\qmlmethod point Qt::point(int x, int y)
Returns a Point with the specified \c x and \c y coordinates.
*/
QScriptValue QDeclarativeEnginePrivate::point(QScriptContext *ctxt, QScriptEngine *engine)
{
   if (ctxt->argumentCount() != 2) {
      return ctxt->throwError(QLatin1String("Qt.point(): Invalid arguments"));
   }
   qsreal x = ctxt->argument(0).toNumber();
   qsreal y = ctxt->argument(1).toNumber();
   return QDeclarativeEnginePrivate::get(engine)->scriptValueFromVariant(QVariant::fromValue(QPointF(x, y)));
}

/*!
\qmlmethod Qt::size(int width, int height)
Returns a Size with the specified \c width and \c height.
*/
QScriptValue QDeclarativeEnginePrivate::size(QScriptContext *ctxt, QScriptEngine *engine)
{
   if (ctxt->argumentCount() != 2) {
      return ctxt->throwError(QLatin1String("Qt.size(): Invalid arguments"));
   }
   qsreal w = ctxt->argument(0).toNumber();
   qsreal h = ctxt->argument(1).toNumber();
   return QDeclarativeEnginePrivate::get(engine)->scriptValueFromVariant(QVariant::fromValue(QSizeF(w, h)));
}

/*!
\qmlmethod color Qt::lighter(color baseColor, real factor)
Returns a color lighter than \c baseColor by the \c factor provided.

If the factor is greater than 1.0, this functions returns a lighter color.
Setting factor to 1.5 returns a color that is 50% brighter. If the factor is less than 1.0,
the return color is darker, but we recommend using the Qt.darker() function for this purpose.
If the factor is 0 or negative, the return value is unspecified.

The function converts the current RGB color to HSV, multiplies the value (V) component
by factor and converts the color back to RGB.

If \c factor is not supplied, returns a color 50% lighter than \c baseColor (factor 1.5).
*/
QScriptValue QDeclarativeEnginePrivate::lighter(QScriptContext *ctxt, QScriptEngine *engine)
{
   if (ctxt->argumentCount() != 1 && ctxt->argumentCount() != 2) {
      return ctxt->throwError(QLatin1String("Qt.lighter(): Invalid arguments"));
   }
   QVariant v = ctxt->argument(0).toVariant();
   QColor color;
   if (v.userType() == QVariant::Color) {
      color = v.value<QColor>();
   } else if (v.userType() == QVariant::String) {
      bool ok;
      color = QDeclarativeStringConverters::colorFromString(v.toString(), &ok);
      if (!ok) {
         return engine->nullValue();
      }
   } else {
      return engine->nullValue();
   }
   qsreal factor = 1.5;
   if (ctxt->argumentCount() == 2) {
      factor = ctxt->argument(1).toNumber();
   }
   color = color.lighter(int(qRound(factor * 100.)));
   return engine->toScriptValue(QVariant::fromValue(color));
}

/*!
\qmlmethod color Qt::darker(color baseColor, real factor)
Returns a color darker than \c baseColor by the \c factor provided.

If the factor is greater than 1.0, this function returns a darker color.
Setting factor to 3.0 returns a color that has one-third the brightness.
If the factor is less than 1.0, the return color is lighter, but we recommend using
the Qt.lighter() function for this purpose. If the factor is 0 or negative, the return
value is unspecified.

The function converts the current RGB color to HSV, divides the value (V) component
by factor and converts the color back to RGB.

If \c factor is not supplied, returns a color 50% darker than \c baseColor (factor 2.0).
*/
QScriptValue QDeclarativeEnginePrivate::darker(QScriptContext *ctxt, QScriptEngine *engine)
{
   if (ctxt->argumentCount() != 1 && ctxt->argumentCount() != 2) {
      return ctxt->throwError(QLatin1String("Qt.darker(): Invalid arguments"));
   }
   QVariant v = ctxt->argument(0).toVariant();
   QColor color;
   if (v.userType() == QVariant::Color) {
      color = v.value<QColor>();
   } else if (v.userType() == QVariant::String) {
      bool ok;
      color = QDeclarativeStringConverters::colorFromString(v.toString(), &ok);
      if (!ok) {
         return engine->nullValue();
      }
   } else {
      return engine->nullValue();
   }
   qsreal factor = 2.0;
   if (ctxt->argumentCount() == 2) {
      factor = ctxt->argument(1).toNumber();
   }
   color = color.darker(int(qRound(factor * 100.)));
   return engine->toScriptValue(QVariant::fromValue(color));
}

/*!
\qmlmethod bool Qt::openUrlExternally(url target)
Attempts to open the specified \c target url in an external application, based on the user's desktop preferences. Returns true if it succeeds, and false otherwise.
*/
QScriptValue QDeclarativeEnginePrivate::desktopOpenUrl(QScriptContext *ctxt, QScriptEngine *e)
{
   if (ctxt->argumentCount() < 1) {
      return QScriptValue(e, false);
   }
   bool ret = false;
#ifndef QT_NO_DESKTOPSERVICES
   ret = QDesktopServices::openUrl(QDeclarativeScriptEngine::get(e)->resolvedUrl(ctxt,
                                   QUrl(ctxt->argument(0).toString())));
#endif
   return QScriptValue(e, ret);
}

/*!
\qmlmethod list<string> Qt::fontFamilies()
Returns a list of the font families available to the application.
*/

QScriptValue QDeclarativeEnginePrivate::fontFamilies(QScriptContext *ctxt, QScriptEngine *e)
{
   if (ctxt->argumentCount() != 0) {
      return ctxt->throwError(QLatin1String("Qt.fontFamilies(): Invalid arguments"));
   }

   QDeclarativeEnginePrivate *p = QDeclarativeEnginePrivate::get(e);
   QFontDatabase database;
   return p->scriptValueFromVariant(database.families());
}

/*!
\qmlmethod string Qt::md5(data)
Returns a hex string of the md5 hash of \c data.
*/
QScriptValue QDeclarativeEnginePrivate::md5(QScriptContext *ctxt, QScriptEngine *)
{
   if (ctxt->argumentCount() != 1) {
      return ctxt->throwError(QLatin1String("Qt.md5(): Invalid arguments"));
   }

   QByteArray data = ctxt->argument(0).toString().toUtf8();
   QByteArray result = QCryptographicHash::hash(data, QCryptographicHash::Md5);

   return QScriptValue(QLatin1String(result.toHex()));
}

/*!
\qmlmethod string Qt::btoa(data)
Binary to ASCII - this function returns a base64 encoding of \c data.
*/
QScriptValue QDeclarativeEnginePrivate::btoa(QScriptContext *ctxt, QScriptEngine *)
{
   if (ctxt->argumentCount() != 1) {
      return ctxt->throwError(QLatin1String("Qt.btoa(): Invalid arguments"));
   }

   QByteArray data = ctxt->argument(0).toString().toUtf8();

   return QScriptValue(QLatin1String(data.toBase64()));
}

/*!
\qmlmethod string Qt::atob(data)
ASCII to binary - this function returns a base64 decoding of \c data.
*/

QScriptValue QDeclarativeEnginePrivate::atob(QScriptContext *ctxt, QScriptEngine *)
{
   if (ctxt->argumentCount() != 1) {
      return ctxt->throwError(QLatin1String("Qt.atob(): Invalid arguments"));
   }

   QByteArray data = ctxt->argument(0).toString().toUtf8();

   return QScriptValue(QLatin1String(QByteArray::fromBase64(data)));
}

QScriptValue QDeclarativeEnginePrivate::consoleLog(QScriptContext *ctxt, QScriptEngine *e)
{
   if (ctxt->argumentCount() < 1) {
      return e->newVariant(QVariant(false));
   }

   QByteArray msg;

   for (int i = 0; i < ctxt->argumentCount(); ++i) {
      if (!msg.isEmpty()) {
         msg += ' ';
      }
      msg += ctxt->argument(i).toString().toLocal8Bit();
      // does not support firebug "%[a-z]" formatting, since firebug really
      // does just ignore the format letter, which makes it pointless.
   }

   qDebug("%s", msg.constData());

   return e->newVariant(QVariant(true));
}

void QDeclarativeEnginePrivate::sendQuit()
{
   Q_Q(QDeclarativeEngine);
   emit q->quit();
   if (q->receivers(SIGNAL(quit())) == 0) {
      qWarning("Signal QDeclarativeEngine::quit() emitted, but no receivers connected to handle it.");
   }
}

static void dumpwarning(const QDeclarativeError &error)
{
   qWarning().nospace() << qPrintable(error.toString());
}

static void dumpwarning(const QList<QDeclarativeError> &errors)
{
   for (int ii = 0; ii < errors.count(); ++ii) {
      dumpwarning(errors.at(ii));
   }
}

void QDeclarativeEnginePrivate::warning(const QDeclarativeError &error)
{
   Q_Q(QDeclarativeEngine);
   q->warnings(QList<QDeclarativeError>() << error);
   if (outputWarningsToStdErr) {
      dumpwarning(error);
   }
}

void QDeclarativeEnginePrivate::warning(const QList<QDeclarativeError> &errors)
{
   Q_Q(QDeclarativeEngine);
   q->warnings(errors);
   if (outputWarningsToStdErr) {
      dumpwarning(errors);
   }
}

void QDeclarativeEnginePrivate::warning(QDeclarativeEngine *engine, const QDeclarativeError &error)
{
   if (engine) {
      QDeclarativeEnginePrivate::get(engine)->warning(error);
   } else {
      dumpwarning(error);
   }
}

void QDeclarativeEnginePrivate::warning(QDeclarativeEngine *engine, const QList<QDeclarativeError> &error)
{
   if (engine) {
      QDeclarativeEnginePrivate::get(engine)->warning(error);
   } else {
      dumpwarning(error);
   }
}

void QDeclarativeEnginePrivate::warning(QDeclarativeEnginePrivate *engine, const QDeclarativeError &error)
{
   if (engine) {
      engine->warning(error);
   } else {
      dumpwarning(error);
   }
}

void QDeclarativeEnginePrivate::warning(QDeclarativeEnginePrivate *engine, const QList<QDeclarativeError> &error)
{
   if (engine) {
      engine->warning(error);
   } else {
      dumpwarning(error);
   }
}

/*!
\qmlmethod Qt::quit()
This function causes the QDeclarativeEngine::quit() signal to be emitted.
Within the \l {QML Viewer}, this causes the launcher application to exit;
to quit a C++ application when this method is called, connect the
QDeclarativeEngine::quit() signal to the QCoreApplication::quit() slot.
*/

QScriptValue QDeclarativeEnginePrivate::quit(QScriptContext * /*ctxt*/, QScriptEngine *e)
{
   QDeclarativeEnginePrivate *qe = get (e);
   qe->sendQuit();
   return QScriptValue();
}

/*!
    \qmlmethod color Qt::tint(color baseColor, color tintColor)
    This function allows tinting one color with another.

    The tint color should usually be mostly transparent, or you will not be
    able to see the underlying color. The below example provides a slight red
    tint by having the tint color be pure red which is only 1/16th opaque.

    \qml
    Item {
        Rectangle {
            x: 0; width: 80; height: 80
            color: "lightsteelblue"
        }
        Rectangle {
            x: 100; width: 80; height: 80
            color: Qt.tint("lightsteelblue", "#10FF0000")
        }
    }
    \endqml
    \image declarative-rect_tint.png

    Tint is most useful when a subtle change is intended to be conveyed due to some event; you can then use tinting to more effectively tune the visible color.
*/
QScriptValue QDeclarativeEnginePrivate::tint(QScriptContext *ctxt, QScriptEngine *engine)
{
   if (ctxt->argumentCount() != 2) {
      return ctxt->throwError(QLatin1String("Qt.tint(): Invalid arguments"));
   }
   //get color
   QVariant v = ctxt->argument(0).toVariant();
   QColor color;
   if (v.userType() == QVariant::Color) {
      color = v.value<QColor>();
   } else if (v.userType() == QVariant::String) {
      bool ok;
      color = QDeclarativeStringConverters::colorFromString(v.toString(), &ok);
      if (!ok) {
         return engine->nullValue();
      }
   } else {
      return engine->nullValue();
   }

   //get tint color
   v = ctxt->argument(1).toVariant();
   QColor tintColor;
   if (v.userType() == QVariant::Color) {
      tintColor = v.value<QColor>();
   } else if (v.userType() == QVariant::String) {
      bool ok;
      tintColor = QDeclarativeStringConverters::colorFromString(v.toString(), &ok);
      if (!ok) {
         return engine->nullValue();
      }
   } else {
      return engine->nullValue();
   }

   //tint
   QColor finalColor;
   int a = tintColor.alpha();
   if (a == 0xFF) {
      finalColor = tintColor;
   } else if (a == 0x00) {
      finalColor = color;
   } else {
      qreal a = tintColor.alphaF();
      qreal inv_a = 1.0 - a;

      finalColor.setRgbF(tintColor.redF() * a + color.redF() * inv_a,
                         tintColor.greenF() * a + color.greenF() * inv_a,
                         tintColor.blueF() * a + color.blueF() * inv_a,
                         a + inv_a * color.alphaF());
   }

   return engine->toScriptValue(QVariant::fromValue(finalColor));
}

QScriptValue QDeclarativeEnginePrivate::scriptValueFromVariant(const QVariant &val)
{
   if (val.userType() == qMetaTypeId<QDeclarativeListReference>()) {
      QDeclarativeListReferencePrivate *p =
         QDeclarativeListReferencePrivate::get((QDeclarativeListReference *)val.constData());
      if (p->object) {
         return listClass->newList(p->property, p->propertyType);
      } else {
         return scriptEngine.nullValue();
      }
   } else if (val.userType() == qMetaTypeId<QList<QObject *> >()) {
      const QList<QObject *> &list = *(QList<QObject *> *)val.constData();
      QScriptValue rv = scriptEngine.newArray(list.count());
      for (int ii = 0; ii < list.count(); ++ii) {
         QObject *object = list.at(ii);
         rv.setProperty(ii, objectClass->newQObject(object));
      }
      return rv;
   } else if (QDeclarativeValueType *vt = valueTypes[val.userType()]) {
      return valueTypeClass->newObject(val, vt);
   }

   bool objOk;
   QObject *obj = QDeclarativeMetaType::toQObject(val, &objOk);
   if (objOk) {
      return objectClass->newQObject(obj);
   } else {
      return scriptEngine.toScriptValue(val);
   }
}

QVariant QDeclarativeEnginePrivate::scriptValueToVariant(const QScriptValue &val, int hint)
{
    QScriptDeclarativeClass *dc = QScriptDeclarativeClass::scriptClass(val);
    if (dc == objectClass)
        return QVariant::fromValue(objectClass->toQObject(val));
    else if (dc == valueTypeClass)
        return valueTypeClass->toVariant(val);
    else if (dc == contextClass)
        return QVariant();

    bool containsQObjects = false;

    if (val.isArray()) {
        int length = val.property(QLatin1String("length")).toInt32();
        if (!length)
            containsQObjects = true;
        for (int ii = 0; ii < length; ++ii) {
            if (val.property(ii).isQObject()) {
                containsQObjects = true;
                break;
            }
        }
    }

    // Convert to a QList<QObject*> only if val is an array and we were explicitly hinted
    if (hint == qMetaTypeId<QList<QObject *> >() && val.isArray() && containsQObjects) {
        QList<QObject *> list;
        int length = val.property(QLatin1String("length")).toInt32();
        for (int ii = 0; ii < length; ++ii) {
            QScriptValue arrayItem = val.property(ii);
            QObject *d = arrayItem.toQObject();
            list << d;
        }
        return QVariant::fromValue(list);
    }

    return val.toVariant();
}

/*!
  Adds \a path as a directory where the engine searches for
  installed modules in a URL-based directory structure.
  The \a path may be a local filesystem directory or a URL.

  The newly added \a path will be first in the importPathList().

  \sa setImportPathList(), {QML Modules}
*/
void QDeclarativeEngine::addImportPath(const QString &path)
{
   Q_D(QDeclarativeEngine);
   d->importDatabase.addImportPath(path);
}

/*!
  Returns the list of directories where the engine searches for
  installed modules in a URL-based directory structure.

  For example, if \c /opt/MyApp/lib/imports is in the path, then QML that
  imports \c com.mycompany.Feature will cause the QDeclarativeEngine to look
  in \c /opt/MyApp/lib/imports/com/mycompany/Feature/ for the components
  provided by that module. A \c qmldir file is required for defining the
  type version mapping and possibly declarative extensions plugins.

  By default, the list contains the directory of the application executable,
  paths specified in the \c QML_IMPORT_PATH environment variable,
  and the builtin \c ImportsPath from QLibraryInfo.

  \sa addImportPath() setImportPathList()
*/
QStringList QDeclarativeEngine::importPathList() const
{
   Q_D(const QDeclarativeEngine);
   return d->importDatabase.importPathList();
}

/*!
  Sets \a paths as the list of directories where the engine searches for
  installed modules in a URL-based directory structure.

  By default, the list contains the directory of the application executable,
  paths specified in the \c QML_IMPORT_PATH environment variable,
  and the builtin \c ImportsPath from QLibraryInfo.

  \sa importPathList() addImportPath()
  */
void QDeclarativeEngine::setImportPathList(const QStringList &paths)
{
   Q_D(QDeclarativeEngine);
   d->importDatabase.setImportPathList(paths);
}


/*!
  Adds \a path as a directory where the engine searches for
  native plugins for imported modules (referenced in the \c qmldir file).

  By default, the list contains only \c .,  i.e. the engine searches
  in the directory of the \c qmldir file itself.

  The newly added \a path will be first in the pluginPathList().

  \sa setPluginPathList()
*/
void QDeclarativeEngine::addPluginPath(const QString &path)
{
   Q_D(QDeclarativeEngine);
   d->importDatabase.addPluginPath(path);
}


/*!
  Returns the list of directories where the engine searches for
  native plugins for imported modules (referenced in the \c qmldir file).

  By default, the list contains only \c .,  i.e. the engine searches
  in the directory of the \c qmldir file itself.

  \sa addPluginPath() setPluginPathList()
*/
QStringList QDeclarativeEngine::pluginPathList() const
{
   Q_D(const QDeclarativeEngine);
   return d->importDatabase.pluginPathList();
}

/*!
  Sets the list of directories where the engine searches for
  native plugins for imported modules (referenced in the \c qmldir file)
  to \a paths.

  By default, the list contains only \c .,  i.e. the engine searches
  in the directory of the \c qmldir file itself.

  \sa pluginPathList() addPluginPath()
  */
void QDeclarativeEngine::setPluginPathList(const QStringList &paths)
{
   Q_D(QDeclarativeEngine);
   d->importDatabase.setPluginPathList(paths);
}


/*!
  Imports the plugin named \a filePath with the \a uri provided.
  Returns true if the plugin was successfully imported; otherwise returns false.

  On failure and if non-null, *\a errorString will be set to a message describing the failure.

  The plugin has to be a Qt plugin which implements the QDeclarativeExtensionPlugin interface.
*/
bool QDeclarativeEngine::importPlugin(const QString &filePath, const QString &uri, QString *errorString)
{
   Q_D(QDeclarativeEngine);
   return d->importDatabase.importPlugin(filePath, uri, errorString);
}

/*!
  \property QDeclarativeEngine::offlineStoragePath
  \brief the directory for storing offline user data

  Returns the directory where SQL and other offline
  storage is placed.

  QDeclarativeWebView and the SQL databases created with openDatabase()
  are stored here.

  The default is QML/OfflineStorage in the platform-standard
  user application data directory.

  Note that the path may not currently exist on the filesystem, so
  callers wanting to \e create new files at this location should create
  it first - see QDir::mkpath().
*/
void QDeclarativeEngine::setOfflineStoragePath(const QString &dir)
{
   Q_D(QDeclarativeEngine);
   d->scriptEngine.offlineStoragePath = dir;
}

QString QDeclarativeEngine::offlineStoragePath() const
{
   Q_D(const QDeclarativeEngine);
   return d->scriptEngine.offlineStoragePath;
}

static void voidptr_destructor(void *v)
{
   void **ptr = (void **)v;
   delete ptr;
}

static void *voidptr_constructor(const void *v)
{
   if (!v) {
      return new void *;
   } else {
      return new void *(*(void **)v);
   }
}

QDeclarativePropertyCache *QDeclarativeEnginePrivate::createCache(const QMetaObject *mo)
{
   Q_Q(QDeclarativeEngine);

   if (!mo->superClass()) {
      QDeclarativePropertyCache *rv = new QDeclarativePropertyCache(q, mo);
      propertyCache.insert(mo, rv);
      return rv;
   } else {
      QDeclarativePropertyCache *super = cache(mo->superClass());
      QDeclarativePropertyCache *rv = super->copy();
      rv->append(q, mo);
      propertyCache.insert(mo, rv);
      return rv;
   }
}

QDeclarativePropertyCache *QDeclarativeEnginePrivate::createCache(QDeclarativeType *type, int minorVersion,
      QDeclarativeError &error)
{
   QList<QDeclarativeType *> types;

   int maxMinorVersion = 0;

   const QMetaObject *metaObject = type->metaObject();
   while (metaObject) {
      QDeclarativeType *t = QDeclarativeMetaType::qmlType(metaObject, type->module(),
                            type->majorVersion(), minorVersion);
      if (t) {
         maxMinorVersion = qMax(maxMinorVersion, t->minorVersion());
         types << t;
      } else {
         types << 0;
      }

      metaObject = metaObject->superClass();
   }

   if (QDeclarativePropertyCache *c = typePropertyCache.value(qMakePair(type, maxMinorVersion))) {
      c->addref();
      typePropertyCache.insert(qMakePair(type, minorVersion), c);
      return c;
   }

   QDeclarativePropertyCache *raw = cache(type->metaObject());

   bool hasCopied = false;

   for (int ii = 0; ii < types.count(); ++ii) {
      QDeclarativeType *currentType = types.at(ii);
      if (!currentType) {
         continue;
      }

      int rev = currentType->metaObjectRevision();
      int moIndex = types.count() - 1 - ii;

      if (raw->allowedRevisionCache[moIndex] != rev) {
         if (!hasCopied) {
            raw = raw->copy();
            hasCopied = true;
         }
         raw->allowedRevisionCache[moIndex] = rev;
      }
   }

   // Test revision compatibility - the basic rule is:
   //    * Anything that is excluded, cannot overload something that is not excluded *

   // Signals override:
   //    * other signals and methods of the same name.
   //    * properties named on<Signal Name>
   //    * automatic <property name>Changed notify signals

   // Methods override:
   //    * other methods of the same name

   // Properties override:
   //    * other elements of the same name

   bool overloadError = false;
   QString overloadName;

#if 0
   for (QDeclarativePropertyCache::StringCache::ConstIterator iter = raw->stringCache.begin();
         !overloadError && iter != raw->stringCache.end();
         ++iter) {

      QDeclarativePropertyCache::Data *d = *iter;
      if (raw->isAllowedInRevision(d)) {
         continue;   // Not excluded - no problems
      }

      // check that a regular "name" overload isn't happening
      QDeclarativePropertyCache::Data *current = d;
      while (!overloadError && current) {
         current = d->overrideData(current);
         if (current && raw->isAllowedInRevision(current)) {
            overloadError = true;
         }
      }
   }
#endif

   if (overloadError) {
      if (hasCopied) {
         raw->release();
      }

      error.setDescription(QLatin1String("Type ") + QString::fromUtf8(type->qmlTypeName()) + QLatin1String(" ") +
                           QString::number(type->majorVersion()) + QLatin1String(".") + QString::number(minorVersion) +
                           QLatin1String(" contains an illegal property \"") + overloadName +
                           QLatin1String("\".  This is an error in the type's implementation."));
      return 0;
   }

   if (!hasCopied) {
      raw->addref();
   }
   typePropertyCache.insert(qMakePair(type, minorVersion), raw);

   if (minorVersion != maxMinorVersion) {
      raw->addref();
      typePropertyCache.insert(qMakePair(type, maxMinorVersion), raw);
   }

   return raw;
}

void QDeclarativeEnginePrivate::registerCompositeType(QDeclarativeCompiledData *data)
{
   QByteArray name = data->root->className();

   QByteArray ptr = name + '*';
   QByteArray lst = "QDeclarativeListProperty<" + name + '>';

   int ptr_type = QMetaType::registerType(ptr.constData(), voidptr_destructor,
                                          voidptr_constructor);
   int lst_type = QMetaType::registerType(lst.constData(), voidptr_destructor,
                                          voidptr_constructor);

   m_qmlLists.insert(lst_type, ptr_type);
   m_compositeTypes.insert(ptr_type, data);
   data->addref();
}

bool QDeclarativeEnginePrivate::isList(int t) const
{
   return m_qmlLists.contains(t) || QDeclarativeMetaType::isList(t);
}

int QDeclarativeEnginePrivate::listType(int t) const
{
   QHash<int, int>::ConstIterator iter = m_qmlLists.find(t);
   if (iter != m_qmlLists.end()) {
      return *iter;
   } else {
      return QDeclarativeMetaType::listType(t);
   }
}

bool QDeclarativeEnginePrivate::isQObject(int t)
{
   return m_compositeTypes.contains(t) || QDeclarativeMetaType::isQObject(t);
}

QObject *QDeclarativeEnginePrivate::toQObject(const QVariant &v, bool *ok) const
{
   int t = v.userType();
   if (t == QMetaType::QObjectStar || m_compositeTypes.contains(t)) {
      if (ok) {
         *ok = true;
      }
      return *(QObject **)(v.constData());
   } else {
      return QDeclarativeMetaType::toQObject(v, ok);
   }
}

QDeclarativeMetaType::TypeCategory QDeclarativeEnginePrivate::typeCategory(int t) const
{
   if (m_compositeTypes.contains(t)) {
      return QDeclarativeMetaType::Object;
   } else if (m_qmlLists.contains(t)) {
      return QDeclarativeMetaType::List;
   } else {
      return QDeclarativeMetaType::typeCategory(t);
   }
}

const QMetaObject *QDeclarativeEnginePrivate::rawMetaObjectForType(int t) const
{
   QHash<int, QDeclarativeCompiledData *>::ConstIterator iter = m_compositeTypes.find(t);
   if (iter != m_compositeTypes.end()) {
      return (*iter)->root;
   } else {
      QDeclarativeType *type = QDeclarativeMetaType::qmlType(t);
      return type ? type->baseMetaObject() : 0;
   }
}

const QMetaObject *QDeclarativeEnginePrivate::metaObjectForType(int t) const
{
   QHash<int, QDeclarativeCompiledData *>::ConstIterator iter = m_compositeTypes.find(t);
   if (iter != m_compositeTypes.end()) {
      return (*iter)->root;
   } else {
      QDeclarativeType *type = QDeclarativeMetaType::qmlType(t);
      return type ? type->metaObject() : 0;
   }
}

bool QDeclarative_isFileCaseCorrect(const QString &fileName)
{
#if defined(Q_OS_DARWIN) || defined(Q_OS_WIN32)
   QFileInfo info(fileName);

   QString absolute = info.absoluteFilePath();

#if defined(Q_OS_DARWIN)
   QString canonical = info.canonicalFilePath();
#elif defined(Q_OS_WIN32)
   wchar_t buffer[1024];

   DWORD rv = ::GetShortPathName((wchar_t *)absolute.utf16(), buffer, 1024);
   if (rv == 0 || rv >= 1024) {
      return true;
   }
   rv = ::GetLongPathName(buffer, buffer, 1024);
   if (rv == 0 || rv >= 1024) {
      return true;
   }

   QString canonical((QChar *)buffer);
#endif

   int absoluteLength = absolute.length();
   int canonicalLength = canonical.length();

   int length = qMin(absoluteLength, canonicalLength);
   for (int ii = 0; ii < length; ++ii) {
      const QChar &a = absolute.at(absoluteLength - 1 - ii);
      const QChar &c = canonical.at(canonicalLength - 1 - ii);

      if (a.toLower() != c.toLower()) {
         return true;
      }
      if (a != c) {
         return false;
      }
   }
#else
   Q_UNUSED(fileName)
#endif
   return true;
}

QT_END_NAMESPACE
