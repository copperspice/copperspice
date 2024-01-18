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

#ifndef QLAYOUT_H
#define QLAYOUT_H

#include <qobject.h>
#include <qlayoutitem.h>
#include <qsizepolicy.h>
#include <qrect.h>
#include <qmargins.h>
#include <qscopedpointer.h>

#include <limits.h>

class QLayout;
class QSize;
class QLayoutPrivate;

class Q_GUI_EXPORT QLayout : public QObject, public QLayoutItem
{
   GUI_CS_OBJECT_MULTIPLE(QLayout, QObject)

   GUI_CS_ENUM(SizeConstraint)

   GUI_CS_PROPERTY_READ(margin, margin)
   GUI_CS_PROPERTY_WRITE(margin, setMargin)

   GUI_CS_PROPERTY_READ(spacing, spacing)
   GUI_CS_PROPERTY_WRITE(spacing, setSpacing)

   GUI_CS_PROPERTY_READ(sizeConstraint, sizeConstraint)
   GUI_CS_PROPERTY_WRITE(sizeConstraint, setSizeConstraint)

 public:
   using QWidgetItemFactory = QWidgetItem * (*)(const QLayout *layout, QWidget *widget);

   using QSpacerItemFactory = QSpacerItem * (*)(const QLayout *layout, int w, int h,
         QSizePolicy::Policy hPolicy, QSizePolicy::Policy);

   GUI_CS_REGISTER_ENUM(
      enum SizeConstraint {
         SetDefaultConstraint,
         SetNoConstraint,
         SetMinimumSize,
         SetFixedSize,
         SetMaximumSize,
         SetMinAndMaxSize
      };
   )

   QLayout(QWidget *parent);
   QLayout();

   QLayout(const QLayout &) = delete;
   QLayout &operator=(const QLayout &) = delete;

   ~QLayout();

   int margin() const;
   int spacing() const;

   void setMargin(int margin);
   void setSpacing(int spacing);

   void setContentsMargins(int left, int top, int right, int bottom);
   void setContentsMargins(const QMargins &margins);
   void getContentsMargins(int *left, int *top, int *right, int *bottom) const;
   QMargins contentsMargins() const;
   QRect contentsRect() const;

   bool setAlignment(QWidget *widget, Qt::Alignment alignment);
   bool setAlignment(QLayout *layout, Qt::Alignment alignment);

   using QLayoutItem::setAlignment;

   void setSizeConstraint(SizeConstraint constraint);
   SizeConstraint sizeConstraint() const;

   void setMenuBar(QWidget *widget);
   QWidget *menuBar() const;

   QWidget *parentWidget() const;

   void invalidate() override;
   QRect geometry() const override;
   bool activate();
   void update();

   void addWidget(QWidget *w);
   virtual void addItem(QLayoutItem *item) = 0;

   void removeWidget(QWidget *widget);
   void removeItem(QLayoutItem *item);

   Qt::Orientations expandingDirections() const override;
   QSize minimumSize() const override;
   QSize maximumSize() const override;
   void setGeometry(const QRect &rect) override;

   virtual QLayoutItem *itemAt(int index) const = 0;
   virtual QLayoutItem *takeAt(int index) = 0;
   virtual int indexOf(QWidget *widget) const;
   virtual int count() const = 0;

   bool isEmpty() const override;
   QSizePolicy::ControlTypes controlTypes() const override;

   virtual QLayoutItem *replaceWidget(QWidget *from, QWidget *to,
      Qt::FindChildOptions options = Qt::FindChildrenRecursively);

   int totalHeightForWidth(int width) const;
   QSize totalMinimumSize() const;
   QSize totalMaximumSize() const;
   QSize totalSizeHint() const;
   QLayout *layout() override;

   void setEnabled(bool enable);
   bool isEnabled() const;

   static QSize closestAcceptableSize(const QWidget *widget, const QSize &size);

   static void setWidgetItemFactory(QWidgetItemFactory factory);
   static QWidgetItemFactory getWidgetItemFactory();

 protected:
   void widgetEvent(QEvent *event);
   void childEvent(QChildEvent *event) override;
   void addChildLayout(QLayout *layout);
   void addChildWidget(QWidget *widget);
   bool adoptLayout(QLayout *layout);

   QRect alignmentRect(const QRect &rect) const;
   QLayout(QLayoutPrivate &d, QLayout *layout, QWidget *widget);

   QScopedPointer<QLayoutPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QLayout)
   static void activateRecursiveHelper(QLayoutItem *item);

   friend class QApplicationPrivate;
   friend class QWidget;
};

//emerald - support old includes
#include <qboxlayout.h>
#include <qgridlayout.h>

#endif
