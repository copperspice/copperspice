/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qwayland_dataoffer_p.h>

#include <qplatform_clipboard.h>

#include <qapplication_p.h>
#include <qcore_unix_p.h>
#include <qwayland_data_devicemanager_p.h>
#include <qwayland_display_p.h>

#ifndef QT_NO_DRAGANDDROP

namespace QtWaylandClient {

static QString utf8Text()
{
   return QString("text/plain;charset=utf-8");
}

QWaylandDataOffer::QWaylandDataOffer(QWaylandDisplay *display, struct ::wl_data_offer *offer)
   : QtWayland::wl_data_offer(offer), m_mimeData(new QWaylandMimeData(this, display))
{
}

QWaylandDataOffer::~QWaylandDataOffer()
{
   destroy();
}

QString QWaylandDataOffer::firstFormat() const
{
   // pending implementation
   return QString();
}

QMimeData *QWaylandDataOffer::mimeData()
{
   return m_mimeData.data();
}

void QWaylandDataOffer::data_offer_offer(const QString &mime_type)
{
   m_mimeData->appendFormat(mime_type);
}

QWaylandMimeData::QWaylandMimeData(QWaylandDataOffer *dataOffer, QWaylandDisplay *display)
   : QInternalMimeData(), m_dataOffer(dataOffer), m_display(display)
{
}

QWaylandMimeData::~QWaylandMimeData()
{
}

void QWaylandMimeData::appendFormat(const QString &mimeType)
{
   m_types << mimeType;
   m_data.remove(mimeType); // Clear previous contents
}

bool QWaylandMimeData::hasFormat_sys(const QString &mimeType) const
{
   if (m_types.contains(mimeType)) {
      return true;
   }

   if (mimeType == "text/plain" && m_types.contains(utf8Text())) {
      return true;
   }

   return false;
}

QStringList QWaylandMimeData::formats_sys() const
{
   return m_types;
}

QVariant QWaylandMimeData::retrieveData_sys(const QString &mimeType, QVariant::Type type) const
{
   (void) type;

   if (m_data.contains(mimeType)) {
      return m_data.value(mimeType);
   }

   QString mime = mimeType;

   if (! m_types.contains(mimeType)) {
      if (mimeType == "text/plain" && m_types.contains(utf8Text())) {
         mime = utf8Text();
      } else {
         return QVariant();
      }
   }

   int pipefd[2];

   if (::pipe2(pipefd, O_CLOEXEC | O_NONBLOCK) == -1) {
      qWarning("QWaylandMimeData::retrieveData_sys() pipe2() failed");
      return QVariant();
   }

   m_dataOffer->receive(mime, pipefd[1]);
   wl_display_flush(m_display->wl_display());

   close(pipefd[1]);

   QByteArray content;

   if (readData(pipefd[0], content) != 0) {
      qWarning("QWaylandMimeData::retrieveData_sys() Error reading data for mimeType %s", csPrintable(mimeType));
      content = QByteArray();
   }

   close(pipefd[0]);
   m_data.insert(mimeType, content);

   return content;
}

int QWaylandMimeData::readData(int fd, QByteArray &data) const
{
   char buf[4096];
   int retryCount = 0;
   int n;

   while (true) {
      n = QT_READ(fd, buf, sizeof buf);

      if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK) && ++retryCount < 1000) {
         usleep(1000);
      } else {
         break;
      }
   }

   if (retryCount >= 1000) {
      qWarning("QWaylandMimeData::readData() Timeout reading from pipe");
   }

   if (n > 0) {
      data.append(buf, n);
      n = readData(fd, data);
   }

   return n;
}

}

#endif // QT_NO_DRAGANDDROP
