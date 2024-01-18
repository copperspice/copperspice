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

#ifndef QBOXLAYOUT_H
#define QBOXLAYOUT_H

#include <qlayout.h>
#include <limits.h>

class QBoxLayoutPrivate;

class Q_GUI_EXPORT QBoxLayout : public QLayout
{
   GUI_CS_OBJECT(QBoxLayout)

 public:
   enum Direction {
      LeftToRight, RightToLeft, TopToBottom, BottomToTop,
      Down = TopToBottom, Up = BottomToTop
   };

   explicit QBoxLayout(Direction direction, QWidget *parent = nullptr);

   QBoxLayout(const QBoxLayout &) = delete;
   QBoxLayout &operator=(const QBoxLayout &) = delete;

   ~QBoxLayout();

   Direction direction() const;
   void setDirection(Direction direction);

   void addSpacing(int size);
   void addStretch(int stretch = 0);
   void addSpacerItem(QSpacerItem *spacerItem);
   void addWidget(QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
   void addLayout(QLayout *layout, int stretch = 0);
   void addStrut(int size);
   void addItem(QLayoutItem *item) override;

   void insertSpacing(int index, int size);
   void insertStretch(int index, int stretch = 0);
   void insertSpacerItem(int index, QSpacerItem *spacerItem);
   void insertWidget(int index, QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
   void insertLayout(int index, QLayout *layout, int stretch = 0);
   void insertItem(int index, QLayoutItem *item);

   int spacing() const;
   void setSpacing(int spacing);

   bool setStretchFactor(QWidget *widget, int stretch);
   bool setStretchFactor(QLayout *layout, int stretch);
   void setStretch(int index, int stretch);
   int stretch(int index) const;

   QSize sizeHint() const override;
   QSize minimumSize() const override;
   QSize maximumSize() const override;

   bool hasHeightForWidth() const override;
   int heightForWidth(int width) const override;
   int minimumHeightForWidth(int width) const override;

   Qt::Orientations expandingDirections() const override;
   void invalidate() override;
   QLayoutItem *itemAt(int index) const override;
   QLayoutItem *takeAt(int index) override;
   int count() const override;
   void setGeometry(const QRect &rect) override;

 private:
   Q_DECLARE_PRIVATE(QBoxLayout)
};

class Q_GUI_EXPORT QHBoxLayout : public QBoxLayout
{
   GUI_CS_OBJECT(QHBoxLayout)

 public:
   QHBoxLayout();
   explicit QHBoxLayout(QWidget *parent);

   QHBoxLayout(const QHBoxLayout &) = delete;
   QHBoxLayout &operator=(const QHBoxLayout &) = delete;

   ~QHBoxLayout();
};

class Q_GUI_EXPORT QVBoxLayout : public QBoxLayout
{
   GUI_CS_OBJECT(QVBoxLayout)

 public:
   QVBoxLayout();
   explicit QVBoxLayout(QWidget *parent);

   QVBoxLayout(const QVBoxLayout &) = delete;
   QVBoxLayout &operator=(const QVBoxLayout &) = delete;

   ~QVBoxLayout();
};

#endif
