/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QLAYOUT_H
#define QLAYOUT_H

#include <QtCore/qobject.h>
#include <QtGui/qlayoutitem.h>
#include <QtGui/qsizepolicy.h>
#include <QtCore/qrect.h>
#include <QtCore/qmargins.h>
#include <QScopedPointer>

#include <limits.h>

QT_BEGIN_NAMESPACE

class QLayout;
class QSize;
class QLayoutPrivate;

class Q_GUI_EXPORT QLayout : public QObject, public QLayoutItem
{
   GUI_CS_OBJECT_MULTIPLE(QLayout, QObject)
   Q_DECLARE_PRIVATE(QLayout)

   GUI_CS_ENUM(SizeConstraint)
   GUI_CS_PROPERTY_READ(margin, margin)
   GUI_CS_PROPERTY_WRITE(margin, setMargin)
   GUI_CS_PROPERTY_READ(spacing, spacing)
   GUI_CS_PROPERTY_WRITE(spacing, setSpacing)
   GUI_CS_PROPERTY_READ(sizeConstraint, sizeConstraint)
   GUI_CS_PROPERTY_WRITE(sizeConstraint, setSizeConstraint)

 public:
   enum SizeConstraint {
      SetDefaultConstraint,
      SetNoConstraint,
      SetMinimumSize,
      SetFixedSize,
      SetMaximumSize,
      SetMinAndMaxSize
   };

   QLayout(QWidget *parent);
   QLayout();
   ~QLayout();

   int margin() const;
   int spacing() const;

   void setMargin(int);
   void setSpacing(int);

   void setContentsMargins(int left, int top, int right, int bottom);
   void setContentsMargins(const QMargins &margins);
   void getContentsMargins(int *left, int *top, int *right, int *bottom) const;
   QMargins contentsMargins() const;
   QRect contentsRect() const;

   bool setAlignment(QWidget *w, Qt::Alignment alignment);
   bool setAlignment(QLayout *l, Qt::Alignment alignment);

   using QLayoutItem::setAlignment;

   void setSizeConstraint(SizeConstraint);
   SizeConstraint sizeConstraint() const;

   void setMenuBar(QWidget *w);
   QWidget *menuBar() const;

   QWidget *parentWidget() const;

   void invalidate() override;
   QRect geometry() const override;
   bool activate();
   void update();

   void addWidget(QWidget *w);
   virtual void addItem(QLayoutItem *) = 0;

   void removeWidget(QWidget *w);
   void removeItem(QLayoutItem *);

   Qt::Orientations expandingDirections() const override;
   QSize minimumSize() const override;
   QSize maximumSize() const override;
   void setGeometry(const QRect &) override;

   virtual QLayoutItem *itemAt(int index) const = 0;
   virtual QLayoutItem *takeAt(int index) = 0;
   virtual int indexOf(QWidget *) const;
   virtual int count() const = 0;

   bool isEmpty() const override;

   int totalHeightForWidth(int w) const;
   QSize totalMinimumSize() const;
   QSize totalMaximumSize() const;
   QSize totalSizeHint() const;
   QLayout *layout() override;

   void setEnabled(bool);
   bool isEnabled() const;

   static QSize closestAcceptableSize(const QWidget *w, const QSize &s);

 protected:
   void widgetEvent(QEvent *);
   void childEvent(QChildEvent *e) override;
   void addChildLayout(QLayout *l);
   void addChildWidget(QWidget *w);
   bool adoptLayout(QLayout *layout);

   QRect alignmentRect(const QRect &) const;
   QLayout(QLayoutPrivate &d, QLayout *, QWidget *);

   QScopedPointer<QLayoutPrivate> d_ptr;

 private:
   Q_DISABLE_COPY(QLayout)

   static void activateRecursiveHelper(QLayoutItem *item);

   friend class QApplicationPrivate;
   friend class QWidget;
};

QT_BEGIN_INCLUDE_NAMESPACE
#include <QtGui/qboxlayout.h>
#include <QtGui/qgridlayout.h>
QT_END_INCLUDE_NAMESPACE

QT_END_NAMESPACE

#endif // QLAYOUT_H
