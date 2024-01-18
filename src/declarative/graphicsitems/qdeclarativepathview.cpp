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

#include "private/qdeclarativepathview_p.h"
#include "private/qdeclarativepathview_p_p.h"

#include <qdeclarativestate_p.h>
#include <qdeclarativeopenmetaobject_p.h>
#include <QDebug>
#include <QEvent>
#include <qlistmodelinterface_p.h>
#include <QGraphicsSceneEvent>

#include <qmath.h>
#include <math.h>

QT_BEGIN_NAMESPACE

inline qreal qmlMod(qreal x, qreal y)
{
#ifdef QT_USE_MATH_H_FLOATS
   if (sizeof(qreal) == sizeof(float)) {
      return fmodf(float(x), float(y));
   } else
#endif
      return fmod(x, y);
}

static QDeclarativeOpenMetaObjectType *qPathViewAttachedType = 0;

QDeclarativePathViewAttached::QDeclarativePathViewAttached(QObject *parent)
   : QObject(parent), m_percent(-1), m_view(0), m_onPath(false), m_isCurrent(false)
{
   if (qPathViewAttachedType) {
      m_metaobject = new QDeclarativeOpenMetaObject(this, qPathViewAttachedType);
      m_metaobject->setCached(true);
   } else {
      m_metaobject = new QDeclarativeOpenMetaObject(this);
   }
}

QDeclarativePathViewAttached::~QDeclarativePathViewAttached()
{
}

QVariant QDeclarativePathViewAttached::value(const QByteArray &name) const
{
   return m_metaobject->value(name);
}
void QDeclarativePathViewAttached::setValue(const QByteArray &name, const QVariant &val)
{
   m_metaobject->setValue(name, val);
}


void QDeclarativePathViewPrivate::init()
{
   Q_Q(QDeclarativePathView);
   offset = 0;
   q->setAcceptedMouseButtons(Qt::LeftButton);
   q->setFlag(QGraphicsItem::ItemIsFocusScope);
   q->setFiltersChildEvents(true);
   q->connect(&tl, SIGNAL(updated()), q, SLOT(ticked()));
   lastPosTime.invalidate();
   static int timelineCompletedIdx = -1;
   static int movementEndingIdx = -1;
   if (timelineCompletedIdx == -1) {
      timelineCompletedIdx = QDeclarativeTimeLine::staticMetaObject.indexOfSignal("completed()");
      movementEndingIdx = QDeclarativePathView::staticMetaObject.indexOfSlot("movementEnding()");
   }
   QMetaObject::connect(&tl, timelineCompletedIdx,
                        q, movementEndingIdx, Qt::DirectConnection);
}

QDeclarativeItem *QDeclarativePathViewPrivate::getItem(int modelIndex)
{
   Q_Q(QDeclarativePathView);
   requestedIndex = modelIndex;
   QDeclarativeItem *item = model->item(modelIndex, false);
   if (item) {
      if (!attType) {
         // pre-create one metatype to share with all attached objects
         attType = new QDeclarativeOpenMetaObjectType(&QDeclarativePathViewAttached::staticMetaObject, qmlEngine(q));
         foreach(const QString & attr, path->attributes())
         attType->createProperty(attr.toUtf8());
      }
      qPathViewAttachedType = attType;
      QDeclarativePathViewAttached *att = static_cast<QDeclarativePathViewAttached *>
                                          (qmlAttachedPropertiesObject<QDeclarativePathView>(item));
      qPathViewAttachedType = 0;
      if (att) {
         att->m_view = q;
         att->setOnPath(true);
      }
      item->setParentItem(q);
      QDeclarativeItemPrivate *itemPrivate = static_cast<QDeclarativeItemPrivate *>(QGraphicsItemPrivate::get(item));
      itemPrivate->addItemChangeListener(this, QDeclarativeItemPrivate::Geometry);
   }
   requestedIndex = -1;
   return item;
}

void QDeclarativePathViewPrivate::releaseItem(QDeclarativeItem *item)
{
   if (!item || !model) {
      return;
   }
   QDeclarativeItemPrivate *itemPrivate = static_cast<QDeclarativeItemPrivate *>(QGraphicsItemPrivate::get(item));
   itemPrivate->removeItemChangeListener(this, QDeclarativeItemPrivate::Geometry);
   if (model->release(item) == 0) {
      // item was not destroyed, and we no longer reference it.
      if (QDeclarativePathViewAttached *att = attached(item)) {
         att->setOnPath(false);
      }
   }
}

QDeclarativePathViewAttached *QDeclarativePathViewPrivate::attached(QDeclarativeItem *item)
{
   return static_cast<QDeclarativePathViewAttached *>(qmlAttachedPropertiesObject<QDeclarativePathView>(item, false));
}

void QDeclarativePathViewPrivate::clear()
{
   for (int i = 0; i < items.count(); i++) {
      QDeclarativeItem *p = items[i];
      releaseItem(p);
   }
   items.clear();
   tl.clear();
}

void QDeclarativePathViewPrivate::updateMappedRange()
{
   if (model && pathItems != -1 && pathItems < modelCount) {
      mappedRange = qreal(pathItems) / modelCount;
   } else {
      mappedRange = 1.0;
   }
}

qreal QDeclarativePathViewPrivate::positionOfIndex(qreal index) const
{
   qreal pos = -1.0;

   if (model && index >= 0 && index < modelCount) {
      qreal start = 0.0;
      if (haveHighlightRange && highlightRangeMode != QDeclarativePathView::NoHighlightRange) {
         start = highlightRangeStart;
      }
      qreal globalPos = index + offset;
      globalPos = qmlMod(globalPos, qreal(modelCount)) / modelCount;
      if (pathItems != -1 && pathItems < modelCount) {
         globalPos += start * mappedRange;
         globalPos = qmlMod(globalPos, 1.0);
         if (globalPos < mappedRange) {
            pos = globalPos / mappedRange;
         }
      } else {
         pos = qmlMod(globalPos + start, 1.0);
      }
   }

   return pos;
}

void QDeclarativePathViewPrivate::createHighlight()
{
   Q_Q(QDeclarativePathView);
   if (!q->isComponentComplete()) {
      return;
   }

   bool changed = false;
   if (highlightItem) {
      if (highlightItem->scene()) {
         highlightItem->scene()->removeItem(highlightItem);
      }
      highlightItem->deleteLater();
      highlightItem = 0;
      changed = true;
   }

   QDeclarativeItem *item = 0;
   if (highlightComponent) {
      QDeclarativeContext *highlightContext = new QDeclarativeContext(qmlContext(q));
      QObject *nobj = highlightComponent->create(highlightContext);
      if (nobj) {
         QDeclarative_setParent_noEvent(highlightContext, nobj);
         item = qobject_cast<QDeclarativeItem *>(nobj);
         if (!item) {
            delete nobj;
         }
      } else {
         delete highlightContext;
      }
   } else {
      item = new QDeclarativeItem;
   }
   if (item) {
      QDeclarative_setParent_noEvent(item, q);
      item->setParentItem(q);
      highlightItem = item;
      changed = true;
   }
   if (changed) {
      emit q->highlightItemChanged();
   }
}

