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

#include <qwayland_window_p.h>

#include <qapplication.h>
#include <qfileinfo.h>
#include <qpointer.h>
#include <qwindow.h>
#include <qwindowsysteminterface.h>

#include <qwayland_abstract_decoration_p.h>
#include <qwayland_buffer_p.h>
#include <qwayland_data_device_p.h>
#include <qwayland_decoration_factory_p.h>
#include <qwayland_display_p.h>
#include <qwayland_inputdevice_p.h>
#include <qwayland_native_interface_p.h>
#include <qwayland_screen_p.h>
#include <qwayland_shellsurface_p.h>
#include <qwayland_shm_backingstore_p.h>
#include <qwayland_subsurface_p.h>
#include <qwayland_toplevel_p.h>

namespace QtWaylandClient {

QWaylandWindow *QWaylandWindow::m_mouseGrab = nullptr;
QVector<QPointer<QWaylandWindow>> QWaylandWindow::m_activePopups;

QWaylandWindow::QWaylandWindow(QWindow *window)
   : QPlatformWindow(window), m_canResize(true), m_inApplyConfigure(false), m_mouseEventsInContentArea(false),
     m_resizeAfterSwap(! qgetenv("QT_WAYLAND_RESIZE_AFTER_SWAP").isNull()), m_resizeDirty(false),
     m_sentInitialResize(false), m_waitToApplyConfigure(false),
     m_screen(QWaylandScreen::waylandScreenFromWindow(window)), m_display(m_screen->display()),
     m_shellSurface(nullptr), m_topLevel(nullptr), m_subSurfaceWindow(nullptr),
     m_windowDecoration(nullptr), m_backingStore(nullptr), m_pendingBuffer(nullptr), m_cursor(Qt::ArrowCursor),
     m_mousePressedInContentArea(Qt::NoButton), m_lastWindowStates(Qt::WindowNoState)
{
   static WId id = 1;

   m_windowId = id;
   ++id;

   initSurface();
}

QWaylandWindow::~QWaylandWindow()
{
   m_display->handleWindowDestroyed(this);

   delete m_windowDecoration;

   if (isInitialized()) {
      reset(false);
   }

   for (auto item : m_display->inputDevices()) {
      item->handleWindowDestroyed(this);
   }

   const QWindow *parent = window();

   for (QWindow *item : QApplication::topLevelWindows()) {
      if (item->transientParent() == parent) {
         QWindowSystemInterface::handleCloseEvent(item);
      }
   }

   if (m_mouseGrab == this) {
      m_mouseGrab = nullptr;
   }
}

void QWaylandWindow::initWindow()
{
   if (window()->type() == Qt::Desktop) {
      return;
   }

   if (! isInitialized()) {
      initSurface();

      QPlatformSurfaceEvent event(QPlatformSurfaceEvent::SurfaceCreated);
      QApplication::sendEvent(window(), &event);
   }

   if (shouldCreateSubSurface()) {
      QWaylandWindow *parent = static_cast<QWaylandWindow *>(QPlatformWindow::parent());

      if (parent->object() != nullptr) {
         if (::wl_subsurface *subSurface = m_display->createSubSurface(this, parent)) {
            m_subSurfaceWindow = new QWaylandSubSurface(this, parent, subSurface);
         }
      }

   } else if (shouldCreateShellSurface()) {
      m_shellSurface = m_display->createShellSurface(this);

      if (window()->type() == Qt::ToolTip || window()->type() == Qt::Popup) {
         // do nothing

      } else {
         m_topLevel = m_shellSurface->topLevel();
      }
   }

   if (m_topLevel != nullptr) {
      setWindowTitle(window()->title());

      // appId is the desktop entry identifier that should follow the reverse DNS convention
      // http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s02.html,
      // Use the application domain if available, otherwise use the executable base name.
      // According to xdg-shell the appId is only the name without the .desktop suffix.

      QFileInfo fi = QCoreApplication::instance()->applicationFilePath();

      QStringList domainName = QCoreApplication::instance()->organizationDomain().split('.', QStringParser::SkipEmptyParts);

      if (domainName.isEmpty()) {
         m_topLevel->setAppId(fi.baseName());

      } else {
         QString appId;

         for (int i = 0; i < domainName.count(); ++i) {
            appId.prepend(QChar('.')).prepend(domainName.at(i));
         }

         appId.append(fi.baseName());
         m_topLevel->setAppId(appId);
      }
   }

   // Enable high-dpi rendering. Scale() returns the screen scale factor and will
   // typically be integer 1 (normal-dpi) or 2 (high-dpi). Call set_buffer_scale()
   // to inform the compositor high-resolution buffers will be provided.

   if (m_display->compositorVersion() >= 3) {
      set_buffer_scale(scale());
   }

   setWindowFlags(window()->flags());
   setGeometry_helper(window()->geometry());
   setMask(window()->mask());

   if (m_topLevel != nullptr) {
      m_topLevel->requestWindowStates(window()->windowState());
   }

   handleContentOrientationChange(window()->contentOrientation());
   m_flags = window()->flags();
}

void QWaylandWindow::initSurface()
{
   QWriteLocker lock(&m_surfaceLock);
   init(m_display->createSurface(static_cast<QtWayland::wl_surface *>(this)));
}

void QWaylandWindow::applyConfigure()
{
   QMutexLocker lock(&m_resizeLock);

   if (m_canResize || ! m_sentInitialResize) {
      applyConfigureNow();
   }

   lock.unlock();

   sendExposeEvent(QRect(QPoint(), geometry().size()));
   QWindowSystemInterface::flushWindowSystemEvents();
}

void QWaylandWindow::applyConfigureLater()
{
   QMutexLocker resizeLocker(&m_resizeLock);

   if (! m_waitToApplyConfigure) {
      m_waitToApplyConfigure = true;
      QMetaObject::invokeMethod(this, "applyConfigure", Qt::QueuedConnection);
   }
}

void QWaylandWindow::applyConfigureNow()
{
   if (! m_waitToApplyConfigure) {
      return;
   }

   if (m_shellSurface != nullptr) {
      m_shellSurface->applyConfigure();
   }

   m_waitToApplyConfigure = false;
}

void QWaylandWindow::attach(QWaylandBuffer *buffer, int x, int y)
{
   if (buffer == nullptr) {
      QtWayland::wl_surface::attach(nullptr, 0, 0);

   } else {
      buffer->setBusy();
      attach(buffer->buffer(), x, y);
   }
}

void QWaylandWindow::attachOffset(QWaylandBuffer *buffer)
{
   attach(buffer, m_offset.x(), m_offset.y());
   m_offset = QPoint();
}

void QWaylandWindow::closePopups(QWaylandWindow *parent)
{
   while (! m_activePopups.isEmpty()) {
      auto popup = m_activePopups.takeLast();

      if (popup == nullptr) {
         continue;
      }

      if (popup == parent) {
         return;
      }
   }
}

void QWaylandWindow::commit(QWaylandBuffer *buffer, QRegion damageRegion)
{
   if (buffer->committed()) {
      return;
   }

   if (! isInitialized()) {
      return;
   }

   attachOffset(buffer);

   for (const QRect &item : damageRegion.rects()) {
      wl_surface::damage(item.x(), item.y(), item.width(), item.height());
   }

   buffer->setCommitted();
   wl_surface::commit();
}

bool QWaylandWindow::createDecoration()
{
   if (! m_display->supportsWindowDecoration()) {
      return false;
   }

   static bool decorationPluginFailed = false;
   bool decoration = false;

   switch (window()->type()) {
      case Qt::Window:
      case Qt::Widget:
      case Qt::Dialog:
      case Qt::Tool:
      case Qt::Drawer:
         decoration = true;
         break;

      default:
         break;
   }

   if (window()->flags() & Qt::FramelessWindowHint || isFullscreen()) {
      decoration = false;
   }

   if (window()->flags() & Qt::BypassWindowManagerHint) {
      decoration = false;
   }

   if (m_subSurfaceWindow) {
      decoration = false;
   }

   if (m_topLevel != nullptr) {
      if (! m_topLevel->wantsDecorations()) {
         decoration = false;
      }
   }

   bool hadDecoration = (m_windowDecoration != nullptr);

   if (decoration && ! decorationPluginFailed) {
      if (m_windowDecoration == nullptr) {
         QStringList decorations = QWaylandDecorationFactory::keys();

         if (decorations.empty()) {
            qWarning("No decoration plugins were available");
            decorationPluginFailed = true;
            return false;
         }

         QString targetKey;
         QByteArray decorationPluginName = qgetenv("QT_WAYLAND_DECORATION");

         if (! decorationPluginName.isEmpty()) {
            targetKey = QString::fromUtf8(decorationPluginName);

            if (! decorations.contains(targetKey)) {
               qWarning() << "Requested decoration " << targetKey << " not found, falling back to default";
               targetKey = QString();
            }
         }

         if (targetKey.isEmpty()) {
            targetKey = decorations.first();
         }

         m_windowDecoration = QWaylandDecorationFactory::create(targetKey, QStringList());

         if (m_windowDecoration == nullptr) {
            qWarning("Unable to create decoration from factory, running with no decorations");
            decorationPluginFailed = true;
            return false;
         }

         m_windowDecoration->setWaylandWindow(this);
      }

   } else {
      delete m_windowDecoration;
      m_windowDecoration = nullptr;
   }

   if (hadDecoration != (m_windowDecoration != nullptr)) {
      for (QWaylandSubSurface *item : m_children) {
         QPoint pos = item->window()->geometry().topLeft();
         QMargins m = frameMargins();

         item->set_position(pos.x() + m.left(), pos.y() + m.top());
      }

      sendExposeEvent(QRect(QPoint(), geometry().size()));
   }

   return (m_windowDecoration != nullptr);
}

void QWaylandWindow::damage(const QRect &rect)
{
   damage(rect.x(), rect.y(), rect.width(), rect.height());
}

QWaylandAbstractDecoration *QWaylandWindow::decoration() const
{
   return m_windowDecoration;
}

qreal QWaylandWindow::devicePixelRatio() const
{
   return screen()->devicePixelRatio();
}

QMargins QWaylandWindow::frameMargins() const
{
   if (m_windowDecoration != nullptr) {
      return m_windowDecoration->margins();
   }

   return QPlatformWindow::frameMargins();
}

QWaylandWindow *QWaylandWindow::fromWlSurface(::wl_surface *surface)
{
   return static_cast<QWaylandWindow *>(static_cast <QtWayland::wl_surface *>(wl_surface_get_user_data(surface)));
}

void QWaylandWindow::handleContentOrientationChange(Qt::ScreenOrientation orientation)
{
   if (m_display->compositorVersion() < 2) {
      return;
   }

   wl_output_transform transform;
   bool isPortrait = window()->screen() && window()->screen()->primaryOrientation() == Qt::PortraitOrientation;

   switch (orientation) {
      case Qt::PrimaryOrientation:
         transform = WL_OUTPUT_TRANSFORM_NORMAL;
         break;

      case Qt::LandscapeOrientation:
         transform = isPortrait ? WL_OUTPUT_TRANSFORM_270 : WL_OUTPUT_TRANSFORM_NORMAL;
         break;

      case Qt::PortraitOrientation:
         transform = isPortrait ? WL_OUTPUT_TRANSFORM_NORMAL : WL_OUTPUT_TRANSFORM_90;
         break;

      case Qt::InvertedLandscapeOrientation:
         transform = isPortrait ? WL_OUTPUT_TRANSFORM_90 : WL_OUTPUT_TRANSFORM_180;
         break;

      case Qt::InvertedPortraitOrientation:
         transform = isPortrait ? WL_OUTPUT_TRANSFORM_180 : WL_OUTPUT_TRANSFORM_270;
         break;

      default:
         // should not get here
         transform = WL_OUTPUT_TRANSFORM_NORMAL;
         break;
   }

   set_buffer_transform(transform);

   // set_buffer_transform is double buffered, need to commit
   wl_surface::commit();
}

void QWaylandWindow::handleExpose(QRegion exposeRegion)
{
   QWindowSystemInterface::handleExposeEvent(window(), exposeRegion);

   if (m_pendingBuffer != nullptr) {
      commit(m_pendingBuffer, m_pendingBufferDamage);

      m_pendingBuffer       = nullptr;
      m_pendingBufferDamage = QRegion();
   }
}

void QWaylandWindow::handleMouse(QWaylandInputDevice *inputDevice, const QWaylandPointerEvent &e)
{
   if (m_windowDecoration != nullptr) {
      handleMouseEventWithDecoration(inputDevice, e);

   } else {
      switch (e.m_type) {
         case QWaylandPointerEvent::EventType::Enter:
            QWindowSystemInterface::handleEnterEvent(window(), e.m_localPos, e.m_globalPos);
            break;

         case QWaylandPointerEvent::EventType::Motion:
            QWindowSystemInterface::handleMouseEvent(window(), e.m_timestamp, e.m_localPos, e.m_globalPos, e.m_buttons, e.m_modifiers);
            break;

         case QWaylandPointerEvent::EventType::Wheel:
            QWindowSystemInterface::handleWheelEvent(window(), e.m_timestamp, e.m_localPos, e.m_globalPos, e.m_pixelDelta, e.m_angleDelta);
            break;
      }
   }

   if (e.m_type == QWaylandPointerEvent::Enter) {
      QRect windowGeometry = window()->frameGeometry();
      windowGeometry.moveTopLeft({0, 0});

      QRect contentGeometry = windowGeometry.marginsRemoved(frameMargins());

      if (contentGeometry.contains(e.m_localPos.toPoint())) {
         restoreMouseCursor(inputDevice);
      }
   }
}

void QWaylandWindow::handleMouseEventWithDecoration(QWaylandInputDevice *inputDevice, const QWaylandPointerEvent &e)
{
   if (m_mousePressedInContentArea == Qt::NoButton &&
         m_windowDecoration->handleMouse(inputDevice, e.m_localPos, e.m_globalPos, e.m_buttons, e.m_modifiers)) {

      if (m_mouseEventsInContentArea) {
         QWindowSystemInterface::handleLeaveEvent(window());
         m_mouseEventsInContentArea = false;
      }

      return;
   }

   QMargins marg = frameMargins();

   QRect windowRect(0 + marg.left(), 0 + marg.top(), geometry().size().width() - marg.right(), geometry().size().height() - marg.bottom());

   if (windowRect.contains(e.m_localPos.toPoint()) || m_mousePressedInContentArea != Qt::NoButton) {
      QPointF localTranslated  = e.m_localPos;
      QPointF globalTranslated = e.m_globalPos;

      localTranslated.setX(localTranslated.x() - marg.left());
      localTranslated.setY(localTranslated.y() - marg.top());
      globalTranslated.setX(globalTranslated.x() - marg.left());
      globalTranslated.setY(globalTranslated.y() - marg.top());

      if (! m_mouseEventsInContentArea) {
         restoreMouseCursor(inputDevice);
         QWindowSystemInterface::handleEnterEvent(window());
      }

      switch (e.m_type) {
         case QWaylandPointerEvent::Enter:
            QWindowSystemInterface::handleEnterEvent(window(), localTranslated, globalTranslated);
            break;

         case QWaylandPointerEvent::Motion:
            QWindowSystemInterface::handleMouseEvent(window(), e.m_timestamp, localTranslated, globalTranslated, e.m_buttons, e.m_modifiers);
            break;

         case QWaylandPointerEvent::Wheel:
            QWindowSystemInterface::handleWheelEvent(window(), e.m_timestamp, localTranslated, globalTranslated, e.m_pixelDelta, e.m_angleDelta);
            break;
      }

      m_mouseEventsInContentArea  = true;
      m_mousePressedInContentArea = e.m_buttons;

   } else {
      if (m_mouseEventsInContentArea) {
         QWindowSystemInterface::handleLeaveEvent(window());
         m_mouseEventsInContentArea = false;
      }
   }
}

void QWaylandWindow::handleMouseLeave(QWaylandInputDevice *inputDevice)
{
   if (m_windowDecoration != nullptr) {
      if (m_mouseEventsInContentArea) {
         QWindowSystemInterface::handleLeaveEvent(window());
      }

   } else {
      QWindowSystemInterface::handleLeaveEvent(window());
   }

   restoreMouseCursor(inputDevice);
}

void QWaylandWindow::handleWindowStatesChanged(Qt::WindowStates newStates)
{
   createDecoration();

   Qt::WindowState state = Qt::WindowState(Qt::WindowStates::int_type(newStates));

   QWindowSystemInterface::handleWindowStateChanged(window(), state);
   m_lastWindowStates = newStates;
}

bool QWaylandWindow::isActive() const
{
   return m_display->isWindowActivated(this);
}

bool QWaylandWindow::isExposed() const
{
   if (! window()->isVisible()) {
      return false;
   }

   if (m_shellSurface != nullptr) {
      return m_shellSurface->isExposed();
   }

   if (m_subSurfaceWindow != nullptr) {
      return m_subSurfaceWindow->parent()->isExposed();
   }

   return ! (shouldCreateShellSurface() || shouldCreateSubSurface());
}

bool QWaylandWindow::isOpaque() const
{
   return window()->requestedFormat().alphaBufferSize() <= 0;
}

QVariant QWaylandWindow::property(const QString &name)
{
   return m_properties.value(name);
}

QVariant QWaylandWindow::property(const QString &name, const QVariant &defaultValue)
{
   return m_properties.value(name, defaultValue);
}

QVariantMap QWaylandWindow::properties() const
{
   return m_properties;
}

void QWaylandWindow::requestActivateWindow()
{
   // Wayland does not have an activation protocol
   // rely on compositor setting keyboard focus based on window stacking
}

void QWaylandWindow::reset(bool sendDestroyEvent)
{
   if (isInitialized() && sendDestroyEvent) {
      QPlatformSurfaceEvent event(QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed);
      QApplication::sendEvent(window(), &event);
   }

   delete m_shellSurface;
   m_shellSurface = nullptr;

   delete m_subSurfaceWindow;
   m_subSurfaceWindow = nullptr;

   m_topLevel = nullptr;

   destroy();

   if (isInitialized()) {
      QWriteLocker lock(&m_surfaceLock);
      destroy();
   }

   m_mask = QRegion();
}

void QWaylandWindow::resizeApplyConfigure(const QSize &size, const QPoint &delta)
{
   QMargins margins = frameMargins();

   int widthWithoutMargins  = qMax(size.width()  - (margins.left()+margins.right()), 1);
   int heightWithoutMargins = qMax(size.height() - (margins.top()+margins.bottom()), 1);

   QRect geometry(windowGeometry().topLeft(), QSize(widthWithoutMargins, heightWithoutMargins));

   m_offset += delta;

   m_inApplyConfigure = true;
   setGeometry(geometry);
   m_inApplyConfigure = false;
}

void QWaylandWindow::restoreMouseCursor(QWaylandInputDevice *device)
{
   setMouseCursor(device, window()->cursor());
}

int QWaylandWindow::scale() const
{
   return waylandScreen()->scale();
}

void QWaylandWindow::sendExposeEvent(const QRect &rect)
{
   if (m_shellSurface == nullptr) {
      QWindowSystemInterface::handleExposeEvent(window(), rect);

   } else if (! m_shellSurface->handleExpose(rect)) {
      QWindowSystemInterface::handleExposeEvent(window(), rect);

   }

   m_lastExposedGeometry = rect;
}

void QWaylandWindow::sendProperty(const QString &name, const QVariant &value)
{
   m_properties.insert(name, value);

   QWaylandNativeInterface *nativeInterface = static_cast<QWaylandNativeInterface *>(QApplication::platformNativeInterface());
   nativeInterface->emitWindowPropertyChanged(this, name);
}

void QWaylandWindow::setCanResize(bool canResize)
{
   QMutexLocker lock(&m_resizeLock);
   m_canResize = canResize;

   if (canResize) {
      if (m_resizeDirty) {
         QWindowSystemInterface::handleGeometryChange(window(), geometry());
      }

      if (m_waitToApplyConfigure) {
         applyConfigureNow();
         sendExposeEvent(QRect(QPoint(), geometry().size()));

      } else if (m_resizeDirty) {
         m_resizeDirty = false;
         sendExposeEvent(QRect(QPoint(), geometry().size()));

      }
   }
}

void QWaylandWindow::setGeometry_helper(const QRect &rect)
{
   QPlatformWindow::setGeometry(QRect(rect.x(), rect.y(),
         qBound(window()->minimumWidth(),  rect.width(),  window()->maximumWidth()),
         qBound(window()->minimumHeight(), rect.height(), window()->maximumHeight())));

   if (m_subSurfaceWindow != nullptr) {
      QMargins margin = QPlatformWindow::parent()->frameMargins();

      m_subSurfaceWindow->set_position(rect.x() + margin.left(), rect.y() + margin.top());
      m_subSurfaceWindow->parent()->window()->requestUpdate();
   }
}

void QWaylandWindow::setGeometry(const QRect &rect)
{
   setGeometry_helper(rect);

   if (window()->isVisible() && rect.isValid()) {
      if (m_windowDecoration != nullptr) {
         m_windowDecoration->update();
      }

      if (m_resizeAfterSwap && windowType() == Egl && m_sentInitialResize) {
         m_resizeDirty = true;
      } else {
         QWindowSystemInterface::handleGeometryChange(window(), geometry());
      }

      m_sentInitialResize = true;
   }

   QRect exposeGeometry = QRect(QPoint(), geometry().size());

   if (isExposed() && ! m_inApplyConfigure && exposeGeometry != m_lastExposedGeometry) {
      sendExposeEvent(exposeGeometry);
   }

   if (m_shellSurface != nullptr && isExposed()) {
      m_shellSurface->setWindowGeometry(windowContentGeometry());
   }
}

void QWaylandWindow::setMask(const QRegion &mask)
{
   if (m_mask == mask) {
      return;
   }

   m_mask = mask;

   if (! isInitialized()) {
      return;
   }

   if (m_mask.isEmpty()) {
      set_input_region(nullptr);

   } else {
      struct ::wl_region *region = m_display->createRegion(m_mask);
      set_input_region(region);
      wl_region_destroy(region);
   }

   wl_surface::commit();
}

void QWaylandWindow::setMouseCursor(QWaylandInputDevice *device, const QCursor &cursor)
{
   device->setCursor(cursor, waylandScreen());
}

bool QWaylandWindow::setMouseGrabEnabled(bool grab)
{
   if (window()->type() != Qt::Popup) {
      qWarning("Plugin only supports grabbing the mouse for popup windows");
      return false;
   }

   m_mouseGrab = grab ? this : nullptr;

   return true;
}

void QWaylandWindow::setParent(const QPlatformWindow *parent)
{
   if (! window()->isVisible()) {
      return;
   }

   QWaylandWindow *oldparent = m_subSurfaceWindow ? m_subSurfaceWindow->parent() : nullptr;

   if (oldparent == parent) {
      return;
   }

   if (m_subSurfaceWindow && parent != nullptr) {
      // new parent, this is a subsurface already
      delete m_subSurfaceWindow;

      QWaylandWindow *p = const_cast<QWaylandWindow *>(static_cast<const QWaylandWindow *>(parent));
      m_subSurfaceWindow = new QWaylandSubSurface(this, p, m_display->createSubSurface(this, p));

   } else {
      // changing role, make a new wl_surface
      reset(true);
      initWindow();
   }
}

void QWaylandWindow::setProperty(const QString &name, const QVariant &value)
{
   m_properties.insert(name, value);

   QWaylandNativeInterface *nativeInterface = static_cast<QWaylandNativeInterface *>(QApplication::platformNativeInterface());
   nativeInterface->emitWindowPropertyChanged(this, name);
}

void QWaylandWindow::setVisible(bool visible)
{
   if (visible) {
      if (window()->type() == Qt::Popup || window()->type() == Qt::ToolTip) {
         m_activePopups.append(QPointer<QWaylandWindow>(this));
      }

      initWindow();
      m_display->flushRequests();
      setGeometry(window()->geometry());

   } else {
      sendExposeEvent(QRect());
      closePopups(this);
      reset(true);
   }
}

void QWaylandWindow::setWindowFlags(Qt::WindowFlags flags)
{
   m_flags = flags;
   createDecoration();
}

void QWaylandWindow::setWindowIcon(const QIcon &icon)
{
   m_windowIcon = icon;

   if (m_windowDecoration != nullptr && window()->isVisible()) {
      m_windowDecoration->update();
   }
}

void QWaylandWindow::setWindowState(Qt::WindowState newStates)
{
   if (m_topLevel != nullptr) {
      m_topLevel->requestWindowStates(Qt::WindowStates(newStates));
   }
}

void QWaylandWindow::setWindowTitle(const QString &title)
{
   if (m_topLevel != nullptr) {
      m_topLevel->setTitle(title);
   }

   if (m_windowDecoration != nullptr && window()->isVisible()) {
      m_windowDecoration->update();
   }
}

QWaylandShellSurface *QWaylandWindow::shellSurface() const
{
   return m_shellSurface;
}

bool QWaylandWindow::shouldCreateShellSurface() const
{
   if (! m_display->shellIntegration()) {
      return false;
   }

   if (shouldCreateSubSurface()) {
      return false;
   }

   if (window()->inherits("QShapedPixmapWindow")) {
      return false;
   }

   if (! qgetenv("QT_WAYLAND_USE_BYPASSWINDOWMANAGERHINT").isNull()) {
      return ! (window()->flags() & Qt::BypassWindowManagerHint);
   }

   return true;
}

bool QWaylandWindow::shouldCreateSubSurface() const
{
   return QPlatformWindow::parent() != nullptr;
}

QWaylandSubSurface *QWaylandWindow::subSurfaceWindow() const
{
   return m_subSurfaceWindow;
}

QSize QWaylandWindow::surfaceSize() const
{
   return geometry().marginsAdded(frameMargins()).size();
}

QWaylandTopLevel *QWaylandWindow::topLevel() const
{
   return m_topLevel;
}

bool QWaylandWindow::touchDragDecoration(QWaylandInputDevice *inputDevice, const QPointF &local,
      const QPointF &global, Qt::TouchPointState state, Qt::KeyboardModifiers mods)
{
   if (m_windowDecoration == nullptr) {
      return false;
   }

   return m_windowDecoration->handleTouch(inputDevice, local, global, state, mods);
}

static QWaylandWindow *findShellSurface(QWindow *window)
{
   while (window != nullptr) {
      auto w = dynamic_cast<QWaylandWindow *>(window->handle());

      if (w != nullptr && w->shellSurface() != nullptr) {
         return w;
      }

      auto tParent = window->transientParent();

      if (tParent == nullptr) {
         window = window->parent();
      } else {
          window = tParent;
      }
   }

   return nullptr;
}

QWaylandWindow *QWaylandWindow::transientParent() const
{
   QWaylandWindow *retval = nullptr;
   QWindow *ancestor = window()->transientParent();

   while (ancestor != nullptr) {
      auto w = static_cast<QWaylandWindow *>(ancestor->handle());

      if (w != nullptr && w->shellSurface()) {
         retval = w;
         break;
      }

      QWindow *obj = ancestor->transientParent();

      if (obj == nullptr) {
         ancestor = ancestor->parent();
      } else {
         ancestor = obj;
      }
   }

   if (QApplication::focusWindow() && (window()->type() == Qt::ToolTip || window()->type() == Qt::Popup)) {
      return findShellSurface(QApplication::focusWindow());
   }

   return retval;
}

void QWaylandWindow::unfocus()
{
   QWaylandInputDevice *inputDevice = m_display->currentInputDevice();

   if (inputDevice != nullptr && inputDevice->dataDevice()) {
      inputDevice->dataDevice()->invalidateSelectionOffer();
   }
}

WId QWaylandWindow::winId() const
{
   return m_windowId;
}

QRect QWaylandWindow::windowContentGeometry() const
{
   return QRect(QPoint(), surfaceSize());
}

}
