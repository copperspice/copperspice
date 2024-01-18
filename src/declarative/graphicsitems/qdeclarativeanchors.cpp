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

#include "private/qdeclarativeanchors_p_p.h"

#include "qdeclarativeitem.h"
#include "private/qdeclarativeitem_p.h"

#include <qdeclarativeinfo.h>

#include <QDebug>

QT_BEGIN_NAMESPACE

//TODO: should we cache relationships, so we don't have to check each time (parent-child or sibling)?
//TODO: support non-parent, non-sibling (need to find lowest common ancestor)

static qreal hcenter(QGraphicsItem *i)
{
   QGraphicsItemPrivate *item = QGraphicsItemPrivate::get(i);

   qreal width = item->width();
   int iw = width;
   if (iw % 2) {
      return (width + 1) / 2;
   } else {
      return width / 2;
   }
}

static qreal vcenter(QGraphicsItem *i)
{
   QGraphicsItemPrivate *item = QGraphicsItemPrivate::get(i);

   qreal height = item->height();
   int ih = height;
   if (ih % 2) {
      return (height + 1) / 2;
   } else {
      return height / 2;
   }
}

//### const item?
//local position
static qreal position(QGraphicsObject *item, QDeclarativeAnchorLine::AnchorLine anchorLine)
{
   qreal ret = 0.0;
   QGraphicsItemPrivate *d = QGraphicsItemPrivate::get(item);
   switch (anchorLine) {
      case QDeclarativeAnchorLine::Left:
         ret = item->x();
         break;
      case QDeclarativeAnchorLine::Right:
         ret = item->x() + d->width();
         break;
      case QDeclarativeAnchorLine::Top:
         ret = item->y();
         break;
      case QDeclarativeAnchorLine::Bottom:
         ret = item->y() + d->height();
         break;
      case QDeclarativeAnchorLine::HCenter:
         ret = item->x() + hcenter(item);
         break;
      case QDeclarativeAnchorLine::VCenter:
         ret = item->y() + vcenter(item);
         break;
      case QDeclarativeAnchorLine::Baseline:
         if (d->isDeclarativeItem) {
            ret = item->y() + static_cast<QDeclarativeItem *>(item)->baselineOffset();
         }
         break;
      default:
         break;
   }

   return ret;
}

//position when origin is 0,0
static qreal adjustedPosition(QGraphicsObject *item, QDeclarativeAnchorLine::AnchorLine anchorLine)
{
   qreal ret = 0.0;
   QGraphicsItemPrivate *d = QGraphicsItemPrivate::get(item);
   switch (anchorLine) {
      case QDeclarativeAnchorLine::Left:
         ret = 0.0;
         break;
      case QDeclarativeAnchorLine::Right:
         ret = d->width();
         break;
      case QDeclarativeAnchorLine::Top:
         ret = 0.0;
         break;
      case QDeclarativeAnchorLine::Bottom:
         ret = d->height();
         break;
      case QDeclarativeAnchorLine::HCenter:
         ret = hcenter(item);
         break;
      case QDeclarativeAnchorLine::VCenter:
         ret = vcenter(item);
         break;
      case QDeclarativeAnchorLine::Baseline:
         if (d->isDeclarativeItem) {
            ret = static_cast<QDeclarativeItem *>(item)->baselineOffset();
         }
         break;
      default:
         break;
   }

   return ret;
}

QDeclarativeAnchors::QDeclarativeAnchors(QObject *parent)
   : QObject(*new QDeclarativeAnchorsPrivate(0), parent)
{
   qFatal("QDeclarativeAnchors::QDeclarativeAnchors(QObject*) called");
}

QDeclarativeAnchors::QDeclarativeAnchors(QGraphicsObject *item, QObject *parent)
   : QObject(*new QDeclarativeAnchorsPrivate(item), parent)
{
}

QDeclarativeAnchors::~QDeclarativeAnchors()
{
   Q_D(QDeclarativeAnchors);
   d->remDepend(d->fill);
   d->remDepend(d->centerIn);
   d->remDepend(d->left.item);
   d->remDepend(d->right.item);
   d->remDepend(d->top.item);
   d->remDepend(d->bottom.item);
   d->remDepend(d->vCenter.item);
   d->remDepend(d->hCenter.item);
   d->remDepend(d->baseline.item);
}

void QDeclarativeAnchorsPrivate::fillChanged()
{
   Q_Q(QDeclarativeAnchors);
   if (!fill || !isItemComplete()) {
      return;
   }

   if (updatingFill < 2) {
      ++updatingFill;

      qreal horizontalMargin = q->mirrored() ? rightMargin : leftMargin;

      if (fill == item->parentItem()) {                         //child-parent
         setItemPos(QPointF(horizontalMargin, topMargin));
      } else if (fill->parentItem() == item->parentItem()) {   //siblings
         setItemPos(QPointF(fill->x() + horizontalMargin, fill->y() + topMargin));
      }
      QGraphicsItemPrivate *fillPrivate = QGraphicsItemPrivate::get(fill);
      setItemSize(QSizeF(fillPrivate->width() - leftMargin - rightMargin, fillPrivate->height() - topMargin - bottomMargin));

      --updatingFill;
   } else {
      // ### Make this certain :)
      qmlInfo(item) << QDeclarativeAnchors::tr("Possible anchor loop detected on fill.");
   }

}

