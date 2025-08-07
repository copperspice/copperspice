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
#include <qwayland_inputdevice_p.h>
#include <qwayland_integration_p.h>
#include <qwayland_key_extension_p.h>
#include <qwayland_screen_p.h>
#include <qwayland_shell_integration_p.h>
#include <qwayland_subsurface_p.h>
#include <qwayland_touch_p.h>
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
   : mFd(-1), mWritableNotificationFd(-1), mCompositorVersion(0), mLastInputSerial(0),
     mReadNotifier(nullptr), mLastInputDevice(nullptr), mWaylandIntegration(waylandIntegration),
     mDisplay(nullptr), mShm(nullptr), mSyncCallback(nullptr)
{
   mDisplay = wl_display_connect(nullptr);

   if (mDisplay == nullptr) {
      qErrnoWarning(errno, "Failed to create display");
      ::exit(1);
   }

   struct ::wl_registry *registry = wl_display_get_registry(mDisplay);

   init(registry);
   mWindowManagerIntegration.reset(new QWaylandWindowManagerIntegration(this));

   forceRoundTrip();

   if (! mWaitingScreens.isEmpty()) {
      forceRoundTrip();
   }
}

QWaylandDisplay::~QWaylandDisplay(void)
{
   if (mSyncCallback) {
      wl_callback_destroy(mSyncCallback);
   }

   qDeleteAll(mInputDevices);
   mInputDevices.clear();

   // pending implementation
   for (QWaylandScreen *screen : mWaitingScreens) {
      delete screen;
   }

   mScreens.clear();

   delete mDndSelectionHandler.take();

   if (mDisplay != nullptr) {
      wl_display_disconnect(mDisplay);
   }
}

void QWaylandDisplay::addRegistryListener(RegistryListener listener, void *data)
{
   Listener l = { listener, data };
   mRegistryListeners.append(l);

   for (int i = 0, ie = mGlobals.count(); i != ie; ++i) {
      (*l.listener)(l.data, mGlobals[i].registry, mGlobals[i].id, mGlobals[i].interface, mGlobals[i].version);
   }
}

void QWaylandDisplay::blockingReadEvents()
{
   if (wl_display_dispatch(mDisplay) < 0) {
      checkError();
   }
}

void QWaylandDisplay::checkError() const
{
   int ecode = wl_display_get_error(mDisplay);

   if ((ecode == EPIPE || ecode == ECONNRESET)) {
      qWarning("Wayland connection broke, Wayland compositor appears to have shut down");

   } else {
      qErrnoWarning(ecode, "Wayland connection experienced a fatal error");
   }

   ::exit(1);
}

QWaylandClientBufferIntegration *QWaylandDisplay::clientBufferIntegration() const
{
   // pending implementation

   return nullptr;
}

struct wl_surface *QWaylandDisplay::createSurface(void *handle)
{
   struct wl_surface *surface = mCompositor.create_surface();
   wl_surface_set_user_data(surface, handle);

   return surface;
}

QWaylandShellSurface *QWaylandDisplay::createShellSurface(QWaylandWindow *window)
{
   // pending implementation


   return nullptr;
}

struct ::wl_region *QWaylandDisplay::createRegion(const QRegion &qregion)
{
   struct ::wl_region *region = mCompositor.create_region();

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
   return mInputDevices.isEmpty() ? nullptr : mInputDevices.first();
}

void QWaylandDisplay::flushRequests()
{
   if (wl_display_prepare_read(mDisplay) == 0) {
      wl_display_read_events(mDisplay);
   }

   if (wl_display_dispatch_pending(mDisplay) < 0) {
      checkError();
   }

   wl_display_flush(mDisplay);
}

