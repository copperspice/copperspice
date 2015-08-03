/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QSIZEGRIP_H
#define QSIZEGRIP_H

#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SIZEGRIP
class QSizeGripPrivate;

class Q_GUI_EXPORT QSizeGrip : public QWidget
{
   GUI_CS_OBJECT(QSizeGrip)

 public:
   explicit QSizeGrip(QWidget *parent);
   ~QSizeGrip();

   QSize sizeHint() const;
   void setVisible(bool);

 protected:
   void paintEvent(QPaintEvent *);
   void mousePressEvent(QMouseEvent *);
   void mouseMoveEvent(QMouseEvent *);
   void mouseReleaseEvent(QMouseEvent *mouseEvent);
   void moveEvent(QMoveEvent *moveEvent);
   void showEvent(QShowEvent *showEvent);
   void hideEvent(QHideEvent *hideEvent);
   bool eventFilter(QObject *, QEvent *);
   bool event(QEvent *);

#ifdef Q_OS_WIN
   bool winEvent(MSG *m, long *result);
#endif

 private:
   Q_DECLARE_PRIVATE(QSizeGrip)
   Q_DISABLE_COPY(QSizeGrip)

   GUI_CS_SLOT_1(Private, void _q_showIfNotHidden())
   GUI_CS_SLOT_2(_q_showIfNotHidden)
};
#endif // QT_NO_SIZEGRIP

QT_END_NAMESPACE

#endif // QSIZEGRIP_H
