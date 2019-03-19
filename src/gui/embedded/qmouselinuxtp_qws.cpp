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

#include <qmouselinuxtp_qws.h>

#ifndef QT_NO_QWS_MOUSE_LINUXTP

#include <qwindowsystem_qws.h>
#include <qsocketnotifier.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qscreen_qws.h>
#include <qcore_unix_p.h>       // overrides QT_OPEN
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

QT_BEGIN_NAMESPACE

#if defined(QT_QWS_IPAQ)
#define QT_QWS_IPAQ_RAW
#define QT_QWS_SCREEN_COORDINATES
typedef struct {
   unsigned short pressure;
   unsigned short x;
   unsigned short y;
   unsigned short pad;
} TS_EVENT;
#elif defined(QT_QWS_EBX)
#define QT_QWS_EBX_RAW
#define QT_QWS_SCREEN_COORDINATES
#ifndef QT_QWS_SHARP
typedef struct {
   unsigned short pressure;
   unsigned short x;
   unsigned short y;
   unsigned short pad;
} TS_EVENT;
#else
typedef struct {
   long y;
   long x;
   long pressure;
   long long millisecs;
} TS_EVENT;
#define QT_QWS_TP_SAMPLE_SIZE 10
#define QT_QWS_TP_MINIMUM_SAMPLES 4
#define QT_QWS_TP_PRESSURE_THRESHOLD 500
#define QT_QWS_TP_MOVE_LIMIT 50
#define QT_QWS_TP_JITTER_LIMIT 2
#endif
#else // not IPAQ, not SHARP
typedef struct {
   unsigned short pressure;
   unsigned short x;
   unsigned short y;
   unsigned short pad;
} TS_EVENT;
#endif

#ifndef QT_QWS_TP_SAMPLE_SIZE
#define QT_QWS_TP_SAMPLE_SIZE 5
#endif

#ifndef QT_QWS_TP_MINIMUM_SAMPLES
#define QT_QWS_TP_MINIMUM_SAMPLES 5
#endif

#ifndef QT_QWS_TP_PRESSURE_THRESHOLD
#define QT_QWS_TP_PRESSURE_THRESHOLD 1
#endif

#ifndef QT_QWS_TP_MOVE_LIMIT
#define QT_QWS_TP_MOVE_LIMIT 100
#endif

#ifndef QT_QWS_TP_JITTER_LIMIT
#define QT_QWS_TP_JITTER_LIMIT 2
#endif

class QWSLinuxTPMouseHandlerPrivate : public QObject
{
   GUI_CS_OBJECT(QWSLinuxTPMouseHandlerPrivate)

 public:
   QWSLinuxTPMouseHandlerPrivate(QWSLinuxTPMouseHandler *h, const QString &);
   ~QWSLinuxTPMouseHandlerPrivate();

   void suspend();
   void resume();

 private:
   static const int mouseBufSize = 2048;
   int mouseFD;
   QPoint oldmouse;
   QPoint oldTotalMousePos;
   bool waspressed;
   QPolygon samples;
   int currSample;
   int lastSample;
   int numSamples;
   int skipCount;
   int mouseIdx;
   uchar mouseBuf[mouseBufSize];
   QWSLinuxTPMouseHandler *handler;
   QSocketNotifier *mouseNotifier;

   GUI_CS_SLOT_1(Private, void readMouseData())
   GUI_CS_SLOT_2(readMouseData)
};

QWSLinuxTPMouseHandler::QWSLinuxTPMouseHandler(const QString &driver, const QString &device)
   : QWSCalibratedMouseHandler(driver, device)
{
   d = new QWSLinuxTPMouseHandlerPrivate(this, device);
}

QWSLinuxTPMouseHandler::~QWSLinuxTPMouseHandler()
{
   delete d;
}

void QWSLinuxTPMouseHandler::suspend()
{
   d->suspend();
}

void QWSLinuxTPMouseHandler::resume()
{
   d->resume();
}

QWSLinuxTPMouseHandlerPrivate::QWSLinuxTPMouseHandlerPrivate(QWSLinuxTPMouseHandler *h,
      const QString &device)
   : samples(QT_QWS_TP_SAMPLE_SIZE), currSample(0), lastSample(0),
     numSamples(0), skipCount(0), handler(h)
{
   QString mousedev;
   if (device.isEmpty()) {
#if defined(QT_QWS_IPAQ)
# ifdef QT_QWS_IPAQ_RAW
      mousedev = QLatin1String("/dev/h3600_tsraw");
# else
      mousedev = QLatin1String("/dev/h3600_ts");
# endif
#else
      mousedev = QLatin1String("/dev/ts");
#endif
   } else {
      mousedev = device;
   }
   if ((mouseFD = QT_OPEN(mousedev.toLatin1().constData(), O_RDONLY | O_NDELAY)) < 0) {
      qWarning("Cannot open %s (%s)", qPrintable(mousedev), strerror(errno));
      return;
   }

   mouseNotifier = new QSocketNotifier(mouseFD, QSocketNotifier::Read,
                                       this);
   connect(mouseNotifier, SIGNAL(activated(int)), this, SLOT(readMouseData()));
   waspressed = false;
   mouseIdx = 0;
}

