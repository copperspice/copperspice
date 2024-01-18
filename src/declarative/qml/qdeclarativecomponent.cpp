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

#include "qdeclarativecomponent.h"
#include "private/qdeclarativecomponent_p.h"

#include "private/qdeclarativecompiler_p.h"
#include "private/qdeclarativecontext_p.h"
#include "private/qdeclarativeengine_p.h"
#include "private/qdeclarativevme_p.h"
#include "qdeclarative.h"
#include "qdeclarativeengine.h"
#include "private/qdeclarativebinding_p.h"
#include "private/qdeclarativebinding_p_p.h"
#include "private/qdeclarativeglobal_p.h"
#include "private/qdeclarativescriptparser_p.h"
#include "private/qdeclarativedebugtrace_p.h"
#include "private/qdeclarativeenginedebugservice_p.h"
#include <QtScript/qscriptvalueiterator.h>

#include <QStack>
#include <QStringList>
#include <QtCore/qdebug.h>
#include <QApplication>
#include <qdeclarativeinfo.h>

QT_BEGIN_NAMESPACE

class QByteArray;

/*!
    \class QDeclarativeComponent
    \since 4.7
    \brief The QDeclarativeComponent class encapsulates a QML component definition.
    \mainclass

    Components are reusable, encapsulated QML elements with well-defined interfaces.
    They are often defined in \l {qdeclarativedocuments.html}{Component Files}.

    A QDeclarativeComponent instance can be created from a QML file.
    For example, if there is a \c main.qml file like this:

    \qml
    import QtQuick 1.0

    Item {
        width: 200
        height: 200
    }
    \endqml

    The following code loads this QML file as a component, creates an instance of
    this component using create(), and then queries the \l Item's \l {Item::}{width}
    value:

    \code
    QDeclarativeEngine *engine = new QDeclarativeEngine;
    QDeclarativeComponent component(engine, QUrl::fromLocalFile("main.qml"));

    QObject *myObject = component.create();
    QDeclarativeItem *item = qobject_cast<QDeclarativeItem*>(myObject);
    int width = item->width();  // width = 200
    \endcode


    \section2 Network Components

    If the URL passed to QDeclarativeComponent is a network resource, or if the QML document references a
    network resource, the QDeclarativeComponent has to fetch the network data before it is able to create
    objects.  In this case, the QDeclarativeComponent will have a \l {QDeclarativeComponent::Loading}{Loading}
    \l {QDeclarativeComponent::status()}{status}.  An application will have to wait until the component
    is \l {QDeclarativeComponent::Ready}{Ready} before calling \l {QDeclarativeComponent::create()}.

    The following example shows how to load a QML file from a network resource.  After creating
    the QDeclarativeComponent, it tests whether the component is loading.  If it is, it connects to the
    QDeclarativeComponent::statusChanged() signal and otherwise calls the \c {continueLoading()} method
    directly. Note that QDeclarativeComponent::isLoading() may be false for a network component if the
    component has been cached and is ready immediately.

    \code
    MyApplication::MyApplication()
    {
        // ...
        component = new QDeclarativeComponent(engine, QUrl("http://www.example.com/main.qml"));
        if (component->isLoading())
            QObject::connect(component, SIGNAL(statusChanged(QDeclarativeComponent::Status)),
                             this, SLOT(continueLoading()));
        else
            continueLoading();
    }

    void MyApplication::continueLoading()
    {
        if (component->isError()) {
            qWarning() << component->errors();
        } else {
            QObject *myObject = component->create();
        }
    }
    \endcode

    \sa {Using QML Bindings in C++ Applications}, {Integrating QML Code with Existing Qt UI Code}
*/