void QDeclarativeAnchorsPrivate::centerInChanged()
{
   Q_Q(QDeclarativeAnchors);
   if (!centerIn || fill || !isItemComplete()) {
      return;
   }

   if (updatingCenterIn < 2) {
      ++updatingCenterIn;

      qreal effectiveHCenterOffset = q->mirrored() ? -hCenterOffset : hCenterOffset;
      if (centerIn == item->parentItem()) {
         QPointF p(hcenter(item->parentItem()) - hcenter(item) + effectiveHCenterOffset,
                   vcenter(item->parentItem()) - vcenter(item) + vCenterOffset);
         setItemPos(p);

      } else if (centerIn->parentItem() == item->parentItem()) {
         QPointF p(centerIn->x() + hcenter(centerIn) - hcenter(item) + effectiveHCenterOffset,
                   centerIn->y() + vcenter(centerIn) - vcenter(item) + vCenterOffset);
         setItemPos(p);
      }

      --updatingCenterIn;
   } else {
      // ### Make this certain :)
      qmlInfo(item) << QDeclarativeAnchors::tr("Possible anchor loop detected on centerIn.");
   }
}

void QDeclarativeAnchorsPrivate::clearItem(QGraphicsObject *item)
{
   if (!item) {
      return;
   }
   if (fill == item) {
      fill = 0;
   }
   if (centerIn == item) {
      centerIn = 0;
   }
   if (left.item == item) {
      left.item = 0;
      usedAnchors &= ~QDeclarativeAnchors::LeftAnchor;
   }
   if (right.item == item) {
      right.item = 0;
      usedAnchors &= ~QDeclarativeAnchors::RightAnchor;
   }
   if (top.item == item) {
      top.item = 0;
      usedAnchors &= ~QDeclarativeAnchors::TopAnchor;
   }
   if (bottom.item == item) {
      bottom.item = 0;
      usedAnchors &= ~QDeclarativeAnchors::BottomAnchor;
   }
   if (vCenter.item == item) {
      vCenter.item = 0;
      usedAnchors &= ~QDeclarativeAnchors::VCenterAnchor;
   }
   if (hCenter.item == item) {
      hCenter.item = 0;
      usedAnchors &= ~QDeclarativeAnchors::HCenterAnchor;
   }
   if (baseline.item == item) {
      baseline.item = 0;
      usedAnchors &= ~QDeclarativeAnchors::BaselineAnchor;
   }
}

void QDeclarativeAnchorsPrivate::addDepend(QGraphicsObject *item)
{
   if (!item) {
      return;
   }
   QGraphicsItemPrivate *itemPrivate = QGraphicsItemPrivate::get(item);
   if (itemPrivate->isDeclarativeItem) {
      QDeclarativeItemPrivate *p =
         static_cast<QDeclarativeItemPrivate *>(QGraphicsItemPrivate::get(item));
      p->addItemChangeListener(this, QDeclarativeItemPrivate::Geometry);
   } else if (itemPrivate->isWidget) {
      Q_Q(QDeclarativeAnchors);
      QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(item);
      QObject::connect(widget, SIGNAL(destroyed(QObject *)), q, SLOT(_q_widgetDestroyed(QObject *)));
      QObject::connect(widget, SIGNAL(geometryChanged()), q, SLOT(_q_widgetGeometryChanged()));
   }
}

void QDeclarativeAnchorsPrivate::remDepend(QGraphicsObject *item)
{
   if (!item) {
      return;
   }
   QGraphicsItemPrivate *itemPrivate = QGraphicsItemPrivate::get(item);
   if (itemPrivate->isDeclarativeItem) {
      QDeclarativeItemPrivate *p =
         static_cast<QDeclarativeItemPrivate *>(itemPrivate);
      p->removeItemChangeListener(this, QDeclarativeItemPrivate::Geometry);
   } else if (itemPrivate->isWidget) {
      Q_Q(QDeclarativeAnchors);
      QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(item);
      QObject::disconnect(widget, SIGNAL(destroyed(QObject *)), q, SLOT(_q_widgetDestroyed(QObject *)));
      QObject::disconnect(widget, SIGNAL(geometryChanged()), q, SLOT(_q_widgetGeometryChanged()));
   }
}

bool QDeclarativeAnchorsPrivate::isItemComplete() const
{
   return componentComplete;
}

void QDeclarativeAnchors::classBegin()
{
   Q_D(QDeclarativeAnchors);
   d->componentComplete = false;
}

void QDeclarativeAnchors::componentComplete()
{
   Q_D(QDeclarativeAnchors);
   d->componentComplete = true;
}

bool QDeclarativeAnchors::mirrored()
{
   Q_D(QDeclarativeAnchors);
   QGraphicsItemPrivate *itemPrivate = QGraphicsItemPrivate::get(d->item);
   return itemPrivate->isDeclarativeItem ? static_cast<QDeclarativeItemPrivate *>(itemPrivate)->effectiveLayoutMirror :
          false;
}

