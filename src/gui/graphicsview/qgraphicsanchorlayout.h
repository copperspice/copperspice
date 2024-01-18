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

#ifndef QGRAPHICSANCHORLAYOUT_H
#define QGRAPHICSANCHORLAYOUT_H

#include <qgraphicsitem.h>
#include <qgraphicslayout.h>
#include <qscopedpointer.h>

#if ! defined(QT_NO_GRAPHICSVIEW)

class QGraphicsAnchorPrivate;
class QGraphicsAnchorLayout;
class QGraphicsAnchorLayoutPrivate;

class Q_GUI_EXPORT QGraphicsAnchor : public QObject
{
   GUI_CS_OBJECT(QGraphicsAnchor)

   GUI_CS_PROPERTY_READ(spacing, spacing)
   GUI_CS_PROPERTY_WRITE(spacing, setSpacing)
   GUI_CS_PROPERTY_RESET(spacing, unsetSpacing)

   GUI_CS_PROPERTY_READ(sizePolicy, sizePolicy)
   GUI_CS_PROPERTY_WRITE(sizePolicy, setSizePolicy)

 public:
   void setSpacing(qreal spacing);
   void unsetSpacing();
   qreal spacing() const;

   void setSizePolicy(QSizePolicy::Policy policy);
   QSizePolicy::Policy sizePolicy() const;

   ~QGraphicsAnchor();

 private:
   QGraphicsAnchor(QGraphicsAnchorLayout *parent);
   Q_DECLARE_PRIVATE(QGraphicsAnchor)

   friend class QGraphicsAnchorLayoutPrivate;
   friend struct AnchorData;

 protected:
   QScopedPointer<QGraphicsAnchorPrivate> d_ptr;
};

class Q_GUI_EXPORT QGraphicsAnchorLayout : public QGraphicsLayout
{
 public:
   QGraphicsAnchorLayout(QGraphicsLayoutItem *parent = nullptr);

   QGraphicsAnchorLayout(const QGraphicsAnchorLayout &) = delete;
   QGraphicsAnchorLayout &operator=(const QGraphicsAnchorLayout &) = delete;

   virtual ~QGraphicsAnchorLayout();

   QGraphicsAnchor *addAnchor(QGraphicsLayoutItem *firstItem, Qt::AnchorPoint firstEdge,
      QGraphicsLayoutItem *secondItem, Qt::AnchorPoint secondEdge);

   QGraphicsAnchor *anchor(QGraphicsLayoutItem *firstItem, Qt::AnchorPoint firstEdge,
      QGraphicsLayoutItem *secondItem, Qt::AnchorPoint secondEdge);

   void addCornerAnchors(QGraphicsLayoutItem *firstItem, Qt::Corner firstCorner,
      QGraphicsLayoutItem *secondItem, Qt::Corner secondCorner);

   void addAnchors(QGraphicsLayoutItem *firstItem, QGraphicsLayoutItem *secondItem,
      Qt::Orientations orientations = Qt::Horizontal | Qt::Vertical);

   void setHorizontalSpacing(qreal spacing);
   void setVerticalSpacing(qreal spacing);
   void setSpacing(qreal spacing);
   qreal horizontalSpacing() const;
   qreal verticalSpacing() const;

   void removeAt(int index) override;
   void setGeometry(const QRectF &rectF) override;
   int count() const override;
   QGraphicsLayoutItem *itemAt(int index) const override;

   void invalidate() override;

 protected:
   QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const override;

 private:
   Q_DECLARE_PRIVATE(QGraphicsAnchorLayout)

   friend class QGraphicsAnchor;
};

#endif

#endif
