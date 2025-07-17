/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
*
* Copyright (C) 2013 Klarälvdalens Datakonsult AB (KDAB).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://opensource.org/licenses/BSD-2-Clause
*
***********************************************************************/

#include <qwayland_data_device_p.h>

#include <qapplication.h>
#include <qmimedata.h>
#include <qplatform_clipboard.h>
#include <qplatform_drag.h>
#include <qwindowsysteminterface.h>

#include <qapplication_p.h>
#include <qwayland_data_devicemanager_p.h>
#include <qwayland_data_source_p.h>
#include <qwayland_dataoffer_p.h>

#ifndef QT_NO_DRAGANDDROP

namespace QtWaylandClient {

QWaylandDataDevice::QWaylandDataDevice(QWaylandDataDeviceManager *manager, QWaylandInputDevice *inputDevice)
   : m_enterSerial(0), m_dragWindow(nullptr), m_display(nullptr), m_inputDevice(nullptr)
{
}

QWaylandDataDevice::~QWaylandDataDevice()
{
}

QWaylandDataOffer *QWaylandDataDevice::selectionOffer() const
{
   return m_selectionOffer.data();
}

void QWaylandDataDevice::invalidateSelectionOffer()
{
   m_selectionOffer.reset();
}

QWaylandDataSource *QWaylandDataDevice::selectionSource() const
{
   return m_selectionSource.data();
}

void QWaylandDataDevice::setSelectionSource(QWaylandDataSource *source)
{
   // pending implementation
}

QWaylandDataOffer *QWaylandDataDevice::dragOffer() const
{
   return m_dragOffer.data();
}

void QWaylandDataDevice::startDrag(QMimeData *mimeData, QWaylandWindow *icon)
{
   // pending implementation
}

void QWaylandDataDevice::cancelDrag()
{
   m_dragSource.reset();
}

void QWaylandDataDevice::data_device_data_offer(struct ::wl_data_offer *id)
{
   // pending implementation
}

void QWaylandDataDevice::data_device_drop()
{
   // pending implementation
}

void QWaylandDataDevice::data_device_enter(uint32_t serial, wl_surface *surface, wl_fixed_t x, wl_fixed_t y, wl_data_offer *id)
{
   // pending implementation
}

void QWaylandDataDevice::data_device_leave()
{
   QWindowSystemInterface::handleDrag(m_dragWindow, nullptr, QPoint(), Qt::IgnoreAction);

   // pending implementation
}

void QWaylandDataDevice::data_device_motion(uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
   (void) time;
   // pending implementation
}

void QWaylandDataDevice::data_device_selection(wl_data_offer *id)
{
   if (id != nullptr) {
      m_selectionOffer.reset(static_cast<QWaylandDataOffer *>(wl_data_offer_get_user_data(id)));
   } else {
      m_selectionOffer.reset();
   }

   QApplicationPrivate::platformIntegration()->clipboard()->emitChanged(QClipboard::Clipboard);
}

void QWaylandDataDevice::selectionSourceCancelled()
{
   m_selectionSource.reset();
   QApplicationPrivate::platformIntegration()->clipboard()->emitChanged(QClipboard::Clipboard);
}

void QWaylandDataDevice::dragSourceCancelled()
{
   m_dragSource.reset();
}

void QWaylandDataDevice::dragSourceTargetChanged(const QString &mimeType)
{
   // pending implementation
}

}

#endif
