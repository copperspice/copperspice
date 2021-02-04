/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#include <qglobal.h>

#ifndef QT_NO_GRAPHICSVIEW

#include <qgraphicslayout.h>
#include <qgraphicsscene.h>
#include <qgraphicslayoutitem.h>
#include <qwidget.h>
#include <qgraphicswidget.h>
#include <qdebug.h>

#include <qgraphics_layoutitem_p.h>

/*
    COMBINE_SIZE() is identical to combineSize(), except that it
    doesn't evaluate 'size' unless necessary.
*/
#define COMBINE_SIZE(result, size) \
    do { \
        if ((result).width() < 0 || (result).height() < 0) \
            combineSize((result), (size)); \
    } while (false)

static void combineSize(QSizeF &result, const QSizeF &size)
{
   if (result.width() < 0) {
      result.setWidth(size.width());
   }
   if (result.height() < 0) {
      result.setHeight(size.height());
   }
}

static void boundSize(QSizeF &result, const QSizeF &size)
{
   if (size.width() >= 0 && size.width() < result.width()) {
      result.setWidth(size.width());
   }
   if (size.height() >= 0 && size.height() < result.height()) {
      result.setHeight(size.height());
   }
}

static void expandSize(QSizeF &result, const QSizeF &size)
{
   if (size.width() >= 0 && size.width() > result.width()) {
      result.setWidth(size.width());
   }
   if (size.height() >= 0 && size.height() > result.height()) {
      result.setHeight(size.height());
   }
}

static void normalizeHints(qreal &minimum, qreal &preferred, qreal &maximum, qreal &descent)
{
   if (minimum >= 0 && maximum >= 0 && minimum > maximum) {
      minimum = maximum;
   }

   if (preferred >= 0) {
      if (minimum >= 0 && preferred < minimum) {
         preferred = minimum;
      } else if (maximum >= 0 && preferred > maximum) {
         preferred = maximum;
      }
   }

   if (minimum >= 0 && descent > minimum) {
      descent = minimum;
   }
}

/*!
    \internal
*/
QGraphicsLayoutItemPrivate::QGraphicsLayoutItemPrivate(QGraphicsLayoutItem *par, bool layout)
   : parent(par), userSizeHints(0), isLayout(layout), ownedByLayout(false), graphicsItem(0)
{
}

/*!
    \internal
*/
QGraphicsLayoutItemPrivate::~QGraphicsLayoutItemPrivate()
{
   // Remove any lazily allocated data
   delete[] userSizeHints;
}

/*!
    \internal
*/
void QGraphicsLayoutItemPrivate::init()
{
   sizeHintCacheDirty = true;
   sizeHintWithConstraintCacheDirty = true;
}


QSizeF *QGraphicsLayoutItemPrivate::effectiveSizeHints(const QSizeF &constraint) const
{
   Q_Q(const QGraphicsLayoutItem);
   QSizeF *sizeHintCache;
   const bool hasConstraint = constraint.width() >= 0 || constraint.height() >= 0;

   if (hasConstraint) {
      if (!sizeHintWithConstraintCacheDirty && constraint == cachedConstraint) {
         return cachedSizeHintsWithConstraints;
      }

      sizeHintCache = cachedSizeHintsWithConstraints;
   } else {
      if (!sizeHintCacheDirty) {
         return cachedSizeHints;
      }
      sizeHintCache = cachedSizeHints;
   }

   for (int i = 0; i < Qt::NSizeHints; ++i) {
      sizeHintCache[i] = constraint;
      if (userSizeHints) {
         combineSize(sizeHintCache[i], userSizeHints[i]);
      }
   }

   QSizeF &minS = sizeHintCache[Qt::MinimumSize];
   QSizeF &prefS = sizeHintCache[Qt::PreferredSize];
   QSizeF &maxS = sizeHintCache[Qt::MaximumSize];
   QSizeF &descentS = sizeHintCache[Qt::MinimumDescent];

   normalizeHints(minS.rwidth(), prefS.rwidth(), maxS.rwidth(), descentS.rwidth());
   normalizeHints(minS.rheight(), prefS.rheight(), maxS.rheight(), descentS.rheight());

   // if the minimum, preferred and maximum sizes contradict each other
   // (e.g. the minimum is larger than the maximum) we give priority to
   // the maximum size, then the minimum size and finally the preferred size
   COMBINE_SIZE(maxS, q->sizeHint(Qt::MaximumSize, maxS));
   combineSize(maxS, QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
   expandSize(maxS, prefS);
   expandSize(maxS, minS);
   boundSize(maxS, QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));

   COMBINE_SIZE(minS, q->sizeHint(Qt::MinimumSize, minS));
   expandSize(minS, QSizeF(0, 0));
   boundSize(minS, prefS);
   boundSize(minS, maxS);

   COMBINE_SIZE(prefS, q->sizeHint(Qt::PreferredSize, prefS));
   expandSize(prefS, minS);
   boundSize(prefS, maxS);

   // Not supported yet
   // COMBINE_SIZE(descentS, q->sizeHint(Qt::MinimumDescent, constraint));

   if (hasConstraint) {
      cachedConstraint = constraint;
      sizeHintWithConstraintCacheDirty = false;
   } else {
      sizeHintCacheDirty = false;
   }
   return sizeHintCache;
}


