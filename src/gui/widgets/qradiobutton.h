/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

#ifndef QRADIOBUTTON_H
#define QRADIOBUTTON_H

#include <QtGui/qabstractbutton.h>

QT_BEGIN_NAMESPACE

class QRadioButtonPrivate;
class QStyleOptionButton;

class Q_GUI_EXPORT QRadioButton : public QAbstractButton
{
   CS_OBJECT(QRadioButton)

 public:
   explicit QRadioButton(QWidget *parent = 0);
   explicit QRadioButton(const QString &text, QWidget *parent = 0);

   QSize sizeHint() const;
   QSize minimumSizeHint() const;

 protected:
   bool event(QEvent *e);
   bool hitButton(const QPoint &) const;
   void paintEvent(QPaintEvent *);
   void mouseMoveEvent(QMouseEvent *);
   void initStyleOption(QStyleOptionButton *button) const;

 private:
   Q_DECLARE_PRIVATE(QRadioButton)
   Q_DISABLE_COPY(QRadioButton)
};

QT_END_NAMESPACE

#endif // QRADIOBUTTON_H
