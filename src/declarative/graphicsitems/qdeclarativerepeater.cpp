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

#include <qdeclarativerepeater_p.h>
#include <qdeclarativerepeater_p_p.h>
#include <qdeclarativevisualitemmodel_p.h>
#include <private/qdeclarativeglobal_p.h>
#include <qdeclarativelistaccessor_p.h>
#include <qlistmodelinterface_p.h>

QT_BEGIN_NAMESPACE

QDeclarativeRepeaterPrivate::QDeclarativeRepeaterPrivate()
   : model(0), ownModel(false)
{
}

QDeclarativeRepeaterPrivate::~QDeclarativeRepeaterPrivate()
{
   if (ownModel) {
      delete model;
   }
}

/*!
    \qmlclass Repeater QDeclarativeRepeater
    \ingroup qml-utility-elements
    \since 4.7
    \inherits Item

    \brief The Repeater element allows you to repeat an Item-based component using a model.

    The Repeater element is used to create a large number of
    similar items. Like other view elements, a Repeater has a \l model and a \l delegate:
    for each entry in the model, the delegate is instantiated
    in a context seeded with data from the model. A Repeater item is usually
    enclosed in a positioner element such as \l Row or \l Column to visually
    position the multiple delegate items created by the Repeater.

    The following Repeater creates three instances of a \l Rectangle item within
    a \l Row:

    \snippet doc/src/snippets/declarative/repeaters/repeater.qml import
    \codeline
    \snippet doc/src/snippets/declarative/repeaters/repeater.qml simple

    \image repeater-simple.png

    A Repeater's \l model can be any of the supported \l {qmlmodels}{data models}.
    Additionally, like delegates for other views, a Repeater delegate can access
    its index within the repeater, as well as the model data relevant to the
    delegate. See the \l delegate property documentation for details.

    Items instantiated by the Repeater are inserted, in order, as
    children of the Repeater's parent.  The insertion starts immediately after
    the repeater's position in its parent stacking list.  This allows
    a Repeater to be used inside a layout. For example, the following Repeater's
    items are stacked between a red rectangle and a blue rectangle:

    \snippet doc/src/snippets/declarative/repeaters/repeater.qml layout

    \image repeater.png


    \note A Repeater item owns all items it instantiates. Removing or dynamically destroying
    an item created by a Repeater results in unpredictable behavior.


    \section2 Considerations when using Repeater

    The Repeater element creates all of its delegate items when the repeater is first
    created. This can be inefficient if there are a large number of delegate items and
    not all of the items are required to be visible at the same time. If this is the case,
    consider using other view elements like ListView (which only creates delegate items
    when they are scrolled into view) or use the \l {Dynamic Object Creation} methods to
    create items as they are required.

    Also, note that Repeater is \l {Item}-based, and can only repeat \l {Item}-derived objects.
    For example, it cannot be used to repeat QtObjects:
    \badcode
    Item {
        //XXX does not work! Can't repeat QtObject as it doesn't derive from Item.
        Repeater {
            model: 10
            QtObject {}
        }
    }
    \endcode
 */

/*!
    \qmlsignal Repeater::onItemAdded(int index, Item item)
    \since QtQuick 1.1

    This handler is called when an item is added to the repeater. The \a index
    parameter holds the index at which the item has been inserted within the
    repeater, and the \a item parameter holds the \l Item that has been added.
*/

/*!
    \qmlsignal Repeater::onItemRemoved(int index, Item item)
    \since QtQuick 1.1

    This handler is called when an item is removed from the repeater. The \a index
    parameter holds the index at which the item was removed from the repeater,
    and the \a item parameter holds the \l Item that was removed.

    Do not keep a reference to \a item if it was created by this repeater, as
    in these cases it will be deleted shortly after the handler is called.
*/

QDeclarativeRepeater::QDeclarativeRepeater(QDeclarativeItem *parent)
   : QDeclarativeItem(*(new QDeclarativeRepeaterPrivate), parent)
{
}

QDeclarativeRepeater::~QDeclarativeRepeater()
{
}

/*!
    \qmlproperty any Repeater::model

    The model providing data for the repeater.

    This property can be set to any of the supported \l {qmlmodels}{data models}:

    \list
    \o A number that indicates the number of delegates to be created by the repeater
    \o A model (e.g. a ListModel item, or a QAbstractItemModel subclass)
    \o A string list
    \o An object list
    \endlist

    The type of model affects the properties that are exposed to the \l delegate.

    \sa {qmlmodels}{Data Models}
*/
QVariant QDeclarativeRepeater::model() const
{
   Q_D(const QDeclarativeRepeater);
   return d->dataSource;
}

