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

#ifndef QCHECKBOX_H
#define QCHECKBOX_H

#include <qabstractbutton.h>

class QCheckBoxPrivate;
class QStyleOptionButton;

class Q_GUI_EXPORT QCheckBox : public QAbstractButton
{
   GUI_CS_OBJECT(QCheckBox)

   GUI_CS_PROPERTY_READ(tristate, isTristate)
   GUI_CS_PROPERTY_WRITE(tristate, setTristate)

 public:
   explicit QCheckBox(QWidget *parent = nullptr);
   explicit QCheckBox(const QString &text, QWidget *parent = nullptr);

   QCheckBox(const QCheckBox &other) = delete;
   QCheckBox &operator=(const QCheckBox &other) = delete;

   ~QCheckBox();

   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;

   void setTristate(bool enable = true);
   bool isTristate() const;

   Qt::CheckState checkState() const;
   void setCheckState(Qt::CheckState state);

   GUI_CS_SIGNAL_1(Public, void stateChanged(int state))
   GUI_CS_SIGNAL_2(stateChanged, state)

 protected:
   bool event(QEvent *event) override;
   bool hitButton(const QPoint &pos) const override;
   void checkStateSet() override;
   void nextCheckState() override;
   void paintEvent(QPaintEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void initStyleOption(QStyleOptionButton *option) const;

 private:
   Q_DECLARE_PRIVATE(QCheckBox)
   friend class QAccessibleButton;
};

#endif // QCHECKBOX_H
