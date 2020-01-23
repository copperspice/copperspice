/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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
      topLevelScreen = 0;
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
      qWarning() << q << '(' << newScreen << "): Attempt to set a screen on a child window.";
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
      qWarning() << "Failed to create platform window for" << q << "with flags" << q->flags();
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
   Q_UNUSED(rect)
   return QRectF();
}

/*!
    Sets the \a surfaceType of the window.

    Specifies whether the window is meant for raster rendering with
    QBackingStore, or OpenGL rendering with QOpenGLContext.

    The surfaceType will be used when the native surface is created
    in the create() function. Calling this function after the native
    surface has been created requires calling destroy() and create()
    to release the old native surface and create a new one.

    \sa QBackingStore, QOpenGLContext, create(), destroy()
*/
void QWindow::setSurfaceType(SurfaceType surfaceType)
{
   Q_D(QWindow);
   d->surfaceType = surfaceType;
}

/*!
    Returns the surface type of the window.

    \sa setSurfaceType()
*/
QWindow::SurfaceType QWindow::surfaceType() const
{
   Q_D(const QWindow);
   return d->surfaceType;
}

/*!
    \property QWindow::visible
    \brief whether the window is visible or not

    This property controls the visibility of the window in the windowing system.

    By default, the window is not visible, you must call setVisible(true), or
    show() or similar to make it visible.

    \sa show()
*/
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

/*!
    Allocates the platform resources associated with the window.

    It is at this point that the surface format set using setFormat() gets resolved
    into an actual native surface. However, the window remains hidden until setVisible() is called.

    Note that it is not usually necessary to call this function directly, as it will be implicitly
    called by show(), setVisible(), and other functions that require access to the platform
    resources.

    Call destroy() to free the platform resources if necessary.

    \sa destroy()
*/
void QWindow::create()
{
   Q_D(QWindow);
   d->create(false);
}

/*!
    Returns the window's platform id.

    For platforms where this id might be useful, the value returned
    will uniquely represent the window inside the corresponding screen.

    \sa screen()
*/
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

/*!
    Returns the parent window, if any.

    A window without a parent is known as a top level window.
*/
QWindow *QWindow::parent() const
{
   Q_D(const QWindow);
   return d->parentWindow;
}

/*!
    Sets the \a parent Window. This will lead to the windowing system managing
    the clip of the window, so it will be clipped to the \a parent window.

    Setting \a parent to be 0 will make the window become a top level window.

    If \a parent is a window created by fromWinId(), then the current window
    will be embedded inside \a parent, if the platform supports it.
*/
void QWindow::setParent(QWindow *parent)
{
   Q_D(QWindow);
   if (d->parentWindow == parent) {
      return;
   }

   QScreen *newScreen = parent ? parent->screen() : screen();
   if (d->windowRecreationRequired(newScreen)) {
      qWarning() << this << '(' << parent << "): Cannot change screens (" << screen() << newScreen << ')';
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
         d->platformWindow->setParent(0);
      }
   }

   QGuiApplicationPrivate::updateBlockedStatus(this);
}

