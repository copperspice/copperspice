/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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

#include <qwindow.h>

#include <qplatform_cursor.h>
#include <qplatform_integration.h>
#include <qplatform_window.h>
#include <qsurfaceformat.h>

#ifndef QT_NO_OPENGL
#include <qplatform_openglcontext.h>
#include <qopenglcontext.h>
#endif

#include <qscreen.h>
#include <QTimer>
#include <QDebug>
#include <QStyleHints>

#include <qwindow_p.h>
#include <qapplication_p.h>
#include <qhighdpiscaling_p.h>
#include <qevent_p.h>

#ifndef QT_NO_ACCESSIBILITY
#  include <qaccessible.h>
#endif

QWindow::QWindow(QScreen *targetScreen)
   : QSurface(QSurface::Window), d_ptr(new QWindowPrivate())
{
   d_ptr->q_ptr = this;
   Q_D(QWindow);

   d->connectToScreen(targetScreen ? targetScreen : QGuiApplication::primaryScreen());
   d->init();
}

QWindow::QWindow(QWindow *parent)
   : QObject(parent), QSurface(QSurface::Window), d_ptr(new QWindowPrivate())
{
   d_ptr->q_ptr = this;
   Q_D(QWindow);

   d->parentWindow = parent;

   if (! parent) {
      d->connectToScreen(QGuiApplication::primaryScreen());
   }

   d->init();
}

QWindow::QWindow(QWindowPrivate &dd, QWindow *parent)
   : QObject(parent), QSurface(QSurface::Window), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
   Q_D(QWindow);

   d->parentWindow = parent;

   if (! parent) {
      d->connectToScreen(QGuiApplication::primaryScreen());
   }
   d->init();
}

QWindow::~QWindow()
{
   destroy();
   QApplicationPrivate::window_list.removeAll(this);

   if (! QApplicationPrivate::is_app_closing) {
      QApplicationPrivate::instance()->modalWindowList.removeOne(this);
   }
}

void QWindowPrivate::init()
{
   Q_Q(QWindow);

   // If your application aborts here, you are probably creating a QWindow
   // before the screen list is populated.
   if (!parentWindow && !topLevelScreen) {
      qFatal("Cannot create window: no screens available");
      exit(1);
   }
   QGuiApplicationPrivate::window_list.prepend(q);

   requestedFormat = QSurfaceFormat::defaultFormat();
}

QWindow::Visibility QWindow::visibility() const
{
   Q_D(const QWindow);
   return d->visibility;
}

void QWindow::setVisibility(Visibility v)
{
   switch (v) {
      case Hidden:
         hide();
         break;
      case AutomaticVisibility:
         show();
         break;
      case Windowed:
         showNormal();
         break;
      case Minimized:
         showMinimized();
         break;
      case Maximized:
         showMaximized();
         break;
      case FullScreen:
         showFullScreen();
         break;
      default:
         Q_ASSERT(false);
         break;
   }
}

void QWindowPrivate::updateVisibility()
{
   Q_Q(QWindow);

   QWindow::Visibility old = visibility;

   if (visible) {
      switch (windowState) {
         case Qt::WindowMinimized:
            visibility = QWindow::Minimized;
            break;
         case Qt::WindowMaximized:
            visibility = QWindow::Maximized;
            break;
         case Qt::WindowFullScreen:
            visibility = QWindow::FullScreen;
            break;
         case Qt::WindowNoState:
            visibility = QWindow::Windowed;
            break;
         default:
            Q_ASSERT(false);
            break;
      }
   } else {
      visibility = QWindow::Hidden;
   }

   if (visibility != old) {
      emit q->visibilityChanged(visibility);
   }
}

inline bool QWindowPrivate::windowRecreationRequired(QScreen *newScreen) const
{
   Q_Q(const QWindow);
   const QScreen *oldScreen = q->screen();
   return oldScreen != newScreen && (platformWindow || !oldScreen)
      && !(oldScreen && oldScreen->virtualSiblings().contains(newScreen));
}

inline void QWindowPrivate::disconnectFromScreen()
{
   if (topLevelScreen) {
      topLevelScreen = nullptr;
   }
}

void QWindowPrivate::connectToScreen(QScreen *screen)
{
   disconnectFromScreen();
   topLevelScreen = screen;
}

void QWindowPrivate::emitScreenChangedRecursion(QScreen *newScreen)
{
   Q_Q(QWindow);
   emit q->screenChanged(newScreen);

   for (QObject *child : q->children()) {
      if (child->isWindowType()) {
         static_cast<QWindow *>(child)->d_func()->emitScreenChangedRecursion(newScreen);
      }
   }
}

void QWindowPrivate::setTopLevelScreen(QScreen *newScreen, bool recreate)
{
   Q_Q(QWindow);

   if (parentWindow) {
      qWarning("QWindow::setTopLevelScreen() Unable to connect the current (child) window since it already has a parent");
      return;
   }

   if (newScreen != topLevelScreen) {
      const bool shouldRecreate = recreate && windowRecreationRequired(newScreen);
      const bool shouldShow = visibilityOnDestroy && !topLevelScreen;
      if (shouldRecreate && platformWindow) {
         q->destroy();
      }
      connectToScreen(newScreen);
      if (shouldShow) {
         q->setVisible(true);
      } else if (newScreen && shouldRecreate) {
         create(true);
      }
      emitScreenChangedRecursion(newScreen);
   }
}

