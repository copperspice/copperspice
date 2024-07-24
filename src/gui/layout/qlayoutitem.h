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

#ifndef QLAYOUTITEM_H
#define QLAYOUTITEM_H

#include <qrect.h>
#include <qsizepolicy.h>

class QLayout;
class QLayoutItem;
class QSize;
class QSpacerItem;
class QWidget;

#include <limits.h>

static constexpr const int QLAYOUTSIZE_MAX = INT_MAX / 256 / 16;

class Q_GUI_EXPORT QLayoutItem
{
 public:
   inline explicit QLayoutItem(Qt::Alignment alignment = Qt::Alignment());

   virtual ~QLayoutItem();

   virtual QSize sizeHint() const = 0;
   virtual QSize minimumSize() const = 0;
   virtual QSize maximumSize() const = 0;
   virtual Qt::Orientations expandingDirections() const = 0;
   virtual void setGeometry(const QRect &rect) = 0;
   virtual QRect geometry() const = 0;
   virtual bool isEmpty() const = 0;
   virtual bool hasHeightForWidth() const;
   virtual int heightForWidth(int width) const;
   virtual int minimumHeightForWidth(int width) const;
   virtual void invalidate();

   virtual QWidget *widget();
   virtual QLayout *layout();
   virtual QSpacerItem *spacerItem();

   Qt::Alignment alignment() const {
      return align;
   }

   void setAlignment(Qt::Alignment alignment);
   virtual QSizePolicy::ControlTypes controlTypes() const;

 protected:
   Qt::Alignment align;
};

inline QLayoutItem::QLayoutItem(Qt::Alignment alignment)
   : align(alignment)
{
}

class Q_GUI_EXPORT QSpacerItem : public QLayoutItem
{

 public:
   QSpacerItem(int w, int h, QSizePolicy::Policy hPolicy = QSizePolicy::Minimum,
      QSizePolicy::Policy vPolicy = QSizePolicy::Minimum)
      : width(w), height(h), sizeP(hPolicy, vPolicy)
   {
   }

   ~QSpacerItem();

   void changeSize(int w, int h, QSizePolicy::Policy hPolicy = QSizePolicy::Minimum,
      QSizePolicy::Policy vPolicy = QSizePolicy::Minimum);

   QSize sizeHint() const override;
   QSize minimumSize() const override;
   QSize maximumSize() const override;
   Qt::Orientations expandingDirections() const override;
   bool isEmpty() const override;
   void setGeometry(const QRect &rect) override;
   QRect geometry() const override;
   QSpacerItem *spacerItem() override;

   QSizePolicy sizePolicy() const {
      return sizeP;
   }

 private:
   int width;
   int height;
   QSizePolicy sizeP;
   QRect rect;
};

class Q_GUI_EXPORT QWidgetItem : public QLayoutItem
{
 public:
   explicit QWidgetItem(QWidget *widget)
      : wid(widget) {
   }

   QWidgetItem(const QWidgetItem &) = delete;
   QWidgetItem &operator=(const QWidgetItem &) = delete;

   ~QWidgetItem();

   QSize sizeHint() const override;
   QSize minimumSize() const override;
   QSize maximumSize() const override;
   Qt::Orientations expandingDirections() const override;
   bool isEmpty() const override;
   void setGeometry(const QRect &rect) override;
   QRect geometry() const override;
   QWidget *widget() override;

   bool hasHeightForWidth() const override;
   int heightForWidth(int width) const override;

   QSizePolicy::ControlTypes controlTypes() const override;

 protected:
   QWidget *wid;
};

class Q_GUI_EXPORT QWidgetItemV2 : public QWidgetItem
{
 public:
   explicit QWidgetItemV2(QWidget *widget);

   QWidgetItemV2(const QWidgetItemV2 &) = delete;
   QWidgetItemV2 &operator=(const QWidgetItemV2 &) = delete;

   ~QWidgetItemV2();

   QSize sizeHint() const override;
   QSize minimumSize() const override;
   QSize maximumSize() const override;
   int heightForWidth(int width) const override;

 private:
   static constexpr const int ItemDirty    = -123;
   static constexpr const int SizeCacheMax = 3;

   inline bool useSizeCache() const;
   void updateCacheIfNecessary() const;

   void invalidateSizeCache() {
      q_cachedMinimumSize.setWidth(ItemDirty);
      q_hfwCacheSize = 0;
   }

   mutable QSize q_cachedMinimumSize;
   mutable QSize q_cachedSizeHint;
   mutable QSize q_cachedMaximumSize;
   mutable QSize q_cachedHfws[SizeCacheMax];
   mutable short q_firstCachedHfw;
   mutable short q_hfwCacheSize;
   void *d;

   friend class QWidgetPrivate;
};

#endif // QLAYOUTITEM_H