void QDeclarativeRepeater::setModel(const QVariant &model)
{
   Q_D(QDeclarativeRepeater);
   if (d->dataSource == model) {
      return;
   }

   clear();
   if (d->model) {
      disconnect(d->model, SIGNAL(itemsInserted(int, int)), this, SLOT(itemsInserted(int, int)));
      disconnect(d->model, SIGNAL(itemsRemoved(int, int)), this, SLOT(itemsRemoved(int, int)));
      disconnect(d->model, SIGNAL(itemsMoved(int, int, int)), this, SLOT(itemsMoved(int, int, int)));
      disconnect(d->model, SIGNAL(modelReset()), this, SLOT(modelReset()));
      /*
      disconnect(d->model, SIGNAL(createdItem(int,QDeclarativeItem*)), this, SLOT(createdItem(int,QDeclarativeItem*)));
      disconnect(d->model, SIGNAL(destroyingItem(QDeclarativeItem*)), this, SLOT(destroyingItem(QDeclarativeItem*)));
      */
   }
   d->dataSource = model;
   QObject *object = qvariant_cast<QObject *>(model);
   QDeclarativeVisualModel *vim = 0;
   if (object && (vim = qobject_cast<QDeclarativeVisualModel *>(object))) {
      if (d->ownModel) {
         delete d->model;
         d->ownModel = false;
      }
      d->model = vim;
   } else {
      if (!d->ownModel) {
         d->model = new QDeclarativeVisualDataModel(qmlContext(this), this);
         d->ownModel = true;
      }
      if (QDeclarativeVisualDataModel *dataModel = qobject_cast<QDeclarativeVisualDataModel *>(d->model)) {
         dataModel->setModel(model);
      }
   }
   if (d->model) {
      connect(d->model, SIGNAL(itemsInserted(int, int)), this, SLOT(itemsInserted(int, int)));
      connect(d->model, SIGNAL(itemsRemoved(int, int)), this, SLOT(itemsRemoved(int, int)));
      connect(d->model, SIGNAL(itemsMoved(int, int, int)), this, SLOT(itemsMoved(int, int, int)));
      connect(d->model, SIGNAL(modelReset()), this, SLOT(modelReset()));
      /*
      connect(d->model, SIGNAL(createdItem(int,QDeclarativeItem*)), this, SLOT(createdItem(int,QDeclarativeItem*)));
      connect(d->model, SIGNAL(destroyingItem(QDeclarativeItem*)), this, SLOT(destroyingItem(QDeclarativeItem*)));
      */
      regenerate();
   }
   emit modelChanged();
   emit countChanged();
}

/*!
    \qmlproperty Component Repeater::delegate
    \default

    The delegate provides a template defining each item instantiated by the repeater.

    Delegates are exposed to a read-only \c index property that indicates the index
    of the delegate within the repeater. For example, the following \l Text delegate
    displays the index of each repeated item:

    \table
    \row
    \o \snippet doc/src/snippets/declarative/repeaters/repeater.qml index
    \o \image repeater-index.png
    \endtable

    If the \l model is a \l{QStringList-based model}{string list} or
    \l{QObjectList-based model}{object list}, the delegate is also exposed to
    a read-only \c modelData property that holds the string or object data. For
    example:

    \table
    \row
    \o \snippet doc/src/snippets/declarative/repeaters/repeater.qml modeldata
    \o \image repeater-modeldata.png
    \endtable

    If the \l model is a model object (such as a \l ListModel) the delegate
    can access all model roles as named properties, in the same way that delegates
    do for view classes like ListView.

    \sa {QML Data Models}
 */
QDeclarativeComponent *QDeclarativeRepeater::delegate() const
{
   Q_D(const QDeclarativeRepeater);
   if (d->model) {
      if (QDeclarativeVisualDataModel *dataModel = qobject_cast<QDeclarativeVisualDataModel *>(d->model)) {
         return dataModel->delegate();
      }
   }

   return 0;
}

void QDeclarativeRepeater::setDelegate(QDeclarativeComponent *delegate)
{
   Q_D(QDeclarativeRepeater);
   if (QDeclarativeVisualDataModel *dataModel = qobject_cast<QDeclarativeVisualDataModel *>(d->model))
      if (delegate == dataModel->delegate()) {
         return;
      }

   if (!d->ownModel) {
      d->model = new QDeclarativeVisualDataModel(qmlContext(this));
      d->ownModel = true;
   }
   if (QDeclarativeVisualDataModel *dataModel = qobject_cast<QDeclarativeVisualDataModel *>(d->model)) {
      dataModel->setDelegate(delegate);
      regenerate();
      emit delegateChanged();
   }
}

