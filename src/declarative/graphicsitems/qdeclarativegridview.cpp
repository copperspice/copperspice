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

#include <qdeclarativegridview_p.h>
#include <qdeclarativevisualitemmodel_p.h>
#include <qdeclarativeflickable_p_p.h>
#include <qdeclarativesmoothedanimation_p_p.h>
#include <qdeclarativeguard_p.h>
#include <qlistmodelinterface_p.h>

#include <QKeyEvent>
#include <qmath.h>
#include <math.h>
#include <qplatformdefs.h>

QT_BEGIN_NAMESPACE

#ifndef QML_FLICK_SNAPONETHRESHOLD
#define QML_FLICK_SNAPONETHRESHOLD 30
#endif

//----------------------------------------------------------------------------

class FxGridItem
{
 public:
   FxGridItem(QDeclarativeItem *i, QDeclarativeGridView *v) : item(i), view(v) {
      attached = static_cast<QDeclarativeGridViewAttached *>(qmlAttachedPropertiesObject<QDeclarativeGridView>(item));
      if (attached) {
         attached->setView(view);
      }
   }
   ~FxGridItem() {}

   qreal rowPos() const {
      qreal rowPos = 0;
      if (view->flow() == QDeclarativeGridView::LeftToRight) {
         rowPos = item->y();
      } else {
         if (view->effectiveLayoutDirection() == Qt::RightToLeft) {
            rowPos = -view->cellWidth() - item->x();
         } else {
            rowPos = item->x();
         }
      }
      return rowPos;
   }
   qreal colPos() const {
      qreal colPos = 0;
      if (view->flow() == QDeclarativeGridView::LeftToRight) {
         if (view->effectiveLayoutDirection() == Qt::RightToLeft) {
            int colSize = view->cellWidth();
            int columns = view->width() / colSize;
            colPos = colSize * (columns - 1) - item->x();
         } else {
            colPos = item->x();
         }
      } else {
         colPos = item->y();
      }

      return colPos;
   }

   qreal endRowPos() const {
      if (view->flow() == QDeclarativeGridView::LeftToRight) {
         return item->y() + view->cellHeight() - 1;
      } else {
         if (view->effectiveLayoutDirection() == Qt::RightToLeft) {
            return -item->x() - 1;
         } else {
            return item->x() + view->cellWidth() - 1;
         }
      }
   }
   void setPosition(qreal col, qreal row) {
      if (view->effectiveLayoutDirection() == Qt::RightToLeft) {
         if (view->flow() == QDeclarativeGridView::LeftToRight) {
            int columns = view->width() / view->cellWidth();
            item->setPos(QPointF((view->cellWidth() * (columns - 1) - col), row));
         } else {
            item->setPos(QPointF(-view->cellWidth() - row, col));
         }
      } else {
         if (view->flow() == QDeclarativeGridView::LeftToRight) {
            item->setPos(QPointF(col, row));
         } else {
            item->setPos(QPointF(row, col));
         }
      }

   }
   bool contains(qreal x, qreal y) const {
      return (x >= item->x() && x < item->x() + view->cellWidth() &&
              y >= item->y() && y < item->y() + view->cellHeight());
   }

   QDeclarativeItem *item;
   QDeclarativeGridView *view;
   QDeclarativeGridViewAttached *attached;
   int index;
};

//----------------------------------------------------------------------------

class QDeclarativeGridViewPrivate : public QDeclarativeFlickablePrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeGridView)

 public:
   QDeclarativeGridViewPrivate()
      : currentItem(0), layoutDirection(Qt::LeftToRight), flow(QDeclarativeGridView::LeftToRight)
      , visibleIndex(0) , currentIndex(-1)
      , cellWidth(100), cellHeight(100), columns(1), requestedIndex(-1), itemCount(0)
      , highlightRangeStart(0), highlightRangeEnd(0)
      , highlightRangeStartValid(false), highlightRangeEndValid(false)
      , highlightRange(QDeclarativeGridView::NoHighlightRange)
      , highlightComponent(0), highlight(0), trackedItem(0)
      , moveReason(Other), buffer(0), highlightXAnimator(0), highlightYAnimator(0)
      , highlightMoveDuration(150)
      , footerComponent(0), footer(0), headerComponent(0), header(0)
      , bufferMode(BufferBefore | BufferAfter), snapMode(QDeclarativeGridView::NoSnap)
      , ownModel(false), wrap(false), autoHighlight(true)
      , fixCurrentVisibility(false), lazyRelease(false), layoutScheduled(false)
      , deferredRelease(false), haveHighlightRange(false), currentIndexCleared(false) {}

   void init();
   void clear();
   FxGridItem *createItem(int modelIndex);
   void releaseItem(FxGridItem *item);
   QDeclarativeItem *createComponentItem(QDeclarativeComponent *component);
   void refill(qreal from, qreal to, bool doBuffer = false);

   void updateGrid();
   void scheduleLayout();
   void layout();
   void updateUnrequestedIndexes();
   void updateUnrequestedPositions();
   void updateTrackedItem();
   void createHighlight();
   void updateHighlight();
   void updateCurrent(int modelIndex);
   void updateHeader();
   void updateFooter();
   void fixupPosition();

   FxGridItem *visibleItem(int modelIndex) const {
      if (modelIndex >= visibleIndex && modelIndex < visibleIndex + visibleItems.count()) {
         for (int i = modelIndex - visibleIndex; i < visibleItems.count(); ++i) {
            FxGridItem *item = visibleItems.at(i);
            if (item->index == modelIndex) {
               return item;
            }
         }
      }
      return 0;
   }

   bool isRightToLeftTopToBottom() const {
      Q_Q(const QDeclarativeGridView);
      return flow == QDeclarativeGridView::TopToBottom && q->effectiveLayoutDirection() == Qt::RightToLeft;
   }

   void regenerate() {
      Q_Q(QDeclarativeGridView);
      if (q->isComponentComplete()) {
         clear();
         updateGrid();
         setPosition(0);
         q->refill();
         updateCurrent(currentIndex);
      }
   }

   void mirrorChange() {
      regenerate();
   }

   qreal position() const {
      Q_Q(const QDeclarativeGridView);
      return flow == QDeclarativeGridView::LeftToRight ? q->contentY() : q->contentX();
   }
   void setPosition(qreal pos) {
      Q_Q(QDeclarativeGridView);
      if (flow == QDeclarativeGridView::LeftToRight) {
         q->QDeclarativeFlickable::setContentY(pos);
         q->QDeclarativeFlickable::setContentX(0);
      } else {
         if (q->effectiveLayoutDirection() == Qt::LeftToRight) {
            q->QDeclarativeFlickable::setContentX(pos);
         } else {
            q->QDeclarativeFlickable::setContentX(-pos - size());
         }
         q->QDeclarativeFlickable::setContentY(0);
      }
   }
   int size() const {
      Q_Q(const QDeclarativeGridView);
      return flow == QDeclarativeGridView::LeftToRight ? q->height() : q->width();
   }
   qreal originPosition() const {
      qreal pos = 0;
      if (!visibleItems.isEmpty()) {
         pos = visibleItems.first()->rowPos() - visibleIndex / columns * rowSize();
      }
      return pos;
   }

   qreal lastPosition() const {
      qreal pos = 0;
      if (model && model->count()) {
         pos = rowPosAt(model->count() - 1) + rowSize();
      }
      return pos;
   }

   qreal startPosition() const {
      return isRightToLeftTopToBottom() ? -lastPosition() + 1 : originPosition();
   }

   qreal endPosition() const {
      return isRightToLeftTopToBottom() ? -originPosition() + 1 : lastPosition();

   }

   bool isValid() const {
      return model && model->count() && model->isValid();
   }

   int rowSize() const {
      return flow == QDeclarativeGridView::LeftToRight ? cellHeight : cellWidth;
   }
   int colSize() const {
      return flow == QDeclarativeGridView::LeftToRight ? cellWidth : cellHeight;
   }

   qreal colPosAt(int modelIndex) const {
      if (FxGridItem *item = visibleItem(modelIndex)) {
         return item->colPos();
      }
      if (!visibleItems.isEmpty()) {
         if (modelIndex < visibleIndex) {
            int count = (visibleIndex - modelIndex) % columns;
            int col = visibleItems.first()->colPos() / colSize();
            col = (columns - count + col) % columns;
            return col * colSize();
         } else {
            int count = columns - 1 - (modelIndex - visibleItems.last()->index - 1) % columns;
            return visibleItems.last()->colPos() - count * colSize();
         }
      } else {
         return (modelIndex % columns) * colSize();
      }
      return 0;
   }
   qreal rowPosAt(int modelIndex) const {
      if (FxGridItem *item = visibleItem(modelIndex)) {
         return item->rowPos();
      }
      if (!visibleItems.isEmpty()) {
         if (modelIndex < visibleIndex) {
            int firstCol = visibleItems.first()->colPos() / colSize();
            int col = visibleIndex - modelIndex + (columns - firstCol - 1);
            int rows = col / columns;
            return visibleItems.first()->rowPos() - rows * rowSize();
         } else {
            int count = modelIndex - visibleItems.last()->index;
            int col = visibleItems.last()->colPos() + count * colSize();
            int rows = col / (columns * colSize());
            return visibleItems.last()->rowPos() + rows * rowSize();
         }
      } else {
         qreal pos = (modelIndex / columns) * rowSize();
         if (header) {
            pos += headerSize();
         }
         return pos;
      }
      return 0;
   }

   FxGridItem *firstVisibleItem() const {
      const qreal pos = isRightToLeftTopToBottom() ? -position() - size() : position();
      for (int i = 0; i < visibleItems.count(); ++i) {
         FxGridItem *item = visibleItems.at(i);
         if (item->index != -1 && item->endRowPos() > pos) {
            return item;
         }
      }
      return visibleItems.count() ? visibleItems.first() : 0;
   }

   int lastVisibleIndex() const {
      for (int i = 0; i < visibleItems.count(); ++i) {
         FxGridItem *item = visibleItems.at(i);
         if (item->index != -1) {
            return item->index;
         }
      }
      return -1;
   }

   // Map a model index to visibleItems list index.
   // These may differ if removed items are still present in the visible list,
   // e.g. doing a removal animation
   int mapFromModel(int modelIndex) const {
      if (modelIndex < visibleIndex || modelIndex >= visibleIndex + visibleItems.count()) {
         return -1;
      }
      for (int i = 0; i < visibleItems.count(); ++i) {
         FxGridItem *listItem = visibleItems.at(i);
         if (listItem->index == modelIndex) {
            return i + visibleIndex;
         }
         if (listItem->index > modelIndex) {
            return -1;
         }
      }
      return -1; // Not in visibleList
   }

   qreal snapPosAt(qreal pos) const {
      Q_Q(const QDeclarativeGridView);
      qreal snapPos = 0;
      if (!visibleItems.isEmpty()) {
         qreal highlightStart = isRightToLeftTopToBottom() &&
                                highlightRangeStartValid ? size() - highlightRangeEnd : highlightRangeStart;
         pos += highlightStart;
         pos += rowSize() / 2;
         snapPos = visibleItems.first()->rowPos() - visibleIndex / columns * rowSize();
         snapPos = pos - fmodf(pos - snapPos, qreal(rowSize()));
         snapPos -= highlightStart;
         qreal maxExtent;
         qreal minExtent;
         if (isRightToLeftTopToBottom()) {
            maxExtent = q->minXExtent();
            minExtent = q->maxXExtent();
         } else {
            maxExtent = flow == QDeclarativeGridView::LeftToRight ? -q->maxYExtent() : -q->maxXExtent();
            minExtent = flow == QDeclarativeGridView::LeftToRight ? -q->minYExtent() : -q->minXExtent();
         }
         if (snapPos > maxExtent) {
            snapPos = maxExtent;
         }
         if (snapPos < minExtent) {
            snapPos = minExtent;
         }
      }
      return snapPos;
   }

   FxGridItem *snapItemAt(qreal pos) {
      for (int i = 0; i < visibleItems.count(); ++i) {
         FxGridItem *item = visibleItems[i];
         if (item->index == -1) {
            continue;
         }
         qreal itemTop = item->rowPos();
         if (itemTop + rowSize() / 2 >= pos && itemTop - rowSize() / 2 <= pos) {
            return item;
         }
      }
      return 0;
   }

   int snapIndex() {
      int index = currentIndex;
      for (int i = 0; i < visibleItems.count(); ++i) {
         FxGridItem *item = visibleItems[i];
         if (item->index == -1) {
            continue;
         }
         qreal itemTop = item->rowPos();
         if (itemTop >= highlight->rowPos() - rowSize() / 2 && itemTop < highlight->rowPos() + rowSize() / 2) {
            index = item->index;
            if (item->colPos() >= highlight->colPos() - colSize() / 2 && item->colPos() < highlight->colPos() + colSize() / 2) {
               return item->index;
            }
         }
      }
      return index;
   }

   qreal headerSize() const {
      if (!header) {
         return 0.0;
      }

      return flow == QDeclarativeGridView::LeftToRight
             ? header->item->height()
             : header->item->width();
   }


   virtual void itemGeometryChanged(QDeclarativeItem *item, const QRectF &newGeometry, const QRectF &oldGeometry) {
      Q_Q(const QDeclarativeGridView);
      QDeclarativeFlickablePrivate::itemGeometryChanged(item, newGeometry, oldGeometry);
      if (item == q) {
         if (newGeometry.height() != oldGeometry.height()
               || newGeometry.width() != oldGeometry.width()) {
            if (q->isComponentComplete()) {
               updateGrid();
               scheduleLayout();
            }
         }
      } else if ((header && header->item == item) || (footer && footer->item == item)) {
         if (header) {
            updateHeader();
         }
         if (footer) {
            updateFooter();
         }
      }
   }

   void positionViewAtIndex(int index, int mode);
   virtual void fixup(AxisData &data, qreal minExtent, qreal maxExtent);
   virtual void flick(AxisData &data, qreal minExtent, qreal maxExtent, qreal vSize,
                      QDeclarativeTimeLineCallback::Callback fixupCallback, qreal velocity);

   // for debugging only
   void checkVisible() const {
      int skip = 0;
      for (int i = 0; i < visibleItems.count(); ++i) {
         FxGridItem *listItem = visibleItems.at(i);
         if (listItem->index == -1) {
            ++skip;
         } else if (listItem->index != visibleIndex + i - skip) {
            for (int j = 0; j < visibleItems.count(); j++) {
               qDebug() << " index" << j << "item index" << visibleItems.at(j)->index;
            }
            qFatal("index %d %d %d", visibleIndex, i, listItem->index);
         }
      }
   }

   QDeclarativeGuard<QDeclarativeVisualModel> model;
   QVariant modelVariant;
   QList<FxGridItem *> visibleItems;
   QHash<QDeclarativeItem *, int> unrequestedItems;
   FxGridItem *currentItem;
   Qt::LayoutDirection layoutDirection;
   QDeclarativeGridView::Flow flow;
   int visibleIndex;
   int currentIndex;
   int cellWidth;
   int cellHeight;
   int columns;
   int requestedIndex;
   int itemCount;
   qreal highlightRangeStart;
   qreal highlightRangeEnd;
   bool highlightRangeStartValid;
   bool highlightRangeEndValid;
   QDeclarativeGridView::HighlightRangeMode highlightRange;
   QDeclarativeComponent *highlightComponent;
   FxGridItem *highlight;
   FxGridItem *trackedItem;
   enum MovementReason { Other, SetIndex, Mouse };
   MovementReason moveReason;
   int buffer;
   QSmoothedAnimation *highlightXAnimator;
   QSmoothedAnimation *highlightYAnimator;
   int highlightMoveDuration;
   QDeclarativeComponent *footerComponent;
   FxGridItem *footer;
   QDeclarativeComponent *headerComponent;
   FxGridItem *header;
   enum BufferMode { NoBuffer = 0x00, BufferBefore = 0x01, BufferAfter = 0x02 };
   int bufferMode;
   QDeclarativeGridView::SnapMode snapMode;

   bool ownModel : 1;
   bool wrap : 1;
   bool autoHighlight : 1;
   bool fixCurrentVisibility : 1;
   bool lazyRelease : 1;
   bool layoutScheduled : 1;
   bool deferredRelease : 1;
   bool haveHighlightRange : 1;
   bool currentIndexCleared : 1;
};

