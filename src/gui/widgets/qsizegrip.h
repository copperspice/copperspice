/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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
   ~QSizeGrip();

   QSize sizeHint() const override;
   void setVisible(bool) override;

 protected:
   void paintEvent(QPaintEvent *) override;
   void mousePressEvent(QMouseEvent *) override;
   void mouseMoveEvent(QMouseEvent *) override;
   void mouseReleaseEvent(QMouseEvent *mouseEvent) override;
   void moveEvent(QMoveEvent *moveEvent) override;
   void showEvent(QShowEvent *showEvent) override;
   void hideEvent(QHideEvent *hideEvent) override;
   bool eventFilter(QObject *, QEvent *) override;

   bool event(QEvent *) override;



 private:
   Q_DECLARE_PRIVATE(QSizeGrip)
   Q_DISABLE_COPY(QSizeGrip)

   GUI_CS_SLOT_1(Private, void _q_showIfNotHidden())
   GUI_CS_SLOT_2(_q_showIfNotHidden)
};
#endif // QT_NO_SIZEGRIP



#endif
