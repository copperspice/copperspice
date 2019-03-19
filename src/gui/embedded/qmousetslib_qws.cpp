/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qmousetslib_qws.h>

#if !defined(QT_NO_QWS_MOUSE_TSLIB) || defined(QT_PLUGIN)

#include <QtCore/QRegularExpression.h>
#include <QtCore/qstringlist.h>
#include <qsocketnotifier.h>
#include <qscreen_qws.h>

#include <tslib.h>
#include <errno.h>

QT_BEGIN_NAMESPACE

#ifdef TSLIBMOUSEHANDLER_DEBUG
#  include <QtCore/QDebug>
#endif

/*!
    \internal

    \class QWSTslibMouseHandler
    \ingroup qws

    \brief The QWSTslibMouseHandler class implements a mouse driver
    for the Universal Touch Screen Library, tslib.

    QWSTslibMouseHandler inherits the QWSCalibratedMouseHandler class,
    providing calibration and noise reduction functionality in
    addition to generating mouse events, for devices using the
    Universal Touch Screen Library.

    To be able to compile this mouse handler, \l{Qt for Embedded Linux}
    must be configured with the \c -qt-mouse-tslib option, see the
    \l{Pointer Handling} documentation for details. In addition, the tslib
    headers and library must be present in the build environment.  The
    tslib sources can be downloaded from \l
    {http://tslib.berlios.de/}.  Use the \c -L and \c -I options
    with \c configure to explicitly specify the location of the
    library and its headers:

    \snippet doc/src/snippets/code/src_gui_embedded_qmousetslib_qws.cpp 0

    In order to use this mouse handler, tslib must also be correctly
    installed on the target machine. This includes providing a \c
    ts.conf configuration file and setting the necessary environment
    variables, see the README file provided with tslib for details.

    The ts.conf file will usually contain the following two lines

    \snippet doc/src/snippets/code/src_gui_embedded_qmousetslib_qws.cpp 1

    To make \l{Qt for Embedded Linux} explicitly choose the tslib mouse
    handler, set the QWS_MOUSE_PROTO environment variable.

    \sa {Pointer Handling}, {Qt for Embedded Linux}
*/