/*!
    Returns whether the window is top level, i.e. has no parent window.
*/
bool QWindow::isTopLevel() const
{
   Q_D(const QWindow);
   return d->parentWindow == 0;
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

/*! \fn void QWindow::modalityChanged(Qt::WindowModality modality)

    This signal is emitted when the Qwindow::modality property changes to \a modality.
*/

/*!
    Sets the window's surface \a format.

    The format determines properties such as color depth, alpha, depth and
    stencil buffer size, etc. For example, to give a window a transparent
    background (provided that the window system supports compositing, and
    provided that other content in the window does not make it opaque again):

    \code
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    window.setFormat(format);
    \endcode

    The surface format will be resolved in the create() function. Calling
    this function after create() has been called will not re-resolve the
    surface format of the native surface.

    When the format is not explicitly set via this function, the format returned
    by QSurfaceFormat::defaultFormat() will be used. This means that when having
    multiple windows, individual calls to this function can be replaced by one
    single call to QSurfaceFormat::setDefaultFormat() before creating the first
    window.

    \sa create(), destroy(), QSurfaceFormat::setDefaultFormat()
*/
void QWindow::setFormat(const QSurfaceFormat &format)
{
   Q_D(QWindow);
   d->requestedFormat = format;
}

/*!
    Returns the requested surface format of this window.

    If the requested format was not supported by the platform implementation,
    the requestedFormat will differ from the actual window format.

    This is the value set with setFormat().

    \sa setFormat(), format()
 */
QSurfaceFormat QWindow::requestedFormat() const
{
   Q_D(const QWindow);
   return d->requestedFormat;
}

/*!
    Returns the actual format of this window.

    After the window has been created, this function will return the actual surface format
    of the window. It might differ from the requested format if the requested format could
    not be fulfilled by the platform. It might also be a superset, for example certain
    buffer sizes may be larger than requested.

    \note Depending on the platform, certain values in this surface format may still
    contain the requested values, that is, the values that have been passed to
    setFormat(). Typical examples are the OpenGL version, profile and options. These may
    not get updated during create() since these are context specific and a single window
    may be used together with multiple contexts over its lifetime. Use the
    QOpenGLContext's format() instead to query such values.

    \sa create(), requestedFormat(), QOpenGLContext::format()
*/
QSurfaceFormat QWindow::format() const
{
   Q_D(const QWindow);
   if (d->platformWindow) {
      return d->platformWindow->format();
   }
   return d->requestedFormat;
}

/*!
    \property QWindow::flags
    \brief the window flags of the window

    The window flags control the window's appearance in the windowing system,
    whether it's a dialog, popup, or a regular window, and whether it should
    have a title bar, etc.

    The actual window flags might differ from the flags set with setFlags()
    if the requested flags could not be fulfilled.
*/
void QWindow::setFlags(Qt::WindowFlags flags)
{
   Q_D(QWindow);
   if (d->platformWindow) {
      d->platformWindow->setWindowFlags(flags);
   }
   d->windowFlags = flags;
}

Qt::WindowFlags QWindow::flags() const
{
   Q_D(const QWindow);
   return d->windowFlags;
}

/*!
    Returns the type of the window.

    This returns the part of the window flags that represents
    whether the window is a dialog, tooltip, popup, regular window, etc.

    \sa flags(), setFlags()
*/
Qt::WindowType QWindow::type() const
{
   Q_D(const QWindow);
   return static_cast<Qt::WindowType>(int(d->windowFlags & Qt::WindowType_Mask));
}

/*!
    \property QWindow::title
    \brief the window's title in the windowing system

    The window title might appear in the title area of the window decorations,
    depending on the windowing system and the window flags. It might also
    be used by the windowing system to identify the window in other contexts,
    such as in the task switcher.

    \sa flags()
*/
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

/*!
    \brief set the file name this window is representing.

    The windowing system might use \a filePath to display the
    path of the document this window is representing in the tile bar.

*/
void QWindow::setFilePath(const QString &filePath)
{
   Q_D(QWindow);
   d->windowFilePath = filePath;
   if (d->platformWindow) {
      d->platformWindow->setWindowFilePath(filePath);
   }
}

/*!
    \brief the file name this window is representing.

    \sa setFilePath()
*/
QString QWindow::filePath() const
{
   Q_D(const QWindow);
   return d->windowFilePath;
}

/*!
    \brief Sets the window's \a icon in the windowing system

    The window icon might be used by the windowing system for example to
    decorate the window, and/or in the task switcher.
*/
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

/*!
    \brief Sets the window's icon in the windowing system

    \sa setIcon()
*/
QIcon QWindow::icon() const
{
   Q_D(const QWindow);
   if (d->windowIcon.isNull()) {
      return QGuiApplication::windowIcon();
   }
   return d->windowIcon;
}

/*!
    Raise the window in the windowing system.

    Requests that the window be raised to appear above other windows.
*/
void QWindow::raise()
{
   Q_D(QWindow);
   if (d->platformWindow) {
      d->platformWindow->raise();
   }
}

/*!
    Lower the window in the windowing system.

    Requests that the window be lowered to appear below other windows.
*/
void QWindow::lower()
{
   Q_D(QWindow);
   if (d->platformWindow) {
      d->platformWindow->lower();
   }
}

/*!
    \property QWindow::opacity
    \brief The opacity of the window in the windowing system.
    \since 5.1

    If the windowing system supports window opacity, this can be used to fade the
    window in and out, or to make it semitransparent.

    A value of 1.0 or above is treated as fully opaque, whereas a value of 0.0 or below
    is treated as fully transparent. Values inbetween represent varying levels of
    translucency between the two extremes.

    The default value is 1.0.
*/
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

/*!
    Sets the mask of the window.

    The mask is a hint to the windowing system that the application does not
    want to receive mouse or touch input outside the given \a region.

    The window manager may or may not choose to display any areas of the window
    not included in the mask, thus it is the application's responsibility to
    clear to transparent the areas that are not part of the mask.

    Setting the mask before the window has been created has no effect.
*/
void QWindow::setMask(const QRegion &region)
{
   Q_D(QWindow);
   if (!d->platformWindow) {
      return;
   }
   d->platformWindow->setMask(QHighDpi::toNativeLocalRegion(region, this));
   d->mask = region;
}

/*!
    Returns the mask set on the window.

    The mask is a hint to the windowing system that the application does not
    want to receive mouse or touch input outside the given region.
*/
QRegion QWindow::mask() const
{
   Q_D(const QWindow);
   return d->mask;
}

/*!
    Requests the window to be activated, i.e. receive keyboard focus.

    \sa isActive(), QGuiApplication::focusWindow()
*/
void QWindow::requestActivate()
{
   Q_D(QWindow);
   if (flags() & Qt::WindowDoesNotAcceptFocus) {
      qWarning() << "requestActivate() called for " << this << " which has Qt::WindowDoesNotAcceptFocus set.";
      return;
   }
   if (d->platformWindow) {
      d->platformWindow->requestActivateWindow();
   }
}

/*!
    Returns if this window is exposed in the windowing system.

    When the window is not exposed, it is shown by the application
    but it is still not showing in the windowing system, so the application
    should minimize rendering and other graphical activities.

    An exposeEvent() is sent every time this value changes.

    \sa exposeEvent()
*/
bool QWindow::isExposed() const
{
   Q_D(const QWindow);
   return d->exposed;
}

/*!
    \property QWindow::active
    \brief the active status of the window
    \since 5.1

    \sa requestActivate()
*/

/*!
    Returns \c true if the window should appear active from a style perspective.

    This is the case for the window that has input focus as well as windows
    that are in the same parent / transient parent chain as the focus window.

    To get the window that currently has focus, use QGuiApplication::focusWindow().
*/
bool QWindow::isActive() const
{
   Q_D(const QWindow);
   if (!d->platformWindow) {
      return false;
   }

   QWindow *focus = QGuiApplication::focusWindow();

   // Means the whole application lost the focus
   if (!focus) {
      return false;
   }

   if (focus == this) {
      return true;
   }

   if (!parent() && !transientParent()) {
      return isAncestorOf(focus);
   } else {
      return (parent() && parent()->isActive()) || (transientParent() && transientParent()->isActive());
   }
}

/*!
    \property QWindow::contentOrientation
    \brief the orientation of the window's contents

    This is a hint to the window manager in case it needs to display
    additional content like popups, dialogs, status bars, or similar
    in relation to the window.

    The recommended orientation is QScreen::orientation() but
    an application doesn't have to support all possible orientations,
    and thus can opt to ignore the current screen orientation.

    The difference between the window and the content orientation
    determines how much to rotate the content by. QScreen::angleBetween(),
    QScreen::transformBetween(), and QScreen::mapBetween() can be used
    to compute the necessary transform.

    The default value is Qt::PrimaryOrientation
*/
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

/*!
    Returns the ratio between physical pixels and device-independent pixels
    for the window. This value is dependent on the screen the window is on,
    and may change when the window is moved.

    Common values are 1.0 on normal displays and 2.0 on Apple "retina" displays.

    \note For windows not backed by a platform window, meaning that create() was not
    called, the function will fall back to QGuiApplication::devicePixelRatio() which in
    turn returns the highest screen device pixel ratio found on the system.

    \sa QScreen::devicePixelRatio(), QGuiApplication::devicePixelRatio()
*/
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

/*!
    \brief set the screen-occupation state of the window

    The window \a state represents whether the window appears in the
    windowing system as maximized, minimized, fullscreen, or normal.

    The enum value Qt::WindowActive is not an accepted parameter.

    \sa showNormal(), showFullScreen(), showMinimized(), showMaximized()
*/
void QWindow::setWindowState(Qt::WindowState state)
{
   if (state == Qt::WindowActive) {
      qWarning() << "QWindow::setWindowState does not accept Qt::WindowActive";
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

/*!
    \brief the screen-occupation state of the window

    \sa setWindowState()
*/
Qt::WindowState QWindow::windowState() const
{
   Q_D(const QWindow);
   return d->windowState;
}

/*!
    \fn QWindow::windowStateChanged(Qt::WindowState windowState)

    This signal is emitted when the \a windowState changes, either
    by being set explicitly with setWindowState(), or automatically when
    the user clicks one of the titlebar buttons or by other means.
*/

/*!
    Sets the transient \a parent

    This is a hint to the window manager that this window is a dialog or pop-up
    on behalf of the given window.

    In order to cause the window to be centered above its transient parent by
    default, depending on the window manager, it may also be necessary to call
    setFlags() with a suitable \l Qt::WindowType (such as \c Qt::Dialog).

    \sa transientParent(), parent()
*/
void QWindow::setTransientParent(QWindow *parent)
{
   Q_D(QWindow);
   if (parent && !parent->isTopLevel()) {
      qWarning() << parent << "must be a top level window.";
      return;
   }

   d->transientParent = parent;

   QGuiApplicationPrivate::updateBlockedStatus(this);
}

/*!
    Returns the transient parent of the window.

    \sa setTransientParent(), parent()
*/
QWindow *QWindow::transientParent() const
{
   Q_D(const QWindow);
   return d->transientParent.data();
}

/*!
    \enum QWindow::AncestorMode

    This enum is used to control whether or not transient parents
    should be considered ancestors.

    \value ExcludeTransients Transient parents are not considered ancestors.
    \value IncludeTransients Transient parents are considered ancestors.
*/

/*!
    Returns \c true if the window is an ancestor of the given \a child. If \a mode
    is IncludeTransients, then transient parents are also considered ancestors.
*/
bool QWindow::isAncestorOf(const QWindow *child, AncestorMode mode) const
{
   if (child->parent() == this || (mode == IncludeTransients && child->transientParent() == this)) {
      return true;
   }

   return (child->parent() && isAncestorOf(child->parent(), mode))
      || (mode == IncludeTransients && child->transientParent() && isAncestorOf(child->transientParent(), mode));
}

/*!
    Returns the minimum size of the window.

    \sa setMinimumSize()
*/
QSize QWindow::minimumSize() const
{
   Q_D(const QWindow);
   return d->minimumSize;
}

/*!
    Returns the maximum size of the window.

    \sa setMaximumSize()
*/
QSize QWindow::maximumSize() const
{
   Q_D(const QWindow);
   return d->maximumSize;
}

/*!
    Returns the base size of the window.

    \sa setBaseSize()
*/
QSize QWindow::baseSize() const
{
   Q_D(const QWindow);
   return d->baseSize;
}

/*!
    Returns the size increment of the window.

    \sa setSizeIncrement()
*/
QSize QWindow::sizeIncrement() const
{
   Q_D(const QWindow);
   return d->sizeIncrement;
}

/*!
    Sets the minimum size of the window.

    This is a hint to the window manager to prevent resizing below the specified \a size.

    \sa setMaximumSize(), minimumSize()
*/
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

/*!
    \property QWindow::x
    \brief the x position of the window's geometry
*/
void QWindow::setX(int arg)
{
   Q_D(QWindow);
   if (x() != arg) {
      setGeometry(QRect(arg, y(), width(), height()));
   } else {
      d->positionAutomatic = false;
   }
}

/*!
    \property QWindow::y
    \brief the y position of the window's geometry
*/
void QWindow::setY(int arg)
{
   Q_D(QWindow);
   if (y() != arg) {
      setGeometry(QRect(x(), arg, width(), height()));
   } else {
      d->positionAutomatic = false;
   }
}

/*!
    \property QWindow::width
    \brief the width of the window's geometry
*/
void QWindow::setWidth(int arg)
{
   if (width() != arg) {
      resize(arg, height());
   }
}

/*!
    \property QWindow::height
    \brief the height of the window's geometry
*/
void QWindow::setHeight(int arg)
{
   if (height() != arg) {
      resize(width(), arg);
   }
}

/*!
    \property QWindow::minimumWidth
    \brief the minimum width of the window's geometry
*/
void QWindow::setMinimumWidth(int w)
{
   setMinimumSize(QSize(w, minimumHeight()));
}

/*!
    \property QWindow::minimumHeight
    \brief the minimum height of the window's geometry
*/
void QWindow::setMinimumHeight(int h)
{
   setMinimumSize(QSize(minimumWidth(), h));
}

/*!
    Sets the maximum size of the window.

    This is a hint to the window manager to prevent resizing above the specified \a size.

    \sa setMinimumSize(), maximumSize()
*/
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

/*!
    \property QWindow::maximumWidth
    \brief the maximum width of the window's geometry
*/
void QWindow::setMaximumWidth(int w)
{
   setMaximumSize(QSize(w, maximumHeight()));
}

/*!
    \property QWindow::maximumHeight
    \brief the maximum height of the window's geometry
*/
void QWindow::setMaximumHeight(int h)
{
   setMaximumSize(QSize(maximumWidth(), h));
}

/*!
    Sets the base \a size of the window.

    The base size is used to calculate a proper window size if the
    window defines sizeIncrement().

    \sa setMinimumSize(), setMaximumSize(), setSizeIncrement(), baseSize()
*/
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

/*!
    Sets the size increment (\a size) of the window.

    When the user resizes the window, the size will move in steps of
    sizeIncrement().width() pixels horizontally and
    sizeIncrement().height() pixels vertically, with baseSize() as the
    basis.

    By default, this property contains a size with zero width and height.

    The windowing system might not support size increments.

    \sa setBaseSize(), setMinimumSize(), setMaximumSize()
*/
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

/*!
    Sets the geometry of the window, excluding its window frame, to a
    rectangle constructed from \a posx, \a posy, \a w and \a h.

    \sa geometry()
*/
void QWindow::setGeometry(int posx, int posy, int w, int h)
{
   setGeometry(QRect(posx, posy, w, h));
}

/*!
    \brief Sets the geometry of the window, excluding its window frame, to \a rect.

    \sa geometry()
*/
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

/*
  This is equivalent to QPlatformWindow::screenForGeometry, but in platform
  independent coordinates. The duplication is unfortunate, but there is a
  chicken and egg problem here: we cannot convert to native coordinates
  before we know which screen we are on.
*/
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


/*!
    Returns the geometry of the window, excluding its window frame.

    \sa frameMargins(), frameGeometry()
*/
QRect QWindow::geometry() const
{
   Q_D(const QWindow);
   if (d->platformWindow) {
      return QHighDpi::fromNativePixels(d->platformWindow->geometry(), this);
   }
   return d->geometry;
}

/*!
    Returns the window frame margins surrounding the window.

    \sa geometry(), frameGeometry()
*/
QMargins QWindow::frameMargins() const
{
   Q_D(const QWindow);
   if (d->platformWindow) {
      return QHighDpi::fromNativePixels(d->platformWindow->frameMargins(), this);
   }
   return QMargins();
}

/*!
    Returns the geometry of the window, including its window frame.

    \sa geometry(), frameMargins()
*/
QRect QWindow::frameGeometry() const
{
   Q_D(const QWindow);
   if (d->platformWindow) {
      QMargins m = frameMargins();
      return QHighDpi::fromNativePixels(d->platformWindow->geometry(), this).adjusted(-m.left(), -m.top(), m.right(), m.bottom());
   }
   return d->geometry;
}

/*!
    Returns the top left position of the window, including its window frame.

    This returns the same value as frameGeometry().topLeft().

    \sa geometry(), frameGeometry()
*/
QPoint QWindow::framePosition() const
{
   Q_D(const QWindow);
   if (d->platformWindow) {
      QMargins margins = frameMargins();
      return QHighDpi::fromNativePixels(d->platformWindow->geometry().topLeft(), this) - QPoint(margins.left(), margins.top());
   }
   return d->geometry.topLeft();
}

/*!
    Sets the upper left position of the window (\a point) including its window frame.

    \sa setGeometry(), frameGeometry()
*/
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

/*!
    \brief set the position of the window on the desktop to \a pt

    \sa position()
*/
void QWindow::setPosition(const QPoint &pt)
{
   setGeometry(QRect(pt, size()));
}

/*!
    \brief set the position of the window on the desktop to \a posx, \a posy

    \sa position()
*/
void QWindow::setPosition(int posx, int posy)
{
   setPosition(QPoint(posx, posy));
}

/*!
    \fn QPoint QWindow::position() const
    \brief Returns the position of the window on the desktop excluding any window frame

    \sa setPosition()
*/

/*!
    \fn QSize QWindow::size() const
    \brief Returns the size of the window excluding any window frame

    \sa resize()
*/

/*!
    set the size of the window, excluding any window frame, to a QSize
    constructed from width \a w and height \a h

    \sa size(), geometry()
*/
void QWindow::resize(int w, int h)
{
   resize(QSize(w, h));
}

/*!
    \brief set the size of the window, excluding any window frame, to \a newSize

    \sa size(), geometry()
*/
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

/*!
    Releases the native platform resources associated with this window.

    \sa create()
*/
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
   d->platformWindow = 0;

   if (wasVisible) {
      d->maybeQuitOnLastWindowClosed();
   }
}

/*!
    Returns the platform window corresponding to the window.

    \internal
*/
QPlatformWindow *QWindow::handle() const
{
   Q_D(const QWindow);
   return d->platformWindow;
}

/*!
    Returns the platform surface corresponding to the window.

    \internal
*/
QPlatformSurface *QWindow::surfaceHandle() const
{
   Q_D(const QWindow);
   return d->platformWindow;
}

/*!
    Sets whether keyboard grab should be enabled or not (\a grab).

    If the return value is true, the window receives all key events until
    setKeyboardGrabEnabled(false) is called; other windows get no key events at
    all. Mouse events are not affected. Use setMouseGrabEnabled() if you want
    to grab that.

    \sa setMouseGrabEnabled()
*/
bool QWindow::setKeyboardGrabEnabled(bool grab)
{
   Q_D(QWindow);
   if (d->platformWindow) {
      return d->platformWindow->setKeyboardGrabEnabled(grab);
   }
   return false;
}

/*!
    Sets whether mouse grab should be enabled or not (\a grab).

    If the return value is true, the window receives all mouse events until setMouseGrabEnabled(false) is
    called; other windows get no mouse events at all. Keyboard events are not affected.
    Use setKeyboardGrabEnabled() if you want to grab that.

    \sa setKeyboardGrabEnabled()
*/
bool QWindow::setMouseGrabEnabled(bool grab)
{
   Q_D(QWindow);
   if (d->platformWindow) {
      return d->platformWindow->setMouseGrabEnabled(grab);
   }
   return false;
}

/*!
    Returns the screen on which the window is shown, or null if there is none.

    For child windows, this returns the screen of the corresponding top level window.

    \sa setScreen(), QScreen::virtualSiblings()
*/
QScreen *QWindow::screen() const
{
   Q_D(const QWindow);
   return d->parentWindow ? d->parentWindow->screen() : d->topLevelScreen.data();
}

/*!
    Sets the screen on which the window should be shown.

    If the window has been created, it will be recreated on the \a newScreen.

    Note that if the screen is part of a virtual desktop of multiple screens,
    the window can appear on any of the screens returned by QScreen::virtualSiblings().

    This function only works for top level windows.

    \sa screen(), QScreen::virtualSiblings()
*/
void QWindow::setScreen(QScreen *newScreen)
{
   Q_D(QWindow);
   if (!newScreen) {
      newScreen = QGuiApplication::primaryScreen();
   }
   d->setTopLevelScreen(newScreen, newScreen != 0);
}

/*!
    \fn QWindow::screenChanged(QScreen *screen)

    This signal is emitted when a window's \a screen changes, either
    by being set explicitly with setScreen(), or automatically when
    the window's screen is removed.
*/

/*!
  Returns the accessibility interface for the object that the window represents
  \internal
  \sa QAccessible
  */
QAccessibleInterface *QWindow::accessibleRoot() const
{
   return 0;
}

/*!
    \fn QWindow::focusObjectChanged(QObject *object)

    This signal is emitted when the final receiver of events tied to focus
    is changed to \a object.

    \sa focusObject()
*/

/*!
    Returns the QObject that will be the final receiver of events tied focus, such
    as key events.
*/
QObject *QWindow::focusObject() const
{
   return const_cast<QWindow *>(this);
}

/*!
    Shows the window.

    This is equivalent to calling showFullScreen(), showMaximized(), or showNormal(),
    depending on the platform's default behavior for the window type and flags.

    \sa showFullScreen(), showMaximized(), showNormal(), hide(), QStyleHints::showIsFullScreen(), flags()
*/
void QWindow::show()
{
   Qt::WindowState defaultState = QGuiApplicationPrivate::platformIntegration()->defaultWindowState(d_func()->windowFlags);
   if (defaultState == Qt::WindowFullScreen) {
      showFullScreen();
   } else if (defaultState == Qt::WindowMaximized) {
      showMaximized();
   } else {
      showNormal();
   }
}

/*!
    Hides the window.

    Equivalent to calling setVisible(false).

    \sa show(), setVisible()
*/
void QWindow::hide()
{
   setVisible(false);
}

/*!
    Shows the window as minimized.

    Equivalent to calling setWindowState(Qt::WindowMinimized) and then
    setVisible(true).

    \sa setWindowState(), setVisible()
*/
void QWindow::showMinimized()
{
   setWindowState(Qt::WindowMinimized);
   setVisible(true);
}

/*!
    Shows the window as maximized.

    Equivalent to calling setWindowState(Qt::WindowMaximized) and then
    setVisible(true).

    \sa setWindowState(), setVisible()
*/
void QWindow::showMaximized()
{
   setWindowState(Qt::WindowMaximized);
   setVisible(true);
}

/*!
    Shows the window as fullscreen.

    Equivalent to calling setWindowState(Qt::WindowFullScreen) and then
    setVisible(true).

    \sa setWindowState(), setVisible()
*/
void QWindow::showFullScreen()
{
   setWindowState(Qt::WindowFullScreen);
   setVisible(true);
#if !defined Q_OS_QNX // On QNX this window will be activated anyway from libscreen
   // activating it here before libscreen activates it causes problems
   requestActivate();
#endif
}

/*!
    Shows the window as normal, i.e. neither maximized, minimized, nor fullscreen.

    Equivalent to calling setWindowState(Qt::WindowNoState) and then
    setVisible(true).

    \sa setWindowState(), setVisible()
*/
void QWindow::showNormal()
{
   setWindowState(Qt::WindowNoState);
   setVisible(true);
}

/*!
    Close the window.

    This closes the window, effectively calling destroy(), and potentially
    quitting the application. Returns \c true on success, false if it has a parent
    window (in which case the top level window should be closed instead).

    \sa destroy(), QGuiApplication::quitOnLastWindowClosed()
*/
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

/*!
    The expose event (\a ev) is sent by the window system whenever the window's
    exposure on screen changes.

    The application can start rendering into the window with QBackingStore
    and QOpenGLContext as soon as it gets an exposeEvent() such that
    isExposed() is true.

    If the window is moved off screen, is made totally obscured by another
    window, iconified or similar, this function might be called and the
    value of isExposed() might change to false. When this happens,
    an application should stop its rendering as it is no longer visible
    to the user.

    A resize event will always be sent before the expose event the first time
    a window is shown.

    \sa isExposed()
*/
void QWindow::exposeEvent(QExposeEvent *ev)
{
   ev->ignore();
}

/*!
    Override this to handle window move events (\a ev).
*/
void QWindow::moveEvent(QMoveEvent *ev)
{
   ev->ignore();
}

/*!
    Override this to handle resize events (\a ev).

    The resize event is called whenever the window is resized in the windowing system,
    either directly through the windowing system acknowledging a setGeometry() or resize() request,
    or indirectly through the user resizing the window manually.
*/
void QWindow::resizeEvent(QResizeEvent *ev)
{
   ev->ignore();
}

/*!
    Override this to handle show events (\a ev).

    The function is called when the window has requested becoming visible.

    If the window is successfully shown by the windowing system, this will
    be followed by a resize and an expose event.
*/
void QWindow::showEvent(QShowEvent *ev)
{
   ev->ignore();
}

/*!
    Override this to handle hide events (\a ev).

    The function is called when the window has requested being hidden in the
    windowing system.
*/
void QWindow::hideEvent(QHideEvent *ev)
{
   ev->ignore();
}

/*!
    Override this to handle any event (\a ev) sent to the window.
    Return \c true if the event was recognized and processed.

    Remember to call the base class version if you wish for mouse events,
    key events, resize events, etc to be dispatched as usual.
*/
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

/*!
    Schedules a QEvent::UpdateRequest event to be delivered to this window.

    The event is delivered in sync with the display vsync on platforms
    where this is possible. When driving animations, this function should
    be called once after drawing has completed.

    Calling this function multiple times will result in a single event
    being delivered to the window.

    Subclasses of QWindow should reimplement event(), intercept the event and
    call the application's rendering code, then call the base class
    implementation.

    \note The subclass' reimplementation of event() must invoke the base class
    implementation, unless it is absolutely sure that the event does not need to
    be handled by the base class. For example, the default implementation of
    this function relies on QEvent::Timer events. Filtering them away would
    therefore break the delivery of the update events.

    \since 5.5
*/
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

/*!
    Override this to handle key press events (\a ev).

    \sa keyReleaseEvent()
*/
void QWindow::keyPressEvent(QKeyEvent *ev)
{
   ev->ignore();
}

/*!
    Override this to handle key release events (\a ev).

    \sa keyPressEvent()
*/
void QWindow::keyReleaseEvent(QKeyEvent *ev)
{
   ev->ignore();
}

/*!
    Override this to handle focus in events (\a ev).

    Focus in events are sent when the window receives keyboard focus.

    \sa focusOutEvent()
*/
void QWindow::focusInEvent(QFocusEvent *ev)
{
   ev->ignore();
}

/*!
    Override this to handle focus out events (\a ev).

    Focus out events are sent when the window loses keyboard focus.

    \sa focusInEvent()
*/
void QWindow::focusOutEvent(QFocusEvent *ev)
{
   ev->ignore();
}

/*!
    Override this to handle mouse press events (\a ev).

    \sa mouseReleaseEvent()
*/
void QWindow::mousePressEvent(QMouseEvent *ev)
{
   ev->ignore();
}

/*!
    Override this to handle mouse release events (\a ev).

    \sa mousePressEvent()
*/
void QWindow::mouseReleaseEvent(QMouseEvent *ev)
{
   ev->ignore();
}

/*!
    Override this to handle mouse double click events (\a ev).

    \sa mousePressEvent(), QStyleHints::mouseDoubleClickInterval()
*/
void QWindow::mouseDoubleClickEvent(QMouseEvent *ev)
{
   ev->ignore();
}

/*!
    Override this to handle mouse move events (\a ev).
*/
void QWindow::mouseMoveEvent(QMouseEvent *ev)
{
   ev->ignore();
}

#ifndef QT_NO_WHEELEVENT
/*!
    Override this to handle mouse wheel or other wheel events (\a ev).
*/
void QWindow::wheelEvent(QWheelEvent *ev)
{
   ev->ignore();
}
#endif //QT_NO_WHEELEVENT

/*!
    Override this to handle touch events (\a ev).
*/
void QWindow::touchEvent(QTouchEvent *ev)
{
   ev->ignore();
}

#ifndef QT_NO_TABLETEVENT
/*!
    Override this to handle tablet press, move, and release events (\a ev).

    Proximity enter and leave events are not sent to windows, they are
    delivered to the application instance.
*/
void QWindow::tabletEvent(QTabletEvent *ev)
{
   ev->ignore();
}
#endif

/*!
    Override this to handle platform dependent events.
    Will be given \a eventType, \a message and \a result.

    This might make your application non-portable.

    Should return true only if the event was handled.
*/
bool QWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
   Q_UNUSED(eventType);
   Q_UNUSED(message);
   Q_UNUSED(result);
   return false;
}

/*!
    \fn QPoint QWindow::mapToGlobal(const QPoint &pos) const

    Translates the window coordinate \a pos to global screen
    coordinates. For example, \c{mapToGlobal(QPoint(0,0))} would give
    the global coordinates of the top-left pixel of the window.

    \sa mapFromGlobal()
*/
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


/*!
    \fn QPoint QWindow::mapFromGlobal(const QPoint &pos) const

    Translates the global screen coordinate \a pos to window
    coordinates.

    \sa mapToGlobal()
*/
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

/*!
    Creates a local representation of a window created by another process or by
    using native libraries below Qt.

    Given the handle \a id to a native window, this method creates a QWindow
    object which can be used to represent the window when invoking methods like
    setParent() and setTransientParent().

    This can be used, on platforms which support it, to embed a QWindow inside a
    native window, or to embed a native window inside a QWindow.

    If foreign windows are not supported, this function returns 0.

    \note The resulting QWindow should not be used to manipulate the underlying
    native window (besides re-parenting), or to observe state changes of the
    native window. Any support for these kind of operations is incidental, highly
    platform dependent and untested.

    \sa setParent()
    \sa setTransientParent()
*/
QWindow *QWindow::fromWinId(WId id)
{
   if (!QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::ForeignWindows)) {
      qWarning() << "QWindow::fromWinId(): platform plugin does not support foreign windows.";
      return 0;
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
   d->setCursor(0);
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

QDebug operator<<(QDebug debug, const QWindow *window)
{
   QDebugStateSaver saver(debug);
   debug.nospace();

   if (window) {
      debug << window->metaObject()->className() << '(' << (const void *)window;

      if (! window->objectName().isEmpty()) {
         debug << ", Name = " << window->objectName();
      }

      if (debug.verbosity() > 2) {
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

