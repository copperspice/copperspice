/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

#ifndef QSTATUSBAR_H
#define QSTATUSBAR_H

#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_STATUSBAR

class QStatusBarPrivate;

class Q_GUI_EXPORT QStatusBar: public QWidget
{
   GUI_CS_OBJECT(QStatusBar)

   GUI_CS_PROPERTY_READ(sizeGripEnabled, isSizeGripEnabled)
   GUI_CS_PROPERTY_WRITE(sizeGripEnabled, setSizeGripEnabled)

 public:
   explicit QStatusBar(QWidget *parent = 0);
   virtual ~QStatusBar();

   void addWidget(QWidget *widget, int stretch = 0);
   int insertWidget(int index, QWidget *widget, int stretch = 0);
   void addPermanentWidget(QWidget *widget, int stretch = 0);
   int insertPermanentWidget(int index, QWidget *widget, int stretch = 0);
   void removeWidget(QWidget *widget);

   void setSizeGripEnabled(bool);
   bool isSizeGripEnabled() const;

   QString currentMessage() const;

   GUI_CS_SLOT_1(Public, void showMessage(const QString &text, int timeout = 0))
   GUI_CS_SLOT_2(showMessage)
   GUI_CS_SLOT_1(Public, void clearMessage())
   GUI_CS_SLOT_2(clearMessage)

   GUI_CS_SIGNAL_1(Public, void messageChanged(const QString &text))
   GUI_CS_SIGNAL_2(messageChanged, text)

 protected:
   void showEvent(QShowEvent *) override;
   void paintEvent(QPaintEvent *) override;
   void resizeEvent(QResizeEvent *) override;

   // ### Qt5/consider making reformat() and hideOrShow() private
   void reformat();
   void hideOrShow();
   bool event(QEvent *) override;

 private:
   Q_DISABLE_COPY(QStatusBar)
   Q_DECLARE_PRIVATE(QStatusBar)
};

#endif // QT_NO_STATUSBAR

QT_END_NAMESPACE

#endif // QSTATUSBAR_H
