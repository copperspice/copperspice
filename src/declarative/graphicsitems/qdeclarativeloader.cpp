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

#include "private/qdeclarativeloader_p_p.h"

#include <qdeclarativeinfo.h>
#include <qdeclarativeengine_p.h>
#include <qdeclarativeglobal_p.h>

QT_BEGIN_NAMESPACE

QDeclarativeLoaderPrivate::QDeclarativeLoaderPrivate()
   : item(0), component(0), ownComponent(false), updatingSize(false),
     itemWidthValid(false), itemHeightValid(false)
{
}

QDeclarativeLoaderPrivate::~QDeclarativeLoaderPrivate()
{
}

void QDeclarativeLoaderPrivate::itemGeometryChanged(QDeclarativeItem *resizeItem, const QRectF &newGeometry,
      const QRectF &oldGeometry)
{
   if (resizeItem == item) {
      if (!updatingSize && newGeometry.width() != oldGeometry.width()) {
         itemWidthValid = true;
      }
      if (!updatingSize && newGeometry.height() != oldGeometry.height()) {
         itemHeightValid = true;
      }
      _q_updateSize(false);
   }
   QDeclarativeItemChangeListener::itemGeometryChanged(resizeItem, newGeometry, oldGeometry);
}

void QDeclarativeLoaderPrivate::clear()
{
   if (ownComponent) {
      component->deleteLater();
      component = 0;
      ownComponent = false;
   }
   source = QUrl();

   if (item) {
      if (QDeclarativeItem *qmlItem = qobject_cast<QDeclarativeItem *>(item)) {
         QDeclarativeItemPrivate *p =
            static_cast<QDeclarativeItemPrivate *>(QGraphicsItemPrivate::get(qmlItem));
         p->removeItemChangeListener(this, QDeclarativeItemPrivate::Geometry);
      }

      // We can't delete immediately because our item may have triggered
      // the Loader to load a different item.
      if (item->scene()) {
         item->scene()->removeItem(item);
      } else {
         item->setParentItem(0);
         item->setVisible(false);
      }
      item->deleteLater();
      item = 0;
   }
}

void QDeclarativeLoaderPrivate::initResize()
{
   Q_Q(QDeclarativeLoader);
   if (QDeclarativeItem *qmlItem = qobject_cast<QDeclarativeItem *>(item)) {
      QDeclarativeItemPrivate *p =
         static_cast<QDeclarativeItemPrivate *>(QGraphicsItemPrivate::get(qmlItem));
      p->addItemChangeListener(this, QDeclarativeItemPrivate::Geometry);
      // We may override the item's size, so we need to remember
      // whether the item provided its own valid size.
      itemWidthValid = p->widthValid;
      itemHeightValid = p->heightValid;
   } else if (item && item->isWidget()) {
      QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(item);
      widget->installEventFilter(q);
   }
   _q_updateSize();
}