void QWindowPrivate::create(bool recursive)
{
   Q_Q(QWindow);
   if (platformWindow) {
      return;
   }

   platformWindow = QGuiApplicationPrivate::platformIntegration()->createPlatformWindow(q);
   Q_ASSERT(platformWindow);

   if (!platformWindow) {
      qWarning("QWindow::create() Failed to create a platform window");
      return;
   }

   QObjectList childObjects = q->children();
   for (int i = 0; i < childObjects.size(); i ++) {
      QObject *object = childObjects.at(i);
      if (object->isWindowType()) {
         QWindow *window = static_cast<QWindow *>(object);
         if (recursive) {
            window->d_func()->create(true);
         }
         if (window->d_func()->platformWindow) {
            window->d_func()->platformWindow->setParent(platformWindow);
         }
      }
   }

   QPlatformSurfaceEvent e(QPlatformSurfaceEvent::SurfaceCreated);
   QGuiApplication::sendEvent(q, &e);
}

void QWindowPrivate::clearFocusObject()
{
}

// Allows for manipulating the suggested geometry before a resize/move
// event in derived classes for platforms that support it, for example to
// implement heightForWidth().
QRectF QWindowPrivate::closestAcceptableGeometry(const QRectF &rect) const
{
   (void) rect;
   return QRectF();
}

void QWindow::setSurfaceType(SurfaceType surfaceType)
{
   Q_D(QWindow);
   d->surfaceType = surfaceType;
}

QWindow::SurfaceType QWindow::surfaceType() const
{
   Q_D(const QWindow);
   return d->surfaceType;
}

void QWindow::setVisible(bool visible)
{
   Q_D(QWindow);

   if (d->visible == visible) {
      return;
   }
   d->visible = visible;
   emit visibleChanged(visible);
   d->updateVisibility();

   if (!d->platformWindow) {
      create();
   }

   if (visible) {
      // remove posted quit events when showing a new window
      QCoreApplication::removePostedEvents(qApp, QEvent::Quit);

      if (type() == Qt::Window) {
         QGuiApplicationPrivate *app_priv = QGuiApplicationPrivate::instance();
         QString &firstWindowTitle = app_priv->firstWindowTitle;
         if (!firstWindowTitle.isEmpty()) {
            setTitle(firstWindowTitle);
            firstWindowTitle = QString();
         }
         if (!app_priv->forcedWindowIcon.isNull()) {
            setIcon(app_priv->forcedWindowIcon);
         }

         // Handling of the -qwindowgeometry, -geometry command line arguments
         static bool geometryApplied = false;
         if (!geometryApplied) {
            geometryApplied = true;
            QGuiApplicationPrivate::applyWindowGeometrySpecificationTo(this);
         }
      }

      QShowEvent showEvent;
      QGuiApplication::sendEvent(this, &showEvent);
   }

   if (isModal()) {
      if (visible) {
         QGuiApplicationPrivate::showModalWindow(this);
      } else {
         QGuiApplicationPrivate::hideModalWindow(this);
      }
   }

#ifndef QT_NO_CURSOR
   if (visible && (d->hasCursor || QGuiApplication::overrideCursor())) {
      d->applyCursor();
   }
#endif
   d->platformWindow->setVisible(visible);

   if (!visible) {
      QHideEvent hideEvent;
      QGuiApplication::sendEvent(this, &hideEvent);
   }
}

bool QWindow::isVisible() const
{
   Q_D(const QWindow);

   return d->visible;
}

void QWindow::create()
{
   Q_D(QWindow);
   d->create(false);
}
WId QWindow::winId() const
{
   Q_D(const QWindow);

   if (type() == Qt::ForeignWindow) {
      return WId(property("_q_foreignWinId").value<WId>());
   }

   if (!d->platformWindow) {
      const_cast<QWindow *>(this)->create();
   }

   return d->platformWindow->winId();
}

QWindow *QWindow::parent() const
{
   Q_D(const QWindow);
   return d->parentWindow;
}

void QWindow::setParent(QWindow *parent)
{
   Q_D(QWindow);
   if (d->parentWindow == parent) {
      return;
   }

   QScreen *newScreen = parent ? parent->screen() : screen();
   if (d->windowRecreationRequired(newScreen)) {
      qWarning("QWindow::setParent() Unable to move current window to a new screen");
      return;
   }

   QObject::setParent(parent);
   d->parentWindow = parent;

   if (parent) {
      d->disconnectFromScreen();
   } else {
      d->connectToScreen(newScreen);
   }

   if (d->platformWindow) {
      if (parent && parent->d_func()->platformWindow) {
         d->platformWindow->setParent(parent->d_func()->platformWindow);
      } else {
         d->platformWindow->setParent(nullptr);
      }
   }

   QGuiApplicationPrivate::updateBlockedStatus(this);
}

bool QWindow::isTopLevel() const
{
   Q_D(const QWindow);
   return d->parentWindow == nullptr;
}

bool QWindow::isModal() const
{
   Q_D(const QWindow);
   return d->modality != Qt::NonModal;
}

bool QWindow::cs_isWindowType() const
{
   return true;
}

Qt::WindowModality QWindow::modality() const
{
   Q_D(const QWindow);
   return d->modality;
}