void QDeclarativeGridViewPrivate::init()
{
   Q_Q(QDeclarativeGridView);
   QObject::connect(q, SIGNAL(movementEnded()), q, SLOT(animStopped()));
   q->setFlag(QGraphicsItem::ItemIsFocusScope);
   q->setFlickableDirection(QDeclarativeFlickable::VerticalFlick);
   addItemChangeListener(this, Geometry);
}

void QDeclarativeGridViewPrivate::clear()
{
   for (int i = 0; i < visibleItems.count(); ++i) {
      releaseItem(visibleItems.at(i));
   }
   visibleItems.clear();
   visibleIndex = 0;
   releaseItem(currentItem);
   currentItem = 0;
   createHighlight();
   trackedItem = 0;
   itemCount = 0;
}

FxGridItem *QDeclarativeGridViewPrivate::createItem(int modelIndex)
{
   Q_Q(QDeclarativeGridView);
   // create object
   requestedIndex = modelIndex;
   FxGridItem *listItem = 0;
   if (QDeclarativeItem *item = model->item(modelIndex, false)) {
      listItem = new FxGridItem(item, q);
      listItem->index = modelIndex;
      if (model->completePending()) {
         // complete
         listItem->item->setZValue(1);
         listItem->item->setParentItem(q->contentItem());
         model->completeItem();
      } else {
         listItem->item->setParentItem(q->contentItem());
      }
      unrequestedItems.remove(listItem->item);
   }
   requestedIndex = -1;
   return listItem;
}


void QDeclarativeGridViewPrivate::releaseItem(FxGridItem *item)
{
   Q_Q(QDeclarativeGridView);
   if (!item || !model) {
      return;
   }
   if (trackedItem == item) {
      QObject::disconnect(trackedItem->item, SIGNAL(yChanged()), q, SLOT(trackedPositionChanged()));
      QObject::disconnect(trackedItem->item, SIGNAL(xChanged()), q, SLOT(trackedPositionChanged()));
      trackedItem = 0;
   }
   if (model->release(item->item) == 0) {
      // item was not destroyed, and we no longer reference it.
      unrequestedItems.insert(item->item, model->indexOf(item->item, q));
   }
   delete item;
}

QDeclarativeItem *QDeclarativeGridViewPrivate::createComponentItem(QDeclarativeComponent *component)
{
   Q_Q(QDeclarativeGridView);
   QDeclarativeItem *item = 0;
   QDeclarativeContext *creationContext = component->creationContext();
   QDeclarativeContext *context = new QDeclarativeContext(
      creationContext ? creationContext : qmlContext(q));
   QObject *nobj = component->create(context);
   if (nobj) {
      QDeclarative_setParent_noEvent(context, nobj);
      item = qobject_cast<QDeclarativeItem *>(nobj);
      if (!item) {
         delete nobj;
      }
   } else {
      delete context;
   }

   return item;
}

void QDeclarativeGridViewPrivate::refill(qreal from, qreal to, bool doBuffer)
{
   Q_Q(QDeclarativeGridView);
   if (!isValid() || !q->isComponentComplete()) {
      return;
   }
   itemCount = model->count();
   qreal bufferFrom = from - buffer;
   qreal bufferTo = to + buffer;
   qreal fillFrom = from;
   qreal fillTo = to;
   if (doBuffer && (bufferMode & BufferAfter)) {
      fillTo = bufferTo;
   }
   if (doBuffer && (bufferMode & BufferBefore)) {
      fillFrom = bufferFrom;
   }

   bool changed = false;

   int colPos = colPosAt(visibleIndex);
   int rowPos = rowPosAt(visibleIndex);
   int modelIndex = visibleIndex;
   if (visibleItems.count()) {
      rowPos = visibleItems.last()->rowPos();
      colPos = visibleItems.last()->colPos() + colSize();
      if (colPos > colSize() * (columns - 1)) {
         colPos = 0;
         rowPos += rowSize();
      }
      int i = visibleItems.count() - 1;
      while (i > 0 && visibleItems.at(i)->index == -1) {
         --i;
      }
      modelIndex = visibleItems.at(i)->index + 1;
   }

   if (visibleItems.count() && (fillFrom > rowPos + rowSize() * 2
                                || fillTo < rowPosAt(visibleIndex) - rowSize())) {
      // We've jumped more than a page.  Estimate which items are now
      // visible and fill from there.
      int count = (fillFrom - (rowPos + rowSize())) / (rowSize()) * columns;
      for (int i = 0; i < visibleItems.count(); ++i) {
         releaseItem(visibleItems.at(i));
      }
      visibleItems.clear();
      modelIndex += count;
      if (modelIndex >= model->count()) {
         modelIndex = model->count() - 1;
      } else if (modelIndex < 0) {
         modelIndex = 0;
      }
      modelIndex = modelIndex / columns * columns;
      visibleIndex = modelIndex;
      colPos = colPosAt(visibleIndex);
      rowPos = rowPosAt(visibleIndex);
   }

   int colNum = colPos / colSize();

   FxGridItem *item = 0;

   // Item creation and release is staggered in order to avoid
   // creating/releasing multiple items in one frame
   // while flicking (as much as possible).
   while (modelIndex < model->count() && rowPos <= fillTo + rowSize() * (columns - colNum) / (columns + 1)) {
      //        qDebug() << "refill: append item" << modelIndex;
      if (!(item = createItem(modelIndex))) {
         break;
      }
      item->setPosition(colPos, rowPos);
      visibleItems.append(item);
      colPos += colSize();
      colNum++;
      if (colPos > colSize() * (columns - 1)) {
         colPos = 0;
         colNum = 0;
         rowPos += rowSize();
      }
      ++modelIndex;
      changed = true;
      if (doBuffer) { // never buffer more than one item per frame
         break;
      }
   }

   if (visibleItems.count()) {
      rowPos = visibleItems.first()->rowPos();
      colPos = visibleItems.first()->colPos() - colSize();
      if (colPos < 0) {
         colPos = colSize() * (columns - 1);
         rowPos -= rowSize();
      }
   }
   colNum = colPos / colSize();
   while (visibleIndex > 0 && rowPos + rowSize() - 1 >= fillFrom - rowSize() * (colNum + 1) / (columns + 1)) {
      //        qDebug() << "refill: prepend item" << visibleIndex-1 << "top pos" << rowPos << colPos;
      if (!(item = createItem(visibleIndex - 1))) {
         break;
      }
      --visibleIndex;
      item->setPosition(colPos, rowPos);
      visibleItems.prepend(item);
      colPos -= colSize();
      colNum--;
      if (colPos < 0) {
         colPos = colSize() * (columns - 1);
         colNum = columns - 1;
         rowPos -= rowSize();
      }
      changed = true;
      if (doBuffer) { // never buffer more than one item per frame
         break;
      }
   }

   if (!lazyRelease || !changed || deferredRelease) { // avoid destroying items in the same frame that we create
      while (visibleItems.count() > 1
             && (item = visibleItems.first())
             && item->rowPos() + rowSize() - 1 < bufferFrom - rowSize() * (item->colPos() / colSize() + 1) / (columns + 1)) {
         if (item->attached->delayRemove()) {
            break;
         }
         //            qDebug() << "refill: remove first" << visibleIndex << "top end pos" << item->endRowPos();
         if (item->index != -1) {
            visibleIndex++;
         }
         visibleItems.removeFirst();
         releaseItem(item);
         changed = true;
      }
      while (visibleItems.count() > 1
             && (item = visibleItems.last())
             && item->rowPos() > bufferTo + rowSize() * (columns - item->colPos() / colSize()) / (columns + 1)) {
         if (item->attached->delayRemove()) {
            break;
         }
         //            qDebug() << "refill: remove last" << visibleIndex+visibleItems.count()-1;
         visibleItems.removeLast();
         releaseItem(item);
         changed = true;
      }
      deferredRelease = false;
   } else {
      deferredRelease = true;
   }
   if (changed) {
      if (header) {
         updateHeader();
      }
      if (footer) {
         updateFooter();
      }
      if (flow == QDeclarativeGridView::LeftToRight) {
         q->setContentHeight(endPosition() - startPosition());
      } else {
         q->setContentWidth(endPosition() - startPosition());
      }
   } else if (!doBuffer && buffer && bufferMode != NoBuffer) {
      refill(from, to, true);
   }
   lazyRelease = false;
}

void QDeclarativeGridViewPrivate::updateGrid()
{
   Q_Q(QDeclarativeGridView);

   columns = (int)qMax((flow == QDeclarativeGridView::LeftToRight ? q->width() : q->height()) / colSize(), qreal(1.));
   if (isValid()) {
      if (flow == QDeclarativeGridView::LeftToRight) {
         q->setContentHeight(endPosition() - startPosition());
      } else {
         q->setContentWidth(lastPosition() - originPosition());
      }
   }
}

void QDeclarativeGridViewPrivate::scheduleLayout()
{
   Q_Q(QDeclarativeGridView);
   if (!layoutScheduled) {
      layoutScheduled = true;
      QCoreApplication::postEvent(q, new QEvent(QEvent::User), Qt::HighEventPriority);
   }
}

void QDeclarativeGridViewPrivate::layout()
{
   Q_Q(QDeclarativeGridView);
   layoutScheduled = false;
   if (!isValid() && !visibleItems.count()) {
      clear();
      return;
   }
   if (visibleItems.count()) {
      qreal rowPos = visibleItems.first()->rowPos();
      qreal colPos = visibleItems.first()->colPos();
      int col = visibleIndex % columns;
      if (colPos != col * colSize()) {
         colPos = col * colSize();
         visibleItems.first()->setPosition(colPos, rowPos);
      }
      for (int i = 1; i < visibleItems.count(); ++i) {
         FxGridItem *item = visibleItems.at(i);
         colPos += colSize();
         if (colPos > colSize() * (columns - 1)) {
            colPos = 0;
            rowPos += rowSize();
         }
         item->setPosition(colPos, rowPos);
      }
   }
   if (header) {
      updateHeader();
   }
   if (footer) {
      updateFooter();
   }
   q->refill();
   updateHighlight();
   moveReason = Other;
   if (flow == QDeclarativeGridView::LeftToRight) {
      q->setContentHeight(endPosition() - startPosition());
      fixupY();
   } else {
      q->setContentWidth(endPosition() - startPosition());
      fixupX();
   }
   updateUnrequestedPositions();
}

void QDeclarativeGridViewPrivate::updateUnrequestedIndexes()
{
   Q_Q(QDeclarativeGridView);
   QHash<QDeclarativeItem *, int>::iterator it;
   for (it = unrequestedItems.begin(); it != unrequestedItems.end(); ++it) {
      *it = model->indexOf(it.key(), q);
   }
}

void QDeclarativeGridViewPrivate::updateUnrequestedPositions()
{
   QHash<QDeclarativeItem *, int>::const_iterator it;
   for (it = unrequestedItems.begin(); it != unrequestedItems.end(); ++it) {
      QDeclarativeItem *item = it.key();
      if (flow == QDeclarativeGridView::LeftToRight) {
         item->setPos(QPointF(colPosAt(*it), rowPosAt(*it)));
      } else {
         if (isRightToLeftTopToBottom()) {
            item->setPos(QPointF(-rowPosAt(*it) - item->width(), colPosAt(*it)));
         } else {
            item->setPos(QPointF(rowPosAt(*it), colPosAt(*it)));
         }
      }
   }
}