/*!
    \qmlclass Component QDeclarativeComponent
    \ingroup qml-utility-elements
    \since 4.7
    \brief The Component element encapsulates a QML component definition.

    Components are reusable, encapsulated QML elements with well-defined interfaces.

    Components are often defined by \l {qdeclarativedocuments.html}{component files} -
    that is, \c .qml files. The \e Component element essentially allows QML components
    to be defined inline, within a \l {QML Document}{QML document}, rather than as a separate QML file.
    This may be useful for reusing a small component within a QML file, or for defining
    a component that logically belongs with other QML components within a file.

    For example, here is a component that is used by multiple \l Loader objects.
    It contains a single item, a \l Rectangle:

    \snippet doc/src/snippets/declarative/component.qml 0

    Notice that while a \l Rectangle by itself would be automatically
    rendered and displayed, this is not the case for the above rectangle
    because it is defined inside a \c Component. The component encapsulates the
    QML elements within, as if they were defined in a separate QML
    file, and is not loaded until requested (in this case, by the
    two \l Loader objects).

    Defining a \c Component is similar to defining a \l {QML Document}{QML document}.
    A QML document has a single top-level item that defines the behaviors and
    properties of that component, and cannot define properties or behaviors outside
    of that top-level item. In the same way, a \c Component definition contains a single
    top level item (which in the above example is a \l Rectangle) and cannot define any
    data outside of this item, with the exception of an \e id (which in the above example
    is \e redSquare).

    The \c Component element is commonly used to provide graphical components
    for views. For example, the ListView::delegate property requires a \c Component
    to specify how each list item is to be displayed.

    \c Component objects can also be created dynamically using
    \l{QML:Qt::createComponent()}{Qt.createComponent()}.
*/

/*!
    \qmlattachedsignal Component::onCompleted()

    Emitted after component "startup" has completed.  This can be used to
    execute script code at startup, once the full QML environment has been
    established.

    The \c {Component::onCompleted} attached property can be applied to
    any element.  The order of running the \c onCompleted scripts is
    undefined.

    \qml
    Rectangle {
        Component.onCompleted: console.log("Completed Running!")
        Rectangle {
            Component.onCompleted: console.log("Nested Completed Running!")
        }
    }
    \endqml
*/

/*!
    \qmlattachedsignal Component::onDestruction()

    Emitted as the component begins destruction.  This can be used to undo
    work done in the onCompleted signal, or other imperative code in your
    application.

    The \c {Component::onDestruction} attached property can be applied to
    any element.  However, it applies to the destruction of the component as
    a whole, and not the destruction of the specific object.  The order of
    running the \c onDestruction scripts is undefined.

    \qml
    Rectangle {
        Component.onDestruction: console.log("Destruction Beginning!")
        Rectangle {
            Component.onDestruction: console.log("Nested Destruction Beginning!")
        }
    }
    \endqml

    \sa QtDeclarative
*/

/*!
    \enum QDeclarativeComponent::Status

    Specifies the loading status of the QDeclarativeComponent.

    \value Null This QDeclarativeComponent has no data.  Call loadUrl() or setData() to add QML content.
    \value Ready This QDeclarativeComponent is ready and create() may be called.
    \value Loading This QDeclarativeComponent is loading network data.
    \value Error An error has occurred.  Call errors() to retrieve a list of \{QDeclarativeError}{errors}.
*/

void QDeclarativeComponentPrivate::typeDataReady(QDeclarativeTypeData *)
{
   Q_Q(QDeclarativeComponent);

   Q_ASSERT(typeData);

   fromTypeData(typeData);
   typeData = 0;

   emit q->statusChanged(q->status());
}

void QDeclarativeComponentPrivate::typeDataProgress(QDeclarativeTypeData *, qreal p)
{
   Q_Q(QDeclarativeComponent);

   progress = p;

   emit q->progressChanged(p);
}

void QDeclarativeComponentPrivate::fromTypeData(QDeclarativeTypeData *data)
{
   url = data->finalUrl();
   QDeclarativeCompiledData *c = data->compiledData();

   if (!c) {
      Q_ASSERT(data->isError());
      state.errors = data->errors();
   } else {
      cc = c;
   }

   data->release();
}

void QDeclarativeComponentPrivate::clear()
{
   if (typeData) {
      typeData->unregisterCallback(this);
      typeData->release();
      typeData = 0;
   }

   if (cc) {
      cc->release();
      cc = 0;
   }
}

/*!
    \internal
*/
QDeclarativeComponent::QDeclarativeComponent(QObject *parent)
   : QObject(*(new QDeclarativeComponentPrivate), parent)
{
}

