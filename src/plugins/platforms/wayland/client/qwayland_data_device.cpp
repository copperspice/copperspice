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
#include <qwayland_display_p.h>
#include <qwayland_dnd_p.h>
#include <qwayland_inputdevice_p.h>
#include <qwayland_window_p.h>

#ifndef QT_NO_DRAGANDDROP

namespace QtWaylandClient {

QWaylandDataDevice::QWaylandDataDevice(QWaylandDataDeviceManager *manager, QWaylandInputDevice *inputDevice)
   : QtWayland::wl_data_device(manager->get_data_device(inputDevice->wl_seat())),
     m_enterSerial(0), m_dragWindow(nullptr), m_display(manager->display()), m_inputDevice(inputDevice)
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
   if (source != nullptr) {
      connect(source, &QWaylandDataSource::cancelled, this, &QWaylandDataDevice::selectionSourceCancelled);
   }

   set_selection(source != nullptr ? source->object() : nullptr, m_inputDevice->serial());
   m_selectionSource.reset(source);
}

QWaylandDataOffer *QWaylandDataDevice::dragOffer() const
{
   return m_dragOffer.data();
}

void QWaylandDataDevice::startDrag(QMimeData *mimeData, QWaylandWindow *icon)
{
   m_dragSource.reset(new QWaylandDataSource(m_display->dndSelectionHandler(), mimeData));
   connect(m_dragSource.data(), &QWaylandDataSource::cancelled, this, &QWaylandDataDevice::dragSourceCancelled);

   QWaylandWindow *origin = m_display->currentInputDevice()->pointerFocus();

   if (origin == nullptr) {
      origin = m_display->currentInputDevice()->touchFocus();
   }

   start_drag(m_dragSource->object(), origin->object(), icon->object(), m_display->currentInputDevice()->serial());
}

void QWaylandDataDevice::cancelDrag()
{
   m_dragSource.reset();
}

void QWaylandDataDevice::data_device_data_offer(struct ::wl_data_offer *id)
{
   new QWaylandDataOffer(m_display, id);
}

void QWaylandDataDevice::data_device_drop()
{
   QDrag *drag = static_cast<QWaylandDrag *>(QApplicationPrivate::platformIntegration()->drag())->currentDrag();

   QMimeData *dragData = nullptr;
   Qt::DropActions supportedActions;

   if (drag != nullptr) {
      dragData = drag->mimeData();
      supportedActions = drag->supportedActions();
   } else if (m_dragOffer != nullptr) {
      dragData = m_dragOffer->mimeData();
      supportedActions = Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
   } else {
      return;
   }

   QPlatformDropQtResponse response = QWindowSystemInterface::handleDrop(m_dragWindow, dragData, m_dragPoint, supportedActions);

   if (drag != nullptr) {
      static_cast<QWaylandDrag *>(QApplicationPrivate::platformIntegration()->drag())->finishDrag(response);
   }
}

void QWaylandDataDevice::data_device_enter(uint32_t serial, wl_surface *surface, wl_fixed_t x, wl_fixed_t y, wl_data_offer *id)
{
   m_enterSerial = serial;
   m_dragWindow  = QWaylandWindow::fromWlSurface(surface)->window();
   m_dragPoint   = QPoint(wl_fixed_to_int(x), wl_fixed_to_int(y));

   QMimeData *dragData = nullptr;
   Qt::DropActions supportedActions;

   m_dragOffer.reset(static_cast<QWaylandDataOffer *>(wl_data_offer_get_user_data(id)));
   QDrag *drag = static_cast<QWaylandDrag *>(QApplicationPrivate::platformIntegration()->drag())->currentDrag();

   if (drag != nullptr) {
      dragData = drag->mimeData();
      supportedActions = drag->supportedActions();
   } else if (m_dragOffer != nullptr) {
      dragData = m_dragOffer->mimeData();
      supportedActions = Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
   }

   const QPlatformDragQtResponse &response = QWindowSystemInterface::handleDrag(m_dragWindow, dragData, m_dragPoint, supportedActions);

   if (drag != nullptr) {
      static_cast<QWaylandDrag * >(QApplicationPrivate::platformIntegration()->drag())->setResponse(response);
   }

   if (response.isAccepted()) {
      wl_data_offer_accept(m_dragOffer->object(), m_enterSerial, m_dragOffer->firstFormat().toUtf8().constData());
   } else {
      wl_data_offer_accept(m_dragOffer->object(), m_enterSerial, nullptr);
   }
}

void QWaylandDataDevice::data_device_leave()
{
   QWindowSystemInterface::handleDrag(m_dragWindow, nullptr, QPoint(), Qt::IgnoreAction);

   QDrag *drag = static_cast<QWaylandDrag *>(QApplicationPrivate::platformIntegration()->drag())->currentDrag();

   if (drag == nullptr) {
      m_dragOffer.reset();
   }
}

void QWaylandDataDevice::data_device_motion(uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
   (void) time;

   QDrag *drag = static_cast<QWaylandDrag *>(QApplicationPrivate::platformIntegration()->drag())->currentDrag();

   if (drag == nullptr && m_dragOffer == nullptr) {
      return;
   }

   m_dragPoint = QPoint(wl_fixed_to_int(x), wl_fixed_to_int(y));

   QMimeData *dragData;
   Qt::DropActions supportedActions;

   if (drag != nullptr) {
      dragData = drag->mimeData();
      supportedActions = drag->supportedActions();
   } else {
      dragData = m_dragOffer->mimeData();
      supportedActions = Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
   }

   QPlatformDragQtResponse response = QWindowSystemInterface::handleDrag(m_dragWindow, dragData, m_dragPoint, supportedActions);

   if (drag != nullptr) {
      static_cast<QWaylandDrag *>(QApplicationPrivate::platformIntegration()->drag())->setResponse(response);
   }

   if (response.isAccepted()) {
      wl_data_offer_accept(m_dragOffer->object(), m_enterSerial, m_dragOffer->firstFormat().toUtf8().constData());
   } else {
      wl_data_offer_accept(m_dragOffer->object(), m_enterSerial, nullptr);
   }
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
   static_cast<QWaylandDrag *>(QApplicationPrivate::platformIntegration()->drag())->updateTarget(mimeType);
}

}

#endif
