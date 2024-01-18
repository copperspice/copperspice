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

#ifndef QGRAPHICSGRIDLAYOUT_H
#define QGRAPHICSGRIDLAYOUT_H

#include <qgraphicsitem.h>
#include <qgraphicslayout.h>

#if ! defined(QT_NO_GRAPHICSVIEW)

class QGraphicsGridLayoutPrivate;

class Q_GUI_EXPORT QGraphicsGridLayout : public QGraphicsLayout
{
 public:
   QGraphicsGridLayout(QGraphicsLayoutItem *parent = nullptr);

   QGraphicsGridLayout(const QGraphicsGridLayout &) = delete;
   QGraphicsGridLayout &operator=(const QGraphicsGridLayout &) = delete;

   virtual ~QGraphicsGridLayout();

   void addItem(QGraphicsLayoutItem *item, int row, int column, int rowSpan, int columnSpan, Qt::Alignment alignment = Qt::Alignment());
   inline void addItem(QGraphicsLayoutItem *item, int row, int column, Qt::Alignment alignment = Qt::Alignment());

   void setHorizontalSpacing(qreal spacing);
   qreal horizontalSpacing() const;
   void setVerticalSpacing(qreal spacing);
   qreal verticalSpacing() const;
   void setSpacing(qreal spacing);

   void setRowSpacing(int row, qreal spacing);
   qreal rowSpacing(int row) const;
   void setColumnSpacing(int column, qreal spacing);
   qreal columnSpacing(int column) const;

   void setRowStretchFactor(int row, int stretch);
   int rowStretchFactor(int row) const;
   void setColumnStretchFactor(int column, int stretch);
   int columnStretchFactor(int column) const;

   void setRowMinimumHeight(int row, qreal height);
   qreal rowMinimumHeight(int row) const;
   void setRowPreferredHeight(int row, qreal height);
   qreal rowPreferredHeight(int row) const;
   void setRowMaximumHeight(int row, qreal height);
   qreal rowMaximumHeight(int row) const;
   void setRowFixedHeight(int row, qreal height);

   void setColumnMinimumWidth(int column, qreal width);
   qreal columnMinimumWidth(int column) const;
   void setColumnPreferredWidth(int column, qreal width);
   qreal columnPreferredWidth(int column) const;
   void setColumnMaximumWidth(int column, qreal width);
   qreal columnMaximumWidth(int column) const;
   void setColumnFixedWidth(int column, qreal width);

   void setRowAlignment(int row, Qt::Alignment alignment);
   Qt::Alignment rowAlignment(int row) const;
   void setColumnAlignment(int column, Qt::Alignment alignment);
   Qt::Alignment columnAlignment(int column) const;

   void setAlignment(QGraphicsLayoutItem *item, Qt::Alignment alignment);
   Qt::Alignment alignment(QGraphicsLayoutItem *item) const;

   int rowCount() const;
   int columnCount() const;

   QGraphicsLayoutItem *itemAt(int row, int column) const;

   // inherited from QGraphicsLayout
   int count() const override;
   QGraphicsLayoutItem *itemAt(int index) const override;
   void removeAt(int index) override;
   void removeItem(QGraphicsLayoutItem *item);

   void invalidate() override;

   // inherited from QGraphicsLayoutItem
   void setGeometry(const QRectF &rect) override;
   QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const override;

   // ####
   // QRect cellRect(int row, int column, int rowSpan = 1, int columnSpan = 1) const;
   // QSizePolicy::ControlTypes controlTypes(LayoutSide side) const;

 private:
   Q_DECLARE_PRIVATE(QGraphicsGridLayout)
};

inline void QGraphicsGridLayout::addItem(QGraphicsLayoutItem *item, int row, int column, Qt::Alignment alignment)
{
   addItem(item, row, column, 1, 1, alignment);
}

#endif

#endif