/*!
    Destruct the QDeclarativeComponent.
*/
QDeclarativeComponent::~QDeclarativeComponent()
{
   Q_D(QDeclarativeComponent);

   if (d->state.completePending) {
      qWarning("QDeclarativeComponent: Component destroyed while completion pending");
      d->completeCreate();
   }

   if (d->typeData) {
      d->typeData->unregisterCallback(d);
      d->typeData->release();
   }
   if (d->cc) {
      d->cc->release();
   }
}

/*!
    \qmlproperty enumeration Component::status
    This property holds the status of component loading.  It can be one of:
    \list
    \o Component.Null - no data is available for the component
    \o Component.Ready - the component has been loaded, and can be used to create instances.
    \o Component.Loading - the component is currently being loaded
    \o Component.Error - an error occurred while loading the component.
               Calling errorString() will provide a human-readable description of any errors.
    \endlist
 */

/*!
    \property QDeclarativeComponent::status
    The component's current \l{QDeclarativeComponent::Status} {status}.
 */
QDeclarativeComponent::Status QDeclarativeComponent::status() const
{
   Q_D(const QDeclarativeComponent);

   if (d->typeData) {
      return Loading;
   } else if (!d->state.errors.isEmpty()) {
      return Error;
   } else if (d->engine && d->cc) {
      return Ready;
   } else {
      return Null;
   }
}

/*!
    Returns true if status() == QDeclarativeComponent::Null.
*/
bool QDeclarativeComponent::isNull() const
{
   return status() == Null;
}

/*!
    Returns true if status() == QDeclarativeComponent::Ready.
*/
bool QDeclarativeComponent::isReady() const
{
   return status() == Ready;
}

/*!
    Returns true if status() == QDeclarativeComponent::Error.
*/
bool QDeclarativeComponent::isError() const
{
   return status() == Error;
}

/*!
    Returns true if status() == QDeclarativeComponent::Loading.
*/
bool QDeclarativeComponent::isLoading() const
{
   return status() == Loading;
}

/*!
    \qmlproperty real Component::progress
    The progress of loading the component, from 0.0 (nothing loaded)
    to 1.0 (finished).
*/

/*!
    \property QDeclarativeComponent::progress
    The progress of loading the component, from 0.0 (nothing loaded)
    to 1.0 (finished).
*/
qreal QDeclarativeComponent::progress() const
{
   Q_D(const QDeclarativeComponent);
   return d->progress;
}

/*!
    \fn void QDeclarativeComponent::progressChanged(qreal progress)

    Emitted whenever the component's loading progress changes.  \a progress will be the
    current progress between 0.0 (nothing loaded) and 1.0 (finished).
*/

/*!
    \fn void QDeclarativeComponent::statusChanged(QDeclarativeComponent::Status status)

    Emitted whenever the component's status changes.  \a status will be the
    new status.
*/

/*!
    Create a QDeclarativeComponent with no data and give it the specified
    \a engine and \a parent. Set the data with setData().
*/
QDeclarativeComponent::QDeclarativeComponent(QDeclarativeEngine *engine, QObject *parent)
   : QObject(*(new QDeclarativeComponentPrivate), parent)
{
   Q_D(QDeclarativeComponent);
   d->engine = engine;
}

/*!
    Create a QDeclarativeComponent from the given \a url and give it the
    specified \a parent and \a engine.

    Ensure that the URL provided is full and correct, in particular, use
    \l QUrl::fromLocalFile() when loading a file from the local filesystem.

    \sa loadUrl()
*/
QDeclarativeComponent::QDeclarativeComponent(QDeclarativeEngine *engine, const QUrl &url, QObject *parent)
   : QObject(*(new QDeclarativeComponentPrivate), parent)
{
   Q_D(QDeclarativeComponent);
   d->engine = engine;
   loadUrl(url);
}

/*!
    Create a QDeclarativeComponent from the given \a fileName and give it the specified
    \a parent and \a engine.

    \sa loadUrl()
*/
QDeclarativeComponent::QDeclarativeComponent(QDeclarativeEngine *engine, const QString &fileName,
      QObject *parent)
   : QObject(*(new QDeclarativeComponentPrivate), parent)
{
   Q_D(QDeclarativeComponent);
   d->engine = engine;
   loadUrl(d->engine->baseUrl().resolved(QUrl::fromLocalFile(fileName)));
}

