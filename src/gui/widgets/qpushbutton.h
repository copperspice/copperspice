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

#ifndef QPUSHBUTTON_H
#define QPUSHBUTTON_H

#include <QtGui/qabstractbutton.h>

QT_BEGIN_NAMESPACE

class QPushButtonPrivate;
class QMenu;
class QStyleOptionButton;

class Q_GUI_EXPORT QPushButton : public QAbstractButton
{
   GUI_CS_OBJECT(QPushButton)

   GUI_CS_PROPERTY_READ(autoDefault, autoDefault)
   GUI_CS_PROPERTY_WRITE(autoDefault, setAutoDefault)
   GUI_CS_PROPERTY_READ(default, isDefault)
   GUI_CS_PROPERTY_WRITE(default, setDefault)
   GUI_CS_PROPERTY_READ(flat, isFlat)
   GUI_CS_PROPERTY_WRITE(flat, setFlat)

 public:
   explicit QPushButton(QWidget *parent = nullptr);
   explicit QPushButton(const QString &text, QWidget *parent = nullptr);
   QPushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);
   ~QPushButton();

   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;

   bool autoDefault() const;
   void setAutoDefault(bool);
   bool isDefault() const;
   void setDefault(bool);

   void setFlat(bool);
   bool isFlat() const;

#ifndef QT_NO_MENU
   void setMenu(QMenu *menu);
   QMenu *menu() const;

   GUI_CS_SLOT_1(Public, void showMenu())
   GUI_CS_SLOT_2(showMenu)
#endif

 protected:
   bool event(QEvent *e) override;

#ifdef Q_OS_MAC
   bool hitButton(const QPoint &pos) const override;
#endif

   void paintEvent(QPaintEvent *) override;
   void keyPressEvent(QKeyEvent *) override;
   void focusInEvent(QFocusEvent *) override;
   void focusOutEvent(QFocusEvent *) override;
   void initStyleOption(QStyleOptionButton *option) const;
   QPushButton(QPushButtonPrivate &dd, QWidget *parent = nullptr);

 private:
   Q_DISABLE_COPY(QPushButton)
   Q_DECLARE_PRIVATE(QPushButton)

#ifndef QT_NO_MENU
   GUI_CS_SLOT_1(Private, void _q_popupPressed())
   GUI_CS_SLOT_2(_q_popupPressed)
#endif

};

QT_END_NAMESPACE

#endif // QPUSHBUTTON_H
