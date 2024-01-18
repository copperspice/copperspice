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

#ifndef QSPLASHSCREEN_H
#define QSPLASHSCREEN_H

#include <qpixmap.h>
#include <qwidget.h>

#ifndef QT_NO_SPLASHSCREEN
class QSplashScreenPrivate;

class Q_GUI_EXPORT QSplashScreen : public QWidget
{
   GUI_CS_OBJECT(QSplashScreen)

 public:
   explicit QSplashScreen(const QPixmap &pixmap = QPixmap(), Qt::WindowFlags flags = Qt::EmptyFlag);
   QSplashScreen(QWidget *parent, const QPixmap &pixmap = QPixmap(), Qt::WindowFlags flags = Qt::EmptyFlag);

   QSplashScreen(const QSplashScreen &) = delete;
   QSplashScreen &operator=(const QSplashScreen &) = delete;

   virtual ~QSplashScreen();

   void setPixmap(const QPixmap &pixmap);
   const QPixmap pixmap() const;
   void finish(QWidget *widget);
   void repaint();
   QString message() const;

   GUI_CS_SLOT_1(Public, void showMessage(const QString &message, int alignment = Qt::AlignLeft,
         const QColor &color = Qt::black))
   GUI_CS_SLOT_2(showMessage)

   GUI_CS_SLOT_1(Public, void clearMessage())
   GUI_CS_SLOT_2(clearMessage)

   GUI_CS_SIGNAL_1(Public, void messageChanged(const QString &message))
   GUI_CS_SIGNAL_2(messageChanged, message)

 protected:
   bool event(QEvent *event) override;
   virtual void drawContents(QPainter *painter);
   void mousePressEvent(QMouseEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QSplashScreen)
};

#endif // QT_NO_SPLASHSCREEN

#endif
