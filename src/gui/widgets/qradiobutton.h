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

#ifndef QRADIOBUTTON_H
#define QRADIOBUTTON_H

#include <qabstractbutton.h>

class QRadioButtonPrivate;
class QStyleOptionButton;

class Q_GUI_EXPORT QRadioButton : public QAbstractButton
{
   GUI_CS_OBJECT(QRadioButton)

 public:
   explicit QRadioButton(QWidget *parent = nullptr);
   explicit QRadioButton(const QString &text, QWidget *parent = nullptr);

   QRadioButton(const QRadioButton &) = delete;
   QRadioButton &operator=(const QRadioButton &) = delete;

   ~QRadioButton();

   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;

 protected:
   bool event(QEvent *event) override;
   bool hitButton(const QPoint &point) const override;
   void paintEvent(QPaintEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void initStyleOption(QStyleOptionButton *button) const;

 private:
   Q_DECLARE_PRIVATE(QRadioButton)

   friend class QAccessibleButton;
};

#endif
