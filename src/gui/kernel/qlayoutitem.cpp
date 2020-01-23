/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qlayout.h>

#include <qapplication.h>
#include <qlayoutengine_p.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qevent.h>
#include <qstyle.h>
#include <qvariant.h>
#include <qwidget_p.h>

inline static QRect fromLayoutItemRect(QWidgetPrivate *priv, const QRect &rect)
{
   return rect.adjusted(priv->leftLayoutItemMargin, priv->topLayoutItemMargin,
         -priv->rightLayoutItemMargin, -priv->bottomLayoutItemMargin);
}

inline static QSize fromLayoutItemSize(QWidgetPrivate *priv, const QSize &size)
{
   return fromLayoutItemRect(priv, QRect(QPoint(0, 0), size)).size();
}

inline static QRect toLayoutItemRect(QWidgetPrivate *priv, const QRect &rect)
{
   return rect.adjusted(-priv->leftLayoutItemMargin, -priv->topLayoutItemMargin,
         priv->rightLayoutItemMargin, priv->bottomLayoutItemMargin);
}

inline static QSize toLayoutItemSize(QWidgetPrivate *priv, const QSize &size)
{
   return toLayoutItemRect(priv, QRect(QPoint(0, 0), size)).size();
}

void QLayoutItem::setAlignment(Qt::Alignment alignment)
{
   align = alignment;
}

QSpacerItem::~QSpacerItem()
{
}
void QSpacerItem::changeSize(int w, int h, QSizePolicy::Policy hPolicy,
   QSizePolicy::Policy vPolicy)
{
   width = w;
   height = h;
   sizeP = QSizePolicy(hPolicy, vPolicy);
}

QWidgetItem::~QWidgetItem()
{
}
QLayoutItem::~QLayoutItem()
{
}

void QLayoutItem::invalidate()
{
}

QLayout *QLayoutItem::layout()
{
   return 0;
}

QSpacerItem *QLayoutItem::spacerItem()
{
   return 0;
}

QLayout *QLayout::layout()
{
   return this;
}

QSpacerItem *QSpacerItem::spacerItem()
{
   return this;
}

QWidget *QLayoutItem::widget()
{
   return 0;
}

QWidget *QWidgetItem::widget()
{
   return wid;
}

bool QLayoutItem::hasHeightForWidth() const
{
   return false;
}

int QLayoutItem::minimumHeightForWidth(int w) const
{
   return heightForWidth(w);
}

int QLayoutItem::heightForWidth(int /* w */) const
{
   return -1;
}

QSizePolicy::ControlTypes QLayoutItem::controlTypes() const
{
   return QSizePolicy::DefaultType;
}

void QSpacerItem::setGeometry(const QRect &r)
{
   rect = r;
}