void QDeclarativePathViewPrivate::updateHighlight()
{
   Q_Q(QDeclarativePathView);
   if (!q->isComponentComplete() || !isValid()) {
      return;
   }
   if (highlightItem) {
      if (haveHighlightRange && highlightRangeMode == QDeclarativePathView::StrictlyEnforceRange) {
         updateItem(highlightItem, highlightRangeStart);
      } else {
         qreal target = currentIndex;

         offsetAdj = qreal(0.0);
         tl.reset(moveHighlight);
         moveHighlight.setValue(highlightPosition);

         const int duration = highlightMoveDuration;

         if (target - highlightPosition > modelCount / 2) {
            highlightUp = false;
            qreal distance = modelCount - target + highlightPosition;
            tl.move(moveHighlight, qreal(0.0), QEasingCurve(QEasingCurve::InQuad), int(duration * highlightPosition / distance));
            tl.set(moveHighlight, modelCount - qreal(0.01));
            tl.move(moveHighlight, target, QEasingCurve(QEasingCurve::OutQuad), int(duration * (modelCount - target) / distance));
         } else if (target - highlightPosition <= -modelCount / 2) {
            highlightUp = true;
            qreal distance = modelCount - highlightPosition + target;
            tl.move(moveHighlight, modelCount - qreal(0.01), QEasingCurve(QEasingCurve::InQuad),
                    int(duration * (modelCount - highlightPosition) / distance));
            tl.set(moveHighlight, qreal(0.0));
            tl.move(moveHighlight, target, QEasingCurve(QEasingCurve::OutQuad), int(duration * target / distance));
         } else {
            highlightUp = highlightPosition - target < 0;
            tl.move(moveHighlight, target, QEasingCurve(QEasingCurve::InOutQuad), duration);
         }
      }
   }
}

void QDeclarativePathViewPrivate::setHighlightPosition(qreal pos)
{
   if (pos != highlightPosition) {
      qreal start = 0.0;
      qreal end = 1.0;
      if (haveHighlightRange && highlightRangeMode != QDeclarativePathView::NoHighlightRange) {
         start = highlightRangeStart;
         end = highlightRangeEnd;
      }

      qreal range = qreal(modelCount);
      // calc normalized position of highlight relative to offset
      qreal relativeHighlight = qmlMod(pos + offset, range) / range;

      if (!highlightUp && relativeHighlight > end * mappedRange) {
         qreal diff = qreal(1.0) - relativeHighlight;
         setOffset(offset + diff * range);
      } else if (highlightUp && relativeHighlight >= (end - start) * mappedRange) {
         qreal diff = relativeHighlight - (end - start) * mappedRange;
         setOffset(offset - diff * range - qreal(0.00001));
      }

      highlightPosition = pos;
      qreal pathPos = positionOfIndex(pos);
      updateItem(highlightItem, pathPos);
      if (QDeclarativePathViewAttached *att = attached(highlightItem)) {
         att->setOnPath(pathPos != qreal(-1.0));
      }
   }
}

void QDeclarativePathView::pathUpdated()
{
   Q_D(QDeclarativePathView);
   QList<QDeclarativeItem *>::iterator it = d->items.begin();
   while (it != d->items.end()) {
      QDeclarativeItem *item = *it;
      if (QDeclarativePathViewAttached *att = d->attached(item)) {
         att->m_percent = -1;
      }
      ++it;
   }
   refill();
}

void QDeclarativePathViewPrivate::updateItem(QDeclarativeItem *item, qreal percent)
{
   if (QDeclarativePathViewAttached *att = attached(item)) {
      if (qFuzzyCompare(att->m_percent, percent)) {
         return;
      }
      att->m_percent = percent;
      foreach(const QString & attr, path->attributes())
      att->setValue(attr.toUtf8(), path->attributeAt(attr, percent));
   }
   QPointF pf = path->pointAt(percent);
   item->setX(qRound(pf.x() - item->width() / 2));
   item->setY(qRound(pf.y() - item->height() / 2));
}

void QDeclarativePathViewPrivate::regenerate()
{
   Q_Q(QDeclarativePathView);
   if (!q->isComponentComplete()) {
      return;
   }

   clear();

   if (!isValid()) {
      return;
   }

   firstIndex = -1;
   updateMappedRange();
   q->refill();
}

/*!
    \qmlclass PathView QDeclarativePathView
    \ingroup qml-view-elements
    \since 4.7
    \brief The PathView element lays out model-provided items on a path.
    \inherits Item

    A PathView displays data from models created from built-in QML elements like ListModel
    and XmlListModel, or custom model classes defined in C++ that inherit from
    QAbstractListModel.

    The view has a \l model, which defines the data to be displayed, and
    a \l delegate, which defines how the data should be displayed.
    The \l delegate is instantiated for each item on the \l path.
    The items may be flicked to move them along the path.

    For example, if there is a simple list model defined in a file \c ContactModel.qml like this:

    \snippet doc/src/snippets/declarative/pathview/ContactModel.qml 0

    This data can be represented as a PathView, like this:

    \snippet doc/src/snippets/declarative/pathview/pathview.qml 0

    \image pathview.gif

    (Note the above example uses PathAttribute to scale and modify the
    opacity of the items as they rotate. This additional code can be seen in the
    PathAttribute documentation.)

    PathView does not automatically handle keyboard navigation.  This is because
    the keys to use for navigation will depend upon the shape of the path.  Navigation
    can be added quite simply by setting \c focus to \c true and calling
    \l decrementCurrentIndex() or \l incrementCurrentIndex(), for example to navigate
    using the left and right arrow keys:

    \qml
    PathView {
        // ...
        focus: true
        Keys.onLeftPressed: decrementCurrentIndex()
        Keys.onRightPressed: incrementCurrentIndex()
    }
    \endqml

    The path view itself is a focus scope (see \l{qmlfocus#Acquiring Focus and Focus Scopes}{the focus documentation page} for more details).

    Delegates are instantiated as needed and may be destroyed at any time.
    State should \e never be stored in a delegate.

    PathView attaches a number of properties to the root item of the delegate, for example
    \c {PathView.isCurrentItem}.  In the following example, the root delegate item can access
    this attached property directly as \c PathView.isCurrentItem, while the child
    \c nameText object must refer to this property as \c wrapper.PathView.isCurrentItem.

    \snippet doc/src/snippets/declarative/pathview/pathview.qml 1

    \bold Note that views do not enable \e clip automatically.  If the view
    is not clipped by another item or the screen, it will be necessary
    to set \e {clip: true} in order to have the out of view items clipped
    nicely.

    \sa Path, {declarative/modelviews/pathview}{PathView example}
*/

QDeclarativePathView::QDeclarativePathView(QDeclarativeItem *parent)
   : QDeclarativeItem(*(new QDeclarativePathViewPrivate), parent)
{
   Q_D(QDeclarativePathView);
   d->init();
}

QDeclarativePathView::~QDeclarativePathView()
{
   Q_D(QDeclarativePathView);
   d->clear();
   if (d->attType) {
      d->attType->release();
   }
   if (d->ownModel) {
      delete d->model;
   }
}