void QWindow::setModality(Qt::WindowModality modality)
{
   Q_D(QWindow);
   if (d->modality == modality) {
      return;
   }
   d->modality = modality;
   emit modalityChanged(modality);
}

void QWindow::setFormat(const QSurfaceFormat &format)
{
   Q_D(QWindow);
   d->requestedFormat = format;
}

QSurfaceFormat QWindow::requestedFormat() const
{
   Q_D(const QWindow);
   return d->requestedFormat;
}

QSurfaceFormat QWindow::format() const
{
   Q_D(const QWindow);
   if (d->platformWindow) {
      return d->platformWindow->format();
   }
   return d->requestedFormat;
}

void QWindow::setFlags(Qt::WindowFlags flags)
{
   Q_D(QWindow);

   if (d->platformWindow) {
      d->platformWindow->setWindowFlags(flags);
   }

   d->m_flags = flags;
}

Qt::WindowFlags QWindow::flags() const
{
   Q_D(const QWindow);
   return d->m_flags;
}

Qt::WindowType QWindow::type() const
{
   Q_D(const QWindow);
   return static_cast<Qt::WindowType>(int(d->m_flags & Qt::WindowType_Mask));
}

void QWindow::setTitle(const QString &title)
{
   Q_D(QWindow);
   bool changed = false;
   if (d->windowTitle != title) {
      d->windowTitle = title;
      changed = true;
   }
   if (d->platformWindow && type() != Qt::Desktop) {
      d->platformWindow->setWindowTitle(title);
   }
   if (changed) {
      emit windowTitleChanged(title);
   }
}

QString QWindow::title() const
{
   Q_D(const QWindow);
   return d->windowTitle;
}

void QWindow::setFilePath(const QString &filePath)
{
   Q_D(QWindow);
   d->windowFilePath = filePath;
   if (d->platformWindow) {
      d->platformWindow->setWindowFilePath(filePath);
   }
}

QString QWindow::filePath() const
{
   Q_D(const QWindow);
   return d->windowFilePath;
}

void QWindow::setIcon(const QIcon &icon)
{
   Q_D(QWindow);
   d->windowIcon = icon;
   if (d->platformWindow) {
      d->platformWindow->setWindowIcon(icon);
   }
   QEvent e(QEvent::WindowIconChange);
   QCoreApplication::sendEvent(this, &e);
}

QIcon QWindow::icon() const
{
   Q_D(const QWindow);
   if (d->windowIcon.isNull()) {
      return QGuiApplication::windowIcon();
   }
   return d->windowIcon;
}

void QWindow::raise()
{
   Q_D(QWindow);
   if (d->platformWindow) {
      d->platformWindow->raise();
   }
}

void QWindow::lower()
{
   Q_D(QWindow);
   if (d->platformWindow) {
      d->platformWindow->lower();
   }
}

void QWindow::setOpacity(qreal level)
{
   Q_D(QWindow);
   if (level == d->opacity) {
      return;
   }
   d->opacity = level;
   if (d->platformWindow) {
      d->platformWindow->setOpacity(level);
      emit opacityChanged(level);
   }
}

qreal QWindow::opacity() const
{
   Q_D(const QWindow);
   return d->opacity;
}

void QWindow::setMask(const QRegion &region)
{
   Q_D(QWindow);
   if (!d->platformWindow) {
      return;
   }
   d->platformWindow->setMask(QHighDpi::toNativeLocalRegion(region, this));
   d->mask = region;
}

QRegion QWindow::mask() const
{
   Q_D(const QWindow);
   return d->mask;
}

void QWindow::requestActivate()
{
   Q_D(QWindow);
   if (flags() & Qt::WindowDoesNotAcceptFocus) {
      qWarning("QWindow::requestActivate() Unable to activate a window with Qt::WindowDoesNotAcceptFocus set");
      return;
   }
   if (d->platformWindow) {
      d->platformWindow->requestActivateWindow();
   }
}

bool QWindow::isExposed() const
{
   Q_D(const QWindow);
   return d->exposed;
}

bool QWindow::isActive() const
{
   Q_D(const QWindow);
   if (! d->platformWindow) {
      return false;
   }

   QWindow *focus = QGuiApplication::focusWindow();

   // Means the whole application lost the focus
   if (! focus) {
      return false;
   }

   if (focus == this) {
      return true;
   }

   if (! parent() && ! transientParent()) {
      return isAncestorOf(focus);
   } else {
      return (parent() && parent()->isActive()) || (transientParent() && transientParent()->isActive());
   }
}

void QWindow::reportContentOrientationChange(Qt::ScreenOrientation orientation)
{
   Q_D(QWindow);
   if (d->contentOrientation == orientation) {
      return;
   }
   if (d->platformWindow) {
      d->platformWindow->handleContentOrientationChange(orientation);
   }
   d->contentOrientation = orientation;
   emit contentOrientationChanged(orientation);
}

Qt::ScreenOrientation QWindow::contentOrientation() const
{
   Q_D(const QWindow);
   return d->contentOrientation;
}

qreal QWindow::devicePixelRatio() const
{
   Q_D(const QWindow);

   // If there is no platform window use the app global devicePixelRatio,
   // which is the the highest devicePixelRatio found on the system
   // screens, and will be correct for single-display systems (a very common case).
   if (!d->platformWindow) {
      return qApp->devicePixelRatio();
   }

   return d->platformWindow->devicePixelRatio() * QHighDpiScaling::factor(this);
}