void QWidgetItem::setGeometry(const QRect &rect)
{
   if (isEmpty()) {
      return;
   }

   QRect r = ! wid->testAttribute(Qt::WA_LayoutUsesWidgetRect)
      ? fromLayoutItemRect(wid->d_func(), rect) : rect;

   const QSize widgetRectSurplus = r.size() - rect.size();

   /*
      this code is done using widget rect coordinates, not layout item rect coordinates.
      However, QWidgetItem's sizeHint(), maximumSize(), and heightForWidth() all work in
      terms of layout item rect coordinates. Therefore, we have to add or subtract
      widgetRectSurplus here and there. The code could be simpler if we did everything
      using layout item rect coordinates and did the conversion right before the call to QWidget::setGeometry().
    */

   QSize s = r.size().boundedTo(maximumSize() + widgetRectSurplus);
   int x   = r.x();
   int y   = r.y();


   if (align & (Qt::AlignHorizontal_Mask | Qt::AlignVertical_Mask)) {
      QSize pref(sizeHint());
      QSizePolicy sp = wid->sizePolicy();

      if (sp.horizontalPolicy() == QSizePolicy::Ignored) {
         pref.setWidth(wid->sizeHint().expandedTo(wid->minimumSize()).width());
      }

      if (sp.verticalPolicy() == QSizePolicy::Ignored) {
         pref.setHeight(wid->sizeHint().expandedTo(wid->minimumSize()).height());
      }

      pref += widgetRectSurplus;
      if (align & Qt::AlignHorizontal_Mask) {
         s.setWidth(qMin(s.width(), pref.width()));
      }

      if (align & Qt::AlignVertical_Mask) {
         if (hasHeightForWidth())
            s.setHeight(qMin(s.height(), heightForWidth(s.width() - widgetRectSurplus.width())
                  + widgetRectSurplus.height()));
         else {
            s.setHeight(qMin(s.height(), pref.height()));
         }
      }
   }

   Qt::Alignment alignHoriz = QStyle::visualAlignment(wid->layoutDirection(), align);

   if (alignHoriz & Qt::AlignRight) {
      x = x + (r.width() - s.width());

   } else if (!(alignHoriz & Qt::AlignLeft)) {
      x = x + (r.width() - s.width()) / 2;

   }

   if (align & Qt::AlignBottom) {
      y = y + (r.height() - s.height());

   } else if (!(align & Qt::AlignTop)) {
      y = y + (r.height() - s.height()) / 2;
   }

   wid->setGeometry(x, y, s.width(), s.height());
}

QRect QSpacerItem::geometry() const
{
   return rect;
}

QRect QWidgetItem::geometry() const
{
   return ! wid->testAttribute(Qt::WA_LayoutUsesWidgetRect)
      ? toLayoutItemRect(wid->d_func(), wid->geometry()) : wid->geometry();
}

bool QWidgetItem::hasHeightForWidth() const
{
   if (isEmpty()) {
      return false;
   }

   return wid->hasHeightForWidth();
}

int QWidgetItem::heightForWidth(int w) const
{
   if (isEmpty()) {
      return -1;
   }

   w = ! wid->testAttribute(Qt::WA_LayoutUsesWidgetRect)
      ? fromLayoutItemSize(wid->d_func(), QSize(w, 0)).width() : w;

   int hfw;
   if (wid->layout()) {
      hfw = wid->layout()->totalHeightForWidth(w);
   } else {
      hfw = wid->heightForWidth(w);
   }

   if (hfw > wid->maximumHeight()) {
      hfw = wid->maximumHeight();
   }
   if (hfw < wid->minimumHeight()) {
      hfw = wid->minimumHeight();
   }

   hfw = !wid->testAttribute(Qt::WA_LayoutUsesWidgetRect)
      ? toLayoutItemSize(wid->d_func(), QSize(0, hfw)).height()
      : hfw;

   if (hfw < 0) {
      hfw = 0;
   }
   return hfw;
}

/*!
    \reimp
*/
Qt::Orientations QSpacerItem::expandingDirections() const
{
   return sizeP.expandingDirections();
}

/*!
    \reimp
*/
Qt::Orientations QWidgetItem::expandingDirections() const
{
   if (isEmpty()) {
      return Qt::Orientations(0);
   }

   Qt::Orientations e = wid->sizePolicy().expandingDirections();

   /*
     ### Qt 4.0:
     If the layout is expanding, we make the widget expanding, even if
     its own size policy isn't expanding. This behavior should be reconsidered.
   */

   if (wid->layout()) {
      if (wid->sizePolicy().horizontalPolicy() & QSizePolicy::GrowFlag
         && (wid->layout()->expandingDirections() & Qt::Horizontal)) {
         e |= Qt::Horizontal;
      }
      if (wid->sizePolicy().verticalPolicy() & QSizePolicy::GrowFlag
         && (wid->layout()->expandingDirections() & Qt::Vertical)) {
         e |= Qt::Vertical;
      }
   }

   if (align & Qt::AlignHorizontal_Mask) {
      e &= ~Qt::Horizontal;
   }

   if (align & Qt::AlignVertical_Mask) {
      e &= ~Qt::Vertical;
   }

   return e;
}