void QDeclarativeGridViewPrivate::updateTrackedItem()
{
   Q_Q(QDeclarativeGridView);
   FxGridItem *item = currentItem;
   if (highlight) {
      item = highlight;
   }

   if (trackedItem && item != trackedItem) {
      QObject::disconnect(trackedItem->item, SIGNAL(yChanged()), q, SLOT(trackedPositionChanged()));
      QObject::disconnect(trackedItem->item, SIGNAL(xChanged()), q, SLOT(trackedPositionChanged()));
      trackedItem = 0;
   }

   if (!trackedItem && item) {
      trackedItem = item;
      QObject::connect(trackedItem->item, SIGNAL(yChanged()), q, SLOT(trackedPositionChanged()));
      QObject::connect(trackedItem->item, SIGNAL(xChanged()), q, SLOT(trackedPositionChanged()));
   }
   if (trackedItem) {
      q->trackedPositionChanged();
   }
}

void QDeclarativeGridViewPrivate::createHighlight()
{
   Q_Q(QDeclarativeGridView);
   bool changed = false;
   if (highlight) {
      if (trackedItem == highlight) {
         trackedItem = 0;
      }
      if (highlight->item->scene()) {
         highlight->item->scene()->removeItem(highlight->item);
      }
      highlight->item->deleteLater();
      delete highlight;
      highlight = 0;
      delete highlightXAnimator;
      delete highlightYAnimator;
      highlightXAnimator = 0;
      highlightYAnimator = 0;
      changed = true;
   }

   if (currentItem) {
      QDeclarativeItem *item = 0;
      if (highlightComponent) {
         item = createComponentItem(highlightComponent);
      } else {
         item = new QDeclarativeItem;
         QDeclarative_setParent_noEvent(item, q->contentItem());
         item->setParentItem(q->contentItem());
      }
      if (item) {
         QDeclarative_setParent_noEvent(item, q->contentItem());
         item->setParentItem(q->contentItem());
         highlight = new FxGridItem(item, q);
         if (currentItem && autoHighlight) {
            highlight->setPosition(currentItem->colPos(), currentItem->rowPos());
         }
         highlightXAnimator = new QSmoothedAnimation(q);
         highlightXAnimator->target = QDeclarativeProperty(highlight->item, QLatin1String("x"));
         highlightXAnimator->userDuration = highlightMoveDuration;
         highlightYAnimator = new QSmoothedAnimation(q);
         highlightYAnimator->target = QDeclarativeProperty(highlight->item, QLatin1String("y"));
         highlightYAnimator->userDuration = highlightMoveDuration;
         if (autoHighlight) {
            highlightXAnimator->restart();
            highlightYAnimator->restart();
         }
         changed = true;
      }
   }
   if (changed) {
      emit q->highlightItemChanged();
   }
}

void QDeclarativeGridViewPrivate::updateHighlight()
{
   if ((!currentItem && highlight) || (currentItem && !highlight)) {
      createHighlight();
   }
   if (currentItem && autoHighlight && highlight && !hData.moving && !vData.moving) {
      // auto-update highlight
      highlightXAnimator->to = currentItem->item->x();
      highlightYAnimator->to = currentItem->item->y();
      highlight->item->setWidth(currentItem->item->width());
      highlight->item->setHeight(currentItem->item->height());
      highlightXAnimator->restart();
      highlightYAnimator->restart();
   }
   updateTrackedItem();
}

void QDeclarativeGridViewPrivate::updateCurrent(int modelIndex)
{
   Q_Q(QDeclarativeGridView);
   if (!q->isComponentComplete() || !isValid() || modelIndex < 0 || modelIndex >= model->count()) {
      if (currentItem) {
         currentItem->attached->setIsCurrentItem(false);
         releaseItem(currentItem);
         currentItem = 0;
         currentIndex = modelIndex;
         emit q->currentIndexChanged();
         updateHighlight();
      } else if (currentIndex != modelIndex) {
         currentIndex = modelIndex;
         emit q->currentIndexChanged();
      }
      return;
   }

   if (currentItem && currentIndex == modelIndex) {
      updateHighlight();
      return;
   }

   FxGridItem *oldCurrentItem = currentItem;
   currentIndex = modelIndex;
   currentItem = createItem(modelIndex);
   fixCurrentVisibility = true;
   if (oldCurrentItem && (!currentItem || oldCurrentItem->item != currentItem->item)) {
      oldCurrentItem->attached->setIsCurrentItem(false);
   }
   if (currentItem) {
      currentItem->setPosition(colPosAt(modelIndex), rowPosAt(modelIndex));
      currentItem->item->setFocus(true);
      currentItem->attached->setIsCurrentItem(true);
   }
   updateHighlight();
   emit q->currentIndexChanged();
   releaseItem(oldCurrentItem);
}

void QDeclarativeGridViewPrivate::updateFooter()
{
   Q_Q(QDeclarativeGridView);
   if (!footer && footerComponent) {
      QDeclarativeItem *item = createComponentItem(footerComponent);
      if (item) {
         QDeclarative_setParent_noEvent(item, q->contentItem());
         item->setParentItem(q->contentItem());
         item->setZValue(1);
         QDeclarativeItemPrivate *itemPrivate = static_cast<QDeclarativeItemPrivate *>(QGraphicsItemPrivate::get(item));
         itemPrivate->addItemChangeListener(this, QDeclarativeItemPrivate::Geometry);
         footer = new FxGridItem(item, q);
      }
   }
   if (footer) {
      qreal colOffset = 0;
      qreal rowOffset;
      if (isRightToLeftTopToBottom()) {
         rowOffset = footer->item->width() - cellWidth;
      } else {
         rowOffset = 0;
         if (q->effectiveLayoutDirection() == Qt::RightToLeft) {
            colOffset = footer->item->width() - cellWidth;
         }
      }
      if (visibleItems.count()) {
         qreal endPos = lastPosition();
         if (lastVisibleIndex() == model->count() - 1) {
            footer->setPosition(colOffset, endPos + rowOffset);
         } else {
            qreal visiblePos = isRightToLeftTopToBottom() ? -position() : position() + size();
            if (endPos <= visiblePos || footer->endRowPos() < endPos + rowOffset) {
               footer->setPosition(colOffset, endPos + rowOffset);
            }
         }
      } else {
         qreal endPos = 0;
         if (header) {
            endPos += flow == QDeclarativeGridView::LeftToRight ? header->item->height() : header->item->width();
         }
         footer->setPosition(colOffset, endPos);
      }
   }
}

void QDeclarativeGridViewPrivate::updateHeader()
{
   Q_Q(QDeclarativeGridView);
   if (!header && headerComponent) {
      QDeclarativeItem *item = createComponentItem(headerComponent);
      if (item) {
         QDeclarative_setParent_noEvent(item, q->contentItem());
         item->setParentItem(q->contentItem());
         item->setZValue(1);
         QDeclarativeItemPrivate *itemPrivate = static_cast<QDeclarativeItemPrivate *>(QGraphicsItemPrivate::get(item));
         itemPrivate->addItemChangeListener(this, QDeclarativeItemPrivate::Geometry);
         header = new FxGridItem(item, q);
      }
   }
   if (header) {
      qreal colOffset = 0;
      qreal rowOffset;
      if (isRightToLeftTopToBottom()) {
         rowOffset = -cellWidth;
      } else {
         rowOffset = -headerSize();
         if (q->effectiveLayoutDirection() == Qt::RightToLeft) {
            colOffset = header->item->width() - cellWidth;
         }
      }
      if (visibleItems.count()) {
         qreal startPos = originPosition();
         if (visibleIndex == 0) {
            header->setPosition(colOffset, startPos + rowOffset);
         } else {
            qreal tempPos = isRightToLeftTopToBottom() ? -position() - size() : position();
            qreal headerPos = isRightToLeftTopToBottom() ? header->rowPos() + cellWidth - headerSize() : header->rowPos();
            if (tempPos <= startPos || headerPos > startPos + rowOffset) {
               header->setPosition(colOffset, startPos + rowOffset);
            }
         }
      } else {
         header->setPosition(colOffset, 0);
      }
   }
}

void QDeclarativeGridViewPrivate::fixupPosition()
{
   moveReason = Other;
   if (flow == QDeclarativeGridView::LeftToRight) {
      fixupY();
   } else {
      fixupX();
   }
}

void QDeclarativeGridViewPrivate::fixup(AxisData &data, qreal minExtent, qreal maxExtent)
{
   if ((flow == QDeclarativeGridView::TopToBottom && &data == &vData)
         || (flow == QDeclarativeGridView::LeftToRight && &data == &hData)) {
      return;
   }

   fixupMode = moveReason == Mouse ? fixupMode : Immediate;

   qreal highlightStart;
   qreal highlightEnd;
   qreal viewPos;
   if (isRightToLeftTopToBottom()) {
      // Handle Right-To-Left exceptions
      viewPos = -position() - size();
      highlightStart = highlightRangeStartValid ? size() - highlightRangeEnd : highlightRangeStart;
      highlightEnd = highlightRangeEndValid ? size() - highlightRangeStart : highlightRangeEnd;
   } else {
      viewPos = position();
      highlightStart = highlightRangeStart;
      highlightEnd = highlightRangeEnd;
   }

   bool strictHighlightRange = haveHighlightRange && highlightRange == QDeclarativeGridView::StrictlyEnforceRange;

   if (snapMode != QDeclarativeGridView::NoSnap) {
      qreal tempPosition = isRightToLeftTopToBottom() ? -position() - size() : position();
      if (snapMode == QDeclarativeGridView::SnapOneRow && moveReason == Mouse) {
         // if we've been dragged < rowSize()/2 then bias towards the next row
         qreal dist = data.move.value() - (data.pressPos - data.dragStartOffset);
         qreal bias = 0;
         if (data.velocity > 0 && dist > QML_FLICK_SNAPONETHRESHOLD && dist < rowSize() / 2) {
            bias = rowSize() / 2;
         } else if (data.velocity < 0 && dist < -QML_FLICK_SNAPONETHRESHOLD && dist > -rowSize() / 2) {
            bias = -rowSize() / 2;
         }
         if (isRightToLeftTopToBottom()) {
            bias = -bias;
         }
         tempPosition -= bias;
      }
      FxGridItem *topItem = snapItemAt(tempPosition + highlightStart);
      if (!topItem && strictHighlightRange && currentItem) {
         // StrictlyEnforceRange always keeps an item in range
         updateHighlight();
         topItem = currentItem;
      }
      FxGridItem *bottomItem = snapItemAt(tempPosition + highlightEnd);
      if (!bottomItem && strictHighlightRange && currentItem) {
         // StrictlyEnforceRange always keeps an item in range
         updateHighlight();
         bottomItem = currentItem;
      }
      qreal pos;
      bool isInBounds = -position() > maxExtent && -position() <= minExtent;
      if (topItem && (isInBounds || strictHighlightRange)) {
         if (topItem->index == 0 && header && tempPosition + highlightStart < header->rowPos() + headerSize() / 2 &&
               !strictHighlightRange) {
            pos = isRightToLeftTopToBottom() ? - header->rowPos() + highlightStart - size() : header->rowPos() - highlightStart;
         } else {
            if (isRightToLeftTopToBottom()) {
               pos = qMax(qMin(-topItem->rowPos() + highlightStart - size(), -maxExtent), -minExtent);
            } else {
               pos = qMax(qMin(topItem->rowPos() - highlightStart, -maxExtent), -minExtent);
            }
         }
      } else if (bottomItem && isInBounds) {
         if (isRightToLeftTopToBottom()) {
            pos = qMax(qMin(-bottomItem->rowPos() + highlightEnd - size(), -maxExtent), -minExtent);
         } else {
            pos = qMax(qMin(bottomItem->rowPos() - highlightEnd, -maxExtent), -minExtent);
         }
      } else {
         QDeclarativeFlickablePrivate::fixup(data, minExtent, maxExtent);
         return;
      }
      qreal dist = qAbs(data.move + pos);
      if (dist > 0) {
         timeline.reset(data.move);
         if (fixupMode != Immediate) {
            timeline.move(data.move, -pos, QEasingCurve(QEasingCurve::InOutQuad), fixupDuration / 2);
            data.fixingUp = true;
         } else {
            timeline.set(data.move, -pos);
         }
         vTime = timeline.time();
      }
   } else if (haveHighlightRange && highlightRange == QDeclarativeGridView::StrictlyEnforceRange) {
      if (currentItem) {
         updateHighlight();
         qreal pos = currentItem->rowPos();
         if (viewPos < pos + rowSize() - highlightEnd) {
            viewPos = pos + rowSize() - highlightEnd;
         }
         if (viewPos > pos - highlightStart) {
            viewPos = pos - highlightStart;
         }
         if (isRightToLeftTopToBottom()) {
            viewPos = -viewPos - size();
         }
         timeline.reset(data.move);
         if (viewPos != position()) {
            if (fixupMode != Immediate) {
               timeline.move(data.move, -viewPos, QEasingCurve(QEasingCurve::InOutQuad), fixupDuration / 2);
               data.fixingUp = true;
            } else {
               timeline.set(data.move, -viewPos);
            }
         }
         vTime = timeline.time();
      }
   } else {
      QDeclarativeFlickablePrivate::fixup(data, minExtent, maxExtent);
   }
   data.inOvershoot = false;
   fixupMode = Normal;
}