/*!
    \qmlattachedproperty PathView PathView::view
    This attached property holds the view that manages this delegate instance.

    It is attached to each instance of the delegate.
*/

/*!
    \qmlattachedproperty bool PathView::onPath
    This attached property holds whether the item is currently on the path.

    If a pathItemCount has been set, it is possible that some items may
    be instantiated, but not considered to be currently on the path.
    Usually, these items would be set invisible, for example:

    \qml
    Component {
        Rectangle {
            visible: PathView.onPath
            // ...
        }
    }
    \endqml

    It is attached to each instance of the delegate.
*/

/*!
    \qmlattachedproperty bool PathView::isCurrentItem
    This attached property is true if this delegate is the current item; otherwise false.

    It is attached to each instance of the delegate.

    This property may be used to adjust the appearance of the current item.

    \snippet doc/src/snippets/declarative/pathview/pathview.qml 1
*/

/*!
    \qmlproperty model PathView::model
    This property holds the model providing data for the view.

    The model provides a set of data that is used to create the items for the view.
    For large or dynamic datasets the model is usually provided by a C++ model object.
    Models can also be created directly in QML, using the ListModel element.

    \note changing the model will reset the offset and currentIndex to 0.

    \sa {qmlmodels}{Data Models}
*/
QVariant QDeclarativePathView::model() const
{
   Q_D(const QDeclarativePathView);
   return d->modelVariant;
}

void QDeclarativePathView::setModel(const QVariant &model)
{
   Q_D(QDeclarativePathView);
   if (d->modelVariant == model) {
      return;
   }

   if (d->model) {
      disconnect(d->model, SIGNAL(itemsInserted(int, int)), this, SLOT(itemsInserted(int, int)));
      disconnect(d->model, SIGNAL(itemsRemoved(int, int)), this, SLOT(itemsRemoved(int, int)));
      disconnect(d->model, SIGNAL(itemsMoved(int, int, int)), this, SLOT(itemsMoved(int, int, int)));
      disconnect(d->model, SIGNAL(modelReset()), this, SLOT(modelReset()));
      disconnect(d->model, SIGNAL(createdItem(int, QDeclarativeItem *)), this, SLOT(createdItem(int, QDeclarativeItem *)));
      d->clear();
   }

   d->modelVariant = model;
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
   int oldModelCount = d->modelCount;
   d->modelCount = 0;
   if (d->model) {
      connect(d->model, SIGNAL(itemsInserted(int, int)), this, SLOT(itemsInserted(int, int)));
      connect(d->model, SIGNAL(itemsRemoved(int, int)), this, SLOT(itemsRemoved(int, int)));
      connect(d->model, SIGNAL(itemsMoved(int, int, int)), this, SLOT(itemsMoved(int, int, int)));
      connect(d->model, SIGNAL(modelReset()), this, SLOT(modelReset()));
      connect(d->model, SIGNAL(createdItem(int, QDeclarativeItem *)), this, SLOT(createdItem(int, QDeclarativeItem *)));
      d->modelCount = d->model->count();
   }
   if (isComponentComplete()) {
      if (d->currentIndex != 0) {
         d->currentIndex = 0;
         emit currentIndexChanged();
      }
      if (d->offset != 0.0) {
         d->offset = 0;
         emit offsetChanged();
      }
   }
   d->regenerate();
   if (d->modelCount != oldModelCount) {
      emit countChanged();
   }
   emit modelChanged();
}

/*!
    \qmlproperty int PathView::count
    This property holds the number of items in the model.
*/
int QDeclarativePathView::count() const
{
   Q_D(const QDeclarativePathView);
   return d->model ? d->modelCount : 0;
}

/*!
    \qmlproperty Path PathView::path
    This property holds the path used to lay out the items.
    For more information see the \l Path documentation.
*/
QDeclarativePath *QDeclarativePathView::path() const
{
   Q_D(const QDeclarativePathView);
   return d->path;
}

void QDeclarativePathView::setPath(QDeclarativePath *path)
{
   Q_D(QDeclarativePathView);
   if (d->path == path) {
      return;
   }
   if (d->path) {
      disconnect(d->path, SIGNAL(changed()), this, SLOT(pathUpdated()));
   }
   d->path = path;
   connect(d->path, SIGNAL(changed()), this, SLOT(pathUpdated()));
   if (d->isValid() && isComponentComplete()) {
      d->clear();
      if (d->attType) {
         d->attType->release();
         d->attType = 0;
      }
      d->regenerate();
   }
   emit pathChanged();
}

/*!
    \qmlproperty int PathView::currentIndex
    This property holds the index of the current item.
*/
int QDeclarativePathView::currentIndex() const
{
   Q_D(const QDeclarativePathView);
   return d->currentIndex;
}

void QDeclarativePathView::setCurrentIndex(int idx)
{
   Q_D(QDeclarativePathView);
   if (!isComponentComplete()) {
      if (idx != d->currentIndex) {
         d->currentIndex = idx;
         emit currentIndexChanged();
      }
      return;
   }

   idx = d->modelCount
         ? ((idx % d->modelCount) + d->modelCount) % d->modelCount
         : 0;
   if (d->model && idx != d->currentIndex) {
      if (d->modelCount) {
         int itemIndex = (d->currentIndex - d->firstIndex + d->modelCount) % d->modelCount;
         if (itemIndex < d->items.count()) {
            if (QDeclarativeItem *item = d->items.at(itemIndex)) {
               if (QDeclarativePathViewAttached *att = d->attached(item)) {
                  att->setIsCurrentItem(false);
               }
            }
         }
      }
      d->currentItem = 0;
      d->moveReason = QDeclarativePathViewPrivate::SetIndex;
      d->currentIndex = idx;
      if (d->modelCount) {
         if (d->haveHighlightRange && d->highlightRangeMode == QDeclarativePathView::StrictlyEnforceRange) {
            d->snapToCurrent();
         }
         int itemIndex = (idx - d->firstIndex + d->modelCount) % d->modelCount;
         if (itemIndex < d->items.count()) {
            d->currentItem = d->items.at(itemIndex);
            d->currentItem->setFocus(true);
            if (QDeclarativePathViewAttached *att = d->attached(d->currentItem)) {
               att->setIsCurrentItem(true);
            }
         }
         d->currentItemOffset = d->positionOfIndex(d->currentIndex);
         d->updateHighlight();
      }
      emit currentIndexChanged();
   }
}

/*!
    \qmlmethod PathView::incrementCurrentIndex()

    Increments the current index.

    \bold Note: methods should only be called after the Component has completed.
*/
void QDeclarativePathView::incrementCurrentIndex()
{
   Q_D(QDeclarativePathView);
   d->moveDirection = QDeclarativePathViewPrivate::Positive;
   setCurrentIndex(currentIndex() + 1);
}


/*!
    \qmlmethod PathView::decrementCurrentIndex()

    Decrements the current index.

    \bold Note: methods should only be called after the Component has completed.
*/
void QDeclarativePathView::decrementCurrentIndex()
{
   Q_D(QDeclarativePathView);
   d->moveDirection = QDeclarativePathViewPrivate::Negative;
   setCurrentIndex(currentIndex() - 1);
}