void QDeclarativeAnchorsPrivate::setItemHeight(qreal v)
{
   updatingMe = true;
   QGraphicsItemPrivate::get(item)->setHeight(v);
   updatingMe = false;
}

void QDeclarativeAnchorsPrivate::setItemWidth(qreal v)
{
   updatingMe = true;
   QGraphicsItemPrivate::get(item)->setWidth(v);
   updatingMe = false;
}

void QDeclarativeAnchorsPrivate::setItemX(qreal v)
{
   updatingMe = true;
   item->setX(v);
   updatingMe = false;
}

void QDeclarativeAnchorsPrivate::setItemY(qreal v)
{
   updatingMe = true;
   item->setY(v);
   updatingMe = false;
}

void QDeclarativeAnchorsPrivate::setItemPos(const QPointF &v)
{
   updatingMe = true;
   item->setPos(v);
   updatingMe = false;
}

void QDeclarativeAnchorsPrivate::setItemSize(const QSizeF &v)
{
   updatingMe = true;
   if (QGraphicsItemPrivate::get(item)->isWidget) {
      static_cast<QGraphicsWidget *>(item)->resize(v);
   } else if (QGraphicsItemPrivate::get(item)->isDeclarativeItem) {
      static_cast<QDeclarativeItem *>(item)->setSize(v);
   }
   updatingMe = false;
}

void QDeclarativeAnchorsPrivate::updateMe()
{
   if (updatingMe) {
      updatingMe = false;
      return;
   }

   fillChanged();
   centerInChanged();
   updateHorizontalAnchors();
   updateVerticalAnchors();
}

void QDeclarativeAnchorsPrivate::updateOnComplete()
{
   fillChanged();
   centerInChanged();
   updateHorizontalAnchors();
   updateVerticalAnchors();
}

void QDeclarativeAnchorsPrivate::_q_widgetDestroyed(QObject *obj)
{
   clearItem(qobject_cast<QGraphicsObject *>(obj));
}

void QDeclarativeAnchorsPrivate::_q_widgetGeometryChanged()
{
   fillChanged();
   centerInChanged();
   updateHorizontalAnchors();
   updateVerticalAnchors();
}

void QDeclarativeAnchorsPrivate::itemGeometryChanged(QDeclarativeItem *, const QRectF &newG, const QRectF &oldG)
{
   fillChanged();
   centerInChanged();
   if (newG.x() != oldG.x() || newG.width() != oldG.width()) {
      updateHorizontalAnchors();
   }
   if (newG.y() != oldG.y() || newG.height() != oldG.height()) {
      updateVerticalAnchors();
   }
}

QGraphicsObject *QDeclarativeAnchors::fill() const
{
   Q_D(const QDeclarativeAnchors);
   return d->fill;
}

void QDeclarativeAnchors::setFill(QGraphicsObject *f)
{
   Q_D(QDeclarativeAnchors);
   if (d->fill == f) {
      return;
   }

   if (!f) {
      d->remDepend(d->fill);
      d->fill = f;
      emit fillChanged();
      return;
   }
   if (f != d->item->parentItem() && f->parentItem() != d->item->parentItem()) {
      qmlInfo(d->item) << tr("Cannot anchor to an item that isn't a parent or sibling.");
      return;
   }
   d->remDepend(d->fill);
   d->fill = f;
   d->addDepend(d->fill);
   emit fillChanged();
   d->fillChanged();
}

void QDeclarativeAnchors::resetFill()
{
   setFill(0);
}

QGraphicsObject *QDeclarativeAnchors::centerIn() const
{
   Q_D(const QDeclarativeAnchors);
   return d->centerIn;
}

void QDeclarativeAnchors::setCenterIn(QGraphicsObject *c)
{
   Q_D(QDeclarativeAnchors);
   if (d->centerIn == c) {
      return;
   }

   if (!c) {
      d->remDepend(d->centerIn);
      d->centerIn = c;
      emit centerInChanged();
      return;
   }
   if (c != d->item->parentItem() && c->parentItem() != d->item->parentItem()) {
      qmlInfo(d->item) << tr("Cannot anchor to an item that isn't a parent or sibling.");
      return;
   }

   d->remDepend(d->centerIn);
   d->centerIn = c;
   d->addDepend(d->centerIn);
   emit centerInChanged();
   d->centerInChanged();
}

void QDeclarativeAnchors::resetCenterIn()
{
   setCenterIn(0);
}