void QWindow::setWindowState(Qt::WindowState state)
{
   if (state == Qt::WindowActive) {
      qWarning("QWindow::setWindowState() Unable to set the state to Qt::WindowActive");
      return;
   }

   Q_D(QWindow);
   if (d->platformWindow) {
      d->platformWindow->setWindowState(state);
   }
   d->windowState = state;
   emit windowStateChanged(d->windowState);
   d->updateVisibility();
}

Qt::WindowState QWindow::windowState() const
{
   Q_D(const QWindow);
   return d->windowState;
}

void QWindow::setTransientParent(QWindow *parent)
{
   Q_D(QWindow);
   if (parent && !parent->isTopLevel()) {
      qWarning("QWindow::setTransientParent() New parent must be a top level window");
      return;
   }

   d->transientParent = parent;

   QGuiApplicationPrivate::updateBlockedStatus(this);
}

QWindow *QWindow::transientParent() const
{
   Q_D(const QWindow);
   return d->transientParent.data();
}

bool QWindow::isAncestorOf(const QWindow *child, AncestorMode mode) const
{
   if (child->parent() == this || (mode == IncludeTransients && child->transientParent() == this)) {
      return true;
   }

   return (child->parent() && isAncestorOf(child->parent(), mode))
      || (mode == IncludeTransients && child->transientParent() && isAncestorOf(child->transientParent(), mode));
}

QSize QWindow::minimumSize() const
{
   Q_D(const QWindow);
   return d->minimumSize;
}

QSize QWindow::maximumSize() const
{
   Q_D(const QWindow);
   return d->maximumSize;
}

QSize QWindow::baseSize() const
{
   Q_D(const QWindow);
   return d->baseSize;
}

QSize QWindow::sizeIncrement() const
{
   Q_D(const QWindow);
   return d->sizeIncrement;
}

void QWindow::setMinimumSize(const QSize &size)
{
   Q_D(QWindow);
   QSize adjustedSize = QSize(qBound(0, size.width(), QWINDOWSIZE_MAX), qBound(0, size.height(), QWINDOWSIZE_MAX));
   if (d->minimumSize == adjustedSize) {
      return;
   }
   QSize oldSize = d->minimumSize;
   d->minimumSize = adjustedSize;
   if (d->platformWindow && isTopLevel()) {
      d->platformWindow->propagateSizeHints();
   }
   if (d->minimumSize.width() != oldSize.width()) {
      emit minimumWidthChanged(d->minimumSize.width());
   }
   if (d->minimumSize.height() != oldSize.height()) {
      emit minimumHeightChanged(d->minimumSize.height());
   }
}

void QWindow::setX(int newX)
{
   Q_D(QWindow);

   if (x() != newX) {
      setGeometry(QRect(newX, y(), width(), height()));
   } else {
      d->positionAutomatic = false;
   }
}

void QWindow::setY(int newY)
{
   Q_D(QWindow);

   if (y() != newY) {
      setGeometry(QRect(x(), newY, width(), height()));
   } else {
      d->positionAutomatic = false;
   }
}

void QWindow::setWidth(int newWidth)
{
   if (width() != newWidth) {
      resize(newWidth, height());
   }
}

void QWindow::setHeight(int newHeight)
{
   if (height() != newHeight) {
      resize(width(), newHeight);
   }
}

void QWindow::setMinimumWidth(int w)
{
   setMinimumSize(QSize(w, minimumHeight()));
}

void QWindow::setMinimumHeight(int h)
{
   setMinimumSize(QSize(minimumWidth(), h));
}

void QWindow::setMaximumSize(const QSize &size)
{
   Q_D(QWindow);
   QSize adjustedSize = QSize(qBound(0, size.width(), QWINDOWSIZE_MAX), qBound(0, size.height(), QWINDOWSIZE_MAX));
   if (d->maximumSize == adjustedSize) {
      return;
   }
   QSize oldSize = d->maximumSize;
   d->maximumSize = adjustedSize;
   if (d->platformWindow && isTopLevel()) {
      d->platformWindow->propagateSizeHints();
   }
   if (d->maximumSize.width() != oldSize.width()) {
      emit maximumWidthChanged(d->maximumSize.width());
   }
   if (d->maximumSize.height() != oldSize.height()) {
      emit maximumHeightChanged(d->maximumSize.height());
   }
}

void QWindow::setMaximumWidth(int w)
{
   setMaximumSize(QSize(w, maximumHeight()));
}

void QWindow::setMaximumHeight(int h)
{
   setMaximumSize(QSize(maximumWidth(), h));
}

void QWindow::setBaseSize(const QSize &size)
{
   Q_D(QWindow);
   if (d->baseSize == size) {
      return;
   }
   d->baseSize = size;
   if (d->platformWindow && isTopLevel()) {
      d->platformWindow->propagateSizeHints();
   }
}

void QWindow::setSizeIncrement(const QSize &size)
{
   Q_D(QWindow);
   if (d->sizeIncrement == size) {
      return;
   }
   d->sizeIncrement = size;
   if (d->platformWindow && isTopLevel()) {
      d->platformWindow->propagateSizeHints();
   }
}

void QWindow::setGeometry(int x_pos, int y_pos, int w, int h)
{
   setGeometry(QRect(x_pos, y_pos, w, h));
}

