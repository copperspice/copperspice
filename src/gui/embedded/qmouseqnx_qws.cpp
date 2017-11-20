/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#include <qmouseqnx_qws.h>
#include <qplatformdefs.h>
#include <qsocketnotifier.h>
#include <qcore_unix_p.h>
#include <sys/dcmd_input.h>
#include <errno.h>

QT_BEGIN_NAMESPACE

QQnxMouseHandler::QQnxMouseHandler(const QString &driver, const QString &device)
   : QObject(), QWSMouseHandler(driver, device), mouseButtons(Qt::NoButton)
{
   // open the mouse device with O_NONBLOCK so reading won't block when there's no data
   mouseFD = QT_OPEN(device.isEmpty() ? "/dev/devi/mouse0" : device.toLatin1().constData(),
                     QT_OPEN_RDONLY | O_NONBLOCK);
   if (mouseFD == -1) {
      qErrnoWarning(errno, "QQnxMouseHandler: Unable to open mouse device");
   } else {
      struct _pointer_info data;
      if (devctl(mouseFD, _POINTERGETINFO, &data, sizeof(data), NULL) == EOK) {
         absolutePositioning = (data.flags & _POINTER_FLAG_ABSOLUTE);
      } else {
         absolutePositioning = !device.isEmpty() && device.contains(QLatin1String("touch"));
      }

      // register a socket notifier on the file descriptor so we'll wake up whenever
      // there's a mouse move waiting for us.
      mouseNotifier = new QSocketNotifier(mouseFD, QSocketNotifier::Read, this);
      connect(mouseNotifier, SIGNAL(activated(int)), SLOT(socketActivated()));

      qDebug("QQnxMouseHandler: connected.");
   }
}

/*!
    Destroys this mouse handler and closes the connection to the mouse device.
 */
QQnxMouseHandler::~QQnxMouseHandler()
{
   if (mouseFD != -1) {
      QT_CLOSE(mouseFD);
   }
}

void QQnxMouseHandler::resume()
{
   if (mouseNotifier) {
      mouseNotifier->setEnabled(true);
   }
}

void QQnxMouseHandler::suspend()
{
   if (mouseNotifier) {
      mouseNotifier->setEnabled(false);
   }
}

/*! \internal

  This function is called whenever there is activity on the mouse device.
  By default, it reads up to 10 mouse move packets and calls mouseChanged()
  for each of them.
*/
void QQnxMouseHandler::socketActivated()
{
   QPoint queuedPos = mousePos;

   // _mouse_packet is a QNX structure. devi-hid is nice enough to translate
   // the raw byte data from mouse devices into generic format for us.
   struct _mouse_packet buffer[32];
   int n = 0;

   forever {
      int bytesRead = QT_READ(mouseFD, reinterpret_cast<char *>(buffer) + n, sizeof(buffer) - n);
      if (bytesRead == -1)
      {
         // EAGAIN means that there are no more mouse events to read
         if (errno != EAGAIN) {
            qErrnoWarning(errno, "QQnxMouseHandler: Could not read from input device");
         }
         break;
      }

      n += bytesRead;
      if (n % sizeof(buffer[0]) == 0)
      {
         break;
      }
   }
   n /= sizeof(buffer[0]);

   for (int i = 0; i < n; ++i) {
      const struct _mouse_packet &packet = buffer[i];

      // translate the coordinates from the QNX data structure to the Qt coordinates
      if (absolutePositioning) {
         queuedPos = QPoint(packet.dx, packet.dy);
      } else {
         // note the swapped y axis
         queuedPos += QPoint(packet.dx, -packet.dy);

         // QNX only tells us relative mouse movements, not absolute ones, so
         // limit the cursor position manually to the screen
         limitToScreen(queuedPos);
      }

      // translate the QNX mouse button bitmask to Qt buttons
      int buttons = Qt::NoButton;
      if (packet.hdr.buttons & _POINTER_BUTTON_LEFT) {
         buttons |= Qt::LeftButton;
      }
      if (packet.hdr.buttons & _POINTER_BUTTON_MIDDLE) {
         buttons |= Qt::MiddleButton;
      }
      if (packet.hdr.buttons & _POINTER_BUTTON_RIGHT) {
         buttons |= Qt::RightButton;
      }

      if (buttons != mouseButtons) {
         // send the MouseEvent to avoid missing any clicks
         mouseChanged(queuedPos, buttons, 0);
         // mousePos updated by the mouseChanged()
         queuedPos = mousePos;
         mouseButtons = buttons;
      }
   }

   if (queuedPos != mousePos) {
      mouseChanged(queuedPos, mouseButtons, 0);
   }
}

QT_END_NAMESPACE
