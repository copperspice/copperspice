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

#include <qwayland_display_p.h>

#include <qabstracteventdispatcher.h>

#include <qwayland-text.h>
#include <qwayland-xdg-shell.h>

#include <qapplication_p.h>
#include <qwayland_abstract_decoration_p.h>
#include <qwayland_clientbuffer_integration_p.h>
#include <qwayland_clipboard_p.h>
#include <qwayland_cursor_p.h>
#include <qwayland_data_devicemanager_p.h>
#include <qwayland_extended_surface_p.h>
#include <qwayland_hardware_integration_p.h>
#include <qwayland_inputdevice_p.h>
#include <qwayland_integration_p.h>
#include <qwayland_key_extension_p.h>
#include <qwayland_screen_p.h>
#include <qwayland_shell_integration_p.h>
#include <qwayland_subsurface_p.h>
#include <qwayland_touch_p.h>
#include <qwayland_window_p.h>
#include <qwayland_windowmanager_integration_p.h>

#include <errno.h>

namespace QtWaylandClient {

static void sync_callback(void *data, struct wl_callback *callback, uint32_t serial)
{
   (void) serial;
   bool *done = static_cast < bool * > (data);

   *done = true;
   wl_callback_destroy(callback);
}

static const struct wl_callback_listener sync_listener = {
   sync_callback
};

QWaylandDisplay::QWaylandDisplay(QWaylandIntegration *waylandIntegration)
   : m_compositorVersion(0), m_lastInputSerial(0), m_readNotifier(nullptr), m_lastInputDevice(nullptr),
     m_waylandIntegration(waylandIntegration), m_display(nullptr), m_shm(nullptr), m_syncCallback(nullptr)
{
   m_display = wl_display_connect(nullptr);

   if (m_display == nullptr) {
      qErrnoWarning(errno, "Failed to create display");
      ::exit(1);
   }

   struct ::wl_registry *registry = wl_display_get_registry(m_display);

   init(registry);
   m_windowManagerIntegration.reset(new QWaylandWindowManagerIntegration(this));

   forceRoundTrip();

   if (! m_waitingScreens.isEmpty()) {
      forceRoundTrip();
   }
}

QWaylandDisplay::~QWaylandDisplay(void)
{
   if (m_syncCallback) {
      wl_callback_destroy(m_syncCallback);
   }

   qDeleteAll(m_inputDevices);
   m_inputDevices.clear();

   for (QWaylandScreen *screen : m_screens) {
      m_waylandIntegration->destroyPlatformScreen(screen);
   }

   for (QWaylandScreen *screen : m_waitingScreens) {
      delete screen;
   }

   m_screens.clear();

   delete m_dndSelectionHandler.take();

   if (m_display != nullptr) {
      wl_display_disconnect(m_display);
   }
}

void QWaylandDisplay::addRegistryListener(RegistryListener listener, void *data)
{
   Listener obj = { listener, data };
   m_registryListeners.append(obj);

   for (const auto &item : m_globals) {
      (*obj.listener)(obj.data, item.registry, item.id, item.interface, item.version);
   }
}

void QWaylandDisplay::blockingReadEvents()
{
   if (wl_display_dispatch(m_display) < 0) {
      checkError();
   }
}

void QWaylandDisplay::checkError() const
{
   int ecode = wl_display_get_error(m_display);

   if ((ecode == EPIPE || ecode == ECONNRESET)) {
      qWarning("Wayland connection broke, Wayland compositor appears to have shut down");

   } else {
      qErrnoWarning(ecode, "Wayland connection experienced a fatal error");
   }

   ::exit(1);
}

QWaylandClientBufferIntegration *QWaylandDisplay::clientBufferIntegration() const
{
   return m_waylandIntegration->clientBufferIntegration();
}

struct wl_surface *QWaylandDisplay::createSurface(void *handle)
{
   struct wl_surface *surface = m_compositor.create_surface();
   wl_surface_set_user_data(surface, handle);

   return surface;
}

QWaylandShellSurface *QWaylandDisplay::createShellSurface(QWaylandWindow *window)
{
   if (m_waylandIntegration->shellIntegration() == nullptr) {
      return nullptr;
   }

   return m_waylandIntegration->shellIntegration()->createShellSurface(window);
}

struct ::wl_region *QWaylandDisplay::createRegion(const QRegion &qregion)
{
   struct ::wl_region *region = m_compositor.create_region();

