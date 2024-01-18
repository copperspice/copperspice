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

#ifndef QSTACKEDLAYOUT_H
#define QSTACKEDLAYOUT_H

#include <qlayout.h>


class QStackedLayoutPrivate;

class Q_GUI_EXPORT QStackedLayout : public QLayout
{
   GUI_CS_OBJECT(QStackedLayout)

   GUI_CS_ENUM(StackingMode)

   GUI_CS_PROPERTY_READ(currentIndex, currentIndex)
   GUI_CS_PROPERTY_WRITE(currentIndex, setCurrentIndex)
   GUI_CS_PROPERTY_NOTIFY(currentIndex, currentChanged)

   GUI_CS_PROPERTY_READ(stackingMode, stackingMode)
   GUI_CS_PROPERTY_WRITE(stackingMode, setStackingMode)

   // following is qdoc_property
   GUI_CS_PROPERTY_READ(count, count)

 public:
   enum StackingMode {
      StackOne,
      StackAll
   };

   QStackedLayout();
   explicit QStackedLayout(QWidget *parent);
   explicit QStackedLayout(QLayout *parentLayout);

   QStackedLayout(const QStackedLayout &) = delete;
   QStackedLayout &operator=(const QStackedLayout &) = delete;

   ~QStackedLayout();

   int addWidget(QWidget *widget);
   int insertWidget(int index, QWidget *widget);

   QWidget *currentWidget() const;
   int currentIndex() const;

   using QLayout::widget;
   QWidget *widget(int index) const;
   int count() const override;

   StackingMode stackingMode() const;
   void setStackingMode(StackingMode stackingMode);

   // abstract virtual functions:
   void addItem(QLayoutItem *item) override;
   QSize sizeHint() const override;
   QSize minimumSize() const override;
   QLayoutItem *itemAt(int index) const override;
   QLayoutItem *takeAt(int index) override;
   void setGeometry(const QRect &rect) override;
   bool hasHeightForWidth() const override;
   int heightForWidth(int width) const override;

   GUI_CS_SIGNAL_1(Public, void widgetRemoved(int index))
   GUI_CS_SIGNAL_2(widgetRemoved, index)

   GUI_CS_SIGNAL_1(Public, void currentChanged(int index))
   GUI_CS_SIGNAL_2(currentChanged, index)

   GUI_CS_SLOT_1(Public, void setCurrentIndex(int index))
   GUI_CS_SLOT_2(setCurrentIndex)

   GUI_CS_SLOT_1(Public, void setCurrentWidget(QWidget *widget))
   GUI_CS_SLOT_2(setCurrentWidget)

 private:
   Q_DECLARE_PRIVATE(QStackedLayout)
};

#endif // QSTACKEDLAYOUT_H