bool QDeclarativeAnchorsPrivate::calcStretch(const QDeclarativeAnchorLine &edge1,
      const QDeclarativeAnchorLine &edge2,
      qreal offset1,
      qreal offset2,
      QDeclarativeAnchorLine::AnchorLine line,
      qreal &stretch)
{
   bool edge1IsParent = (edge1.item == item->parentItem());
   bool edge2IsParent = (edge2.item == item->parentItem());
   bool edge1IsSibling = (edge1.item->parentItem() == item->parentItem());
   bool edge2IsSibling = (edge2.item->parentItem() == item->parentItem());

   bool invalid = false;
   if ((edge2IsParent && edge1IsParent) || (edge2IsSibling && edge1IsSibling)) {
      stretch = (position(edge2.item, edge2.anchorLine) + offset2)
                - (position(edge1.item, edge1.anchorLine) + offset1);
   } else if (edge2IsParent && edge1IsSibling) {
      stretch = (position(edge2.item, edge2.anchorLine) + offset2)
                - (position(item->parentObject(), line)
                   + position(edge1.item, edge1.anchorLine) + offset1);
   } else if (edge2IsSibling && edge1IsParent) {
      stretch = (position(item->parentObject(), line) + position(edge2.item, edge2.anchorLine) + offset2)
                - (position(edge1.item, edge1.anchorLine) + offset1);
   } else {
      invalid = true;
   }

   return invalid;
}

void QDeclarativeAnchorsPrivate::updateVerticalAnchors()
{
   if (fill || centerIn || !isItemComplete()) {
      return;
   }

   if (updatingVerticalAnchor < 2) {
      ++updatingVerticalAnchor;
      QGraphicsItemPrivate *itemPrivate = QGraphicsItemPrivate::get(item);
      if (usedAnchors & QDeclarativeAnchors::TopAnchor) {
         //Handle stretching
         bool invalid = true;
         qreal height = 0.0;
         if (usedAnchors & QDeclarativeAnchors::BottomAnchor) {
            invalid = calcStretch(top, bottom, topMargin, -bottomMargin, QDeclarativeAnchorLine::Top, height);
         } else if (usedAnchors & QDeclarativeAnchors::VCenterAnchor) {
            invalid = calcStretch(top, vCenter, topMargin, vCenterOffset, QDeclarativeAnchorLine::Top, height);
            height *= 2;
         }
         if (!invalid) {
            setItemHeight(height);
         }

         //Handle top
         if (top.item == item->parentItem()) {
            setItemY(adjustedPosition(top.item, top.anchorLine) + topMargin);
         } else if (top.item->parentItem() == item->parentItem()) {
            setItemY(position(top.item, top.anchorLine) + topMargin);
         }
      } else if (usedAnchors & QDeclarativeAnchors::BottomAnchor) {
         //Handle stretching (top + bottom case is handled above)
         if (usedAnchors & QDeclarativeAnchors::VCenterAnchor) {
            qreal height = 0.0;
            bool invalid = calcStretch(vCenter, bottom, vCenterOffset, -bottomMargin,
                                       QDeclarativeAnchorLine::Top, height);
            if (!invalid) {
               setItemHeight(height * 2);
            }
         }

         //Handle bottom
         if (bottom.item == item->parentItem()) {
            setItemY(adjustedPosition(bottom.item, bottom.anchorLine) - itemPrivate->height() - bottomMargin);
         } else if (bottom.item->parentItem() == item->parentItem()) {
            setItemY(position(bottom.item, bottom.anchorLine) - itemPrivate->height() - bottomMargin);
         }
      } else if (usedAnchors & QDeclarativeAnchors::VCenterAnchor) {
         //(stetching handled above)

         //Handle vCenter
         if (vCenter.item == item->parentItem()) {
            setItemY(adjustedPosition(vCenter.item, vCenter.anchorLine)
                     - vcenter(item) + vCenterOffset);
         } else if (vCenter.item->parentItem() == item->parentItem()) {
            setItemY(position(vCenter.item, vCenter.anchorLine) - vcenter(item) + vCenterOffset);
         }
      } else if (usedAnchors & QDeclarativeAnchors::BaselineAnchor) {
         //Handle baseline
         if (baseline.item == item->parentItem()) {
            if (itemPrivate->isDeclarativeItem)
               setItemY(adjustedPosition(baseline.item, baseline.anchorLine)
                        - static_cast<QDeclarativeItem *>(item)->baselineOffset() + baselineOffset);
         } else if (baseline.item->parentItem() == item->parentItem()) {
            if (itemPrivate->isDeclarativeItem)
               setItemY(position(baseline.item, baseline.anchorLine)
                        - static_cast<QDeclarativeItem *>(item)->baselineOffset() + baselineOffset);
         }
      }
      --updatingVerticalAnchor;
   } else {
      // ### Make this certain :)
      qmlInfo(item) << QDeclarativeAnchors::tr("Possible anchor loop detected on vertical anchor.");
   }
}

inline QDeclarativeAnchorLine::AnchorLine reverseAnchorLine(QDeclarativeAnchorLine::AnchorLine anchorLine)
{
   if (anchorLine == QDeclarativeAnchorLine::Left) {
      return QDeclarativeAnchorLine::Right;
   } else if (anchorLine == QDeclarativeAnchorLine::Right) {
      return QDeclarativeAnchorLine::Left;
   } else {
      return anchorLine;
   }
}