/*!
    \reimp
*/
QSize QSpacerItem::minimumSize() const
{
   return QSize(sizeP.horizontalPolicy() & QSizePolicy::ShrinkFlag ? 0 : width,
         sizeP.verticalPolicy() & QSizePolicy::ShrinkFlag ? 0 : height);
}

/*!
    \reimp
*/
QSize QWidgetItem::minimumSize() const
{
   if (isEmpty()) {
      return QSize(0, 0);
   }

   return ! wid->testAttribute(Qt::WA_LayoutUsesWidgetRect)
      ? toLayoutItemSize(wid->d_func(), qSmartMinSize(this))
      : qSmartMinSize(this);
}

/*!
    \reimp
*/
QSize QSpacerItem::maximumSize() const
{
   return QSize(sizeP.horizontalPolicy() & QSizePolicy::GrowFlag ? QLAYOUTSIZE_MAX : width,
         sizeP.verticalPolicy() & QSizePolicy::GrowFlag ? QLAYOUTSIZE_MAX : height);
}

/*!
    \reimp
*/
QSize QWidgetItem::maximumSize() const
{
   if (isEmpty()) {
      return QSize(0, 0);

   } else {
      return !wid->testAttribute(Qt::WA_LayoutUsesWidgetRect)
         ? toLayoutItemSize(wid->d_func(), qSmartMaxSize(this, align))
         : qSmartMaxSize(this, align);
   }
}

/*!
    \reimp
*/
QSize QSpacerItem::sizeHint() const
{
   return QSize(width, height);
}

QSize QWidgetItem::sizeHint() const
{
   QSize s(0, 0);

   if (!isEmpty()) {
      s = wid->sizeHint().expandedTo(wid->minimumSizeHint());
      s = s.boundedTo(wid->maximumSize()).expandedTo(wid->minimumSize());

      s = ! wid->testAttribute(Qt::WA_LayoutUsesWidgetRect)
         ? toLayoutItemSize(wid->d_func(), s)
         : s;

      if (wid->sizePolicy().horizontalPolicy() == QSizePolicy::Ignored) {
         s.setWidth(0);
      }

      if (wid->sizePolicy().verticalPolicy() == QSizePolicy::Ignored) {
         s.setHeight(0);
      }
   }
   return s;
}

bool QSpacerItem::isEmpty() const
{
   return true;
}

bool QWidgetItem::isEmpty() const
{
   return (wid->isHidden() && !wid->sizePolicy().retainSizeWhenHidden()) || wid->isWindow();
}


QSizePolicy::ControlTypes QWidgetItem::controlTypes() const
{
   return wid->sizePolicy().controlType();
}

QWidgetItemV2::QWidgetItemV2(QWidget *widget)
   : QWidgetItem(widget),
     q_cachedMinimumSize(Dirty, Dirty),
     q_cachedSizeHint(Dirty, Dirty),
     q_cachedMaximumSize(Dirty, Dirty),
     q_firstCachedHfw(0),
     q_hfwCacheSize(0),
     d(0)
{
   QWidgetPrivate *wd = wid->d_func();
   if (!wd->widgetItem) {
      wd->widgetItem = this;
   }
}

QWidgetItemV2::~QWidgetItemV2()
{
   if (wid) {
      QWidgetPrivate *wd = wid->d_func();
      if (wd->widgetItem == this) {
         wd->widgetItem = 0;
      }
   }
}

inline bool QWidgetItemV2::useSizeCache() const
{
   return wid->d_func()->widgetItem == this;
}