/*!
    \qmlproperty real PathView::offset

    The offset specifies how far along the path the items are from their initial positions.
    This is a real number that ranges from 0.0 to the count of items in the model.
*/
qreal QDeclarativePathView::offset() const
{
   Q_D(const QDeclarativePathView);
   return d->offset;
}

void QDeclarativePathView::setOffset(qreal offset)
{
   Q_D(QDeclarativePathView);
   d->setOffset(offset);
   d->updateCurrent();
}

void QDeclarativePathViewPrivate::setOffset(qreal o)
{
   Q_Q(QDeclarativePathView);
   if (offset != o) {
      if (isValid() && q->isComponentComplete()) {
         offset = qmlMod(o, qreal(modelCount));
         if (offset < 0) {
            offset += qreal(modelCount);
         }
         q->refill();
      } else {
         offset = o;
      }
      emit q->offsetChanged();
   }
}

void QDeclarativePathViewPrivate::setAdjustedOffset(qreal o)
{
   setOffset(o + offsetAdj);
}

/*!
    \qmlproperty Component PathView::highlight
    This property holds the component to use as the highlight.

    An instance of the highlight component will be created for each view.
    The geometry of the resultant component instance will be managed by the view
    so as to stay with the current item.

    The below example demonstrates how to make a simple highlight.  Note the use
    of the \l{PathView::onPath}{PathView.onPath} attached property to ensure that
    the highlight is hidden when flicked away from the path.

    \qml
    Component {
        Rectangle {
            visible: PathView.onPath
            // ...
        }
    }
    \endqml

    \sa highlightItem, highlightRangeMode
*/

QDeclarativeComponent *QDeclarativePathView::highlight() const
{
   Q_D(const QDeclarativePathView);
   return d->highlightComponent;
}

void QDeclarativePathView::setHighlight(QDeclarativeComponent *highlight)
{
   Q_D(QDeclarativePathView);
   if (highlight != d->highlightComponent) {
      d->highlightComponent = highlight;
      d->createHighlight();
      d->updateHighlight();
      emit highlightChanged();
   }
}

/*!
  \qmlproperty Item PathView::highlightItem

  \c highlightItem holds the highlight item, which was created
  from the \l highlight component.

  \sa highlight
*/
QDeclarativeItem *QDeclarativePathView::highlightItem()
{
   Q_D(const QDeclarativePathView);
   return d->highlightItem;
}
/*!
    \qmlproperty real PathView::preferredHighlightBegin
    \qmlproperty real PathView::preferredHighlightEnd
    \qmlproperty enumeration PathView::highlightRangeMode

    These properties set the preferred range of the highlight (current item)
    within the view.  The preferred values must be in the range 0.0-1.0.

    If highlightRangeMode is set to \e PathView.NoHighlightRange

    If highlightRangeMode is set to \e PathView.ApplyRange the view will
    attempt to maintain the highlight within the range, however
    the highlight can move outside of the range at the ends of the path
    or due to a mouse interaction.

    If highlightRangeMode is set to \e PathView.StrictlyEnforceRange the highlight will never
    move outside of the range.  This means that the current item will change
    if a keyboard or mouse action would cause the highlight to move
    outside of the range.

    Note that this is the correct way to influence where the
    current item ends up when the view moves. For example, if you want the
    currently selected item to be in the middle of the path, then set the
    highlight range to be 0.5,0.5 and highlightRangeMode to PathView.StrictlyEnforceRange.
    Then, when the path scrolls,
    the currently selected item will be the item at that position. This also applies to
    when the currently selected item changes - it will scroll to within the preferred
    highlight range. Furthermore, the behaviour of the current item index will occur
    whether or not a highlight exists.

    The default value is \e PathView.StrictlyEnforceRange.

    Note that a valid range requires preferredHighlightEnd to be greater
    than or equal to preferredHighlightBegin.
*/
qreal QDeclarativePathView::preferredHighlightBegin() const
{
   Q_D(const QDeclarativePathView);
   return d->highlightRangeStart;
}

void QDeclarativePathView::setPreferredHighlightBegin(qreal start)
{
   Q_D(QDeclarativePathView);
   if (d->highlightRangeStart == start || start < 0 || start > 1.0) {
      return;
   }
   d->highlightRangeStart = start;
   d->haveHighlightRange = d->highlightRangeMode != NoHighlightRange && d->highlightRangeStart <= d->highlightRangeEnd;
   refill();
   emit preferredHighlightBeginChanged();
}

qreal QDeclarativePathView::preferredHighlightEnd() const
{
   Q_D(const QDeclarativePathView);
   return d->highlightRangeEnd;
}

void QDeclarativePathView::setPreferredHighlightEnd(qreal end)
{
   Q_D(QDeclarativePathView);
   if (d->highlightRangeEnd == end || end < 0 || end > 1.0) {
      return;
   }
   d->highlightRangeEnd = end;
   d->haveHighlightRange = d->highlightRangeMode != NoHighlightRange && d->highlightRangeStart <= d->highlightRangeEnd;
   refill();
   emit preferredHighlightEndChanged();
}

QDeclarativePathView::HighlightRangeMode QDeclarativePathView::highlightRangeMode() const
{
   Q_D(const QDeclarativePathView);
   return d->highlightRangeMode;
}

void QDeclarativePathView::setHighlightRangeMode(HighlightRangeMode mode)
{
   Q_D(QDeclarativePathView);
   if (d->highlightRangeMode == mode) {
      return;
   }
   d->highlightRangeMode = mode;
   d->haveHighlightRange = d->highlightRangeMode != NoHighlightRange && d->highlightRangeStart <= d->highlightRangeEnd;
   emit highlightRangeModeChanged();
}


/*!
    \qmlproperty int PathView::highlightMoveDuration
    This property holds the move animation duration of the highlight delegate.

    If the highlightRangeMode is StrictlyEnforceRange then this property
    determines the speed that the items move along the path.

    The default value for the duration is 300ms.
*/
int QDeclarativePathView::highlightMoveDuration() const
{
   Q_D(const QDeclarativePathView);
   return d->highlightMoveDuration;
}

void QDeclarativePathView::setHighlightMoveDuration(int duration)
{
   Q_D(QDeclarativePathView);
   if (d->highlightMoveDuration == duration) {
      return;
   }
   d->highlightMoveDuration = duration;
   emit highlightMoveDurationChanged();
}

/*!
    \qmlproperty real PathView::dragMargin
    This property holds the maximum distance from the path that initiate mouse dragging.

    By default the path can only be dragged by clicking on an item.  If
    dragMargin is greater than zero, a drag can be initiated by clicking
    within dragMargin pixels of the path.
*/
qreal QDeclarativePathView::dragMargin() const
{
   Q_D(const QDeclarativePathView);
   return d->dragMargin;
}

void QDeclarativePathView::setDragMargin(qreal dragMargin)
{
   Q_D(QDeclarativePathView);
   if (d->dragMargin == dragMargin) {
      return;
   }
   d->dragMargin = dragMargin;
   emit dragMarginChanged();
}

