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

#ifndef QT_NO_QWS_MOUSE_QVFB

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <qvfbhdr.h>
#include <qmousevfb_qws.h>
#include <qwindowsystem_qws.h>
#include <qsocketnotifier.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qcore_unix_p.h>       // overrides QT_OPEN

QT_BEGIN_NAMESPACE

QVFbMouseHandler::QVFbMouseHandler(const QString &driver, const QString &device)
   : QObject(), QWSMouseHandler(driver, device)
{
   QString mouseDev = device;
   if (device.isEmpty()) {
      mouseDev = QLatin1String("/dev/vmouse");
   }

   mouseFD = QT_OPEN(mouseDev.toLatin1().constData(), O_RDWR | O_NDELAY);
   if (mouseFD == -1) {
      perror("QVFbMouseHandler::QVFbMouseHandler");
      qWarning("QVFbMouseHander: Unable to open device %s",
               qPrintable(mouseDev));
      return;
   }

   // Clear pending input
   char buf[2];
   while (QT_READ(mouseFD, buf, 1) > 0) { }

   mouseIdx = 0;

   mouseNotifier = new QSocketNotifier(mouseFD, QSocketNotifier::Read, this);
   connect(mouseNotifier, SIGNAL(activated(int)), this, SLOT(readMouseData()));
}

QVFbMouseHandler::~QVFbMouseHandler()
{
   if (mouseFD >= 0) {
      QT_CLOSE(mouseFD);
   }
}

void QVFbMouseHandler::resume()
{
   mouseNotifier->setEnabled(true);
}

void QVFbMouseHandler::suspend()
{
   mouseNotifier->setEnabled(false);
}

void QVFbMouseHandler::readMouseData()
{
   int n;
   do {
      n = QT_READ(mouseFD, mouseBuf + mouseIdx, mouseBufSize - mouseIdx);
      if (n > 0) {
         mouseIdx += n;
      }
   } while (n > 0);

   int idx = 0;
   static const int packetsize = sizeof(QPoint) + 2 * sizeof(int);
   while (mouseIdx - idx >= packetsize) {
      uchar *mb = mouseBuf + idx;
      QPoint mousePos = *reinterpret_cast<QPoint *>(mb);
      mb += sizeof(QPoint);
      int bstate = *reinterpret_cast<int *>(mb);
      mb += sizeof(int);
      int wheel = *reinterpret_cast<int *>(mb);
      //        limitToScreen(mousePos);
      mouseChanged(mousePos, bstate, wheel);
      idx += packetsize;
   }

   int surplus = mouseIdx - idx;
   for (int i = 0; i < surplus; i++) {
      mouseBuf[i] = mouseBuf[idx + i];
   }
   mouseIdx = surplus;
}

QT_END_NAMESPACE

#endif // QT_NO_QWS_MOUSE_QVFB
