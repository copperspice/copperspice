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

#ifndef QWAYLAND_DISPLAY_H
#define QWAYLAND_DISPLAY_H

#include <qobject.h>
#include <qpointer.h>
#include <qrect.h>
#include <qvector.h>
#include <qwaitcondition.h>

#include <qwayland-wayland.h>
#include <qwayland-xdg-shell.h>
#include <wayland-client.h>

class QAbstractEventDispatcher;
class QSocketNotifier;
class QPlatformScreen;

struct wl_cursor_image;

namespace QtWayland {
   class qt_surface_extension;
   class wl_text_input_manager;
   class xdg_wm_base;
}

namespace QtWaylandClient {

class QWaylandBuffer;
class QWaylandClientBufferIntegration;
class QWaylandDataDeviceManager;
class QWaylandEventThread;
class QWaylandHardwareIntegration;
class QWaylandInputDevice;
class QWaylandIntegration;
class QWaylandKeyExtension;
class QWaylandScreen;
class QWaylandShellIntegration;
class QWaylandShellSurface;
class QWaylandTouchExtension;
class QWaylandWindow;
class QWaylandWindowManagerIntegration;
class QWaylandXdgShell;

typedef void (*RegistryListener)(void *data, struct wl_registry *registry, uint32_t id, const QString &interface, uint32_t version);

class Q_WAYLAND_CLIENT_EXPORT QWaylandDisplay : public QObject, public QtWayland::wl_registry
{
   CS_OBJECT(QWaylandDisplay)

 public:
   struct RegistryGlobal {
      uint32_t id;
      QString interface;
      uint32_t version;

      struct ::wl_registry *registry;

      RegistryGlobal(uint32_t id_, const QString &interface_, uint32_t version_, struct ::wl_registry *registry_)
         : id(id_), interface(interface_), version(version_), registry(registry_)
      { }
   };

   QWaylandDisplay(QWaylandIntegration *waylandIntegration);
   ~QWaylandDisplay(void);

   // sets a listener, used to enable several listeners at one time
   void addRegistryListener(RegistryListener listener, void *data);

   QtWayland::wl_compositor *compositor() {
      return &m_compositor;
   }

   int compositorVersion() const {
      return m_compositorVersion;
   }

   QWaylandClientBufferIntegration *clientBufferIntegration() const;

   struct ::wl_region *createRegion(const QRegion &qregion);

   QWaylandShellSurface *createShellSurface(QWaylandWindow *window);

   struct ::wl_surface *createSurface(void *handle);
   struct ::wl_subsurface *createSubSurface(QWaylandWindow *window, QWaylandWindow *parent);

   QWaylandInputDevice *currentInputDevice() const {
      return defaultInputDevice();
   }

   static uint32_t currentTimeMillisec();

   QWaylandInputDevice *defaultInputDevice() const;

   QWaylandDataDeviceManager *dndSelectionHandler() const {
      return m_dndSelectionHandler.data();
   }

   void forceRoundTrip();

   QList<RegistryGlobal> globals() const {
      return m_globals;
   }

   void handleWindowActivated(QWaylandWindow *window);
   void handleWindowDeactivated(QWaylandWindow *window);
   void handleKeyboardFocusChanged(QWaylandInputDevice *inputDevice);
   void handleScreenInitialized(QWaylandScreen *screen);
   void handleWindowDestroyed(QWaylandWindow *window);

   QWaylandHardwareIntegration *hardwareIntegration() const {
      return m_hardwareIntegration.data();
   }

   bool hasRegistryGlobal(const QString &interfaceName);

   QList<QWaylandInputDevice *> inputDevices() const {
      return m_inputDevices;
   }

   bool isWindowActivated(const QWaylandWindow *window);

   uint32_t lastInputSerial() const {
      return m_lastInputSerial;
   }

   QWaylandInputDevice *lastInputDevice() const {
      return m_lastInputDevice;
   }

   QWaylandWindow *lastInputWindow() const;

   QWaylandScreen *screenForOutput(struct wl_output *output) const;

   QList<QWaylandScreen *> screens() const {
      return m_screens;
   }

   void setCursor(struct wl_buffer *buffer, struct wl_cursor_image *image, qreal ratio);
   void setCursor(const QSharedPointer<QWaylandBuffer> &buffer, const QPoint &hotSpot, qreal ratio);

   void setLastInputDevice(QWaylandInputDevice *device, uint32_t serial, QWaylandWindow *window);

   QWaylandShellIntegration *shellIntegration() const;

   struct wl_shm *shm() const {
      return m_shm;
   }

   bool supportsWindowDecoration() const;

   QWaylandTouchExtension *touchExtension() const {
      return m_touchExtension.data();
   }

   QtWayland::wl_text_input_manager *textInputManager() const {
      return m_textInputManager.data();
   }

   struct wl_display *wl_display() const {
      return m_display;
   }

   const struct wl_compositor *wl_compositor() const {
      return m_compositor.object();
   }

   struct ::wl_registry *wl_registry() {
      return object();
   }

   QWaylandWindowManagerIntegration *windowManagerIntegration() const;

   QtWayland::qt_surface_extension *windowExtension() const {
      return m_windowExtension.data();
   }

   CS_SLOT_1(Public, void blockingReadEvents())
   CS_SLOT_2(blockingReadEvents)

   CS_SLOT_1(Public, void flushRequests())
   CS_SLOT_2(flushRequests)

 private:
   struct Listener {
      RegistryListener listener;
      void *data;
   };

   void checkError() const;

   void handleWaylandSync();
   void requestWaylandSync();

   void registry_global(uint32_t id, const QString &interface, uint32_t version) override;
   void registry_global_remove(uint32_t id) override;

   void waitForScreens();

   int m_compositorVersion;

   uint32_t m_lastInputSerial;

   QtWayland::wl_compositor m_compositor;

   QSocketNotifier *m_readNotifier;
   QWaylandInputDevice *m_lastInputDevice;
   QWaylandIntegration *m_waylandIntegration;

   QList<QWaylandScreen *> m_screens;
   QList<QWaylandInputDevice *> m_inputDevices;
   QList<Listener> m_registryListeners;

   QScopedPointer<QWaylandDataDeviceManager> m_dndSelectionHandler;
   QScopedPointer<QtWayland::qt_surface_extension> m_windowExtension;
   QScopedPointer<QtWayland::wl_subcompositor> m_subCompositor;
   QScopedPointer<QWaylandTouchExtension> m_touchExtension;
   QScopedPointer<QWaylandKeyExtension> m_keyExtension;
   QScopedPointer<QWaylandWindowManagerIntegration> m_windowManagerIntegration;
   QScopedPointer<QtWayland::wl_text_input_manager> m_textInputManager;
   QScopedPointer<QWaylandHardwareIntegration> m_hardwareIntegration;

   QList<RegistryGlobal> m_globals;

   QPointer<QWaylandWindow> m_lastInputWindow;
   QPointer<QWaylandWindow> m_lastKeyboardFocus;
   QVector<QWaylandWindow *> m_activeWindows;

   QList<QWaylandScreen *> m_waitingScreens;

   struct wl_display  *m_display;
   struct wl_shm      *m_shm;
   struct wl_callback *m_syncCallback;

   static const wl_callback_listener syncCallbackListener;
};

}

#endif