/*!
    \internal

    Returns the parent item of this layout, or 0 if this layout is
    not installed on any widget.

    If this is the item that the layout is installed on, it will return "itself".

    If the layout is a sub-layout, this function returns the parent
    widget of the parent layout.

    Note that it will traverse up the layout item hierarchy instead of just calling
    QGraphicsItem::parentItem(). This is on purpose.

    \sa parent()
*/
QGraphicsItem *QGraphicsLayoutItemPrivate::parentItem() const
{
   Q_Q(const QGraphicsLayoutItem);

   const QGraphicsLayoutItem *parent = q;
   while (parent && parent->isLayout()) {
      parent = parent->parentLayoutItem();
   }
   return parent ? parent->graphicsItem() : 0;
}

/*!
    \internal

     Ensures that userSizeHints is allocated.
     This function must be called before any dereferencing.
*/
void QGraphicsLayoutItemPrivate::ensureUserSizeHints()
{
   if (!userSizeHints) {
      userSizeHints = new QSizeF[Qt::NSizeHints];
   }
}

/*!
    \internal

    Sets the user size hint \a which to \a size. Use an invalid size to unset the size hint.
 */
void QGraphicsLayoutItemPrivate::setSize(Qt::SizeHint which, const QSizeF &size)
{
   Q_Q(QGraphicsLayoutItem);

   if (userSizeHints) {
      if (size == userSizeHints[which]) {
         return;
      }
   } else if (size.width() < 0 && size.height() < 0) {
      return;
   }

   ensureUserSizeHints();
   userSizeHints[which] = size;
   q->updateGeometry();
}

/*!
    \internal

    Sets the width of the user size hint \a which to \a width.
 */
void QGraphicsLayoutItemPrivate::setSizeComponent(
   Qt::SizeHint which, SizeComponent component, qreal value)
{
   Q_Q(QGraphicsLayoutItem);
   ensureUserSizeHints();
   qreal &userValue = (component == Width)
      ? userSizeHints[which].rwidth()
      : userSizeHints[which].rheight();
   if (value == userValue) {
      return;
   }
   userValue = value;
   q->updateGeometry();
}


bool QGraphicsLayoutItemPrivate::hasHeightForWidth() const
{
   Q_Q(const QGraphicsLayoutItem);
   if (isLayout) {
      const QGraphicsLayout *l = static_cast<const QGraphicsLayout *>(q);
      for (int i = l->count() - 1; i >= 0; --i) {
         if (QGraphicsLayoutItemPrivate::get(l->itemAt(i))->hasHeightForWidth()) {
            return true;
         }
      }
   } else if (QGraphicsItem *item = q->graphicsItem()) {
      if (item->isWidget()) {
         QGraphicsWidget *w = static_cast<QGraphicsWidget *>(item);
         if (w->layout()) {
            return QGraphicsLayoutItemPrivate::get(w->layout())->hasHeightForWidth();
         }
      }
   }
   return q->sizePolicy().hasHeightForWidth();
}