class QWSTslibMouseHandlerPrivate : public QObject
{
   GUI_CS_OBJECT(QWSTslibMouseHandlerPrivate

             public:
             QWSTslibMouseHandlerPrivate(QWSTslibMouseHandler *h,
                                         const QString &device);
             ~QWSTslibMouseHandlerPrivate();

             void suspend();
             void resume();

             void calibrate(const QWSPointerCalibrationData *data);
             void clearCalibration();

             private:
             QWSTslibMouseHandler *handler;
             struct tsdev *dev;
             QSocketNotifier *mouseNotifier;
             int jitter_limit;

             struct ts_sample lastSample;
             bool wasPressed;
             int lastdx;
             int lastdy;

             bool calibrated;
             QString devName;

             bool open();
             void close();
             inline bool get_sample(struct ts_sample *sample);

             GUI_CS_SLOT_1(Private, void readMouseData())
             GUI_CS_SLOT_2(readMouseData)
};

QWSTslibMouseHandlerPrivate::QWSTslibMouseHandlerPrivate(QWSTslibMouseHandler *h, const QString &device)
   : handler(h), dev(0), mouseNotifier(0), jitter_limit(3)
{
   QStringList args = device.split(QLatin1Char(':'), QStringParser::SkipEmptyParts);
   QRegularExpression jitterRegex(QLatin1String("^jitter_limit=(\\d+)$"));
   int index = args.indexOf(jitterRegex);
   if (index >= 0) {
      jitter_limit = jitterRegex.cap(1).toInt();
      args.removeAt(index);
   }

   devName = args.join(QString());

   if (devName.isNull()) {
      const char *str = getenv("TSLIB_TSDEVICE");
      if (str) {
         devName = QString::fromUtf8(str);
      }
   }

   if (devName.isNull()) {
      devName = "/dev/ts";
   }

   if (!open()) {
      return;
   }

   calibrated = true;

   int fd = ts_fd(dev);
   mouseNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
   connect(mouseNotifier, SIGNAL(activated(int)), this, SLOT(readMouseData()));
   resume();
}

QWSTslibMouseHandlerPrivate::~QWSTslibMouseHandlerPrivate()
{
   close();
}

bool QWSTslibMouseHandlerPrivate::open()
{
   dev = ts_open(devName.toUtf8().constData(), 1);
   if (!dev) {
      qCritical("QWSTslibMouseHandlerPrivate: ts_open() failed"
                " with error: '%s'", strerror(errno));
      qCritical("Please check your tslib installation!");
      return false;
   }

   if (ts_config(dev)) {
      qCritical("QWSTslibMouseHandlerPrivate: ts_config() failed"
                " with error: '%s'", strerror(errno));
      qCritical("Please check your tslib installation!");
      close();
      return false;
   }

   return true;
}

void QWSTslibMouseHandlerPrivate::close()
{
   if (dev) {
      ts_close(dev);
   }
}

void QWSTslibMouseHandlerPrivate::suspend()
{
   if (mouseNotifier) {
      mouseNotifier->setEnabled(false);
   }
}

void QWSTslibMouseHandlerPrivate::resume()
{
   memset(&lastSample, 0, sizeof(lastSample));
   wasPressed = false;
   lastdx = 0;
   lastdy = 0;
   if (mouseNotifier) {
      mouseNotifier->setEnabled(true);
   }
}

bool QWSTslibMouseHandlerPrivate::get_sample(struct ts_sample *sample)
{
   if (!calibrated) {
      return (ts_read_raw(dev, sample, 1) == 1);
   }

   return (ts_read(dev, sample, 1) == 1);
}

void QWSTslibMouseHandlerPrivate::readMouseData()
{
   if (!qt_screen) {
      return;
   }

   for (;;) {
      struct ts_sample sample = lastSample;
      bool pressed = wasPressed;

      // Fast return if there's no events.
      if (!get_sample(&sample)) {
         return;
      }
      pressed = (sample.pressure > 0);

      // Only return last sample unless there's a press/release event.
      while (pressed == wasPressed) {
         if (!get_sample(&sample)) {
            break;
         }
         pressed = (sample.pressure > 0);
      }

      // work around missing coordinates on mouse release
      if (!pressed && sample.x == 0 && sample.y == 0) {
	 sample.x = lastSample.x;
	 sample.y = lastSample.y;
      }

      int dx = sample.x - lastSample.x;
      int dy = sample.y - lastSample.y;

      // Remove small movements in oppsite direction
      if (dx * lastdx < 0 && qAbs(dx) < jitter_limit) {
         sample.x = lastSample.x;
         dx = 0;
      }
      if (dy * lastdy < 0 && qAbs(dy) < jitter_limit) {
         sample.y = lastSample.y;
         dy = 0;
      }

      if (wasPressed == pressed && dx == 0 && dy == 0) {
         return;
      }

#ifdef TSLIBMOUSEHANDLER_DEBUG
      qDebug() << "last" << QPoint(lastSample.x, lastSample.y)
               << "curr" << QPoint(sample.x, sample.y)
               << "dx,dy" << QPoint(dx, dy)
               << "ddx,ddy" << QPoint(dx * lastdx, dy * lastdy)
               << "pressed" << wasPressed << pressed;
#endif

      lastSample = sample;
      wasPressed = pressed;
      if (dx != 0) {
         lastdx = dx;
      }
      if (dy != 0) {
         lastdy = dy;
      }

      const QPoint p(sample.x, sample.y);
      if (calibrated) {
         // tslib should do all the translation and filtering, so we send a
         // "raw" mouse event
         handler->QWSMouseHandler::mouseChanged(p, pressed);
      } else {
         handler->sendFiltered(p, pressed);
      }
   }
}

void QWSTslibMouseHandlerPrivate::clearCalibration()
{
   suspend();
   close();
   handler->QWSCalibratedMouseHandler::clearCalibration();
   calibrated = false;
   open();
   resume();
}

void QWSTslibMouseHandlerPrivate::calibrate(const QWSPointerCalibrationData *data)
{
   suspend();
   close();
   // default implementation writes to /etc/pointercal
   // using the same format as the tslib linear module.
   handler->QWSCalibratedMouseHandler::calibrate(data);
   calibrated = true;
   open();
   resume();
}

/*!
    \internal
*/
QWSTslibMouseHandler::QWSTslibMouseHandler(const QString &driver,
      const QString &device)
   : QWSCalibratedMouseHandler(driver, device)
{
   d = new QWSTslibMouseHandlerPrivate(this, device);
}

/*!
    \internal
*/
QWSTslibMouseHandler::~QWSTslibMouseHandler()
{
   delete d;
}

/*!
    \reimp
*/
void QWSTslibMouseHandler::suspend()
{
   d->suspend();
}

/*!
    \reimp
*/
void QWSTslibMouseHandler::resume()
{
   d->resume();
}

/*!
    \reimp
*/
void QWSTslibMouseHandler::clearCalibration()
{
   d->clearCalibration();
}

/*!
    \reimp
*/
void QWSTslibMouseHandler::calibrate(const QWSPointerCalibrationData *data)
{
   d->calibrate(data);
}

QT_END_NAMESPACE

#endif //QT_NO_QWS_MOUSE_TSLIB