/*!
    \qmlproperty real PathView::flickDeceleration
    This property holds the rate at which a flick will decelerate.

    The default is 100.
*/
qreal QDeclarativePathView::flickDeceleration() const
{
   Q_D(const QDeclarativePathView);
   return d->deceleration;
}

void QDeclarativePathView::setFlickDeceleration(qreal dec)
{
   Q_D(QDeclarativePathView);
   if (d->deceleration == dec) {
      return;
   }
   d->deceleration = dec;
   emit flickDecelerationChanged();
}

/*!
    \qmlproperty bool PathView::interactive

    A user cannot drag or flick a PathView that is not interactive.

    This property is useful for temporarily disabling flicking. This allows
    special interaction with PathView's children.
*/
bool QDeclarativePathView::isInteractive() const
{
   Q_D(const QDeclarativePathView);
   return d->interactive;
}

void QDeclarativePathView::setInteractive(bool interactive)
{
   Q_D(QDeclarativePathView);
   if (interactive != d->interactive) {
      d->interactive = interactive;
      if (!interactive) {
         d->tl.clear();
      }
      emit interactiveChanged();
   }
}

/*!
    \qmlproperty bool PathView::moving

    This property holds whether the view is currently moving
    due to the user either dragging or flicking the view.
*/
bool QDeclarativePathView::isMoving() const
{
   Q_D(const QDeclarativePathView);
   return d->moving;
}

/*!
    \qmlproperty bool PathView::flicking

    This property holds whether the view is currently moving
    due to the user flicking the view.
*/
bool QDeclarativePathView::isFlicking() const
{
   Q_D(const QDeclarativePathView);
   return d->flicking;
}

/*!
    \qmlsignal PathView::onMovementStarted()

    This handler is called when the view begins moving due to user
    interaction.
*/

/*!
    \qmlsignal PathView::onMovementEnded()

    This handler is called when the view stops moving due to user
    interaction.  If a flick was generated, this handler will
    be triggered once the flick stops.  If a flick was not
    generated, the handler will be triggered when the
    user stops dragging - i.e. a mouse or touch release.
*/

/*!
    \qmlsignal PathView::onFlickStarted()

    This handler is called when the view is flicked.  A flick
    starts from the point that the mouse or touch is released,
    while still in motion.
*/

/*!
    \qmlsignal PathView::onFlickEnded()

    This handler is called when the view stops moving due to a flick.
*/

/*!
    \qmlproperty Component PathView::delegate

    The delegate provides a template defining each item instantiated by the view.
    The index is exposed as an accessible \c index property.  Properties of the
    model are also available depending upon the type of \l {qmlmodels}{Data Model}.

    The number of elements in the delegate has a direct effect on the
    flicking performance of the view when pathItemCount is specified.  If at all possible, place functionality
    that is not needed for the normal display of the delegate in a \l Loader which
    can load additional elements when needed.

    Note that the PathView will layout the items based on the size of the root
    item in the delegate.

    Here is an example delegate:
    \snippet doc/src/snippets/declarative/pathview/pathview.qml 1
*/
QDeclarativeComponent *QDeclarativePathView::delegate() const
{
   Q_D(const QDeclarativePathView);
   if (d->model) {
      if (QDeclarativeVisualDataModel *dataModel = qobject_cast<QDeclarativeVisualDataModel *>(d->model)) {
         return dataModel->delegate();
      }
   }

   return 0;
}

void QDeclarativePathView::setDelegate(QDeclarativeComponent *delegate)
{
   Q_D(QDeclarativePathView);
   if (delegate == this->delegate()) {
      return;
   }
   if (!d->ownModel) {
      d->model = new QDeclarativeVisualDataModel(qmlContext(this));
      d->ownModel = true;
   }
   if (QDeclarativeVisualDataModel *dataModel = qobject_cast<QDeclarativeVisualDataModel *>(d->model)) {
      int oldCount = dataModel->count();
      dataModel->setDelegate(delegate);
      d->modelCount = dataModel->count();
      d->regenerate();
      if (oldCount != dataModel->count()) {
         emit countChanged();
      }
      emit delegateChanged();
   }
}

/*!
  \qmlproperty int PathView::pathItemCount
  This property holds the number of items visible on the path at any one time.
*/
int QDeclarativePathView::pathItemCount() const
{
   Q_D(const QDeclarativePathView);
   return d->pathItems;
}

void QDeclarativePathView::setPathItemCount(int i)
{
   Q_D(QDeclarativePathView);
   if (i == d->pathItems) {
      return;
   }
   if (i < 1) {
      i = 1;
   }
   d->pathItems = i;
   d->updateMappedRange();
   if (d->isValid() && isComponentComplete()) {
      d->regenerate();
   }
   emit pathItemCountChanged();
}

QPointF QDeclarativePathViewPrivate::pointNear(const QPointF &point, qreal *nearPercent) const
{
   //XXX maybe do recursively at increasing resolution.
   qreal mindist = 1e10; // big number
   QPointF nearPoint = path->pointAt(0);
   qreal nearPc = 0;
   for (qreal i = 1; i < 1000; i++) {
      QPointF pt = path->pointAt(i / 1000.0);
      QPointF diff = pt - point;
      qreal dist = diff.x() * diff.x() + diff.y() * diff.y();
      if (dist < mindist) {
         nearPoint = pt;
         nearPc = i;
         mindist = dist;
      }
   }

   if (nearPercent) {
      *nearPercent = nearPc / 1000.0;
   }

   return nearPoint;
}

void QDeclarativePathView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativePathView);
   if (d->interactive) {
      d->handleMousePressEvent(event);
      event->accept();
   } else {
      QDeclarativeItem::mousePressEvent(event);
   }
}

void QDeclarativePathViewPrivate::handleMousePressEvent(QGraphicsSceneMouseEvent *event)
{
   Q_Q(QDeclarativePathView);
   if (!interactive || !items.count()) {
      return;
   }
   QPointF scenePoint = q->mapToScene(event->pos());
   int idx = 0;
   for (; idx < items.count(); ++idx) {
      QRectF rect = items.at(idx)->boundingRect();
      rect = items.at(idx)->mapToScene(rect).boundingRect();
      if (rect.contains(scenePoint)) {
         break;
      }
   }
   if (idx == items.count() && dragMargin == 0.) { // didn't click on an item
      return;
   }

   startPoint = pointNear(event->pos(), &startPc);
   if (idx == items.count()) {
      qreal distance = qAbs(event->pos().x() - startPoint.x()) + qAbs(event->pos().y() - startPoint.y());
      if (distance > dragMargin) {
         return;
      }
   }

   if (tl.isActive() && flicking) {
      stealMouse = true;   // If we've been flicked then steal the click.
   } else {
      stealMouse = false;
   }

   lastElapsed = 0;
   lastDist = 0;
   QDeclarativeItemPrivate::start(lastPosTime);
   tl.clear();
}

void QDeclarativePathView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativePathView);
   if (d->interactive) {
      d->handleMouseMoveEvent(event);
      if (d->stealMouse) {
         setKeepMouseGrab(true);
      }
      event->accept();
   } else {
      QDeclarativeItem::mouseMoveEvent(event);
   }
}