QWSLinuxTPMouseHandlerPrivate::~QWSLinuxTPMouseHandlerPrivate()
{
   if (mouseFD >= 0) {
      QT_CLOSE(mouseFD);
   }
}

void QWSLinuxTPMouseHandlerPrivate::suspend()
{
   if (mouseNotifier) {
      mouseNotifier->setEnabled(false);
   }
}

void QWSLinuxTPMouseHandlerPrivate::resume()
{
   mouseIdx = 0;
   currSample = 0;
   lastSample = 0;
   numSamples = 0;
   skipCount = 0;
   if (mouseNotifier) {
      mouseNotifier->setEnabled(true);
   }
}


void QWSLinuxTPMouseHandlerPrivate::readMouseData()
{
   if (!qt_screen) {
      return;
   }

   int n;
   do {
      n = QT_READ(mouseFD, mouseBuf + mouseIdx, mouseBufSize - mouseIdx);
      if (n > 0) {
         mouseIdx += n;
      }
   } while (n > 0 && mouseIdx < mouseBufSize);

   //qDebug("readMouseData()");

   TS_EVENT *data;
   int idx = 0;

   // perhaps we shouldn't be reading EVERY SAMPLE.
   while (mouseIdx - idx >= (int)sizeof(TS_EVENT)) {
      uchar *mb = mouseBuf + idx;
      data = (TS_EVENT *) mb;

      if (data->pressure >= QT_QWS_TP_PRESSURE_THRESHOLD) {
#ifdef QT_QWS_SHARP
         samples[currSample] = QPoint(1000 - data->x, data->y);
#else
         samples[currSample] = QPoint(data->x, data->y);
#endif
         numSamples++;
         if (numSamples >= QT_QWS_TP_MINIMUM_SAMPLES) {
            int sampleCount = qMin(numSamples + 1, samples.count());

            // average the rest
            QPoint mousePos = QPoint(0, 0);
            QPoint totalMousePos = oldTotalMousePos;
            totalMousePos += samples[currSample];
            if (numSamples >= samples.count()) {
               totalMousePos -= samples[lastSample];
            }

            mousePos = totalMousePos / (sampleCount - 1);
#if defined(QT_QWS_SCREEN_COORDINATES)
            mousePos = handler->transform(mousePos);
#endif
            if (!waspressed) {
               oldmouse = mousePos;
            }
            QPoint dp = mousePos - oldmouse;
            int dxSqr = dp.x() * dp.x();
            int dySqr = dp.y() * dp.y();
            if (dxSqr + dySqr < (QT_QWS_TP_MOVE_LIMIT * QT_QWS_TP_MOVE_LIMIT)) {
               if (waspressed) {
                  if ((dxSqr + dySqr > (QT_QWS_TP_JITTER_LIMIT * QT_QWS_TP_JITTER_LIMIT)) || skipCount > 2) {
                     handler->mouseChanged(mousePos, Qt::LeftButton);
                     oldmouse = mousePos;
                     skipCount = 0;
                  } else {
                     skipCount++;
                  }
               } else {
                  handler->mouseChanged(mousePos, Qt::LeftButton);
                  oldmouse = mousePos;
                  waspressed = true;
               }

               // save recuring information
               currSample++;
               if (numSamples >= samples.count()) {
                  lastSample++;
               }
               oldTotalMousePos = totalMousePos;
            } else {
               numSamples--; // don't use this sample, it was bad.
            }
         } else {
            // build up the average
            oldTotalMousePos += samples[currSample];
            currSample++;
         }
         if (currSample >= samples.count()) {
            currSample = 0;
         }
         if (lastSample >= samples.count()) {
            lastSample = 0;
         }
      } else {
         currSample = 0;
         lastSample = 0;
         numSamples = 0;
         skipCount = 0;
         oldTotalMousePos = QPoint(0, 0);
         if (waspressed) {
            handler->mouseChanged(oldmouse, 0);
            oldmouse = QPoint(-100, -100);
            waspressed = false;
         }
      }
      idx += sizeof(TS_EVENT);
   }

   int surplus = mouseIdx - idx;
   for (int i = 0; i < surplus; i++) {
      mouseBuf[i] = mouseBuf[idx + i];
   }
   mouseIdx = surplus;
}

QT_END_NAMESPACE

#endif //QT_NO_QWS_MOUSE_LINUXTP
