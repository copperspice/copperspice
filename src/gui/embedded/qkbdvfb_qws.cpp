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

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <qvfbhdr.h>
#include <qkbdvfb_qws.h>

#ifndef QT_NO_QWS_KEYBOARD
#ifndef QT_NO_QWS_KBD_QVFB

#include <qwindowsystem_qws.h>
#include <qsocketnotifier.h>
#include <qapplication.h>
#include <qcore_unix_p.h> // overrides QT_OPEN

QT_BEGIN_NAMESPACE

QVFbKeyboardHandler::QVFbKeyboardHandler(const QString &device)
   : QObject()
{
   terminalName = device;
   if (terminalName.isEmpty()) {
      terminalName = QLatin1String("/dev/vkdb");
   }
   kbdFD = -1;
   kbdIdx = 0;
   kbdBufferLen = sizeof(QVFbKeyData) * 5;
   kbdBuffer = new unsigned char [kbdBufferLen];

   if ((kbdFD = QT_OPEN(terminalName.toLatin1().constData(), O_RDONLY | O_NDELAY)) < 0) {
      qWarning("Cannot open %s (%s)", terminalName.toLatin1().constData(),
               strerror(errno));
   } else {
      // Clear pending input
      char buf[2];
      while (QT_READ(kbdFD, buf, 1) > 0) { }

      notifier = new QSocketNotifier(kbdFD, QSocketNotifier::Read, this);
      connect(notifier, SIGNAL(activated(int)), this, SLOT(readKeyboardData()));
   }
}

QVFbKeyboardHandler::~QVFbKeyboardHandler()
{
   if (kbdFD >= 0) {
      QT_CLOSE(kbdFD);
   }
   delete [] kbdBuffer;
}


void QVFbKeyboardHandler::readKeyboardData()
{
   int n;
   do {
      n  = QT_READ(kbdFD, kbdBuffer + kbdIdx, kbdBufferLen - kbdIdx);
      if (n > 0) {
         kbdIdx += n;
      }
   } while (n > 0);

   int idx = 0;
   while (kbdIdx - idx >= (int)sizeof(QVFbKeyData)) {
      QVFbKeyData *kd = (QVFbKeyData *)(kbdBuffer + idx);
      if (kd->unicode == 0 && kd->keycode == 0 && kd->modifiers == 0 && kd->press) {
         // magic exit key
         qWarning("Instructed to quit by Virtual Keyboard");
         qApp->quit();
      }
      QWSServer::processKeyEvent(kd->unicode ? kd->unicode : 0xffff, kd->keycode, kd->modifiers, kd->press, kd->repeat);
      idx += sizeof(QVFbKeyData);
   }

   int surplus = kbdIdx - idx;
   for (int i = 0; i < surplus; i++) {
      kbdBuffer[i] = kbdBuffer[idx + i];
   }
   kbdIdx = surplus;
}

QT_END_NAMESPACE

#endif // QT_NO_QWS_KBD_QVFB
#endif // QT_NO_QWS_KEYBOARD