void QWindow::setGeometry(const QRect &rect)
{
   Q_D(QWindow);

   d->positionAutomatic = false;
   if (rect == geometry()) {
      return;
   }
   QRect oldRect = geometry();

   d->positionPolicy = QWindowPrivate::WindowFrameExclusive;
   if (d->platformWindow) {
      QRect nativeRect;
      QScreen *newScreen = d->screenForGeometry(rect);

      if (newScreen && isTopLevel()) {
         nativeRect = QHighDpi::toNativePixels(rect, newScreen);
      } else {
         nativeRect = QHighDpi::toNativePixels(rect, this);
      }

      d->platformWindow->setGeometry(nativeRect);

   } else {
      d->geometry = rect;

      if (rect.x() != oldRect.x()) {
         emit xChanged(rect.x());
      }
      if (rect.y() != oldRect.y()) {
         emit yChanged(rect.y());
      }
      if (rect.width() != oldRect.width()) {
         emit widthChanged(rect.width());
      }
      if (rect.height() != oldRect.height()) {
         emit heightChanged(rect.height());
      }
   }
}

QScreen *QWindowPrivate::screenForGeometry(const QRect &newGeometry)
{
   Q_Q(QWindow);

   QScreen *currentScreen = q->screen();
   QScreen *fallback = currentScreen;
   QPoint center = newGeometry.center();

   if (! q->parent() && currentScreen && !currentScreen->geometry().contains(center)) {
      for (QScreen *screen : currentScreen->virtualSiblings()) {
         if (screen->geometry().contains(center)) {
            return screen;
         }

         if (screen->geometry().intersects(newGeometry)) {
            fallback = screen;
         }
      }
   }
   return fallback;
}

QRect QWindow::geometry() const
{
   Q_D(const QWindow);
   if (d->platformWindow) {
      return QHighDpi::fromNativePixels(d->platformWindow->geometry(), this);
   }
   return d->geometry;
}

QMargins QWindow::frameMargins() const
{
   Q_D(const QWindow);
   if (d->platformWindow) {
      return QHighDpi::fromNativePixels(d->platformWindow->frameMargins(), this);
   }
   return QMargins();
}

QRect QWindow::frameGeometry() const
{
   Q_D(const QWindow);
   if (d->platformWindow) {
      QMargins m = frameMargins();
      return QHighDpi::fromNativePixels(d->platformWindow->geometry(), this).adjusted(-m.left(), -m.top(), m.right(), m.bottom());
   }
   return d->geometry;
}

QPoint QWindow::framePosition() const
{
   Q_D(const QWindow);
   if (d->platformWindow) {
      QMargins margins = frameMargins();
      return QHighDpi::fromNativePixels(d->platformWindow->geometry().topLeft(), this) - QPoint(margins.left(), margins.top());
   }
   return d->geometry.topLeft();
}

void QWindow::setFramePosition(const QPoint &point)
{
   Q_D(QWindow);
   d->positionPolicy = QWindowPrivate::WindowFrameInclusive;
   d->positionAutomatic = false;
   if (d->platformWindow) {
      d->platformWindow->setGeometry(QHighDpi::toNativePixels(QRect(point, size()), this));
   } else {
      d->geometry.moveTopLeft(point);
   }
}

void QWindow::setPosition(const QPoint &pt)
{
   setGeometry(QRect(pt, size()));
}

void QWindow::setPosition(int x_pos, int y_pos)
{
   setPosition(QPoint(x_pos, y_pos));
}

void QWindow::resize(int w, int h)
{
   resize(QSize(w, h));
}

void QWindow::resize(const QSize &newSize)
{
   Q_D(QWindow);
   if (d->platformWindow) {
      d->platformWindow->setGeometry(QHighDpi::toNativePixels(QRect(position(), newSize), this));
   } else {
      const QSize oldSize = d->geometry.size();
      d->geometry.setSize(newSize);
      if (newSize.width() != oldSize.width()) {
         emit widthChanged(newSize.width());
      }
      if (newSize.height() != oldSize.height()) {
         emit heightChanged(newSize.height());
      }
   }
}

void QWindow::destroy()
{
   Q_D(QWindow);
   if (!d->platformWindow) {
      return;
   }

   QObjectList childrenWindows = children();
   for (int i = 0; i < childrenWindows.size(); i++) {
      QObject *object = childrenWindows.at(i);
      if (object->isWindowType()) {
         QWindow *w = static_cast<QWindow *>(object);
         w->destroy();
      }
   }

   if (QGuiApplicationPrivate::focus_window == this) {
      QGuiApplicationPrivate::focus_window = parent();
   }
   if (QGuiApplicationPrivate::currentMouseWindow == this) {
      QGuiApplicationPrivate::currentMouseWindow = parent();
   }
   if (QGuiApplicationPrivate::currentMousePressWindow == this) {
      QGuiApplicationPrivate::currentMousePressWindow = parent();
   }
   if (QGuiApplicationPrivate::tabletPressTarget == this) {
      QGuiApplicationPrivate::tabletPressTarget = parent();
   }

   bool wasVisible = isVisible();
   d->visibilityOnDestroy = wasVisible && d->platformWindow;

   setVisible(false);

   QPlatformSurfaceEvent e(QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed);
   QGuiApplication::sendEvent(this, &e);

   delete d->platformWindow;
   d->resizeEventPending = true;
   d->receivedExpose = false;
   d->exposed = false;
   d->platformWindow = nullptr;

   if (wasVisible) {
      d->maybeQuitOnLastWindowClosed();
   }
}

