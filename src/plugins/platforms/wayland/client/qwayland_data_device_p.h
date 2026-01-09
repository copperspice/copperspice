/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#ifndef QWAYLAND_DATA_DEVICE_H
#define QWAYLAND_DATA_DEVICE_H

#include <qobject.h>
#include <qpoint.h>
#include <qwayland-wayland.h>

#ifndef QT_NO_DRAGANDDROP

class QMimeData;
class QWindow;

namespace QtWaylandClient {

class QWaylandDataDeviceManager;
class QWaylandDataOffer;
class QWaylandDataSource;
class QWaylandDisplay;
class QWaylandInputDevice;
class QWaylandWindow;

class QWaylandDataDevice : public QObject, public QtWayland::wl_data_device
{
   CS_OBJECT(QWaylandDataDevice)

 public:
   QWaylandDataDevice(QWaylandDataDeviceManager *manager, QWaylandInputDevice *inputDevice);
   ~QWaylandDataDevice();

   QWaylandDataOffer *selectionOffer() const;
   void invalidateSelectionOffer();

   QWaylandDataSource *selectionSource() const;
   void setSelectionSource(QWaylandDataSource *source);

   QWaylandDataOffer *dragOffer() const;
   void startDrag(QMimeData *mimeData, QWaylandWindow *icon);
   void cancelDrag();

 protected:
   void data_device_data_offer(struct ::wl_data_offer *id) override;
   void data_device_drop() override;

   void data_device_enter(uint32_t serial, struct ::wl_surface *surface, wl_fixed_t x, wl_fixed_t y,
         struct ::wl_data_offer *id) override;

   void data_device_leave() override;
   void data_device_motion(uint32_t time, wl_fixed_t x, wl_fixed_t y) override;
   void data_device_selection(struct ::wl_data_offer *id) override;

 private:
   CS_SLOT_1(Private, void selectionSourceCancelled())
   CS_SLOT_2(selectionSourceCancelled)

   CS_SLOT_1(Private, void dragSourceCancelled())
   CS_SLOT_2(dragSourceCancelled)

   CS_SLOT_1(Private, void dragSourceTargetChanged(const QString & mimeType))
   CS_SLOT_2(dragSourceTargetChanged)

   uint32_t m_enterSerial;

   QWindow *m_dragWindow;
   QWaylandDisplay *m_display;
   QWaylandInputDevice *m_inputDevice;

   QPoint m_dragPoint;

   QScopedPointer<QWaylandDataOffer>  m_dragOffer;
   QScopedPointer<QWaylandDataOffer>  m_selectionOffer;
   QScopedPointer<QWaylandDataSource> m_dragSource;
   QScopedPointer<QWaylandDataSource> m_selectionSource;
};

}

#endif // QT_NO_DRAGANDDROP

#endif