/*!
    \qmlclass Loader QDeclarativeLoader
    \ingroup qml-utility-elements
    \since 4.7
    \inherits Item

    \brief The Loader item allows dynamically loading an Item-based
    subtree from a URL or Component.

    Loader is used to dynamically load visual QML components. It can load a
    QML file (using the \l source property) or a \l Component object (using
    the \l sourceComponent property). It is useful for delaying the creation
    of a component until it is required: for example, when a component should
    be created on demand, or when a component should not be created
    unnecessarily for performance reasons.

    Here is a Loader that loads "Page1.qml" as a component when the
    \l MouseArea is clicked:

    \snippet doc/src/snippets/declarative/loader/simple.qml 0

    The loaded item can be accessed using the \l item property.

    If the \l source or \l sourceComponent changes, any previously instantiated
    items are destroyed. Setting \l source to an empty string or setting
    \l sourceComponent to \c undefined destroys the currently loaded item,
    freeing resources and leaving the Loader empty.

    \section2 Loader sizing behavior

    Loader is like any other visual item and must be positioned and sized
    accordingly to become visible.

    \list
    \o If an explicit size is not specified for the Loader, the Loader
    is automatically resized to the size of the loaded item once the
    component is loaded.
    \o If the size of the Loader is specified explicitly by setting
    the width, height or by anchoring, the loaded item will be resized
    to the size of the Loader.
    \endlist

    In both scenarios the size of the item and the Loader are identical.
    This ensures that anchoring to the Loader is equivalent to anchoring
    to the loaded item.

    \table
    \row
    \o sizeloader.qml
    \o sizeitem.qml
    \row
    \o \snippet doc/src/snippets/declarative/loader/sizeloader.qml 0
    \o \snippet doc/src/snippets/declarative/loader/sizeitem.qml 0
    \row
    \o The red rectangle will be sized to the size of the root item.
    \o The red rectangle will be 50x50, centered in the root item.
    \endtable


    \section2 Receiving signals from loaded items

    Any signals emitted from the loaded item can be received using the
    \l Connections element. For example, the following \c application.qml
    loads \c MyItem.qml, and is able to receive the \c message signal from
    the loaded item through a \l Connections object:

    \table
    \row
    \o application.qml
    \o MyItem.qml
    \row
    \o \snippet doc/src/snippets/declarative/loader/connections.qml 0
    \o \snippet doc/src/snippets/declarative/loader/MyItem.qml 0
    \endtable

    Alternatively, since \c MyItem.qml is loaded within the scope of the
    Loader, it could also directly call any function defined in the Loader or
    its parent \l Item.


    \section2 Focus and key events

    Loader is a focus scope. Its \l {Item::}{focus} property must be set to
    \c true for any of its children to get the \e {active focus}. (See
    \l{qmlfocus#Acquiring Focus and Focus Scopes}{the focus documentation page}
    for more details.) Any key events received in the loaded item should likely
    also be \l {KeyEvent::}{accepted} so they are not propagated to the Loader.

    For example, the following \c application.qml loads \c KeyReader.qml when
    the \l MouseArea is clicked.  Notice the \l {Item::}{focus} property is
    set to \c true for the Loader as well as the \l Item in the dynamically
    loaded object:

    \table
    \row
    \o application.qml
    \o KeyReader.qml
    \row
    \o \snippet doc/src/snippets/declarative/loader/focus.qml 0
    \o \snippet doc/src/snippets/declarative/loader/KeyReader.qml 0
    \endtable

    Once \c KeyReader.qml is loaded, it accepts key events and sets
    \c event.accepted to \c true so that the event is not propagated to the
    parent \l Rectangle.

    \sa {dynamic-object-creation}{Dynamic Object Creation}
*/

QDeclarativeLoader::QDeclarativeLoader(QDeclarativeItem *parent)
   : QDeclarativeImplicitSizeItem(*(new QDeclarativeLoaderPrivate), parent)
{
   Q_D(QDeclarativeLoader);
   d->flags |= QGraphicsItem::ItemIsFocusScope;
}

QDeclarativeLoader::~QDeclarativeLoader()
{
   Q_D(QDeclarativeLoader);
   if (d->item) {
      if (QDeclarativeItem *qmlItem = qobject_cast<QDeclarativeItem *>(d->item)) {
         QDeclarativeItemPrivate *p =
            static_cast<QDeclarativeItemPrivate *>(QGraphicsItemPrivate::get(qmlItem));
         p->removeItemChangeListener(d, QDeclarativeItemPrivate::Geometry);
      }
   }
}

/*!
    \qmlproperty url Loader::source
    This property holds the URL of the QML component to instantiate.

    Note the QML component must be an \l{Item}-based component. The loader
    cannot load non-visual components.

    To unload the currently loaded item, set this property to an empty string,
    or set \l sourceComponent to \c undefined. Setting \c source to a
    new URL will also cause the item created by the previous URL to be unloaded.

    \sa sourceComponent, status, progress
*/
QUrl QDeclarativeLoader::source() const
{
   Q_D(const QDeclarativeLoader);
   return d->source;
}

void QDeclarativeLoader::setSource(const QUrl &url)
{
   Q_D(QDeclarativeLoader);
   if (d->source == url) {
      return;
   }

   d->clear();

   d->source = url;

   if (d->source.isEmpty()) {
      emit sourceChanged();
      emit statusChanged();
      emit progressChanged();
      emit itemChanged();
      return;
   }

   d->component = new QDeclarativeComponent(qmlEngine(this), d->source, this);
   d->ownComponent = true;

   if (isComponentComplete()) {
      d->load();
   }
}

