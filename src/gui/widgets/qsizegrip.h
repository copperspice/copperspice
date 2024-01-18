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

#ifndef QSIZEGRIP_H
#define QSIZEGRIP_H

#include <qwidget.h>

#ifndef QT_NO_SIZEGRIP

class QSizeGripPrivate;

class Q_GUI_EXPORT QSizeGrip : public QWidget
{
   GUI_CS_OBJECT(QSizeGrip)

 public:
   explicit QSizeGrip(QWidget *parent);

   QSizeGrip(const QSizeGrip &) = delete;
   QSizeGrip &operator=(const QSizeGrip &) = delete;

   ~QSizeGrip();

   QSize sizeHint() const override;
   void setVisible(bool visible) override;

 protected:
   void paintEvent(QPaintEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void moveEvent(QMoveEvent *event) override;
   void showEvent(QShowEvent *event) override;
   void hideEvent(QHideEvent *event) override;
   bool eventFilter(QObject *object, QEvent *event) override;

   bool event(QEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QSizeGrip)

   GUI_CS_SLOT_1(Private, void _q_showIfNotHidden())
   GUI_CS_SLOT_2(_q_showIfNotHidden)
};

#endif // QT_NO_SIZEGRIP

#endif