void QDeclarativeAnchorsPrivate::updateHorizontalAnchors()
{
   Q_Q(QDeclarativeAnchors);
   if (fill || centerIn || !isItemComplete()) {
      return;
   }

   if (updatingHorizontalAnchor < 3) {
      ++updatingHorizontalAnchor;
      qreal effectiveRightMargin, effectiveLeftMargin, effectiveHorizontalCenterOffset;
      QDeclarativeAnchorLine effectiveLeft, effectiveRight, effectiveHorizontalCenter;
      QDeclarativeAnchors::Anchor effectiveLeftAnchor, effectiveRightAnchor;
      if (q->mirrored()) {
         effectiveLeftAnchor = QDeclarativeAnchors::RightAnchor;
         effectiveRightAnchor = QDeclarativeAnchors::LeftAnchor;
         effectiveLeft.item = right.item;
         effectiveLeft.anchorLine = reverseAnchorLine(right.anchorLine);
         effectiveRight.item = left.item;
         effectiveRight.anchorLine = reverseAnchorLine(left.anchorLine);
         effectiveHorizontalCenter.item = hCenter.item;
         effectiveHorizontalCenter.anchorLine = reverseAnchorLine(hCenter.anchorLine);
         effectiveLeftMargin = rightMargin;
         effectiveRightMargin = leftMargin;
         effectiveHorizontalCenterOffset = -hCenterOffset;
      } else {
         effectiveLeftAnchor = QDeclarativeAnchors::LeftAnchor;
         effectiveRightAnchor = QDeclarativeAnchors::RightAnchor;
         effectiveLeft = left;
         effectiveRight = right;
         effectiveHorizontalCenter = hCenter;
         effectiveLeftMargin = leftMargin;
         effectiveRightMargin = rightMargin;
         effectiveHorizontalCenterOffset = hCenterOffset;
      }

      QGraphicsItemPrivate *itemPrivate = QGraphicsItemPrivate::get(item);
      if (usedAnchors & effectiveLeftAnchor) {
         //Handle stretching
         bool invalid = true;
         qreal width = 0.0;
         if (usedAnchors & effectiveRightAnchor) {
            invalid = calcStretch(effectiveLeft, effectiveRight, effectiveLeftMargin, -effectiveRightMargin,
                                  QDeclarativeAnchorLine::Left, width);
         } else if (usedAnchors & QDeclarativeAnchors::HCenterAnchor) {
            invalid = calcStretch(effectiveLeft, effectiveHorizontalCenter, effectiveLeftMargin, effectiveHorizontalCenterOffset,
                                  QDeclarativeAnchorLine::Left, width);
            width *= 2;
         }
         if (!invalid) {
            setItemWidth(width);
         }

         //Handle left
         if (effectiveLeft.item == item->parentItem()) {
            setItemX(adjustedPosition(effectiveLeft.item, effectiveLeft.anchorLine) + effectiveLeftMargin);
         } else if (effectiveLeft.item->parentItem() == item->parentItem()) {
            setItemX(position(effectiveLeft.item, effectiveLeft.anchorLine) + effectiveLeftMargin);
         }
      } else if (usedAnchors & effectiveRightAnchor) {
         //Handle stretching (left + right case is handled in updateLeftAnchor)
         if (usedAnchors & QDeclarativeAnchors::HCenterAnchor) {
            qreal width = 0.0;
            bool invalid = calcStretch(effectiveHorizontalCenter, effectiveRight, effectiveHorizontalCenterOffset,
                                       -effectiveRightMargin,
                                       QDeclarativeAnchorLine::Left, width);
            if (!invalid) {
               setItemWidth(width * 2);
            }
         }

         //Handle right
         if (effectiveRight.item == item->parentItem()) {
            setItemX(adjustedPosition(effectiveRight.item,
                                      effectiveRight.anchorLine) - itemPrivate->width() - effectiveRightMargin);
         } else if (effectiveRight.item->parentItem() == item->parentItem()) {
            setItemX(position(effectiveRight.item, effectiveRight.anchorLine) - itemPrivate->width() - effectiveRightMargin);
         }
      } else if (usedAnchors & QDeclarativeAnchors::HCenterAnchor) {
         //Handle hCenter
         if (effectiveHorizontalCenter.item == item->parentItem()) {
            setItemX(adjustedPosition(effectiveHorizontalCenter.item,
                                      effectiveHorizontalCenter.anchorLine) - hcenter(item) + effectiveHorizontalCenterOffset);
         } else if (effectiveHorizontalCenter.item->parentItem() == item->parentItem()) {
            setItemX(position(effectiveHorizontalCenter.item,
                              effectiveHorizontalCenter.anchorLine) - hcenter(item) + effectiveHorizontalCenterOffset);
         }
      }
      --updatingHorizontalAnchor;
   } else {
      // ### Make this certain :)
      qmlInfo(item) << QDeclarativeAnchors::tr("Possible anchor loop detected on horizontal anchor.");
   }
}

QDeclarativeAnchorLine QDeclarativeAnchors::top() const
{
   Q_D(const QDeclarativeAnchors);
   return d->top;
}

void QDeclarativeAnchors::setTop(const QDeclarativeAnchorLine &edge)
{
   Q_D(QDeclarativeAnchors);
   if (!d->checkVAnchorValid(edge) || d->top == edge) {
      return;
   }

   d->usedAnchors |= TopAnchor;

   if (!d->checkVValid()) {
      d->usedAnchors &= ~TopAnchor;
      return;
   }

   d->remDepend(d->top.item);
   d->top = edge;
   d->addDepend(d->top.item);
   emit topChanged();
   d->updateVerticalAnchors();
}