bool QGraphicsLayoutItemPrivate::hasWidthForHeight() const
{
   Q_Q(const QGraphicsLayoutItem);
   if (isLayout) {
      const QGraphicsLayout *l = static_cast<const QGraphicsLayout *>(q);
      for (int i = l->count() - 1; i >= 0; --i) {
         if (QGraphicsLayoutItemPrivate::get(l->itemAt(i))->hasWidthForHeight()) {
            return true;
         }
      }
   } else if (QGraphicsItem *item = q->graphicsItem()) {
      if (item->isWidget()) {
         QGraphicsWidget *w = static_cast<QGraphicsWidget *>(item);
         if (w->layout()) {
            return QGraphicsLayoutItemPrivate::get(w->layout())->hasWidthForHeight();
         }
      }
   }
   return q->sizePolicy().hasWidthForHeight();
}

QGraphicsLayoutItem::QGraphicsLayoutItem(QGraphicsLayoutItem *parent, bool isLayout)
   : d_ptr(new QGraphicsLayoutItemPrivate(parent, isLayout))
{
   Q_D(QGraphicsLayoutItem);
   d->init();
   d->sizePolicy = QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   d->q_ptr = this;
}

/*!
    \internal
*/
QGraphicsLayoutItem::QGraphicsLayoutItem(QGraphicsLayoutItemPrivate &dd)
   : d_ptr(&dd)
{
   Q_D(QGraphicsLayoutItem);
   d->init();
   d->q_ptr = this;
}


QGraphicsLayoutItem::~QGraphicsLayoutItem()
{
   QGraphicsLayoutItem *parentLI = parentLayoutItem();
   if (parentLI && parentLI->isLayout()) {
      QGraphicsLayout *lay = static_cast<QGraphicsLayout *>(parentLI);
      // this is not optimal
      for (int i = lay->count() - 1; i >= 0; --i) {
         if (lay->itemAt(i) == this) {
            lay->removeAt(i);
            break;
         }
      }
   }
}


void QGraphicsLayoutItem::setSizePolicy(const QSizePolicy &policy)
{
   Q_D(QGraphicsLayoutItem);
   if (d->sizePolicy == policy) {
      return;
   }
   d->sizePolicy = policy;
   updateGeometry();
}

/*!
    \overload

    This function is equivalent to calling
    setSizePolicy(QSizePolicy(\a hPolicy, \a vPolicy, \a controlType)).

    \sa sizePolicy(), QWidget::sizePolicy()
*/
void QGraphicsLayoutItem::setSizePolicy(QSizePolicy::Policy hPolicy,
   QSizePolicy::Policy vPolicy,
   QSizePolicy::ControlType controlType)
{
   setSizePolicy(QSizePolicy(hPolicy, vPolicy, controlType));
}

/*!
    Returns the current size policy.

    \sa setSizePolicy(), QWidget::sizePolicy()
*/
QSizePolicy QGraphicsLayoutItem::sizePolicy() const
{
   Q_D(const QGraphicsLayoutItem);
   return d->sizePolicy;
}

/*!
    Sets the minimum size to \a size. This property overrides sizeHint() for
    Qt::MinimumSize and ensures that effectiveSizeHint() will never return
    a size smaller than \a size. In order to unset the minimum size, use an
    invalid size.

    \sa minimumSize(), maximumSize(), preferredSize(), Qt::MinimumSize,
    sizeHint(), setMinimumWidth(), setMinimumHeight()
*/
void QGraphicsLayoutItem::setMinimumSize(const QSizeF &size)
{
   d_ptr->setSize(Qt::MinimumSize, size);
}

/*!
    \fn QGraphicsLayoutItem::setMinimumSize(qreal w, qreal h)

    This convenience function is equivalent to calling
    setMinimumSize(QSizeF(\a w, \a h)).

    \sa minimumSize(), setMaximumSize(), setPreferredSize(), sizeHint()
*/

/*!
    Returns the minimum size.

    \sa setMinimumSize(), preferredSize(), maximumSize(), Qt::MinimumSize,
    sizeHint()
*/
QSizeF QGraphicsLayoutItem::minimumSize() const
{
   return effectiveSizeHint(Qt::MinimumSize);
}