/*!
    \qmlproperty int Repeater::count

    This property holds the number of items in the repeater.
*/
int QDeclarativeRepeater::count() const
{
   Q_D(const QDeclarativeRepeater);
   if (d->model) {
      return d->model->count();
   }
   return 0;
}

/*!
    \qmlmethod Item Repeater::itemAt(index)
    \since QtQuick 1.1

    Returns the \l Item that has been created at the given \a index, or \c null
    if no item exists at \a index.
*/
QDeclarativeItem *QDeclarativeRepeater::itemAt(int index) const
{
   Q_D(const QDeclarativeRepeater);
   if (index >= 0 && index < d->deletables.count()) {
      return d->deletables[index];
   }
   return 0;

}

void QDeclarativeRepeater::componentComplete()
{
   QDeclarativeItem::componentComplete();
   regenerate();
}

QVariant QDeclarativeRepeater::itemChange(GraphicsItemChange change,
      const QVariant &value)
{
   QVariant rv = QDeclarativeItem::itemChange(change, value);
   if (change == ItemParentHasChanged) {
      regenerate();
   }

   return rv;
}

void QDeclarativeRepeater::clear()
{
   Q_D(QDeclarativeRepeater);
   bool complete = isComponentComplete();

   if (d->model) {
      while (d->deletables.count() > 0) {
         QDeclarativeItem *item = d->deletables.takeLast();
         if (complete) {
            emit itemRemoved(d->deletables.count() - 1, item);
         }
         d->model->release(item);
      }
   }
   d->deletables.clear();
}

void QDeclarativeRepeater::regenerate()
{
   Q_D(QDeclarativeRepeater);
   if (!isComponentComplete()) {
      return;
   }

   clear();

   if (!d->model || !d->model->count() || !d->model->isValid() || !parentItem() || !isComponentComplete()) {
      return;
   }

   for (int ii = 0; ii < count(); ++ii) {
      QDeclarativeItem *item = d->model->item(ii);
      if (item) {
         QDeclarative_setParent_noEvent(item, parentItem());
         item->setParentItem(parentItem());
         item->stackBefore(this);
         d->deletables << item;
         emit itemAdded(ii, item);
      }
   }
}

void QDeclarativeRepeater::itemsInserted(int index, int count)
{
   Q_D(QDeclarativeRepeater);
   if (!isComponentComplete()) {
      return;
   }
   for (int i = 0; i < count; ++i) {
      int modelIndex = index + i;
      QDeclarativeItem *item = d->model->item(modelIndex);
      if (item) {
         QDeclarative_setParent_noEvent(item, parentItem());
         item->setParentItem(parentItem());
         if (modelIndex < d->deletables.count()) {
            item->stackBefore(d->deletables.at(modelIndex));
         } else {
            item->stackBefore(this);
         }
         d->deletables.insert(modelIndex, item);
         emit itemAdded(modelIndex, item);
      }
   }
   emit countChanged();
}

void QDeclarativeRepeater::itemsRemoved(int index, int count)
{
   Q_D(QDeclarativeRepeater);
   if (!isComponentComplete() || count <= 0) {
      return;
   }
   while (count--) {
      QDeclarativeItem *item = d->deletables.takeAt(index);
      emit itemRemoved(index, item);
      if (item) {
         d->model->release(item);
      } else {
         break;
      }
   }
   emit countChanged();
}

void QDeclarativeRepeater::itemsMoved(int from, int to, int count)
{
   Q_D(QDeclarativeRepeater);
   if (!isComponentComplete() || count <= 0) {
      return;
   }
   if (from + count > d->deletables.count()) {
      regenerate();
      return;
   }
   QList<QDeclarativeItem *> removed;
   int removedCount = count;
   while (removedCount--) {
      removed << d->deletables.takeAt(from);
   }
   for (int i = 0; i < count; ++i) {
      d->deletables.insert(to + i, removed.at(i));
   }
   d->deletables.last()->stackBefore(this);
   for (int i = d->model->count() - 1; i > 0; --i) {
      QDeclarativeItem *item = d->deletables.at(i - 1);
      item->stackBefore(d->deletables.at(i));
   }
}

void QDeclarativeRepeater::modelReset()
{
   if (!isComponentComplete()) {
      return;
   }
   regenerate();
   emit countChanged();
}

QT_END_NAMESPACE