/*!
    \internal
*/
QDeclarativeComponent::QDeclarativeComponent(QDeclarativeEngine *engine, QDeclarativeCompiledData *cc, int start,
      int count, QObject *parent)
   : QObject(*(new QDeclarativeComponentPrivate), parent)
{
   Q_D(QDeclarativeComponent);
   d->engine = engine;
   d->cc = cc;
   cc->addref();
   d->start = start;
   d->count = count;
   d->url = cc->url;
   d->progress = 1.0;
}

/*!
    Sets the QDeclarativeComponent to use the given QML \a data.  If \a url
    is provided, it is used to set the component name and to provide
    a base path for items resolved by this component.
*/
void QDeclarativeComponent::setData(const QByteArray &data, const QUrl &url)
{
   Q_D(QDeclarativeComponent);

   d->clear();

   d->url = url;

   QDeclarativeTypeData *typeData = QDeclarativeEnginePrivate::get(d->engine)->typeLoader.get(data, url);

   if (typeData->isCompleteOrError()) {
      d->fromTypeData(typeData);
   } else {
      d->typeData = typeData;
      d->typeData->registerCallback(d);
   }

   d->progress = 1.0;
   emit statusChanged(status());
   emit progressChanged(d->progress);
}

/*!
Returns the QDeclarativeContext the component was created in.  This is only
valid for components created directly from QML.
*/
QDeclarativeContext *QDeclarativeComponent::creationContext() const
{
   Q_D(const QDeclarativeComponent);
   if (d->creationContext) {
      return d->creationContext->asQDeclarativeContext();
   }

   return qmlContext(this);
}

/*!
    Load the QDeclarativeComponent from the provided \a url.

    Ensure that the URL provided is full and correct, in particular, use
    \l QUrl::fromLocalFile() when loading a file from the local filesystem.
*/
void QDeclarativeComponent::loadUrl(const QUrl &url)
{
   Q_D(QDeclarativeComponent);

   d->clear();

   if ((url.isRelative() && !url.isEmpty())
         || url.scheme() == QLatin1String("file")) { // Workaround QTBUG-11929
      d->url = d->engine->baseUrl().resolved(url);
   } else {
      d->url = url;
   }

   if (url.isEmpty()) {
      QDeclarativeError error;
      error.setDescription(tr("Invalid empty URL"));
      d->state.errors << error;
      return;
   }

   QDeclarativeTypeData *data = QDeclarativeEnginePrivate::get(d->engine)->typeLoader.get(d->url);

   if (data->isCompleteOrError()) {
      d->fromTypeData(data);
      d->progress = 1.0;
   } else {
      d->typeData = data;
      d->typeData->registerCallback(d);
      d->progress = data->progress();
   }

   emit statusChanged(status());
   emit progressChanged(d->progress);
}

/*!
    Return the list of errors that occurred during the last compile or create
    operation.  An empty list is returned if isError() is not set.
*/
QList<QDeclarativeError> QDeclarativeComponent::errors() const
{
   Q_D(const QDeclarativeComponent);
   if (isError()) {
      return d->state.errors;
   } else {
      return QList<QDeclarativeError>();
   }
}

/*!
    \qmlmethod string Component::errorString()

    Returns a human-readable description of any errors.

    The string includes the file, location, and description of each error.
    If multiple errors are present they are separated by a newline character.

    If no errors are present, an empty string is returned.
*/

/*!
    \internal
    errorString is only meant as a way to get the errors in script
*/
QString QDeclarativeComponent::errorString() const
{
   Q_D(const QDeclarativeComponent);
   QString ret;
   if (!isError()) {
      return ret;
   }
   foreach(const QDeclarativeError & e, d->state.errors) {
      ret += e.url().toString() + QLatin1Char(':') +
             QString::number(e.line()) + QLatin1Char(' ') +
             e.description() + QLatin1Char('\n');
   }
   return ret;
}

/*!
    \qmlproperty url Component::url
    The component URL.  This is the URL that was used to construct the component.
*/

/*!
    \property QDeclarativeComponent::url
    The component URL.  This is the URL passed to either the constructor,
    or the loadUrl() or setData() methods.
*/
QUrl QDeclarativeComponent::url() const
{
   Q_D(const QDeclarativeComponent);
   return d->url;
}

