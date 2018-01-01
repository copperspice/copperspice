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

#include <qkbdum_qws.h>

#if !defined(QT_NO_QWS_KEYBOARD) && !defined(QT_NO_QWS_KBD_UM)

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <qstring.h>
#include <qwindowsystem_qws.h>
#include <qsocketnotifier.h>
#include <qplatformdefs.h>
#include <qvfbhdr.h>

QT_BEGIN_NAMESPACE

class QWSUmKeyboardHandlerPrivate : public QObject
{
   GUI_CS_OBJECT(QWSUmKeyboardHandlerPrivate)

 public:
   QWSUmKeyboardHandlerPrivate(const QString &);
   ~QWSUmKeyboardHandlerPrivate();

 private:
   int kbdFD;
   int kbdIdx;
   const int kbdBufferLen;
   unsigned char *kbdBuffer;
   QSocketNotifier *notifier;

   GUI_CS_SLOT_1(Private, void readKeyboardData())
   GUI_CS_SLOT_2(readKeyboardData)
};

QWSUmKeyboardHandlerPrivate::QWSUmKeyboardHandlerPrivate(const QString &device)
   : kbdFD(-1), kbdIdx(0), kbdBufferLen(sizeof(QVFbKeyData) * 5)
{
   kbdBuffer = new unsigned char [kbdBufferLen];

   if ((kbdFD = QT_OPEN((const char *)device.toLocal8Bit(), O_RDONLY | O_NDELAY, 0)) < 0) {
      qDebug("Can not open %s (%s)", (const char *)device.toLocal8Bit(),
             strerror(errno));
   } else {
      // Clear pending input
      char buf[2];
      while (QT_READ(kbdFD, buf, 1) > 0) { }

      notifier = new QSocketNotifier(kbdFD, QSocketNotifier::Read, this);
      connect(notifier, SIGNAL(activated(int)), this, SLOT(readKeyboardData()));
   }
}

QWSUmKeyboardHandlerPrivate::~QWSUmKeyboardHandlerPrivate()
{
   if (kbdFD >= 0) {
      QT_CLOSE(kbdFD);
   }
   delete [] kbdBuffer;
}


void QWSUmKeyboardHandlerPrivate::readKeyboardData()
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
      // Qtopia Key filters must still work.
      QWSServer::processKeyEvent(kd->unicode, kd->keycode, kd->modifiers, kd->press, kd->repeat);
      idx += sizeof(QVFbKeyData);
   }

   int surplus = kbdIdx - idx;
   for (int i = 0; i < surplus; i++) {
      kbdBuffer[i] = kbdBuffer[idx + i];
   }
   kbdIdx = surplus;
}

QWSUmKeyboardHandler::QWSUmKeyboardHandler(const QString &device)
   : QWSKeyboardHandler()
{
   d = new QWSUmKeyboardHandlerPrivate(device);
}

QWSUmKeyboardHandler::~QWSUmKeyboardHandler()
{
   delete d;
}

QT_END_NAMESPACE

#endif // QT_NO_QWS_KEYBOARD && QT_NO_QWS_KBD_UM