/*!
    \qmlproperty Component Loader::sourceComponent
    This property holds the \l{Component} to instantiate.

    \qml
    Item {
        Component {
            id: redSquare
            Rectangle { color: "red"; width: 10; height: 10 }
        }

        Loader { sourceComponent: redSquare }
        Loader { sourceComponent: redSquare; x: 10 }
    }
    \endqml

    To unload the currently loaded item, set this property to an empty string
    or \c undefined.

    \sa source, progress
*/

QDeclarativeComponent *QDeclarativeLoader::sourceComponent() const
{
   Q_D(const QDeclarativeLoader);
   return d->component;
}

void QDeclarativeLoader::setSourceComponent(QDeclarativeComponent *comp)
{
   Q_D(QDeclarativeLoader);
   if (comp == d->component) {
      return;
   }

   d->clear();

   d->component = comp;
   d->ownComponent = false;

   if (!d->component) {
      emit sourceChanged();
      emit statusChanged();
      emit progressChanged();
      emit itemChanged();
      return;
   }

   if (isComponentComplete()) {
      d->load();
   }
}

void QDeclarativeLoader::resetSourceComponent()
{
   setSourceComponent(0);
}

void QDeclarativeLoaderPrivate::load()
{
   Q_Q(QDeclarativeLoader);

   if (!q->isComponentComplete() || !component) {
      return;
   }

   if (!component->isLoading()) {
      _q_sourceLoaded();
   } else {
      QObject::connect(component, SIGNAL(statusChanged(QDeclarativeComponent::Status)),
                       q, SLOT(_q_sourceLoaded()));
      QObject::connect(component, SIGNAL(progressChanged(qreal)),
                       q, SIGNAL(progressChanged()));
      emit q->statusChanged();
      emit q->progressChanged();
      emit q->sourceChanged();
      emit q->itemChanged();
   }
}

void QDeclarativeLoaderPrivate::_q_sourceLoaded()
{
   Q_Q(QDeclarativeLoader);

   if (component) {
      if (!component->errors().isEmpty()) {
         QDeclarativeEnginePrivate::warning(qmlEngine(q), component->errors());
         emit q->sourceChanged();
         emit q->statusChanged();
         emit q->progressChanged();
         return;
      }

      QDeclarativeContext *creationContext = component->creationContext();
      if (!creationContext) {
         creationContext = qmlContext(q);
      }
      QDeclarativeContext *ctxt = new QDeclarativeContext(creationContext);
      ctxt->setContextObject(q);

      QDeclarativeGuard<QDeclarativeComponent> c = component;
      QObject *obj = component->beginCreate(ctxt);
      if (component != c) {
         // component->create could trigger a change in source that causes
         // component to be set to something else. In that case we just
         // need to cleanup.
         if (c) {
            c->completeCreate();
         }
         delete obj;
         delete ctxt;
         return;
      }
      if (obj) {
         item = qobject_cast<QGraphicsObject *>(obj);
         if (item) {
            QDeclarative_setParent_noEvent(ctxt, obj);
            QDeclarative_setParent_noEvent(item, q);
            item->setParentItem(q);
            //                item->setFocus(true);
            initResize();
         } else {
            qmlInfo(q) << QDeclarativeLoader::tr("Loader does not support loading non-visual elements.");
            delete obj;
            delete ctxt;
         }
      } else {
         if (!component->errors().isEmpty()) {
            QDeclarativeEnginePrivate::warning(qmlEngine(q), component->errors());
         }
         delete obj;
         delete ctxt;
         source = QUrl();
      }
      component->completeCreate();
      emit q->sourceChanged();
      emit q->statusChanged();
      emit q->progressChanged();
      emit q->itemChanged();
      emit q->loaded();
   }
}

/*!
    \qmlproperty enumeration Loader::status

    This property holds the status of QML loading.  It can be one of:
    \list
    \o Loader.Null - no QML source has been set
    \o Loader.Ready - the QML source has been loaded
    \o Loader.Loading - the QML source is currently being loaded
    \o Loader.Error - an error occurred while loading the QML source
    \endlist

    Use this status to provide an update or respond to the status change in some way.
    For example, you could:

    \list
    \o Trigger a state change:
    \qml
        State { name: 'loaded'; when: loader.status == Loader.Ready }
    \endqml

    \o Implement an \c onStatusChanged signal handler:
    \qml
        Loader {
            id: loader
            onStatusChanged: if (loader.status == Loader.Ready) console.log('Loaded')
        }
    \endqml

    \o Bind to the status value:
    \qml
        Text { text: loader.status == Loader.Ready ? 'Loaded' : 'Not loaded' }
    \endqml
    \endlist

    Note that if the source is a local file, the status will initially be Ready (or Error). While
    there will be no onStatusChanged signal in that case, the onLoaded will still be invoked.

    \sa progress
*/

