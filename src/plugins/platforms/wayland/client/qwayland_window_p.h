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

#ifndef QWAYLAND_WINDOW_H
#define QWAYLAND_WINDOW_H

#include <qicon.h>
#include <qmutex.h>
#include <qregion.h>
#include <qplatform_window.h>
#include <qreadwritelock.h>
#include <qvariant.h>
#include <qwaitcondition.h>

#include <qwayland-wayland.h>

struct wl_egl_window;

namespace QtWaylandClient {

class QWaylandAbstractDecoration;
class QWaylandBuffer;
class QWaylandDisplay;
class QWaylandInputDevice;
class QWaylandPointerEvent;
class QWaylandScreen;
class QWaylandShellSurface;
class QWaylandShmBackingStore;
class QWaylandSubSurface;
class QWaylandTopLevel;

class Q_WAYLAND_CLIENT_EXPORT QWaylandWindow : public QObject, public QPlatformWindow, public QtWayland::wl_surface
{
   CS_OBJECT(QWaylandWindow)

 public:
   enum WindowType {
      Shm,
      Egl
   };

   QWaylandWindow(QWindow *window);
   ~QWaylandWindow();

   void applyConfigureLater();
   void applyConfigureNow();

   using QtWayland::wl_surface::attach;
   void attach(QWaylandBuffer *buffer, int x, int y);

   void attachOffset(QWaylandBuffer *buffer);
   QPoint attachOffset() const;

   QWaylandShmBackingStore *backingStore() const {
      return m_backingStore;
   }

   using QtWayland::wl_surface::commit;
   void commit(QWaylandBuffer *buffer, QRegion damageRegion);

   void closePopups(QWaylandWindow *parent);
   bool createDecoration();

   QWaylandAbstractDecoration *decoration() const;

   using QtWayland::wl_surface::damage;
   void damage(const QRect &rect);

   qreal devicePixelRatio() const override;

   QWaylandDisplay *display() const {
      return m_display;
   }

   QMargins frameMargins() const override;

   static QWaylandWindow *fromWlSurface(::wl_surface *surface);

   void handleExpose(QRegion exposeRegion);
   void handleContentOrientationChange(Qt::ScreenOrientation orientation) override;
   void handleMouse(QWaylandInputDevice *inputDevice, const QWaylandPointerEvent &e);
   void handleMouseLeave(QWaylandInputDevice *inputDevice);
   void handleWindowStatesChanged(Qt::WindowStates newStates);

   bool isActive() const override;
   bool isExposed() const override;

   bool isFullscreen() const {
      return m_lastWindowStates & Qt::WindowFullScreen;
   }

   bool isMaximized() const {
      return m_lastWindowStates & Qt::WindowMaximized;
   }

   static QWaylandWindow *mouseGrab() {
      return m_mouseGrab;
   }

   QVariantMap properties() const;
   QVariant property(const QString &name);
   QVariant property(const QString &name, const QVariant &defaultValue);

   void requestActivateWindow() override;
   void restoreMouseCursor(QWaylandInputDevice *device);

   void resizeApplyConfigure(const QSize &size, const QPoint &delta = QPoint(0,0));

   QMutex *resizeMutex() {
      return &m_resizeLock;
   }

   int scale() const;

   QWaylandShellSurface *shellSurface() const;

   void sendProperty(const QString &name, const QVariant &value);

   void setBackingStore(QWaylandShmBackingStore *backingStore) {
      m_backingStore = backingStore;
   }

   void setCanResize(bool canResize);
   void setGeometry(const QRect &rect) override;
   void setMask(const QRegion &region) override;
   void setMouseCursor(QWaylandInputDevice *device, const QCursor &cursor);
   void setParent(const QPlatformWindow *parent) override;
   void setProperty(const QString &name, const QVariant &value);

   void setVisible(bool visible) override;
   void setWindowFlags(Qt::WindowFlags flags) override;
   void setWindowState(Qt::WindowState newStates) override;
   void setWindowTitle(const QString &title) override;

   bool setKeyboardGrabEnabled(bool) override {
      return false;
   }

   bool setMouseGrabEnabled(bool grab) override;

   QWaylandSubSurface *subSurfaceWindow() const;
   QSize surfaceSize() const;

   bool touchDragDecoration(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global,
         Qt::TouchPointState state, Qt::KeyboardModifiers mods);

   void unfocus();

   QWaylandScreen *waylandScreen() const {
      return m_screen;
   }

   virtual WindowType windowType() const = 0;

   inline QIcon windowIcon() const;
   void setWindowIcon(const QIcon &icon) override;

   QWaylandTopLevel *topLevel() const;
   QWaylandWindow *transientParent() const;

   WId winId() const override;
   QRect windowContentGeometry() const;

   CS_SLOT_1(Public, void applyConfigure())
   CS_SLOT_2(applyConfigure)

 protected:
   bool m_canResize;
   bool m_inApplyConfigure;
   bool m_mouseEventsInContentArea;
   bool m_resizeAfterSwap;
   bool m_resizeDirty;
   bool m_sentInitialResize;
   bool m_waitToApplyConfigure;

   QWaylandScreen *m_screen;
   QWaylandDisplay *m_display;

   QWaylandShellSurface *m_shellSurface;
   QWaylandTopLevel *m_topLevel;
   QWaylandSubSurface *m_subSurfaceWindow;

   QWaylandAbstractDecoration *m_windowDecoration;
   QWaylandShmBackingStore *m_backingStore;

   QMutex m_resizeLock;

   QVector<QWaylandSubSurface *> m_children;
   QVariantMap m_properties;

   QWaylandBuffer *m_pendingBuffer;
   QRegion m_pendingBufferDamage;

   QCursor m_cursor;
   QPoint m_offset;
   QIcon m_windowIcon;
   QRegion m_mask;

   Qt::MouseButtons m_mousePressedInContentArea;
   Qt::WindowFlags m_flags;
   Qt::WindowStates m_lastWindowStates;

   WId m_windowId;

 private:
   void handleMouseEventWithDecoration(QWaylandInputDevice *inputDevice, const QWaylandPointerEvent &e);

   void initSurface();
   void initWindow();
   bool isOpaque() const;

   void reset(bool sendDestroyEvent);

   void sendExposeEvent(const QRect &rect);
   void setGeometry_helper(const QRect &rect);

   bool shouldCreateShellSurface() const;
   bool shouldCreateSubSurface() const;

   QRect m_lastExposedGeometry;
   QReadWriteLock m_surfaceLock;

   static QWaylandWindow *m_mouseGrab;
   static QVector<QPointer<QWaylandWindow>> m_activePopups;

   friend class QWaylandSubSurface;
};

inline QPoint QWaylandWindow::attachOffset() const
{
   return m_offset;
}

inline QIcon QWaylandWindow::windowIcon() const
{
   return m_windowIcon;
}

}

#endif
