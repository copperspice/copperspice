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

#include <qmouselinuxinput_qws.h>
#include <QScreen>
#include <QSocketNotifier>
#include <QStringList>
#include <qplatformdefs.h>
#include <qcore_unix_p.h> // overrides QT_OPEN
#include <errno.h>
#include <linux/input.h>

QT_BEGIN_NAMESPACE

class QWSLinuxInputMousePrivate : public QObject
{
   GUI_CS_OBJECT(QWSLinuxInputMousePrivate)

 public:
   QWSLinuxInputMousePrivate(QWSLinuxInputMouseHandler *, const QString &);
   ~QWSLinuxInputMousePrivate();

   void enable(bool on);

 private:
   GUI_CS_SLOT_1(Private, void readMouseData())
   GUI_CS_SLOT_2(readMouseData)

   QWSLinuxInputMouseHandler *m_handler;
   QSocketNotifier           *m_notify;
   int                        m_fd;
   int                        m_x, m_y;
   int                        m_buttons;
};

QWSLinuxInputMouseHandler::QWSLinuxInputMouseHandler(const QString &device)
   : QWSCalibratedMouseHandler(device)
{
   d = new QWSLinuxInputMousePrivate(this, device);
}

QWSLinuxInputMouseHandler::~QWSLinuxInputMouseHandler()
{
   delete d;
}

void QWSLinuxInputMouseHandler::suspend()
{
   d->enable(false);
}

void QWSLinuxInputMouseHandler::resume()
{
   d->enable(true);
}

QWSLinuxInputMousePrivate::QWSLinuxInputMousePrivate(QWSLinuxInputMouseHandler *h, const QString &device)
   : m_handler(h), m_notify(0), m_x(0), m_y(0), m_buttons(0)
{
   setObjectName(QLatin1String("LinuxInputSubsystem Mouse Handler"));

   QString dev = QLatin1String("/dev/input/event0");
   int grab = 0;

   QStringList args = device.split(QLatin1Char(':'));
   for (const QString & arg : args) {
      if (arg.startsWith(QLatin1String("grab="))) {
         grab = arg.mid(5).toInt();
      } else if (arg.startsWith(QLatin1String("/dev/"))) {
         dev = arg;
      }
   }

   m_fd = QT_OPEN(dev.toLocal8Bit().constData(), O_RDONLY | O_NDELAY, 0);
   if (m_fd >= 0) {
      ::ioctl(m_fd, EVIOCGRAB, grab);
      m_notify = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
      connect(m_notify, SIGNAL(activated(int)), this, SLOT(readMouseData()));
   } else {
      qWarning("Cannot open mouse input device '%s': %s", qPrintable(dev), strerror(errno));
      return;
   }
}

QWSLinuxInputMousePrivate::~QWSLinuxInputMousePrivate()
{
   if (m_fd >= 0) {
      QT_CLOSE(m_fd);
   }
}

void QWSLinuxInputMousePrivate::enable(bool on)
{
   if (m_notify) {
      m_notify->setEnabled(on);
   }
}

void QWSLinuxInputMousePrivate::readMouseData()
{
   if (!qt_screen) {
      return;
   }

   struct ::input_event buffer[32];
   int n = 0;

   forever {
      int bytesRead = QT_READ(m_fd, reinterpret_cast<char *>(buffer) + n, sizeof(buffer) - n);
      if (bytesRead == 0)
      {
         qWarning("Got EOF from the input device.");
         return;
      }
      if (bytesRead == -1)
      {
         if (errno != EAGAIN) {
            qWarning("Could not read from input device: %s", strerror(errno));
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
      struct ::input_event *data = &buffer[i];

      bool unknown = false;
      if (data->type == EV_ABS) {
         if (data->code == ABS_X) {
            m_x = data->value;
         } else if (data->code == ABS_Y) {
            m_y = data->value;
         } else {
            unknown = true;
         }
      } else if (data->type == EV_REL) {
         if (data->code == REL_X) {
            m_x += data->value;
         } else if (data->code == REL_Y) {
            m_y += data->value;
         } else {
            unknown = true;
         }
      } else if (data->type == EV_KEY && data->code == BTN_TOUCH) {
         m_buttons = data->value ? Qt::LeftButton : 0;
      } else if (data->type == EV_KEY) {
         int button = 0;
         switch (data->code) {
            case BTN_LEFT:
               button = Qt::LeftButton;
               break;
            case BTN_MIDDLE:
               button = Qt::MiddleButton;
               break;
            case BTN_RIGHT:
               button = Qt::RightButton;
               break;
         }
         if (data->value) {
            m_buttons |= button;
         } else {
            m_buttons &= ~button;
         }
      } else if (data->type == EV_SYN && data->code == SYN_REPORT) {
         QPoint pos(m_x, m_y);
         pos = m_handler->transform(pos);
         m_handler->limitToScreen(pos);
         m_handler->mouseChanged(pos, m_buttons);
      } else if (data->type == EV_MSC && data->code == MSC_SCAN) {
         // kernel encountered an unmapped key - just ignore it
         continue;
      } else {
         unknown = true;
      }
      if (unknown) {
         qWarning("unknown mouse event type=%x, code=%x, value=%x", data->type, data->code, data->value);
      }
   }
}

QT_END_NAMESPACE
