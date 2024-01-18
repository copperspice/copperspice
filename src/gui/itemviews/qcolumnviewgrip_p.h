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

#ifndef QCOLUMNVIEWGRIP_P_H
#define QCOLUMNVIEWGRIP_P_H

#include <qwidget_p.h>

#ifndef QT_NO_QCOLUMNVIEW

class QColumnViewGripPrivate;

class QColumnViewGrip : public QWidget
{
   GUI_CS_OBJECT(QColumnViewGrip)

 public:
   explicit QColumnViewGrip(QWidget *parent = nullptr);

   QColumnViewGrip(const QColumnViewGrip &) = delete;
   QColumnViewGrip &operator=(const QColumnViewGrip &) = delete;

   ~QColumnViewGrip();
   int moveGrip(int offset);

   GUI_CS_SIGNAL_1(Public, void gripMoved(int offset))
   GUI_CS_SIGNAL_2(gripMoved, offset)

 protected:
   QColumnViewGrip(QColumnViewGripPrivate &, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);

   void paintEvent(QPaintEvent *event) override;
   void mouseDoubleClickEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QColumnViewGrip)
};

class QColumnViewGripPrivate : public QWidgetPrivate
{
 public:
   QColumnViewGripPrivate();

   ~QColumnViewGripPrivate()
   {
   }

   int originalXLocation;

 private:
   Q_DECLARE_PUBLIC(QColumnViewGrip)
};

#endif //QT_NO_QCOLUMNVIEW

#endif
