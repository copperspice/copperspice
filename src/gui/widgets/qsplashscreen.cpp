/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#include <qsplashscreen.h>

#ifndef QT_NO_SPLASHSCREEN

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtextdocument.h>
#include <qtextcursor.h>
#include <qwindow.h>
#include <qdebug.h>
#include <qelapsedtimer.h>

#include <qwidget_p.h>
#ifdef Q_OS_WIN
#  include <qt_windows.h>
#else
#  include <time.h>
#endif

class QSplashScreenPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QSplashScreen)
 public:
   QPixmap pixmap;
   QString currStatus;
   QColor currColor;
   int currAlign;

   inline QSplashScreenPrivate();
};

QSplashScreen::QSplashScreen(const QPixmap &pixmap, Qt::WindowFlags f)
   : QWidget(*(new QSplashScreenPrivate()), nullptr, Qt::SplashScreen | Qt::FramelessWindowHint | f)
{
   setPixmap(pixmap);  // Does an implicit repaint
}

QSplashScreen::QSplashScreen(QWidget *parent, const QPixmap &pixmap, Qt::WindowFlags f)
   : QWidget(*new QSplashScreenPrivate, parent, Qt::SplashScreen | f)
{
   d_func()->pixmap = pixmap;
   setPixmap(d_func()->pixmap);  // Does an implicit repaint
}

QSplashScreen::~QSplashScreen()
{
}

/*!
    \reimp
*/
void QSplashScreen::mousePressEvent(QMouseEvent *)
{
   hide();
}

/*!
    This overrides QWidget::repaint(). It differs from the standard
    repaint function in that it also calls QApplication::flush() to
    ensure the updates are displayed, even when there is no event loop
    present.
*/
void QSplashScreen::repaint()
{
   QWidget::repaint();
   QApplication::flush();
}

/*!
    \fn QSplashScreen::messageChanged(const QString &message)

    This signal is emitted when the message on the splash screen
    changes. \a message is the new message and is a null-string
    when the message has been removed.

    \sa showMessage(), clearMessage()
*/



/*!
    Draws the \a message text onto the splash screen with color \a
    color and aligns the text according to the flags in \a alignment.

    To make sure the splash screen is repainted immediately, you can
    call \l{QCoreApplication}'s
    \l{QCoreApplication::}{processEvents()} after the call to
    showMessage(). You usually want this to make sure that the message
    is kept up to date with what your application is doing (e.g.,
    loading files).

    \sa Qt::Alignment, clearMessage()
*/
void QSplashScreen::showMessage(const QString &message, int alignment,
   const QColor &color)
{
   Q_D(QSplashScreen);
   d->currStatus = message;
   d->currAlign = alignment;
   d->currColor = color;
   emit messageChanged(d->currStatus);
   repaint();
}

QString QSplashScreen::message() const
{
   Q_D(const QSplashScreen);
   return d->currStatus;
}

void QSplashScreen::clearMessage()
{
   d_func()->currStatus.clear();
   emit messageChanged(d_func()->currStatus);
   repaint();
}

inline static bool waitForWindowExposed(QWindow *window, int timeout = 1000)
{
   enum { TimeOutMs = 10 };
   QElapsedTimer timer;
   timer.start();

   while (!window->isExposed()) {
      const int remaining = timeout - int(timer.elapsed());
      if (remaining <= 0) {
         break;
      }
      QCoreApplication::processEvents(QEventLoop::AllEvents, remaining);
      QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

#if defined(Q_OS_WIN)
      Sleep(uint(TimeOutMs));
#else
      struct timespec ts = { TimeOutMs / 1000, (TimeOutMs % 1000) * 1000 * 1000 };
      nanosleep(&ts, nullptr);
#endif
   }

   return window->isExposed();
}

void QSplashScreen::finish(QWidget *mainWin)
{
   if (mainWin) {
      if (!mainWin->windowHandle()) {
         mainWin->createWinId();
      }
      waitForWindowExposed(mainWin->windowHandle());
   }
   close();
}

void QSplashScreen::setPixmap(const QPixmap &pixmap)
{
   Q_D(QSplashScreen);

   d->pixmap = pixmap;
   setAttribute(Qt::WA_TranslucentBackground, pixmap.hasAlpha());

   QRect r(QPoint(), d->pixmap.size()  / d->pixmap.devicePixelRatio());
   resize(r.size());
   move(QApplication::desktop()->screenGeometry().center() - r.center());

   if (isVisible()) {
      repaint();
   }
}

const QPixmap QSplashScreen::pixmap() const
{
   return d_func()->pixmap;
}

inline QSplashScreenPrivate::QSplashScreenPrivate() : currAlign(Qt::AlignLeft)
{
}

void QSplashScreen::drawContents(QPainter *painter)
{
   Q_D(QSplashScreen);
   painter->setPen(d->currColor);
   QRect r = rect().adjusted(5, 5, -5, -5);
   if (Qt::mightBeRichText(d->currStatus)) {
      QTextDocument doc;
#ifdef QT_NO_TEXTHTMLPARSER
      doc.setPlainText(d->currStatus);
#else
      doc.setHtml(d->currStatus);
#endif
      doc.setTextWidth(r.width());
      QTextCursor cursor(&doc);
      cursor.select(QTextCursor::Document);
      QTextBlockFormat fmt;
      fmt.setAlignment(Qt::Alignment(d->currAlign));
      cursor.mergeBlockFormat(fmt);
      painter->save();
      painter->translate(r.topLeft());
      doc.drawContents(painter);
      painter->restore();
   } else {
      painter->drawText(r, d->currAlign, d->currStatus);
   }
}

bool QSplashScreen::event(QEvent *e)
{
   if (e->type() == QEvent::Paint) {
      Q_D(QSplashScreen);
      QPainter painter(this);
      if (!d->pixmap.isNull()) {
         painter.drawPixmap(QPoint(), d->pixmap);
      }
      drawContents(&painter);
   }
   return QWidget::event(e);
}
#endif //QT_NO_SPLASHSCREEN