void QDeclarativeAnchors::resetTop()
{
   Q_D(QDeclarativeAnchors);
   d->usedAnchors &= ~TopAnchor;
   d->remDepend(d->top.item);
   d->top = QDeclarativeAnchorLine();
   emit topChanged();
   d->updateVerticalAnchors();
}

QDeclarativeAnchorLine QDeclarativeAnchors::bottom() const
{
   Q_D(const QDeclarativeAnchors);
   return d->bottom;
}

void QDeclarativeAnchors::setBottom(const QDeclarativeAnchorLine &edge)
{
   Q_D(QDeclarativeAnchors);
   if (!d->checkVAnchorValid(edge) || d->bottom == edge) {
      return;
   }

   d->usedAnchors |= BottomAnchor;

   if (!d->checkVValid()) {
      d->usedAnchors &= ~BottomAnchor;
      return;
   }

   d->remDepend(d->bottom.item);
   d->bottom = edge;
   d->addDepend(d->bottom.item);
   emit bottomChanged();
   d->updateVerticalAnchors();
}

void QDeclarativeAnchors::resetBottom()
{
   Q_D(QDeclarativeAnchors);
   d->usedAnchors &= ~BottomAnchor;
   d->remDepend(d->bottom.item);
   d->bottom = QDeclarativeAnchorLine();
   emit bottomChanged();
   d->updateVerticalAnchors();
}

QDeclarativeAnchorLine QDeclarativeAnchors::verticalCenter() const
{
   Q_D(const QDeclarativeAnchors);
   return d->vCenter;
}

void QDeclarativeAnchors::setVerticalCenter(const QDeclarativeAnchorLine &edge)
{
   Q_D(QDeclarativeAnchors);
   if (!d->checkVAnchorValid(edge) || d->vCenter == edge) {
      return;
   }

   d->usedAnchors |= VCenterAnchor;

   if (!d->checkVValid()) {
      d->usedAnchors &= ~VCenterAnchor;
      return;
   }

   d->remDepend(d->vCenter.item);
   d->vCenter = edge;
   d->addDepend(d->vCenter.item);
   emit verticalCenterChanged();
   d->updateVerticalAnchors();
}

void QDeclarativeAnchors::resetVerticalCenter()
{
   Q_D(QDeclarativeAnchors);
   d->usedAnchors &= ~VCenterAnchor;
   d->remDepend(d->vCenter.item);
   d->vCenter = QDeclarativeAnchorLine();
   emit verticalCenterChanged();
   d->updateVerticalAnchors();
}

QDeclarativeAnchorLine QDeclarativeAnchors::baseline() const
{
   Q_D(const QDeclarativeAnchors);
   return d->baseline;
}

void QDeclarativeAnchors::setBaseline(const QDeclarativeAnchorLine &edge)
{
   Q_D(QDeclarativeAnchors);
   if (!d->checkVAnchorValid(edge) || d->baseline == edge) {
      return;
   }

   d->usedAnchors |= BaselineAnchor;

   if (!d->checkVValid()) {
      d->usedAnchors &= ~BaselineAnchor;
      return;
   }

   d->remDepend(d->baseline.item);
   d->baseline = edge;
   d->addDepend(d->baseline.item);
   emit baselineChanged();
   d->updateVerticalAnchors();
}

void QDeclarativeAnchors::resetBaseline()
{
   Q_D(QDeclarativeAnchors);
   d->usedAnchors &= ~BaselineAnchor;
   d->remDepend(d->baseline.item);
   d->baseline = QDeclarativeAnchorLine();
   emit baselineChanged();
   d->updateVerticalAnchors();
}

QDeclarativeAnchorLine QDeclarativeAnchors::left() const
{
   Q_D(const QDeclarativeAnchors);
   return d->left;
}

void QDeclarativeAnchors::setLeft(const QDeclarativeAnchorLine &edge)
{
   Q_D(QDeclarativeAnchors);
   if (!d->checkHAnchorValid(edge) || d->left == edge) {
      return;
   }

   d->usedAnchors |= LeftAnchor;

   if (!d->checkHValid()) {
      d->usedAnchors &= ~LeftAnchor;
      return;
   }

   d->remDepend(d->left.item);
   d->left = edge;
   d->addDepend(d->left.item);
   emit leftChanged();
   d->updateHorizontalAnchors();
}

void QDeclarativeAnchors::resetLeft()
{
   Q_D(QDeclarativeAnchors);
   d->usedAnchors &= ~LeftAnchor;
   d->remDepend(d->left.item);
   d->left = QDeclarativeAnchorLine();
   emit leftChanged();
   d->updateHorizontalAnchors();
}

QDeclarativeAnchorLine QDeclarativeAnchors::right() const
{
   Q_D(const QDeclarativeAnchors);
   return d->right;
}