void QDeclarativeGridViewPrivate::flick(AxisData &data, qreal minExtent, qreal maxExtent, qreal vSize,
                                        QDeclarativeTimeLineCallback::Callback fixupCallback, qreal velocity)
{
   Q_Q(QDeclarativeGridView);
   data.fixingUp = false;
   moveReason = Mouse;
   if ((!haveHighlightRange || highlightRange != QDeclarativeGridView::StrictlyEnforceRange)
         && snapMode == QDeclarativeGridView::NoSnap) {
      QDeclarativeFlickablePrivate::flick(data, minExtent, maxExtent, vSize, fixupCallback, velocity);
      return;
   }
   qreal maxDistance = 0;
   qreal dataValue = isRightToLeftTopToBottom() ? -data.move.value() + size() : data.move.value();
   // -ve velocity means list is moving up/left
   if (velocity > 0) {
      if (data.move.value() < minExtent) {
         if (snapMode == QDeclarativeGridView::SnapOneRow) {
            // if we've been dragged < averageSize/2 then bias towards the next item
            qreal dist = data.move.value() - (data.pressPos - data.dragStartOffset);
            qreal bias = dist < rowSize() / 2 ? rowSize() / 2 : 0;
            if (isRightToLeftTopToBottom()) {
               bias = -bias;
            }
            data.flickTarget = -snapPosAt(-dataValue - bias);
            maxDistance = qAbs(data.flickTarget - data.move.value());
            velocity = maxVelocity;
         } else {
            maxDistance = qAbs(minExtent - data.move.value());
         }
      }
      if (snapMode == QDeclarativeGridView::NoSnap && highlightRange != QDeclarativeGridView::StrictlyEnforceRange) {
         data.flickTarget = minExtent;
      }
   } else {
      if (data.move.value() > maxExtent) {
         if (snapMode == QDeclarativeGridView::SnapOneRow) {
            // if we've been dragged < averageSize/2 then bias towards the next item
            qreal dist = data.move.value() - (data.pressPos - data.dragStartOffset);
            qreal bias = -dist < rowSize() / 2 ? rowSize() / 2 : 0;
            if (isRightToLeftTopToBottom()) {
               bias = -bias;
            }
            data.flickTarget = -snapPosAt(-dataValue + bias);
            maxDistance = qAbs(data.flickTarget - data.move.value());
            velocity = -maxVelocity;
         } else {
            maxDistance = qAbs(maxExtent - data.move.value());
         }
      }
      if (snapMode == QDeclarativeGridView::NoSnap && highlightRange != QDeclarativeGridView::StrictlyEnforceRange) {
         data.flickTarget = maxExtent;
      }
   }

   bool overShoot = boundsBehavior == QDeclarativeFlickable::DragAndOvershootBounds;

   if (maxDistance > 0 || overShoot) {
      // This mode requires the grid to stop exactly on a row boundary.
      qreal v = velocity;
      if (maxVelocity != -1 && maxVelocity < qAbs(v)) {
         if (v < 0) {
            v = -maxVelocity;
         } else {
            v = maxVelocity;
         }
      }
      qreal accel = deceleration;
      qreal v2 = v * v;
      qreal overshootDist = qreal(0.0);
      if ((maxDistance > qreal(0.0) && v2 / (2.0f * maxDistance) < accel) || snapMode == QDeclarativeGridView::SnapOneRow) {
         // + rowSize()/4 to encourage moving at least one item in the flick direction
         qreal dist = v2 / (accel * qreal(2.0)) + rowSize() / 4;
         dist = qMin(dist, maxDistance);
         if (v > 0) {
            dist = -dist;
         }
         if (snapMode != QDeclarativeGridView::SnapOneRow) {
            qreal distTemp = isRightToLeftTopToBottom() ? -dist : dist;
            data.flickTarget = -snapPosAt(-dataValue + distTemp);
         }
         data.flickTarget = isRightToLeftTopToBottom() ? -data.flickTarget + size() : data.flickTarget;
         if (overShoot) {
            if (data.flickTarget >= minExtent) {
               overshootDist = overShootDistance(vSize);
               data.flickTarget += overshootDist;
            } else if (data.flickTarget <= maxExtent) {
               overshootDist = overShootDistance(vSize);
               data.flickTarget -= overshootDist;
            }
         }
         qreal adjDist = -data.flickTarget + data.move.value();
         if (qAbs(adjDist) > qAbs(dist)) {
            // Prevent painfully slow flicking - adjust velocity to suit flickDeceleration
            qreal adjv2 = accel * 2.0f * qAbs(adjDist);
            if (adjv2 > v2) {
               v2 = adjv2;
               v = qSqrt(v2);
               if (dist > 0) {
                  v = -v;
               }
            }
         }
         dist = adjDist;
         accel = v2 / (2.0f * qAbs(dist));
      } else {
         data.flickTarget = velocity > 0 ? minExtent : maxExtent;
         overshootDist = overShoot ? overShootDistance(vSize) : 0;
      }
      timeline.reset(data.move);
      timeline.accel(data.move, v, accel, maxDistance + overshootDist);
      timeline.callback(QDeclarativeTimeLineCallback(&data.move, fixupCallback, this));
      if (!hData.flicking && q->xflick()) {
         hData.flicking = true;
         emit q->flickingChanged();
         emit q->flickingHorizontallyChanged();
         emit q->flickStarted();
      }
      if (!vData.flicking && q->yflick()) {
         vData.flicking = true;
         emit q->flickingChanged();
         emit q->flickingVerticallyChanged();
         emit q->flickStarted();
      }
   } else {
      timeline.reset(data.move);
      fixup(data, minExtent, maxExtent);
   }
}


//----------------------------------------------------------------------------

/*!
    \qmlclass GridView QDeclarativeGridView
    \since 4.7
    \ingroup qml-view-elements

    \inherits Flickable
    \brief The GridView item provides a grid view of items provided by a model.

    A GridView displays data from models created from built-in QML elements like ListModel
    and XmlListModel, or custom model classes defined in C++ that inherit from
    QAbstractListModel.

    A GridView has a \l model, which defines the data to be displayed, and
    a \l delegate, which defines how the data should be displayed. Items in a
    GridView are laid out horizontally or vertically. Grid views are inherently flickable
    as GridView inherits from \l Flickable.

    \section1 Example Usage

    The following example shows the definition of a simple list model defined
    in a file called \c ContactModel.qml:

    \snippet doc/src/snippets/declarative/gridview/ContactModel.qml 0

    \div {class="float-right"}
    \inlineimage gridview-simple.png
    \enddiv

    This model can be referenced as \c ContactModel in other QML files. See \l{QML Modules}
    for more information about creating reusable components like this.

    Another component can display this model data in a GridView, as in the following
    example, which creates a \c ContactModel component for its model, and a \l Column element
    (containing \l Image and \l Text elements) for its delegate.

    \clearfloat
    \snippet doc/src/snippets/declarative/gridview/gridview.qml import
    \codeline
    \snippet doc/src/snippets/declarative/gridview/gridview.qml classdocs simple

    \div {class="float-right"}
    \inlineimage gridview-highlight.png
    \enddiv

    The view will create a new delegate for each item in the model. Note that the delegate
    is able to access the model's \c name and \c portrait data directly.

    An improved grid view is shown below. The delegate is visually improved and is moved
    into a separate \c contactDelegate component.

    \clearfloat
    \snippet doc/src/snippets/declarative/gridview/gridview.qml classdocs advanced

    The currently selected item is highlighted with a blue \l Rectangle using the \l highlight property,
    and \c focus is set to \c true to enable keyboard navigation for the grid view.
    The grid view itself is a focus scope (see \l{qmlfocus#Acquiring Focus and Focus Scopes}{the focus documentation page} for more details).

    Delegates are instantiated as needed and may be destroyed at any time.
    State should \e never be stored in a delegate.

    GridView attaches a number of properties to the root item of the delegate, for example
    \c {GridView.isCurrentItem}.  In the following example, the root delegate item can access
    this attached property directly as \c GridView.isCurrentItem, while the child
    \c contactInfo object must refer to this property as \c wrapper.GridView.isCurrentItem.

    \snippet doc/src/snippets/declarative/gridview/gridview.qml isCurrentItem

    \note Views do not set the \l{Item::}{clip} property automatically.
    If the view is not clipped by another item or the screen, it will be necessary
    to set this property to true in order to clip the items that are partially or
    fully outside the view.

    \sa {declarative/modelviews/gridview}{GridView example}
*/
QDeclarativeGridView::QDeclarativeGridView(QDeclarativeItem *parent)
   : QDeclarativeFlickable(*(new QDeclarativeGridViewPrivate), parent)
{
   Q_D(QDeclarativeGridView);
   d->init();
}

QDeclarativeGridView::~QDeclarativeGridView()
{
   Q_D(QDeclarativeGridView);
   d->clear();
   if (d->ownModel) {
      delete d->model;
   }
   delete d->header;
   delete d->footer;
}

/*!
    \qmlattachedproperty bool GridView::isCurrentItem
    This attached property is true if this delegate is the current item; otherwise false.

    It is attached to each instance of the delegate.
*/

/*!
    \qmlattachedproperty GridView GridView::view
    This attached property holds the view that manages this delegate instance.

    It is attached to each instance of the delegate.

    \snippet doc/src/snippets/declarative/gridview/gridview.qml isCurrentItem
*/

/*!
    \qmlattachedproperty bool GridView::delayRemove
    This attached property holds whether the delegate may be destroyed.

    It is attached to each instance of the delegate.

    It is sometimes necessary to delay the destruction of an item
    until an animation completes.

    The example below ensures that the animation completes before
    the item is removed from the grid.

    \snippet doc/src/snippets/declarative/gridview/gridview.qml delayRemove
*/

/*!
    \qmlattachedsignal GridView::onAdd()
    This attached handler is called immediately after an item is added to the view.
*/

/*!
    \qmlattachedsignal GridView::onRemove()
    This attached handler is called immediately before an item is removed from the view.
*/


/*!
  \qmlproperty model GridView::model
  This property holds the model providing data for the grid.

    The model provides the set of data that is used to create the items
    in the view. Models can be created directly in QML using \l ListModel, \l XmlListModel
    or \l VisualItemModel, or provided by C++ model classes. If a C++ model class is
    used, it must be a subclass of \l QAbstractItemModel or a simple list.

  \sa {qmlmodels}{Data Models}
*/
QVariant QDeclarativeGridView::model() const
{
   Q_D(const QDeclarativeGridView);
   return d->modelVariant;
}

// For internal use
int QDeclarativeGridView::modelCount() const
{
   Q_D(const QDeclarativeGridView);
   return d->model->count();
}

void QDeclarativeGridView::setModel(const QVariant &model)
{
   Q_D(QDeclarativeGridView);
   if (d->modelVariant == model) {
      return;
   }
   if (d->model) {
      disconnect(d->model, SIGNAL(itemsInserted(int, int)), this, SLOT(itemsInserted(int, int)));
      disconnect(d->model, SIGNAL(itemsRemoved(int, int)), this, SLOT(itemsRemoved(int, int)));
      disconnect(d->model, SIGNAL(itemsMoved(int, int, int)), this, SLOT(itemsMoved(int, int, int)));
      disconnect(d->model, SIGNAL(modelReset()), this, SLOT(modelReset()));
      disconnect(d->model, SIGNAL(createdItem(int, QDeclarativeItem *)), this, SLOT(createdItem(int, QDeclarativeItem *)));
      disconnect(d->model, SIGNAL(destroyingItem(QDeclarativeItem *)), this, SLOT(destroyingItem(QDeclarativeItem *)));
   }
   d->clear();
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
   if (d->model) {
      d->bufferMode = QDeclarativeGridViewPrivate::BufferBefore | QDeclarativeGridViewPrivate::BufferAfter;
      if (isComponentComplete()) {
         refill();
         if ((d->currentIndex >= d->model->count() || d->currentIndex < 0) && !d->currentIndexCleared) {
            setCurrentIndex(0);
         } else {
            d->moveReason = QDeclarativeGridViewPrivate::SetIndex;
            d->updateCurrent(d->currentIndex);
            if (d->highlight && d->currentItem) {
               if (d->autoHighlight) {
                  d->highlight->setPosition(d->currentItem->colPos(), d->currentItem->rowPos());
               }
               d->updateTrackedItem();
            }
            d->moveReason = QDeclarativeGridViewPrivate::Other;
         }
      }
      connect(d->model, SIGNAL(itemsInserted(int, int)), this, SLOT(itemsInserted(int, int)));
      connect(d->model, SIGNAL(itemsRemoved(int, int)), this, SLOT(itemsRemoved(int, int)));
      connect(d->model, SIGNAL(itemsMoved(int, int, int)), this, SLOT(itemsMoved(int, int, int)));
      connect(d->model, SIGNAL(modelReset()), this, SLOT(modelReset()));
      connect(d->model, SIGNAL(createdItem(int, QDeclarativeItem *)), this, SLOT(createdItem(int, QDeclarativeItem *)));
      connect(d->model, SIGNAL(destroyingItem(QDeclarativeItem *)), this, SLOT(destroyingItem(QDeclarativeItem *)));
      emit countChanged();
   }
   emit modelChanged();
}

/*!
    \qmlproperty Component GridView::delegate

    The delegate provides a template defining each item instantiated by the view.
    The index is exposed as an accessible \c index property.  Properties of the
    model are also available depending upon the type of \l {qmlmodels}{Data Model}.

    The number of elements in the delegate has a direct effect on the
    flicking performance of the view.  If at all possible, place functionality
    that is not needed for the normal display of the delegate in a \l Loader which
    can load additional elements when needed.

    The GridView will layout the items based on the size of the root item
    in the delegate.

    \note Delegates are instantiated as needed and may be destroyed at any time.
    State should \e never be stored in a delegate.
*/
QDeclarativeComponent *QDeclarativeGridView::delegate() const
{
   Q_D(const QDeclarativeGridView);
   if (d->model) {
      if (QDeclarativeVisualDataModel *dataModel = qobject_cast<QDeclarativeVisualDataModel *>(d->model)) {
         return dataModel->delegate();
      }
   }

   return 0;
}