void QDeclarativePathViewPrivate::handleMouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
   Q_Q(QDeclarativePathView);
   if (!interactive || !lastPosTime.isValid()) {
      return;
   }

   qreal newPc;
   QPointF pathPoint = pointNear(event->pos(), &newPc);
   if (!stealMouse) {
      QPointF delta = pathPoint - startPoint;
      if (qAbs(delta.x()) > QApplication::startDragDistance() || qAbs(delta.y()) > QApplication::startDragDistance()) {
         stealMouse = true;
         startPc = newPc;
      }
   }

   if (stealMouse) {
      moveReason = QDeclarativePathViewPrivate::Mouse;
      qreal diff = (newPc - startPc) * modelCount * mappedRange;
      if (diff) {
         q->setOffset(offset + diff);

         if (diff > modelCount / 2) {
            diff -= modelCount;
         } else if (diff < -modelCount / 2) {
            diff += modelCount;
         }

         lastElapsed = QDeclarativeItemPrivate::restart(lastPosTime);
         lastDist = diff;
         startPc = newPc;
      }
      if (!moving) {
         moving = true;
         emit q->movingChanged();
         emit q->movementStarted();
      }
   }
}

void QDeclarativePathView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativePathView);
   if (d->interactive) {
      d->handleMouseReleaseEvent(event);
      event->accept();
      ungrabMouse();
   } else {
      QDeclarativeItem::mouseReleaseEvent(event);
   }
}

void QDeclarativePathViewPrivate::handleMouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
   Q_Q(QDeclarativePathView);
   stealMouse = false;
   q->setKeepMouseGrab(false);
   if (!interactive || !lastPosTime.isValid()) {
      return;
   }

   qreal elapsed = qreal(lastElapsed + QDeclarativeItemPrivate::elapsed(lastPosTime)) / 1000.;
   qreal velocity = elapsed > 0. ? lastDist / elapsed : 0;
   if (model && modelCount && qAbs(velocity) > qreal(1.)) {
      qreal count = pathItems == -1 ? modelCount : pathItems;
      if (qAbs(velocity) > count * 2) { // limit velocity
         velocity = (velocity > 0 ? count : -count) * 2;
      }
      // Calculate the distance to be travelled
      qreal v2 = velocity * velocity;
      qreal accel = deceleration / 10;
      // + 0.25 to encourage moving at least one item in the flick direction
      qreal dist = qMin(qreal(modelCount - 1), qreal(v2 / (accel * qreal(2.0)) + qreal(0.25)));
      if (haveHighlightRange && highlightRangeMode == QDeclarativePathView::StrictlyEnforceRange) {
         // round to nearest item.
         if (velocity > 0.) {
            dist = qRound(dist + offset) - offset;
         } else {
            dist = qRound(dist - offset) + offset;
         }
         // Calculate accel required to stop on item boundary
         if (dist <= 0.) {
            dist = qreal(0.);
            accel = qreal(0.);
         } else {
            accel = v2 / (2.0f * qAbs(dist));
         }
      }
      offsetAdj = qreal(0.0);
      moveOffset.setValue(offset);
      tl.accel(moveOffset, velocity, accel, dist);
      tl.callback(QDeclarativeTimeLineCallback(&moveOffset, fixOffsetCallback, this));
      if (!flicking) {
         flicking = true;
         emit q->flickingChanged();
         emit q->flickStarted();
      }
   } else {
      fixOffset();
   }

   lastPosTime.invalidate();
   if (!tl.isActive()) {
      q->movementEnding();
   }
}

bool QDeclarativePathView::sendMouseEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativePathView);
   QGraphicsSceneMouseEvent mouseEvent(event->type());
   QRectF myRect = mapToScene(QRectF(0, 0, width(), height())).boundingRect();
   QGraphicsScene *s = scene();
   QDeclarativeItem *grabber = s ? qobject_cast<QDeclarativeItem *>(s->mouseGrabberItem()) : 0;
   bool stealThisEvent = d->stealMouse;
   if ((stealThisEvent || myRect.contains(event->scenePos().toPoint())) && (!grabber || !grabber->keepMouseGrab())) {
      mouseEvent.setAccepted(false);
      for (int i = 0x1; i <= 0x10; i <<= 1) {
         if (event->buttons() & i) {
            Qt::MouseButton button = Qt::MouseButton(i);
            mouseEvent.setButtonDownPos(button, mapFromScene(event->buttonDownPos(button)));
         }
      }
      mouseEvent.setScenePos(event->scenePos());
      mouseEvent.setLastScenePos(event->lastScenePos());
      mouseEvent.setPos(mapFromScene(event->scenePos()));
      mouseEvent.setLastPos(mapFromScene(event->lastScenePos()));

      switch (mouseEvent.type()) {
         case QEvent::GraphicsSceneMouseMove:
            d->handleMouseMoveEvent(&mouseEvent);
            break;
         case QEvent::GraphicsSceneMousePress:
            d->handleMousePressEvent(&mouseEvent);
            stealThisEvent = d->stealMouse;   // Update stealThisEvent in case changed by function call above
            break;
         case QEvent::GraphicsSceneMouseRelease:
            d->handleMouseReleaseEvent(&mouseEvent);
            break;
         default:
            break;
      }
      grabber = qobject_cast<QDeclarativeItem *>(s->mouseGrabberItem());
      if (grabber && stealThisEvent && !grabber->keepMouseGrab() && grabber != this) {
         grabMouse();
      }

      return d->stealMouse;
   } else if (d->lastPosTime.isValid()) {
      d->lastPosTime.invalidate();
   }
   if (mouseEvent.type() == QEvent::GraphicsSceneMouseRelease) {
      d->stealMouse = false;
   }
   return false;
}

bool QDeclarativePathView::sceneEventFilter(QGraphicsItem *i, QEvent *e)
{
   Q_D(QDeclarativePathView);
   if (!isVisible() || !d->interactive) {
      return QDeclarativeItem::sceneEventFilter(i, e);
   }

   switch (e->type()) {
      case QEvent::GraphicsSceneMousePress:
      case QEvent::GraphicsSceneMouseMove:
      case QEvent::GraphicsSceneMouseRelease:
         return sendMouseEvent(static_cast<QGraphicsSceneMouseEvent *>(e));
      default:
         break;
   }

   return QDeclarativeItem::sceneEventFilter(i, e);
}

bool QDeclarativePathView::event(QEvent *event)
{
   if (event->type() == QEvent::User) {
      refill();
      return true;
   }

   return QDeclarativeItem::event(event);
}

void QDeclarativePathView::componentComplete()
{
   Q_D(QDeclarativePathView);
   QDeclarativeItem::componentComplete();

   if (d->model) {
      d->modelCount = d->model->count();
      if (d->modelCount && d->currentIndex != 0) { // an initial value has been provided for currentIndex
         d->offset = qmlMod(d->modelCount - d->currentIndex, d->modelCount);
      }
   }

   d->createHighlight();
   d->regenerate();
   d->updateHighlight();
   d->updateCurrent();

   if (d->modelCount) {
      emit countChanged();
   }
}

