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

#ifndef QWAYLAND_DATAOFFER_H
#define QWAYLAND_DATAOFFER_H

#include <qdnd_p.h>
#include <qwayland-wayland.h>

#ifndef QT_NO_DRAGANDDROP

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandMimeData;

class Q_WAYLAND_CLIENT_EXPORT QWaylandDataOffer : public QtWayland::wl_data_offer
{
 public:
   explicit QWaylandDataOffer(QWaylandDisplay *display, struct ::wl_data_offer *offer);
   ~QWaylandDataOffer();

   QString firstFormat() const;

   QMimeData *mimeData();

 protected:
   void data_offer_offer(const QString &mime_type) override;

 private:
   QScopedPointer<QWaylandMimeData> m_mimeData;
};

class QWaylandMimeData : public QInternalMimeData
{
 public:
   explicit QWaylandMimeData(QWaylandDataOffer *dataOffer, QWaylandDisplay *display);
   ~QWaylandMimeData();

   void appendFormat(const QString &mimeType);

 protected:
   bool hasFormat_sys(const QString &mimeType) const override;
   QStringList formats_sys() const override;
   QVariant retrieveData_sys(const QString &mimeType, QVariant::Type type) const override;

 private:
   int readData(int fd, QByteArray &data) const;

   mutable QWaylandDataOffer *m_dataOffer;
   QWaylandDisplay *m_display;

   mutable QStringList m_types;
   mutable QHash<QString, QByteArray> m_data;
};

}

#endif // QT_NO_DRAGANDDROP

#endif