void QDeclarativeGridView::setDelegate(QDeclarativeComponent *delegate)
{
   Q_D(QDeclarativeGridView);
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
      if (isComponentComplete()) {
         for (int i = 0; i < d->visibleItems.count(); ++i) {
            d->releaseItem(d->visibleItems.at(i));
         }
         d->visibleItems.clear();
         d->releaseItem(d->currentItem);
         d->currentItem = 0;
         refill();
         d->moveReason = QDeclarativeGridViewPrivate::SetIndex;
         d->updateCurrent(d->currentIndex);
         if (d->highlight && d->currentItem) {
            if (d->autoHighlight) {
               d->highlight->setPosition(d->currentItem->colPos(), d->currentItem->rowPos());
            }
            d->updateTrackedItem();
         }
         d->moveReason = QDeclarativeGridViewPrivate::Other;
      }
      if (oldCount != dataModel->count()) {
         emit countChanged();
      }
      emit delegateChanged();
   }
}

/*!
  \qmlproperty int GridView::currentIndex
  \qmlproperty Item GridView::currentItem

    The \c currentIndex property holds the index of the current item, and
    \c currentItem holds the current item.  Setting the currentIndex to -1
    will clear the highlight and set currentItem to null.

    If highlightFollowsCurrentItem is \c true, setting either of these
    properties will smoothly scroll the GridView so that the current
    item becomes visible.

    Note that the position of the current item
    may only be approximate until it becomes visible in the view.
*/
int QDeclarativeGridView::currentIndex() const
{
   Q_D(const QDeclarativeGridView);
   return d->currentIndex;
}

void QDeclarativeGridView::setCurrentIndex(int index)
{
   Q_D(QDeclarativeGridView);
   if (d->requestedIndex >= 0) { // currently creating item
      return;
   }
   d->currentIndexCleared = (index == -1);
   if (index == d->currentIndex) {
      return;
   }
   if (isComponentComplete() && d->isValid()) {
      if (d->layoutScheduled) {
         d->layout();
      }
      d->moveReason = QDeclarativeGridViewPrivate::SetIndex;
      d->updateCurrent(index);
   } else {
      d->currentIndex = index;
      emit currentIndexChanged();
   }
}

QDeclarativeItem *QDeclarativeGridView::currentItem()
{
   Q_D(QDeclarativeGridView);
   if (!d->currentItem) {
      return 0;
   }
   return d->currentItem->item;
}

/*!
  \qmlproperty Item GridView::highlightItem

  This holds the highlight item created from the \l highlight component.

  The highlightItem is managed by the view unless
  \l highlightFollowsCurrentItem is set to false.

  \sa highlight, highlightFollowsCurrentItem
*/
QDeclarativeItem *QDeclarativeGridView::highlightItem()
{
   Q_D(QDeclarativeGridView);
   if (!d->highlight) {
      return 0;
   }
   return d->highlight->item;
}

/*!
  \qmlproperty int GridView::count
  This property holds the number of items in the view.
*/
int QDeclarativeGridView::count() const
{
   Q_D(const QDeclarativeGridView);
   if (d->model) {
      return d->model->count();
   }
   return 0;
}

/*!
  \qmlproperty Component GridView::highlight
  This property holds the component to use as the highlight.

  An instance of the highlight component is created for each view.
  The geometry of the resulting component instance will be managed by the view
  so as to stay with the current item, unless the highlightFollowsCurrentItem property is false.

  \sa highlightItem, highlightFollowsCurrentItem
*/
QDeclarativeComponent *QDeclarativeGridView::highlight() const
{
   Q_D(const QDeclarativeGridView);
   return d->highlightComponent;
}

void QDeclarativeGridView::setHighlight(QDeclarativeComponent *highlight)
{
   Q_D(QDeclarativeGridView);
   if (highlight != d->highlightComponent) {
      d->highlightComponent = highlight;
      d->updateCurrent(d->currentIndex);
      emit highlightChanged();
   }
}

/*!
  \qmlproperty bool GridView::highlightFollowsCurrentItem
  This property sets whether the highlight is managed by the view.

    If this property is true (the default value), the highlight is moved smoothly
    to follow the current item.  Otherwise, the
    highlight is not moved by the view, and any movement must be implemented
    by the highlight.

    Here is a highlight with its motion defined by a \l {SpringAnimation} item:

    \snippet doc/src/snippets/declarative/gridview/gridview.qml highlightFollowsCurrentItem
*/
bool QDeclarativeGridView::highlightFollowsCurrentItem() const
{
   Q_D(const QDeclarativeGridView);
   return d->autoHighlight;
}

void QDeclarativeGridView::setHighlightFollowsCurrentItem(bool autoHighlight)
{
   Q_D(QDeclarativeGridView);
   if (d->autoHighlight != autoHighlight) {
      d->autoHighlight = autoHighlight;
      if (autoHighlight) {
         d->updateHighlight();
      } else if (d->highlightXAnimator) {
         d->highlightXAnimator->stop();
         d->highlightYAnimator->stop();
      }
   }
}

/*!
    \qmlproperty int GridView::highlightMoveDuration
    This property holds the move animation duration of the highlight delegate.

    highlightFollowsCurrentItem must be true for this property
    to have effect.

    The default value for the duration is 150ms.

    \sa highlightFollowsCurrentItem
*/
int QDeclarativeGridView::highlightMoveDuration() const
{
   Q_D(const QDeclarativeGridView);
   return d->highlightMoveDuration;
}

void QDeclarativeGridView::setHighlightMoveDuration(int duration)
{
   Q_D(QDeclarativeGridView);
   if (d->highlightMoveDuration != duration) {
      d->highlightMoveDuration = duration;
      if (d->highlightYAnimator) {
         d->highlightXAnimator->userDuration = d->highlightMoveDuration;
         d->highlightYAnimator->userDuration = d->highlightMoveDuration;
      }
      emit highlightMoveDurationChanged();
   }
}


/*!
    \qmlproperty real GridView::preferredHighlightBegin
    \qmlproperty real GridView::preferredHighlightEnd
    \qmlproperty enumeration GridView::highlightRangeMode

    These properties define the preferred range of the highlight (for the current item)
    within the view. The \c preferredHighlightBegin value must be less than the
    \c preferredHighlightEnd value.

    These properties affect the position of the current item when the view is scrolled.
    For example, if the currently selected item should stay in the middle of the
    view when it is scrolled, set the \c preferredHighlightBegin and
    \c preferredHighlightEnd values to the top and bottom coordinates of where the middle
    item would be. If the \c currentItem is changed programmatically, the view will
    automatically scroll so that the current item is in the middle of the view.
    Furthermore, the behavior of the current item index will occur whether or not a
    highlight exists.

    Valid values for \c highlightRangeMode are:

    \list
    \o GridView.ApplyRange - the view attempts to maintain the highlight within the range.
       However, the highlight can move outside of the range at the ends of the view or due
       to mouse interaction.
    \o GridView.StrictlyEnforceRange - the highlight never moves outside of the range.
       The current item changes if a keyboard or mouse action would cause the highlight to move
       outside of the range.
    \o GridView.NoHighlightRange - this is the default value.
    \endlist
*/
qreal QDeclarativeGridView::preferredHighlightBegin() const
{
   Q_D(const QDeclarativeGridView);
   return d->highlightRangeStart;
}

void QDeclarativeGridView::setPreferredHighlightBegin(qreal start)
{
   Q_D(QDeclarativeGridView);
   d->highlightRangeStartValid = true;
   if (d->highlightRangeStart == start) {
      return;
   }
   d->highlightRangeStart = start;
   d->haveHighlightRange = d->highlightRange != NoHighlightRange && d->highlightRangeStart <= d->highlightRangeEnd;
   emit preferredHighlightBeginChanged();
}

void QDeclarativeGridView::resetPreferredHighlightBegin()
{
   Q_D(QDeclarativeGridView);
   d->highlightRangeStartValid = false;
   if (d->highlightRangeStart == 0) {
      return;
   }
   d->highlightRangeStart = 0;
   emit preferredHighlightBeginChanged();
}

qreal QDeclarativeGridView::preferredHighlightEnd() const
{
   Q_D(const QDeclarativeGridView);
   return d->highlightRangeEnd;
}

void QDeclarativeGridView::setPreferredHighlightEnd(qreal end)
{
   Q_D(QDeclarativeGridView);
   d->highlightRangeEndValid = true;
   if (d->highlightRangeEnd == end) {
      return;
   }
   d->highlightRangeEnd = end;
   d->haveHighlightRange = d->highlightRange != NoHighlightRange && d->highlightRangeStart <= d->highlightRangeEnd;
   emit preferredHighlightEndChanged();
}

void QDeclarativeGridView::resetPreferredHighlightEnd()
{
   Q_D(QDeclarativeGridView);
   d->highlightRangeEndValid = false;
   if (d->highlightRangeEnd == 0) {
      return;
   }
   d->highlightRangeEnd = 0;
   emit preferredHighlightEndChanged();
}

QDeclarativeGridView::HighlightRangeMode QDeclarativeGridView::highlightRangeMode() const
{
   Q_D(const QDeclarativeGridView);
   return d->highlightRange;
}

void QDeclarativeGridView::setHighlightRangeMode(HighlightRangeMode mode)
{
   Q_D(QDeclarativeGridView);
   if (d->highlightRange == mode) {
      return;
   }
   d->highlightRange = mode;
   d->haveHighlightRange = d->highlightRange != NoHighlightRange && d->highlightRangeStart <= d->highlightRangeEnd;
   emit highlightRangeModeChanged();
}

/*!
  \qmlproperty enumeration GridView::layoutDirection
  This property holds the layout direction of the grid.

    Possible values:

  \list
  \o Qt.LeftToRight (default) - Items will be laid out starting in the top, left corner. The flow is
  dependent on the \l GridView::flow property.
  \o Qt.RightToLeft - Items will be laid out starting in the top, right corner. The flow is dependent
  on the \l GridView::flow property.
  \endlist

  When using the attached property \l {LayoutMirroring::enabled} for locale layouts,
  the layout direction of the grid view will be mirrored. However, the actual property
  \c layoutDirection will remain unchanged. You can use the property
  \l {LayoutMirroring::enabled} to determine whether the direction has been mirrored.

  \sa {LayoutMirroring}{LayoutMirroring}
*/

Qt::LayoutDirection QDeclarativeGridView::layoutDirection() const
{
   Q_D(const QDeclarativeGridView);
   return d->layoutDirection;
}

void QDeclarativeGridView::setLayoutDirection(Qt::LayoutDirection layoutDirection)
{
   Q_D(QDeclarativeGridView);
   if (d->layoutDirection != layoutDirection) {
      d->layoutDirection = layoutDirection;
      d->regenerate();
      emit layoutDirectionChanged();
   }
}

Qt::LayoutDirection QDeclarativeGridView::effectiveLayoutDirection() const
{
   Q_D(const QDeclarativeGridView);
   if (d->effectiveLayoutMirror) {
      return d->layoutDirection == Qt::RightToLeft ? Qt::LeftToRight : Qt::RightToLeft;
   } else {
      return d->layoutDirection;
   }
}

/*!
  \qmlproperty enumeration GridView::flow
  This property holds the flow of the grid.

    Possible values:

    \list
    \o GridView.LeftToRight (default) - Items are laid out from left to right, and the view scrolls vertically
    \o GridView.TopToBottom - Items are laid out from top to bottom, and the view scrolls horizontally
    \endlist
*/
QDeclarativeGridView::Flow QDeclarativeGridView::flow() const
{
   Q_D(const QDeclarativeGridView);
   return d->flow;
}

void QDeclarativeGridView::setFlow(Flow flow)
{
   Q_D(QDeclarativeGridView);
   if (d->flow != flow) {
      d->flow = flow;
      if (d->flow == LeftToRight) {
         setContentWidth(-1);
         setFlickableDirection(QDeclarativeFlickable::VerticalFlick);
      } else {
         setContentHeight(-1);
         setFlickableDirection(QDeclarativeFlickable::HorizontalFlick);
      }
      setContentX(0);
      setContentY(0);
      d->regenerate();
      emit flowChanged();
   }
}

/*!
  \qmlproperty bool GridView::keyNavigationWraps
  This property holds whether the grid wraps key navigation

    If this is true, key navigation that would move the current item selection
    past one end of the view instead wraps around and moves the selection to
    the other end of the view.

    By default, key navigation is not wrapped.
*/
bool QDeclarativeGridView::isWrapEnabled() const
{
   Q_D(const QDeclarativeGridView);
   return d->wrap;
}

void QDeclarativeGridView::setWrapEnabled(bool wrap)
{
   Q_D(QDeclarativeGridView);
   if (d->wrap == wrap) {
      return;
   }
   d->wrap = wrap;
   emit keyNavigationWrapsChanged();
}

/*!
    \qmlproperty int GridView::cacheBuffer
    This property determines whether delegates are retained outside the
    visible area of the view.

    If non-zero the view will keep as many delegates
    instantiated as will fit within the buffer specified.  For example,
    if in a vertical view the delegate is 20 pixels high and \c cacheBuffer is
    set to 40, then up to 2 delegates above and 2 delegates below the visible
    area may be retained.

    Note that cacheBuffer is not a pixel buffer - it only maintains additional
    instantiated delegates.

    Setting this value can make scrolling the list smoother at the expense
    of additional memory usage.  It is not a substitute for creating efficient
    delegates; the fewer elements in a delegate, the faster a view may be
    scrolled.
*/
int QDeclarativeGridView::cacheBuffer() const
{
   Q_D(const QDeclarativeGridView);
   return d->buffer;
}