QPlatformWindow *QWindow::handle() const
{
   Q_D(const QWindow);
   return d->platformWindow;
}

QPlatformSurface *QWindow::surfaceHandle() const
{
   Q_D(const QWindow);
   return d->platformWindow;
}

bool QWindow::setKeyboardGrabEnabled(bool grab)
{
   Q_D(QWindow);
   if (d->platformWindow) {
      return d->platformWindow->setKeyboardGrabEnabled(grab);
   }
   return false;
}

bool QWindow::setMouseGrabEnabled(bool grab)
{
   Q_D(QWindow);
   if (d->platformWindow) {
      return d->platformWindow->setMouseGrabEnabled(grab);
   }
   return false;
}

QScreen *QWindow::screen() const
{
   Q_D(const QWindow);
   return d->parentWindow ? d->parentWindow->screen() : d->topLevelScreen.data();
}

void QWindow::setScreen(QScreen *newScreen)
{
   Q_D(QWindow);
   if (!newScreen) {
      newScreen = QGuiApplication::primaryScreen();
   }
   d->setTopLevelScreen(newScreen, newScreen != nullptr);
}

QAccessibleInterface *QWindow::accessibleRoot() const
{
   return nullptr;
}

QObject *QWindow::focusObject() const
{
   return const_cast<QWindow *>(this);
}

void QWindow::show()
{
   Qt::WindowState defaultState = QGuiApplicationPrivate::platformIntegration()->defaultWindowState(d_func()->m_flags);

   if (defaultState == Qt::WindowFullScreen) {
      showFullScreen();
   } else if (defaultState == Qt::WindowMaximized) {
      showMaximized();
   } else {
      showNormal();
   }
}

void QWindow::hide()
{
   setVisible(false);
}

void QWindow::showMinimized()
{
   setWindowState(Qt::WindowMinimized);
   setVisible(true);
}

void QWindow::showMaximized()
{
   setWindowState(Qt::WindowMaximized);
   setVisible(true);
}

void QWindow::showFullScreen()
{
   setWindowState(Qt::WindowFullScreen);
   setVisible(true);

   // activating it here before libscreen activates it causes problems
   requestActivate();
}

void QWindow::showNormal()
{
   setWindowState(Qt::WindowNoState);
   setVisible(true);
}

bool QWindow::close()
{
   Q_D(QWindow);

   // Do not close non top level windows
   if (parent()) {
      return false;
   }

   if (!d->platformWindow) {
      return true;
   }

   bool accepted = false;
   QWindowSystemInterface::handleCloseEvent(this, &accepted);
   QWindowSystemInterface::flushWindowSystemEvents();
   return accepted;
}

void QWindow::exposeEvent(QExposeEvent *ev)
{
   ev->ignore();
}

void QWindow::moveEvent(QMoveEvent *ev)
{
   ev->ignore();
}

void QWindow::resizeEvent(QResizeEvent *ev)
{
   ev->ignore();
}

void QWindow::showEvent(QShowEvent *ev)
{
   ev->ignore();
}

void QWindow::hideEvent(QHideEvent *ev)
{
   ev->ignore();
}

bool QWindow::event(QEvent *ev)
{
   switch (ev->type()) {
      case QEvent::MouseMove:
         mouseMoveEvent(static_cast<QMouseEvent *>(ev));
         break;

      case QEvent::MouseButtonPress:
         mousePressEvent(static_cast<QMouseEvent *>(ev));
         break;

      case QEvent::MouseButtonRelease:
         mouseReleaseEvent(static_cast<QMouseEvent *>(ev));
         break;

      case QEvent::MouseButtonDblClick:
         mouseDoubleClickEvent(static_cast<QMouseEvent *>(ev));
         break;

      case QEvent::TouchBegin:
      case QEvent::TouchUpdate:
      case QEvent::TouchEnd:
      case QEvent::TouchCancel:
         touchEvent(static_cast<QTouchEvent *>(ev));
         break;

      case QEvent::Move:
         moveEvent(static_cast<QMoveEvent *>(ev));
         break;

      case QEvent::Resize:
         resizeEvent(static_cast<QResizeEvent *>(ev));
         break;

      case QEvent::KeyPress:
         keyPressEvent(static_cast<QKeyEvent *>(ev));
         break;

      case QEvent::KeyRelease:
         keyReleaseEvent(static_cast<QKeyEvent *>(ev));
         break;

      case QEvent::FocusIn: {
         focusInEvent(static_cast<QFocusEvent *>(ev));
#ifndef QT_NO_ACCESSIBILITY
         QAccessible::State state;
         state.active = true;
         QAccessibleStateChangeEvent event(this, state);
         QAccessible::updateAccessibility(&event);
#endif
         break;
      }

      case QEvent::FocusOut: {
         focusOutEvent(static_cast<QFocusEvent *>(ev));
#ifndef QT_NO_ACCESSIBILITY
         QAccessible::State state;
         state.active = true;
         QAccessibleStateChangeEvent event(this, state);
         QAccessible::updateAccessibility(&event);
#endif
         break;
      }

#ifndef QT_NO_WHEELEVENT
      case QEvent::Wheel:
         wheelEvent(static_cast<QWheelEvent *>(ev));
         break;
#endif

      case QEvent::Close:
         if (ev->isAccepted()) {
            destroy();
         }
         break;

      case QEvent::Expose:
         exposeEvent(static_cast<QExposeEvent *>(ev));
         break;

      case QEvent::Show:
         showEvent(static_cast<QShowEvent *>(ev));
         break;

      case QEvent::Hide:
         hideEvent(static_cast<QHideEvent *>(ev));
         break;

      case QEvent::ApplicationWindowIconChange:
         setIcon(icon());
         break;

      case QEvent::WindowStateChange: {
         Q_D(QWindow);
         emit windowStateChanged(d->windowState);
         d->updateVisibility();
         break;
      }

#ifndef QT_NO_TABLETEVENT
      case QEvent::TabletPress:
      case QEvent::TabletMove:
      case QEvent::TabletRelease:
         tabletEvent(static_cast<QTabletEvent *>(ev));
         break;
#endif

      case QEvent::Timer: {
         Q_D(QWindow);

         if (static_cast<QTimerEvent *>(ev)->timerId() == d->updateTimer) {
            killTimer(d->updateTimer);
            d->updateTimer = 0;
            d->deliverUpdateRequest();

         } else {
            QObject::event(ev);
         }
         break;
      }

      case QEvent::PlatformSurface: {
         if ((static_cast<QPlatformSurfaceEvent *>(ev))->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed) {
#ifndef QT_NO_OPENGL
            QOpenGLContext *context = QOpenGLContext::currentContext();
            if (context && context->surface() == static_cast<QSurface *>(this)) {
               context->doneCurrent();
            }
#endif
         }
         break;
      }

      default:
         return QObject::event(ev);
   }
   return true;
}