/*!
    \internal
*/
QDeclarativeComponent::QDeclarativeComponent(QDeclarativeComponentPrivate &dd, QObject *parent)
   : QObject(dd, parent)
{
}

/*!
    \qmlmethod object Component::createObject(Item parent, object properties)

    Creates and returns an object instance of this component that will have
    the given \a parent and \a properties. The \a properties argument is optional.
    Returns null if object creation fails.

    The object will be created in the same context as the one in which the component
    was created. This function will always return null when called on components
    which were not created in QML.

    If you wish to create an object without setting a parent, specify \c null for
    the \a parent value. Note that if the returned object is to be displayed, you
    must provide a valid \a parent value or set the returned object's \l{Item::parent}{parent}
    property, or else the object will not be visible.

    If a \a parent is not provided to createObject(), a reference to the returned object must be held so that
    it is not destroyed by the garbage collector.  This is true regardless of whether \l{Item::parent} is set afterwards,
    since setting the Item parent does not change object ownership; only the graphical parent is changed.

    As of QtQuick 1.1, this method accepts an optional \a properties argument that specifies a
    map of initial property values for the created object. These values are applied before object
    creation is finalized. (This is more efficient than setting property values after object creation,
    particularly where large sets of property values are defined, and also allows property bindings
    to be set up before the object is created.)

    The \a properties argument is specified as a map of property-value items. For example, the code
    below creates an object with initial \c x and \c y values of 100 and 200, respectively:

    \js
        var component = Qt.createComponent("Button.qml");
        if (component.status == Component.Ready)
            component.createObject(parent, {"x": 100, "y": 100});
    \endjs

    Dynamically created instances can be deleted with the \c destroy() method.
    See \l {Dynamic Object Management in QML} for more information.
*/

/*!
    \internal
    A version of create which returns a scriptObject, for use in script.
    This function will only work on components created in QML.

    Sets graphics object parent because forgetting to do this is a frequent
    and serious problem.
*/
QScriptValue QDeclarativeComponent::createObject(QObject *parent)
{
   Q_D(QDeclarativeComponent);
   return d->createObject(parent, QScriptValue(QScriptValue::NullValue));
}

/*!
    \internal
    Overloadable method allows properties to be set during creation
*/
QScriptValue QDeclarativeComponent::createObject(QObject *parent, const QScriptValue &valuemap)
{
   Q_D(QDeclarativeComponent);

   if (!valuemap.isObject() || valuemap.isArray()) {
      qmlInfo(this) << tr("createObject: value is not an object");
      return QScriptValue(QScriptValue::NullValue);
   }
   return d->createObject(parent, valuemap);
}

QScriptValue QDeclarativeComponentPrivate::createObject(QObject *publicParent, const QScriptValue valuemap)
{
   Q_Q(QDeclarativeComponent);
   QDeclarativeContext *ctxt = q->creationContext();
   if (!ctxt && engine) {
      ctxt = engine->rootContext();
   }
   if (!ctxt) {
      return QScriptValue(QScriptValue::NullValue);
   }
   QObject *ret = q->beginCreate(ctxt);
   if (!ret) {
      q->completeCreate();
      return QScriptValue(QScriptValue::NullValue);
   }

   if (publicParent) {
      ret->setParent(publicParent);
      QList<QDeclarativePrivate::AutoParentFunction> functions = QDeclarativeMetaType::parentFunctions();

      bool needParent = false;

      for (int ii = 0; ii < functions.count(); ++ii) {
         QDeclarativePrivate::AutoParentResult res = functions.at(ii)(ret, publicParent);
         if (res == QDeclarativePrivate::Parented) {
            needParent = false;
            break;
         } else if (res == QDeclarativePrivate::IncompatibleParent) {
            needParent = true;
         }
      }

      if (needParent) {
         qWarning("QDeclarativeComponent: Created graphical object was not placed in the graphics scene.");
      }
   }

   QDeclarativeEnginePrivate *priv = QDeclarativeEnginePrivate::get(engine);
   QDeclarativeData::get(ret, true)->setImplicitDestructible();
   QScriptValue newObject = priv->objectClass->newQObject(ret, QMetaType::QObjectStar);

   if (valuemap.isObject() && !valuemap.isArray()) {
      //Iterate through and assign properties
      QScriptValueIterator it(valuemap);
      while (it.hasNext()) {
         it.next();
         QScriptValue prop = newObject;
         QString propName = it.name();
         int index = propName.indexOf(QLatin1Char('.'));
         if (index > 0) {
            QString subProp = propName;
            int lastIndex = 0;
            while (index > 0) {
               subProp = propName.mid(lastIndex, index - lastIndex);
               prop = prop.property(subProp);
               lastIndex = index + 1;
               index = propName.indexOf(QLatin1Char('.'), index + 1);
            }
            prop.setProperty(propName.mid(propName.lastIndexOf(QLatin1Char('.')) + 1), it.value());
         } else {
            newObject.setProperty(propName, it.value());
         }
      }
   }

   q->completeCreate();

   return newObject;
}

