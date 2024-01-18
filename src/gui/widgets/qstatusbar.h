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

#ifndef QSTATUSBAR_H
#define QSTATUSBAR_H

#include <qwidget.h>

#ifndef QT_NO_STATUSBAR

class QStatusBarPrivate;

class Q_GUI_EXPORT QStatusBar : public QWidget
{
   GUI_CS_OBJECT(QStatusBar)

   GUI_CS_PROPERTY_READ(sizeGripEnabled, isSizeGripEnabled)
   GUI_CS_PROPERTY_WRITE(sizeGripEnabled, setSizeGripEnabled)

 public:
   explicit QStatusBar(QWidget *parent = nullptr);

   QStatusBar(const QStatusBar &) = delete;
   QStatusBar &operator=(const QStatusBar &) = delete;

   virtual ~QStatusBar();

   void addWidget(QWidget *widget, int stretch = 0);
   int insertWidget(int index, QWidget *widget, int stretch = 0);
   void addPermanentWidget(QWidget *widget, int stretch = 0);
   int insertPermanentWidget(int index, QWidget *widget, int stretch = 0);
   void removeWidget(QWidget *widget);

   void setSizeGripEnabled(bool enabled);
   bool isSizeGripEnabled() const;

   QString currentMessage() const;

   GUI_CS_SLOT_1(Public, void showMessage(const QString &msg, int timeout = 0))
   GUI_CS_SLOT_2(showMessage)

   GUI_CS_SLOT_1(Public, void clearMessage())
   GUI_CS_SLOT_2(clearMessage)

   GUI_CS_SIGNAL_1(Public, void messageChanged(const QString &msg))
   GUI_CS_SIGNAL_2(messageChanged, msg)

 protected:
   void showEvent(QShowEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;

   // TODO: consider making reformat() and hideOrShow() private
   void reformat();
   void hideOrShow();
   bool event(QEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QStatusBar)
};

#endif // QT_NO_STATUSBAR

#endif