   for (const QRect &rect : qregion.rects()) {
      wl_region_add(region, rect.x(), rect.y(), rect.width(), rect.height());
   }

   return region;
}

::wl_subsurface *QWaylandDisplay::createSubSurface(QWaylandWindow *window, QWaylandWindow *parent)
{
   // pending implementation

   return nullptr;
}

uint32_t QWaylandDisplay::currentTimeMillisec()
{
   // throw away time information
   struct timeval tv;

   int retval = gettimeofday(&tv, nullptr);

   if (retval == 0) {
      return tv.tv_sec * 1000 + tv.tv_usec / 1000;
   }

   return 0;
}

QWaylandInputDevice *QWaylandDisplay::defaultInputDevice() const
{
   return m_inputDevices.isEmpty() ? nullptr : m_inputDevices.first();
}

void QWaylandDisplay::flushRequests()
{
   if (wl_display_prepare_read(m_display) == 0) {
      wl_display_read_events(m_display);
   }

   if (wl_display_dispatch_pending(m_display) < 0) {
      checkError();
   }

   wl_display_flush(m_display);
}

void QWaylandDisplay::forceRoundTrip()
{
   // wl_display_roundtrip() works on the main queue only,
   // since we use a separate one, reimplement it here

   int retval = 0;
   bool done  = false;

   wl_callback *callback = wl_display_sync(m_display);
   wl_callback_add_listener(callback, &sync_listener, &done);

   flushRequests();

   if (QThread::currentThread()->eventDispatcher()) {
      while (! done && retval >= 0) {
         QThread::currentThread()->eventDispatcher()->processEvents(QEventLoop::WaitForMoreEvents);
         retval = wl_display_dispatch_pending(m_display);
      }

   } else {
      while (! done && retval >= 0) {
         retval = wl_display_dispatch(m_display);
      }
   }

   if (retval == -1 && ! done) {
      wl_callback_destroy(callback);
   }
}

void QWaylandDisplay::handleKeyboardFocusChanged(QWaylandInputDevice *inputDevice)
{
   // pending implementation
}

void QWaylandDisplay::handleWindowDestroyed(QWaylandWindow *window)
{
   if (m_activeWindows.contains(window)) {
      handleWindowDeactivated(window);
   }
}

void QWaylandDisplay::handleWindowActivated(QWaylandWindow *window)
{
   if (m_activeWindows.contains(window)) {
      return;
   }

   // pending implementation
}

void QWaylandDisplay::handleWindowDeactivated(QWaylandWindow *window)
{
   // pending implementation
}

void QWaylandDisplay::handleWaylandSync()
{

   // pending implementation
}

void QWaylandDisplay::handleScreenInitialized(QWaylandScreen *screen)
{
   // pending implementation
}

bool QWaylandDisplay::hasRegistryGlobal(const QString &interfaceName)
{
   for (const RegistryGlobal &global : m_globals) {
      if (global.interface == interfaceName) {
         return true;
      }
   }

   return false;
}

bool QWaylandDisplay::isWindowActivated(const QWaylandWindow *window)
{
   return m_activeWindows.contains(window);
}

QWaylandWindow *QWaylandDisplay::lastInputWindow() const
{
   return m_lastInputWindow.data();
}

void QWaylandDisplay::registry_global(uint32_t id, const QString &interface, uint32_t version)
{
   struct ::wl_registry *registry = object();

   if (interface == "wl_output") {
      QWaylandScreen *screen = new QWaylandScreen(this, version, id);
      m_waitingScreens.append(screen);

   } else if (interface == "wl_compositor") {
      m_compositorVersion = qMin((int)version, 3);
      m_compositor.init(registry, id, m_compositorVersion);

   } else if (interface == "wl_shm") {
      m_shm = static_cast<struct wl_shm *>(wl_registry_bind(registry, id, &wl_shm_interface, 1));

   } else if (interface == "wl_seat") {
      QWaylandInputDevice *inputDevice = m_waylandIntegration->createInputDevice(this, version, id);
      m_inputDevices.append(inputDevice);

   } else if (interface == "wl_data_device_manager") {
      m_dndSelectionHandler.reset(new QWaylandDataDeviceManager(this, id));

   } else if (interface == "qt_surface_extension") {
      m_windowExtension.reset(new QtWayland::qt_surface_extension(registry, id, 1));

   } else if (interface == "wl_subcompositor") {
      m_subCompositor.reset(new QtWayland::wl_subcompositor(registry, id, 1));

   } else if (interface == "qt_touch_extension") {
      m_touchExtension.reset(new QWaylandTouchExtension(this, id));

   } else if (interface == "qt_key_extension") {
      m_keyExtension.reset(new QWaylandKeyExtension(this, id));

   } else if (interface == "wl_text_input_manager") {
      m_textInputManager.reset(new QtWayland::wl_text_input_manager(registry, id, 1));

   } else if (interface == "qt_hardware_integration") {
      m_hardwareIntegration.reset(new QWaylandHardwareIntegration(registry, id));

     forceRoundTrip();
   }

   m_globals.append(RegistryGlobal(id, interface, version, registry));

   auto cloneRegistry = m_registryListeners;

   for (const Listener &item : cloneRegistry) {
      (*item.listener)(item.data, registry, id, interface, version);
   }
}

void QWaylandDisplay::registry_global_remove(uint32_t id)
{
   for (int i = 0; i < m_globals.count(); ++i) {
      RegistryGlobal &global = m_globals[i];

      if (global.id == id) {
         if (global.interface == "wl_output") {

            for (QWaylandScreen *screen : m_waitingScreens) {
               if (screen->outputId() == id) {
                  m_waitingScreens.removeOne(screen);
                  delete screen;
                  break;
               }
            }

            // pending implementation

         }

         m_globals.removeAt(i);
         break;
      }
   }
}

void QWaylandDisplay::requestWaylandSync()
{
   if (m_syncCallback != nullptr) {
      return;
   }

   m_syncCallback = wl_display_sync(m_display);
   wl_callback_add_listener(m_syncCallback, &syncCallbackListener, this);
}

bool QWaylandDisplay::supportsWindowDecoration() const
{
   static bool disabled = qgetenv("QT_WAYLAND_DISABLE_WINDOWDECORATION").toInt();

   // Stop early when disabled via the environment. Do not try to load the integration in
   // order to play nice with SHM-only, buffer integration-less systems.

   if (disabled) {
      return false;
   }

   static bool integrationSupport = clientBufferIntegration() && clientBufferIntegration()->supportsWindowDecoration();

   return integrationSupport;
}

const wl_callback_listener QWaylandDisplay::syncCallbackListener = {
   [](void *data, struct wl_callback * callback, uint32_t time)
   {
      (void) time;

      wl_callback_destroy(callback);

      QWaylandDisplay *display = static_cast<QWaylandDisplay *>(data);
      display->m_syncCallback = nullptr;
      display->handleWaylandSync();
   }
};

QWaylandScreen *QWaylandDisplay::screenForOutput(struct wl_output *output) const
{
   for (auto item : m_screens) {
      QWaylandScreen *screenItem = static_cast<QWaylandScreen *>(item);

      if (screenItem->output() == output) {
         return screenItem;
      }
   }

   return nullptr;
}

void QWaylandDisplay::setCursor(struct wl_buffer *buffer, struct wl_cursor_image *image, qreal ratio)
{
   for (auto item : m_inputDevices) {
      item->setCursor(buffer, image, ratio);
   }
}

void QWaylandDisplay::setCursor(const QSharedPointer<QWaylandBuffer> &buffer, const QPoint &hotSpot, qreal ratio)
{
   for (auto item : m_inputDevices) {
      item->setCursor(buffer, hotSpot, ratio);
   }
}

void QWaylandDisplay::setLastInputDevice(QWaylandInputDevice *device, uint32_t serial, QWaylandWindow *win)
{
   m_lastInputDevice = device;
   m_lastInputSerial = serial;
   m_lastInputWindow = win;
}

QWaylandShellIntegration *QWaylandDisplay::shellIntegration() const
{
   // pending implementation

   return nullptr;
}

void QWaylandDisplay::waitForScreens()
{
   flushRequests();

   while (true) {
      bool screensReady = ! m_screens.isEmpty();

      for (auto item : m_screens) {
         if (item->geometry() == QRect(0, 0, 0, 0)) {
            screensReady = false;
            break;
         }
      }

      if (! screensReady) {
         blockingReadEvents();
      } else {
         return;
      }
   }
}

QWaylandWindowManagerIntegration *QWaylandDisplay::windowManagerIntegration() const
{
   return m_windowManagerIntegration.data();
}

}