void QWaylandDisplay::forceRoundTrip()
{
   // wl_display_roundtrip() works on the main queue only,
   // since we use a separate one, reimplement it here

   int retval = 0;
   bool done  = false;

   wl_callback *callback = wl_display_sync(mDisplay);
   wl_callback_add_listener(callback, &sync_listener, &done);

   flushRequests();

   if (QThread::currentThread()->eventDispatcher()) {
      while (! done && retval >= 0) {
         QThread::currentThread()->eventDispatcher()->processEvents(QEventLoop::WaitForMoreEvents);
         retval = wl_display_dispatch_pending(mDisplay);
      }

   } else {
      while (! done && retval >= 0) {
         retval = wl_display_dispatch(mDisplay);
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
   if (mActiveWindows.contains(window)) {
      handleWindowDeactivated(window);
   }
}

void QWaylandDisplay::handleWindowActivated(QWaylandWindow *window)
{
   if (mActiveWindows.contains(window)) {
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
   for (const RegistryGlobal &global : mGlobals) {
      if (global.interface == interfaceName) {
         return true;
      }
   }

   return false;
}

bool QWaylandDisplay::isWindowActivated(const QWaylandWindow *window)
{
   return mActiveWindows.contains(window);
}

QWaylandWindow *QWaylandDisplay::lastInputWindow() const
{
   return nullptr;
}

void QWaylandDisplay::registry_global(uint32_t id, const QString &interface, uint32_t version)
{
   struct ::wl_registry *registry = object();

   if (interface == "wl_output") {
      QWaylandScreen *screen = new QWaylandScreen(this, version, id);
      mWaitingScreens.append(screen);

   } else if (interface == "wl_compositor") {
      mCompositorVersion = qMin((int)version, 3);
      mCompositor.init(registry, id, mCompositorVersion);

   } else if (interface == "wl_shm") {
      mShm = static_cast<struct wl_shm *>(wl_registry_bind(registry, id, &wl_shm_interface, 1));

   // pending implementation

   } else if (interface == "wl_data_device_manager") {
      mDndSelectionHandler.reset(new QWaylandDataDeviceManager(this, id));

   } else if (interface == "qt_surface_extension") {
      mWindowExtension.reset(new QtWayland::qt_surface_extension(registry, id, 1));

   } else if (interface == "wl_subcompositor") {
      mSubCompositor.reset(new QtWayland::wl_subcompositor(registry, id, 1));

   } else if (interface == "qt_touch_extension") {
      mTouchExtension.reset(new QWaylandTouchExtension(this, id));

   } else if (interface == "qt_key_extension") {
      mKeyExtension.reset(new QWaylandKeyExtension(this, id));

   } else if (interface == "wl_text_input_manager") {
      mTextInputManager.reset(new QtWayland::wl_text_input_manager(registry, id, 1));

   } else if (interface == "qt_hardware_integration") {

      // pending implementation
   }

   mGlobals.append(RegistryGlobal(id, interface, version, registry));

   auto cloneRegistry = mRegistryListeners;

   for (Listener item : cloneRegistry) {
      (*item.listener)(item.data, registry, id, interface, version);
   }
}

void QWaylandDisplay::registry_global_remove(uint32_t id)
{
   for (int i = 0, ie = mGlobals.count(); i != ie; ++i) {
      RegistryGlobal &global = mGlobals[i];

      if (global.id == id) {
         if (global.interface == "wl_output") {
            for (QWaylandScreen *screen : mWaitingScreens) {
               if (screen->outputId() == id) {
                  mWaitingScreens.removeOne(screen);
                  delete screen;
                  break;
               }
            }

            // pending implementation

         }

         mGlobals.removeAt(i);
         break;
      }
   }
}

void QWaylandDisplay::requestWaylandSync()
{
   if (mSyncCallback) {
      return;
   }

   mSyncCallback = wl_display_sync(mDisplay);
   wl_callback_add_listener(mSyncCallback, &syncCallbackListener, this);
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
      display->mSyncCallback = nullptr;
      display->handleWaylandSync();
   }
};

QWaylandScreen *QWaylandDisplay::screenForOutput(struct wl_output *output) const
{
   for (int i = 0; i < mScreens.size(); ++i) {
      QWaylandScreen *screen = static_cast < QWaylandScreen * > (mScreens.at(i));

      if (screen->output() == output) {
         return screen;
      }
   }

   return nullptr;
}

void QWaylandDisplay::setCursor(struct wl_buffer *buffer, struct wl_cursor_image *image, qreal ratio)
{
   for (int i = 0; i < mInputDevices.count(); i++) {
      QWaylandInputDevice *inputDevice = mInputDevices.at(i);
      inputDevice->setCursor(buffer, image, ratio);
   }
}

void QWaylandDisplay::setCursor(const QSharedPointer<QWaylandBuffer> &buffer, const QPoint &hotSpot, qreal ratio)
{
   for (int i = 0; i < mInputDevices.count(); i++) {
      QWaylandInputDevice *inputDevice = mInputDevices.at(i);
      inputDevice->setCursor(buffer, hotSpot, ratio);
   }
}

void QWaylandDisplay::setLastInputDevice(QWaylandInputDevice *device, uint32_t serial, QWaylandWindow *win)
{
   // pending implementation
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
      bool screensReady = ! mScreens.isEmpty();

      for (int ii = 0; screensReady && ii < mScreens.count(); ++ii) {
         if (mScreens.at(ii)->geometry() == QRect(0, 0, 0, 0)) {
            screensReady = false;
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
   return mWindowManagerIntegration.data();
}

}