void QDeclarativePathView::refill()
{
   Q_D(QDeclarativePathView);
   if (!d->isValid() || !isComponentComplete()) {
      return;
   }

   d->layoutScheduled = false;
   bool currentVisible = false;

   // first move existing items and remove items off path
   int idx = d->firstIndex;
   QList<QDeclarativeItem *>::iterator it = d->items.begin();
   while (it != d->items.end()) {
      qreal pos = d->positionOfIndex(idx);
      QDeclarativeItem *item = *it;
      if (pos >= 0.0) {
         d->updateItem(item, pos);
         if (idx == d->currentIndex) {
            currentVisible = true;
            d->currentItemOffset = pos;
         }
         ++it;
      } else {
         //            qDebug() << "release";
         d->updateItem(item, 1.0);
         d->releaseItem(item);
         if (it == d->items.begin()) {
            if (++d->firstIndex >= d->modelCount) {
               d->firstIndex = 0;
            }
         }
         it = d->items.erase(it);
      }
      ++idx;
      if (idx >= d->modelCount) {
         idx = 0;
      }
   }
   if (!d->items.count()) {
      d->firstIndex = -1;
   }

   if (d->modelCount) {
      // add items to beginning and end
      int count = d->pathItems == -1 ? d->modelCount : qMin(d->pathItems, d->modelCount);
      if (d->items.count() < count) {
         int idx = qRound(d->modelCount - d->offset) % d->modelCount;
         qreal startPos = 0.0;
         if (d->haveHighlightRange && d->highlightRangeMode != QDeclarativePathView::NoHighlightRange) {
            startPos = d->highlightRangeStart;
         }
         if (d->firstIndex >= 0) {
            startPos = d->positionOfIndex(d->firstIndex);
            idx = (d->firstIndex + d->items.count()) % d->modelCount;
         }
         qreal pos = d->positionOfIndex(idx);
         while ((pos > startPos || !d->items.count()) && d->items.count() < count) {
            //            qDebug() << "append" << idx;
            QDeclarativeItem *item = d->getItem(idx);
            if (d->model->completePending()) {
               item->setZValue(idx + 1);
            }
            if (d->currentIndex == idx) {
               item->setFocus(true);
               if (QDeclarativePathViewAttached *att = d->attached(item)) {
                  att->setIsCurrentItem(true);
               }
               currentVisible = true;
               d->currentItemOffset = pos;
               d->currentItem = item;
            }
            if (d->items.count() == 0) {
               d->firstIndex = idx;
            }
            d->items.append(item);
            d->updateItem(item, pos);
            if (d->model->completePending()) {
               d->model->completeItem();
            }
            ++idx;
            if (idx >= d->modelCount) {
               idx = 0;
            }
            pos = d->positionOfIndex(idx);
         }

         idx = d->firstIndex - 1;
         if (idx < 0) {
            idx = d->modelCount - 1;
         }
         pos = d->positionOfIndex(idx);
         while (pos >= 0.0 && pos < startPos) {
            //            qDebug() << "prepend" << idx;
            QDeclarativeItem *item = d->getItem(idx);
            if (d->model->completePending()) {
               item->setZValue(idx + 1);
            }
            if (d->currentIndex == idx) {
               item->setFocus(true);
               if (QDeclarativePathViewAttached *att = d->attached(item)) {
                  att->setIsCurrentItem(true);
               }
               currentVisible = true;
               d->currentItemOffset = pos;
               d->currentItem = item;
            }
            d->items.prepend(item);
            d->updateItem(item, pos);
            if (d->model->completePending()) {
               d->model->completeItem();
            }
            d->firstIndex = idx;
            idx = d->firstIndex - 1;
            if (idx < 0) {
               idx = d->modelCount - 1;
            }
            pos = d->positionOfIndex(idx);
         }
      }
   }

   if (!currentVisible) {
      d->currentItemOffset = 1.0;
   }

   if (d->highlightItem && d->haveHighlightRange && d->highlightRangeMode == QDeclarativePathView::StrictlyEnforceRange) {
      d->updateItem(d->highlightItem, d->highlightRangeStart);
      if (QDeclarativePathViewAttached *att = d->attached(d->highlightItem)) {
         att->setOnPath(true);
      }
   } else if (d->highlightItem && d->moveReason != QDeclarativePathViewPrivate::SetIndex) {
      d->updateItem(d->highlightItem, d->currentItemOffset);
      if (QDeclarativePathViewAttached *att = d->attached(d->highlightItem)) {
         att->setOnPath(currentVisible);
      }
   }
   while (d->itemCache.count()) {
      d->releaseItem(d->itemCache.takeLast());
   }
}

void QDeclarativePathView::itemsInserted(int modelIndex, int count)
{
   //XXX support animated insertion
   Q_D(QDeclarativePathView);
   if (!d->isValid() || !isComponentComplete()) {
      return;
   }

   if (d->modelCount) {
      d->itemCache += d->items;
      d->items.clear();
      if (modelIndex <= d->currentIndex) {
         d->currentIndex += count;
         emit currentIndexChanged();
      } else if (d->offset != 0) {
         d->offset += count;
         d->offsetAdj += count;
      }
   }
   d->modelCount += count;
   if (d->flicking || d->moving) {
      d->regenerate();
      d->updateCurrent();
   } else {
      d->firstIndex = -1;
      d->updateMappedRange();
      d->scheduleLayout();
   }
   emit countChanged();
}

void QDeclarativePathView::itemsRemoved(int modelIndex, int count)
{
   //XXX support animated removal
   Q_D(QDeclarativePathView);
   if (!d->model || !d->modelCount || !d->model->isValid() || !d->path || !isComponentComplete()) {
      return;
   }

   // fix current
   bool currentChanged = false;
   if (d->currentIndex >= modelIndex + count) {
      d->currentIndex -= count;
      currentChanged = true;
   } else if (d->currentIndex >= modelIndex && d->currentIndex < modelIndex + count) {
      // current item has been removed.
      d->currentIndex = qMin(modelIndex, d->modelCount - count - 1);
      if (d->currentItem) {
         if (QDeclarativePathViewAttached *att = d->attached(d->currentItem)) {
            att->setIsCurrentItem(true);
         }
      }
      currentChanged = true;
   }

   d->itemCache += d->items;
   d->items.clear();

   bool changedOffset = false;
   if (modelIndex > d->currentIndex) {
      if (d->offset >= count) {
         changedOffset = true;
         d->offset -= count;
         d->offsetAdj -= count;
      }
   }

   d->modelCount -= count;

   if (d->currentIndex == -1) {
      d->currentIndex = d->calcCurrentIndex();
   }

   if (!d->modelCount) {
      while (d->itemCache.count()) {
         d->releaseItem(d->itemCache.takeLast());
      }
      d->offset = 0;
      changedOffset = true;
      d->tl.reset(d->moveOffset);
      update();
   } else {
      d->regenerate();
      d->updateCurrent();
      if (!d->flicking && !d->moving && d->haveHighlightRange &&
            d->highlightRangeMode == QDeclarativePathView::StrictlyEnforceRange) {
         d->snapToCurrent();
      }
   }
   if (changedOffset) {
      emit offsetChanged();
   }
   if (currentChanged) {
      emit currentIndexChanged();
   }
   emit countChanged();
}