/*!
    Sets the minimum width to \a width.

    \sa minimumWidth(), setMinimumSize(), minimumSize()
*/
void QGraphicsLayoutItem::setMinimumWidth(qreal width)
{
   d_ptr->setSizeComponent(Qt::MinimumSize, d_ptr->Width, width);
}

/*!
    Sets the minimum height to \a height.

    \sa minimumHeight(), setMinimumSize(), minimumSize()
*/
void QGraphicsLayoutItem::setMinimumHeight(qreal height)
{
   d_ptr->setSizeComponent(Qt::MinimumSize, d_ptr->Height, height);
}


/*!
    Sets the preferred size to \a size. This property overrides sizeHint() for
    Qt::PreferredSize and provides the default value for effectiveSizeHint().
    In order to unset the preferred size, use an invalid size.

    \sa preferredSize(), minimumSize(), maximumSize(), Qt::PreferredSize,
    sizeHint()
*/
void QGraphicsLayoutItem::setPreferredSize(const QSizeF &size)
{
   d_ptr->setSize(Qt::PreferredSize, size);
}

/*!
    \fn QGraphicsLayoutItem::setPreferredSize(qreal w, qreal h)

    This convenience function is equivalent to calling
    setPreferredSize(QSizeF(\a w, \a h)).

    \sa preferredSize(), setMaximumSize(), setMinimumSize(), sizeHint()
*/

/*!
    Returns the preferred size.

    \sa setPreferredSize(), minimumSize(), maximumSize(), Qt::PreferredSize,
    sizeHint()
*/
QSizeF QGraphicsLayoutItem::preferredSize() const
{
   return effectiveSizeHint(Qt::PreferredSize);
}

/*!
    Sets the preferred height to \a height.

    \sa preferredWidth(), setPreferredSize(), preferredSize()
*/
void QGraphicsLayoutItem::setPreferredHeight(qreal height)
{
   d_ptr->setSizeComponent(Qt::PreferredSize, d_ptr->Height, height);
}

/*!
    Sets the preferred width to \a width.

    \sa preferredHeight(), setPreferredSize(), preferredSize()
*/
void QGraphicsLayoutItem::setPreferredWidth(qreal width)
{
   d_ptr->setSizeComponent(Qt::PreferredSize, d_ptr->Width, width);
}

/*!
    Sets the maximum size to \a size. This property overrides sizeHint() for
    Qt::MaximumSize and ensures that effectiveSizeHint() will never return a
    size larger than \a size. In order to unset the maximum size, use an
    invalid size.

    \sa maximumSize(), minimumSize(), preferredSize(), Qt::MaximumSize,
    sizeHint()
*/
void QGraphicsLayoutItem::setMaximumSize(const QSizeF &size)
{
   d_ptr->setSize(Qt::MaximumSize, size);
}

/*!
    \fn QGraphicsLayoutItem::setMaximumSize(qreal w, qreal h)

    This convenience function is equivalent to calling
    setMaximumSize(QSizeF(\a w, \a h)).

    \sa maximumSize(), setMinimumSize(), setPreferredSize(), sizeHint()
*/

/*!
    Returns the maximum size.

    \sa setMaximumSize(), minimumSize(), preferredSize(), Qt::MaximumSize,
    sizeHint()
*/
QSizeF QGraphicsLayoutItem::maximumSize() const
{
   return effectiveSizeHint(Qt::MaximumSize);
}

/*!
    Sets the maximum width to \a width.

    \sa maximumWidth(), setMaximumSize(), maximumSize()
*/
void QGraphicsLayoutItem::setMaximumWidth(qreal width)
{
   d_ptr->setSizeComponent(Qt::MaximumSize, d_ptr->Width, width);
}

/*!
    Sets the maximum height to \a height.

    \sa maximumHeight(), setMaximumSize(), maximumSize()
*/
void QGraphicsLayoutItem::setMaximumHeight(qreal height)
{
   d_ptr->setSizeComponent(Qt::MaximumSize, d_ptr->Height, height);
}