/*!
    Create an object instance from this component.  Returns 0 if creation
    failed.  \a context specifies the context within which to create the object
    instance.

    If \a context is 0 (the default), it will create the instance in the
    engine' s \l {QDeclarativeEngine::rootContext()}{root context}.
*/
QObject *QDeclarativeComponent::create(QDeclarativeContext *context)
{
   Q_D(QDeclarativeComponent);

   if (!context) {
      context = d->engine->rootContext();
   }

   QObject *rv = beginCreate(context);
   completeCreate();
   return rv;
}

/*!
    This method provides more advanced control over component instance creation.
    In general, programmers should use QDeclarativeComponent::create() to create a
    component.

    Create an object instance from this component.  Returns 0 if creation
    failed.  \a context specifies the context within which to create the object
    instance.

    When QDeclarativeComponent constructs an instance, it occurs in three steps:
    \list 1
    \i The object hierarchy is created, and constant values are assigned.
    \i Property bindings are evaluated for the the first time.
    \i If applicable, QDeclarativeParserStatus::componentComplete() is called on objects.
    \endlist
    QDeclarativeComponent::beginCreate() differs from QDeclarativeComponent::create() in that it
    only performs step 1.  QDeclarativeComponent::completeCreate() must be called to
    complete steps 2 and 3.

    This breaking point is sometimes useful when using attached properties to
    communicate information to an instantiated component, as it allows their
    initial values to be configured before property bindings take effect.
*/
QObject *QDeclarativeComponent::beginCreate(QDeclarativeContext *context)
{
   Q_D(QDeclarativeComponent);
   QObject *rv = d->beginCreate(context ? QDeclarativeContextData::get(context) : 0, QBitField());
   if (rv) {
      QDeclarativeData *ddata = QDeclarativeData::get(rv);
      Q_ASSERT(ddata);
      ddata->indestructible = true;
   }
   return rv;
}

QObject *
QDeclarativeComponentPrivate::beginCreate(QDeclarativeContextData *context, const QBitField &bindings)
{
   Q_Q(QDeclarativeComponent);
   if (!context) {
      qWarning("QDeclarativeComponent: Cannot create a component in a null context");
      return 0;
   }

   if (!context->isValid()) {
      qWarning("QDeclarativeComponent: Cannot create a component in an invalid context");
      return 0;
   }

   if (context->engine != engine) {
      qWarning("QDeclarativeComponent: Must create component in context from the same QDeclarativeEngine");
      return 0;
   }

   if (state.completePending) {
      qWarning("QDeclarativeComponent: Cannot create new component instance before completing the previous");
      return 0;
   }

   if (!q->isReady()) {
      qWarning("QDeclarativeComponent: Component is not ready");
      return 0;
   }

   return begin(context, creationContext, cc, start, count, &state, 0, bindings);
}