void QDeclarativePathView::itemsMoved(int /*from*/, int /*to*/, int /*count*/)
{
   Q_D(QDeclarativePathView);
   if (!d->isValid() || !isComponentComplete()) {
      return;
   }

   int oldCurrent = d->currentIndex;
   // Fix current index
   if (d->currentIndex >= 0 && d->currentItem) {
      d->currentIndex = d->model->indexOf(d->currentItem, this);
   }

   QList<QDeclarativeItem *> removedItems = d->items;
   d->items.clear();
   d->regenerate();
   while (removedItems.count()) {
      d->releaseItem(removedItems.takeLast());
   }

   if (oldCurrent != d->currentIndex) {
      emit currentIndexChanged();
   }
   d->updateCurrent();
}

void QDeclarativePathView::modelReset()
{
   Q_D(QDeclarativePathView);
   d->modelCount = d->model->count();
   d->regenerate();
   emit countChanged();
}

void QDeclarativePathView::createdItem(int index, QDeclarativeItem *item)
{
   Q_D(QDeclarativePathView);
   if (d->requestedIndex != index) {
      if (!d->attType) {
         // pre-create one metatype to share with all attached objects
         d->attType = new QDeclarativeOpenMetaObjectType(&QDeclarativePathViewAttached::staticMetaObject, qmlEngine(this));
         foreach(const QString & attr, d->path->attributes())
         d->attType->createProperty(attr.toUtf8());
      }
      qPathViewAttachedType = d->attType;
      QDeclarativePathViewAttached *att = static_cast<QDeclarativePathViewAttached *>
                                          (qmlAttachedPropertiesObject<QDeclarativePathView>(item));
      qPathViewAttachedType = 0;
      if (att) {
         att->m_view = this;
         att->setOnPath(false);
      }
      item->setParentItem(this);
      d->updateItem(item, index < d->firstIndex ? qreal(0.0) : qreal(1.0));
   }
}

void QDeclarativePathView::destroyingItem(QDeclarativeItem *item)
{
   Q_UNUSED(item);
}

void QDeclarativePathView::ticked()
{
   Q_D(QDeclarativePathView);
   d->updateCurrent();
}

void QDeclarativePathView::movementEnding()
{
   Q_D(QDeclarativePathView);
   if (d->flicking) {
      d->flicking = false;
      emit flickingChanged();
      emit flickEnded();
   }
   if (d->moving && !d->stealMouse) {
      d->moving = false;
      emit movingChanged();
      emit movementEnded();
   }
}

// find the item closest to the snap position
int QDeclarativePathViewPrivate::calcCurrentIndex()
{
   int current = 0;
   if (modelCount && model && items.count()) {
      offset = qmlMod(offset, modelCount);
      if (offset < 0) {
         offset += modelCount;
      }
      current = qRound(qAbs(qmlMod(modelCount - offset, modelCount)));
      current = current % modelCount;
   }

   return current;
}

void QDeclarativePathViewPrivate::updateCurrent()
{
   Q_Q(QDeclarativePathView);
   if (moveReason != Mouse) {
      return;
   }
   if (!modelCount || !haveHighlightRange || highlightRangeMode != QDeclarativePathView::StrictlyEnforceRange) {
      return;
   }

   int idx = calcCurrentIndex();
   if (model && (idx != currentIndex || !currentItem)) {
      int itemIndex = (currentIndex - firstIndex + modelCount) % modelCount;
      if (itemIndex < items.count()) {
         if (QDeclarativeItem *item = items.at(itemIndex)) {
            if (QDeclarativePathViewAttached *att = attached(item)) {
               att->setIsCurrentItem(false);
            }
         }
      }
      int oldCurrentIndex = currentIndex;
      currentIndex = idx;
      currentItem = 0;
      itemIndex = (idx - firstIndex + modelCount) % modelCount;
      if (itemIndex < items.count()) {
         currentItem = items.at(itemIndex);
         currentItem->setFocus(true);
         if (QDeclarativePathViewAttached *att = attached(currentItem)) {
            att->setIsCurrentItem(true);
         }
      }
      if (oldCurrentIndex != currentIndex) {
         emit q->currentIndexChanged();
      }
   }
}

void QDeclarativePathViewPrivate::fixOffsetCallback(void *d)
{
   ((QDeclarativePathViewPrivate *)d)->fixOffset();
}

void QDeclarativePathViewPrivate::fixOffset()
{
   Q_Q(QDeclarativePathView);
   if (model && items.count()) {
      if (haveHighlightRange && highlightRangeMode == QDeclarativePathView::StrictlyEnforceRange) {
         int curr = calcCurrentIndex();
         if (curr != currentIndex) {
            q->setCurrentIndex(curr);
         } else {
            snapToCurrent();
         }
      }
   }
}

void QDeclarativePathViewPrivate::snapToCurrent()
{
   if (!model || modelCount <= 0) {
      return;
   }

   qreal targetOffset = qmlMod(modelCount - currentIndex, modelCount);

   moveReason = Other;
   offsetAdj = 0.0;
   tl.reset(moveOffset);
   moveOffset.setValue(offset);

   const int duration = highlightMoveDuration;

   if (moveDirection == Positive || (moveDirection == Shortest && targetOffset - offset > modelCount / 2)) {
      qreal distance = modelCount - targetOffset + offset;
      if (targetOffset > moveOffset) {
         tl.move(moveOffset, 0.0, QEasingCurve(QEasingCurve::InQuad), int(duration * offset / distance));
         tl.set(moveOffset, modelCount);
         tl.move(moveOffset, targetOffset, QEasingCurve(offset == 0.0 ? QEasingCurve::InOutQuad : QEasingCurve::OutQuad),
                 int(duration * (modelCount - targetOffset) / distance));
      } else {
         tl.move(moveOffset, targetOffset, QEasingCurve(QEasingCurve::InOutQuad), duration);
      }
   } else if (moveDirection == Negative || targetOffset - offset <= -modelCount / 2) {
      qreal distance = modelCount - offset + targetOffset;
      if (targetOffset < moveOffset) {
         tl.move(moveOffset, modelCount, QEasingCurve(targetOffset == 0 ? QEasingCurve::InOutQuad : QEasingCurve::InQuad),
                 int(duration * (modelCount - offset) / distance));
         tl.set(moveOffset, 0.0);
         tl.move(moveOffset, targetOffset, QEasingCurve(QEasingCurve::OutQuad), int(duration * targetOffset / distance));
      } else {
         tl.move(moveOffset, targetOffset, QEasingCurve(QEasingCurve::InOutQuad), duration);
      }
   } else {
      tl.move(moveOffset, targetOffset, QEasingCurve(QEasingCurve::InOutQuad), duration);
   }
   moveDirection = Shortest;
}

QDeclarativePathViewAttached *QDeclarativePathView::qmlAttachedProperties(QObject *obj)
{
   return new QDeclarativePathViewAttached(obj);
}

QT_END_NAMESPACE