QDeclarativeLoader::Status QDeclarativeLoader::status() const
{
   Q_D(const QDeclarativeLoader);

   if (d->component) {
      return static_cast<QDeclarativeLoader::Status>(d->component->status());
   }

   if (d->item) {
      return Ready;
   }

   return d->source.isEmpty() ? Null : Error;
}

void QDeclarativeLoader::componentComplete()
{
   Q_D(QDeclarativeLoader);

   QDeclarativeItem::componentComplete();
   d->load();
}


/*!
    \qmlsignal Loader::onLoaded()

    This handler is called when the \l status becomes \c Loader.Ready, or on successful
    initial load.
*/


/*!
\qmlproperty real Loader::progress

This property holds the progress of loading QML data from the network, from
0.0 (nothing loaded) to 1.0 (finished).  Most QML files are quite small, so
this value will rapidly change from 0 to 1.

\sa status
*/
qreal QDeclarativeLoader::progress() const
{
   Q_D(const QDeclarativeLoader);

   if (d->item) {
      return 1.0;
   }

   if (d->component) {
      return d->component->progress();
   }

   return 0.0;
}

void QDeclarativeLoaderPrivate::_q_updateSize(bool loaderGeometryChanged)
{
   Q_Q(QDeclarativeLoader);
   if (!item || updatingSize) {
      return;
   }

   updatingSize = true;
   if (QDeclarativeItem *qmlItem = qobject_cast<QDeclarativeItem *>(item)) {
      if (!itemWidthValid) {
         q->setImplicitWidth(qmlItem->implicitWidth());
      } else {
         q->setImplicitWidth(qmlItem->width());
      }
      if (loaderGeometryChanged && q->widthValid()) {
         qmlItem->setWidth(q->width());
      }
      if (!itemHeightValid) {
         q->setImplicitHeight(qmlItem->implicitHeight());
      } else {
         q->setImplicitHeight(qmlItem->height());
      }
      if (loaderGeometryChanged && q->heightValid()) {
         qmlItem->setHeight(q->height());
      }
   } else if (item && item->isWidget()) {
      QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(item);
      QSizeF widgetSize = widget->size();
      q->setImplicitWidth(widgetSize.width());
      if (loaderGeometryChanged && q->widthValid()) {
         widgetSize.setWidth(q->width());
      }
      q->setImplicitHeight(widgetSize.height());
      if (loaderGeometryChanged && q->heightValid()) {
         widgetSize.setHeight(q->height());
      }
      if (widget->size() != widgetSize) {
         widget->resize(widgetSize);
      }
   }
   updatingSize = false;
}

/*!
    \qmlproperty Item Loader::item
    This property holds the top-level item that is currently loaded.
*/
QGraphicsObject *QDeclarativeLoader::item() const
{
   Q_D(const QDeclarativeLoader);
   return d->item;
}

void QDeclarativeLoader::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
   Q_D(QDeclarativeLoader);
   if (newGeometry != oldGeometry) {
      d->_q_updateSize();
   }
   QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);
}

QVariant QDeclarativeLoader::itemChange(GraphicsItemChange change, const QVariant &value)
{
   Q_D(QDeclarativeLoader);
   if (change == ItemSceneHasChanged) {
      if (d->item && d->item->isWidget()) {
         d->item->removeEventFilter(this);
         d->item->installEventFilter(this);
      }
   }
   return QDeclarativeItem::itemChange(change, value);
}

bool QDeclarativeLoader::eventFilter(QObject *watched, QEvent *e)
{
   Q_D(QDeclarativeLoader);
   if (watched == d->item && e->type() == QEvent::GraphicsSceneResize) {
      if (d->item && d->item->isWidget()) {
         d->_q_updateSize(false);
      }
   }
   return QDeclarativeItem::eventFilter(watched, e);
}

void QDeclarativeLoader::_q_sourceLoaded()
{
   Q_D(QDeclarativeLoader);
   d->_q_sourceLoaded();
}

void QDeclarativeLoader::_q_updateSize()
{
   Q_D(QDeclarativeLoader);
   d->_q_updateSize();
}


QT_END_NAMESPACE
