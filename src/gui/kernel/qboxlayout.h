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

#ifndef QBOXLAYOUT_H
#define QBOXLAYOUT_H

#include <QtGui/qlayout.h>
#include <limits.h>

QT_BEGIN_NAMESPACE

class QBoxLayoutPrivate;

class Q_GUI_EXPORT QBoxLayout : public QLayout
{
   GUI_CS_OBJECT(QBoxLayout)
   Q_DECLARE_PRIVATE(QBoxLayout)

 public:
   enum Direction { LeftToRight, RightToLeft, TopToBottom, BottomToTop,
                    Down = TopToBottom, Up = BottomToTop
                  };

   explicit QBoxLayout(Direction, QWidget *parent = nullptr);
   ~QBoxLayout();

   Direction direction() const;
   void setDirection(Direction);

   void addSpacing(int size);
   void addStretch(int stretch = 0);
   void addSpacerItem(QSpacerItem *spacerItem);
   void addWidget(QWidget *, int stretch = 0, Qt::Alignment alignment = 0);
   void addLayout(QLayout *layout, int stretch = 0);
   void addStrut(int);
   void addItem(QLayoutItem *) override;

   void insertSpacing(int index, int size);
   void insertStretch(int index, int stretch = 0);
   void insertSpacerItem(int index, QSpacerItem *spacerItem);
   void insertWidget(int index, QWidget *widget, int stretch = 0, Qt::Alignment alignment = 0);
   void insertLayout(int index, QLayout *layout, int stretch = 0);

   int spacing() const;
   void setSpacing(int spacing);

   bool setStretchFactor(QWidget *w, int stretch);
   bool setStretchFactor(QLayout *l, int stretch);
   void setStretch(int index, int stretch);
   int stretch(int index) const;

   QSize sizeHint() const override;
   QSize minimumSize() const override;
   QSize maximumSize() const override;

   bool hasHeightForWidth() const override;
   int heightForWidth(int) const override;
   int minimumHeightForWidth(int) const override;

   Qt::Orientations expandingDirections() const override;
   void invalidate() override;
   QLayoutItem *itemAt(int) const override;
   QLayoutItem *takeAt(int) override;
   int count() const override;
   void setGeometry(const QRect &) override;

   void insertItem(int index, QLayoutItem *);

 private:
   Q_DISABLE_COPY(QBoxLayout)
};

class Q_GUI_EXPORT QHBoxLayout : public QBoxLayout
{
   GUI_CS_OBJECT(QHBoxLayout)

 public:
   QHBoxLayout();
   explicit QHBoxLayout(QWidget *parent);
   ~QHBoxLayout();

 private:
   Q_DISABLE_COPY(QHBoxLayout)
};

class Q_GUI_EXPORT QVBoxLayout : public QBoxLayout
{
   GUI_CS_OBJECT(QVBoxLayout)

 public:
   QVBoxLayout();
   explicit QVBoxLayout(QWidget *parent);
   ~QVBoxLayout();

 private:
   Q_DISABLE_COPY(QVBoxLayout)
};

QT_END_NAMESPACE

#endif // QBOXLAYOUT_H
