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

#ifndef QWAYLAND_DATA_SOURCE_H
#define QWAYLAND_DATA_SOURCE_H

#include <qobject.h>

#include <qwayland-wayland.h>

#ifndef QT_NO_DRAGANDDROP

class QMimeData;

namespace QtWaylandClient {

class QWaylandDataDeviceManager;

class Q_WAYLAND_CLIENT_EXPORT QWaylandDataSource : public QObject, public QtWayland::wl_data_source
{
   CS_OBJECT(QWaylandDataSource)

 public:
   QWaylandDataSource(QWaylandDataDeviceManager *dataDeviceManager, QMimeData *mimeData);
   ~QWaylandDataSource();

   QMimeData *mimeData() const;

   CS_SIGNAL_1(Public, void targetChanged(const QString &mime_type))
   CS_SIGNAL_2(targetChanged, mime_type)

   CS_SIGNAL_1(Public, void cancelled())
   CS_SIGNAL_2(cancelled)

 protected:
   void data_source_cancelled() override;
   void data_source_send(const QString &mime_type, int32_t fd) override;
   void data_source_target(const QString &mime_type) override;

 private:
   QMimeData *m_mime_data;
};

}

#endif // QT_NO_DRAGANDDROP

#endif