QObject *QDeclarativeComponentPrivate::begin(QDeclarativeContextData *parentContext,
      QDeclarativeContextData *componentCreationContext,
      QDeclarativeCompiledData *component, int start, int count,
      ConstructionState *state, QList<QDeclarativeError> *errors,
      const QBitField &bindings)
{
   QDeclarativeEnginePrivate *enginePriv = QDeclarativeEnginePrivate::get(parentContext->engine);
   bool isRoot = !enginePriv->inBeginCreate;

   Q_ASSERT(!isRoot || state); // Either this isn't a root component, or a state data must be provided
   Q_ASSERT((state != 0) ^ (errors != 0)); // One of state or errors (but not both) must be provided

   if (isRoot) {
      QDeclarativeDebugTrace::startRange(QDeclarativeDebugTrace::Creating);
      QDeclarativeDebugTrace::rangeData(QDeclarativeDebugTrace::Creating, component->url);
   }

   QDeclarativeContextData *ctxt = new QDeclarativeContextData;
   ctxt->isInternal = true;
   ctxt->url = component->url;
   ctxt->imports = component->importCache;

   // Nested global imports
   if (componentCreationContext && start != -1) {
      ctxt->importedScripts = componentCreationContext->importedScripts;
   }

   component->importCache->addref();
   ctxt->setParent(parentContext);

   enginePriv->inBeginCreate = true;

   QDeclarativeVME vme;
   QObject *rv = vme.run(ctxt, component, start, count, bindings);

   if (vme.isError()) {
      if (errors) {
         *errors = vme.errors();
      } else {
         state->errors = vme.errors();
      }
   }

   if (isRoot) {
      enginePriv->inBeginCreate = false;

      state->bindValues = enginePriv->bindValues;
      state->parserStatus = enginePriv->parserStatus;
      state->finalizedParserStatus = enginePriv->finalizedParserStatus;
      state->componentAttached = enginePriv->componentAttached;
      if (state->componentAttached) {
         state->componentAttached->prev = &state->componentAttached;
      }

      enginePriv->componentAttached = 0;
      enginePriv->bindValues.clear();
      enginePriv->parserStatus.clear();
      enginePriv->finalizedParserStatus.clear();
      state->completePending = true;
      enginePriv->inProgressCreations++;
   }

   if (enginePriv->isDebugging && rv) {
      if  (!parentContext->isInternal) {
         parentContext->asQDeclarativeContextPrivate()->instances.append(rv);
      }
      QDeclarativeEngineDebugService::instance()->objectCreated(parentContext->engine, rv);
   }

   return rv;
}

void QDeclarativeComponentPrivate::beginDeferred(QDeclarativeEnginePrivate *enginePriv,
      QObject *object, ConstructionState *state)
{
   bool isRoot = !enginePriv->inBeginCreate;
   enginePriv->inBeginCreate = true;

   QDeclarativeVME vme;
   vme.runDeferred(object);

   if (vme.isError()) {
      state->errors = vme.errors();
   }

   if (isRoot) {
      enginePriv->inBeginCreate = false;

      state->bindValues = enginePriv->bindValues;
      state->parserStatus = enginePriv->parserStatus;
      state->finalizedParserStatus = enginePriv->finalizedParserStatus;
      state->componentAttached = enginePriv->componentAttached;
      if (state->componentAttached) {
         state->componentAttached->prev = &state->componentAttached;
      }

      enginePriv->componentAttached = 0;
      enginePriv->bindValues.clear();
      enginePriv->parserStatus.clear();
      enginePriv->finalizedParserStatus.clear();
      state->completePending = true;
      enginePriv->inProgressCreations++;
   }
}