void QWidgetItemV2::updateCacheIfNecessary() const
{
   if (q_cachedMinimumSize.width() != Dirty) {
      return;
   }

   const QSize sizeHint(wid->sizeHint());
   const QSize minimumSizeHint(wid->minimumSizeHint());
   const QSize minimumSize(wid->minimumSize());
   const QSize maximumSize(wid->maximumSize());
   const QSizePolicy sizePolicy(wid->sizePolicy());
   const QSize expandedSizeHint(sizeHint.expandedTo(minimumSizeHint));

   const QSize smartMinSize(qSmartMinSize(sizeHint, minimumSizeHint, minimumSize, maximumSize, sizePolicy));
   const QSize smartMaxSize(qSmartMaxSize(expandedSizeHint, minimumSize, maximumSize, sizePolicy, align));

   const bool useLayoutItemRect = !wid->testAttribute(Qt::WA_LayoutUsesWidgetRect);

   q_cachedMinimumSize = useLayoutItemRect
      ? toLayoutItemSize(wid->d_func(), smartMinSize)
      : smartMinSize;

   q_cachedSizeHint = expandedSizeHint;
   q_cachedSizeHint = q_cachedSizeHint.boundedTo(maximumSize).expandedTo(minimumSize);

   q_cachedSizeHint = useLayoutItemRect
      ? toLayoutItemSize(wid->d_func(), q_cachedSizeHint)
      : q_cachedSizeHint;
   if (wid->sizePolicy().horizontalPolicy() == QSizePolicy::Ignored) {
      q_cachedSizeHint.setWidth(0);
   }

   if (wid->sizePolicy().verticalPolicy() == QSizePolicy::Ignored) {
      q_cachedSizeHint.setHeight(0);
   }

   q_cachedMaximumSize = useLayoutItemRect
      ? toLayoutItemSize(wid->d_func(), smartMaxSize)
      : smartMaxSize;
}

QSize QWidgetItemV2::sizeHint() const
{

   if (isEmpty()) {
      return QSize(0, 0);
   }

   if (useSizeCache()) {
      updateCacheIfNecessary();
      return q_cachedSizeHint;

   } else {
      return QWidgetItem::sizeHint();

   }
}

QSize QWidgetItemV2::minimumSize() const
{
   if (isEmpty()) {
      return QSize(0, 0);
   }

   if (useSizeCache()) {
      updateCacheIfNecessary();
      return q_cachedMinimumSize;

   } else {
      return QWidgetItem::minimumSize();

   }
}

QSize QWidgetItemV2::maximumSize() const
{
   if (isEmpty()) {
      return QSize(0, 0);
   }

   if (useSizeCache()) {
      updateCacheIfNecessary();
      return q_cachedMaximumSize;

   } else {
      return QWidgetItem::maximumSize();

   }
}

/*
    The height-for-width cache is organized as a circular buffer. The entries

        q_hfwCachedHfws[q_firstCachedHfw],
        ...,
        q_hfwCachedHfws[(q_firstCachedHfw + q_hfwCacheSize - 1) % HfwCacheMaxSize]

    contain the last cached values. When the cache is full, the first entry to
    be erased is the entry before q_hfwCachedHfws[q_firstCachedHfw]. When
    values are looked up, we try to move q_firstCachedHfw to point to that new
    entry (unless the cache is not full, in which case it would leave the cache
    in a broken state), so that the most recently used entry is also the last
    to be erased.
*/

int QWidgetItemV2::heightForWidth(int width) const
{
   if (isEmpty()) {
      return -1;
   }

   for (int i = 0; i < q_hfwCacheSize; ++i) {
      int offset = q_firstCachedHfw + i;
      const QSize &size = q_cachedHfws[offset % HfwCacheMaxSize];

      if (size.width() == width) {
         if (q_hfwCacheSize == HfwCacheMaxSize) {
            q_firstCachedHfw = offset;
         }

         return size.height();
      }
   }

   if (q_hfwCacheSize < HfwCacheMaxSize) {
      ++q_hfwCacheSize;
   }

   q_firstCachedHfw = (q_firstCachedHfw + HfwCacheMaxSize - 1) % HfwCacheMaxSize;

   int height = QWidgetItem::heightForWidth(width);
   q_cachedHfws[q_firstCachedHfw] = QSize(width, height);
   return height;
}