void QDeclarativeGridView::setCacheBuffer(int buffer)
{
   Q_D(QDeclarativeGridView);
   if (d->buffer != buffer) {
      d->buffer = buffer;
      if (isComponentComplete()) {
         refill();
      }
      emit cacheBufferChanged();
   }
}

/*!
  \qmlproperty int GridView::cellWidth
  \qmlproperty int GridView::cellHeight

  These properties holds the width and height of each cell in the grid.

  The default cell size is 100x100.
*/
int QDeclarativeGridView::cellWidth() const
{
   Q_D(const QDeclarativeGridView);
   return d->cellWidth;
}

void QDeclarativeGridView::setCellWidth(int cellWidth)
{
   Q_D(QDeclarativeGridView);
   if (cellWidth != d->cellWidth && cellWidth > 0) {
      d->cellWidth = qMax(1, cellWidth);
      d->updateGrid();
      emit cellWidthChanged();
      d->layout();
   }
}

int QDeclarativeGridView::cellHeight() const
{
   Q_D(const QDeclarativeGridView);
   return d->cellHeight;
}

void QDeclarativeGridView::setCellHeight(int cellHeight)
{
   Q_D(QDeclarativeGridView);
   if (cellHeight != d->cellHeight && cellHeight > 0) {
      d->cellHeight = qMax(1, cellHeight);
      d->updateGrid();
      emit cellHeightChanged();
      d->layout();
   }
}
/*!
    \qmlproperty enumeration GridView::snapMode

    This property determines how the view scrolling will settle following a drag or flick.
    The possible values are:

    \list
    \o GridView.NoSnap (default) - the view stops anywhere within the visible area.
    \o GridView.SnapToRow - the view settles with a row (or column for \c GridView.TopToBottom flow)
    aligned with the start of the view.
    \o GridView.SnapOneRow - the view will settle no more than one row (or column for \c GridView.TopToBottom flow)
    away from the first visible row at the time the mouse button is released.
    This mode is particularly useful for moving one page at a time.
    \endlist

*/
QDeclarativeGridView::SnapMode QDeclarativeGridView::snapMode() const
{
   Q_D(const QDeclarativeGridView);
   return d->snapMode;
}

void QDeclarativeGridView::setSnapMode(SnapMode mode)
{
   Q_D(QDeclarativeGridView);
   if (d->snapMode != mode) {
      d->snapMode = mode;
      emit snapModeChanged();
   }
}

/*!
    \qmlproperty Component GridView::footer
    This property holds the component to use as the footer.

    An instance of the footer component is created for each view.  The
    footer is positioned at the end of the view, after any items.

    \sa header
*/
QDeclarativeComponent *QDeclarativeGridView::footer() const
{
   Q_D(const QDeclarativeGridView);
   return d->footerComponent;
}

void QDeclarativeGridView::setFooter(QDeclarativeComponent *footer)
{
   Q_D(QDeclarativeGridView);
   if (d->footerComponent != footer) {
      if (d->footer) {
         if (scene()) {
            scene()->removeItem(d->footer->item);
         }
         d->footer->item->deleteLater();
         delete d->footer;
         d->footer = 0;
      }
      d->footerComponent = footer;
      if (isComponentComplete()) {
         d->updateFooter();
         d->updateGrid();
         d->fixupPosition();
      }
      emit footerChanged();
   }
}

/*!
    \qmlproperty Component GridView::header
    This property holds the component to use as the header.

    An instance of the header component is created for each view.  The
    header is positioned at the beginning of the view, before any items.

    \sa footer
*/
QDeclarativeComponent *QDeclarativeGridView::header() const
{
   Q_D(const QDeclarativeGridView);
   return d->headerComponent;
}

void QDeclarativeGridView::setHeader(QDeclarativeComponent *header)
{
   Q_D(QDeclarativeGridView);
   if (d->headerComponent != header) {
      if (d->header) {
         if (scene()) {
            scene()->removeItem(d->header->item);
         }
         d->header->item->deleteLater();
         delete d->header;
         d->header = 0;
      }
      d->headerComponent = header;
      if (isComponentComplete()) {
         d->updateHeader();
         d->updateFooter();
         d->updateGrid();
         d->fixupPosition();
      }
      emit headerChanged();
   }
}

void QDeclarativeGridView::setContentX(qreal pos)
{
   Q_D(QDeclarativeGridView);
   // Positioning the view manually should override any current movement state
   d->moveReason = QDeclarativeGridViewPrivate::Other;
   QDeclarativeFlickable::setContentX(pos);
}

void QDeclarativeGridView::setContentY(qreal pos)
{
   Q_D(QDeclarativeGridView);
   // Positioning the view manually should override any current movement state
   d->moveReason = QDeclarativeGridViewPrivate::Other;
   QDeclarativeFlickable::setContentY(pos);
}

bool QDeclarativeGridView::event(QEvent *event)
{
   Q_D(QDeclarativeGridView);
   if (event->type() == QEvent::User) {
      if (d->layoutScheduled) {
         d->layout();
      }
      return true;
   }

   return QDeclarativeFlickable::event(event);
}

void QDeclarativeGridView::viewportMoved()
{
   Q_D(QDeclarativeGridView);
   QDeclarativeFlickable::viewportMoved();
   if (!d->itemCount) {
      return;
   }
   d->lazyRelease = true;
   if (d->hData.flicking || d->vData.flicking) {
      if (yflick()) {
         if (d->vData.velocity > 0) {
            d->bufferMode = QDeclarativeGridViewPrivate::BufferBefore;
         } else if (d->vData.velocity < 0) {
            d->bufferMode = QDeclarativeGridViewPrivate::BufferAfter;
         }
      }

      if (xflick()) {
         if (d->hData.velocity > 0) {
            d->bufferMode = QDeclarativeGridViewPrivate::BufferBefore;
         } else if (d->hData.velocity < 0) {
            d->bufferMode = QDeclarativeGridViewPrivate::BufferAfter;
         }
      }
   }
   refill();
   if (d->hData.flicking || d->vData.flicking || d->hData.moving || d->vData.moving) {
      d->moveReason = QDeclarativeGridViewPrivate::Mouse;
   }
   if (d->moveReason != QDeclarativeGridViewPrivate::SetIndex) {
      if (d->haveHighlightRange && d->highlightRange == StrictlyEnforceRange && d->highlight) {
         // reposition highlight
         qreal pos = d->highlight->rowPos();
         qreal viewPos;
         qreal highlightStart;
         qreal highlightEnd;
         if (d->isRightToLeftTopToBottom()) {
            highlightStart = d->highlightRangeStartValid ? d->size() - d->highlightRangeEnd : d->highlightRangeStart;
            highlightEnd = d->highlightRangeEndValid ? d->size() - d->highlightRangeStart : d->highlightRangeEnd;
            viewPos = -d->position() - d->size();
         } else {
            highlightStart = d->highlightRangeStart;
            highlightEnd = d->highlightRangeEnd;
            viewPos = d->position();
         }
         if (pos > viewPos + highlightEnd - d->rowSize()) {
            pos = viewPos + highlightEnd - d->rowSize();
         }
         if (pos < viewPos + highlightStart) {
            pos = viewPos + highlightStart;
         }

         d->highlight->setPosition(d->highlight->colPos(), qRound(pos));

         // update current index
         int idx = d->snapIndex();
         if (idx >= 0 && idx != d->currentIndex) {
            d->updateCurrent(idx);
            if (d->currentItem && d->currentItem->colPos() != d->highlight->colPos() && d->autoHighlight) {
               if (d->flow == LeftToRight) {
                  d->highlightXAnimator->to = d->currentItem->item->x();
               } else {
                  d->highlightYAnimator->to = d->currentItem->item->y();
               }
            }
         }
      }
   }
}

qreal QDeclarativeGridView::minYExtent() const
{
   Q_D(const QDeclarativeGridView);
   if (d->flow == QDeclarativeGridView::TopToBottom) {
      return QDeclarativeFlickable::minYExtent();
   }
   qreal extent = -d->startPosition();
   if (d->header && d->visibleItems.count()) {
      extent += d->header->item->height();
   }
   if (d->haveHighlightRange && d->highlightRange == StrictlyEnforceRange) {
      extent += d->highlightRangeStart;
      extent = qMax(extent, -(d->rowPosAt(0) + d->rowSize() - d->highlightRangeEnd));
   }
   return extent;
}

qreal QDeclarativeGridView::maxYExtent() const
{
   Q_D(const QDeclarativeGridView);
   if (d->flow == QDeclarativeGridView::TopToBottom) {
      return QDeclarativeFlickable::maxYExtent();
   }
   qreal extent;
   if (!d->model || !d->model->count()) {
      extent = 0;
   } else if (d->haveHighlightRange && d->highlightRange == StrictlyEnforceRange) {
      extent = -(d->rowPosAt(d->model->count() - 1) - d->highlightRangeStart);
      if (d->highlightRangeEnd != d->highlightRangeStart) {
         extent = qMin(extent, -(d->endPosition() - d->highlightRangeEnd + 1));
      }
   } else {
      extent = -(d->endPosition() - height());
   }
   if (d->footer) {
      extent -= d->footer->item->height();
   }
   const qreal minY = minYExtent();
   if (extent > minY) {
      extent = minY;
   }
   return extent;
}

qreal QDeclarativeGridView::minXExtent() const
{
   Q_D(const QDeclarativeGridView);
   if (d->flow == QDeclarativeGridView::LeftToRight) {
      return QDeclarativeFlickable::minXExtent();
   }
   qreal extent = -d->startPosition();
   qreal highlightStart;
   qreal highlightEnd;
   qreal endPositionFirstItem = 0;
   if (d->isRightToLeftTopToBottom()) {
      if (d->model && d->model->count()) {
         endPositionFirstItem = d->rowPosAt(d->model->count() - 1);
      }
      highlightStart = d->highlightRangeStartValid
                       ? d->highlightRangeStart - (d->lastPosition() - endPositionFirstItem)
                       : d->size() - (d->lastPosition() - endPositionFirstItem);
      highlightEnd = d->highlightRangeEndValid ? d->highlightRangeEnd : d->size();
      if (d->footer && d->visibleItems.count()) {
         extent += d->footer->item->width();
      }
   } else {
      endPositionFirstItem = d->rowPosAt(0) + d->rowSize();
      highlightStart = d->highlightRangeStart;
      highlightEnd = d->highlightRangeEnd;
      if (d->header && d->visibleItems.count()) {
         extent += d->header->item->width();
      }
   }
   if (d->haveHighlightRange && d->highlightRange == StrictlyEnforceRange) {
      extent += d->isRightToLeftTopToBottom() ? -highlightStart : highlightStart;
      extent = qMax(extent, -(endPositionFirstItem - highlightEnd));
   }
   return extent;
}

qreal QDeclarativeGridView::maxXExtent() const
{
   Q_D(const QDeclarativeGridView);
   if (d->flow == QDeclarativeGridView::LeftToRight) {
      return QDeclarativeFlickable::maxXExtent();
   }
   qreal extent;
   qreal highlightStart;
   qreal highlightEnd;
   qreal lastItemPosition = 0;
   if (d->isRightToLeftTopToBottom()) {
      highlightStart = d->highlightRangeStartValid ? d->highlightRangeEnd : d->size();
      highlightEnd = d->highlightRangeEndValid ? d->highlightRangeStart : d->size();
      lastItemPosition = d->endPosition();
   } else {
      highlightStart = d->highlightRangeStart;
      highlightEnd = d->highlightRangeEnd;
      lastItemPosition = 0;
      if (d->model && d->model->count()) {
         lastItemPosition = d->rowPosAt(d->model->count() - 1);
      }
   }
   if (!d->model || !d->model->count()) {
      extent = 0;
   } else if (d->haveHighlightRange && d->highlightRange == StrictlyEnforceRange) {
      extent = -(lastItemPosition - highlightStart);
      if (highlightEnd != highlightStart)
         extent = d->isRightToLeftTopToBottom()
                  ? qMax(extent, -(d->endPosition() - highlightEnd + 1))
                  : qMin(extent, -(d->endPosition() - highlightEnd + 1));
   } else {
      extent = -(d->endPosition() - width());
   }
   if (d->isRightToLeftTopToBottom()) {
      if (d->header) {
         extent -= d->header->item->width();
      }
   } else {
      if (d->footer) {
         extent -= d->footer->item->width();
      }
   }

   const qreal minX = minXExtent();
   if (extent > minX) {
      extent = minX;
   }
   return extent;
}

void QDeclarativeGridView::keyPressEvent(QKeyEvent *event)
{
   Q_D(QDeclarativeGridView);
   keyPressPreHandler(event);
   if (event->isAccepted()) {
      return;
   }
   if (d->model && d->model->count() && d->interactive) {
      d->moveReason = QDeclarativeGridViewPrivate::SetIndex;
      int oldCurrent = currentIndex();
      switch (event->key()) {
         case Qt::Key_Up:
            moveCurrentIndexUp();
            break;
         case Qt::Key_Down:
            moveCurrentIndexDown();
            break;
         case Qt::Key_Left:
            moveCurrentIndexLeft();
            break;
         case Qt::Key_Right:
            moveCurrentIndexRight();
            break;
         default:
            break;
      }
      if (oldCurrent != currentIndex()) {
         event->accept();
         return;
      }
   }
   d->moveReason = QDeclarativeGridViewPrivate::Other;
   event->ignore();
   QDeclarativeFlickable::keyPressEvent(event);
}