void QWindowPrivate::deliverUpdateRequest()
{
   Q_Q(QWindow);
   updateRequestPending = false;
   QEvent request(QEvent::UpdateRequest);
   QCoreApplication::sendEvent(q, &request);
}

void QWindow::requestUpdate()
{
   Q_ASSERT_X(QThread::currentThread() == QCoreApplication::instance()->thread(),
      "QWindow", "Updates can only be scheduled from the GUI (main) thread");

   Q_D(QWindow);
   if (d->updateRequestPending || !d->platformWindow) {
      return;
   }
   d->updateRequestPending = true;
   d->platformWindow->requestUpdate();
}

void QWindow::keyPressEvent(QKeyEvent *ev)
{
   ev->ignore();
}

void QWindow::keyReleaseEvent(QKeyEvent *ev)
{
   ev->ignore();
}

void QWindow::focusInEvent(QFocusEvent *ev)
{
   ev->ignore();
}

void QWindow::focusOutEvent(QFocusEvent *ev)
{
   ev->ignore();
}

void QWindow::mousePressEvent(QMouseEvent *ev)
{
   ev->ignore();
}

void QWindow::mouseReleaseEvent(QMouseEvent *ev)
{
   ev->ignore();
}

void QWindow::mouseDoubleClickEvent(QMouseEvent *ev)
{
   ev->ignore();
}

void QWindow::mouseMoveEvent(QMouseEvent *ev)
{
   ev->ignore();
}

#ifndef QT_NO_WHEELEVENT

void QWindow::wheelEvent(QWheelEvent *ev)
{
   ev->ignore();
}
#endif

void QWindow::touchEvent(QTouchEvent *ev)
{
   ev->ignore();
}

#ifndef QT_NO_TABLETEVENT

void QWindow::tabletEvent(QTabletEvent *ev)
{
   ev->ignore();
}
#endif

bool QWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
   (void) eventType;
   (void) message;
   (void) result;

   return false;
}

QPoint QWindow::mapToGlobal(const QPoint &pos) const
{
   Q_D(const QWindow);
   // QTBUG-43252, prefer platform implementation for foreign windows.
   if (d->platformWindow
      && (type() == Qt::ForeignWindow || d->platformWindow->isEmbedded())) {
      return QHighDpi::fromNativeLocalPosition(d->platformWindow->mapToGlobal(QHighDpi::toNativeLocalPosition(pos, this)), this);
   }
   return pos + d->globalPosition();
}

QPoint QWindow::mapFromGlobal(const QPoint &pos) const
{
   Q_D(const QWindow);

   // QTBUG-43252, prefer platform implementation for foreign windows

   if (d->platformWindow
      && (type() == Qt::ForeignWindow || d->platformWindow->isEmbedded())) {
      return QHighDpi::fromNativeLocalPosition(d->platformWindow->mapFromGlobal(QHighDpi::toNativeLocalPosition(pos, this)), this);
   }
   return pos - d->globalPosition();
}


Q_GUI_EXPORT QWindowPrivate *qt_window_private(QWindow *window)
{
   return window->d_func();
}

void QWindowPrivate::maybeQuitOnLastWindowClosed()
{
   if (! QCoreApplication::instance()) {
      return;
   }

   Q_Q(QWindow);

   // Attempt to close the application only if this has WA_QuitOnClose set and a non-visible parent
   bool quitOnClose = QGuiApplication::quitOnLastWindowClosed() && !q->parent();
   QWindowList list = QGuiApplication::topLevelWindows();
   bool lastWindowClosed = true;

   for (int i = 0; i < list.size(); ++i) {
      QWindow *w = list.at(i);

      if (! w->isVisible() || w->transientParent() || w->type() == Qt::ToolTip) {
         continue;
      }

      lastWindowClosed = false;
      break;
   }

   if (lastWindowClosed) {
      QGuiApplicationPrivate::emitLastWindowClosed();

      if (quitOnClose) {
         qApp->cs_internal_maybeQuit();
      }
   }
}

