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
      return &mCompositor;
   }

   int compositorVersion() const {
      return mCompositorVersion;
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
      return mDndSelectionHandler.data();
   }

   void forceRoundTrip();

   QList<RegistryGlobal> globals() const {
      return mGlobals;
   }

   void handleWindowActivated(QWaylandWindow *window);
   void handleWindowDeactivated(QWaylandWindow *window);
   void handleKeyboardFocusChanged(QWaylandInputDevice *inputDevice);
   void handleScreenInitialized(QWaylandScreen *screen);
   void handleWindowDestroyed(QWaylandWindow *window);

   bool hasRegistryGlobal(const QString &interfaceName);

   QList<QWaylandInputDevice *> inputDevices() const {
      return mInputDevices;
   }

   bool isWindowActivated(const QWaylandWindow *window);

   uint32_t lastInputSerial() const {
      return mLastInputSerial;
   }

   QWaylandInputDevice *lastInputDevice() const {
      return mLastInputDevice;
   }

   QWaylandWindow *lastInputWindow() const;

   QWaylandScreen *screenForOutput(struct wl_output *output) const;

   QList<QWaylandScreen *> screens() const {
      return mScreens;
   }

   void setCursor(struct wl_buffer *buffer, struct wl_cursor_image *image, qreal ratio);
   void setCursor(const QSharedPointer<QWaylandBuffer> &buffer, const QPoint &hotSpot, qreal ratio);

   void setLastInputDevice(QWaylandInputDevice *device, uint32_t serial, QWaylandWindow *window);

   QWaylandShellIntegration *shellIntegration() const;

   struct wl_shm *shm() const {
      return mShm;
   }

   bool supportsWindowDecoration() const;

   QWaylandTouchExtension *touchExtension() const {
      return mTouchExtension.data();
   }

   QtWayland::wl_text_input_manager *textInputManager() const {
      return mTextInputManager.data();
   }

   struct wl_display *wl_display() const {
      return mDisplay;
   }

   const struct wl_compositor *wl_compositor() const {
      return mCompositor.object();
   }

   struct ::wl_registry *wl_registry() {
      return object();
   }

   QWaylandWindowManagerIntegration *windowManagerIntegration() const;

   QtWayland::qt_surface_extension *windowExtension() const {
      return mWindowExtension.data();
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

   int mFd;
   int mWritableNotificationFd;
   int mCompositorVersion;

   uint32_t mLastInputSerial;

   QtWayland::wl_compositor mCompositor;

   QSocketNotifier *mReadNotifier;
   QWaylandInputDevice *mLastInputDevice;
   QWaylandIntegration *mWaylandIntegration;

   QList<QWaylandScreen *> mScreens;
   QList<QWaylandInputDevice *> mInputDevices;
   QList<Listener> mRegistryListeners;

   QScopedPointer<QWaylandDataDeviceManager> mDndSelectionHandler;
   QScopedPointer<QtWayland::qt_surface_extension> mWindowExtension;
   QScopedPointer<QtWayland::wl_subcompositor> mSubCompositor;
   QScopedPointer<QWaylandTouchExtension> mTouchExtension;
   QScopedPointer<QWaylandKeyExtension> mKeyExtension;
   QScopedPointer<QWaylandWindowManagerIntegration> mWindowManagerIntegration;
   QScopedPointer<QtWayland::wl_text_input_manager> mTextInputManager;

   QList<RegistryGlobal> mGlobals;

   QPointer<QWaylandWindow> mLastInputWindow;
   QPointer<QWaylandWindow> mLastKeyboardFocus;
   QVector<QWaylandWindow *> mActiveWindows;

   QList<QWaylandScreen *> mWaitingScreens;

   struct wl_display *mDisplay;
   struct wl_shm *mShm;
   struct wl_callback *mSyncCallback;

   static const wl_callback_listener syncCallbackListener;
};

}

#endif
