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

#ifndef QSTACKEDWIDGET_H
#define QSTACKEDWIDGET_H

#include <qframe.h>

#ifndef QT_NO_STACKEDWIDGET

class QStackedWidgetPrivate;

class Q_GUI_EXPORT QStackedWidget : public QFrame
{
   GUI_CS_OBJECT(QStackedWidget)

   GUI_CS_PROPERTY_READ(currentIndex, currentIndex)
   GUI_CS_PROPERTY_WRITE(currentIndex, setCurrentIndex)
   GUI_CS_PROPERTY_NOTIFY(currentIndex, currentChanged)

   GUI_CS_PROPERTY_READ(count, count)

 public:
   explicit QStackedWidget(QWidget *parent = nullptr);

   QStackedWidget(const QStackedWidget &) = delete;
   QStackedWidget &operator=(const QStackedWidget &) = delete;

   ~QStackedWidget();

   int addWidget(QWidget *widget);
   int insertWidget(int index, QWidget *widget);
   void removeWidget(QWidget *widget);

   QWidget *currentWidget() const;
   int currentIndex() const;

   int indexOf(QWidget *widget) const;
   QWidget *widget(int index) const;
   int count() const;

   GUI_CS_SLOT_1(Public, void setCurrentIndex(int index))
   GUI_CS_SLOT_2(setCurrentIndex)

   GUI_CS_SLOT_1(Public, void setCurrentWidget(QWidget *widget))
   GUI_CS_SLOT_2(setCurrentWidget)

   GUI_CS_SIGNAL_1(Public, void currentChanged(int index))
   GUI_CS_SIGNAL_2(currentChanged, index)

   GUI_CS_SIGNAL_1(Public, void widgetRemoved(int index))
   GUI_CS_SIGNAL_2(widgetRemoved, index)

 protected:
   bool event(QEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QStackedWidget)
};

#endif // QT_NO_STACKEDWIDGET

#endif