void QGraphicsLayoutItem::setGeometry(const QRectF &rect)
{
   Q_D(QGraphicsLayoutItem);
   QSizeF effectiveSize = rect.size().expandedTo(effectiveSizeHint(Qt::MinimumSize))
      .boundedTo(effectiveSizeHint(Qt::MaximumSize));
   d->geom = QRectF(rect.topLeft(), effectiveSize);
}

QRectF QGraphicsLayoutItem::geometry() const
{
   Q_D(const QGraphicsLayoutItem);
   return d->geom;
}

/*!
    This virtual function provides the \a left, \a top, \a right and \a bottom
    contents margins for this QGraphicsLayoutItem. The default implementation
    assumes all contents margins are 0. The parameters point to values stored
    in qreals. If any of the pointers is 0, that value will not be updated.

    \sa QGraphicsWidget::setContentsMargins()
*/
void QGraphicsLayoutItem::getContentsMargins(qreal *left, qreal *top, qreal *right, qreal *bottom) const
{
   if (left) {
      *left = 0;
   }
   if (top) {
      *top = 0;
   }
   if (right) {
      *right = 0;
   }
   if (bottom) {
      *bottom = 0;
   }
}

QRectF QGraphicsLayoutItem::contentsRect() const
{
   qreal left, top, right, bottom;
   getContentsMargins(&left, &top, &right, &bottom);
   return QRectF(QPointF(), geometry().size()).adjusted(+left, +top, -right, -bottom);
}

QSizeF QGraphicsLayoutItem::effectiveSizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
   Q_D(const QGraphicsLayoutItem);

   if (!d->userSizeHints && constraint.isValid()) {
      return constraint;
   }

   // ### should respect size policy???
   return d_ptr->effectiveSizeHints(constraint)[which];
}


void QGraphicsLayoutItem::updateGeometry()
{
   Q_D(QGraphicsLayoutItem);
   d->sizeHintCacheDirty = true;
   d->sizeHintWithConstraintCacheDirty = true;
}

/*!
    Returns the parent of this QGraphicsLayoutItem, or 0 if there is no parent,
    or if the parent does not inherit from QGraphicsLayoutItem
    (QGraphicsLayoutItem is often used through multiple inheritance with
    QObject-derived classes).

    \sa setParentLayoutItem()
*/
QGraphicsLayoutItem *QGraphicsLayoutItem::parentLayoutItem() const
{
   return d_func()->parent;
}


void QGraphicsLayoutItem::setParentLayoutItem(QGraphicsLayoutItem *parent)
{
   d_func()->parent = parent;
}

/*!
    Returns true if this QGraphicsLayoutItem is a layout (e.g., is inherited
    by an object that arranges other QGraphicsLayoutItem objects); otherwise
    returns false.

    \sa QGraphicsLayout
*/
bool QGraphicsLayoutItem::isLayout() const
{
   return d_func()->isLayout;
}


bool QGraphicsLayoutItem::ownedByLayout() const
{
   return d_func()->ownedByLayout;
}
/*!
    \since 4.6

    Sets whether a layout should delete this item in its destructor or not.
    \a ownership must be true to in order for the layout to delete it.
    \sa ownedByLayout()
*/
void QGraphicsLayoutItem::setOwnedByLayout(bool ownership)
{
   d_func()->ownedByLayout = ownership;
}

/*!
 * Returns the QGraphicsItem that this layout item represents.
 * For QGraphicsWidget it will return itself. For custom items it can return an
 * aggregated value.
 *
 * \sa setGraphicsItem()
 */
QGraphicsItem *QGraphicsLayoutItem::graphicsItem() const
{
   return d_func()->graphicsItem;
}

/*!
 * If the QGraphicsLayoutItem represents a QGraphicsItem, and it wants to take
 * advantage of the automatic reparenting capabilities of QGraphicsLayout it
 * should set this value.
 * Note that if you delete \a item and not delete the layout item, you are
 * responsible of calling setGraphicsItem(0) in order to avoid having a
 * dangling pointer.
 *
 * \sa graphicsItem()
 */
void QGraphicsLayoutItem::setGraphicsItem(QGraphicsItem *item)
{
   d_func()->graphicsItem = item;
}


#endif //QT_NO_GRAPHICSVIEW