/*!
    \qmlmethod GridView::moveCurrentIndexUp()

    Move the currentIndex up one item in the view.
    The current index will wrap if keyNavigationWraps is true and it
    is currently at the end. This method has no effect if the \l count is zero.

    \bold Note: methods should only be called after the Component has completed.
*/
void QDeclarativeGridView::moveCurrentIndexUp()
{
   Q_D(QDeclarativeGridView);
   const int count = d->model ? d->model->count() : 0;
   if (!count) {
      return;
   }
   if (d->flow == QDeclarativeGridView::LeftToRight) {
      if (currentIndex() >= d->columns || d->wrap) {
         int index = currentIndex() - d->columns;
         setCurrentIndex((index >= 0 && index < count) ? index : count - 1);
      }
   } else {
      if (currentIndex() > 0 || d->wrap) {
         int index = currentIndex() - 1;
         setCurrentIndex((index >= 0 && index < count) ? index : count - 1);
      }
   }
}

/*!
    \qmlmethod GridView::moveCurrentIndexDown()

    Move the currentIndex down one item in the view.
    The current index will wrap if keyNavigationWraps is true and it
    is currently at the end. This method has no effect if the \l count is zero.

    \bold Note: methods should only be called after the Component has completed.
*/
void QDeclarativeGridView::moveCurrentIndexDown()
{
   Q_D(QDeclarativeGridView);
   const int count = d->model ? d->model->count() : 0;
   if (!count) {
      return;
   }
   if (d->flow == QDeclarativeGridView::LeftToRight) {
      if (currentIndex() < count - d->columns || d->wrap) {
         int index = currentIndex() + d->columns;
         setCurrentIndex((index >= 0 && index < count) ? index : 0);
      }
   } else {
      if (currentIndex() < count - 1 || d->wrap) {
         int index = currentIndex() + 1;
         setCurrentIndex((index >= 0 && index < count) ? index : 0);
      }
   }
}

/*!
    \qmlmethod GridView::moveCurrentIndexLeft()

    Move the currentIndex left one item in the view.
    The current index will wrap if keyNavigationWraps is true and it
    is currently at the end. This method has no effect if the \l count is zero.

    \bold Note: methods should only be called after the Component has completed.
*/
void QDeclarativeGridView::moveCurrentIndexLeft()
{
   Q_D(QDeclarativeGridView);
   const int count = d->model ? d->model->count() : 0;
   if (!count) {
      return;
   }

   if (effectiveLayoutDirection() == Qt::LeftToRight) {
      if (d->flow == QDeclarativeGridView::LeftToRight) {
         if (currentIndex() > 0 || d->wrap) {
            int index = currentIndex() - 1;
            setCurrentIndex((index >= 0 && index < count) ? index : count - 1);
         }
      } else {
         if (currentIndex() >= d->columns || d->wrap) {
            int index = currentIndex() - d->columns;
            setCurrentIndex((index >= 0 && index < count) ? index : count - 1);
         }
      }
   } else {
      if (d->flow == QDeclarativeGridView::LeftToRight) {
         if (currentIndex() < count - 1 || d->wrap) {
            int index = currentIndex() + 1;
            setCurrentIndex((index >= 0 && index < count) ? index : 0);
         }
      } else {
         if (currentIndex() < count - d->columns || d->wrap) {
            int index = currentIndex() + d->columns;
            setCurrentIndex((index >= 0 && index < count) ? index : 0);
         }
      }
   }
}

/*!
    \qmlmethod GridView::moveCurrentIndexRight()

    Move the currentIndex right one item in the view.
    The current index will wrap if keyNavigationWraps is true and it
    is currently at the end. This method has no effect if the \l count is zero.

    \bold Note: methods should only be called after the Component has completed.
*/
void QDeclarativeGridView::moveCurrentIndexRight()
{
   Q_D(QDeclarativeGridView);
   const int count = d->model ? d->model->count() : 0;
   if (!count) {
      return;
   }

   if (effectiveLayoutDirection() == Qt::LeftToRight) {
      if (d->flow == QDeclarativeGridView::LeftToRight) {
         if (currentIndex() < count - 1 || d->wrap) {
            int index = currentIndex() + 1;
            setCurrentIndex((index >= 0 && index < count) ? index : 0);
         }
      } else {
         if (currentIndex() < count - d->columns || d->wrap) {
            int index = currentIndex() + d->columns;
            setCurrentIndex((index >= 0 && index < count) ? index : 0);
         }
      }
   } else {
      if (d->flow == QDeclarativeGridView::LeftToRight) {
         if (currentIndex() > 0 || d->wrap) {
            int index = currentIndex() - 1;
            setCurrentIndex((index >= 0 && index < count) ? index : count - 1);
         }
      } else {
         if (currentIndex() >= d->columns || d->wrap) {
            int index = currentIndex() - d->columns;
            setCurrentIndex((index >= 0 && index < count) ? index : count - 1);
         }
      }
   }
}

void QDeclarativeGridViewPrivate::positionViewAtIndex(int index, int mode)
{
   Q_Q(QDeclarativeGridView);
   if (!isValid()) {
      return;
   }
   if (mode < QDeclarativeGridView::Beginning || mode > QDeclarativeGridView::Contain) {
      return;
   }

   int idx = qMax(qMin(index, model->count() - 1), 0);

   if (layoutScheduled) {
      layout();
   }
   qreal pos = isRightToLeftTopToBottom() ? -position() - size() : position();
   FxGridItem *item = visibleItem(idx);
   qreal maxExtent;
   if (flow == QDeclarativeGridView::LeftToRight) {
      maxExtent = -q->maxYExtent();
   } else {
      maxExtent = isRightToLeftTopToBottom() ? q->minXExtent() - size() : -q->maxXExtent();
   }

   if (!item) {
      int itemPos = rowPosAt(idx);
      // save the currently visible items in case any of them end up visible again
      QList<FxGridItem *> oldVisible = visibleItems;
      visibleItems.clear();
      visibleIndex = idx - idx % columns;
      if (flow == QDeclarativeGridView::LeftToRight) {
         maxExtent = -q->maxYExtent();
      } else {
         maxExtent = isRightToLeftTopToBottom() ? q->minXExtent() - size() : -q->maxXExtent();
      }
      setPosition(qMin(qreal(itemPos), maxExtent));
      // now release the reference to all the old visible items.
      for (int i = 0; i < oldVisible.count(); ++i) {
         releaseItem(oldVisible.at(i));
      }
      item = visibleItem(idx);
   }
   if (item) {
      qreal itemPos = item->rowPos();
      switch (mode) {
         case QDeclarativeGridView::Beginning:
            pos = itemPos;
            if (index < 0 && header) {
               pos -= flow == QDeclarativeGridView::LeftToRight
                      ? header->item->height()
                      : header->item->width();
            }
            break;
         case QDeclarativeGridView::Center:
            pos = itemPos - (size() - rowSize()) / 2;
            break;
         case QDeclarativeGridView::End:
            pos = itemPos - size() + rowSize();
            if (index >= model->count() && footer) {
               pos += flow == QDeclarativeGridView::LeftToRight
                      ? footer->item->height()
                      : footer->item->width();
            }
            break;
         case QDeclarativeGridView::Visible:
            if (itemPos > pos + size()) {
               pos = itemPos - size() + rowSize();
            } else if (item->endRowPos() < pos) {
               pos = itemPos;
            }
            break;
         case QDeclarativeGridView::Contain:
            if (item->endRowPos() > pos + size()) {
               pos = itemPos - size() + rowSize();
            }
            if (itemPos < pos) {
               pos = itemPos;
            }
      }

      pos = qMin(pos, maxExtent);
      qreal minExtent;
      if (flow == QDeclarativeGridView::LeftToRight) {
         minExtent = -q->minYExtent();
      } else {
         minExtent = isRightToLeftTopToBottom() ? q->maxXExtent() - size() : -q->minXExtent();
      }
      pos = qMax(pos, minExtent);
      moveReason = QDeclarativeGridViewPrivate::Other;
      q->cancelFlick();
      setPosition(pos);
   }
   fixupPosition();
}

/*!
    \qmlmethod GridView::positionViewAtIndex(int index, PositionMode mode)

    Positions the view such that the \a index is at the position specified by
    \a mode:

    \list
    \o GridView.Beginning - position item at the top (or left for \c GridView.TopToBottom flow) of the view.
    \o GridView.Center - position item in the center of the view.
    \o GridView.End - position item at bottom (or right for horizontal orientation) of the view.
    \o GridView.Visible - if any part of the item is visible then take no action, otherwise
    bring the item into view.
    \o GridView.Contain - ensure the entire item is visible.  If the item is larger than
    the view the item is positioned at the top (or left for \c GridView.TopToBottom flow) of the view.
    \endlist

    If positioning the view at the index would cause empty space to be displayed at
    the beginning or end of the view, the view will be positioned at the boundary.

    It is not recommended to use \l {Flickable::}{contentX} or \l {Flickable::}{contentY} to position the view
    at a particular index.  This is unreliable since removing items from the start
    of the view does not cause all other items to be repositioned.
    The correct way to bring an item into view is with \c positionViewAtIndex.

    \bold Note: methods should only be called after the Component has completed.  To position
    the view at startup, this method should be called by Component.onCompleted.  For
    example, to position the view at the end:

    \code
    Component.onCompleted: positionViewAtIndex(count - 1, GridView.Beginning)
    \endcode
*/
void QDeclarativeGridView::positionViewAtIndex(int index, int mode)
{
   Q_D(QDeclarativeGridView);
   if (!d->isValid() || index < 0 || index >= d->model->count()) {
      return;
   }
   d->positionViewAtIndex(index, mode);
}

/*!
    \qmlmethod GridView::positionViewAtBeginning()
    \qmlmethod GridView::positionViewAtEnd()
    \since QtQuick 1.1

    Positions the view at the beginning or end, taking into account any header or footer.

    It is not recommended to use \l {Flickable::}{contentX} or \l {Flickable::}{contentY} to position the view
    at a particular index.  This is unreliable since removing items from the start
    of the list does not cause all other items to be repositioned, and because
    the actual start of the view can vary based on the size of the delegates.

    \bold Note: methods should only be called after the Component has completed.  To position
    the view at startup, this method should be called by Component.onCompleted.  For
    example, to position the view at the end on startup:

    \code
    Component.onCompleted: positionViewAtEnd()
    \endcode
*/
void QDeclarativeGridView::positionViewAtBeginning()
{
   Q_D(QDeclarativeGridView);
   if (!d->isValid()) {
      return;
   }
   d->positionViewAtIndex(-1, Beginning);
}

void QDeclarativeGridView::positionViewAtEnd()
{
   Q_D(QDeclarativeGridView);
   if (!d->isValid()) {
      return;
   }
   d->positionViewAtIndex(d->model->count(), End);
}

/*!
    \qmlmethod int GridView::indexAt(int x, int y)

    Returns the index of the visible item containing the point \a x, \a y in content
    coordinates.  If there is no item at the point specified, or the item is
    not visible -1 is returned.

    If the item is outside the visible area, -1 is returned, regardless of
    whether an item will exist at that point when scrolled into view.

    \bold Note: methods should only be called after the Component has completed.
*/
int QDeclarativeGridView::indexAt(qreal x, qreal y) const
{
   Q_D(const QDeclarativeGridView);
   for (int i = 0; i < d->visibleItems.count(); ++i) {
      const FxGridItem *listItem = d->visibleItems.at(i);
      if (listItem->contains(x, y)) {
         return listItem->index;
      }
   }

   return -1;
}

void QDeclarativeGridView::componentComplete()
{
   Q_D(QDeclarativeGridView);
   QDeclarativeFlickable::componentComplete();
   d->updateHeader();
   d->updateFooter();
   d->updateGrid();
   if (d->isValid()) {
      refill();
      d->moveReason = QDeclarativeGridViewPrivate::SetIndex;
      if (d->currentIndex < 0 && !d->currentIndexCleared) {
         d->updateCurrent(0);
      } else {
         d->updateCurrent(d->currentIndex);
      }
      if (d->highlight && d->currentItem) {
         if (d->autoHighlight) {
            d->highlight->setPosition(d->currentItem->colPos(), d->currentItem->rowPos());
         }
         d->updateTrackedItem();
      }
      d->moveReason = QDeclarativeGridViewPrivate::Other;
      d->fixupPosition();
   }
}

void QDeclarativeGridView::trackedPositionChanged()
{
   Q_D(QDeclarativeGridView);
   if (!d->trackedItem || !d->currentItem) {
      return;
   }
   if (d->moveReason == QDeclarativeGridViewPrivate::SetIndex) {
      const qreal trackedPos = d->trackedItem->rowPos();
      qreal viewPos;
      qreal highlightStart;
      qreal highlightEnd;
      if (d->isRightToLeftTopToBottom()) {
         viewPos = -d->position() - d->size();
         highlightStart = d->highlightRangeStartValid ? d->size() - d->highlightRangeEnd : d->highlightRangeStart;
         highlightEnd = d->highlightRangeEndValid ? d->size() - d->highlightRangeStart : d->highlightRangeEnd;
      } else {
         viewPos = d->position();
         highlightStart = d->highlightRangeStart;
         highlightEnd = d->highlightRangeEnd;
      }
      qreal pos = viewPos;
      if (d->haveHighlightRange) {
         if (d->highlightRange == StrictlyEnforceRange) {
            if (trackedPos > pos + highlightEnd - d->rowSize()) {
               pos = trackedPos - highlightEnd + d->rowSize();
            }
            if (trackedPos < pos + highlightStart) {
               pos = trackedPos - highlightStart;
            }
         } else {
            if (trackedPos < d->startPosition() + highlightStart) {
               pos = d->startPosition();
            } else if (d->trackedItem->endRowPos() > d->endPosition() - d->size() + highlightEnd) {
               pos = d->endPosition() - d->size() + 1;
               if (pos < d->startPosition()) {
                  pos = d->startPosition();
               }
            } else {
               if (trackedPos < viewPos + highlightStart) {
                  pos = trackedPos - highlightStart;
               } else if (trackedPos > viewPos + highlightEnd - d->rowSize()) {
                  pos = trackedPos - highlightEnd + d->rowSize();
               }
            }
         }
      } else {
         if (trackedPos < viewPos && d->currentItem->rowPos() < viewPos) {
            pos = qMax(trackedPos, d->currentItem->rowPos());
         } else if (d->trackedItem->endRowPos() >= viewPos + d->size()
                    && d->currentItem->endRowPos() >= viewPos + d->size()) {
            if (d->trackedItem->endRowPos() <= d->currentItem->endRowPos()) {
               pos = d->trackedItem->endRowPos() - d->size() + 1;
               if (d->rowSize() > d->size()) {
                  pos = trackedPos;
               }
            } else {
               pos = d->currentItem->endRowPos() - d->size() + 1;
               if (d->rowSize() > d->size()) {
                  pos = d->currentItem->rowPos();
               }
            }
         }
      }
      if (viewPos != pos) {
         cancelFlick();
         d->calcVelocity = true;
         d->setPosition(pos);
         d->calcVelocity = false;
      }
   }
}