void QDeclarativeComponentPrivate::complete(QDeclarativeEnginePrivate *enginePriv, ConstructionState *state)
{
   if (state->completePending) {
      QT_TRY {
         for (int ii = 0; ii < state->bindValues.count(); ++ii)
         {
            QDeclarativeEnginePrivate::SimpleList<QDeclarativeAbstractBinding> bv =
            state->bindValues.at(ii);
            for (int jj = 0; jj < bv.count; ++jj) {
               if (bv.at(jj)) {
                  // XXX akennedy
                  bv.at(jj)->m_mePtr = 0;
                  bv.at(jj)->setEnabled(true, QDeclarativePropertyPrivate::BypassInterceptor |
                  QDeclarativePropertyPrivate::DontRemoveBinding);
               }
            }
            QDeclarativeEnginePrivate::clear(bv);
         }

         for (int ii = 0; ii < state->parserStatus.count(); ++ii)
         {
            QDeclarativeEnginePrivate::SimpleList<QDeclarativeParserStatus> ps =
               state->parserStatus.at(ii);

            for (int jj = ps.count - 1; jj >= 0; --jj) {
               QDeclarativeParserStatus *status = ps.at(jj);
               if (status && status->d) {
                  status->d = 0;
                  status->componentComplete();
               }
            }
            QDeclarativeEnginePrivate::clear(ps);
         }

         for (int ii = 0; ii < state->finalizedParserStatus.count(); ++ii)
         {
            QPair<QDeclarativeGuard<QObject>, int> status = state->finalizedParserStatus.at(ii);
            QObject *obj = status.first;
            if (obj) {
               void *args[] = { 0 };
               QMetaObject::metacall(obj, QMetaObject::InvokeMetaMethod,
                                     status.second, args);
            }
         }

         //componentComplete() can register additional finalization objects
         //that are then never handled. Handle them manually here.
         if (1 == enginePriv->inProgressCreations)
         {
            for (int ii = 0; ii < enginePriv->finalizedParserStatus.count(); ++ii) {
               QPair<QDeclarativeGuard<QObject>, int> status = enginePriv->finalizedParserStatus.at(ii);
               QObject *obj = status.first;
               if (obj) {
                  void *args[] = { 0 };
                  QMetaObject::metacall(obj, QMetaObject::InvokeMetaMethod,
                                        status.second, args);
               }
            }
            enginePriv->finalizedParserStatus.clear();
         }

         while (state->componentAttached)
         {
            QDeclarativeComponentAttached *a = state->componentAttached;
            a->rem();
            QDeclarativeData *d = QDeclarativeData::get(a->parent());
            Q_ASSERT(d);
            Q_ASSERT(d->context);
            a->add(&d->context->componentAttached);
            emit a->completed();
         }
      } QT_CATCH(const std::exception &) {
         state->bindValues.clear();
         state->parserStatus.clear();
         state->finalizedParserStatus.clear();
         state->completePending = false;
         enginePriv->inProgressCreations--;
         QT_RETHROW;
      }

      state->bindValues.clear();
      state->parserStatus.clear();
      state->finalizedParserStatus.clear();
      state->completePending = false;

      enginePriv->inProgressCreations--;
      if (0 == enginePriv->inProgressCreations) {
         while (enginePriv->erroredBindings) {
            enginePriv->warning(enginePriv->erroredBindings->error);
            enginePriv->erroredBindings->removeError();
         }
      }
   }
}

/*!
    This method provides more advanced control over component instance creation.
    In general, programmers should use QDeclarativeComponent::create() to create a
    component.

    Complete a component creation begin with QDeclarativeComponent::beginCreate().
*/
void QDeclarativeComponent::completeCreate()
{
   Q_D(QDeclarativeComponent);
   d->completeCreate();
}

void QDeclarativeComponentPrivate::completeCreate()
{
   if (state.completePending) {
      QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(engine);
      complete(ep, &state);

      QDeclarativeDebugTrace::endRange(QDeclarativeDebugTrace::Creating);
   }
}

QDeclarativeComponentAttached::QDeclarativeComponentAttached(QObject *parent)
   : QObject(parent), prev(0), next(0)
{
}

QDeclarativeComponentAttached::~QDeclarativeComponentAttached()
{
   if (prev) {
      *prev = next;
   }
   if (next) {
      next->prev = prev;
   }
   prev = 0;
   next = 0;
}

/*!
    \internal
*/
QDeclarativeComponentAttached *QDeclarativeComponent::qmlAttachedProperties(QObject *obj)
{
   QDeclarativeComponentAttached *a = new QDeclarativeComponentAttached(obj);

   QDeclarativeEngine *engine = qmlEngine(obj);
   if (!engine) {
      return a;
   }

   if (QDeclarativeEnginePrivate::get(engine)->inBeginCreate) {
      QDeclarativeEnginePrivate *p = QDeclarativeEnginePrivate::get(engine);
      a->add(&p->componentAttached);
   } else {
      QDeclarativeData *d = QDeclarativeData::get(obj);
      Q_ASSERT(d);
      Q_ASSERT(d->context);
      a->add(&d->context->componentAttached);
   }

   return a;
}

QT_END_NAMESPACE
