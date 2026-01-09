/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#include <qwayland_data_source_p.h>

#include <qstringlist.h>
#include <qwayland_mimehelper.h>

#include <qwayland_data_devicemanager_p.h>
#include <qwayland_dataoffer_p.h>
#include <qwayland_inputdevice_p.h>

#include <unistd.h>

#ifndef QT_NO_DRAGANDDROP

namespace QtWaylandClient {

QWaylandDataSource::QWaylandDataSource(QWaylandDataDeviceManager *dataDeviceManager, QMimeData *mimeData)
   : QtWayland::wl_data_source(dataDeviceManager->create_data_source()), m_mime_data(mimeData)
{
   if (! mimeData) {
      return;
   }

   for (const QString &format : mimeData->formats()) {
      offer(format);
   }
}

QWaylandDataSource::~QWaylandDataSource()
{
   destroy();
}

QMimeData *QWaylandDataSource::mimeData() const
{
   return m_mime_data;
}

void QWaylandDataSource::data_source_cancelled()
{
   emit cancelled();
}

void QWaylandDataSource::data_source_send(const QString &mime_type, int32_t fd)
{
   QByteArray content = QWaylandMimeHelper::getByteArray(m_mime_data, mime_type);

   if (! content.isEmpty()) {
      auto result = write(fd, content.constData(), content.size());
      (void) result;
   }

   close(fd);
}

void QWaylandDataSource::data_source_target(const QString &mime_type)
{
   emit targetChanged(mime_type);
}

}

#endif // QT_NO_DRAGANDDROP