void QDeclarativeGridView::itemsInserted(int modelIndex, int count)
{
   Q_D(QDeclarativeGridView);
   if (!isComponentComplete()) {
      return;
   }

   int index = d->visibleItems.count() ? d->mapFromModel(modelIndex) : 0;
   if (index < 0) {
      int i = d->visibleItems.count() - 1;
      while (i > 0 && d->visibleItems.at(i)->index == -1) {
         --i;
      }
      if (d->visibleItems.at(i)->index + 1 == modelIndex) {
         // Special case of appending an item to the model.
         index = d->visibleIndex + d->visibleItems.count();
      } else {
         if (modelIndex <= d->visibleIndex) {
            // Insert before visible items
            d->visibleIndex += count;
            for (int i = 0; i < d->visibleItems.count(); ++i) {
               FxGridItem *listItem = d->visibleItems.at(i);
               if (listItem->index != -1 && listItem->index >= modelIndex) {
                  listItem->index += count;
               }
            }
         }
         if (d->currentIndex >= modelIndex) {
            // adjust current item index
            d->currentIndex += count;
            if (d->currentItem) {
               d->currentItem->index = d->currentIndex;
            }
            emit currentIndexChanged();
         }
         d->scheduleLayout();
         d->itemCount += count;
         emit countChanged();
         return;
      }
   }

   int insertCount = count;
   if (index < d->visibleIndex && d->visibleItems.count()) {
      insertCount -= d->visibleIndex - index;
      index = d->visibleIndex;
      modelIndex = d->visibleIndex;
   }

   qreal tempPos = d->isRightToLeftTopToBottom() ? -d->position() - d->size() + d->width() + 1 : d->position();
   int to = d->buffer + tempPos + d->size() - 1;
   int colPos = 0;
   int rowPos = 0;
   if (d->visibleItems.count()) {
      index -= d->visibleIndex;
      if (index < d->visibleItems.count()) {
         colPos = d->visibleItems.at(index)->colPos();
         rowPos = d->visibleItems.at(index)->rowPos();
      } else {
         // appending items to visible list
         colPos = d->visibleItems.at(index - 1)->colPos() + d->colSize();
         rowPos = d->visibleItems.at(index - 1)->rowPos();
         if (colPos > d->colSize() * (d->columns - 1)) {
            colPos = 0;
            rowPos += d->rowSize();
         }
      }
   } else if (d->itemCount == 0 && d->header) {
      rowPos = d->headerSize();
   }

   // Update the indexes of the following visible items.
   for (int i = 0; i < d->visibleItems.count(); ++i) {
      FxGridItem *listItem = d->visibleItems.at(i);
      if (listItem->index != -1 && listItem->index >= modelIndex) {
         listItem->index += count;
      }
   }

   bool addedVisible = false;
   QList<FxGridItem *> added;
   int i = 0;
   while (i < insertCount &&
          rowPos <= to + d->rowSize() * (d->columns - (colPos / d->colSize())) / qreal(d->columns + 1)) {
      if (!addedVisible) {
         d->scheduleLayout();
         addedVisible = true;
      }
      FxGridItem *item = d->createItem(modelIndex + i);
      if (!item) {
         // broken or no delegate
         d->clear();
         return;
      }
      d->visibleItems.insert(index, item);
      item->setPosition(colPos, rowPos);
      added.append(item);
      colPos += d->colSize();
      if (colPos > d->colSize() * (d->columns - 1)) {
         colPos = 0;
         rowPos += d->rowSize();
      }
      ++index;
      ++i;
   }
   if (i < insertCount) {
      // We didn't insert all our new items, which means anything
      // beyond the current index is not visible - remove it.
      while (d->visibleItems.count() > index) {
         d->releaseItem(d->visibleItems.takeLast());
      }
   }

   // update visibleIndex
   d->visibleIndex = 0;
   for (QList<FxGridItem *>::Iterator it = d->visibleItems.begin(); it != d->visibleItems.end(); ++it) {
      if ((*it)->index != -1) {
         d->visibleIndex = (*it)->index;
         break;
      }
   }

   if (d->itemCount && d->currentIndex >= modelIndex) {
      // adjust current item index
      d->currentIndex += count;
      if (d->currentItem) {
         d->currentItem->index = d->currentIndex;
         d->currentItem->setPosition(d->colPosAt(d->currentIndex), d->rowPosAt(d->currentIndex));
      }
      emit currentIndexChanged();
   } else if (d->itemCount == 0 && (!d->currentIndex || (d->currentIndex < 0 && !d->currentIndexCleared))) {
      setCurrentIndex(0);
   }

   // everything is in order now - emit add() signal
   for (int j = 0; j < added.count(); ++j) {
      added.at(j)->attached->emitAdd();
   }

   d->itemCount += count;
   emit countChanged();
}

void QDeclarativeGridView::itemsRemoved(int modelIndex, int count)
{
   Q_D(QDeclarativeGridView);
   if (!isComponentComplete()) {
      return;
   }

   d->itemCount -= count;
   bool currentRemoved = d->currentIndex >= modelIndex && d->currentIndex < modelIndex + count;
   bool removedVisible = false;
   // Remove the items from the visible list, skipping anything already marked for removal
   QList<FxGridItem *>::Iterator it = d->visibleItems.begin();
   while (it != d->visibleItems.end()) {
      FxGridItem *item = *it;
      if (item->index == -1 || item->index < modelIndex) {
         // already removed, or before removed items
         if (item->index < modelIndex && !removedVisible) {
            d->scheduleLayout();
            removedVisible = true;
         }
         ++it;
      } else if (item->index >= modelIndex + count) {
         // after removed items
         item->index -= count;
         ++it;
      } else {
         // removed item
         if (!removedVisible) {
            d->scheduleLayout();
            removedVisible = true;
         }
         item->attached->emitRemove();
         if (item->attached->delayRemove()) {
            item->index = -1;
            connect(item->attached, SIGNAL(delayRemoveChanged()), this, SLOT(destroyRemoved()), Qt::QueuedConnection);
            ++it;
         } else {
            it = d->visibleItems.erase(it);
            d->releaseItem(item);
         }
      }
   }

   // If we removed items before visible items a layout may be
   // required to ensure item 0 is in the first column.
   if (!removedVisible && modelIndex < d->visibleIndex) {
      d->scheduleLayout();
   }

   // fix current
   if (d->currentIndex >= modelIndex + count) {
      d->currentIndex -= count;
      if (d->currentItem) {
         d->currentItem->index -= count;
      }
      emit currentIndexChanged();
   } else if (currentRemoved) {
      // current item has been removed.
      d->releaseItem(d->currentItem);
      d->currentItem = 0;
      d->currentIndex = -1;
      if (d->itemCount) {
         d->updateCurrent(qMin(modelIndex, d->itemCount - 1));
      } else {
         emit currentIndexChanged();
      }
   }

   // update visibleIndex
   d->visibleIndex = 0;
   for (it = d->visibleItems.begin(); it != d->visibleItems.end(); ++it) {
      if ((*it)->index != -1) {
         d->visibleIndex = (*it)->index;
         break;
      }
   }

   if (removedVisible && d->visibleItems.isEmpty()) {
      d->timeline.clear();
      if (d->itemCount == 0) {
         d->setPosition(0);
         d->updateHeader();
         d->updateFooter();
         update();
      }
   }

   emit countChanged();
}

void QDeclarativeGridView::destroyRemoved()
{
   Q_D(QDeclarativeGridView);
   for (QList<FxGridItem *>::Iterator it = d->visibleItems.begin();
         it != d->visibleItems.end();) {
      FxGridItem *listItem = *it;
      if (listItem->index == -1 && listItem->attached->delayRemove() == false) {
         d->releaseItem(listItem);
         it = d->visibleItems.erase(it);
      } else {
         ++it;
      }
   }

   // Correct the positioning of the items
   d->layout();
}

void QDeclarativeGridView::itemsMoved(int from, int to, int count)
{
   Q_D(QDeclarativeGridView);
   if (!isComponentComplete()) {
      return;
   }
   QHash<int, FxGridItem *> moved;

   FxGridItem *firstItem = d->firstVisibleItem();

   QList<FxGridItem *>::Iterator it = d->visibleItems.begin();
   while (it != d->visibleItems.end()) {
      FxGridItem *item = *it;
      if (item->index >= from && item->index < from + count) {
         // take the items that are moving
         item->index += (to - from);
         moved.insert(item->index, item);
         it = d->visibleItems.erase(it);
      } else {
         if (item->index > from && item->index != -1) {
            // move everything after the moved items.
            item->index -= count;
            if (item->index < d->visibleIndex) {
               d->visibleIndex = item->index;
            }
         }
         ++it;
      }
   }

   int remaining = count;
   int endIndex = d->visibleIndex;
   it = d->visibleItems.begin();
   while (it != d->visibleItems.end()) {
      FxGridItem *item = *it;
      if (remaining && item->index >= to && item->index < to + count) {
         // place items in the target position, reusing any existing items
         FxGridItem *movedItem = moved.take(item->index);
         if (!movedItem) {
            movedItem = d->createItem(item->index);
         }
         if (!movedItem) {
            // broken or no delegate
            d->clear();
            return;
         }
         it = d->visibleItems.insert(it, movedItem);
         if (it == d->visibleItems.begin() && firstItem) {
            movedItem->setPosition(firstItem->colPos(), firstItem->rowPos());
         }
         ++it;
         --remaining;
      } else {
         if (item->index != -1) {
            if (item->index >= to) {
               // update everything after the moved items.
               item->index += count;
            }
            endIndex = item->index;
         }
         ++it;
      }
   }

   // If we have moved items to the end of the visible items
   // then add any existing moved items that we have
   while (FxGridItem *item = moved.take(endIndex + 1)) {
      d->visibleItems.append(item);
      ++endIndex;
   }

   // update visibleIndex
   for (it = d->visibleItems.begin(); it != d->visibleItems.end(); ++it) {
      if ((*it)->index != -1) {
         d->visibleIndex = (*it)->index;
         break;
      }
   }

   // Fix current index
   if (d->currentIndex >= 0 && d->currentItem) {
      int oldCurrent = d->currentIndex;
      d->currentIndex = d->model->indexOf(d->currentItem->item, this);
      if (oldCurrent != d->currentIndex) {
         d->currentItem->index = d->currentIndex;
         emit currentIndexChanged();
      }
   }

   // Whatever moved items remain are no longer visible items.
   while (moved.count()) {
      int idx = moved.begin().key();
      FxGridItem *item = moved.take(idx);
      if (d->currentItem && item->item == d->currentItem->item) {
         item->setPosition(d->colPosAt(idx), d->rowPosAt(idx));
      }
      d->releaseItem(item);
   }

   d->layout();
}

void QDeclarativeGridView::modelReset()
{
   Q_D(QDeclarativeGridView);
   d->clear();
   refill();
   d->moveReason = QDeclarativeGridViewPrivate::SetIndex;
   d->updateCurrent(d->currentIndex);
   if (d->highlight && d->currentItem) {
      if (d->autoHighlight) {
         d->highlight->setPosition(d->currentItem->colPos(), d->currentItem->rowPos());
      }
      d->updateTrackedItem();
   }
   d->moveReason = QDeclarativeGridViewPrivate::Other;

   emit countChanged();
}

void QDeclarativeGridView::createdItem(int index, QDeclarativeItem *item)
{
   Q_D(QDeclarativeGridView);
   if (d->requestedIndex != index) {
      item->setParentItem(this);
      d->unrequestedItems.insert(item, index);
      if (d->flow == QDeclarativeGridView::LeftToRight) {
         item->setPos(QPointF(d->colPosAt(index), d->rowPosAt(index)));
      } else {
         item->setPos(QPointF(d->rowPosAt(index), d->colPosAt(index)));
      }
   }
}

void QDeclarativeGridView::destroyingItem(QDeclarativeItem *item)
{
   Q_D(QDeclarativeGridView);
   d->unrequestedItems.remove(item);
}

void QDeclarativeGridView::animStopped()
{
   Q_D(QDeclarativeGridView);
   d->bufferMode = QDeclarativeGridViewPrivate::NoBuffer;
   if (d->haveHighlightRange && d->highlightRange == QDeclarativeGridView::StrictlyEnforceRange) {
      d->updateHighlight();
   }
}

void QDeclarativeGridView::refill()
{
   Q_D(QDeclarativeGridView);
   if (d->isRightToLeftTopToBottom()) {
      d->refill(-d->position() - d->size() + 1, -d->position());
   } else {
      d->refill(d->position(), d->position() + d->size() - 1);
   }
}


QDeclarativeGridViewAttached *QDeclarativeGridView::qmlAttachedProperties(QObject *obj)
{
   return new QDeclarativeGridViewAttached(obj);
}

QT_END_NAMESPACE