void QDeclarativeAnchors::setRight(const QDeclarativeAnchorLine &edge)
{
   Q_D(QDeclarativeAnchors);
   if (!d->checkHAnchorValid(edge) || d->right == edge) {
      return;
   }

   d->usedAnchors |= RightAnchor;

   if (!d->checkHValid()) {
      d->usedAnchors &= ~RightAnchor;
      return;
   }

   d->remDepend(d->right.item);
   d->right = edge;
   d->addDepend(d->right.item);
   emit rightChanged();
   d->updateHorizontalAnchors();
}

void QDeclarativeAnchors::resetRight()
{
   Q_D(QDeclarativeAnchors);
   d->usedAnchors &= ~RightAnchor;
   d->remDepend(d->right.item);
   d->right = QDeclarativeAnchorLine();
   emit rightChanged();
   d->updateHorizontalAnchors();
}

QDeclarativeAnchorLine QDeclarativeAnchors::horizontalCenter() const
{
   Q_D(const QDeclarativeAnchors);
   return d->hCenter;
}

void QDeclarativeAnchors::setHorizontalCenter(const QDeclarativeAnchorLine &edge)
{
   Q_D(QDeclarativeAnchors);
   if (!d->checkHAnchorValid(edge) || d->hCenter == edge) {
      return;
   }

   d->usedAnchors |= HCenterAnchor;

   if (!d->checkHValid()) {
      d->usedAnchors &= ~HCenterAnchor;
      return;
   }

   d->remDepend(d->hCenter.item);
   d->hCenter = edge;
   d->addDepend(d->hCenter.item);
   emit horizontalCenterChanged();
   d->updateHorizontalAnchors();
}

void QDeclarativeAnchors::resetHorizontalCenter()
{
   Q_D(QDeclarativeAnchors);
   d->usedAnchors &= ~HCenterAnchor;
   d->remDepend(d->hCenter.item);
   d->hCenter = QDeclarativeAnchorLine();
   emit horizontalCenterChanged();
   d->updateHorizontalAnchors();
}

qreal QDeclarativeAnchors::leftMargin() const
{
   Q_D(const QDeclarativeAnchors);
   return d->leftMargin;
}

void QDeclarativeAnchors::setLeftMargin(qreal offset)
{
   Q_D(QDeclarativeAnchors);
   if (d->leftMargin == offset) {
      return;
   }
   d->leftMargin = offset;
   if (d->fill) {
      d->fillChanged();
   } else {
      d->updateHorizontalAnchors();
   }
   emit leftMarginChanged();
}

qreal QDeclarativeAnchors::rightMargin() const
{
   Q_D(const QDeclarativeAnchors);
   return d->rightMargin;
}

void QDeclarativeAnchors::setRightMargin(qreal offset)
{
   Q_D(QDeclarativeAnchors);
   if (d->rightMargin == offset) {
      return;
   }
   d->rightMargin = offset;
   if (d->fill) {
      d->fillChanged();
   } else {
      d->updateHorizontalAnchors();
   }
   emit rightMarginChanged();
}

qreal QDeclarativeAnchors::margins() const
{
   Q_D(const QDeclarativeAnchors);
   return d->margins;
}

void QDeclarativeAnchors::setMargins(qreal offset)
{
   Q_D(QDeclarativeAnchors);
   if (d->margins == offset) {
      return;
   }
   //###Is it significantly faster to set them directly so we can call fillChanged only once?
   if (!d->rightMargin || d->rightMargin == d->margins) {
      setRightMargin(offset);
   }
   if (!d->leftMargin || d->leftMargin == d->margins) {
      setLeftMargin(offset);
   }
   if (!d->topMargin || d->topMargin == d->margins) {
      setTopMargin(offset);
   }
   if (!d->bottomMargin || d->bottomMargin == d->margins) {
      setBottomMargin(offset);
   }
   d->margins = offset;
   emit marginsChanged();

}

qreal QDeclarativeAnchors::horizontalCenterOffset() const
{
   Q_D(const QDeclarativeAnchors);
   return d->hCenterOffset;
}

void QDeclarativeAnchors::setHorizontalCenterOffset(qreal offset)
{
   Q_D(QDeclarativeAnchors);
   if (d->hCenterOffset == offset) {
      return;
   }
   d->hCenterOffset = offset;
   if (d->centerIn) {
      d->centerInChanged();
   } else {
      d->updateHorizontalAnchors();
   }
   emit horizontalCenterOffsetChanged();
}

qreal QDeclarativeAnchors::topMargin() const
{
   Q_D(const QDeclarativeAnchors);
   return d->topMargin;
}

void QDeclarativeAnchors::setTopMargin(qreal offset)
{
   Q_D(QDeclarativeAnchors);
   if (d->topMargin == offset) {
      return;
   }
   d->topMargin = offset;
   if (d->fill) {
      d->fillChanged();
   } else {
      d->updateVerticalAnchors();
   }
   emit topMarginChanged();
}

qreal QDeclarativeAnchors::bottomMargin() const
{
   Q_D(const QDeclarativeAnchors);
   return d->bottomMargin;
}