void QWindow::cs_internal_updateTimer(int value)
{
   Q_D(QWindow);

   Q_ASSERT(d->updateTimer == 0);
   d->updateTimer = value;
}

QWindow *QWindowPrivate::topLevelWindow() const
{
   Q_Q(const QWindow);

   QWindow *window = const_cast<QWindow *>(q);

   while (window) {
      QWindow *parent = window->parent();
      if (!parent) {
         parent = window->transientParent();
      }

      if (!parent) {
         break;
      }

      window = parent;
   }

   return window;
}

QWindow *QWindow::fromWinId(WId id)
{
   if (!QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::ForeignWindows)) {
      qWarning("QWindow::fromWinId() Platform plugin does not support foreign windows");
      return nullptr;
   }

   QWindow *window = new QWindow;
   window->setFlags(Qt::ForeignWindow);
   window->setProperty("_q_foreignWinId", QVariant::fromValue(id));
   window->create();
   return window;
}

void QWindow::alert(int msec)
{
   Q_D(QWindow);
   if (!d->platformWindow || d->platformWindow->isAlertState() || isActive()) {
      return;
   }

   d->platformWindow->setAlertState(true);
   if (d->platformWindow->isAlertState() && msec) {
      QTimer::singleShot(msec, this, SLOT(_q_clearAlert()));
   }
}

void QWindowPrivate::_q_clearAlert()
{
   if (platformWindow && platformWindow->isAlertState()) {
      platformWindow->setAlertState(false);
   }
}

#ifndef QT_NO_CURSOR

void QWindow::setCursor(const QCursor &cursor)
{
   Q_D(QWindow);
   d->setCursor(&cursor);
}

void QWindow::unsetCursor()
{
   Q_D(QWindow);
   d->setCursor(nullptr);
}

QCursor QWindow::cursor() const
{
   Q_D(const QWindow);
   return d->cursor;
}

void QWindowPrivate::setCursor(const QCursor *newCursor)
{
   Q_Q(QWindow);

   if (newCursor) {
      const Qt::CursorShape newShape = newCursor->shape();

      if (newShape <= Qt::LastCursor && hasCursor && newShape == cursor.shape()) {
         return;   // Unchanged and no bitmap/custom cursor
      }

      cursor = *newCursor;
      hasCursor = true;

   } else {
      if (! hasCursor) {
         return;
      }
      cursor = QCursor(Qt::ArrowCursor);
      hasCursor = false;
   }

   // only attempt to emit signal if there is an actual platform cursor
   if (applyCursor()) {
      QEvent event(QEvent::CursorChange);
      QApplication::sendEvent(q, &event);
   }
}

// Apply the cursor and returns true if the platform cursor exists
bool QWindowPrivate::applyCursor()
{
   Q_Q(QWindow);

   if (QScreen *screen = q->screen()) {
      if (QPlatformCursor *platformCursor = screen->handle()->cursor()) {
         if (! platformWindow) {
            return true;
         }

         QCursor *c = QApplication::overrideCursor();
         if (! c && hasCursor) {
            c = &cursor;
         }

         platformCursor->changeCursor(c, q);
         return true;
      }
   }

   return false;
}
#endif // QT_NO_CURSOR

QVulkanInstance* QWindow::vulkanInstance() const{
   return m_vulkanInstance;
}

void QWindow::setVulkanInstance(QVulkanInstance* instance)
{
   m_vulkanInstance = instance;
}

QDebug operator<<(QDebug debug, const QWindow *window)
{
   QDebugStateSaver saver(debug);
   debug.nospace();

   if (window) {
      debug << window->metaObject()->className() << '(' << (const void *)window;

      if (! window->objectName().isEmpty()) {
         debug << ", Name = " << window->objectName();
      }

      const QRect geometry = window->geometry();

      if (window->isVisible()) {
         debug << ", visible";
      }

      if (window->isExposed()) {
         debug << ", exposed";
      }
      debug << ", State = " << window->windowState()
            << ", Type = " << window->type() << ", Flags =" << window->flags()
            << ", Surface Type = " << window->surfaceType();

      if (window->isTopLevel()) {
         debug << ", toplevel";
      }

      debug << ", " << geometry.width() << 'x' << geometry.height()
         << forcesign << geometry.x() << geometry.y() << noforcesign;

      const QMargins margins = window->frameMargins();

      if (! margins.isNull()) {
         debug << ", Margins = " << margins;
      }

      debug << ", DP Ratio = " << window->devicePixelRatio();

      if (const QPlatformWindow *platformWindow = window->handle()) {
         debug << ", winId = 0x" << hex << platformWindow->winId() << dec;
      }

      if (const QScreen *screen = window->screen()) {
         debug << ", On = " << screen->name();
      }

      debug << ')';

   } else {
      debug << "QWindow(0x0)";
   }

   return debug;
}

void QWindow::_q_clearAlert()
{
   Q_D(QWindow);
   d->_q_clearAlert();
}