void QDeclarativeAnchors::setBottomMargin(qreal offset)
{
   Q_D(QDeclarativeAnchors);
   if (d->bottomMargin == offset) {
      return;
   }
   d->bottomMargin = offset;
   if (d->fill) {
      d->fillChanged();
   } else {
      d->updateVerticalAnchors();
   }
   emit bottomMarginChanged();
}

qreal QDeclarativeAnchors::verticalCenterOffset() const
{
   Q_D(const QDeclarativeAnchors);
   return d->vCenterOffset;
}

void QDeclarativeAnchors::setVerticalCenterOffset(qreal offset)
{
   Q_D(QDeclarativeAnchors);
   if (d->vCenterOffset == offset) {
      return;
   }
   d->vCenterOffset = offset;
   if (d->centerIn) {
      d->centerInChanged();
   } else {
      d->updateVerticalAnchors();
   }
   emit verticalCenterOffsetChanged();
}

qreal QDeclarativeAnchors::baselineOffset() const
{
   Q_D(const QDeclarativeAnchors);
   return d->baselineOffset;
}

void QDeclarativeAnchors::setBaselineOffset(qreal offset)
{
   Q_D(QDeclarativeAnchors);
   if (d->baselineOffset == offset) {
      return;
   }
   d->baselineOffset = offset;
   d->updateVerticalAnchors();
   emit baselineOffsetChanged();
}

QDeclarativeAnchors::Anchors QDeclarativeAnchors::usedAnchors() const
{
   Q_D(const QDeclarativeAnchors);
   return d->usedAnchors;
}

bool QDeclarativeAnchorsPrivate::checkHValid() const
{
   if (usedAnchors & QDeclarativeAnchors::LeftAnchor &&
         usedAnchors & QDeclarativeAnchors::RightAnchor &&
         usedAnchors & QDeclarativeAnchors::HCenterAnchor) {
      qmlInfo(item) << QDeclarativeAnchors::tr("Cannot specify left, right, and hcenter anchors.");
      return false;
   }

   return true;
}

bool QDeclarativeAnchorsPrivate::checkHAnchorValid(QDeclarativeAnchorLine anchor) const
{
   if (!anchor.item) {
      qmlInfo(item) << QDeclarativeAnchors::tr("Cannot anchor to a null item.");
      return false;
   } else if (anchor.anchorLine & QDeclarativeAnchorLine::Vertical_Mask) {
      qmlInfo(item) << QDeclarativeAnchors::tr("Cannot anchor a horizontal edge to a vertical edge.");
      return false;
   } else if (anchor.item != item->parentItem() && anchor.item->parentItem() != item->parentItem()) {
      qmlInfo(item) << QDeclarativeAnchors::tr("Cannot anchor to an item that isn't a parent or sibling.");
      return false;
   } else if (anchor.item == item) {
      qmlInfo(item) << QDeclarativeAnchors::tr("Cannot anchor item to self.");
      return false;
   }

   return true;
}

bool QDeclarativeAnchorsPrivate::checkVValid() const
{
   if (usedAnchors & QDeclarativeAnchors::TopAnchor &&
         usedAnchors & QDeclarativeAnchors::BottomAnchor &&
         usedAnchors & QDeclarativeAnchors::VCenterAnchor) {
      qmlInfo(item) << QDeclarativeAnchors::tr("Cannot specify top, bottom, and vcenter anchors.");
      return false;
   } else if (usedAnchors & QDeclarativeAnchors::BaselineAnchor &&
              (usedAnchors & QDeclarativeAnchors::TopAnchor ||
               usedAnchors & QDeclarativeAnchors::BottomAnchor ||
               usedAnchors & QDeclarativeAnchors::VCenterAnchor)) {
      qmlInfo(item) <<
                    QDeclarativeAnchors::tr("Baseline anchor cannot be used in conjunction with top, bottom, or vcenter anchors.");
      return false;
   }

   return true;
}

bool QDeclarativeAnchorsPrivate::checkVAnchorValid(QDeclarativeAnchorLine anchor) const
{
   if (!anchor.item) {
      qmlInfo(item) << QDeclarativeAnchors::tr("Cannot anchor to a null item.");
      return false;
   } else if (anchor.anchorLine & QDeclarativeAnchorLine::Horizontal_Mask) {
      qmlInfo(item) << QDeclarativeAnchors::tr("Cannot anchor a vertical edge to a horizontal edge.");
      return false;
   } else if (anchor.item != item->parentItem() && anchor.item->parentItem() != item->parentItem()) {
      qmlInfo(item) << QDeclarativeAnchors::tr("Cannot anchor to an item that isn't a parent or sibling.");
      return false;
   } else if (anchor.item == item) {
      qmlInfo(item) << QDeclarativeAnchors::tr("Cannot anchor item to self.");
      return false;
   }

   return true;
}

void QDeclarativeAnchors::_q_widgetGeometryChanged()
{
   Q_D(QDeclarativeAnchors);
   d->_q_widgetGeometryChanged();
}

void QDeclarativeAnchors::_q_widgetDestroyed(QObject *obj)
{
   Q_D(QDeclarativeAnchors);
   d->_q_widgetDestroyed(obj);
}

QT_END_NAMESPACE
