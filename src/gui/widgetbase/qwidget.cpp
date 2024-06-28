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

#include <qwidget.h>

#include <qabstractscrollarea.h>
#include <qapplication.h>
#include <qbackingstore.h>
#include <qbrush.h>
#include <qcursor.h>
#include <qdebug.h>
#include <qevent.h>
#include <qfileinfo.h>
#include <qgraphicsproxywidget.h>
#include <qgraphicsscene.h>
#include <qinputmethod.h>
#include <qlayout.h>
#include <qmenu.h>
#include <qmetaobject.h>
#include <qoffscreensurface.h>
#include <qopenglcontext.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qplatform_backingstore.h>
#include <qplatform_window.h>
#include <qpointer.h>
#include <qstack.h>
#include <qstyle.h>
#include <qstylefactory.h>
#include <qstylehints.h>
#include <qstyleoption.h>
#include <qtooltip.h>
#include <qvariant.h>
#include <qwhatsthis.h>
#include <platformheaders/qxcbwindowfunctions.h>

#ifndef QT_NO_ACCESSIBILITY
# include <qaccessible.h>
#endif

#include <qabstractscrollarea_p.h>
#include <qaction_p.h>
#include <qapplication_p.h>
#include <qdesktopwidget_p.h>
#include <qevent_p.h>
#include <qgesturemanager_p.h>
#include <qgraphics_proxywidget_p.h>
#include <qgraphicseffect_p.h>
#include <qhighdpiscaling_p.h>
#include <qlayout_p.h>
#include <qopenglcontext_p.h>
#include <qpaintengine_raster_p.h>
#include <qstyle_p.h>
#include <qstylesheetstyle_p.h>
#include <qwidget_p.h>
#include <qwidgetbackingstore_p.h>
#include <qwidgetwindow_p.h>
#include <qwindow_p.h>
#include <qwindowcontainer_p.h>

static bool qt_enable_backingstore = true;

static inline bool qRectIntersects(const QRect &r1, const QRect &r2)
{
   return (qMax(r1.left(), r2.left()) <= qMin(r1.right(), r2.right()) &&
         qMax(r1.top(), r2.top()) <= qMin(r1.bottom(), r2.bottom()));
}

static inline bool hasBackingStoreSupport()
{
   return true;
}

extern bool qt_sendSpontaneousEvent(QObject *, QEvent *); // qapplication.cpp
extern QDesktopWidget *qt_desktopWidget; // qapplication.cpp

QWidgetBackingStoreTracker::QWidgetBackingStoreTracker()
   :   m_ptr(nullptr)
{
}

QWidgetBackingStoreTracker::~QWidgetBackingStoreTracker()
{
   delete m_ptr;
}

void QWidgetBackingStoreTracker::create(QWidget *widget)
{
   destroy();
   m_ptr = new QWidgetBackingStore(widget);
}

void QWidgetBackingStoreTracker::destroy()
{
   delete m_ptr;
   m_ptr = nullptr;
   m_widgets.clear();
}

void QWidgetBackingStoreTracker::registerWidget(QWidget *w)
{
   Q_ASSERT(m_ptr);
   Q_ASSERT(w->internalWinId());
   Q_ASSERT(qt_widget_private(w)->maybeBackingStore() == m_ptr);
   m_widgets.insert(w);
}

void QWidgetBackingStoreTracker::unregisterWidget(QWidget *w)
{
   if (m_widgets.remove(w) && m_widgets.isEmpty()) {
      delete m_ptr;
      m_ptr = nullptr;
   }
}

void QWidgetBackingStoreTracker::unregisterWidgetSubtree(QWidget *widget)
{
   unregisterWidget(widget);

   for (QObject *child : widget->children())  {
      if (QWidget *childWidget = dynamic_cast<QWidget *>(child)) {
         unregisterWidgetSubtree(childWidget);
      }
   }
}

QWidgetPrivate::QWidgetPrivate()
   : extra(nullptr), focus_next(nullptr), focus_prev(nullptr), focus_child(nullptr),
     layout(nullptr), needsFlush(nullptr), redirectDev(nullptr), widgetItem(nullptr),
     extraPaintEngine(nullptr), polished(nullptr), graphicsEffect(nullptr),

#if ! defined(QT_NO_IM)
     imHints(Qt::ImhNone),
#endif

#ifndef QT_NO_TOOLTIP
     toolTipDuration(-1),
#endif

     inheritedFontResolveMask(0), inheritedPaletteResolveMask(0), leftmargin(0), topmargin(0),
     rightmargin(0), bottommargin(0), leftLayoutItemMargin(0), topLayoutItemMargin(0),
     rightLayoutItemMargin(0), bottomLayoutItemMargin(0), hd(nullptr),
     size_policy(QSizePolicy::Preferred, QSizePolicy::Preferred),
     fg_role(QPalette::NoRole), bg_role(QPalette::NoRole), dirtyOpaqueChildren(1),
     isOpaque(0), retainSizeWhenHiddenChanged(0), inDirtyList(0), isScrolled(0), isMoved(0),
     usesDoubleBufferedGLContext(0), mustHaveWindowHandle(0), renderToTexture(0), textureChildSeen(0),

#ifndef QT_NO_IM
     inheritsInputMethodHints(0),
#endif

#ifndef QT_NO_OPENGL
     renderToTextureReallyDirty(1), renderToTextureComposeActive(0),
#endif

     childrenHiddenByWState(0), childrenShownByExpose(0)

#if defined(Q_OS_WIN)
   , noPaintOnScreen(0)
#endif
{
   if (! qApp) {
      qFatal("QWidget() Must construct a QApplication before a QWidget");
      return;
   }

   memset(high_attributes, 0, sizeof(high_attributes));
}

QWidgetPrivate::~QWidgetPrivate()
{
   if (widgetItem != nullptr) {
      widgetItem->wid = nullptr;
   }

   if (extra) {
      deleteExtra();
   }

#ifndef QT_NO_GRAPHICSEFFECT
   delete graphicsEffect;
#endif
}

void QWidgetPrivate::scrollChildren(int dx, int dy)
{
   Q_Q(QWidget);

   if (q->children().size() > 0) {        // scroll children
      QPoint pd(dx, dy);
      QObjectList childObjects = q->children();

      for (auto item : childObjects) {
         // move all children
         QWidget *w = dynamic_cast<QWidget *>(item);

         if (w != nullptr && ! w->isWindow()) {
            QPoint oldp = w->pos();
            QRect  r(w->pos() + pd, w->size());
            w->m_widgetData->crect = r;

            if (w->testAttribute(Qt::WA_WState_Created)) {
               w->d_func()->setWSGeometry();
            }

            w->d_func()->setDirtyOpaqueRegion();
            QMoveEvent e(r.topLeft(), oldp);
            QApplication::sendEvent(w, &e);
         }
      }
   }
}

void QWidgetPrivate::setWSGeometry()
{
   Q_Q(QWidget);

   if (QWindow *window = q->windowHandle()) {
      window->setGeometry(m_privateData.crect);
   }
}

void QWidgetPrivate::updateWidgetTransform(QEvent *event)
{
   Q_Q(QWidget);

   if (q == QGuiApplication::focusObject() || event->type() == QEvent::FocusIn) {
      QTransform t;
      QPoint p = q->mapTo(q->topLevelWidget(), QPoint(0, 0));
      t.translate(p.x(), p.y());
      QGuiApplication::inputMethod()->setInputItemTransform(t);
      QGuiApplication::inputMethod()->setInputItemRectangle(q->rect());
   }
}

#ifdef QT_KEYPAD_NAVIGATION
QPointer<QWidget> QWidgetPrivate::editingWidget;

bool QWidget::hasEditFocus() const
{
   const QWidget *w = this;

   while (w->d_func()->extra && w->d_func()->extra->focus_proxy) {
      w = w->d_func()->extra->focus_proxy;
   }

   return QWidgetPrivate::editingWidget == w;
}

void QWidget::setEditFocus(bool on)
{
   QWidget *f = this;

   while (f->d_func()->extra && f->d_func()->extra->focus_proxy) {
      f = f->d_func()->extra->focus_proxy;
   }

   if (QWidgetPrivate::editingWidget && QWidgetPrivate::editingWidget != f) {
      QWidgetPrivate::editingWidget->setEditFocus(false);
   }

   if (on && ! f->hasFocus()) {
      f->setFocus();
   }

   if ((! on && !QWidgetPrivate::editingWidget) || (on && QWidgetPrivate::editingWidget == f)) {
      return;
   }

   if (! on && QWidgetPrivate::editingWidget == f) {
      QWidgetPrivate::editingWidget = 0;
      QEvent event(QEvent::LeaveEditFocus);
      QApplication::sendEvent(f, &event);
      QApplication::sendEvent(f->style(), &event);

   } else if (on) {
      QWidgetPrivate::editingWidget = f;
      QEvent event(QEvent::EnterEditFocus);
      QApplication::sendEvent(f, &event);
      QApplication::sendEvent(f->style(), &event);
   }
}
#endif

bool QWidget::autoFillBackground() const
{
   Q_D(const QWidget);
   return d->extra && d->extra->autoFillBackground;
}

void QWidget::setAutoFillBackground(bool enabled)
{
   Q_D(QWidget);

   if (! d->extra) {
      d->createExtra();
   }

   if (d->extra->autoFillBackground == enabled) {
      return;
   }

   d->extra->autoFillBackground = enabled;
   d->updateIsOpaque();
   update();
   d->updateIsOpaque();
}

QWidgetMapper *QWidgetPrivate::mapper  = nullptr;         // widget with wid
QWidgetSet *QWidgetPrivate::allWidgets = nullptr;         // widgets with no wid

QRegion qt_dirtyRegion(QWidget *widget)
{
   if (widget == nullptr) {
      return QRegion();
   }

   QWidgetBackingStore *bs = qt_widget_private(widget)->maybeBackingStore();

   if (bs == nullptr) {
      return QRegion();
   }

   return bs->dirtyRegion(widget);
}

struct QWidgetExceptionCleaner {
   // cleans up when the constructor throws an exception

   static inline void cleanup(QWidget *that, QWidgetPrivate *d) {
      QWidgetPrivate::allWidgets->remove(that);

      if (d->focus_next != that) {
         if (d->focus_next) {
            d->focus_next->d_func()->focus_prev = d->focus_prev;
         }

         if (d->focus_prev) {
            d->focus_prev->d_func()->focus_next = d->focus_next;
         }
      }
   }
};

QWidget::QWidget(QWidget *parent, Qt::WindowFlags flags)
   : QObject(nullptr), QPaintDevice(), d_ptr(new QWidgetPrivate)
{
   d_ptr->q_ptr = this;
   Q_D(QWidget);

   try {
      d->init(parent, flags);

   } catch (...) {
      QWidgetExceptionCleaner::cleanup(this, d_func());
      throw;
   }
}

QWidget::QWidget(QWidgetPrivate &dd, QWidget *parent, Qt::WindowFlags flags)
   : QObject(nullptr), QPaintDevice(), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
   Q_D(QWidget);

   try {
      d->init(parent, flags);

   } catch (...) {
      QWidgetExceptionCleaner::cleanup(this, d_func());
      throw;
   }
}

void QWidget::_q_showIfNotHidden()
{
   Q_D(QWidget);
   d->_q_showIfNotHidden();
}

int QWidget::devType() const
{
   return QInternal::Widget;
}

// w is a "this" ptr, passed as a param because QWorkspace needs special logic
void QWidgetPrivate::adjustFlags(Qt::WindowFlags &flags, QWidget *w)
{
   bool customize =  (flags & (Qt::CustomizeWindowHint
         | Qt::FramelessWindowHint
         | Qt::WindowTitleHint
         | Qt::WindowSystemMenuHint
         | Qt::WindowMinimizeButtonHint
         | Qt::WindowMaximizeButtonHint
         | Qt::WindowCloseButtonHint
         | Qt::WindowContextHelpButtonHint));

   uint type = (flags & Qt::WindowType_Mask);

   if ((type == Qt::Widget || type == Qt::SubWindow) && w && ! w->parent()) {
      type = Qt::Window;
      flags |= Qt::Window;
   }

   if (flags & Qt::CustomizeWindowHint) {
      // modify window flags to make them consistent.
      // Only enable this on non-Mac platforms. Since the old way of doing this would
      // interpret WindowSystemMenuHint as a close button and we can't change that behavior
      // we can't just add this in.

#ifdef Q_OS_WIN
      if ((flags & (Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint))
            && type != Qt::Dialog) {

#else
      if (flags & (Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint)) {

#endif

         flags |= Qt::WindowSystemMenuHint;
         flags |= Qt::WindowTitleHint;
         flags &= ~Qt::FramelessWindowHint;
      }

   } else if (customize && !(flags & Qt::FramelessWindowHint)) {
      // if any of the window hints that affect the titlebar are set
      // and the window is supposed to have frame, we add a titlebar
      // and system menu by default.

      flags |= Qt::WindowSystemMenuHint;
      flags |= Qt::WindowTitleHint;
   }

   if (customize) {
      // do not modify window flags if the user explicitly set them

   } else if (type == Qt::Dialog || type == Qt::Sheet) {
      flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowContextHelpButtonHint | Qt::WindowCloseButtonHint;

   } else if (type == Qt::Tool) {
      flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint;

   } else {
      flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint |
            Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint | Qt::WindowFullscreenButtonHint;
   }

   if (w->testAttribute(Qt::WA_TransparentForMouseEvents)) {
      flags |= Qt::WindowTransparentForInput;
   }
}

void QWidgetPrivate::init(QWidget *parentWidget, Qt::WindowFlags flags)
{
   Q_Q(QWidget);

   if (! dynamic_cast<QApplication *>(QCoreApplication::instance())) {
      qFatal("QWidget: Must construct a QApplication before a QWidget");
   }

   Q_ASSERT(allWidgets);

   if (allWidgets) {
      allWidgets->insert(q);
   }

   int targetScreen = -1;

   if (parentWidget != nullptr && parentWidget->windowType() == Qt::Desktop) {
      const QDesktopScreenWidget *sw = dynamic_cast<const QDesktopScreenWidget *>(parentWidget);

      if (sw == nullptr) {
         targetScreen = 0;
      } else {
         targetScreen = sw->screenNumber();
      }

      parentWidget = nullptr;
   }

   q->m_widgetData = &m_privateData;

   if (! q->parent() ) {
      Q_ASSERT_X(q->thread() == qApp->thread(), "QWidget", "Widgets must be created in the GUI thread.");
   }

   if (targetScreen >= 0) {
      topData()->initialScreenIndex = targetScreen;

      if (QWindow *window = q->windowHandle()) {
         window->setScreen(QGuiApplication::screens().value(targetScreen, nullptr));
      }
   }

   m_privateData.fstrut_dirty = true;

   m_privateData.winid = 0;
   m_privateData.widget_attributes = 0;
   m_privateData.m_flags = flags;
   m_privateData.window_state = 0;
   m_privateData.focus_policy = 0;
   m_privateData.context_menu_policy = Qt::DefaultContextMenu;
   m_privateData.window_modality = Qt::NonModal;

   m_privateData.sizehint_forced = 0;
   m_privateData.is_closing = 0;
   m_privateData.in_show = 0;
   m_privateData.in_set_window_state = 0;
   m_privateData.in_destructor = false;

   // Widgets with Qt::MSWindowsOwnDC (typically QGLWidget) must have a window handle.
   if (flags & Qt::MSWindowsOwnDC)  {
      mustHaveWindowHandle = 1;
      q->setAttribute(Qt::WA_NativeWindow);
   }

   q->setAttribute(Qt::WA_QuitOnClose); // might be cleared in adjustQuitOnCloseAttribute()
   adjustQuitOnCloseAttribute();

   q->setAttribute(Qt::WA_WState_Hidden);

   // give potential windows a bigger "pre-initial" size, create_sys() will give them a new size later
   m_privateData.crect = parentWidget ? QRect(0, 0, 100, 30) : QRect(0, 0, 640, 480);

   focus_next = q;
   focus_prev = q;

   if ((flags & Qt::WindowType_Mask) == Qt::Desktop) {
      q->create();
   }

   else if (parentWidget) {
      q->setParent(parentWidget, m_privateData.m_flags);
   }

   else {
      adjustFlags(m_privateData.m_flags, q);
      resolveLayoutDirection();

      // opaque system background?
      const QBrush &background = q->palette().brush(QPalette::Window);
      setOpaque(q->isWindow() && background.style() != Qt::NoBrush && background.isOpaque());
   }

   m_privateData.fnt = QFont(m_privateData.fnt, q);

   q->setAttribute(Qt::WA_PendingMoveEvent);
   q->setAttribute(Qt::WA_PendingResizeEvent);

   if (++QWidgetPrivate::instanceCounter > QWidgetPrivate::maxInstances) {
      QWidgetPrivate::maxInstances = QWidgetPrivate::instanceCounter;
   }

   if (QApplicationPrivate::testAttribute(Qt::AA_ImmediateWidgetCreation)) {
      // TODO: Remove AA_ImmediateWidgetCreation
      q->create();
   }

   QEvent e(QEvent::Create);
   QApplication::sendEvent(q, &e);
   QApplication::postEvent(q, new QEvent(QEvent::PolishRequest));

   extraPaintEngine = nullptr;
}

void QWidgetPrivate::createRecursively()
{
   Q_Q(QWidget);

   q->create(0, true, true);

   for (auto item : q->children()) {
      QWidget *child = dynamic_cast<QWidget *>(item);

      if (child != nullptr && ! child->isHidden() && ! child->isWindow()
            && ! child->testAttribute(Qt::WA_WState_Created)) {
         child->d_func()->createRecursively();
      }
   }
}

void QWidget::create(WId window, bool initializeWindow, bool destroyOldWindow)
{
   Q_D(QWidget);

   if (window) {
      qWarning("QWidget::create() Parameter 'window' will be ignored");
   }

   if (testAttribute(Qt::WA_WState_Created) && window == 0 && internalWinId()) {
      return;
   }

   if (d->m_privateData.in_destructor) {
      return;
   }

   Qt::WindowType type = windowType();
   Qt::WindowFlags &flags = m_widgetData->m_flags;

   if ((type == Qt::Widget || type == Qt::SubWindow) && ! parentWidget()) {
      type = Qt::Window;
      flags |= Qt::Window;
   }

   if (QWidget *parent = parentWidget()) {

      if (type & Qt::Window) {
         if (! parent->testAttribute(Qt::WA_WState_Created)) {
            parent->createWinId();
         }

      } else if (testAttribute(Qt::WA_NativeWindow) && !parent->internalWinId()
            && ! testAttribute(Qt::WA_DontCreateNativeAncestors)) {

         // about to create a native child widget that does not have a native parent
         // enforce a native handle for the parent unless the Qt::WA_DontCreateNativeAncestors
         // attribute is set

         d->createWinId();

         // Nothing more to do.
         Q_ASSERT(testAttribute(Qt::WA_WState_Created));
         Q_ASSERT(internalWinId());
         return;
      }
   }

   static const bool paintOnScreenEnv = qgetenv("QT_ONSCREEN_PAINT").toInt() > 0;

   if (paintOnScreenEnv) {
      setAttribute(Qt::WA_PaintOnScreen);
   }

   if (QApplicationPrivate::testAttribute(Qt::AA_NativeWindows)) {
      setAttribute(Qt::WA_NativeWindow);
   }

#if defined(CS_SHOW_DEBUG_GUI_WIDGETS)
   qDebug() << "QWidget::create() " << this << " parent =" << parentWidget()
         << "Is Native =" << testAttribute(Qt::WA_NativeWindow);
#endif

   d->updateIsOpaque();

   setAttribute(Qt::WA_WState_Created);                        // set created flag
   d->create_sys(window, initializeWindow, destroyOldWindow);

   // a real toplevel window needs a backing store
   if (isWindow() && windowType() != Qt::Desktop) {
      d->topData()->backingStoreTracker.destroy();

      if (hasBackingStoreSupport()) {
         d->topData()->backingStoreTracker.create(this);
      }
   }

   d->setModal_sys();

   if (! isWindow() && parentWidget() && parentWidget()->testAttribute(Qt::WA_DropSiteRegistered)) {
      setAttribute(Qt::WA_DropSiteRegistered, true);
   }

#ifdef QT_EVAL
   extern void qt_eval_init_widget(QWidget * w);
   qt_eval_init_widget(this);
#endif

   // need to force the resting of the icon after changing parents
   if (testAttribute(Qt::WA_SetWindowIcon)) {
      d->setWindowIcon_sys();
   }

   if (isWindow() && !d->topData()->iconText.isEmpty()) {
      d->setWindowIconText_helper(d->topData()->iconText);
   }

   if (isWindow() && !d->topData()->caption.isEmpty()) {
      d->setWindowTitle_helper(d->topData()->caption);
   }

   if (windowType() != Qt::Desktop) {
      d->updateSystemBackground();

      if (isWindow() && !testAttribute(Qt::WA_SetWindowIcon))  {
         d->setWindowIcon_sys();
      }
   }

   // Frame strut update needed in cases where there are native widgets such as QGLWidget,
   // as those force native window creation on their ancestors before they are shown.
   // If the strut is not updated, any subsequent move of the top level window before show
   // will cause window frame to be ignored when positioning the window.
   // Note that this only helps on platforms that handle window creation synchronously.

   d->updateFrameStrut();
}

void q_createNativeChildrenAndSetParent(const QWidget *parentWidget)
{
   QObjectList children = parentWidget->children();

   for (auto item : children) {

      if (item->isWidgetType()) {
         const QWidget *childWidget = dynamic_cast<const QWidget *>(item);

         if (childWidget != nullptr) {
            if (childWidget->testAttribute(Qt::WA_NativeWindow)) {
               if (! childWidget->internalWinId()) {
                  childWidget->winId();
               }

               if (childWidget->windowHandle()) {
                  if (childWidget->isWindow()) {
                     childWidget->windowHandle()->setTransientParent(parentWidget->window()->windowHandle());
                  } else {
                     childWidget->windowHandle()->setParent(childWidget->nativeParentWidget()->windowHandle());
                  }
               }

            } else {
               q_createNativeChildrenAndSetParent(childWidget);
            }
         }
      }
   }
}

void QWidgetPrivate::create_sys(WId window, bool initializeWindow, bool destroyOldWindow)
{
   (void) window;
   (void) initializeWindow;
   (void) destroyOldWindow;

   Q_Q(QWidget);

   Qt::WindowFlags flags = m_privateData.m_flags;

   if (!q->testAttribute(Qt::WA_NativeWindow) && !q->isWindow()) {
      return;   // we only care about real toplevels
   }

   QWindow *win = topData()->window;

   // topData() ensures the extra is created but does not ensure 'window' is non-null
   // in case the extra was already valid.
   if (!win) {
      createTLSysExtra();
      win = topData()->window;
   }

   for (const QString &propertyName : q->dynamicPropertyNames()) {
      if (propertyName.startsWith("_q_platform_")) {
         win->setProperty(propertyName, q->property(propertyName));
      }
   }

   if (q->testAttribute(Qt::WA_ShowWithoutActivating)) {
      win->setProperty("_q_showWithoutActivating", QVariant(true));
   }

   if (q->testAttribute(Qt::WA_MacAlwaysShowToolWindow)) {
      win->setProperty("_q_macAlwaysShowToolWindow", QVariant::fromValue(QVariant(true)));
   }

   setNetWmWindowTypes(true);          // do nothing if none of WA_X11NetWmWindowType* is set
   win->setFlags(m_privateData.m_flags);
   fixPosIncludesFrame();

   if (q->testAttribute(Qt::WA_Moved)
         || !  QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::WindowManagement)) {
      win->setGeometry(q->geometry());

   } else {
      win->resize(q->size());
   }

   if (win->isTopLevel()) {
      int screenNumber = topData()->initialScreenIndex;
      topData()->initialScreenIndex = -1;

      if (screenNumber < 0) {
         screenNumber = q->windowType() != Qt::Desktop
               ? QApplication::desktop()->screenNumber(q) : 0;
      }

      win->setScreen(QGuiApplication::screens().value(screenNumber, nullptr));
   }

   QSurfaceFormat format = win->requestedFormat();

   if ((flags & Qt::Window) && win->surfaceType() != QSurface::OpenGLSurface
         && q->testAttribute(Qt::WA_TranslucentBackground)) {
      format.setAlphaBufferSize(8);
   }

   win->setFormat(format);

   if (QWidget *nativeParent = q->nativeParentWidget()) {
      if (nativeParent->windowHandle()) {
         if (flags & Qt::Window) {
            win->setTransientParent(nativeParent->window()->windowHandle());
            win->setParent(nullptr);
         } else {
            win->setTransientParent(nullptr);
            win->setParent(nativeParent->windowHandle());
         }
      }
   }

   qt_window_private(win)->positionPolicy = topData()->posIncludesFrame ?
         QWindowPrivate::WindowFrameInclusive : QWindowPrivate::WindowFrameExclusive;

   win->create();

   // Enable nonclient-area events for QDockWidget and other NonClientArea-mouse event processing.
   if ((flags & Qt::Desktop) == Qt::Window) {
      if (QPlatformWindow *platformWindow = win->handle()) {
         platformWindow->setFrameStrutEventsEnabled(true);
      }
   }

   m_privateData.m_flags = win->flags();

   if (! topData()->role.isEmpty()) {
      QXcbWindowFunctions::setWmWindowRole(win, topData()->role.toUtf8());
   }

   QBackingStore *store = q->backingStore();

   if (! store) {
      if (win && q->windowType() != Qt::Desktop) {
         if (q->isTopLevel()) {
            q->setBackingStore(new QBackingStore(win));
         }
      } else {
         q->setAttribute(Qt::WA_PaintOnScreen, true);
      }
   }

   setWindowModified_helper();
   WId id = win->winId();

   // See the QPlatformWindow::winId() documentation
   Q_ASSERT(id != WId(0));
   setWinId(id);

   // Check children and create windows for them if necessary
   q_createNativeChildrenAndSetParent(q);

   if (extra && !extra->mask.isEmpty()) {
      setMask_sys(extra->mask);
   }

   if (m_privateData.crect.width() == 0 || m_privateData.crect.height() == 0) {
      q->setAttribute(Qt::WA_OutsideWSRange, true);
   } else if (q->isVisible()) {
      // If widget is already shown, set window visible, too
      win->setVisible(true);
   }
}

#ifdef Q_OS_WIN
static const char activeXNativeParentHandleProperty[] = "_q_embedded_native_parent_handle";
#endif

void QWidgetPrivate::createTLSysExtra()
{
   Q_Q(QWidget);

   if (!extra->topextra->window && (q->testAttribute(Qt::WA_NativeWindow) || q->isWindow())) {
      extra->topextra->window = new QWidgetWindow(q);

      if (extra->minw || extra->minh) {
         extra->topextra->window->setMinimumSize(QSize(extra->minw, extra->minh));
      }

      if (extra->maxw != QWIDGETSIZE_MAX || extra->maxh != QWIDGETSIZE_MAX) {
         extra->topextra->window->setMaximumSize(QSize(extra->maxw, extra->maxh));
      }

      if (extra->topextra->opacity != 255 && q->isWindow()) {
         extra->topextra->window->setOpacity(qreal(extra->topextra->opacity) / qreal(255));
      }

#ifdef Q_OS_WIN
      // Pass on native parent handle for Widget embedded into Active X.
      const QVariant activeXNativeParentHandle = q->property(activeXNativeParentHandleProperty);

      if (activeXNativeParentHandle.isValid()) {
         extra->topextra->window->setProperty(activeXNativeParentHandleProperty, activeXNativeParentHandle);
      }

      if (q->inherits("QTipLabel") || q->inherits("QAlphaWidget")) {
         extra->topextra->window->setProperty("_q_windowsDropShadow", QVariant(true));
      }

#endif
   }
}

QWidget::~QWidget()
{
   Q_D(QWidget);

   d->m_privateData.in_destructor = true;

#ifndef QT_NO_GESTURES

   for (Qt::GestureType type : d->gestureContext.keys()) {
      ungrabGesture(type);
   }

#endif

   // force acceptDrops false before winId is destroyed
   d->registerDropSite(false);

#ifndef QT_NO_ACTION
   // remove all actions from this widget
   for (auto item : d->actions) {
      QActionPrivate *apriv = item->d_func();
      apriv->widgets.removeAll(this);
   }

   d->actions.clear();
#endif

#ifndef QT_NO_SHORTCUT
   // Remove all shortcuts grabbed by this widget, unless application is closing
   if (! QApplicationPrivate::is_app_closing && testAttribute(Qt::WA_GrabbedShortcut)) {
      qApp->d_func()->shortcutMap.removeShortcut(0, this, QKeySequence());
   }

#endif

   // ensure parent is notified while 'this' is still a widget
   QObject::cs_forceRemoveChild();

   // delete layout while we still are a valid widget
   delete d->layout;
   d->layout = nullptr;

   // remove myself from focus list
   Q_ASSERT(d->focus_next->d_func()->focus_prev == this);
   Q_ASSERT(d->focus_prev->d_func()->focus_next == this);

   if (d->focus_next != this) {
      d->focus_next->d_func()->focus_prev = d->focus_prev;
      d->focus_prev->d_func()->focus_next = d->focus_next;
      d->focus_next = d->focus_prev = nullptr;
   }

   try {
#ifndef QT_NO_GRAPHICSVIEW
      const QWidget *w = this;

      while (w->d_func()->extra && w->d_func()->extra->focus_proxy) {
         w = w->d_func()->extra->focus_proxy;
      }

      QWidget *window = w->window();
      QWExtra *e = window ? window->d_func()->extra : nullptr;

      if (! e || !e->proxyWidget || (w->parentWidget() && w->parentWidget()->d_func()->focus_child == this))
#endif
         clearFocus();

   } catch (...) {
      // ignore this issue siince we are in a destructor
   }

   d->setDirtyOpaqueRegion();

   if (isWindow() && isVisible() && internalWinId()) {
      try {
         d->close_helper(QWidgetPrivate::CloseNoEvent);

      } catch (...) {
         // if we are out of memory, at least hide the window
         try {
            hide();
         } catch (...) {
            // and if that does not work, give up
         }
      }

   }  else if (isVisible()) {
      qApp->d_func()->sendSyntheticEnterLeave(this);
   }

   if (QWidgetBackingStore *bs = d->maybeBackingStore()) {
      bs->removeDirtyWidget(this);

      if (testAttribute(Qt::WA_StaticContents)) {
         bs->removeStaticWidget(this);
      }
   }

   delete d->needsFlush;
   d->needsFlush = nullptr;

   // used by declarative library
   CSAbstractDeclarativeData *tmp_Data = CSInternalDeclarativeData::get_m_declarativeData(this);

   if (tmp_Data) {
      CSAbstractDeclarativeData::destroyed(tmp_Data, this);
      CSInternalDeclarativeData::set_m_declarativeData(this, nullptr);
   }

   CSInternalChildren::deleteChildren(this);
   QApplication::removePostedEvents(this);

   try {
      destroy();    // platform-dependent cleanup

   } catch (...) {
      // if this fails we can not do anything
   }

   --QWidgetPrivate::instanceCounter;

   // might have been deleted by ~QApplication
   if (QWidgetPrivate::allWidgets) {
      QWidgetPrivate::allWidgets->remove(this);
   }

   try {
      QEvent e(QEvent::Destroy);
      QCoreApplication::sendEvent(this, &e);

   } catch (const std::exception &) {
      // if this fails we ca not do anything
   }
}

bool QWidget::cs_isWidgetType() const
{
   return true;
}

int QWidgetPrivate::instanceCounter = 0;  // Current number of widget instances
int QWidgetPrivate::maxInstances = 0;     // Maximum number of widget instances

void QWidgetPrivate::setWinId(WId id)
{
   Q_Q(QWidget);

   // set widget identifier

   // the user might create a widget with Qt::Desktop window
   // attribute (or create another QDesktopWidget instance), which
   // will have the same windowid (the root window id) as the
   // qt_desktopWidget. We should not add the second desktop widget to the mapper.

   bool userDesktopWidget = qt_desktopWidget != nullptr && qt_desktopWidget != q && q->windowType() == Qt::Desktop;

   if (mapper && m_privateData.winid && !userDesktopWidget) {
      mapper->remove(m_privateData.winid);
   }

   const WId oldWinId = m_privateData.winid;

   m_privateData.winid = id;

   if (mapper && id && !userDesktopWidget) {
      mapper->insert(m_privateData.winid, q);
   }

   if (oldWinId != id) {
      QEvent e(QEvent::WinIdChange);
      QCoreApplication::sendEvent(q, &e);
   }
}

void QWidgetPrivate::createTLExtra()
{
   if (!extra) {
      createExtra();
   }

   if (!extra->topextra) {
      QTLWExtra *x = extra->topextra = new QTLWExtra;
      x->icon             = nullptr;
      x->backingStore     = nullptr;
      x->sharedPainter    = nullptr;
      x->incw             = 0;
      x->inch             = 0;
      x->basew            = 0;
      x->baseh            = 0;

      x->frameStrut.setCoords(0, 0, 0, 0);

      x->normalGeometry   = QRect(0, 0, -1, -1);
      x->savedFlags       = Qt::EmptyFlag;
      x->opacity          = 255;
      x->posIncludesFrame = 0;
      x->sizeAdjusted     = false;
      x->inTopLevelResize = false;
      x->inRepaint        = false;
      x->embedded         = 0;
      x->window           = nullptr;
      x->shareContext     = nullptr;

      x->initialScreenIndex = -1;
   }
}

void QWidgetPrivate::createExtra()
{
   if (! extra) {                                // if not exists
      extra = new QWExtra;
      extra->glContext = nullptr;
      extra->topextra  = nullptr;

#ifndef QT_NO_GRAPHICSVIEW
      extra->proxyWidget = nullptr;
#endif

#ifndef QT_NO_CURSOR
      extra->curs = nullptr;
#endif

      extra->minw = 0;
      extra->minh = 0;
      extra->maxw = QWIDGETSIZE_MAX;
      extra->maxh = QWIDGETSIZE_MAX;
      extra->customDpiX      = 0;
      extra->customDpiY      = 0;
      extra->explicitMinSize = 0;
      extra->explicitMaxSize = 0;

      extra->autoFillBackground   = 0;
      extra->nativeChildrenForced = 0;
      extra->inRenderWithPainter  = 0;
      extra->hasWindowContainer   = false;
      extra->hasMask              = 0;
      createSysExtra();
   }
}

void QWidgetPrivate::createSysExtra()
{
}

void QWidgetPrivate::deleteExtra()
{
   if (extra != nullptr) {

#ifndef QT_NO_CURSOR
      delete extra->curs;
#endif

      deleteSysExtra();

#ifndef QT_NO_STYLE_STYLESHEET
      // dereference the stylesheet style
      QStyleSheetStyle *proxy = dynamic_cast<QStyleSheetStyle *>(extra->style.data());

      if (proxy != nullptr) {
         proxy->deref();
      }

#endif

      if (extra->topextra != nullptr) {
         deleteTLSysExtra();

         delete extra->topextra->icon;
         delete extra->topextra;
      }

      delete extra;
      // extra->xic destroyed in QWidget::destroy()

      extra = nullptr;
   }
}

void QWidgetPrivate::deleteSysExtra()
{
}

static void deleteBackingStore(QWidgetPrivate *d)
{
   QTLWExtra *topData = d->topData();

   // The context must be current when destroying the backing store as it may attempt to
   // release resources like textures and shader programs. The window may not be suitable
   // anymore as there will often not be a platform window underneath at this stage. Fall
   // back to a QOffscreenSurface in this case.
   QScopedPointer<QOffscreenSurface> tempSurface;

#ifndef QT_NO_OPENGL

   if (d->textureChildSeen && topData->shareContext) {
      if (topData->window->handle()) {
         topData->shareContext->makeCurrent(topData->window);
      } else {
         tempSurface.reset(new QOffscreenSurface);
         tempSurface->setFormat(topData->shareContext->format());
         tempSurface->create();
         topData->shareContext->makeCurrent(tempSurface.data());
      }
   }

#endif

   delete topData->backingStore;
   topData->backingStore = nullptr;

#ifndef QT_NO_OPENGL

   if (d->textureChildSeen && topData->shareContext) {
      topData->shareContext->doneCurrent();
   }

#endif
}
void QWidgetPrivate::deleteTLSysExtra()
{
   if (extra && extra->topextra) {
      //the qplatformbackingstore may hold a reference to the window, so the backingstore
      //needs to be deleted first. If the backingstore holds GL resources, we need to
      // make the context current here. This is taken care of by deleteBackingStore().

      extra->topextra->backingStoreTracker.destroy();
      deleteBackingStore(this);

#ifndef QT_NO_OPENGL
      qDeleteAll(extra->topextra->widgetTextures);
      extra->topextra->widgetTextures.clear();
      delete extra->topextra->shareContext;
      extra->topextra->shareContext = nullptr;
#endif

      //the toplevel might have a context with a "qglcontext associated with it. We need to
      //delete the qglcontext before we delete the qplatformopenglcontext.
      //One unfortunate thing about this is that we potentially create a glContext just to
      //delete it straight afterwards.

      if (extra->topextra->window) {
         extra->topextra->window->destroy();
      }

      delete extra->topextra->window;
      extra->topextra->window = nullptr;

   }
}

bool QWidgetPrivate::isOverlapped(const QRect &rect) const
{
   Q_Q(const QWidget);

   const QWidget *w = q;
   QRect r = rect;

   while (w != nullptr) {
      if (w->isWindow()) {
         return false;
      }

      QWidget *parent    = w->parentWidget();
      QWidgetPrivate *pd = parent->d_func();

      bool above = false;

      for (auto item : parent->children()) {
         QWidget *sibling = dynamic_cast<QWidget *>(item);

         if (sibling == nullptr || ! sibling->isVisible() || sibling->isWindow()) {
            continue;
         }

         if (! above) {
            above = (sibling == w);
            continue;
         }

         if (qRectIntersects(sibling->d_func()->effectiveRectFor(sibling->m_widgetData->crect), r)) {
            const QWExtra *siblingExtra = sibling->d_func()->extra;

            if (siblingExtra && siblingExtra->hasMask && ! sibling->d_func()->graphicsEffect
                  && ! siblingExtra->mask.translated(sibling->m_widgetData->crect.topLeft()).intersects(r)) {
               continue;
            }

            return true;
         }
      }

      w = w->parentWidget();
      r.translate(pd->m_privateData.crect.topLeft());
   }

   return false;
}

void QWidgetPrivate::syncBackingStore()
{
   if (paintOnScreen()) {
      repaint_sys(dirty);
      dirty = QRegion();
   } else if (QWidgetBackingStore *bs = maybeBackingStore()) {
      bs->sync();
   }
}

void QWidgetPrivate::syncBackingStore(const QRegion &region)
{
   if (paintOnScreen()) {
      repaint_sys(region);

   } else if (QWidgetBackingStore *bs = maybeBackingStore()) {
      bs->sync(q_func(), region);
   }
}

void QWidgetPrivate::setUpdatesEnabled_helper(bool enable)
{
   Q_Q(QWidget);

   if (enable && ! q->isWindow() && q->parentWidget() && ! q->parentWidget()->updatesEnabled()) {
      return;   // nothing we can do
   }

   if (enable != q->testAttribute(Qt::WA_UpdatesDisabled)) {
      return;   // nothing to do
   }

   q->setAttribute(Qt::WA_UpdatesDisabled, !enable);

   if (enable) {
      q->update();
   }

   Qt::WidgetAttribute attribute = enable ? Qt::WA_ForceUpdatesDisabled : Qt::WA_UpdatesDisabled;

   for (auto item : q->children()) {
      QWidget *w = dynamic_cast<QWidget *>(item);

      if (w != nullptr && ! w->isWindow() && ! w->testAttribute(attribute)) {
         w->d_func()->setUpdatesEnabled_helper(enable);
      }
   }
}

void QWidgetPrivate::propagatePaletteChange()
{
   Q_Q(QWidget);

   // Propagate a new inherited mask to all children.

#ifndef QT_NO_GRAPHICSVIEW

   if (! q->parentWidget() && extra && extra->proxyWidget) {
      QGraphicsProxyWidget *p = extra->proxyWidget;
      inheritedPaletteResolveMask = p->d_func()->inheritedPaletteResolveMask | p->palette().resolve();

   } else
#endif

   {
      if (q->isWindow() && ! q->testAttribute(Qt::WA_WindowPropagation)) {
         inheritedPaletteResolveMask = 0;
      }
   }

   int mask = m_privateData.pal.resolve() | inheritedPaletteResolveMask;

   QEvent pc(QEvent::PaletteChange);
   QApplication::sendEvent(q, &pc);

   for (auto item : q->children()) {
      QWidget *w = dynamic_cast<QWidget *>(item);

      if (w != nullptr && ! w->testAttribute(Qt::WA_StyleSheet)
            && (! w->isWindow() || w->testAttribute(Qt::WA_WindowPropagation))) {
         QWidgetPrivate *wd = w->d_func();
         wd->inheritedPaletteResolveMask = mask;
         wd->resolvePalette();
      }
   }
}

QRect QWidgetPrivate::clipRect() const
{
   Q_Q(const QWidget);

   const QWidget *w = q;

   if (! w->isVisible()) {
      return QRect();
   }

   QRect r = effectiveRectFor(q->rect());
   int ox  = 0;
   int oy  = 0;

   while (w && w->isVisible() && ! w->isWindow() && w->parentWidget()) {
      ox -= w->x();
      oy -= w->y();
      w   = w->parentWidget();
      r  &= QRect(ox, oy, w->width(), w->height());
   }

   return r;
}

QRegion QWidgetPrivate::clipRegion() const
{
   Q_Q(const QWidget);

   if (! q->isVisible()) {
      return QRegion();
   }

   QRegion r(q->rect());

   const QWidget *w = q;
   const QWidget *ignoreUpTo;
   int ox = 0;
   int oy = 0;

   while (w && w->isVisible() && ! w->isWindow() && w->parentWidget()) {
      ox -= w->x();
      oy -= w->y();
      ignoreUpTo = w;
      w = w->parentWidget();
      r &= QRegion(ox, oy, w->width(), w->height());

      int i = 0;

      while (w->children().at(i++) != static_cast<const QObject *>(ignoreUpTo)) {
      }

      while (i < w->children().size()) {
         QWidget *sibling = dynamic_cast<QWidget *>(w->children().at(i));

         if (sibling != nullptr) {

            if (sibling->isVisible() && ! sibling->isWindow()) {
               QRect siblingRect(ox + sibling->x(), oy + sibling->y(), sibling->width(), sibling->height());

               if (qRectIntersects(siblingRect, q->rect())) {
                  r -= QRegion(siblingRect);
               }
            }
         }

         ++i;
      }
   }

   return r;
}

void QWidgetPrivate::setSystemClip(QPaintDevice *paintDevice, const QRegion &region)
{
   // Transform the system clip region from device-independent pixels to device pixels
   QPaintEngine *paintEngine = paintDevice->paintEngine();
   QTransform scaleTransform;
   const qreal devicePixelRatio = paintDevice->devicePixelRatioF();
   scaleTransform.scale(devicePixelRatio, devicePixelRatio);
   paintEngine->d_func()->systemClip = scaleTransform.map(region);
}

#ifndef QT_NO_GRAPHICSEFFECT
void QWidgetPrivate::invalidateGraphicsEffectsRecursively()
{
   Q_Q(QWidget);

   QWidget *w = q;

   do {
      if (w->graphicsEffect()) {

         QWidgetEffectSourcePrivate *sourced =
               static_cast<QWidgetEffectSourcePrivate *>(w->graphicsEffect()->source()->d_func());

         if (sourced != nullptr && ! sourced->updateDueToGraphicsEffect) {
            w->graphicsEffect()->source()->d_func()->invalidateCache();
         }
      }

      w = w->parentWidget();

   } while (w);
}
#endif

void QWidgetPrivate::setDirtyOpaqueRegion()
{
   Q_Q(QWidget);

   dirtyOpaqueChildren = true;

#ifndef QT_NO_GRAPHICSEFFECT
   invalidateGraphicsEffectsRecursively();
#endif

   if (q->isWindow()) {
      return;
   }

   QWidget *parent = q->parentWidget();

   if (! parent) {
      return;
   }

   // TODO: instead of setting dirtyflag, manipulate the dirtyregion directly?
   QWidgetPrivate *pd = parent->d_func();

   if (! pd->dirtyOpaqueChildren) {
      pd->setDirtyOpaqueRegion();
   }
}

const QRegion &QWidgetPrivate::getOpaqueChildren() const
{
   Q_Q(const QWidget);

   if (! dirtyOpaqueChildren) {
      return opaqueChildren;
   }

   QWidgetPrivate *that = const_cast<QWidgetPrivate *>(this);
   that->opaqueChildren = QRegion();

   for (auto item : q->children()) {
      QWidget *child = dynamic_cast<QWidget *>(item);

      if (child == nullptr || ! child->isVisible() || child->isWindow()) {
         continue;
      }

      const QPoint offset = child->geometry().topLeft();
      QWidgetPrivate *childd = child->d_func();
      QRegion r = childd->isOpaque ? child->rect() : childd->getOpaqueChildren();

      if (childd->extra && childd->extra->hasMask) {
         r &= childd->extra->mask;
      }

      if (r.isEmpty()) {
         continue;
      }

      r.translate(offset);
      that->opaqueChildren += r;
   }

   that->opaqueChildren &= q_func()->rect();
   that->dirtyOpaqueChildren = false;

   return that->opaqueChildren;
}

void QWidgetPrivate::subtractOpaqueChildren(QRegion &source, const QRect &clipRect) const
{
   Q_Q(const QWidget);

   if (q->children().isEmpty() || clipRect.isEmpty()) {
      return;
   }

   const QRegion &r = getOpaqueChildren();

   if (! r.isEmpty()) {
      source -= (r & clipRect);
   }
}

//subtract any relatives that are higher up than me --- this is too expensive !!!
void QWidgetPrivate::subtractOpaqueSiblings(QRegion &sourceRegion, bool *hasDirtySiblingsAbove,
      bool alsoNonOpaque) const
{
   Q_Q(const QWidget);
   static int disableSubtractOpaqueSiblings = qgetenv("QT_NO_SUBTRACTOPAQUESIBLINGS").toInt();

   if (disableSubtractOpaqueSiblings || q->isWindow()) {
      return;
   }

   QRect clipBoundingRect;
   bool dirtyClipBoundingRect = true;

   QRegion parentClip;
   bool dirtyParentClip = true;

   QPoint parentOffset = m_privateData.crect.topLeft();

   const QWidget *w = q;

   while (w) {
      if (w->isWindow()) {
         break;
      }

      QWidget *parent = w->parentWidget();
      QWidgetPrivate *pd = parent->d_func();

      const int myIndex = parent->children().indexOf(const_cast<QWidget *>(w));
      const QRect widgetGeometry = w->d_func()->effectiveRectFor(w->m_widgetData->crect);

      for (int i = myIndex + 1; i < parent->children().size(); ++i) {
         QWidget *sibling = dynamic_cast<QWidget *>(parent->children().at(i));

         if (sibling  == nullptr || ! sibling->isVisible() || sibling->isWindow()) {
            continue;
         }

         const QRect siblingGeometry = sibling->d_func()->effectiveRectFor(sibling->m_widgetData->crect);

         if (! qRectIntersects(siblingGeometry, widgetGeometry)) {
            continue;
         }

         if (dirtyClipBoundingRect) {
            clipBoundingRect = sourceRegion.boundingRect();
            dirtyClipBoundingRect = false;
         }

         if (! qRectIntersects(siblingGeometry, clipBoundingRect.translated(parentOffset))) {
            continue;
         }

         if (dirtyParentClip) {
            parentClip = sourceRegion.translated(parentOffset);
            dirtyParentClip = false;
         }

         const QPoint siblingPos(sibling->m_widgetData->crect.topLeft());
         const QRect siblingClipRect(sibling->d_func()->clipRect());
         QRegion siblingDirty(parentClip);

         siblingDirty &= (siblingClipRect.translated(siblingPos));
         const bool hasMask = sibling->d_func()->extra && sibling->d_func()->extra->hasMask
               && !sibling->d_func()->graphicsEffect;

         if (hasMask) {
            siblingDirty &= sibling->d_func()->extra->mask.translated(siblingPos);
         }

         if (siblingDirty.isEmpty()) {
            continue;
         }

         if (sibling->d_func()->isOpaque || alsoNonOpaque) {
            if (hasMask) {
               siblingDirty.translate(-parentOffset);
               sourceRegion -= siblingDirty;
            } else {
               sourceRegion -= siblingGeometry.translated(-parentOffset);
            }

         } else {
            if (hasDirtySiblingsAbove) {
               *hasDirtySiblingsAbove = true;
            }

            if (sibling->children().isEmpty()) {
               continue;
            }

            QRegion opaqueSiblingChildren(sibling->d_func()->getOpaqueChildren());
            opaqueSiblingChildren.translate(-parentOffset + siblingPos);
            sourceRegion -= opaqueSiblingChildren;
         }

         if (sourceRegion.isEmpty()) {
            return;
         }

         dirtyClipBoundingRect = true;
         dirtyParentClip = true;
      }

      w = w->parentWidget();
      parentOffset += pd->m_privateData.crect.topLeft();
      dirtyParentClip = true;
   }
}

void QWidgetPrivate::clipToEffectiveMask(QRegion &region) const
{
   Q_Q(const QWidget);

   const QWidget *w = q;
   QPoint offset;

#ifndef QT_NO_GRAPHICSEFFECT

   if (graphicsEffect) {
      w = q->parentWidget();
      offset -= m_privateData.crect.topLeft();
   }

#endif

   while (w) {
      const QWidgetPrivate *wd = w->d_func();

      if (wd->extra && wd->extra->hasMask) {
         region &= (w != q) ? wd->extra->mask.translated(offset) : wd->extra->mask;
      }

      if (w->isWindow()) {
         return;
      }

      offset -= wd->m_privateData.crect.topLeft();
      w = w->parentWidget();
   }
}

bool QWidgetPrivate::paintOnScreen() const
{
#if defined(QT_NO_BACKINGSTORE)
   return true;

#else
   Q_Q(const QWidget);

   if (q->testAttribute(Qt::WA_PaintOnScreen)
         || (!q->isWindow() && q->window()->testAttribute(Qt::WA_PaintOnScreen))) {
      return true;
   }

   return !qt_enable_backingstore;
#endif
}

void QWidgetPrivate::updateIsOpaque()
{
   // only needed if opacity actually changed
   setDirtyOpaqueRegion();

#ifndef QT_NO_GRAPHICSEFFECT

   if (graphicsEffect) {
      // ### We should probably add QGraphicsEffect::isOpaque at some point
      setOpaque(false);
      return;
   }

#endif

   Q_Q(QWidget);

   if (q->testAttribute(Qt::WA_OpaquePaintEvent) || q->testAttribute(Qt::WA_PaintOnScreen)) {
      setOpaque(true);
      return;
   }

   const QPalette &pal = q->palette();

   if (q->autoFillBackground()) {
      const QBrush &autoFillBrush = pal.brush(q->backgroundRole());

      if (autoFillBrush.style() != Qt::NoBrush && autoFillBrush.isOpaque()) {
         setOpaque(true);
         return;
      }
   }

   if (q->isWindow() && !q->testAttribute(Qt::WA_NoSystemBackground)) {
      const QBrush &windowBrush = q->palette().brush(QPalette::Window);

      if (windowBrush.style() != Qt::NoBrush && windowBrush.isOpaque()) {
         setOpaque(true);
         return;
      }

   }

   setOpaque(false);
}

void QWidgetPrivate::setOpaque(bool opaque)
{
   if (isOpaque != opaque) {
      isOpaque = opaque;
      updateIsTranslucent();
   }
}

void QWidgetPrivate::updateIsTranslucent()
{
   Q_Q(QWidget);

   if (QWindow *window = q->windowHandle()) {
      QSurfaceFormat format = window->format();
      const int oldAlpha = format.alphaBufferSize();
      const int newAlpha = q->testAttribute(Qt::WA_TranslucentBackground) ? 8 : 0;

      if (oldAlpha != newAlpha) {
         format.setAlphaBufferSize(newAlpha);

         window->setFormat(format);
      }
   }
}

static inline void fillRegion(QPainter *painter, const QRegion &rgn, const QBrush &brush)
{
   Q_ASSERT(painter);

   if (brush.style() == Qt::TexturePattern) {

      const QRect rect(rgn.boundingRect());
      painter->setClipRegion(rgn);
      painter->drawTiledPixmap(rect, brush.texture(), rect.topLeft());

   } else if (brush.gradient() && brush.gradient()->coordinateMode() == QGradient::ObjectBoundingMode) {
      painter->save();
      painter->setClipRegion(rgn);
      painter->fillRect(0, 0, painter->device()->width(), painter->device()->height(), brush);
      painter->restore();

   } else {
      const QVector<QRect> &rects = rgn.rects();

      for (auto item : rects) {
         painter->fillRect(item, brush);
      }
   }
}

void QWidgetPrivate::paintBackground(QPainter *painter, const QRegion &rgn, int flags) const
{
   Q_Q(const QWidget);

#ifndef QT_NO_SCROLLAREA
   bool resetBrushOrigin = false;
   QPointF oldBrushOrigin;

   // If we are painting the viewport of a scrollarea, we must apply an offset to the brush
   // in case we are drawing a texture

   QAbstractScrollArea *scrollArea = dynamic_cast<QAbstractScrollArea *>(q->parent());

   if (scrollArea != nullptr && scrollArea->viewport() == q) {
      QWidgetPrivate *scrollPrivate    = static_cast<QWidget *>(scrollArea)->d_ptr.data();
      QAbstractScrollAreaPrivate *priv = static_cast<QAbstractScrollAreaPrivate *>(scrollPrivate);

      oldBrushOrigin   = painter->brushOrigin();
      resetBrushOrigin = true;
      painter->setBrushOrigin(- (priv->contentsOffset()) );
   }

#endif

   const QBrush autoFillBrush = q->palette().brush(q->backgroundRole());

   if ((flags & DrawAsRoot) && ! (q->autoFillBackground() && autoFillBrush.isOpaque())) {
      const QBrush bg = q->palette().brush(QPalette::Window);

      if (! (flags & DontSetCompositionMode)) {

         //copy alpha straight in
         QPainter::CompositionMode oldMode = painter->compositionMode();
         painter->setCompositionMode(QPainter::CompositionMode_Source);
         fillRegion(painter, rgn, bg);
         painter->setCompositionMode(oldMode);

      } else {
         fillRegion(painter, rgn, bg);
      }
   }

   if (q->autoFillBackground()) {
      fillRegion(painter, rgn, autoFillBrush);
   }

   if (q->testAttribute(Qt::WA_StyledBackground)) {
      painter->setClipRegion(rgn);
      QStyleOption opt;
      opt.initFrom(q);
      q->style()->drawPrimitive(QStyle::PE_Widget, &opt, painter, q);
   }

#ifndef QT_NO_SCROLLAREA

   if (resetBrushOrigin) {
      painter->setBrushOrigin(oldBrushOrigin);
   }

#endif
}

extern QWidget *qt_button_down;

void QWidgetPrivate::deactivateWidgetCleanup()
{
   Q_Q(QWidget);

   // If this was the active application window, reset it
   if (QApplication::activeWindow() == q) {
      QApplication::setActiveWindow(nullptr);
   }

   // If the is the active mouse press widget, reset it
   if (q == qt_button_down) {
      qt_button_down = nullptr;
   }
}

QWidget *QWidget::find(WId id)
{
   return QWidgetPrivate::mapper ? QWidgetPrivate::mapper->value(id, nullptr) : nullptr;
}

WId QWidget::winId() const
{
   if (! testAttribute(Qt::WA_WState_Created) || ! internalWinId()) {

#if defined(CS_SHOW_DEBUG_GUI_WIDGETS)
      qDebug() << "QWidget::winId() Creating native window for " << this;
#endif

      QWidget *that = const_cast<QWidget *>(this);
      that->setAttribute(Qt::WA_NativeWindow);

      that->d_func()->createWinId();
      return that->m_widgetData->winid;
   }

   return m_widgetData->winid;
}

void QWidgetPrivate::createWinId()
{
   Q_Q(QWidget);

   const bool forceNativeWindow = q->testAttribute(Qt::WA_NativeWindow);

   if (!q->testAttribute(Qt::WA_WState_Created) || (forceNativeWindow && ! q->internalWinId())) {

      if (! q->isWindow()) {

         QWidget *parent = q->parentWidget();
         QWidgetPrivate *pd = parent->d_func();

         if (forceNativeWindow && ! q->testAttribute(Qt::WA_DontCreateNativeAncestors)) {
            parent->setAttribute(Qt::WA_NativeWindow);
         }

         if (! parent->internalWinId()) {
            pd->createWinId();
         }

         for (auto item : parent->children()) {
            QWidget *w = dynamic_cast<QWidget *>(item);

            if (w != nullptr && ! w->isWindow() && (!w->testAttribute(Qt::WA_WState_Created)
                  || (! w->internalWinId() && w->testAttribute(Qt::WA_NativeWindow)))) {

               w->create();
            }
         }

      } else {
         q->create();
      }

   }
}

void QWidget::createWinId()
{
   Q_D(QWidget);

#if defined(CS_SHOW_DEBUG_GUI_WIDGETS)
   qDebug() << "QWidget::createWinId() Creating Window id for " << this;
#endif

   d->createWinId();
}

WId QWidget::effectiveWinId() const
{
   const WId id = internalWinId();

   if (id || ! testAttribute(Qt::WA_WState_Created)) {
      return id;
   }

   if (const QWidget *realParent = nativeParentWidget()) {
      return realParent->internalWinId();
   }

   return 0;
}

QWindow *QWidget::windowHandle() const
{
   Q_D(const QWidget);
   QTLWExtra *extra = d->maybeTopData();

   if (extra) {
      return extra->window;
   }

   return nullptr;
}

#ifndef QT_NO_STYLE_STYLESHEET

QString QWidget::styleSheet() const
{
   Q_D(const QWidget);

   if (!d->extra) {
      return QString();
   }

   return d->extra->styleSheet;
}

void QWidget::setStyleSheet(const QString &styleSheet)
{
   Q_D(QWidget);

   if (m_widgetData->in_destructor) {
      return;
   }

   d->createExtra();

   QStyleSheetStyle *proxy = dynamic_cast<QStyleSheetStyle *>(d->extra->style.data());
   d->extra->styleSheet    = styleSheet;

   if (styleSheet.isEmpty()) {
      // stylesheet removed

      if (! proxy) {
         return;
      }

      d->inheritStyle();
      return;
   }

   if (proxy) {
      // style sheet update

      if (d->polished) {
         proxy->repolish(this);
      }

      return;
   }

   if (testAttribute(Qt::WA_SetStyle)) {
      d->setStyle_helper(new QStyleSheetStyle(d->extra->style), true);
   } else {
      d->setStyle_helper(new QStyleSheetStyle(nullptr), true);
   }
}

#endif

QStyle *QWidget::style() const
{
   Q_D(const QWidget);

   if (d->extra && d->extra->style) {
      return d->extra->style;
   }

   return QApplication::style();
}

void QWidget::setStyle(QStyle *style)
{
   Q_D(QWidget);

   setAttribute(Qt::WA_SetStyle, style != nullptr);
   d->createExtra();

#ifndef QT_NO_STYLE_STYLESHEET

   if (QStyleSheetStyle *proxy = dynamic_cast<QStyleSheetStyle *>(style)) {

      // if someone tries to set a QStyleSheetStyle, increment the ref count
      // (this may happen in QButtonDialogBox which propagates its style)

      proxy->ref();
      d->setStyle_helper(style, false);

   } else if (dynamic_cast<QStyleSheetStyle *>(d->extra->style.data()) || ! qApp->styleSheet().isEmpty()) {
      // if we have an application stylesheet or have a proxy already, propagate
      d->setStyle_helper(new QStyleSheetStyle(style), true);

   } else
#endif

   {
      d->setStyle_helper(style, false);
   }
}

void QWidgetPrivate::setStyle_helper(QStyle *newStyle, bool propagate, bool)
{
   Q_Q(QWidget);

   QStyle *oldStyle  = q->style();

#ifndef QT_NO_STYLE_STYLESHEET
   QPointer<QStyle> origStyle;
#endif

   createExtra();

#ifndef QT_NO_STYLE_STYLESHEET
   origStyle = extra->style.data();
#endif

   extra->style = newStyle;

   // repolish
   if (q->windowType() != Qt::Desktop) {

      if (polished) {
         oldStyle->unpolish(q);
         q->style()->polish(q);
      }
   }

   if (propagate) {
      // copy the list because the order may be modified
      const QObjectList childrenList = q->children();

      for (auto item : childrenList) {
         QWidget *tmp = dynamic_cast<QWidget *>(item);

         if (tmp != nullptr) {
            tmp->d_func()->inheritStyle();
         }
      }
   }

#ifndef QT_NO_STYLE_STYLESHEET

   if (! dynamic_cast<QStyleSheetStyle *>(newStyle)) {
      const QStyleSheetStyle *cssStyle = dynamic_cast<QStyleSheetStyle *>(origStyle.data());

      if (cssStyle != nullptr) {
         cssStyle->clearWidgetFont(q);
      }
   }

#endif

   QEvent e(QEvent::StyleChange);
   QApplication::sendEvent(q, &e);

#ifndef QT_NO_STYLE_STYLESHEET
   // dereference the old stylesheet style
   QStyleSheetStyle *proxy = dynamic_cast<QStyleSheetStyle *>(origStyle.data());

   if (proxy != nullptr) {
      proxy->deref();
   }

#endif
}

// Inherits style from the current parent and propagates it as necessary
void QWidgetPrivate::inheritStyle()
{
#ifndef QT_NO_STYLE_STYLESHEET
   Q_Q(QWidget);

   QStyleSheetStyle *proxy = extra ? dynamic_cast<QStyleSheetStyle *>(extra->style.data()) : nullptr;

   if (! q->styleSheet().isEmpty()) {
      Q_ASSERT(proxy);
      proxy->repolish(q);

      return;
   }

   QStyle *origStyle   = proxy ? proxy->base : (extra ? static_cast<QStyle *>(extra->style.data()) : nullptr);
   QWidget *parent     = q->parentWidget();
   QStyle *parentStyle = (parent && parent->d_func()->extra) ? (QStyle *)parent->d_func()->extra->style : nullptr;

   // If we have stylesheet on app or parent has stylesheet style, we need
   // to be running a proxy

   if (! qApp->styleSheet().isEmpty() || dynamic_cast<QStyleSheetStyle *>(parentStyle)) {
      QStyle *newStyle = parentStyle;

      if (q->testAttribute(Qt::WA_SetStyle)) {
         newStyle = new QStyleSheetStyle(origStyle);
      } else if (QStyleSheetStyle *newProxy = dynamic_cast<QStyleSheetStyle *>(parentStyle)) {
         newProxy->ref();
      }

      setStyle_helper(newStyle, true);
      return;
   }

   // So, we have no stylesheet on parent/app and we have an empty stylesheet
   // we just need our original style back
   if (origStyle == (extra ? (QStyle *)extra->style : nullptr)) {
      // is it any different?
      return;
   }

   // We could have inherited the proxy from our parent (which has a custom style)
   // In such a case we need to start following the application style (i.e revert
   // the propagation behavior of QStyleSheetStyle)
   if (! q->testAttribute(Qt::WA_SetStyle)) {
      origStyle = nullptr;
   }

   setStyle_helper(origStyle, true);
#endif
}

Qt::WindowModality QWidget::windowModality() const
{
   return static_cast<Qt::WindowModality>(m_widgetData->window_modality);
}

void QWidget::setWindowModality(Qt::WindowModality windowModality)
{
   m_widgetData->window_modality = windowModality;

   // setModal_sys() will be called by setAttribute()
   setAttribute(Qt::WA_ShowModal, (m_widgetData->window_modality != Qt::NonModal));
   setAttribute(Qt::WA_SetWindowModality, true);
}

void QWidgetPrivate::setModal_sys()
{
   Q_Q(QWidget);

   if (q->windowHandle()) {
      q->windowHandle()->setModality(q->windowModality());
   }
}

bool QWidget::isMinimized() const
{
   return m_widgetData->window_state & Qt::WindowMinimized;
}

void QWidget::showMinimized()
{
   bool isMin = isMinimized();

   if (isMin && isVisible()) {
      return;
   }

   ensurePolished();

   if (!isMin) {
      setWindowState((windowState() & ~Qt::WindowActive) | Qt::WindowMinimized);
   }

   setVisible(true);
}

bool QWidget::isMaximized() const
{
   return m_widgetData->window_state & Qt::WindowMaximized;
}

Qt::WindowStates QWidget::windowState() const
{
   return Qt::WindowStates(m_widgetData->window_state);
}

void QWidget::overrideWindowState(Qt::WindowStates newstate)
{
   QWindowStateChangeEvent e(Qt::WindowStates(m_widgetData->window_state), true);
   m_widgetData->window_state = newstate;
   QApplication::sendEvent(this, &e);
}

Qt::WindowState effectiveState(Qt::WindowStates state)
{
   if (state & Qt::WindowMinimized) {
      return Qt::WindowMinimized;

   } else if (state & Qt::WindowFullScreen) {
      return Qt::WindowFullScreen;

   } else if (state & Qt::WindowMaximized) {
      return Qt::WindowMaximized;
   }

   return Qt::WindowNoState;
}

void QWidget::setWindowState(Qt::WindowStates newstate)
{
   Q_D(QWidget);

   Qt::WindowStates oldstate = windowState();

   if (oldstate == newstate) {
      return;
   }

   if (isWindow() && ! testAttribute(Qt::WA_WState_Created)) {
      create();
   }

   m_widgetData->window_state = newstate;
   m_widgetData->in_set_window_state = 1;
   Qt::WindowState newEffectiveState = effectiveState(newstate);
   Qt::WindowState oldEffectiveState = effectiveState(oldstate);

   if (isWindow() && newEffectiveState != oldEffectiveState) {
      // ensure the initial size is valid, since we store it as normalGeometry below

      if (! testAttribute(Qt::WA_Resized) && ! isVisible()) {
         adjustSize();
      }

      d->createTLExtra();

      if (oldEffectiveState == Qt::WindowNoState) {
         d->topData()->normalGeometry = geometry();
      }

      Q_ASSERT(windowHandle());
      windowHandle()->setWindowState(newEffectiveState);
   }

   m_widgetData->in_set_window_state = 0;

   if (newstate & Qt::WindowActive) {
      activateWindow();
   }

   QWindowStateChangeEvent e(oldstate);
   QApplication::sendEvent(this, &e);
}

bool QWidget::isFullScreen() const
{
   return m_widgetData->window_state & Qt::WindowFullScreen;
}

void QWidget::showFullScreen()
{

   ensurePolished();

   setWindowState((windowState() & ~(Qt::WindowMinimized | Qt::WindowMaximized)) | Qt::WindowFullScreen);
   setVisible(true);
   activateWindow();
}

void QWidget::showMaximized()
{
   ensurePolished();

   setWindowState((windowState() & ~(Qt::WindowMinimized | Qt::WindowFullScreen)) | Qt::WindowMaximized);

   setVisible(true);
}

void QWidget::showNormal()
{
   ensurePolished();
   setWindowState(windowState() & ~(Qt::WindowMinimized | Qt::WindowMaximized | Qt::WindowFullScreen));

   setVisible(true);
}

bool QWidget::isEnabledTo(const QWidget *ancestor) const
{
   const QWidget *w = this;

   while (! w->testAttribute(Qt::WA_ForceDisabled) && ! w->isWindow()
         && w->parentWidget() && w->parentWidget() != ancestor) {
      w = w->parentWidget();
   }

   return !w->testAttribute(Qt::WA_ForceDisabled);
}

#ifndef QT_NO_ACTION

void QWidget::addAction(QAction *action)
{
   insertAction(nullptr, action);
}

void QWidget::addActions(const QList<QAction *> &actions)
{
   for (auto item : actions) {
      insertAction(nullptr, item);
   }
}

void QWidget::insertAction(QAction *before, QAction *action)
{
   if (action == nullptr) {
      qWarning("QWidget::insertAction() New action has an invalid value (nullptr)");
      return;
   }

   Q_D(QWidget);

   if (d->actions.contains(action)) {
      removeAction(action);
   }

   int pos = d->actions.indexOf(before);

   if (pos < 0) {
      before = nullptr;
      pos = d->actions.size();
   }

   d->actions.insert(pos, action);

   QActionPrivate *apriv = action->d_func();
   apriv->widgets.append(this);

   QActionEvent e(QEvent::ActionAdded, action, before);
   QApplication::sendEvent(this, &e);
}

void QWidget::insertActions(QAction *before, QList<QAction *> actions)
{
   for (auto item : actions) {
      insertAction(before, item);
   }
}

void QWidget::removeAction(QAction *action)
{
   if (! action) {
      return;
   }

   Q_D(QWidget);

   QActionPrivate *apriv = action->d_func();
   apriv->widgets.removeAll(this);

   if (d->actions.removeAll(action)) {
      QActionEvent e(QEvent::ActionRemoved, action);
      QApplication::sendEvent(this, &e);
   }
}

QList<QAction *> QWidget::actions() const
{
   Q_D(const QWidget);
   return d->actions;
}
#endif // QT_NO_ACTION

void QWidget::setEnabled(bool enable)
{
   Q_D(QWidget);
   setAttribute(Qt::WA_ForceDisabled, !enable);
   d->setEnabled_helper(enable);
}

void QWidgetPrivate::setEnabled_helper(bool enable)
{
   Q_Q(QWidget);

   if (enable && ! q->isWindow() && q->parentWidget() && ! q->parentWidget()->isEnabled()) {
      return;   // nothing we can do
   }

   if (enable != q->testAttribute(Qt::WA_Disabled)) {
      return;   // nothing to do
   }

   q->setAttribute(Qt::WA_Disabled, !enable);
   updateSystemBackground();

   if (! enable && q->window()->focusWidget() == q) {
      bool parentIsEnabled = (! q->parentWidget() || q->parentWidget()->isEnabled());

      if (! parentIsEnabled || ! q->focusNextChild()) {
         q->clearFocus();
      }
   }

   Qt::WidgetAttribute attribute = enable ? Qt::WA_ForceDisabled : Qt::WA_Disabled;

   for (auto item : q->children()) {
      QWidget *w = dynamic_cast<QWidget *>(item);

      if (w != nullptr && ! w->testAttribute(attribute)) {
         w->d_func()->setEnabled_helper(enable);
      }
   }

#ifndef QT_NO_CURSOR

   if (q->testAttribute(Qt::WA_SetCursor) || q->isWindow()) {
      // enforce the windows behavior of clearing the cursor on disabled widgets
      cs_internal_set_cursor(q, false);
   }

#endif

#ifndef QT_NO_IM

   if (q->testAttribute(Qt::WA_InputMethodEnabled) && q->hasFocus()) {
      QWidget *focusWidget = effectiveFocusWidget();

      if (enable) {
         if (focusWidget->testAttribute(Qt::WA_InputMethodEnabled)) {
            QGuiApplication::inputMethod()->update(Qt::ImEnabled);
         }

      } else {
         QGuiApplication::inputMethod()->commit();
         QGuiApplication::inputMethod()->update(Qt::ImEnabled);
      }
   }

#endif

   QEvent e(QEvent::EnabledChange);
   QApplication::sendEvent(q, &e);
}

bool QWidget::acceptDrops() const
{
   return testAttribute(Qt::WA_AcceptDrops);
}

void QWidget::setAcceptDrops(bool on)
{
   setAttribute(Qt::WA_AcceptDrops, on);
}

void QWidgetPrivate::registerDropSite(bool on)
{
   (void) on;
}

void QWidget::setDisabled(bool disable)
{
   setEnabled(!disable);
}

QRect QWidget::frameGeometry() const
{
   Q_D(const QWidget);

   if (isWindow() && ! (windowType() == Qt::Popup)) {
      QRect fs = d->frameStrut();

      return QRect(m_widgetData->crect.x() - fs.left(),
            m_widgetData->crect.y() - fs.top(),
            m_widgetData->crect.width() + fs.left() + fs.right(),
            m_widgetData->crect.height() + fs.top() + fs.bottom());
   }

   return m_widgetData->crect;
}

int QWidget::x() const
{
   Q_D(const QWidget);

   if (isWindow() && ! (windowType() == Qt::Popup)) {
      return m_widgetData->crect.x() - d->frameStrut().left();
   }

   return m_widgetData->crect.x();
}

int QWidget::y() const
{
   Q_D(const QWidget);

   if (isWindow() && ! (windowType() == Qt::Popup)) {
      return m_widgetData->crect.y() - d->frameStrut().top();
   }

   return m_widgetData->crect.y();
}

QPoint QWidget::pos() const
{
   Q_D(const QWidget);

   QPoint result = m_widgetData->crect.topLeft();

   if (isWindow() && ! (windowType() == Qt::Popup)) {
      if (! d->maybeTopData() || ! d->maybeTopData()->posIncludesFrame) {
         result -= d->frameStrut().topLeft();
      }
   }

   return result;
}

QRect QWidget::normalGeometry() const
{
   Q_D(const QWidget);

   if (! d->extra || ! d->extra->topextra) {
      return QRect();
   }

   if (! isMaximized() && !isFullScreen()) {
      return geometry();
   }

   return d->topData()->normalGeometry;
}

QRect QWidget::childrenRect() const
{
   QRect r(0, 0, 0, 0);

   for (auto item : children()) {
      QWidget *w = dynamic_cast<QWidget *>(item);

      if (w != nullptr && ! w->isWindow() && ! w->isHidden()) {
         r |= w->geometry();
      }
   }

   return r;
}

QRegion QWidget::childrenRegion() const
{
   QRegion r;

   for (auto item : children()) {
      QWidget *w = dynamic_cast<QWidget *>(item);

      if (w != nullptr && ! w->isWindow() && ! w->isHidden()) {
         QRegion mask = w->mask();

         if (mask.isEmpty()) {
            r |= w->geometry();
         } else {
            r |= mask.translated(w->pos());
         }
      }
   }

   return r;
}

QSize QWidget::minimumSize() const
{
   Q_D(const QWidget);

   return d->extra ? QSize(d->extra->minw, d->extra->minh) : QSize(0, 0);
}

QSize QWidget::maximumSize() const
{
   Q_D(const QWidget);

   return d->extra ? QSize(d->extra->maxw, d->extra->maxh)
         : QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}

QSize QWidget::sizeIncrement() const
{
   Q_D(const QWidget);

   return (d->extra && d->extra->topextra)
         ? QSize(d->extra->topextra->incw, d->extra->topextra->inch) : QSize(0, 0);
}

QSize QWidget::baseSize() const
{
   Q_D(const QWidget);

   return (d->extra != nullptr && d->extra->topextra != nullptr)
         ? QSize(d->extra->topextra->basew, d->extra->topextra->baseh) : QSize(0, 0);
}

bool QWidgetPrivate::setMinimumSize_helper(int &minw, int &minh)
{
   Q_Q(QWidget);

   int mw = minw;
   int mh = minh;

   if (mw == QWIDGETSIZE_MAX) {
      mw = 0;
   }

   if (mh == QWIDGETSIZE_MAX) {
      mh = 0;
   }

   if (minw > QWIDGETSIZE_MAX || minh > QWIDGETSIZE_MAX) {
      qWarning("QWidget::setMinimumSize() For (%s/%s), largest allowed size is (%d,%d)",
            csPrintable(q->objectName()), csPrintable(q->metaObject()->className()), QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

      minw = mw = qMin(minw, QWIDGETSIZE_MAX);
      minh = mh = qMin(minh, QWIDGETSIZE_MAX);
   }

   if (minw < 0 || minh < 0) {
      qWarning("QWidget::setMinimumSize() For (%s/%s), negative sizes (%d,%d) are not allowed",
            csPrintable(q->objectName()), csPrintable(q->metaObject()->className()), minw, minh);

      minw = mw = qMax(minw, 0);
      minh = mh = qMax(minh, 0);
   }

   createExtra();

   if (extra->minw == mw && extra->minh == mh) {
      return false;
   }

   extra->minw = mw;
   extra->minh = mh;
   extra->explicitMinSize = (mw ? Qt::Horizontal : 0) | (mh ? Qt::Vertical : 0);
   return true;
}

void QWidgetPrivate::setConstraints_sys()
{
   Q_Q(QWidget);

   if (extra && q->windowHandle()) {
      QWindow *win = q->windowHandle();
      QWindowPrivate *winp = qt_window_private(win);

      winp->minimumSize = QSize(extra->minw, extra->minh);
      winp->maximumSize = QSize(extra->maxw, extra->maxh);

      if (extra->topextra) {
         winp->baseSize = QSize(extra->topextra->basew, extra->topextra->baseh);
         winp->sizeIncrement = QSize(extra->topextra->incw, extra->topextra->inch);
      }

      if (winp->platformWindow) {
         fixPosIncludesFrame();
         winp->platformWindow->propagateSizeHints();
      }
   }
}
void QWidget::setMinimumSize(int minw, int minh)
{
   Q_D(QWidget);

   if (! d->setMinimumSize_helper(minw, minh)) {
      return;
   }

   if (isWindow()) {
      d->setConstraints_sys();
   }

   if (minw > width() || minh > height()) {
      bool resized = testAttribute(Qt::WA_Resized);
      bool maximized = isMaximized();
      resize(qMax(minw, width()), qMax(minh, height()));
      setAttribute(Qt::WA_Resized, resized); //not a user resize

      if (maximized) {
         m_widgetData->window_state = m_widgetData->window_state | Qt::WindowMaximized;
      }
   }

#ifndef QT_NO_GRAPHICSVIEW

   if (d->extra) {
      if (d->extra->proxyWidget) {
         d->extra->proxyWidget->setMinimumSize(minw, minh);
      }
   }

#endif

   d->updateGeometry_helper(d->extra->minw == d->extra->maxw && d->extra->minh == d->extra->maxh);
}

bool QWidgetPrivate::setMaximumSize_helper(int &maxw, int &maxh)
{
   Q_Q(QWidget);

   if (maxw > QWIDGETSIZE_MAX || maxh > QWIDGETSIZE_MAX) {
      qWarning("QWidget::setMaximumSize() For (%s/%s), largest allowed size is (%d,%d)",
            csPrintable(q->objectName()), csPrintable(q->metaObject()->className()), QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

      maxw = qMin(maxw, QWIDGETSIZE_MAX);
      maxh = qMin(maxh, QWIDGETSIZE_MAX);
   }

   if (maxw < 0 || maxh < 0) {
      qWarning("QWidget::setMaximumSize() For (%s/%s), negative sizes (%d,%d) are not allowed",
            csPrintable(q->objectName()), csPrintable(q->metaObject()->className()), maxw, maxh);

      maxw = qMax(maxw, 0);
      maxh = qMax(maxh, 0);
   }

   createExtra();

   if (extra->maxw == maxw && extra->maxh == maxh) {
      return false;
   }

   extra->maxw = maxw;
   extra->maxh = maxh;
   extra->explicitMaxSize = (maxw != QWIDGETSIZE_MAX ? Qt::Horizontal : 0) |
         (maxh != QWIDGETSIZE_MAX ? Qt::Vertical : 0);

   return true;
}

void QWidget::setMaximumSize(int maxw, int maxh)
{
   Q_D(QWidget);

   if (! d->setMaximumSize_helper(maxw, maxh)) {
      return;
   }

   if (isWindow()) {
      d->setConstraints_sys();
   }

   if (maxw < width() || maxh < height()) {
      bool resized = testAttribute(Qt::WA_Resized);
      resize(qMin(maxw, width()), qMin(maxh, height()));
      setAttribute(Qt::WA_Resized, resized); //not a user resize
   }

#ifndef QT_NO_GRAPHICSVIEW

   if (d->extra) {
      if (d->extra->proxyWidget) {
         d->extra->proxyWidget->setMaximumSize(maxw, maxh);
      }
   }

#endif

   d->updateGeometry_helper(d->extra->minw == d->extra->maxw && d->extra->minh == d->extra->maxh);
}

void QWidget::setSizeIncrement(int w, int h)
{
   Q_D(QWidget);
   d->createTLExtra();
   QTLWExtra *x = d->topData();

   if (x->incw == w && x->inch == h) {
      return;
   }

   x->incw = w;
   x->inch = h;

   if (isWindow()) {
      d->setConstraints_sys();
   }
}

void QWidget::setBaseSize(int basew, int baseh)
{
   Q_D(QWidget);
   d->createTLExtra();
   QTLWExtra *x = d->topData();

   if (x->basew == basew && x->baseh == baseh) {
      return;
   }

   x->basew = basew;
   x->baseh = baseh;

   if (isWindow()) {
      d->setConstraints_sys();
   }
}

void QWidget::setFixedSize(const QSize &s)
{
   setFixedSize(s.width(), s.height());
}

void QWidget::setFixedSize(int w, int h)
{
   Q_D(QWidget);

   bool minSizeSet = d->setMinimumSize_helper(w, h);
   bool maxSizeSet = d->setMaximumSize_helper(w, h);

   if (!minSizeSet && !maxSizeSet) {
      return;
   }

   if (isWindow()) {
      d->setConstraints_sys();
   } else {
      d->updateGeometry_helper(true);
   }

   if (w != QWIDGETSIZE_MAX || h != QWIDGETSIZE_MAX) {
      resize(w, h);
   }
}

void QWidget::setMinimumWidth(int w)
{
   Q_D(QWidget);

   d->createExtra();
   uint expl = d->extra->explicitMinSize | (w ? Qt::Horizontal : 0);
   setMinimumSize(w, minimumSize().height());
   d->extra->explicitMinSize = expl;
}

void QWidget::setMinimumHeight(int h)
{
   Q_D(QWidget);

   d->createExtra();
   uint expl = d->extra->explicitMinSize | (h ? Qt::Vertical : 0);
   setMinimumSize(minimumSize().width(), h);
   d->extra->explicitMinSize = expl;
}

void QWidget::setMaximumWidth(int w)
{
   Q_D(QWidget);

   d->createExtra();
   uint expl = d->extra->explicitMaxSize | (w == QWIDGETSIZE_MAX ? 0 : Qt::Horizontal);
   setMaximumSize(w, maximumSize().height());
   d->extra->explicitMaxSize = expl;
}

void QWidget::setMaximumHeight(int h)
{
   Q_D(QWidget);

   d->createExtra();
   uint expl = d->extra->explicitMaxSize | (h == QWIDGETSIZE_MAX ? 0 : Qt::Vertical);
   setMaximumSize(maximumSize().width(), h);
   d->extra->explicitMaxSize = expl;
}

void QWidget::setFixedWidth(int w)
{
   Q_D(QWidget);

   d->createExtra();
   uint explMin = d->extra->explicitMinSize | Qt::Horizontal;
   uint explMax = d->extra->explicitMaxSize | Qt::Horizontal;
   setMinimumSize(w, minimumSize().height());
   setMaximumSize(w, maximumSize().height());
   d->extra->explicitMinSize = explMin;
   d->extra->explicitMaxSize = explMax;
}

void QWidget::setFixedHeight(int h)
{
   Q_D(QWidget);

   d->createExtra();
   uint explMin = d->extra->explicitMinSize | Qt::Vertical;
   uint explMax = d->extra->explicitMaxSize | Qt::Vertical;
   setMinimumSize(minimumSize().width(), h);
   setMaximumSize(maximumSize().width(), h);
   d->extra->explicitMinSize = explMin;
   d->extra->explicitMaxSize = explMax;
}

QPoint QWidget::mapTo(const QWidget *parent, const QPoint &pos) const
{
   QPoint p = pos;

   if (parent) {
      const QWidget *w = this;

      while (w != parent) {
         Q_ASSERT_X(w, "QWidget::mapTo(const QWidget *parent, const QPoint &pos)",
               "parent must be in a parent hierarchy");
         p = w->mapToParent(p);
         w = w->parentWidget();
      }
   }

   return p;
}

QPoint QWidget::mapFrom(const QWidget *parent, const QPoint &pos) const
{
   QPoint p(pos);

   if (parent) {
      const QWidget *w = this;

      while (w != parent) {
         Q_ASSERT_X(w, "QWidget::mapFrom(const QWidget *parent, const QPoint &pos)",
               "parent must be in parent hierarchy");

         p = w->mapFromParent(p);
         w = w->parentWidget();
      }
   }

   return p;
}

QPoint QWidget::mapToParent(const QPoint &pos) const
{
   return pos + m_widgetData->crect.topLeft();
}

QPoint QWidget::mapFromParent(const QPoint &pos) const
{
   return pos - m_widgetData->crect.topLeft();
}

QWidget *QWidget::window() const
{
   QWidget *w = const_cast<QWidget *>(this);
   QWidget *p = w->parentWidget();

   while (!w->isWindow() && p) {
      w = p;
      p = p->parentWidget();
   }

   return w;
}

QWidget *QWidget::nativeParentWidget() const
{
   QWidget *parent = parentWidget();

   while (parent && ! parent->internalWinId()) {
      parent = parent->parentWidget();
   }

   return parent;
}

QPalette::ColorRole QWidget::backgroundRole() const
{
   const QWidget *w = this;

   do {
      QPalette::ColorRole role = w->d_func()->bg_role;

      if (role != QPalette::NoRole) {
         return role;
      }

      if (w->isWindow() || w->windowType() == Qt::SubWindow) {
         break;
      }

      w = w->parentWidget();

   } while (w);

   return QPalette::Window;
}

void QWidget::setBackgroundRole(QPalette::ColorRole role)
{
   Q_D(QWidget);

   d->bg_role = role;
   d->updateSystemBackground();
   d->propagatePaletteChange();
   d->updateIsOpaque();
}

QPalette::ColorRole QWidget::foregroundRole() const
{
   Q_D(const QWidget);

   QPalette::ColorRole rl = QPalette::ColorRole(d->fg_role);

   if (rl != QPalette::NoRole) {
      return rl;
   }

   QPalette::ColorRole role = QPalette::WindowText;

   switch (backgroundRole()) {
      case QPalette::Button:
         role = QPalette::ButtonText;
         break;

      case QPalette::Base:
         role = QPalette::Text;
         break;

      case QPalette::Dark:
      case QPalette::Shadow:
         role = QPalette::Light;
         break;

      case QPalette::Highlight:
         role = QPalette::HighlightedText;
         break;

      case QPalette::ToolTipBase:
         role = QPalette::ToolTipText;
         break;

      default:
         ;
   }

   return role;
}

void QWidget::setForegroundRole(QPalette::ColorRole role)
{
   Q_D(QWidget);

   d->fg_role = role;
   d->updateSystemBackground();
   d->propagatePaletteChange();
}

const QPalette &QWidget::palette() const
{
   if (! isEnabled()) {
      m_widgetData->pal.setCurrentColorGroup(QPalette::Disabled);

   } else if ((! isVisible() || isActiveWindow())

#if defined(Q_OS_WIN)
         && ! QApplicationPrivate::isBlockedByModal(const_cast<QWidget *>(this))
#endif

   ) {
      m_widgetData->pal.setCurrentColorGroup(QPalette::Active);

   } else {

      m_widgetData->pal.setCurrentColorGroup(QPalette::Inactive);
   }

   return m_widgetData->pal;
}

void QWidget::setPalette(const QPalette &palette)
{
   Q_D(QWidget);

   setAttribute(Qt::WA_SetPalette, palette.resolve() != 0);

   // Determine which palette is inherited from this widget's ancestors and
   // QApplication::palette, resolve this against \a palette (attributes from
   // the inherited palette are copied over this widget's palette). Then
   // propagate this palette to this widget's children.

   QPalette naturalPalette = d->naturalWidgetPalette(d->inheritedPaletteResolveMask);
   QPalette resolvedPalette = palette.resolve(naturalPalette);
   d->setPalette_helper(resolvedPalette);
}

QPalette QWidgetPrivate::naturalWidgetPalette(uint inheritedMask) const
{
   Q_Q(const QWidget);
   QPalette naturalPalette = QApplication::palette(q);

   if (! q->testAttribute(Qt::WA_StyleSheet)
         && (! q->isWindow() || q->testAttribute(Qt::WA_WindowPropagation)

#ifndef QT_NO_GRAPHICSVIEW
         || (extra && extra->proxyWidget)
#endif
       )) {

      if (QWidget *p = q->parentWidget()) {
         if (!p->testAttribute(Qt::WA_StyleSheet)) {
            if (!naturalPalette.isCopyOf(QApplication::palette())) {
               QPalette inheritedPalette = p->palette();
               inheritedPalette.resolve(inheritedMask);
               naturalPalette = inheritedPalette.resolve(naturalPalette);
            } else {
               naturalPalette = p->palette();
            }
         }
      }

#ifndef QT_NO_GRAPHICSVIEW
      else if (extra && extra->proxyWidget) {
         QPalette inheritedPalette = extra->proxyWidget->palette();
         inheritedPalette.resolve(inheritedMask);
         naturalPalette = inheritedPalette.resolve(naturalPalette);
      }

#endif

   }

   naturalPalette.resolve(0);
   return naturalPalette;
}

void QWidgetPrivate::resolvePalette()
{
   QPalette naturalPalette  = naturalWidgetPalette(inheritedPaletteResolveMask);
   QPalette resolvedPalette = m_privateData.pal.resolve(naturalPalette);
   setPalette_helper(resolvedPalette);
}

void QWidgetPrivate::setPalette_helper(const QPalette &palette)
{
   Q_Q(QWidget);

   if (m_privateData.pal == palette && m_privateData.pal.resolve() == palette.resolve()) {
      return;
   }

   m_privateData.pal = palette;
   updateSystemBackground();
   propagatePaletteChange();
   updateIsOpaque();

   q->update();
   updateIsOpaque();
}

void QWidgetPrivate::updateSystemBackground()
{
}

void QWidget::setFont(const QFont &font)
{
   Q_D(QWidget);

#ifndef QT_NO_STYLE_STYLESHEET
   const QStyleSheetStyle *style;

   if (d->extra && (style = dynamic_cast<const QStyleSheetStyle *>(d->extra->style.data()))) {
      style->saveWidgetFont(this, font);
   }

#endif

   setAttribute(Qt::WA_SetFont, font.resolve() != 0);

   // Determine which font is inherited from this widget's ancestors and
   // QApplication::font, resolve this against \a font (attributes from the
   // inherited font are copied over). Then propagate this font to this
   // widget's children.

   QFont naturalFont = d->naturalWidgetFont(d->inheritedFontResolveMask);
   QFont resolvedFont = font.resolve(naturalFont);
   d->setFont_helper(resolvedFont);
}

QFont QWidgetPrivate::naturalWidgetFont(uint inheritedMask) const
{
   Q_Q(const QWidget);
   QFont naturalFont = QApplication::font(q);

   if (! q->testAttribute(Qt::WA_StyleSheet)
         && (!q->isWindow() || q->testAttribute(Qt::WA_WindowPropagation)

#ifndef QT_NO_GRAPHICSVIEW
         || (extra && extra->proxyWidget)
#endif
      )) {

      if (QWidget *p = q->parentWidget()) {
         if (!p->testAttribute(Qt::WA_StyleSheet)) {
            if (!naturalFont.isCopyOf(QApplication::font())) {
               if (inheritedMask != 0) {
                  QFont inheritedFont = p->font();
                  inheritedFont.resolve(inheritedMask);
                  naturalFont = inheritedFont.resolve(naturalFont);
               }

            } else {
               naturalFont = p->font();
            }
         }
      }

#ifndef QT_NO_GRAPHICSVIEW
      else if (extra && extra->proxyWidget) {
         if (inheritedMask != 0) {
            QFont inheritedFont = extra->proxyWidget->font();
            inheritedFont.resolve(inheritedMask);
            naturalFont = inheritedFont.resolve(naturalFont);
         }
      }

#endif
   }

   naturalFont.resolve(0);
   return naturalFont;
}

void QWidgetPrivate::resolveFont()
{
   QFont naturalFont  = naturalWidgetFont(inheritedFontResolveMask);
   QFont resolvedFont = m_privateData.fnt.resolve(naturalFont);
   setFont_helper(resolvedFont);
}

void QWidgetPrivate::updateFont(const QFont &font)
{
   Q_Q(QWidget);

#ifndef QT_NO_STYLE_STYLESHEET
   const QStyleSheetStyle *cssStyle;
   cssStyle = extra ? dynamic_cast<const QStyleSheetStyle *>(extra->style.data()) : nullptr;
#endif

   m_privateData.fnt = QFont(font, q);

   // Combine new mask with natural mask and propagate to children.
#ifndef QT_NO_GRAPHICSVIEW

   if (! q->parentWidget() && extra && extra->proxyWidget) {
      QGraphicsProxyWidget *p = extra->proxyWidget;
      inheritedFontResolveMask = p->d_func()->inheritedFontResolveMask | p->font().resolve();

   } else
#endif

      if (q->isWindow() && !q->testAttribute(Qt::WA_WindowPropagation)) {
         inheritedFontResolveMask = 0;
      }

   uint newMask = m_privateData.fnt.resolve() | inheritedFontResolveMask;

   for (int i = 0; i < q->children().size(); ++i) {
      QWidget *w = dynamic_cast<QWidget *>(q->children().at(i));

      if (w != nullptr) {

#ifndef QT_NO_STYLE_STYLESHEET

         if (w->testAttribute(Qt::WA_StyleSheet)) {
            // Style sheets follow a different font propagation scheme

            if (cssStyle) {
               cssStyle->updateStyleSheetFont(w);
            }

         } else if ((! w->isWindow() || w->testAttribute(Qt::WA_WindowPropagation))) {
            // Propagate font changes

            QWidgetPrivate *wd = w->d_func();
            wd->inheritedFontResolveMask = newMask;
            wd->resolveFont();
         }

#else

         if ((! w->isWindow() || w->testAttribute(Qt::WA_WindowPropagation))) {
            // Propagate font changes

            QWidgetPrivate *wd = w->d_func();
            wd->inheritedFontResolveMask = newMask;
            wd->resolveFont();
         }

#endif

      }
   }

#ifndef QT_NO_STYLE_STYLESHEET

   if (cssStyle) {
      cssStyle->updateStyleSheetFont(q);
   }

#endif

   QEvent e(QEvent::FontChange);
   QApplication::sendEvent(q, &e);
}

void QWidgetPrivate::setLayoutDirection_helper(Qt::LayoutDirection direction)
{
   Q_Q(QWidget);

   if ((direction == Qt::RightToLeft) == q->testAttribute(Qt::WA_RightToLeft)) {
      return;
   }

   q->setAttribute(Qt::WA_RightToLeft, (direction == Qt::RightToLeft));

   if (! q->children().isEmpty()) {

      for (auto item : q->children()) {
         QWidget *w = dynamic_cast<QWidget *>(item);

         if (w != nullptr && !w->isWindow() && !w->testAttribute(Qt::WA_SetLayoutDirection)) {
            w->d_func()->setLayoutDirection_helper(direction);
         }
      }
   }

   QEvent e(QEvent::LayoutDirectionChange);
   QApplication::sendEvent(q, &e);
}

void QWidgetPrivate::resolveLayoutDirection()
{
   Q_Q(const QWidget);

   if (! q->testAttribute(Qt::WA_SetLayoutDirection)) {
      setLayoutDirection_helper(q->isWindow() ? QApplication::layoutDirection() : q->parentWidget()->layoutDirection());
   }
}

void QWidget::setLayoutDirection(Qt::LayoutDirection direction)
{
   Q_D(QWidget);

   if (direction == Qt::LayoutDirectionAuto) {
      unsetLayoutDirection();
      return;
   }

   setAttribute(Qt::WA_SetLayoutDirection);
   d->setLayoutDirection_helper(direction);
}

Qt::LayoutDirection QWidget::layoutDirection() const
{
   return testAttribute(Qt::WA_RightToLeft) ? Qt::RightToLeft : Qt::LeftToRight;
}

void QWidget::unsetLayoutDirection()
{
   Q_D(QWidget);
   setAttribute(Qt::WA_SetLayoutDirection, false);
   d->resolveLayoutDirection();
}

#ifndef QT_NO_CURSOR
QCursor QWidget::cursor() const
{
   Q_D(const QWidget);

   if (testAttribute(Qt::WA_SetCursor)) {
      return (d->extra && d->extra->curs) ? *d->extra->curs : QCursor(Qt::ArrowCursor);
   }

   if (isWindow() || ! parentWidget()) {
      return QCursor(Qt::ArrowCursor);
   }

   return parentWidget()->cursor();
}

void QWidget::setCursor(const QCursor &cursor)
{
   Q_D(QWidget);

   if (cursor.shape() != Qt::ArrowCursor || (d->extra && d->extra->curs))  {
      d->createExtra();
      QCursor *newCursor = new QCursor(cursor);
      delete d->extra->curs;
      d->extra->curs = newCursor;
   }

   setAttribute(Qt::WA_SetCursor);
   d->setCursor_sys(cursor);

   QEvent event(QEvent::CursorChange);
   QApplication::sendEvent(this, &event);
}

void QWidgetPrivate::setCursor_sys(const QCursor &cursor)
{
   (void) cursor;

   Q_Q(QWidget);
   cs_internal_set_cursor(q, false);
}

void QWidget::unsetCursor()
{
   Q_D(QWidget);

   if (d->extra) {
      delete d->extra->curs;
      d->extra->curs = nullptr;
   }

   if (! isWindow()) {
      setAttribute(Qt::WA_SetCursor, false);
   }

   d->unsetCursor_sys();

   QEvent event(QEvent::CursorChange);
   QApplication::sendEvent(this, &event);
}

void QWidgetPrivate::unsetCursor_sys()
{
   Q_Q(QWidget);
   cs_internal_set_cursor(q, false);
}

static inline void applyCursor(QWidget *w, QCursor c)
{
   if (QWindow *window = w->windowHandle()) {
      window->setCursor(c);
   }
}

static inline void unsetCursor(QWidget *w)
{
   if (QWindow *window = w->windowHandle()) {
      window->unsetCursor();
   }
}

void cs_internal_set_cursor(QWidget *w, bool force)
{
   if (! w->testAttribute(Qt::WA_WState_Created)) {
      return;
   }

   static QPointer<QWidget> lastUnderMouse = nullptr;

   if (force) {
      lastUnderMouse = w;

   } else if (lastUnderMouse) {
      const WId lastWinId = lastUnderMouse->effectiveWinId();
      const WId winId = w->effectiveWinId();

      if (lastWinId && lastWinId == winId) {
         w = lastUnderMouse;
      }

   } else if (! w->internalWinId()) {
      return; // mouse is not under this widget and it is not native, do not change it
   }

   while (! w->internalWinId() && w->parentWidget() && ! w->isWindow()
         && ! w->testAttribute(Qt::WA_SetCursor)) {
      w = w->parentWidget();
   }

   QWidget *nativeParent = w;

   if (! w->internalWinId()) {
      nativeParent = w->nativeParentWidget();
   }

   if (! nativeParent || ! nativeParent->internalWinId()) {
      return;
   }

   if (w->isWindow() || w->testAttribute(Qt::WA_SetCursor)) {
      if (w->isEnabled()) {
         applyCursor(nativeParent, w->cursor());

      } else {
         // enforce the windows behavior of clearing the cursor on disabled widgets
         unsetCursor(nativeParent);
      }

   } else {
      unsetCursor(nativeParent);
   }
}
#endif

void QWidget::render(QPaintDevice *target, const QPoint &targetOffset,
      const QRegion &sourceRegion, RenderFlags renderFlags)
{
   QPainter p(target);
   render(&p, targetOffset, sourceRegion, renderFlags);
}

void QWidget::render(QPainter *painter, const QPoint &targetOffset,
      const QRegion &sourceRegion, RenderFlags renderFlags)
{
   if (! painter) {
      qWarning("QWidget::render() Painter has an invalid value (nullptr)");
      return;
   }

   if (! painter->isActive()) {
      qWarning("QWidget::render() Unable to render with an inactive Painter");
      return;
   }

   const qreal opacity = painter->opacity();

   if (qFuzzyIsNull(opacity)) {
      return;   // Fully transparent
   }

   Q_D(QWidget);

   const bool inRenderWithPainter = d->extra && d->extra->inRenderWithPainter;
   const QRegion toBePainted = ! inRenderWithPainter ?
         d->prepareToRender(sourceRegion, renderFlags) : sourceRegion;

   if (toBePainted.isEmpty()) {
      return;
   }

   if (! d->extra) {
      d->createExtra();
   }

   d->extra->inRenderWithPainter = true;

   QPaintEngine *engine = painter->paintEngine();
   Q_ASSERT(engine);

   QPaintEnginePrivate *enginePriv = engine->d_func();
   Q_ASSERT(enginePriv);

   QPaintDevice *target = engine->paintDevice();
   Q_ASSERT(target);

   // Render via a pixmap when dealing with non-opaque painters or printers.
   if (!inRenderWithPainter && (opacity < 1.0 || (target->devType() == QInternal::Printer))) {
      d->render_helper(painter, targetOffset, toBePainted, renderFlags);
      d->extra->inRenderWithPainter = inRenderWithPainter;
      return;
   }

   // Set new shared painter.
   QPainter *oldPainter = d->sharedPainter();
   d->setSharedPainter(painter);

   // Save current system clip, viewport and transform,
   const QTransform oldTransform   = enginePriv->systemTransform;
   const QRegion oldSystemClip     = enginePriv->systemClip;
   const QRegion oldSystemViewport = enginePriv->systemViewport;

   // This ensures that all painting triggered by render() is clipped to the current engine clip.
   if (painter->hasClipping()) {
      const QRegion painterClip = painter->deviceTransform().map(painter->clipRegion());
      enginePriv->setSystemViewport(oldSystemClip.isEmpty() ? painterClip : oldSystemClip & painterClip);
   } else {
      enginePriv->setSystemViewport(oldSystemClip);
   }

   d->render(target, targetOffset, toBePainted, renderFlags);

   // Restore system clip, viewport and transform
   enginePriv->systemClip = oldSystemClip;
   enginePriv->setSystemViewport(oldSystemViewport);
   enginePriv->setSystemTransform(oldTransform);

   // Restore shared painter
   d->setSharedPainter(oldPainter);

   d->extra->inRenderWithPainter = inRenderWithPainter;
}

static void sendResizeEvents(QWidget *target)
{
   QResizeEvent e(target->size(), QSize());
   QApplication::sendEvent(target, &e);

   const QObjectList children = target->children();

   for (auto item : children) {
      if (! item->isWidgetType()) {
         continue;
      }

      QWidget *child = static_cast<QWidget *>(item);

      if (! child->isWindow() && child->testAttribute(Qt::WA_PendingResizeEvent)) {
         sendResizeEvents(child);
      }
   }
}

QPixmap QWidget::grab(const QRect &rectangle)
{
   Q_D(QWidget);

   if (testAttribute(Qt::WA_PendingResizeEvent) || !testAttribute(Qt::WA_WState_Created)) {
      sendResizeEvents(this);
   }

   const QWidget::RenderFlags renderFlags = QWidget::DrawWindowBackground | QWidget::DrawChildren | QWidget::IgnoreMask;

   const bool oldDirtyOpaqueChildren =  d->dirtyOpaqueChildren;
   QRect r(rectangle);

   if (r.width() < 0 || r.height() < 0) {
      // For grabbing widgets that haven't been shown yet,
      // we trigger the layouting mechanism to determine the widget's size.
      r = d->prepareToRender(QRegion(), renderFlags).boundingRect();
      r.setTopLeft(rectangle.topLeft());
   }

   if (! r.intersects(rect())) {
      return QPixmap();
   }

   const qreal dpr = devicePixelRatioF();
   QPixmap res((QSizeF(r.size()) * dpr).toSize());
   res.setDevicePixelRatio(dpr);

   if (!d->isOpaque) {
      res.fill(Qt::transparent);
   }

   d->render(&res, QPoint(), QRegion(r), renderFlags);
   d->dirtyOpaqueChildren = oldDirtyOpaqueChildren;

   return res;
}

#ifndef QT_NO_GRAPHICSEFFECT
QGraphicsEffect *QWidget::graphicsEffect() const
{
   Q_D(const QWidget);
   return d->graphicsEffect;
}

void QWidget::setGraphicsEffect(QGraphicsEffect *effect)
{
   Q_D(QWidget);

   if (d->graphicsEffect == effect) {
      return;
   }

   if (d->graphicsEffect) {
      d->invalidateBuffer(rect());
      delete d->graphicsEffect;
      d->graphicsEffect = nullptr;
   }

   if (effect) {
      // Set new effect.
      QGraphicsEffectSourcePrivate *sourced = new QWidgetEffectSourcePrivate(this);
      QGraphicsEffectSource *source = new QGraphicsEffectSource(*sourced);
      d->graphicsEffect = effect;
      effect->d_func()->setGraphicsEffectSource(source);
      update();
   }

   d->updateIsOpaque();
}
#endif

bool QWidgetPrivate::isAboutToShow() const
{
   if (m_privateData.in_show) {
      return true;
   }

   Q_Q(const QWidget);

   if (q->isHidden()) {
      return false;
   }

   // widget will be shown if any of its ancestors are about to show
   QWidget *parent = q->parentWidget();
   return parent ? parent->d_func()->isAboutToShow() : false;
}

QRegion QWidgetPrivate::prepareToRender(const QRegion &region, QWidget::RenderFlags renderFlags)
{
   Q_Q(QWidget);
   const bool isVisible = q->isVisible();

   // Make sure the widget is laid out correctly
   if (! isVisible && !isAboutToShow()) {
      QWidget *topLevel = q->window();

      (void)topLevel->d_func()->topData();   // Make sure we at least have top-data
      topLevel->ensurePolished();

      // Invalidate the layout of hidden ancestors (incl. myself) and pretend
      // they are not explicitly hidden.
      QWidget *widget = q;
      QWidgetList hiddenWidgets;

      while (widget != nullptr) {
         if (widget->isHidden()) {
            widget->setAttribute(Qt::WA_WState_Hidden, false);
            hiddenWidgets.append(widget);

            if (! widget->isWindow() && widget->parentWidget()->d_func()->layout) {
               widget->d_func()->updateGeometry_helper(true);
            }
         }

         widget = widget->parentWidget();
      }

      // Activate top-level layout
      if (topLevel->d_func()->layout) {
         topLevel->d_func()->layout->activate();
      }

      // Adjust size if necessary
      QTLWExtra *topLevelExtra = topLevel->d_func()->maybeTopData();

      if (topLevelExtra && !topLevelExtra->sizeAdjusted
            && !topLevel->testAttribute(Qt::WA_Resized)) {
         topLevel->adjustSize();
         topLevel->setAttribute(Qt::WA_Resized, false);
      }

      // Activate child layouts
      topLevel->d_func()->activateChildLayoutsRecursively();

      // we not cheating with WA_WState_Hidden anymore
      for (auto item : hiddenWidgets) {
         QWidget *widget = item;
         widget->setAttribute(Qt::WA_WState_Hidden);

         if (! widget->isWindow() && widget->parentWidget()->d_func()->layout) {
            widget->parentWidget()->d_func()->layout->invalidate();
         }
      }

   } else if (isVisible) {
      q->window()->d_func()->sendPendingMoveAndResizeEvents(true, true);
   }

   // Calculate the region to be painted
   QRegion toBePainted = !region.isEmpty() ? region : QRegion(q->rect());

   if (! (renderFlags & QWidget::IgnoreMask) && extra && extra->hasMask) {
      toBePainted &= extra->mask;
   }

   return toBePainted;
}

void QWidgetPrivate::render_helper(QPainter *painter, const QPoint &targetOffset, const QRegion &toBePainted,
      QWidget::RenderFlags renderFlags)
{
   Q_ASSERT(painter);
   Q_ASSERT(!toBePainted.isEmpty());

   Q_Q(QWidget);

   const QTransform originalTransform = painter->worldTransform();
   const bool useDeviceCoordinates = originalTransform.isScaling();

   if ( !useDeviceCoordinates) {
      // Render via a pixmap.
      const QRect rect = toBePainted.boundingRect();
      const QSize size = rect.size();

      if (size.isNull()) {
         return;
      }

      const qreal pixmapDevicePixelRatio = painter->device()->devicePixelRatioF();
      QPixmap pixmap(size * pixmapDevicePixelRatio);
      pixmap.setDevicePixelRatio(pixmapDevicePixelRatio);

      if (! (renderFlags & QWidget::DrawWindowBackground) || !isOpaque) {
         pixmap.fill(Qt::transparent);
      }

      q->render(&pixmap, QPoint(), toBePainted, renderFlags);

      const bool restore = !(painter->renderHints() & QPainter::SmoothPixmapTransform);
      painter->setRenderHints(QPainter::SmoothPixmapTransform, true);

      painter->drawPixmap(targetOffset, pixmap);

      if (restore) {
         painter->setRenderHints(QPainter::SmoothPixmapTransform, false);
      }

   } else {
      // Render via a pixmap in device coordinates (to avoid pixmap scaling).
      QTransform transform = originalTransform;
      transform.translate(targetOffset.x(), targetOffset.y());

      QPaintDevice *device = painter->device();
      Q_ASSERT(device);

      // Calculate device rect.
      const QRectF rect(toBePainted.boundingRect());
      QRect deviceRect = transform.mapRect(QRectF(0, 0, rect.width(), rect.height())).toAlignedRect();
      deviceRect &= QRect(0, 0, device->width(), device->height());

      QPixmap pixmap(deviceRect.size());
      pixmap.fill(Qt::transparent);

      // Create a pixmap device coordinate painter.
      QPainter pixmapPainter(&pixmap);
      pixmapPainter.setRenderHints(painter->renderHints());
      transform *= QTransform::fromTranslate(-deviceRect.x(), -deviceRect.y());
      pixmapPainter.setTransform(transform);

      q->render(&pixmapPainter, QPoint(), toBePainted, renderFlags);
      pixmapPainter.end();

      // And then draw the pixmap.
      painter->setTransform(QTransform());
      painter->drawPixmap(deviceRect.topLeft(), pixmap);
      painter->setTransform(originalTransform);
   }
}

void QWidgetPrivate::drawWidget(QPaintDevice *pdev, const QRegion &rgn, const QPoint &offset, int flags,
      QPainter *sharedPainter, QWidgetBackingStore *backingStore)
{
   if (rgn.isEmpty()) {
      return;
   }

   const bool asRoot = flags & DrawAsRoot;
   bool onScreen = paintOnScreen();

   Q_Q(QWidget);

#ifndef QT_NO_GRAPHICSEFFECT

   if (graphicsEffect && graphicsEffect->isEnabled()) {
      QGraphicsEffectSource *source = graphicsEffect->d_func()->source;

      QWidgetEffectSourcePrivate *sourced = static_cast<QWidgetEffectSourcePrivate *>
            (source->d_func());

      if (!sourced->context) {
         QWidgetPaintContext context(pdev, rgn, offset, flags, sharedPainter, backingStore);
         sourced->context = &context;

         if (!sharedPainter) {

            setSystemClip(pdev, rgn.translated(offset));
            QPainter p(pdev);
            p.translate(offset);
            context.painter = &p;
            graphicsEffect->draw(&p);
            setSystemClip(pdev, QRegion());

         } else {
            context.painter = sharedPainter;

            if (sharedPainter->worldTransform() != sourced->lastEffectTransform) {
               sourced->invalidateCache();
               sourced->lastEffectTransform = sharedPainter->worldTransform();
            }

            sharedPainter->save();
            sharedPainter->translate(offset);
            graphicsEffect->draw(sharedPainter);
            sharedPainter->restore();
         }

         sourced->context = nullptr;

         // Native widgets need to be marked dirty on screen so painting will be done in correct context
         // Same check as in the no effects case below.
         if (backingStore && !onScreen && !asRoot && (q->internalWinId() || !q->nativeParentWidget()->isWindow())) {
            backingStore->markDirtyOnScreen(rgn, q, offset);
         }

         return;
      }
   }

#endif

   const bool alsoOnScreen  = flags & DrawPaintOnScreen;
   const bool recursive     = flags & DrawRecursive;
   const bool alsoInvisible = flags & DrawInvisible;

   Q_ASSERT(sharedPainter ? sharedPainter->isActive() : true);

   QRegion toBePainted(rgn);

   if (asRoot && ! alsoInvisible) {
      toBePainted &= clipRect();   //(rgn & visibleRegion());
   }

   if (! (flags & DontSubtractOpaqueChildren)) {
      subtractOpaqueChildren(toBePainted, q->rect());
   }

   if (! toBePainted.isEmpty()) {

      if (!onScreen || alsoOnScreen) {
         //update the "in paint event" flag
         if (q->testAttribute(Qt::WA_WState_InPaintEvent)) {
            qWarning("QWidget::repaint() Recursion in repaint event");
         }

         q->setAttribute(Qt::WA_WState_InPaintEvent);

         // clip away the new area
         QPaintEngine *paintEngine = pdev->paintEngine();

         if (paintEngine) {
            setRedirected(pdev, -offset);

            if (sharedPainter) {
               setSystemClip(pdev, toBePainted);
            } else {
               paintEngine->d_func()->systemRect = q->m_widgetData->crect;
            }

            // paint the background
            if ((asRoot || q->autoFillBackground() || onScreen || q->testAttribute(Qt::WA_StyledBackground))
                  && !q->testAttribute(Qt::WA_OpaquePaintEvent) && !q->testAttribute(Qt::WA_NoSystemBackground)) {

#ifndef QT_NO_OPENGL
               beginBackingStorePainting();
#endif
               QPainter p(q);
               paintBackground(&p, toBePainted, (asRoot || onScreen) ? flags | DrawAsRoot : 0);

#ifndef QT_NO_OPENGL
               endBackingStorePainting();
#endif
            }

            if (! sharedPainter) {
               setSystemClip(pdev, toBePainted.translated(offset));
            }

            if (!onScreen && !asRoot && !isOpaque && q->testAttribute(Qt::WA_TintedBackground)) {

#ifndef QT_NO_OPENGL
               beginBackingStorePainting();
#endif

               QPainter p(q);
               QColor tint = q->palette().window().color();
               tint.setAlphaF(qreal(.6));
               p.fillRect(toBePainted.boundingRect(), tint);

#ifndef QT_NO_OPENGL
               endBackingStorePainting();
#endif
            }
         }

         bool skipPaintEvent = false;

#ifndef QT_NO_OPENGL

         if (renderToTexture) {
            // This widget renders into a texture which is composed later. We just need to
            // punch a hole in the backingstore, so the texture will be visible.
            if (!q->testAttribute(Qt::WA_AlwaysStackOnTop)) {
               beginBackingStorePainting();

               if (backingStore) {
                  QPainter p(q);
                  p.setCompositionMode(QPainter::CompositionMode_Source);
                  p.fillRect(q->rect(), Qt::transparent);
               } else {
                  QImage img = grabFramebuffer();
                  QPainter p(q);
                  // We are not drawing to a backingstore: fall back to QImage
                  p.drawImage(q->rect(), img);
                  skipPaintEvent = true;
               }

               endBackingStorePainting();
            }

            if (renderToTextureReallyDirty) {
               renderToTextureReallyDirty = 0;
            } else {
               skipPaintEvent = true;
            }
         }

#endif

         if (! skipPaintEvent) {
            //actually send the paint event
            sendPaintEvent(toBePainted);
         }

         // Native widgets need to be marked dirty on screen so painting will be done in correct context
         if (backingStore && !onScreen && ! asRoot && (q->internalWinId() || (q->nativeParentWidget() &&
               ! q->nativeParentWidget()->isWindow()))) {
            backingStore->markDirtyOnScreen(toBePainted, q, offset);
         }

         //restore
         if (paintEngine) {
            restoreRedirected();

            if (!sharedPainter) {
               paintEngine->d_func()->systemRect = QRect();
            } else {
               paintEngine->d_func()->currentClipDevice = nullptr;
            }

            setSystemClip(pdev, QRegion());
         }

         q->setAttribute(Qt::WA_WState_InPaintEvent, false);

         if (q->paintingActive())  {
            qWarning("QWidget::repaint() Painters should not be active on a widget outside of the PaintEvent");
         }

         if (paintEngine && paintEngine->autoDestruct()) {
            delete paintEngine;
         }

      } else if (q->isWindow()) {
         QPaintEngine *engine = pdev->paintEngine();

         if (engine) {
            QPainter p(pdev);
            p.setClipRegion(toBePainted);
            const QBrush bg = q->palette().brush(QPalette::Window);

            if (bg.style() == Qt::TexturePattern) {
               p.drawTiledPixmap(q->rect(), bg.texture());
            } else {
               p.fillRect(q->rect(), bg);
            }

            if (engine->autoDestruct()) {
               delete engine;
            }
         }
      }
   }

   if (recursive && ! q->children().isEmpty()) {
      paintSiblingsRecursive(pdev, q->children(), q->children().size() - 1, rgn, offset,
            flags & ~DrawAsRoot, sharedPainter, backingStore);
   }
}

void QWidgetPrivate::sendPaintEvent(const QRegion &toBePainted)
{
   Q_Q(QWidget);

   QPaintEvent e(toBePainted);
   QCoreApplication::sendSpontaneousEvent(q, &e);

#ifndef QT_NO_OPENGL

   if (renderToTexture) {
      resolveSamples();
   }

#endif
}

void QWidgetPrivate::render(QPaintDevice *target, const QPoint &targetOffset,
      const QRegion &sourceRegion, QWidget::RenderFlags renderFlags)
{
   if (! target) {
      qWarning("QWidget::render() Paint device has an invalid value (nullptr)");
      return;
   }

   const bool inRenderWithPainter = extra && extra->inRenderWithPainter;

   QRegion paintRegion;

   if (! inRenderWithPainter)  {
      paintRegion = prepareToRender(sourceRegion, renderFlags);
   } else {
      paintRegion = sourceRegion;
   }

   if (paintRegion.isEmpty()) {
      return;
   }

   QPainter *oldSharedPainter = inRenderWithPainter ? sharedPainter() : nullptr;

   // Use the target's shared painter if set (typically set when doing
   // "other->render(widget);" in the widget's paintEvent.
   if (target->devType() == QInternal::Widget) {
      QWidgetPrivate *targetPrivate = static_cast<QWidget *>(target)->d_func();

      if (targetPrivate->extra && targetPrivate->extra->inRenderWithPainter) {
         QPainter *targetPainter = targetPrivate->sharedPainter();

         if (targetPainter && targetPainter->isActive()) {
            setSharedPainter(targetPainter);
         }
      }
   }

   // Use the target's redirected device if set and adjust offset and paint
   // region accordingly. This is typically the case when people call render
   // from the paintEvent.
   QPoint offset = targetOffset;
   offset -= paintRegion.boundingRect().topLeft();
   QPoint redirectionOffset;
   QPaintDevice *redirected = nullptr;

   if (target->devType() == QInternal::Widget) {
      redirected = static_cast<QWidget *>(target)->d_func()->redirected(&redirectionOffset);
   }

   if (! redirected) {
      redirected = QPainter::redirected(target, &redirectionOffset);
   }

   if (redirected) {
      target = redirected;
      offset -= redirectionOffset;
   }

   if (! inRenderWithPainter) {
      // Clip handled by shared painter (in qpainter.cpp).

      if (QPaintEngine *targetEngine = target->paintEngine()) {
         const QRegion targetSystemClip = targetEngine->systemClip();

         if (! targetSystemClip.isEmpty()) {
            paintRegion &= targetSystemClip.translated(-offset);
         }
      }
   }

   // Set backingstore flags.
   int flags = DrawPaintOnScreen | DrawInvisible;

   if (renderFlags & QWidget::DrawWindowBackground) {
      flags |= DrawAsRoot;
   }

   if (renderFlags & QWidget::DrawChildren) {
      flags |= DrawRecursive;
   } else {
      flags |= DontSubtractOpaqueChildren;
   }

   flags |= DontSetCompositionMode;

   // Render via backingstore
   drawWidget(target, paintRegion, offset, flags, sharedPainter());

   // Restore shared painter
   if (oldSharedPainter) {
      setSharedPainter(oldSharedPainter);
   }
}

void QWidgetPrivate::paintSiblingsRecursive(QPaintDevice *pdev, const QObjectList &siblings, int index, const QRegion &rgn,
      const QPoint &offset, int flags, QPainter *sharedPainter, QWidgetBackingStore *backingStore)
{
   QWidget *w = nullptr;
   QRect boundingRect;
   bool dirtyBoundingRect = true;

   const bool exludeOpaqueChildren = (flags & DontDrawOpaqueChildren);
   const bool excludeNativeChildren = (flags & DontDrawNativeChildren);

   do {
      QWidget *x =  dynamic_cast<QWidget *>(siblings.at(index));

      if (x != nullptr && !(exludeOpaqueChildren && x->d_func()->isOpaque) && !x->isHidden() && !x->isWindow()
            && !(excludeNativeChildren && x->internalWinId())) {

         if (dirtyBoundingRect) {
            boundingRect = rgn.boundingRect();
            dirtyBoundingRect = false;
         }

         if (qRectIntersects(boundingRect, x->d_func()->effectiveRectFor(x->m_widgetData->crect))) {
            w = x;
            break;
         }
      }

      --index;

   } while (index >= 0);

   if (! w) {
      return;
   }

   QWidgetPrivate *wd = w->d_func();
   const QPoint widgetPos(w->m_widgetData->crect.topLeft());
   const bool hasMask = wd->extra && wd->extra->hasMask && ! wd->graphicsEffect;

   if (index > 0) {
      QRegion wr(rgn);

      if (wd->isOpaque) {
         wr -= hasMask ? wd->extra->mask.translated(widgetPos) : w->m_widgetData->crect;
      }

      paintSiblingsRecursive(pdev, siblings, --index, wr, offset, flags, sharedPainter, backingStore);
   }

#ifdef QT_NO_GRAPHICSVIEW

   if (w->updatesEnabled()) {
#else

   if (w->updatesEnabled() && (! w->d_func()->extra || ! w->d_func()->extra->proxyWidget)) {
#endif

      QRegion wRegion(rgn);
      wRegion &= wd->effectiveRectFor(w->m_widgetData->crect);
      wRegion.translate(-widgetPos);

      if (hasMask) {
         wRegion &= wd->extra->mask;
      }

      wd->drawWidget(pdev, wRegion, offset + widgetPos, flags, sharedPainter, backingStore);
   }
}

#ifndef QT_NO_GRAPHICSEFFECT
QRectF QWidgetEffectSourcePrivate::boundingRect(Qt::CoordinateSystem system) const
{
   if (system != Qt::DeviceCoordinates) {
      return m_widget->rect();
   }

   if (! context) {
      // Device coordinates without context not yet supported
      qWarning("QGraphicsEffectSource::boundingRect() Device context is missing, unable to create the bounding rectangle");
      return QRectF();
   }

   return context->painter->worldTransform().mapRect(m_widget->rect());
}

void QWidgetEffectSourcePrivate::draw(QPainter *painter)
{
   if (! context || context->painter != painter) {
      m_widget->render(painter);
      return;
   }

   // The region saved in the context is neither clipped to the rect
   // nor the mask, so we have to clip it here before calling drawWidget.
   QRegion toBePainted = context->rgn;
   toBePainted &= m_widget->rect();

   QWidgetPrivate *wd = qt_widget_private(m_widget);

   if (wd->extra && wd->extra->hasMask) {
      toBePainted &= wd->extra->mask;
   }

   wd->drawWidget(context->pdev, toBePainted, context->offset, context->flags,
         context->sharedPainter, context->backingStore);
}

QPixmap QWidgetEffectSourcePrivate::pixmap(Qt::CoordinateSystem system, QPoint *offset,
      QGraphicsEffect::PixmapPadMode mode) const
{
   const bool deviceCoordinates = (system == Qt::DeviceCoordinates);

   if (! context && deviceCoordinates) {
      // Device coordinates without context not yet supported.
      qWarning("QGraphicsEffectSource::pixmap() Device context is missing, unable to create the pixmap");
      return QPixmap();
   }

   QPoint pixmapOffset;
   QRectF sourceRect = m_widget->rect();

   if (deviceCoordinates) {
      const QTransform &painterTransform = context->painter->worldTransform();
      sourceRect = painterTransform.mapRect(sourceRect);
      pixmapOffset = painterTransform.map(pixmapOffset);
   }

   QRect effectRect;

   if (mode == QGraphicsEffect::PadToEffectiveBoundingRect) {
      effectRect = m_widget->graphicsEffect()->boundingRectFor(sourceRect).toAlignedRect();
   } else if (mode == QGraphicsEffect::PadToTransparentBorder) {
      effectRect = sourceRect.adjusted(-1, -1, 1, 1).toAlignedRect();
   } else {
      effectRect = sourceRect.toAlignedRect();
   }

   if (offset) {
      *offset = effectRect.topLeft();
   }

   pixmapOffset -= effectRect.topLeft();

   const qreal dpr = context->painter->device()->devicePixelRatio();
   QPixmap pixmap(effectRect.size() * dpr);
   pixmap.setDevicePixelRatio(dpr);
   pixmap.fill(Qt::transparent);
   m_widget->render(&pixmap, pixmapOffset, QRegion(), QWidget::DrawChildren);
   return pixmap;
}
#endif

#ifndef QT_NO_GRAPHICSVIEW
QGraphicsProxyWidget *QWidgetPrivate::nearestGraphicsProxyWidget(const QWidget *origin)
{
   if (origin) {
      QWExtra *extra = origin->d_func()->extra;

      if (extra && extra->proxyWidget) {
         return extra->proxyWidget;
      }

      return nearestGraphicsProxyWidget(origin->parentWidget());
   }

   return nullptr;
}
#endif

void QWidgetPrivate::setLocale_helper(const QLocale &loc, bool forceUpdate)
{
   Q_Q(QWidget);

   if (locale == loc && ! forceUpdate) {
      return;
   }

   locale = loc;

   if (! q->children().isEmpty()) {
      for (auto item : q->children()) {
         QWidget *w = dynamic_cast<QWidget *>(item);

         if (w == nullptr) {
            continue;
         }

         if (w->testAttribute(Qt::WA_SetLocale)) {
            continue;
         }

         if (w->isWindow() && !w->testAttribute(Qt::WA_WindowPropagation)) {
            continue;
         }

         w->d_func()->setLocale_helper(loc, forceUpdate);
      }
   }

   QEvent e(QEvent::LocaleChange);
   QApplication::sendEvent(q, &e);
}

void QWidget::setLocale(const QLocale &locale)
{
   Q_D(QWidget);

   setAttribute(Qt::WA_SetLocale);
   d->setLocale_helper(locale);
}

QLocale QWidget::locale() const
{
   Q_D(const QWidget);

   return d->locale;
}

void QWidgetPrivate::resolveLocale()
{
   Q_Q(const QWidget);

   if (!q->testAttribute(Qt::WA_SetLocale)) {
      setLocale_helper(q->isWindow() ? QLocale() : q->parentWidget()->locale());
   }
}

void QWidget::unsetLocale()
{
   Q_D(QWidget);

   setAttribute(Qt::WA_SetLocale, false);
   d->resolveLocale();
}

QString QWidget::windowTitle() const
{
   Q_D(const QWidget);

   if (d->extra && d->extra->topextra) {
      if (!d->extra->topextra->caption.isEmpty()) {
         return d->extra->topextra->caption;
      }

      if (! d->extra->topextra->filePath.isEmpty()) {
         return QFileInfo(d->extra->topextra->filePath).fileName() + "[*]";
      }
   }

   return QString();
}

QString cs_internal_parseWindowTitle(const QString &title, const QWidget *widget)
{
   Q_ASSERT(widget);

   QString cap = title;

   if (cap.isEmpty()) {
      return cap;
   }

   QString placeHolder("[*]");
   int index = cap.indexOf(placeHolder);

   while (index != -1) {
      index += placeHolder.size();
      int count = 1;

      while (cap.indexOf(placeHolder, index) == index) {
         ++count;
         index += placeHolder.size();
      }

      if (count % 2) {
         // odd number of [*] -> replace last one
         int lastIndex = cap.lastIndexOf(placeHolder, index - 1);

         if (widget->isWindowModified() && widget->style()->styleHint(QStyle::SH_TitleBar_ModifyNotification, nullptr, widget)) {
            cap.replace(lastIndex, 3, QWidget::tr("*"));
         } else {
            cap.remove(lastIndex, 3);
         }
      }

      index = cap.indexOf(placeHolder, index);
   }

   cap.replace("[*][*]", placeHolder);

   return cap;
}

void QWidgetPrivate::setWindowTitle_helper(const QString &title)
{
   Q_Q(QWidget);

   if (q->testAttribute(Qt::WA_WState_Created)) {
      setWindowTitle_sys(cs_internal_parseWindowTitle(title, q));
   }
}

void QWidgetPrivate::setWindowTitle_sys(const QString &caption)
{
   Q_Q(QWidget);

   if (! q->isWindow()) {
      return;
   }

   if (QWindow *window = q->windowHandle()) {
      window->setTitle(caption);
   }
}

void QWidgetPrivate::setWindowIconText_helper(const QString &title)
{
   Q_Q(QWidget);

   if (q->testAttribute(Qt::WA_WState_Created)) {
      setWindowIconText_sys(cs_internal_parseWindowTitle(title, q));
   }
}

void QWidgetPrivate::setWindowIconText_sys(const QString &iconText)
{
   Q_Q(QWidget);

   // QWidget property is deprecated, but the XCB window function is not.
   // It should remain available for the rare application that needs it.

   if (QWindow *window = q->windowHandle()) {
      QXcbWindowFunctions::setWmWindowIconText(window, iconText);
   }
}

void QWidget::setWindowIconText(const QString &iconText)
{
   if (QWidget::windowIconText() == iconText) {
      return;
   }

   Q_D(QWidget);

   d->topData()->iconText = iconText;
   d->setWindowIconText_helper(iconText);

   QEvent e(QEvent::IconTextChange);
   QApplication::sendEvent(this, &e);
   emit windowIconTextChanged(iconText);
}

void QWidget::setWindowTitle(const QString &title)
{
   if (QWidget::windowTitle() == title && ! title.isEmpty()) {
      return;
   }

   Q_D(QWidget);

   d->topData()->caption = title;
   d->setWindowTitle_helper(title);

   QEvent e(QEvent::WindowTitleChange);
   QApplication::sendEvent(this, &e);
   emit windowTitleChanged(title);
}

QIcon QWidget::windowIcon() const
{
   const QWidget *w = this;

   while (w) {
      const QWidgetPrivate *d = w->d_func();

      if (d->extra && d->extra->topextra && d->extra->topextra->icon) {
         return *d->extra->topextra->icon;
      }

      w = w->parentWidget();
   }

   return QApplication::windowIcon();
}

void QWidgetPrivate::setWindowIcon_helper()
{
   Q_Q(QWidget);

   QEvent e(QEvent::WindowIconChange);

   if (! q->windowHandle()) {
      QApplication::sendEvent(q_func(), &e);
   }

   for (auto item : q->children()) {
      QWidget *w = dynamic_cast<QWidget *>(item);

      if (w != nullptr && ! w->isWindow()) {
         QApplication::sendEvent(w, &e);
      }
   }
}

void QWidget::setWindowIcon(const QIcon &icon)
{
   Q_D(QWidget);

   setAttribute(Qt::WA_SetWindowIcon, !icon.isNull());
   d->createTLExtra();

   if (! d->extra->topextra->icon) {
      d->extra->topextra->icon = new QIcon();
   }

   *d->extra->topextra->icon = icon;

   d->setWindowIcon_sys();
   d->setWindowIcon_helper();
   emit windowIconChanged(icon);
}

void QWidgetPrivate::setWindowIcon_sys()
{
   Q_Q(QWidget);

   if (QWindow *window = q->windowHandle()) {
      window->setIcon(q->windowIcon());
   }
}

QString QWidget::windowIconText() const
{
   Q_D(const QWidget);
   return (d->extra && d->extra->topextra) ? d->extra->topextra->iconText : QString();
}

QString QWidget::windowFilePath() const
{
   Q_D(const QWidget);
   return (d->extra && d->extra->topextra) ? d->extra->topextra->filePath : QString();
}

void QWidget::setWindowFilePath(const QString &filePath)
{
   if (filePath == windowFilePath()) {
      return;
   }

   Q_D(QWidget);

   d->createTLExtra();
   d->extra->topextra->filePath = filePath;
   d->setWindowFilePath_helper(filePath);
}

void QWidgetPrivate::setWindowFilePath_helper(const QString &filePath)
{
   if (extra->topextra && extra->topextra->caption.isEmpty()) {

#ifdef Q_OS_DARWIN
      setWindowTitle_helper(QFileInfo(filePath).fileName());
#else
      Q_Q(QWidget);
      (void) filePath;

      setWindowTitle_helper(q->windowTitle());
#endif

   }

#ifdef Q_OS_DARWIN
   setWindowFilePath_sys(filePath);
#endif
}

void QWidgetPrivate::setWindowFilePath_sys(const QString &filePath)
{
   Q_Q(QWidget);

   if (! q->isWindow()) {
      return;
   }

   if (QWindow *window = q->windowHandle()) {
      window->setFilePath(filePath);
   }
}

QString QWidget::windowRole() const
{
   Q_D(const QWidget);
   return (d->extra && d->extra->topextra) ? d->extra->topextra->role : QString();
}

void QWidget::setWindowRole(const QString &role)
{
   Q_D(QWidget);

   d->createTLExtra();
   d->topData()->role = role;

   if (windowHandle()) {
      QXcbWindowFunctions::setWmWindowRole(windowHandle(), role.toUtf8());
   }
}

void QWidget::setFocusProxy(QWidget *w)
{
   Q_D(QWidget);

   if (! w && !d->extra) {
      return;
   }

   for (QWidget *fp  = w; fp; fp = fp->focusProxy()) {
      if (fp == this) {
         qWarning("QWidget::setFocusProxy() %s (%s) already in focus proxy",
               csPrintable(metaObject()->className()), csPrintable(objectName()));
         return;
      }
   }

   d->createExtra();
   d->extra->focus_proxy = w;
}

QWidget *QWidget::focusProxy() const
{
   Q_D(const QWidget);
   return d->extra ? (QWidget *)d->extra->focus_proxy : nullptr;
}

bool QWidget::hasFocus() const
{
   const QWidget *w = this;

   while (true) {
      const QWidgetPrivate *d = w->d_func();

      if (d->extra && d->extra->focus_proxy) {
         w = d->extra->focus_proxy;

      } else {
         break;

      }
   }

#ifndef QT_NO_GRAPHICSVIEW

   if (QWidget *window = w->window()) {
      QWExtra *e = window->d_func()->extra;

      if (e && e->proxyWidget && e->proxyWidget->hasFocus() && window->focusWidget() == w) {
         return true;
      }
   }

#endif

   return (QApplication::focusWidget() == w);
}

void QWidget::setFocus()
{
   setFocus(Qt::OtherFocusReason);
}

void QWidget::setFocus(Qt::FocusReason reason)
{
   if (! isEnabled()) {
      return;
   }

   QWidget *f = this;

   while (f->d_func()->extra && f->d_func()->extra->focus_proxy) {
      f = f->d_func()->extra->focus_proxy;
   }

   if (QApplication::focusWidget() == f) {
      return;
   }

#ifndef QT_NO_GRAPHICSVIEW
   QWidget *previousProxyFocus = nullptr;

   if (QWExtra *topData = window()->d_func()->extra) {
      if (topData->proxyWidget && topData->proxyWidget->hasFocus()) {
         previousProxyFocus = topData->proxyWidget->widget()->focusWidget();

         if (previousProxyFocus && previousProxyFocus->focusProxy()) {
            previousProxyFocus = previousProxyFocus->focusProxy();
         }

         if (previousProxyFocus == this && !topData->proxyWidget->d_func()->proxyIsGivingFocus) {
            return;
         }
      }
   }

#endif

#ifndef QT_NO_GRAPHICSVIEW
   // Update proxy state

   if (QWExtra *topData = window()->d_func()->extra) {
      if (topData->proxyWidget && !topData->proxyWidget->hasFocus()) {
         f->d_func()->updateFocusChild();
         topData->proxyWidget->d_func()->focusFromWidgetToProxy = 1;
         topData->proxyWidget->setFocus(reason);
         topData->proxyWidget->d_func()->focusFromWidgetToProxy = 0;
      }
   }

#endif

   if (f->isActiveWindow()) {
      QWidget *prev = QApplicationPrivate::focus_widget;

      if (prev) {
         if (reason != Qt::PopupFocusReason && reason != Qt::MenuBarFocusReason
               && prev->testAttribute(Qt::WA_InputMethodEnabled)) {
            QGuiApplication::inputMethod()->commit();
         }

         if (reason != Qt::NoFocusReason) {
            QFocusEvent focusAboutToChange(QEvent::FocusAboutToChange, reason);
            QApplication::sendEvent(prev, &focusAboutToChange);
         }
      }

      f->d_func()->updateFocusChild();
      QApplicationPrivate::setFocusWidget(f, reason);

#ifndef QT_NO_ACCESSIBILITY

# ifdef Q_OS_WIN
      // negation of the condition in setFocus_sys

      if (! (testAttribute(Qt::WA_WState_Created) && window()->windowType() != Qt::Popup && internalWinId()))
         //setFocusWidget will already post a focus event for us (that the AT client receives) on Windows
# endif

         // menus update the focus manually and this would create bogus events
         if (! (f->inherits("QMenuBar") || f->inherits("QMenu") || f->inherits("QMenuItem"))) {
            QAccessibleEvent event(f, QAccessible::Focus);
            QAccessible::updateAccessibility(&event);
         }

#endif

#ifndef QT_NO_GRAPHICSVIEW

      if (QWExtra *topData = window()->d_func()->extra) {
         if (topData->proxyWidget) {
            if (previousProxyFocus && previousProxyFocus != f) {
               // Send event to self
               QFocusEvent event(QEvent::FocusOut, reason);
               QPointer<QWidget> that = previousProxyFocus;
               QApplication::sendEvent(previousProxyFocus, &event);

               if (that) {
                  QApplication::sendEvent(that->style(), &event);
               }
            }

            if (! isHidden()) {
               // Update proxy state
               if (QWExtra *topData = window()->d_func()->extra) {
                  if (topData->proxyWidget && topData->proxyWidget->hasFocus()) {
                     topData->proxyWidget->d_func()->updateProxyInputMethodAcceptanceFromWidget();
                  }
               }

               // Send event to self
               QFocusEvent event(QEvent::FocusIn, reason);
               QPointer<QWidget> that = f;
               QApplication::sendEvent(f, &event);

               if (that) {
                  QApplication::sendEvent(that->style(), &event);
               }
            }
         }
      }

#endif

   } else {
      f->d_func()->updateFocusChild();

   }

   if (QTLWExtra *extra = f->window()->d_func()->maybeTopData()) {
      if (extra->window) {
         emit extra->window->focusObjectChanged(f);
      }
   }
}

void QWidgetPrivate::setFocus_sys()
{
   Q_Q(QWidget);

   // Embedded native widget may have taken the focus; get it back to toplevel if that is the case
   const QWidget *topLevel = q->window();

   if (topLevel->windowType() != Qt::Popup) {
      if (QWindow *nativeWindow = q->window()->windowHandle()) {
         if (nativeWindow != QGuiApplication::focusWindow()
               && q->testAttribute(Qt::WA_WState_Created)) {
            nativeWindow->requestActivate();
         }
      }
   }
}

// updates focus_child on parent widgets to point into this widget
void QWidgetPrivate::updateFocusChild()
{
   Q_Q(QWidget);

   QWidget *w = q;

   if (q->isHidden()) {
      while (w && w->isHidden()) {
         w->d_func()->focus_child = q;
         w = w->isWindow() ? nullptr : w->parentWidget();
      }

   } else {
      while (w) {
         w->d_func()->focus_child = q;
         w = w->isWindow() ? nullptr : w->parentWidget();
      }
   }
}
void QWidget::clearFocus()
{
   if (hasFocus()) {
      if (testAttribute(Qt::WA_InputMethodEnabled)) {
         QGuiApplication::inputMethod()->commit();
      }

      QFocusEvent focusAboutToChange(QEvent::FocusAboutToChange);
      QApplication::sendEvent(this, &focusAboutToChange);
   }

   QWidget *w = this;

   while (w) {
      if (w->d_func()->focus_child == this) {
         w->d_func()->focus_child = nullptr;
      }

      w = w->parentWidget();
   }

#ifndef QT_NO_GRAPHICSVIEW
   QWExtra *topData = d_func()->extra;

   if (topData && topData->proxyWidget) {
      topData->proxyWidget->clearFocus();
   }

#endif

   if (hasFocus()) {
      // Update proxy state
      QApplicationPrivate::setFocusWidget(nullptr, Qt::OtherFocusReason);

#ifndef QT_NO_ACCESSIBILITY
      QAccessibleEvent event(this, QAccessible::Focus);
      QAccessible::updateAccessibility(&event);
#endif

   }

   // Since we've unconditionally cleared the focus_child of our parents, we need
   // to report this to the rest of Qt. Note that the focus_child is not the same
   // thing as the application's focusWidget, which is why this piece of code is
   // not inside the hasFocus() block above.
   if (QTLWExtra *extra = window()->d_func()->maybeTopData()) {
      if (extra->window) {
         emit extra->window->focusObjectChanged(extra->window->focusObject());
      }
   }
}

bool QWidget::focusNextPrevChild(bool next)
{
   Q_D(QWidget);

   QWidget *p = parentWidget();
   bool isSubWindow = (windowType() == Qt::SubWindow);

   if (! isWindow() && !isSubWindow && p) {
      return p->focusNextPrevChild(next);
   }

#ifndef QT_NO_GRAPHICSVIEW

   if (d->extra && d->extra->proxyWidget) {
      return d->extra->proxyWidget->focusNextPrevChild(next);
   }

#endif

   bool wrappingOccurred = false;
   QWidget *w = QApplicationPrivate::focusNextPrevChild_helper(this, next, &wrappingOccurred);

   if (! w) {
      return false;
   }

   Qt::FocusReason reason = next ? Qt::TabFocusReason : Qt::BacktabFocusReason;

   // If we are about to wrap the focus chain, give the platform
   // implementation a chance to alter the wrapping behavior.  This is
   // especially needed when the window is embedded in a window created by another process.

   if (wrappingOccurred) {
      QWindow *window = windowHandle();

      if (window != nullptr) {
         QWindowPrivate *winp = qt_window_private(window);

         if (winp->platformWindow != nullptr) {
            QFocusEvent event(QEvent::FocusIn, reason);
            event.ignore();
            winp->platformWindow->windowEvent(&event);

            if (event.isAccepted()) {
               return true;
            }
         }
      }
   }

   w->setFocus(reason);
   return true;
}

QWidget *QWidget::focusWidget() const
{
   return const_cast<QWidget *>(d_func()->focus_child);
}

QWidget *QWidget::nextInFocusChain() const
{
   return const_cast<QWidget *>(d_func()->focus_next);
}

QWidget *QWidget::previousInFocusChain() const
{
   return const_cast<QWidget *>(d_func()->focus_prev);
}

bool QWidget::isActiveWindow() const
{
   QWidget *tlw = window();

   if (tlw == QApplication::activeWindow() || (isVisible() && (tlw->windowType() == Qt::Popup))) {
      return true;
   }

#ifndef QT_NO_GRAPHICSVIEW

   if (QWExtra *tlwExtra = tlw->d_func()->extra) {
      if (isVisible() && tlwExtra->proxyWidget) {
         return tlwExtra->proxyWidget->isActiveWindow();
      }
   }

#endif

   if (style()->styleHint(QStyle::SH_Widget_ShareActivation, nullptr, this)) {
      if (tlw->windowType() == Qt::Tool &&
            ! tlw->isModal() && (!tlw->parentWidget() || tlw->parentWidget()->isActiveWindow())) {
         return true;
      }

      QWidget *w = QApplication::activeWindow();

      while (w && tlw->windowType() == Qt::Tool && ! w->isModal() && w->parentWidget()) {
         w = w->parentWidget()->window();

         if (w == tlw) {
            return true;
         }
      }
   }

   if (QWindow *ww = QGuiApplication::focusWindow()) {

      while (ww != nullptr) {
         QWidgetWindow *qww    = dynamic_cast<QWidgetWindow *>(ww);
         QWindowContainer *qwc = qww ? dynamic_cast<QWindowContainer *>(qww->widget()) : nullptr;

         if (qwc != nullptr && qwc->topLevelWidget() == tlw) {
            return true;
         }

         ww = ww->parent();
      }
   }

   // Check if platform adaptation thinks the window is active. This is necessary for
   // example in case of ActiveQt servers that are embedded into another application.
   // Those are separate processes that are not part of the parent application Qt window/widget
   // hierarchy, so they need to rely on native methods to determine if they are part of the
   // active window.

   if (const QWindow *w = tlw->windowHandle()) {
      if (w->handle()) {
         return w->handle()->isActive();
      }
   }

   return false;
}

void QWidget::setTabOrder(QWidget *first, QWidget *second)
{
   if (! first || !second || first->focusPolicy() == Qt::NoFocus || second->focusPolicy() == Qt::NoFocus) {
      return;
   }

   if (first->window() != second->window()) {
      qWarning("QWidget::setTabOrder() Both widgets must be in the same window");
      return;
   }

   QWidget *fp = first->focusProxy();

   if (fp) {
      // If first is redirected, set first to the last child of first
      // that can take keyboard focus so that second is inserted after
      // that last child, and the focus order within first is (more
      // likely to be) preserved.

      QList<QWidget *> l = first->findChildren<QWidget *>();

      for (int i = l.size() - 1; i >= 0; --i) {
         QWidget *next = l.at(i);

         if (next->window() == fp->window()) {
            fp = next;

            if (fp->focusPolicy() != Qt::NoFocus) {
               break;
            }
         }
      }

      first = fp;
   }

   if (fp == second) {
      return;
   }

   if (QWidget *sp = second->focusProxy()) {
      second = sp;
   }

   //  QWidget *fp = first->d_func()->focus_prev;

   QWidget *fn = first->d_func()->focus_next;

   if (fn == second || first == second) {
      return;
   }

   QWidget *sp = second->d_func()->focus_prev;
   QWidget *sn = second->d_func()->focus_next;

   fn->d_func()->focus_prev = second;
   first->d_func()->focus_next = second;

   second->d_func()->focus_next = fn;
   second->d_func()->focus_prev = first;

   sp->d_func()->focus_next = sn;
   sn->d_func()->focus_prev = sp;

   Q_ASSERT(first->d_func()->focus_next->d_func()->focus_prev == first);
   Q_ASSERT(first->d_func()->focus_prev->d_func()->focus_next == first);

   Q_ASSERT(second->d_func()->focus_next->d_func()->focus_prev == second);
   Q_ASSERT(second->d_func()->focus_prev->d_func()->focus_next == second);
}

void QWidgetPrivate::reparentFocusWidgets(QWidget *oldtlw)
{
   Q_Q(QWidget);

   if (oldtlw == q->window()) {
      return;   // nothing to do
   }

   if (focus_child) {
      focus_child->clearFocus();
   }

   // separate the focus chain into new (children of myself) and old (the rest)
   QWidget *firstOld = nullptr;

   // QWidget *firstNew = q;    //invariant

   QWidget *o = nullptr;       // last in the old list
   QWidget *n = q;             // last in the new list

   bool prevWasNew = true;
   QWidget *w = focus_next;

   //Note: for efficiency, we do not maintain the list invariant inside the loop
   //we append items to the relevant list, and we optimize by not changing pointers
   //when subsequent items are going into the same list.
   while (w != q) {
      bool currentIsNew =  q->isAncestorOf(w);

      if (currentIsNew) {
         if (! prevWasNew) {
            // prev was old, append to new list
            n->d_func()->focus_next = w;
            w->d_func()->focus_prev = n;
         }

         n = w;

      } else {
         if (prevWasNew) {
            //prev was new -- append to old list, if there is one
            if (o) {
               o->d_func()->focus_next = w;
               w->d_func()->focus_prev = o;
            } else {
               // "create" the old list
               firstOld = w;
            }
         }

         o = w;
      }

      w = w->d_func()->focus_next;
      prevWasNew = currentIsNew;
   }

   //repair the old list
   if (firstOld) {
      o->d_func()->focus_next = firstOld;
      firstOld->d_func()->focus_prev = o;
   }

   if (! q->isWindow()) {
      QWidget *topLevel = q->window();

      // insert new chain into toplevel's chain
      QWidget *prev = topLevel->d_func()->focus_prev;

      topLevel->d_func()->focus_prev = n;
      prev->d_func()->focus_next = q;

      focus_prev = prev;
      n->d_func()->focus_next = topLevel;

   } else {
      // repair the new list
      n->d_func()->focus_next = q;
      focus_prev = n;
   }

}

int QWidgetPrivate::pointToRect(const QPoint &p, const QRect &r)
{
   int dx = 0;
   int dy = 0;

   if (p.x() < r.left()) {
      dx = r.left() - p.x();
   } else if (p.x() > r.right()) {
      dx = p.x() - r.right();
   }

   if (p.y() < r.top()) {
      dy = r.top() - p.y();
   } else if (p.y() > r.bottom()) {
      dy = p.y() - r.bottom();
   }

   return dx + dy;
}

QSize QWidget::frameSize() const
{
   Q_D(const QWidget);

   if (isWindow() && !(windowType() == Qt::Popup)) {
      QRect fs = d->frameStrut();

      return QSize(m_widgetData->crect.width() + fs.left() + fs.right(),
            m_widgetData->crect.height() + fs.top() + fs.bottom());
   }

   return m_widgetData->crect.size();
}

void QWidget::move(const QPoint &p)
{
   Q_D(QWidget);
   setAttribute(Qt::WA_Moved);

   if (testAttribute(Qt::WA_WState_Created)) {

      if (isWindow()) {
         d->topData()->posIncludesFrame = false;
      }

      d->setGeometry_sys(p.x() + geometry().x() - QWidget::x(),
            p.y() + geometry().y() - QWidget::y(), width(), height(), true);
      d->setDirtyOpaqueRegion();

   } else {
      if (isWindow()) {
         d->topData()->posIncludesFrame = true;
      }

      m_widgetData->crect.moveTopLeft(p); // no frame yet
      setAttribute(Qt::WA_PendingMoveEvent);
   }

   if (d->extra && d->extra->hasWindowContainer) {
      QWindowContainer::parentWasMoved(this);
   }
}

void QWidgetPrivate::fixPosIncludesFrame()
{
   Q_Q(QWidget);

   if (QTLWExtra *te = maybeTopData()) {
      if (te->posIncludesFrame) {
         // For Qt::WA_DontShowOnScreen, assume a frame of 0 (for
         // example, in QGraphicsProxyWidget)

         if (q->testAttribute(Qt::WA_DontShowOnScreen)) {
            te->posIncludesFrame = 0;

         } else {
            if (q->windowHandle()) {
               updateFrameStrut();

               if (! q->m_widgetData->fstrut_dirty) {
                  m_privateData.crect.translate(te->frameStrut.x(), te->frameStrut.y());
                  te->posIncludesFrame = 0;
               }
            }
         }
      }
   }
}
void QWidget::resize(const QSize &s)
{
   Q_D(QWidget);
   setAttribute(Qt::WA_Resized);

   if (testAttribute(Qt::WA_WState_Created)) {
      d->fixPosIncludesFrame();
      d->setGeometry_sys(geometry().x(), geometry().y(), s.width(), s.height(), false);
      d->setDirtyOpaqueRegion();

   } else {
      m_widgetData->crect.setSize(s.boundedTo(maximumSize()).expandedTo(minimumSize()));
      setAttribute(Qt::WA_PendingResizeEvent);

   }
}

void QWidget::setGeometry(const QRect &r)
{
   Q_D(QWidget);

   setAttribute(Qt::WA_Resized);
   setAttribute(Qt::WA_Moved);

   if (isWindow()) {
      d->topData()->posIncludesFrame = 0;
   }

   if (testAttribute(Qt::WA_WState_Created)) {
      d->setGeometry_sys(r.x(), r.y(), r.width(), r.height(), true);
      d->setDirtyOpaqueRegion();

   } else {

      m_widgetData->crect.setTopLeft(r.topLeft());
      m_widgetData->crect.setSize(r.size().boundedTo(maximumSize()).expandedTo(minimumSize()));

      setAttribute(Qt::WA_PendingMoveEvent);
      setAttribute(Qt::WA_PendingResizeEvent);
   }

   if (d->extra && d->extra->hasWindowContainer) {
      QWindowContainer::parentWasMoved(this);
   }
}

void QWidgetPrivate::setGeometry_sys(int x, int y, int w, int h, bool isMove)
{
   Q_Q(QWidget);

   if (extra) {                                // any size restrictions?
      w = qMin(w, extra->maxw);
      h = qMin(h, extra->maxh);
      w = qMax(w, extra->minw);
      h = qMax(h, extra->minh);
   }

   if (q->isWindow() && q->windowHandle()) {
      QPlatformIntegration *integration = QGuiApplicationPrivate::platformIntegration();

      if (! integration->hasCapability(QPlatformIntegration::NonFullScreenWindows)) {
         x = 0;
         y = 0;
         w = q->windowHandle()->width();
         h = q->windowHandle()->height();
      }
   }

   QPoint oldp = q->geometry().topLeft();
   QSize olds = q->size();
   QRect r(x, y, w, h);

   bool isResize = olds != r.size();
   isMove = oldp != r.topLeft(); //### why do we have isMove as a parameter?

   // only care about stuff that changes the geometry, or may
   // cause the window manager to change its state

   if (r.size() == olds && oldp == r.topLeft()) {
      return;
   }

   if (! m_privateData.in_set_window_state) {
      q->m_widgetData->window_state &= ~Qt::WindowMaximized;
      q->m_widgetData->window_state &= ~Qt::WindowFullScreen;

      if (q->isWindow()) {
         topData()->normalGeometry = QRect(0, 0, -1, -1);
      }
   }

   QPoint oldPos = q->pos();
   m_privateData.crect = r;

   bool needsShow = false;

   if (q->isWindow() || q->windowHandle()) {
      if (! (m_privateData.window_state & Qt::WindowFullScreen) && (w == 0 || h == 0)) {
         q->setAttribute(Qt::WA_OutsideWSRange, true);

         if (q->isVisible()) {
            hide_sys();
         }

         m_privateData.crect = QRect(x, y, w, h);

      } else if (q->testAttribute(Qt::WA_OutsideWSRange)) {
         q->setAttribute(Qt::WA_OutsideWSRange, false);
         needsShow = true;
      }
   }

   if (q->isVisible()) {
      if (! q->testAttribute(Qt::WA_DontShowOnScreen) && !q->testAttribute(Qt::WA_OutsideWSRange)) {
         if (q->windowHandle()) {
            if (q->isWindow()) {
               q->windowHandle()->setGeometry(q->geometry());
            } else {
               QPoint posInNativeParent =  q->mapTo(q->nativeParentWidget(), QPoint());
               q->windowHandle()->setGeometry(QRect(posInNativeParent, r.size()));
            }

            if (needsShow) {
               show_sys();
            }
         }

         if (! q->isWindow()) {
            if (renderToTexture) {
               QRegion updateRegion(q->geometry());
               updateRegion += QRect(oldPos, olds);
               q->parentWidget()->d_func()->invalidateBuffer(updateRegion);
            } else if (isMove && !isResize) {
               moveRect(QRect(oldPos, olds), x - oldPos.x(), y - oldPos.y());
            } else {
               invalidateBuffer_resizeHelper(oldPos, olds);
            }
         }
      }

      if (isMove) {
         QMoveEvent e(q->pos(), oldPos);
         QApplication::sendEvent(q, &e);
      }

      if (isResize) {
         QResizeEvent e(r.size(), olds);
         QApplication::sendEvent(q, &e);

         if (q->windowHandle()) {
            q->update();
         }
      }

   } else {
      // not visible

      if (isMove && q->pos() != oldPos) {
         q->setAttribute(Qt::WA_PendingMoveEvent, true);
      }

      if (isResize) {
         q->setAttribute(Qt::WA_PendingResizeEvent, true);
      }
   }
}

QByteArray QWidget::saveGeometry() const
{
   QByteArray array;

   const quint32 magicNumber = 0x1D9D0CB;
   quint16 majorVersion = 2;
   quint16 minorVersion = 0;

   const int screenNumber = QApplication::desktop()->screenNumber(this);

   QDataStream stream(&array, QIODevice::WriteOnly);
   stream << magicNumber
         << majorVersion
         << minorVersion

         << frameGeometry()
         << normalGeometry()

         << qint32(screenNumber)

         << quint8(windowState() & Qt::WindowMaximized)
         << quint8(windowState() & Qt::WindowFullScreen)
         << qint32(QApplication::desktop()->screenGeometry(screenNumber).width()); // 1.1 onwards

   return array;
}

bool QWidget::restoreGeometry(const QByteArray &geometry)
{
   if (geometry.size() < 4) {
      return false;
   }

   const quint32 magicNumber = 0x1D9D0CB;
   quint32 storedMagicNumber;

   QDataStream stream(geometry);
   stream >> storedMagicNumber;

   if (storedMagicNumber != magicNumber) {
      return false;
   }

   const quint16 currentMajorVersion = 2;
   quint16 majorVersion = 0;
   quint16 minorVersion = 0;

   stream >> majorVersion >> minorVersion;

   if (majorVersion > currentMajorVersion) {
      return false;
   }

   // Allow all minor versions

   QRect restoredFrameGeometry;
   QRect restoredNormalGeometry;

   qint32 restoredScreenNumber;
   quint8 maximized;
   quint8 fullScreen;
   qint32 restoredScreenWidth = 0;

   stream >> restoredFrameGeometry
         >> restoredNormalGeometry
         >> restoredScreenNumber
         >> maximized
         >> fullScreen;

   if (majorVersion > 1) {
      stream >> restoredScreenWidth;
   }

   const QDesktopWidget *const desktop = QApplication::desktop();

   if (restoredScreenNumber >= desktop->numScreens()) {
      restoredScreenNumber = desktop->primaryScreen();
   }

   const qreal screenWidthF = qreal(desktop->screenGeometry(restoredScreenNumber).width());

   // Sanity check bailing out when large variations of screen sizes occur due to
   // high DPI scaling or different levels of DPI awareness.
   if (restoredScreenWidth) {
      const qreal factor = qreal(restoredScreenWidth) / screenWidthF;

      if (factor < 0.8 || factor > 1.25) {
         return false;
      }

   } else {
      if (! maximized && ! fullScreen && qreal(restoredFrameGeometry.width()) / screenWidthF > 1.5) {
         return false;
      }
   }

   const int frameHeight = 20;

   if (!restoredFrameGeometry.isValid()) {
      restoredFrameGeometry = QRect(QPoint(0, 0), sizeHint());
   }

   if (!restoredNormalGeometry.isValid()) {
      restoredNormalGeometry = QRect(QPoint(0, frameHeight), sizeHint());
   }

   if (!restoredNormalGeometry.isValid()) {
      // use the widget's adjustedSize if the sizeHint() doesn't help
      restoredNormalGeometry.setSize(restoredNormalGeometry
                  .size().expandedTo(d_func()->adjustedSize()));
   }

   const QRect availableGeometry = desktop->availableGeometry(restoredScreenNumber);

   // Modify the restored geometry if we are about to restore to coordinates
   // that would make the window "lost". This happens if:
   // - The restored geometry is completely oustside the available geometry
   // - The title bar is outside the available geometry.
   // - (Mac only) The window is higher than the available geometry. It must
   //   be possible to bring the size grip on screen by moving the window.

   if (! restoredFrameGeometry.intersects(availableGeometry)) {
      restoredFrameGeometry.moveBottom(qMin(restoredFrameGeometry.bottom(), availableGeometry.bottom()));
      restoredFrameGeometry.moveLeft(qMax(restoredFrameGeometry.left(), availableGeometry.left()));
      restoredFrameGeometry.moveRight(qMin(restoredFrameGeometry.right(), availableGeometry.right()));
   }

   restoredFrameGeometry.moveTop(qMax(restoredFrameGeometry.top(), availableGeometry.top()));

   if (! restoredNormalGeometry.intersects(availableGeometry)) {
      restoredNormalGeometry.moveBottom(qMin(restoredNormalGeometry.bottom(), availableGeometry.bottom()));
      restoredNormalGeometry.moveLeft(qMax(restoredNormalGeometry.left(), availableGeometry.left()));
      restoredNormalGeometry.moveRight(qMin(restoredNormalGeometry.right(), availableGeometry.right()));
   }

   restoredNormalGeometry.moveTop(qMax(restoredNormalGeometry.top(), availableGeometry.top() + frameHeight));

   if (maximized || fullScreen) {
      // set geomerty before setting the window state to make
      // sure the window is maximized to the right screen.
      Qt::WindowStates ws = windowState();

#ifndef Q_OS_WIN
      setGeometry(restoredNormalGeometry);
#else

      if (ws & Qt::WindowFullScreen) {
         // Full screen is not a real window state on Windows.
         move(availableGeometry.topLeft());

      } else if (ws & Qt::WindowMaximized) {
         // Setting a geometry on an already maximized window causes this to be
         // restored into a broken, half-maximized state, non-resizable state (QTBUG-4397).
         // Move the window in normal state if needed.
         if (restoredScreenNumber != desktop->screenNumber(this)) {
            setWindowState(Qt::WindowNoState);
            setGeometry(restoredNormalGeometry);
         }

      } else {
         setGeometry(restoredNormalGeometry);
      }

#endif

      if (maximized) {
         ws |= Qt::WindowMaximized;
      }

      if (fullScreen) {
         ws |= Qt::WindowFullScreen;
      }

      setWindowState(ws);
      d_func()->topData()->normalGeometry = restoredNormalGeometry;

   } else {
      QPoint offset;

      setWindowState(windowState() & ~(Qt::WindowMaximized | Qt::WindowFullScreen));
      move(restoredFrameGeometry.topLeft() + offset);
      resize(restoredNormalGeometry.size());
   }

   return true;
}

void QWidget::setContentsMargins(int left, int top, int right, int bottom)
{
   Q_D(QWidget);

   if (left == d->leftmargin && top == d->topmargin && right == d->rightmargin && bottom == d->bottommargin) {
      return;
   }

   d->leftmargin   = left;
   d->topmargin    = top;
   d->rightmargin  = right;
   d->bottommargin = bottom;

   if (QLayout *l = d->layout) {
      l->update();   //force activate; will do updateGeometry
   } else {
      updateGeometry();
   }

   // ### TODO: consider removing
   if (isVisible()) {
      update();
      QResizeEvent e(m_widgetData->crect.size(), m_widgetData->crect.size());
      QApplication::sendEvent(this, &e);

   } else {
      setAttribute(Qt::WA_PendingResizeEvent, true);
   }

   QEvent e(QEvent::ContentsRectChange);
   QApplication::sendEvent(this, &e);
}

void QWidget::setContentsMargins(const QMargins &margins)
{
   setContentsMargins(margins.left(), margins.top(), margins.right(), margins.bottom());
}

void QWidget::getContentsMargins(int *left, int *top, int *right, int *bottom) const
{
   Q_D(const QWidget);

   if (left) {
      *left = d->leftmargin;
   }

   if (top) {
      *top = d->topmargin;
   }

   if (right) {
      *right = d->rightmargin;
   }

   if (bottom) {
      *bottom = d->bottommargin;
   }
}

QMargins QWidget::contentsMargins() const
{
   Q_D(const QWidget);
   return QMargins(d->leftmargin, d->topmargin, d->rightmargin, d->bottommargin);
}

QRect QWidget::contentsRect() const
{
   Q_D(const QWidget);

   return QRect(QPoint(d->leftmargin, d->topmargin),
         QPoint(m_widgetData->crect.width() - 1 - d->rightmargin,
         m_widgetData->crect.height() - 1 - d->bottommargin));
}

Qt::ContextMenuPolicy QWidget::contextMenuPolicy() const
{
   return (Qt::ContextMenuPolicy)m_widgetData->context_menu_policy;
}

void QWidget::setContextMenuPolicy(Qt::ContextMenuPolicy policy)
{
   m_widgetData->context_menu_policy = (uint) policy;
}

Qt::FocusPolicy QWidget::focusPolicy() const
{
   return (Qt::FocusPolicy)m_widgetData->focus_policy;
}

void QWidget::setFocusPolicy(Qt::FocusPolicy policy)
{
   Q_D(QWidget);

   m_widgetData->focus_policy = (uint)policy;

   if (d->extra && d->extra->focus_proxy) {
      d->extra->focus_proxy->setFocusPolicy(policy);
   }
}

void QWidget::setUpdatesEnabled(bool enable)
{
   Q_D(QWidget);

   setAttribute(Qt::WA_ForceUpdatesDisabled, ! enable);
   d->setUpdatesEnabled_helper(enable);
}

void QWidget::show()
{
   Qt::WindowState defaultState = QGuiApplicationPrivate::platformIntegration()->defaultWindowState(m_widgetData->m_flags);

   if (defaultState == Qt::WindowFullScreen) {
      showFullScreen();
   } else if (defaultState == Qt::WindowMaximized) {
      showMaximized();
   } else {
      setVisible(true);   // FIXME: Why not showNormal(), like QWindow::show()?
   }
}
void QWidgetPrivate::show_recursive()
{
   Q_Q(QWidget);

   if (! q->testAttribute(Qt::WA_WState_Created)) {
      createRecursively();
   }

   q->ensurePolished();

   if (! q->isWindow() && q->parentWidget()->d_func()->layout && ! q->parentWidget()->m_widgetData->in_show) {
      q->parentWidget()->d_func()->layout->activate();
   }

   // activate our layout before we and our children become visible
   if (layout) {
      layout->activate();
   }

   show_helper();
}

void QWidgetPrivate::sendPendingMoveAndResizeEvents(bool recursive, bool disableUpdates)
{
   Q_Q(QWidget);

   disableUpdates = disableUpdates && q->updatesEnabled();

   if (disableUpdates) {
      q->setAttribute(Qt::WA_UpdatesDisabled);
   }

   if (q->testAttribute(Qt::WA_PendingMoveEvent)) {
      QMoveEvent e(m_privateData.crect.topLeft(), m_privateData.crect.topLeft());
      QApplication::sendEvent(q, &e);
      q->setAttribute(Qt::WA_PendingMoveEvent, false);
   }

   if (q->testAttribute(Qt::WA_PendingResizeEvent)) {
      QResizeEvent e(m_privateData.crect.size(), QSize());
      QApplication::sendEvent(q, &e);
      q->setAttribute(Qt::WA_PendingResizeEvent, false);
   }

   if (disableUpdates) {
      q->setAttribute(Qt::WA_UpdatesDisabled, false);
   }

   if (!recursive) {
      return;
   }

   for (int i = 0; i < q->children().size(); ++i) {
      if (QWidget *child = dynamic_cast<QWidget *>(q->children().at(i))) {
         child->d_func()->sendPendingMoveAndResizeEvents(recursive, disableUpdates);
      }
   }
}

void QWidgetPrivate::activateChildLayoutsRecursively()
{
   Q_Q(QWidget);

   sendPendingMoveAndResizeEvents(false, true);

   for (int i = 0; i < q->children().size(); ++i) {
      QWidget *child = dynamic_cast<QWidget *>(q->children().at(i));

      if (child == nullptr || child->isHidden() || child->isWindow()) {
         continue;
      }

      child->ensurePolished();

      // Activate child's layout
      QWidgetPrivate *childPrivate = child->d_func();

      if (childPrivate->layout) {
         childPrivate->layout->activate();
      }

      // Pretend we're visible.
      const bool wasVisible = child->isVisible();

      if (!wasVisible) {
         child->setAttribute(Qt::WA_WState_Visible);
      }

      // Do the same for all my children.
      childPrivate->activateChildLayoutsRecursively();

      // We're not cheating anymore.
      if (! wasVisible) {
         child->setAttribute(Qt::WA_WState_Visible, false);
      }
   }
}

void QWidgetPrivate::show_helper()
{
   Q_Q(QWidget);

   m_privateData.in_show = true;    // optimization

   // make sure we receive pending move and resize events
   sendPendingMoveAndResizeEvents();

   // become visible before showing all children
   q->setAttribute(Qt::WA_WState_Visible);

   // finally show all children recursively
   showChildren(false);

   const bool isWindow = q->isWindow();

#ifndef QT_NO_GRAPHICSVIEW
   bool isEmbedded = isWindow && q->graphicsProxyWidget() != nullptr;
#else
   bool isEmbedded = false;
#endif
   // popup handling: new popups and tools need to be raised, and
   // existing popups must be closed. Also propagate the current
   // windows's KeyboardFocusChange status.

   if (isWindow && !isEmbedded) {
      if ((q->windowType() == Qt::Tool) || (q->windowType() == Qt::Popup) || q->windowType() == Qt::ToolTip) {
         q->raise();

         if (q->parentWidget() && q->parentWidget()->window()->testAttribute(Qt::WA_KeyboardFocusChange)) {
            q->setAttribute(Qt::WA_KeyboardFocusChange);
         }

      } else {
         while (QApplication::activePopupWidget()) {
            if (!QApplication::activePopupWidget()->close()) {
               break;
            }
         }
      }
   }

   // Automatic embedding of child windows of widgets already embedded into
   // QGraphicsProxyWidget when they are shown the first time.

#ifndef QT_NO_GRAPHICSVIEW

   if (q->isWindow()) {

      if (! isEmbedded && !bypassGraphicsProxyWidget(q)) {
         QGraphicsProxyWidget *ancestorProxy = nearestGraphicsProxyWidget(q->parentWidget());

         if (ancestorProxy) {
            isEmbedded = true;
            ancestorProxy->d_func()->embedSubWindow(q);
         }
      }
   }

#endif

   // send the show event before showing the window
   QShowEvent showEvent;
   QApplication::sendEvent(q, &showEvent);

   show_sys();

   if (! isEmbedded && q->windowType() == Qt::Popup) {
      qApp->d_func()->openPopup(q);
   }

#ifndef QT_NO_ACCESSIBILITY

   if (q->windowType() != Qt::ToolTip) {   // Tooltips are read aloud twice in MS narrator.
      QAccessibleEvent event(q, QAccessible::ObjectShow);
      QAccessible::updateAccessibility(&event);
   }

#endif

   if (QApplicationPrivate::hidden_focus_widget == q) {
      QApplicationPrivate::hidden_focus_widget = nullptr;
      q->setFocus(Qt::OtherFocusReason);
   }

   // Process events when showing a Qt::SplashScreen widget before the event loop
   // is spinnning; otherwise it might not show up on particular platforms.
   // This makes QSplashScreen behave the same on all platforms.
   if (!qApp->d_func()->in_exec && q->windowType() == Qt::SplashScreen) {
      QApplication::processEvents();
   }

   m_privateData.in_show = false;  // reset qws optimization
}

void QWidgetPrivate::show_sys()
{
   Q_Q(QWidget);

   QWindow *window = q->windowHandle();

   if (q->testAttribute(Qt::WA_DontShowOnScreen)) {
      invalidateBuffer(q->rect());
      q->setAttribute(Qt::WA_Mapped);

      // add our window the modal window list (native dialogs)
      if (window && q->isWindow()

#ifndef QT_NO_GRAPHICSVIEW
            && (!extra || !extra->proxyWidget)
#endif
            && q->windowModality() != Qt::NonModal) {

         QGuiApplicationPrivate::showModalWindow(window);
      }

      return;
   }

   if (renderToTexture && !q->isWindow()) {
      QApplication::postEvent(q->parentWidget(), new QUpdateLaterEvent(q->geometry()));
   } else {
      QApplication::postEvent(q, new QUpdateLaterEvent(q->rect()));
   }

   if ((! q->isWindow() && !q->testAttribute(Qt::WA_NativeWindow))
         || q->testAttribute(Qt::WA_OutsideWSRange)) {
      return;
   }

   if (window) {
      if (q->isWindow()) {
         fixPosIncludesFrame();
      }

      QRect geomRect = q->geometry();

      if (!q->isWindow()) {
         QPoint topLeftOfWindow = q->mapTo(q->nativeParentWidget(), QPoint());
         geomRect.moveTopLeft(topLeftOfWindow);
      }

      const QRect windowRect = window->geometry();

      if (windowRect != geomRect) {
         if (q->testAttribute(Qt::WA_Moved)
               || !QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::WindowManagement)) {
            window->setGeometry(geomRect);
         } else {
            window->resize(geomRect.size());
         }
      }

#ifndef QT_NO_CURSOR
      cs_internal_set_cursor(q, false); // Needed in case cursor was set before show
#endif

      invalidateBuffer(q->rect());
      window->setVisible(true);

      // Was the window moved by the Window system or QPlatformWindow::initialGeometry() ?
      if (window->isTopLevel()) {
         const QPoint crectTopLeft = q->m_widgetData->crect.topLeft();
         const QPoint windowTopLeft = window->geometry().topLeft();

         if (crectTopLeft == QPoint(0, 0) && windowTopLeft != crectTopLeft) {
            q->m_widgetData->crect.moveTopLeft(windowTopLeft);
         }
      }
   }
}

void QWidget::hide()
{
   setVisible(false);
}

void QWidgetPrivate::hide_helper()
{
   Q_Q(QWidget);

   bool isEmbedded = false;

#if ! defined QT_NO_GRAPHICSVIEW
   isEmbedded = q->isWindow() && !bypassGraphicsProxyWidget(q) && nearestGraphicsProxyWidget(q->parentWidget()) != nullptr;
#else
   (void) isEmbedded;
#endif

   if (! isEmbedded && (q->windowType() == Qt::Popup)) {
      qApp->d_func()->closePopup(q);
   }

   q->setAttribute(Qt::WA_Mapped, false);
   hide_sys();

   bool wasVisible = q->testAttribute(Qt::WA_WState_Visible);

   if (wasVisible) {
      q->setAttribute(Qt::WA_WState_Visible, false);
   }

   QHideEvent hideEvent;
   QApplication::sendEvent(q, &hideEvent);
   hideChildren(false);

   // next bit tries to move the focus if the focus widget is now hidden
   if (wasVisible) {

      qApp->d_func()->sendSyntheticEnterLeave(q);

      QWidget *fw = QApplication::focusWidget();

      while (fw &&  !fw->isWindow()) {
         if (fw == q) {
            q->focusNextPrevChild(true);
            break;
         }

         fw = fw->parentWidget();
      }
   }

   if (QWidgetBackingStore *bs = maybeBackingStore()) {
      bs->removeDirtyWidget(q);
   }

#ifndef QT_NO_ACCESSIBILITY

   if (wasVisible) {
      QAccessibleEvent event(q, QAccessible::ObjectHide);
      QAccessible::updateAccessibility(&event);
   }

#endif
}

void QWidgetPrivate::hide_sys()
{
   Q_Q(QWidget);

   QWindow *window = q->windowHandle();

   if (q->testAttribute(Qt::WA_DontShowOnScreen)) {
      q->setAttribute(Qt::WA_Mapped, false);

      // remove our window from the modal window list (native dialogs)
      if (window && q->isWindow()

#ifndef QT_NO_GRAPHICSVIEW
            && (! extra || !extra->proxyWidget)
#endif
            && q->windowModality() != Qt::NonModal) {

         QGuiApplicationPrivate::hideModalWindow(window);
      }

      // do not return here, if window non-zero then hide it
   }

   deactivateWidgetCleanup();

   if (! q->isWindow()) {
      QWidget *p = q->parentWidget();

      if (p && p->isVisible()) {
         if (renderToTexture) {
            p->d_func()->invalidateBuffer(q->geometry());
         } else {
            invalidateBuffer(q->rect());
         }
      }

   } else {
      invalidateBuffer(q->rect());
   }

   if (window) {
      window->setVisible(false);
   }
}
void QWidget::setVisible(bool visible)
{
   if (visible) {
      // show

      if (testAttribute(Qt::WA_WState_ExplicitShowHide) && ! testAttribute(Qt::WA_WState_Hidden)) {
         return;
      }

      Q_D(QWidget);

      // Designer uses a trick to make grabWidget work without showing
      if (! isWindow() && parentWidget() && parentWidget()->isVisible()
            && ! parentWidget()->testAttribute(Qt::WA_WState_Created)) {

         parentWidget()->window()->d_func()->createRecursively();
      }

      //we have to at least create toplevels before applyX11SpecificCommandLineArguments
      //but not children of non-visible parents

      QWidget *pw = parentWidget();

      if (! testAttribute(Qt::WA_WState_Created)
            && (isWindow() || pw->testAttribute(Qt::WA_WState_Created))) {
         create();
      }

      bool wasResized = testAttribute(Qt::WA_Resized);
      Qt::WindowStates initialWindowState = windowState();

      // polish if necessary
      ensurePolished();

      // remember that show was called explicitly
      setAttribute(Qt::WA_WState_ExplicitShowHide);

      // whether we need to inform the parent widget immediately
      bool needUpdateGeometry = !isWindow() && testAttribute(Qt::WA_WState_Hidden);

      // we are no longer hidden
      setAttribute(Qt::WA_WState_Hidden, false);

      if (needUpdateGeometry) {
         d->updateGeometry_helper(true);
      }

      // activate our layout before we and our children become visible
      if (d->layout) {
         d->layout->activate();
      }

      if (! isWindow()) {
         QWidget *parent = parentWidget();

         while (parent && parent->isVisible() && parent->d_func()->layout && ! parent->m_widgetData->in_show) {
            parent->d_func()->layout->activate();

            if (parent->isWindow()) {
               break;
            }

            parent = parent->parentWidget();
         }

         if (parent) {
            parent->d_func()->setDirtyOpaqueRegion();
         }
      }

      // adjust size if necessary
      if (! wasResized && (isWindow() || !parentWidget()->d_func()->layout))  {
         if (isWindow()) {
            adjustSize();

            if (windowState() != initialWindowState) {
               setWindowState(initialWindowState);
            }
         } else {
            adjustSize();
         }

         setAttribute(Qt::WA_Resized, false);
      }

      setAttribute(Qt::WA_KeyboardFocusChange, false);

      if (isWindow() || parentWidget()->isVisible()) {
         d->show_helper();
         qApp->d_func()->sendSyntheticEnterLeave(this);
      }

      QEvent showToParentEvent(QEvent::ShowToParent);
      QApplication::sendEvent(this, &showToParentEvent);

   } else {
      // hide

      if (testAttribute(Qt::WA_WState_ExplicitShowHide) && testAttribute(Qt::WA_WState_Hidden)) {
         return;
      }

      if (QApplicationPrivate::hidden_focus_widget == this) {
         QApplicationPrivate::hidden_focus_widget = nullptr;
      }

      Q_D(QWidget);

      // hw: The test on getOpaqueRegion() needs to be more intelligent
      // currently it doesn't work if the widget is hidden (the region will
      // be clipped). The real check should be testing the cached region
      // (and dirty flag) directly.

      if (! isWindow() && parentWidget()) {
         parentWidget()->d_func()->setDirtyOpaqueRegion();
      }

      setAttribute(Qt::WA_WState_Hidden);
      setAttribute(Qt::WA_WState_ExplicitShowHide);

      if (testAttribute(Qt::WA_WState_Created)) {
         d->hide_helper();
      }

      // invalidate layout similar to updateGeometry()
      if (! isWindow() && parentWidget()) {
         if (parentWidget()->d_func()->layout) {
            parentWidget()->d_func()->layout->invalidate();
         } else if (parentWidget()->isVisible()) {
            QApplication::postEvent(parentWidget(), new QEvent(QEvent::LayoutRequest));
         }
      }

      QEvent hideToParentEvent(QEvent::HideToParent);
      QApplication::sendEvent(this, &hideToParentEvent);
   }
}

void QWidget::setHidden(bool hidden)
{
   setVisible(!hidden);
}

void QWidgetPrivate::_q_showIfNotHidden()
{
   Q_Q(QWidget);

   if ( ! (q->isHidden() && q->testAttribute(Qt::WA_WState_ExplicitShowHide)) ) {
      q->setVisible(true);
   }
}

void QWidgetPrivate::showChildren(bool spontaneous)
{
   Q_Q(QWidget);

   QList<QObject *> childList = q->children();

   for (int i = 0; i < childList.size(); ++i) {
      QWidget *widget = dynamic_cast<QWidget *>(childList.at(i));

      if (widget == nullptr || widget->isWindow() || widget->testAttribute(Qt::WA_WState_Hidden)) {
         continue;
      }

      if (spontaneous) {
         widget->setAttribute(Qt::WA_Mapped);
         widget->d_func()->showChildren(true);
         QShowEvent e;
         QApplication::sendSpontaneousEvent(widget, &e);

      } else {
         if (widget->testAttribute(Qt::WA_WState_ExplicitShowHide)) {
            widget->d_func()->show_recursive();
         } else {
            widget->show();
         }
      }
   }
}

void QWidgetPrivate::hideChildren(bool spontaneous)
{
   Q_Q(QWidget);

   QList<QObject *> childList = q->children();

   for (int i = 0; i < childList.size(); ++i) {
      QWidget *widget = dynamic_cast<QWidget *>(childList.at(i));

      if (widget == nullptr || widget->isWindow() || widget->testAttribute(Qt::WA_WState_Hidden)) {
         continue;
      }

      if (spontaneous) {
         widget->setAttribute(Qt::WA_Mapped, false);
      } else {
         widget->setAttribute(Qt::WA_WState_Visible, false);
      }

      widget->d_func()->hideChildren(spontaneous);
      QHideEvent e;

      if (spontaneous) {
         QApplication::sendSpontaneousEvent(widget, &e);

      } else {
         QApplication::sendEvent(widget, &e);

         if (widget->internalWinId() && widget->testAttribute(Qt::WA_DontCreateNativeAncestors)) {
            // hide_sys() on an ancestor won't have any affect on this
            // widget, so it needs an explicit hide_sys() of its own
            widget->d_func()->hide_sys();
         }
      }

      qApp->d_func()->sendSyntheticEnterLeave(widget);

#ifndef QT_NO_ACCESSIBILITY

      if (! spontaneous) {
         QAccessibleEvent event(widget, QAccessible::ObjectHide);
         QAccessible::updateAccessibility(&event);
      }

#endif
   }
}

bool QWidgetPrivate::close_helper(CloseMode mode)
{
   if (m_privateData.is_closing) {
      return true;
   }

   Q_Q(QWidget);
   m_privateData.is_closing = 1;

   QPointer<QWidget> that = q;
   QPointer<QWidget> parentWidget = q->parentWidget();

   bool quitOnClose = q->testAttribute(Qt::WA_QuitOnClose);

   if (mode != CloseNoEvent) {
      QCloseEvent e;

      if (mode == CloseWithSpontaneousEvent) {
         QApplication::sendSpontaneousEvent(q, &e);
      } else {
         QApplication::sendEvent(q, &e);
      }

      if (!that.isNull() && !e.isAccepted()) {
         m_privateData.is_closing = 0;
         return false;
      }
   }

   if (! that.isNull() && ! q->isHidden()) {
      q->hide();
   }

   // Attempt to close the application only if this has WA_QuitOnClose set and a non-visible parent
   quitOnClose = quitOnClose && (parentWidget.isNull() || ! parentWidget->isVisible());

   if (quitOnClose) {
      /* if there is no non-withdrawn primary window left (except
         the ones without QuitOnClose), we emit the lastWindowClosed
         signal */

      QWidgetList list = QApplication::topLevelWidgets();
      bool lastWindowClosed = true;

      for (int i = 0; i < list.size(); ++i) {
         QWidget *w = list.at(i);

         if (! w->isVisible() || w->parentWidget() || !w->testAttribute(Qt::WA_QuitOnClose)) {
            continue;
         }

         lastWindowClosed = false;
         break;
      }

      if (lastWindowClosed) {
         QGuiApplicationPrivate::emitLastWindowClosed();

         qApp->cs_internal_maybeQuit();
      }
   }

   if (! that.isNull()) {
      m_privateData.is_closing = 0;

      if (q->testAttribute(Qt::WA_DeleteOnClose)) {
         q->setAttribute(Qt::WA_DeleteOnClose, false);
         q->deleteLater();
      }
   }

   return true;
}

bool QWidget::close()
{
   return d_func()->close_helper(QWidgetPrivate::CloseWithEvent);
}

bool QWidget::isVisibleTo(const QWidget *ancestor) const
{
   if (!ancestor) {
      return isVisible();
   }

   const QWidget *w = this;

   while (! w->isHidden() && ! w->isWindow() && w->parentWidget() && w->parentWidget() != ancestor) {
      w = w->parentWidget();
   }

   return !w->isHidden();
}

QRegion QWidget::visibleRegion() const
{
   Q_D(const QWidget);

   QRect clipRect = d->clipRect();

   if (clipRect.isEmpty()) {
      return QRegion();
   }

   QRegion r(clipRect);
   d->subtractOpaqueChildren(r, clipRect);
   d->subtractOpaqueSiblings(r);

   return r;
}

QSize QWidgetPrivate::adjustedSize() const
{
   Q_Q(const QWidget);

   QSize s = q->sizeHint();

   if (q->isWindow()) {
      Qt::Orientations exp;

      if (layout) {
         if (layout->hasHeightForWidth()) {
            s.setHeight(layout->totalHeightForWidth(s.width()));
         }

         exp = layout->expandingDirections();

      } else {
         if (q->sizePolicy().hasHeightForWidth()) {
            s.setHeight(q->heightForWidth(s.width()));
         }

         exp = q->sizePolicy().expandingDirections();
      }

      if (exp & Qt::Horizontal) {
         s.setWidth(qMax(s.width(), 200));
      }

      if (exp & Qt::Vertical) {
         s.setHeight(qMax(s.height(), 100));
      }

      QRect screen = QApplication::desktop()->screenGeometry(q->pos());

      s.setWidth(qMin(s.width(), screen.width() * 2 / 3));
      s.setHeight(qMin(s.height(), screen.height() * 2 / 3));

      if (QTLWExtra *extra = maybeTopData()) {
         extra->sizeAdjusted = true;
      }
   }

   if (! s.isValid()) {
      QRect r = q->childrenRect(); // get children rectangle

      if (r.isNull()) {
         return s;
      }

      s = r.size() + QSize(2 * r.x(), 2 * r.y());
   }

   return s;
}

void QWidget::adjustSize()
{
   Q_D(QWidget);
   ensurePolished();
   QSize s = d->adjustedSize();

   if (d->layout) {
      d->layout->activate();
   }

   if (s.isValid()) {
      resize(s);
   }
}

QSize QWidget::sizeHint() const
{
   Q_D(const QWidget);

   if (d->layout) {
      return d->layout->totalSizeHint();
   }

   return QSize(-1, -1);
}

QSize QWidget::minimumSizeHint() const
{
   Q_D(const QWidget);

   if (d->layout) {
      return d->layout->totalMinimumSize();
   }

   return QSize(-1, -1);
}

bool QWidget::isAncestorOf(const QWidget *child) const
{
   while (child) {
      if (child == this) {
         return true;
      }

      if (child->isWindow()) {
         return false;
      }

      child = child->parentWidget();
   }

   return false;
}

bool QWidget::event(QEvent *event)
{
   Q_D(QWidget);

   // ignore mouse events when disabled
   if (!isEnabled()) {
      switch (event->type()) {

         case QEvent::TabletPress:
         case QEvent::TabletRelease:
         case QEvent::TabletMove:
         case QEvent::MouseButtonPress:
         case QEvent::MouseButtonRelease:
         case QEvent::MouseButtonDblClick:
         case QEvent::MouseMove:
         case QEvent::TouchBegin:
         case QEvent::TouchUpdate:
         case QEvent::TouchEnd:
         case QEvent::TouchCancel:
         case QEvent::ContextMenu:
         case QEvent::KeyPress:
         case QEvent::KeyRelease:
#ifndef QT_NO_WHEELEVENT
         case QEvent::Wheel:
#endif
            return false;

         default:
            break;
      }
   }

   switch (event->type()) {
      case QEvent::MouseMove:
         mouseMoveEvent((QMouseEvent *)event);
         break;

      case QEvent::MouseButtonPress:
         // Do not reset input context here. Whether reset or not is
         // a responsibility of input method. reset() will be
         // called by mouseHandler() of input method if necessary
         // via mousePressEvent() of text widgets.

         mousePressEvent((QMouseEvent *)event);
         break;

      case QEvent::MouseButtonRelease:
         mouseReleaseEvent((QMouseEvent *)event);
         break;

      case QEvent::MouseButtonDblClick:
         mouseDoubleClickEvent((QMouseEvent *)event);
         break;

#ifndef QT_NO_WHEELEVENT

      case QEvent::Wheel:
         wheelEvent((QWheelEvent *)event);
         break;
#endif

#ifndef QT_NO_TABLETEVENT

      case QEvent::TabletMove:
      case QEvent::TabletPress:
      case QEvent::TabletRelease:
         tabletEvent((QTabletEvent *)event);
         break;
#endif

      case QEvent::KeyPress: {
         QKeyEvent *k = (QKeyEvent *)event;
         bool res = false;

         if (! (k->modifiers() & (Qt::ControlModifier | Qt::AltModifier))) {
            // ### Add MetaModifier?

            if (k->key() == Qt::Key_Backtab || (k->key() == Qt::Key_Tab && (k->modifiers() & Qt::ShiftModifier))) {
               res = focusNextPrevChild(false);

            } else if (k->key() == Qt::Key_Tab) {
               res = focusNextPrevChild(true);
            }

            if (res) {
               break;
            }
         }

         keyPressEvent(k);
#ifdef QT_KEYPAD_NAVIGATION

         if (! k->isAccepted() && QApplication::keypadNavigationEnabled() &&
               ! (k->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::ShiftModifier))) {

            if (QApplication::navigationMode() == Qt::NavigationModeKeypadTabOrder) {
               if (k->key() == Qt::Key_Up) {
                  res = focusNextPrevChild(false);
               } else if (k->key() == Qt::Key_Down) {
                  res = focusNextPrevChild(true);
               }

            } else if (QApplication::navigationMode() == Qt::NavigationModeKeypadDirectional) {
               if (k->key() == Qt::Key_Up) {
                  res = QWidgetPrivate::navigateToDirection(QWidgetPrivate::DirectionNorth);
               } else if (k->key() == Qt::Key_Right) {
                  res = QWidgetPrivate::navigateToDirection(QWidgetPrivate::DirectionEast);
               } else if (k->key() == Qt::Key_Down) {
                  res = QWidgetPrivate::navigateToDirection(QWidgetPrivate::DirectionSouth);
               } else if (k->key() == Qt::Key_Left) {
                  res = QWidgetPrivate::navigateToDirection(QWidgetPrivate::DirectionWest);
               }
            }

            if (res) {
               k->accept();
               break;
            }
         }

#endif

#ifndef QT_NO_WHATSTHIS

         if (! k->isAccepted() && k->modifiers() & Qt::ShiftModifier && k->key() == Qt::Key_F1
               && d->whatsThis.size()) {
            QWhatsThis::showText(mapToGlobal(inputMethodQuery(Qt::ImCursorRectangle).toRect().center()), d->whatsThis, this);
            k->accept();
         }

#endif
      }
      break;

      case QEvent::KeyRelease:
         keyReleaseEvent((QKeyEvent *)event);
         [[fallthrough]];

      case QEvent::ShortcutOverride:
         break;

      case QEvent::InputMethod:
         inputMethodEvent((QInputMethodEvent *) event);
         break;

      case QEvent::InputMethodQuery:
         if (testAttribute(Qt::WA_InputMethodEnabled)) {
            QInputMethodQueryEvent *query = static_cast<QInputMethodQueryEvent *>(event);
            Qt::InputMethodQueries queries = query->queries();

            for (uint i = 0; i < 32; ++i) {
               Qt::InputMethodQuery q = (Qt::InputMethodQuery)(int)(queries & (1 << i));

               if (q) {
                  QVariant v = inputMethodQuery(q);

                  if (q == Qt::ImEnabled && !v.isValid() && isEnabled()) {
                     v = QVariant(true);   // special case for backward compatibility
                  }

                  query->setValue(q, v);
               }
            }

            query->accept();
         }

         break;

      case QEvent::PolishRequest:
         ensurePolished();
         break;

      case QEvent::Polish: {
         style()->polish(this);
         setAttribute(Qt::WA_WState_Polished);

         if (! QApplication::font(this).isCopyOf(QApplication::font())) {
            d->resolveFont();
         }

         if (! QApplication::palette(this).isCopyOf(QApplication::palette())) {
            d->resolvePalette();
         }
      }
      break;

      case QEvent::ApplicationWindowIconChange:
         if (isWindow() && !testAttribute(Qt::WA_SetWindowIcon)) {
            d->setWindowIcon_sys();
            d->setWindowIcon_helper();
         }

         break;

      case QEvent::FocusIn:
         focusInEvent((QFocusEvent *)event);
         d->updateWidgetTransform(event);
         break;

      case QEvent::FocusOut:
         focusOutEvent((QFocusEvent *)event);
         break;

      case QEvent::Enter:

#ifndef QT_NO_STATUSTIP
         if (d->statusTip.size()) {
            QStatusTipEvent tip(d->statusTip);
            QApplication::sendEvent(const_cast<QWidget *>(this), &tip);
         }

#endif
         enterEvent(event);
         break;

      case QEvent::Leave:
#ifndef QT_NO_STATUSTIP
         if (d->statusTip.size()) {
            QString empty;
            QStatusTipEvent tip(empty);
            QApplication::sendEvent(const_cast<QWidget *>(this), &tip);
         }

#endif
         leaveEvent(event);
         break;

      case QEvent::HoverEnter:
      case QEvent::HoverLeave:
         update();
         break;

      case QEvent::Paint:
         // At this point the event has to be delivered, regardless whether the widget
         // isVisible() or not because it already went through the filters

         paintEvent((QPaintEvent *)event);
         break;

      case QEvent::Move:
         moveEvent((QMoveEvent *)event);
         d->updateWidgetTransform(event);
         break;

      case QEvent::Resize:
         resizeEvent((QResizeEvent *)event);
         d->updateWidgetTransform(event);
         break;

      case QEvent::Close:
         closeEvent((QCloseEvent *)event);
         break;

#ifndef QT_NO_CONTEXTMENU

      case QEvent::ContextMenu:
         switch (m_widgetData->context_menu_policy) {
            case Qt::PreventContextMenu:
               break;

            case Qt::DefaultContextMenu:
               contextMenuEvent(static_cast<QContextMenuEvent *>(event));
               break;

            case Qt::CustomContextMenu:
               emit customContextMenuRequested(static_cast<QContextMenuEvent *>(event)->pos());
               break;

#ifndef QT_NO_MENU

            case Qt::ActionsContextMenu:
               if (d->actions.count()) {
                  QMenu::exec(d->actions, static_cast<QContextMenuEvent *>(event)->globalPos(), nullptr, this);
                  break;
               }

               [[fallthrough]];
#endif

            default:
               event->ignore();
               break;
         }

         break;

#endif // QT_NO_CONTEXTMENU

#ifndef QT_NO_DRAGANDDROP

      case QEvent::Drop:
         dropEvent((QDropEvent *) event);
         break;

      case QEvent::DragEnter:
         dragEnterEvent((QDragEnterEvent *) event);
         break;

      case QEvent::DragMove:
         dragMoveEvent((QDragMoveEvent *) event);
         break;

      case QEvent::DragLeave:
         dragLeaveEvent((QDragLeaveEvent *) event);
         break;
#endif

      case QEvent::Show:
         showEvent((QShowEvent *) event);
         break;

      case QEvent::Hide:
         hideEvent((QHideEvent *) event);
         break;

      case QEvent::ShowWindowRequest:
         if (!isHidden()) {
            d->show_sys();
         }

         break;

      case QEvent::ApplicationFontChange:
         d->resolveFont();
         break;

      case QEvent::ApplicationPaletteChange:
         if (!(windowType() == Qt::Desktop)) {
            d->resolvePalette();
         }

         break;

      case QEvent::ToolBarChange:
      case QEvent::ActivationChange:
      case QEvent::EnabledChange:
      case QEvent::FontChange:
      case QEvent::StyleChange:
      case QEvent::PaletteChange:
      case QEvent::WindowTitleChange:
      case QEvent::IconTextChange:
      case QEvent::ModifiedChange:
      case QEvent::MouseTrackingChange:
      case QEvent::ParentChange:

      case QEvent::LocaleChange:
      case QEvent::MacSizeChange:
      case QEvent::ContentsRectChange:
      case QEvent::ThemeChange:
      case QEvent::ReadOnlyChange:
         changeEvent(event);
         break;

      case QEvent::WindowStateChange: {
         const bool wasMinimized = static_cast<const QWindowStateChangeEvent *>(event)->oldState() & Qt::WindowMinimized;

         if (wasMinimized != isMinimized()) {
            QWidget *widget = const_cast<QWidget *>(this);

            if (wasMinimized) {
               // Always send the spontaneous events here, otherwise it can break the application!
               if (! d->childrenShownByExpose) {
                  // Show widgets only when they are not yet shown by the expose event
                  d->showChildren(true);
                  QShowEvent showEvent;
                  QCoreApplication::sendSpontaneousEvent(widget, &showEvent);
               }

               d->childrenHiddenByWState = false;   // Set it always to "false" when window is restored

            } else {
               QHideEvent hideEvent;
               QCoreApplication::sendSpontaneousEvent(widget, &hideEvent);
               d->hideChildren(true);
               d->childrenHiddenByWState = true;
            }

            d->childrenShownByExpose = false;      // Set it always to "false" when window state changes
         }

         changeEvent(event);
      }
      break;

      case QEvent::WindowActivate:
      case QEvent::WindowDeactivate: {
         if (isVisible() && !palette().isEqual(QPalette::Active, QPalette::Inactive)) {
            update();
         }

         QList<QObject *> childList = children();

         for (int i = 0; i < childList.size(); ++i) {
            QWidget *w = dynamic_cast<QWidget *>(childList.at(i));

            if (w != nullptr && w->isVisible() && !w->isWindow()) {
               QApplication::sendEvent(w, event);
            }
         }

         break;
      }

      case QEvent::LanguageChange:
         changeEvent(event);

         {
            QList<QObject *> childList = children();

            for (int i = 0; i < childList.size(); ++i) {
               QObject *o = childList.at(i);

               if (o) {
                  QApplication::sendEvent(o, event);
               }
            }
         }
         update();
         break;

      case QEvent::ApplicationLayoutDirectionChange:
         d->resolveLayoutDirection();
         break;

      case QEvent::LayoutDirectionChange:
         if (d->layout) {
            d->layout->invalidate();
         }

         update();
         changeEvent(event);
         break;

      case QEvent::UpdateRequest:
         d->syncBackingStore();
         break;

      case QEvent::UpdateLater:
         update(static_cast<QUpdateLaterEvent *>(event)->region());
         break;

      case QEvent::StyleAnimationUpdate:
         if (isVisible() && !window()->isMinimized()) {
            event->accept();
            update();
         }

         break;

      case QEvent::WindowBlocked:
      case QEvent::WindowUnblocked: {

         QList<QObject *> childList = children();

         if (! childList.isEmpty()) {
            QWidget *modalWidget = QApplication::activeModalWidget();

            for (auto obj : childList) {

               if (obj != nullptr && obj != modalWidget && obj->isWidgetType()) {
                  QWidget *w = dynamic_cast<QWidget *>(obj);

                  // do not forward the event to child windows since QApplication will do this
                  if (w  != nullptr && ! w->isWindow()) {
                     QApplication::sendEvent(w, event);
                  }
               }
            }
         }

      }

      break;

#ifndef QT_NO_TOOLTIP

      case QEvent::ToolTip:
         if (! d->toolTip.isEmpty()) {
            QToolTip::showText(static_cast<QHelpEvent *>(event)->globalPos(), d->toolTip, this);
         } else {
            event->ignore();
         }

         break;
#endif

#ifndef QT_NO_WHATSTHIS

      case QEvent::WhatsThis:
         if (d->whatsThis.size()) {
            QWhatsThis::showText(static_cast<QHelpEvent *>(event)->globalPos(), d->whatsThis, this);
         } else {
            event->ignore();
         }

         break;

      case QEvent::QueryWhatsThis:
         if (d->whatsThis.isEmpty()) {
            event->ignore();
         }

         break;
#endif

      case QEvent::EmbeddingControl:
         d->topData()->frameStrut.setCoords(0, 0, 0, 0);
         m_widgetData->fstrut_dirty = false;
         break;

#ifndef QT_NO_ACTION

      case QEvent::ActionAdded:
      case QEvent::ActionRemoved:
      case QEvent::ActionChanged:
         actionEvent((QActionEvent *)event);
         break;
#endif

      case QEvent::KeyboardLayoutChange: {
         changeEvent(event);

         // inform children of the change
         QList<QObject *> childList = children();

         for (int i = 0; i < childList.size(); ++i) {
            QWidget *w = dynamic_cast<QWidget *>(childList.at(i));

            if (w != nullptr && w->isVisible() && ! w->isWindow()) {
               QApplication::sendEvent(w, event);
            }
         }

         break;
      }

      case QEvent::TouchBegin:
      case QEvent::TouchUpdate:
      case QEvent::TouchEnd:
      case QEvent::TouchCancel: {
         event->ignore();
         break;
      }

#ifndef QT_NO_GESTURES

      case QEvent::Gesture:
         event->ignore();
         break;
#endif

      case QEvent::ScreenChangeInternal:
         if (const QTLWExtra *te = d->maybeTopData()) {
            const QWindow *win = te->window;
            d->setWinId((win && win->handle()) ? win->handle()->winId() : 0);
         }

#ifndef QT_NO_OPENGL
         d->renderToTextureReallyDirty = 1;
#endif
         break;

#ifndef QT_NO_PROPERTIES

      case QEvent::DynamicPropertyChange: {
         const QString &propName = static_cast<QDynamicPropertyChangeEvent *>(event)->propertyName();

         if (propName == "_q_customDpiX") {
            uint value = property(propName).toUInt();

            if (! d->extra) {
               d->createExtra();
            }

            d->extra->customDpiX = value;
            d->updateFont(d->m_privateData.fnt);

         } else if (propName == "_q_customDpiY") {

            uint value = property(propName).toUInt();

            if (! d->extra) {
               d->createExtra();
            }

            d->extra->customDpiY = value;
            d->updateFont(d->m_privateData.fnt);

         } else if (windowHandle() && propName.startsWith("_q_platform_")) {
            windowHandle()->setProperty(propName, property(propName));

         }

         [[fallthrough]];
      }

#endif

      default:
         return QObject::event(event);
   }

   return true;
}

void QWidget::changeEvent(QEvent *event)
{
   switch (event->type()) {
      case QEvent::EnabledChange: {
         update();

#ifndef QT_NO_ACCESSIBILITY
         QAccessible::State s;
         s.disabled = true;
         QAccessibleStateChangeEvent event(this, s);
         QAccessible::updateAccessibility(&event);
#endif
         break;
      }

      case QEvent::FontChange:
      case QEvent::StyleChange: {
         Q_D(QWidget);
         update();
         updateGeometry();

         if (d->layout) {
            d->layout->invalidate();
         }

         break;
      }

      case QEvent::PaletteChange:
         update();
         break;

      case QEvent::ThemeChange:
         if (QApplication::desktopSettingsAware() && windowType() != Qt::Desktop
               && qApp && !QApplication::closingDown()) {
            if (testAttribute(Qt::WA_WState_Polished)) {
               QApplication::style()->unpolish(this);
            }

            if (testAttribute(Qt::WA_WState_Polished)) {
               QApplication::style()->polish(this);
            }

            QEvent styleChangedEvent(QEvent::StyleChange);
            QCoreApplication::sendEvent(this, &styleChangedEvent);

            if (isVisible()) {
               update();
            }
         }

         break;

#ifdef Q_OS_DARWIN
      case QEvent::MacSizeChange:
         updateGeometry();
         break;
#endif

      default:
         break;
   }
}

void QWidget::mouseMoveEvent(QMouseEvent *event)
{
   event->ignore();
}

void QWidget::mousePressEvent(QMouseEvent *event)
{
   event->ignore();

   if ((windowType() == Qt::Popup)) {
      event->accept();
      QWidget *w;

      while ((w = QApplication::activePopupWidget()) && w != this) {
         w->close();

         if (QApplication::activePopupWidget() == w) {
            // widget does not want to disappear
            w->hide();
         }
      }

      if (! rect().contains(event->pos())) {
         close();
      }
   }
}

void QWidget::mouseReleaseEvent(QMouseEvent *event)
{
   event->ignore();
}

void QWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
   mousePressEvent(event);
}

#ifndef QT_NO_WHEELEVENT
void QWidget::wheelEvent(QWheelEvent *event)
{
   event->ignore();
}
#endif

#ifndef QT_NO_TABLETEVENT
void QWidget::tabletEvent(QTabletEvent *event)
{
   event->ignore();
}
#endif

void QWidget::keyPressEvent(QKeyEvent *event)
{
   if ((windowType() == Qt::Popup) && event->matches(QKeySequence::Cancel)) {
      event->accept();
      close();
   } else {
      event->ignore();
   }
}

void QWidget::keyReleaseEvent(QKeyEvent *event)
{
   event->ignore();
}

void QWidget::focusInEvent(QFocusEvent *)
{
   if (focusPolicy() != Qt::NoFocus || !isWindow()) {
      update();
   }
}

void QWidget::focusOutEvent(QFocusEvent *)
{
   if (focusPolicy() != Qt::NoFocus || !isWindow()) {
      update();
   }

   if (qApp->autoSipEnabled() && testAttribute(Qt::WA_InputMethodEnabled)) {
      QGuiApplication::inputMethod()->hide();
   }
}

void QWidget::enterEvent(QEvent *)
{
}

void QWidget::leaveEvent(QEvent *)
{
}

void QWidget::paintEvent(QPaintEvent *)
{
}

void QWidget::moveEvent(QMoveEvent *)
{
}

void QWidget::resizeEvent(QResizeEvent *)
{
}

#ifndef QT_NO_ACTION
void QWidget::actionEvent(QActionEvent *)
{

}
#endif

void QWidget::closeEvent(QCloseEvent *event)
{
   event->accept();
}

#ifndef QT_NO_CONTEXTMENU
void QWidget::contextMenuEvent(QContextMenuEvent *event)
{
   event->ignore();
}
#endif

void QWidget::inputMethodEvent(QInputMethodEvent *event)
{
   event->ignore();
}

QVariant QWidget::inputMethodQuery(Qt::InputMethodQuery query) const
{
   switch (query) {
      case Qt::ImCursorRectangle:
         return QRect(width() / 2, 0, 1, height());

      case Qt::ImFont:
         return font();

      case Qt::ImAnchorPosition:
         // Fallback
         return inputMethodQuery(Qt::ImCursorPosition);

      case Qt::ImHints:
         return (int)inputMethodHints();

      default:
         return QVariant();
   }
}

Qt::InputMethodHints QWidget::inputMethodHints() const
{
#ifndef QT_NO_IM
   const QWidgetPrivate *priv = d_func();

   while (priv->inheritsInputMethodHints) {
      priv = priv->q_func()->parentWidget()->d_func();
      Q_ASSERT(priv);
   }

   return priv->imHints;

#else
   return Qt::EmptyFlag;
#endif
}

void QWidget::setInputMethodHints(Qt::InputMethodHints hints)
{
#ifndef QT_NO_IM
   Q_D(QWidget);

   if (d->imHints == hints) {
      return;
   }

   d->imHints = hints;

   if (this == QGuiApplication::focusObject()) {
      QGuiApplication::inputMethod()->update(Qt::ImHints);
   }

#endif
}

#ifndef QT_NO_DRAGANDDROP
void QWidget::dragEnterEvent(QDragEnterEvent *)
{
}

void QWidget::dragMoveEvent(QDragMoveEvent *)
{
}

void QWidget::dragLeaveEvent(QDragLeaveEvent *)
{
}

void QWidget::dropEvent(QDropEvent *)
{
}
#endif

void QWidget::showEvent(QShowEvent *)
{
}

void QWidget::hideEvent(QHideEvent *)
{
}

bool QWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
   (void) eventType;
   (void) message;
   (void) result;

   return false;
}

void QWidget::ensurePolished() const
{
   Q_D(const QWidget);

   const QMetaObject *m = metaObject();

   if (m == d->polished) {
      return;
   }

   d->polished = m;

   QEvent e(QEvent::Polish);
   QCoreApplication::sendEvent(const_cast<QWidget *>(this), &e);

   // polish children after 'this'
   QList<QObject *> childrenList = children();

   for (int i = 0; i < childrenList.size(); ++i) {
      QObject *o = childrenList.at(i);

      if (! o->isWidgetType()) {
         continue;
      }

      if (QWidget *w = dynamic_cast<QWidget *>(o)) {
         w->ensurePolished();
      }
   }

   bool sendChildEvents = CSInternalEvents::get_m_sendChildEvents(this);

   if (parent() && sendChildEvents) {
      QChildEvent e(QEvent::ChildPolished, const_cast<QWidget *>(this));
      QCoreApplication::sendEvent(parent(), &e);
   }
}

QRegion QWidget::mask() const
{
   Q_D(const QWidget);
   return d->extra ? d->extra->mask : QRegion();
}

QLayout *QWidget::layout() const
{
   return d_func()->layout;
}

void QWidget::setLayout(QLayout *l)
{
   if (! l) {
      qWarning("QWidget::setLayout() Unable to set layout to an invalid value (nullptr)");
      return;
   }

   if (layout() && (layout() != l)) {
      qWarning("QWidget::setLayout() Attempting to set QLayout \"%s\" on %s \"%s\", which already has a layout",
            csPrintable(l->objectName()), csPrintable(metaObject()->className()), csPrintable(objectName()));

      return;
   }

   QObject *oldParent = l->parent();

   if (oldParent && oldParent != this) {

      if (oldParent->isWidgetType()) {
         // Steal the layout off a widget parent. Takes effect when
         // morphing laid-out container widgets in Designer.
         QWidget *oldParentWidget = static_cast<QWidget *>(oldParent);
         oldParentWidget->takeLayout();

      } else {
         qWarning("QWidget::setLayout() Attempting to set QLayout \"%s\" on %s \"%s\", when the QLayout already has a parent",
               csPrintable(l->objectName()), csPrintable(metaObject()->className()), csPrintable(objectName()));
         return;
      }
   }

   Q_D(QWidget);

   l->d_func()->topLevel = true;
   d->layout = l;

   if (oldParent != this) {
      l->setParent(this);
      l->d_func()->reparentChildWidgets(this);
      l->invalidate();
   }

   if (isWindow() && d->maybeTopData()) {
      d->topData()->sizeAdjusted = false;
   }
}

QLayout *QWidget::takeLayout()
{
   Q_D(QWidget);

   QLayout *tmp = layout();

   if (tmp != nullptr) {
      d->layout = nullptr;
      tmp->setParent(nullptr);
   }

   return tmp;
}

QSizePolicy QWidget::sizePolicy() const
{
   Q_D(const QWidget);
   return d->size_policy;
}

void QWidget::setSizePolicy(QSizePolicy policy)
{
   Q_D(QWidget);
   setAttribute(Qt::WA_WState_OwnSizePolicy);

   if (policy == d->size_policy) {
      return;
   }

   if (d->size_policy.retainSizeWhenHidden() != policy.retainSizeWhenHidden()) {
      d->retainSizeWhenHiddenChanged = 1;
   }

   d->size_policy = policy;

#ifndef QT_NO_GRAPHICSVIEW
   if (QWExtra *extra = d->extra) {
      if (extra->proxyWidget) {
         extra->proxyWidget->setSizePolicy(policy);
      }
   }

#endif

   updateGeometry();
   d->retainSizeWhenHiddenChanged = 0;

   if (isWindow() && d->maybeTopData()) {
      d->topData()->sizeAdjusted = false;
   }
}

int QWidget::heightForWidth(int w) const
{
   if (layout() && layout()->hasHeightForWidth()) {
      return layout()->totalHeightForWidth(w);
   }

   return -1;
}

bool QWidget::hasHeightForWidth() const
{
   Q_D(const QWidget);
   return d->layout ? d->layout->hasHeightForWidth() : d->size_policy.hasHeightForWidth();
}

QWidget *QWidget::childAt(const QPoint &p) const
{
   return d_func()->childAt_helper(p, false);
}

QWidget *QWidgetPrivate::childAt_helper(const QPoint &p, bool ignoreChildrenInDestructor) const
{
   Q_Q(const QWidget);

   if (q->children().isEmpty()) {
      return nullptr;
   }

   if (!pointInsideRectAndMask(p)) {
      return nullptr;
   }

   return childAtRecursiveHelper(p, ignoreChildrenInDestructor);
}

QWidget *QWidgetPrivate::childAtRecursiveHelper(const QPoint &p, bool ignoreChildrenInDestructor) const
{
   Q_Q(const QWidget);

   for (int i = q->children().size() - 1; i >= 0; --i) {
      QWidget *child = dynamic_cast<QWidget *>(q->children().at(i));

      if (child == nullptr || child->isWindow() || child->isHidden() ||
            child->testAttribute(Qt::WA_TransparentForMouseEvents) ||
            (ignoreChildrenInDestructor && child->m_widgetData->in_destructor)) {
         continue;
      }

      // Map the point 'p' from parent coordinates to child coordinates.
      QPoint childPoint = p;

      childPoint -= child->m_widgetData->crect.topLeft();

      // Check if the point hits the child.
      if (!child->d_func()->pointInsideRectAndMask(childPoint)) {
         continue;
      }

      // Do the same for the child's descendants.
      if (QWidget *w = child->d_func()->childAtRecursiveHelper(childPoint, ignoreChildrenInDestructor)) {
         return w;
      }

      // We have found our target; namely the child at position 'p'.
      return child;
   }

   return nullptr;
}

void QWidgetPrivate::updateGeometry_helper(bool forceUpdate)
{
   Q_Q(QWidget);

   if (widgetItem != nullptr) {
      widgetItem->invalidateSizeCache();
   }

   QWidget *parent;

   if (forceUpdate || ! extra || extra->minw != extra->maxw || extra->minh != extra->maxh) {
      const int isHidden = q->isHidden() && !size_policy.retainSizeWhenHidden() && ! retainSizeWhenHiddenChanged;

      if (! q->isWindow() && ! isHidden && (parent = q->parentWidget())) {

         if (parent->d_func()->layout) {
            parent->d_func()->layout->invalidate();
         } else if (parent->isVisible()) {
            QApplication::postEvent(parent, new QEvent(QEvent::LayoutRequest));
         }
      }
   }
}

void QWidget::updateGeometry()
{
   Q_D(QWidget);
   d->updateGeometry_helper(false);
}

void QWidget::setWindowFlags(Qt::WindowFlags flags)
{
   Q_D(QWidget);
   d->setWindowFlags(flags);
}

void QWidgetPrivate::setWindowFlags(Qt::WindowFlags flags)
{
   Q_Q(QWidget);

   if (q->m_widgetData->m_flags == flags) {
      return;
   }

   if ((q->m_widgetData->m_flags | flags) & Qt::Window) {
      // the old type was a window and/or the new type is a window
      QPoint oldPos = q->pos();
      bool visible = q->isVisible();

      const bool windowFlagChanged = (q->m_widgetData->m_flags ^ flags) & Qt::Window;

      q->setParent(q->parentWidget(), flags);

      // if both types are windows or neither of them are, we restore
      // the old position
      if (! windowFlagChanged && (visible || q->testAttribute(Qt::WA_Moved))) {
         q->move(oldPos);
      }

      // for backward-compatibility we change Qt::WA_QuitOnClose attribute value only
      // when the window was recreated.
      adjustQuitOnCloseAttribute();

   } else {
      q->m_widgetData->m_flags = flags;

   }
}

void QWidget::overrideWindowFlags(Qt::WindowFlags flags)
{
   m_widgetData->m_flags = flags;
}

void QWidget::setParent(QWidget *parent)
{
   if (parent == parentWidget()) {
      return;
   }

   setParent((QWidget *)parent, windowFlags() & ~Qt::WindowType_Mask);
}

#ifndef QT_NO_OPENGL
static void sendWindowChangeToTextureChildrenRecursively(QWidget *widget)
{
   QWidgetPrivate *d = QWidgetPrivate::get(widget);

   if (d->renderToTexture) {
      QEvent e(QEvent::WindowChangeInternal);
      QApplication::sendEvent(widget, &e);
   }

   for (int i = 0; i < widget->children().size(); ++i) {
      QWidget *w = dynamic_cast<QWidget *>(widget->children().at(i));

      if (w != nullptr && ! w->isWindow() && ! w->isHidden() && QWidgetPrivate::get(w)->textureChildSeen) {
         sendWindowChangeToTextureChildrenRecursively(w);
      }
   }
}
#endif

void QWidget::setParent(QWidget *parent, Qt::WindowFlags flags)
{
   Q_D(QWidget);

   bool resized    = testAttribute(Qt::WA_Resized);
   bool wasCreated = testAttribute(Qt::WA_WState_Created);

   QWidget *oldtlw = window();

   if (flags & Qt::Window) {
      // Frame geometry likely changes, refresh.
      d->m_privateData.fstrut_dirty = true;
   }

   QWidget *desktopWidget = nullptr;

   if (parent && parent->windowType() == Qt::Desktop) {
      desktopWidget = parent;
   }

   bool newParent = (parent != parentWidget()) || ! wasCreated || desktopWidget;

   if (newParent && parent && ! desktopWidget) {
      if (testAttribute(Qt::WA_NativeWindow) && ! qApp->testAttribute(Qt::AA_DontCreateNativeWidgetSiblings)) {
         parent->d_func()->enforceNativeChildren();

      } else if (parent->d_func()->nativeChildrenForced() || parent->testAttribute(Qt::WA_PaintOnScreen)) {
         setAttribute(Qt::WA_NativeWindow);
      }
   }

   if (wasCreated) {
      if (! testAttribute(Qt::WA_WState_Hidden)) {
         hide();
         setAttribute(Qt::WA_WState_ExplicitShowHide, false);
      }

      if (newParent) {
         QEvent e(QEvent::ParentAboutToChange);
         QApplication::sendEvent(this, &e);
      }
   }

   if (newParent && isAncestorOf(focusWidget())) {
      focusWidget()->clearFocus();
   }

   QTLWExtra *oldTopExtra = window()->d_func()->maybeTopData();
   QWidgetBackingStoreTracker *oldBsTracker = oldTopExtra ? &oldTopExtra->backingStoreTracker : nullptr;

   d->setParent_sys(parent, flags);

   QTLWExtra *topExtra = window()->d_func()->maybeTopData();
   QWidgetBackingStoreTracker *bsTracker = topExtra ? &topExtra->backingStoreTracker : nullptr;

   if (oldBsTracker && oldBsTracker != bsTracker) {
      oldBsTracker->unregisterWidgetSubtree(this);
   }

   if (desktopWidget) {
      parent = nullptr;
   }

#ifndef QT_NO_OPENGL
   if (d->textureChildSeen && parent) {
      // set the textureChildSeen flag up the whole parent chain
      QWidgetPrivate::get(parent)->setTextureChildSeen();
   }
#endif

   if (QWidgetBackingStore *oldBs = oldtlw->d_func()->maybeBackingStore()) {
      if (newParent) {
         oldBs->removeDirtyWidget(this);
      }

      // Move the widget and all its static children from
      // the old backing store to the new one.
      oldBs->moveStaticWidgets(this);
   }

   if (QApplicationPrivate::testAttribute(Qt::AA_ImmediateWidgetCreation) && ! testAttribute(Qt::WA_WState_Created)) {
      create();
   }

   d->reparentFocusWidgets(oldtlw);
   setAttribute(Qt::WA_Resized, resized);

   if (! testAttribute(Qt::WA_StyleSheet) && (! parent || ! parent->testAttribute(Qt::WA_StyleSheet))) {
      d->resolveFont();
      d->resolvePalette();
   }

   d->resolveLayoutDirection();
   d->resolveLocale();

   // Note: GL widgets under WGL or EGL will always need a ParentChange
   // event to handle recreation/rebinding of the GL context, hence the
   // (f & Qt::MSWindowsOwnDC) clause (which is set on QGLWidgets on all platforms)

   if (newParent

#if defined(QT_OPENGL_ES)
         || (flags & Qt::MSWindowsOwnDC)
#endif

   ) {
      // propagate enabled updates enabled state to non-windows
      if (! isWindow()) {
         if (! testAttribute(Qt::WA_ForceDisabled)) {
            d->setEnabled_helper(parent ? parent->isEnabled() : true);
         }

         if (! testAttribute(Qt::WA_ForceUpdatesDisabled)) {
            d->setUpdatesEnabled_helper(parent ? parent->updatesEnabled() : true);
         }
      }

      d->inheritStyle();

      // send and post remaining QObject events
      bool sendChildEvents = CSInternalEvents::get_m_sendChildEvents(this);

      if (parent && sendChildEvents) {
         QChildEvent e(QEvent::ChildAdded, this);
         QApplication::sendEvent(parent, &e);
      }

      //### already hidden above, may want to so something different on mac
      // q->setAttribute(Qt::WA_WState_Hidden);

      sendChildEvents = CSInternalEvents::get_m_sendChildEvents(this);

      if (parent && sendChildEvents && d->polished) {
         QChildEvent e(QEvent::ChildPolished, this);
         QCoreApplication::sendEvent(parent, &e);
      }

      QEvent e(QEvent::ParentChange);
      QApplication::sendEvent(this, &e);
   }

#ifndef QT_NO_OPENGL
   // renderToTexture widgets also need to know when their top-level window changes
   if (d->textureChildSeen && oldtlw != window()) {
      sendWindowChangeToTextureChildrenRecursively(this);
   }

#endif

   if (! wasCreated) {
      if (isWindow() || parentWidget()->isVisible()) {
         setAttribute(Qt::WA_WState_Hidden, true);
      } else if (!testAttribute(Qt::WA_WState_ExplicitShowHide)) {
         setAttribute(Qt::WA_WState_Hidden, false);
      }
   }

   d->updateIsOpaque();

#ifndef QT_NO_GRAPHICSVIEW

   // Embed the widget into a proxy if the parent is embedded.
   // ### Doesn't handle reparenting out of an embedded widget.
   if (oldtlw->graphicsProxyWidget()) {
      if (QGraphicsProxyWidget *ancestorProxy = d->nearestGraphicsProxyWidget(oldtlw)) {
         ancestorProxy->d_func()->unembedSubWindow(this);
      }
   }

   if (isWindow() && parent && !graphicsProxyWidget() && ! bypassGraphicsProxyWidget(this)) {
      if (QGraphicsProxyWidget *ancestorProxy = d->nearestGraphicsProxyWidget(parent)) {
         ancestorProxy->d_func()->embedSubWindow(this);
      }
   }

#endif

   if (d->extra && d->extra->hasWindowContainer) {
      QWindowContainer::parentWasChanged(this);
   }
}

void QWidgetPrivate::setParent_sys(QWidget *newparent, Qt::WindowFlags flags)
{
   Q_Q(QWidget);

   Qt::WindowFlags oldFlags = m_privateData.m_flags;
   bool wasCreated = q->testAttribute(Qt::WA_WState_Created);

   int targetScreen = -1;
   // Handle a request to move the widget to a particular screen

   if (newparent && newparent->windowType() == Qt::Desktop) {
      // make sure the widget is created on the same screen as the
      // programmer specified desktop widget
      const QDesktopScreenWidget *sw = dynamic_cast<const QDesktopScreenWidget *>(newparent);
      targetScreen = sw ? sw->screenNumber() : 0;
      newparent    = nullptr;
   }

   setWinId(0);

   if (q->parent() != newparent) {

      q->QObject::setParent(newparent);

      if (q->windowHandle()) {
         q->windowHandle()->setFlags(flags);
         QWidget *parentWithWindow = newparent ? (newparent->windowHandle() ? newparent : newparent->nativeParentWidget()) : nullptr;

         if (parentWithWindow) {
            QWidget *topLevel = parentWithWindow->window();

            if ((flags & Qt::Window) && topLevel && topLevel->windowHandle()) {
               q->windowHandle()->setTransientParent(topLevel->windowHandle());
               q->windowHandle()->setParent(nullptr);

            } else {
               q->windowHandle()->setTransientParent(nullptr);
               q->windowHandle()->setParent(parentWithWindow->windowHandle());
            }

         } else {
            q->windowHandle()->setTransientParent(nullptr);
            q->windowHandle()->setParent(nullptr);
         }
      }
   }

   if (! newparent) {
      flags |= Qt::Window;

      if (targetScreen == -1) {
         if (q->parent()) {
            targetScreen = QApplication::desktop()->screenNumber(q->parentWidget()->window());
         }
      }
   }

   bool explicitlyHidden = q->testAttribute(Qt::WA_WState_Hidden) && q->testAttribute(Qt::WA_WState_ExplicitShowHide);

   // Reparenting toplevel to child
   if (wasCreated && ! (flags & Qt::Window) && (oldFlags & Qt::Window) && ! q->testAttribute(Qt::WA_NativeWindow)) {
      if (extra && extra->hasWindowContainer) {
         QWindowContainer::toplevelAboutToBeDestroyed(q);
      }

      QWindow *newParentWindow = newparent->windowHandle();

      if (! newParentWindow)
         if (QWidget *npw = newparent->nativeParentWidget()) {
            newParentWindow = npw->windowHandle();
         }

      for (QObject *child : q->windowHandle()->children()) {
         QWindow *childWindow = dynamic_cast<QWindow *>(child);

         if (childWindow == nullptr) {
            continue;
         }

         QWidgetWindow *childWW = dynamic_cast<QWidgetWindow *>(childWindow);
         QWidget *childWidget   = childWW ? childWW->widget() : nullptr;

         if (! childWW || (childWidget && childWidget->testAttribute(Qt::WA_NativeWindow))) {
            childWindow->setParent(newParentWindow);
         }
      }

      q->destroy();
   }

   adjustFlags(flags, q);
   m_privateData.m_flags = flags;

   q->setAttribute(Qt::WA_WState_Created, false);
   q->setAttribute(Qt::WA_WState_Visible, false);
   q->setAttribute(Qt::WA_WState_Hidden,  false);

   if (newparent && wasCreated && (q->testAttribute(Qt::WA_NativeWindow) || (flags & Qt::Window))) {
      q->createWinId();
   }

   if (q->isWindow() || (! newparent || newparent->isVisible()) || explicitlyHidden) {
      q->setAttribute(Qt::WA_WState_Hidden);
   }

   q->setAttribute(Qt::WA_WState_ExplicitShowHide, explicitlyHidden);

   // move the window to the selected screen
   if (! newparent && targetScreen != -1) {
      // only if it is already created
      if (q->testAttribute(Qt::WA_WState_Created)) {
         q->windowHandle()->setScreen(QGuiApplication::screens().value(targetScreen, nullptr));
      } else {
         topData()->initialScreenIndex = targetScreen;
      }
   }
}

void QWidget::scroll(int dx, int dy)
{
   if ((! updatesEnabled() && children().size() == 0) || ! isVisible()) {
      return;
   }

   if (dx == 0 && dy == 0) {
      return;
   }

   Q_D(QWidget);

#ifndef QT_NO_GRAPHICSVIEW

   if (QGraphicsProxyWidget *proxy = QWidgetPrivate::nearestGraphicsProxyWidget(this)) {
      // Graphics View maintains its own dirty region as a list of rects;
      // until we can connect item updates directly to the view, we must
      // separately add a translated dirty region.
      if (!d->dirty.isEmpty()) {
         for (const QRect &rect : (d->dirty.translated(dx, dy)).rects()) {
            proxy->update(rect);
         }
      }

      proxy->scroll(dx, dy, proxy->subWidgetRect(this));

      return;
   }

#endif

   d->setDirtyOpaqueRegion();
   d->scroll_sys(dx, dy);
}

void QWidgetPrivate::scroll_sys(int dx, int dy)
{
   Q_Q(QWidget);

   scrollChildren(dx, dy);
   scrollRect(q->rect(), dx, dy);
}

void QWidget::scroll(int dx, int dy, const QRect &r)
{

   if ((! updatesEnabled() && children().size() == 0) || ! isVisible()) {
      return;
   }

   if (dx == 0 && dy == 0) {
      return;
   }

   Q_D(QWidget);

#ifndef QT_NO_GRAPHICSVIEW

   if (QGraphicsProxyWidget *proxy = QWidgetPrivate::nearestGraphicsProxyWidget(this)) {
      // Graphics View maintains its own dirty region as a list of rects;
      // until we can connect item updates directly to the view, we must
      // separately add a translated dirty region.
      if (! d->dirty.isEmpty()) {
         for (const QRect &rect : (d->dirty.translated(dx, dy) & r).rects()) {
            proxy->update(rect);
         }
      }

      proxy->scroll(dx, dy, r.translated(proxy->subWidgetRect(this).topLeft().toPoint()));
      return;
   }

#endif

   d->scroll_sys(dx, dy, r);
}

void QWidgetPrivate::scroll_sys(int dx, int dy, const QRect &r)
{
   scrollRect(r, dx, dy);
}

void QWidget::repaint()
{
   repaint(rect());
}

void QWidget::repaint(int x, int y, int w, int h)
{
   if (x > m_widgetData->crect.width() || y > m_widgetData->crect.height()) {
      return;
   }

   if (w < 0) {
      w = m_widgetData->crect.width()  - x;
   }

   if (h < 0) {
      h = m_widgetData->crect.height() - y;
   }

   repaint(QRect(x, y, w, h));
}

void QWidget::repaint(const QRect &rect)
{
   Q_D(QWidget);

   if (testAttribute(Qt::WA_WState_ConfigPending)) {
      update(rect);
      return;
   }

   if (! isVisible() || ! updatesEnabled() || rect.isEmpty()) {
      return;
   }

   if (hasBackingStoreSupport()) {
      QTLWExtra *tlwExtra = window()->d_func()->maybeTopData();

      if (tlwExtra && !tlwExtra->inTopLevelResize && tlwExtra->backingStore) {
         tlwExtra->inRepaint = true;
         tlwExtra->backingStoreTracker->markDirty(rect, this, QWidgetBackingStore::UpdateNow);
         tlwExtra->inRepaint = false;
      }
   } else {
      d->repaint_sys(rect);
   }
}

void QWidget::repaint(const QRegion &rgn)
{
   Q_D(QWidget);

   if (testAttribute(Qt::WA_WState_ConfigPending)) {
      update(rgn);
      return;
   }

   if (!isVisible() || !updatesEnabled() || rgn.isEmpty()) {
      return;
   }

   if (hasBackingStoreSupport()) {

      QTLWExtra *tlwExtra = window()->d_func()->maybeTopData();

      if (tlwExtra && !tlwExtra->inTopLevelResize && tlwExtra->backingStore) {
         tlwExtra->inRepaint = true;
         tlwExtra->backingStoreTracker->markDirty(rgn, this, QWidgetBackingStore::UpdateNow);
         tlwExtra->inRepaint = false;
      }

   } else {
      d->repaint_sys(rgn);
   }
}

void QWidget::update()
{
   update(rect());
}

void QWidget::update(const QRect &rect)
{
   if (! isVisible() || !updatesEnabled()) {
      return;
   }

   QRect r = rect & QWidget::rect();

   if (r.isEmpty()) {
      return;
   }

   if (testAttribute(Qt::WA_WState_InPaintEvent)) {
      QApplication::postEvent(this, new QUpdateLaterEvent(r));
      return;
   }

   if (hasBackingStoreSupport()) {

      QTLWExtra *tlwExtra = window()->d_func()->maybeTopData();

      if (tlwExtra && !tlwExtra->inTopLevelResize && tlwExtra->backingStore) {
         tlwExtra->backingStoreTracker->markDirty(r, this);
      }

   } else {
      d_func()->repaint_sys(r);
   }
}

void QWidget::update(const QRegion &rgn)
{
   if (!isVisible() || !updatesEnabled()) {
      return;
   }

   QRegion r = rgn & QWidget::rect();

   if (r.isEmpty()) {
      return;
   }

   if (testAttribute(Qt::WA_WState_InPaintEvent)) {
      QApplication::postEvent(this, new QUpdateLaterEvent(r));
      return;
   }

   if (hasBackingStoreSupport()) {

      QTLWExtra *tlwExtra = window()->d_func()->maybeTopData();

      if (tlwExtra && ! tlwExtra->inTopLevelResize && tlwExtra->backingStore) {
         tlwExtra->backingStoreTracker->markDirty(r, this);
      }

   } else {
      d_func()->repaint_sys(r);
   }
}

static void setAttribute_internal(Qt::WidgetAttribute attribute, bool on,
      QWidgetData *widgetData, QWidgetPrivate *d)
{
   if (attribute < int(8 * sizeof(uint))) {
      if (on) {
         widgetData->widget_attributes |= (1 << attribute);
      } else {
         widgetData->widget_attributes &= ~(1 << attribute);
      }

   } else {
      const int x = attribute - 8 * sizeof(uint);
      const int int_off = x / (8 * sizeof(uint));

      if (on) {
         d->high_attributes[int_off] |= (1 << (x - (int_off * 8 * sizeof(uint))));
      } else {
         d->high_attributes[int_off] &= ~(1 << (x - (int_off * 8 * sizeof(uint))));
      }
   }
}

#ifdef Q_OS_DARWIN
void QWidgetPrivate::macUpdateSizeAttribute()
{
   Q_Q(QWidget);

   QEvent event(QEvent::MacSizeChange);
   QApplication::sendEvent(q, &event);

   for (int i = 0; i < q->children().size(); ++i) {
      QWidget *w = dynamic_cast<QWidget *>(q->children().at(i));

      if (w != nullptr && (! w->isWindow() || w->testAttribute(Qt::WA_WindowPropagation))
            && ! q->testAttribute(Qt::WA_MacMiniSize)
            && ! w->testAttribute(Qt::WA_MacSmallSize)
            && ! w->testAttribute(Qt::WA_MacNormalSize)) {

         // no attribute set, inherit from parent?

         w->d_func()->macUpdateSizeAttribute();
      }
   }

   resolveFont();
}
#endif

void QWidget::setAttribute(Qt::WidgetAttribute attribute, bool on)
{
   if (testAttribute(attribute) == on) {
      return;
   }

   Q_D(QWidget);

   static_assert(sizeof(d->high_attributes) * 8 >= (Qt::WA_AttributeCount - sizeof(uint) * 8),
         "QWidget::setAttribute(WidgetAttribute, bool): "
         "QWidgetPrivate::high_attributes[] too small to contain all attributes in WidgetAttribute");

#ifdef Q_OS_WIN
   // ### Do not use PaintOnScreen+paintEngine() to do native painting

   if (attribute == Qt::WA_PaintOnScreen && on && windowType() != Qt::Desktop && !inherits("QGLWidget")) {
      // refer to qwidget_win.cpp, ::paintEngine for details
      paintEngine();

      if (d->noPaintOnScreen) {
         return;
      }
   }
#endif

   if (attribute == Qt::WA_NativeWindow && !d->mustHaveWindowHandle) {
      QPlatformIntegration *platformIntegration = QGuiApplicationPrivate::platformIntegration();

      if (! platformIntegration->hasCapability(QPlatformIntegration::NativeWidgets)) {
         return;
      }
   }

   setAttribute_internal(attribute, on, m_widgetData, d);

   switch (attribute) {

#ifndef QT_NO_DRAGANDDROP

      case Qt::WA_AcceptDrops:  {
         if (on && ! testAttribute(Qt::WA_DropSiteRegistered)) {
            setAttribute(Qt::WA_DropSiteRegistered, true);

         } else if (!on && (isWindow() || ! parentWidget() ||
               ! parentWidget()->testAttribute(Qt::WA_DropSiteRegistered))) {
            setAttribute(Qt::WA_DropSiteRegistered, false);

         }

         QEvent e(QEvent::AcceptDropsChange);
         QApplication::sendEvent(this, &e);
         break;
      }

      case Qt::WA_DropSiteRegistered:  {
         d->registerDropSite(on);

         for (int i = 0; i < children().size(); ++i) {
            QWidget *w = dynamic_cast<QWidget *>(children().at(i));

            if (w != nullptr && ! w->isWindow() && ! w->testAttribute(Qt::WA_AcceptDrops) &&
                  w->testAttribute(Qt::WA_DropSiteRegistered) != on) {
               w->setAttribute(Qt::WA_DropSiteRegistered, on);
            }
         }

         break;
      }

#endif

      case Qt::WA_NoChildEventsForParent:
         CSInternalEvents::set_m_sendChildEvents(this, ! on);
         break;

      case Qt::WA_NoChildEventsFromChildren:
         CSInternalEvents::set_m_receiveChildEvents(this, ! on);
         break;

      case Qt::WA_MacBrushedMetal:

      case Qt::WA_MacAlwaysShowToolWindow:
         break;

      case Qt::WA_MacNormalSize:
      case Qt::WA_MacSmallSize:
      case Qt::WA_MacMiniSize:

#ifdef Q_OS_DARWIN
         {
            // we can only have one of these set at a time
            const Qt::WidgetAttribute MacSizes[] = { Qt::WA_MacNormalSize, Qt::WA_MacSmallSize,
                  Qt::WA_MacMiniSize};

            for (int i = 0; i < 3; ++i) {
               if (MacSizes[i] != attribute) {
                  setAttribute_internal(MacSizes[i], false, m_widgetData, d);
               }
            }

            d->macUpdateSizeAttribute();
         }

#endif
         break;

      case Qt::WA_ShowModal:
         if (! on) {
            // reset modality type to Modeless when clearing WA_ShowModal
            m_widgetData->window_modality = Qt::NonModal;

         } else if (m_widgetData->window_modality == Qt::NonModal) {
            // determine the modality type if it hasn't been set prior
            // to setting WA_ShowModal. set the default to WindowModal
            // if we are the child of a group leader; otherwise use ApplicationModal

            QWidget *w = parentWidget();

            if (w) {
               w = w->window();
            }

            while (w && !w->testAttribute(Qt::WA_GroupLeader)) {
               w = w->parentWidget();

               if (w) {
                  w = w->window();
               }
            }

            m_widgetData->window_modality = (w && w->testAttribute(Qt::WA_GroupLeader))
                  ? Qt::WindowModal : Qt::ApplicationModal;

            // Some window managers does not allow us to enter modal after the
            // window is showing. Therefore, to be consistent, we cannot call
            // QApplicationPrivate::enterModal(this) here. The window must be
            // hidden before changing modality.
         }

         if (testAttribute(Qt::WA_WState_Created)) {
            // don't call setModal_sys() before create_sys()
            d->setModal_sys();
         }

         break;

      case Qt::WA_MouseTracking: {
         QEvent e(QEvent::MouseTrackingChange);
         QApplication::sendEvent(this, &e);
         break;
      }

      case Qt::WA_NativeWindow: {
         d->createTLExtra();

         if (on) {
            d->createTLSysExtra();
         }

#ifndef QT_NO_IM
         QWidget *focusWidget = d->effectiveFocusWidget();

         if (on && ! internalWinId() && this == QGuiApplication::focusObject()
               && focusWidget->testAttribute(Qt::WA_InputMethodEnabled)) {
            QGuiApplication::inputMethod()->commit();
            QGuiApplication::inputMethod()->update(Qt::ImEnabled);
         }

         if (!qApp->testAttribute(Qt::AA_DontCreateNativeWidgetSiblings) && parentWidget()) {
            parentWidget()->d_func()->enforceNativeChildren();
         }

         if (on && ! internalWinId() && testAttribute(Qt::WA_WState_Created)) {
            d->createWinId();
         }

         if (isEnabled() && focusWidget->isEnabled() && this == QGuiApplication::focusObject()
               && focusWidget->testAttribute(Qt::WA_InputMethodEnabled)) {
            QGuiApplication::inputMethod()->update(Qt::ImEnabled);
         }

#endif
         break;
      }

      case Qt::WA_PaintOnScreen:
         d->updateIsOpaque();
         [[fallthrough]];

      case Qt::WA_OpaquePaintEvent:
         d->updateIsOpaque();
         break;

      case Qt::WA_NoSystemBackground:
         d->updateIsOpaque();
         [[fallthrough]];

      case Qt::WA_UpdatesDisabled:
         d->updateSystemBackground();
         break;

      case Qt::WA_TransparentForMouseEvents:

         break;

      case Qt::WA_InputMethodEnabled: {

#ifndef QT_NO_IM

         if (QGuiApplication::focusObject() == this) {
            if (!on) {
               QGuiApplication::inputMethod()->commit();
            }

            QGuiApplication::inputMethod()->update(Qt::ImEnabled);
         }

#endif
         break;
      }

      case Qt::WA_WindowPropagation:
         d->resolvePalette();
         d->resolveFont();
         d->resolveLocale();
         break;

      case Qt::WA_DontShowOnScreen: {
         if (on && isVisible()) {
            // Make sure we keep the current state and only hide the widget
            // from the desktop. show_sys will only update platform specific
            // attributes at this point.
            d->hide_sys();
            d->show_sys();
         }

         break;
      }

      case Qt::WA_X11NetWmWindowTypeDesktop:
      case Qt::WA_X11NetWmWindowTypeDock:
      case Qt::WA_X11NetWmWindowTypeToolBar:
      case Qt::WA_X11NetWmWindowTypeMenu:
      case Qt::WA_X11NetWmWindowTypeUtility:
      case Qt::WA_X11NetWmWindowTypeSplash:
      case Qt::WA_X11NetWmWindowTypeDialog:
      case Qt::WA_X11NetWmWindowTypeDropDownMenu:
      case Qt::WA_X11NetWmWindowTypePopupMenu:
      case Qt::WA_X11NetWmWindowTypeToolTip:
      case Qt::WA_X11NetWmWindowTypeNotification:
      case Qt::WA_X11NetWmWindowTypeCombo:
      case Qt::WA_X11NetWmWindowTypeDND:
         d->setNetWmWindowTypes();
         break;

      case Qt::WA_StaticContents:
         if (QWidgetBackingStore *bs = d->maybeBackingStore()) {
            if (on) {
               bs->addStaticWidget(this);
            } else {
               bs->removeStaticWidget(this);
            }
         }

         break;

      case Qt::WA_TranslucentBackground:
         if (on) {
            setAttribute(Qt::WA_NoSystemBackground);
            d->updateIsTranslucent();
         }

         break;

      case Qt::WA_AcceptTouchEvents:
         break;

      default:
         break;
   }
}

bool QWidget::testAttribute_helper(Qt::WidgetAttribute attribute) const
{
   Q_D(const QWidget);

   const int x = attribute - 8 * sizeof(uint);
   const int int_off = x / (8 * sizeof(uint));

   return (d->high_attributes[int_off] & (1 << (x - (int_off * 8 * sizeof(uint)))));
}

qreal QWidget::windowOpacity() const
{
   Q_D(const QWidget);
   return (isWindow() && d->maybeTopData()) ? d->maybeTopData()->opacity / qreal(255.) : qreal(1.0);
}

void QWidget::setWindowOpacity(qreal opacity)
{
   Q_D(QWidget);

   if (! isWindow()) {
      return;
   }

   opacity = qBound(qreal(0.0), opacity, qreal(1.0));
   QTLWExtra *extra = d->topData();
   extra->opacity = uint(opacity * 255);
   setAttribute(Qt::WA_WState_WindowOpacitySet);
   d->setWindowOpacity_sys(opacity);

   if (! testAttribute(Qt::WA_WState_Created)) {
      return;
   }

#ifndef QT_NO_GRAPHICSVIEW

   if (QGraphicsProxyWidget *proxy = graphicsProxyWidget()) {
      // Avoid invalidating the cache if set.
      if (proxy->cacheMode() == QGraphicsItem::NoCache) {
         proxy->update();
      } else if (QGraphicsScene *scene = proxy->scene()) {
         scene->update(proxy->sceneBoundingRect());
      }

      return;
   }

#endif
}

void QWidgetPrivate::setWindowOpacity_sys(qreal level)
{
   Q_Q(QWidget);

   if (q->windowHandle()) {
      q->windowHandle()->setOpacity(level);
   }
}

bool QWidget::isWindowModified() const
{
   return testAttribute(Qt::WA_WindowModified);
}

void QWidget::setWindowModified(bool mod)
{
   Q_D(QWidget);
   setAttribute(Qt::WA_WindowModified, mod);

   d->setWindowModified_helper();

   QEvent e(QEvent::ModifiedChange);
   QApplication::sendEvent(this, &e);
}

void QWidgetPrivate::setWindowModified_helper()
{
   Q_Q(QWidget);
   QWindow *window = q->windowHandle();

   if (! window) {
      return;
   }

   QPlatformWindow *platformWindow = window->handle();

   if (! platformWindow) {
      return;
   }

   bool on = q->testAttribute(Qt::WA_WindowModified);

   if (! platformWindow->setWindowModified(on)) {

      if (! q->windowTitle().contains("[*]") && on) {
         qWarning("QWidget::setWindowModified() Window title does not contain a '[*]' placeholder");
      }

      setWindowTitle_helper(q->windowTitle());
      setWindowIconText_helper(q->windowIconText());
   }
}

#ifndef QT_NO_TOOLTIP

void QWidget::setToolTip(const QString &s)
{
   Q_D(QWidget);
   d->toolTip = s;

   QEvent event(QEvent::ToolTipChange);
   QApplication::sendEvent(this, &event);
}

QString QWidget::toolTip() const
{
   Q_D(const QWidget);
   return d->toolTip;
}

void QWidget::setToolTipDuration(int msec)
{
   Q_D(QWidget);
   d->toolTipDuration = msec;
}

int QWidget::toolTipDuration() const
{
   Q_D(const QWidget);
   return d->toolTipDuration;
}
#endif

#ifndef QT_NO_STATUSTIP

void QWidget::setStatusTip(const QString &s)
{
   Q_D(QWidget);
   d->statusTip = s;
}

QString QWidget::statusTip() const
{
   Q_D(const QWidget);
   return d->statusTip;
}
#endif

#ifndef QT_NO_WHATSTHIS

void QWidget::setWhatsThis(const QString &s)
{
   Q_D(QWidget);
   d->whatsThis = s;
}

QString QWidget::whatsThis() const
{
   Q_D(const QWidget);
   return d->whatsThis;
}
#endif

#ifndef QT_NO_ACCESSIBILITY

void QWidget::setAccessibleName(const QString &name)
{
   Q_D(QWidget);
   d->accessibleName = name;
   QAccessibleEvent event(this, QAccessible::NameChanged);
   QAccessible::updateAccessibility(&event);
}

QString QWidget::accessibleName() const
{
   Q_D(const QWidget);
   return d->accessibleName;
}

void QWidget::setAccessibleDescription(const QString &description)
{
   Q_D(QWidget);
   d->accessibleDescription = description;
   QAccessibleEvent event(this, QAccessible::DescriptionChanged);
   QAccessible::updateAccessibility(&event);
}

QString QWidget::accessibleDescription() const
{
   Q_D(const QWidget);
   return d->accessibleDescription;
}
#endif // QT_NO_ACCESSIBILITY

#ifndef QT_NO_SHORTCUT
int QWidget::grabShortcut(const QKeySequence &key, Qt::ShortcutContext context)
{
   Q_ASSERT(qApp);

   if (key.isEmpty()) {
      return 0;
   }

   setAttribute(Qt::WA_GrabbedShortcut);
   return qApp->d_func()->shortcutMap.addShortcut(this, key, context, qWidgetShortcutContextMatcher);
}

void QWidget::releaseShortcut(int id)
{
   Q_ASSERT(qApp);

   if (id) {
      qApp->d_func()->shortcutMap.removeShortcut(id, this, 0);
   }
}

void QWidget::setShortcutEnabled(int id, bool enable)
{
   Q_ASSERT(qApp);

   if (id) {
      qApp->d_func()->shortcutMap.setShortcutEnabled(enable, id, this, 0);
   }
}

void QWidget::setShortcutAutoRepeat(int id, bool enable)
{
   Q_ASSERT(qApp);

   if (id) {
      qApp->d_func()->shortcutMap.setShortcutAutoRepeat(enable, id, this, 0);
   }
}
#endif

void QWidget::updateMicroFocus()
{

   if (this == QGuiApplication::focusObject()) {
      QGuiApplication::inputMethod()->update(Qt::ImQueryAll);
   }
}

void QWidget::raise()
{
   Q_D(QWidget);

   if (! isWindow()) {
      QWidget *p = parentWidget();
      const int parentChildCount = p->children().size();

      if (parentChildCount < 2) {
         return;
      }

      const int from = p->children().indexOf(this);
      Q_ASSERT(from >= 0);

      // Do nothing if the widget is already in correct stacking order _and_ created
      if (from != parentChildCount - 1) {
         CSInternalChildren::moveChildren(p, from, parentChildCount - 1);
      }

      if (!testAttribute(Qt::WA_WState_Created) && p->testAttribute(Qt::WA_WState_Created)) {
         create();

      } else if (from == parentChildCount - 1) {
         return;
      }

      QRegion region(rect());
      d->subtractOpaqueSiblings(region);
      d->invalidateBuffer(region);
   }

   if (testAttribute(Qt::WA_WState_Created)) {
      d->raise_sys();
   }

   if (d->extra && d->extra->hasWindowContainer) {
      QWindowContainer::parentWasRaised(this);
   }

   QEvent e(QEvent::ZOrderChange);
   QApplication::sendEvent(this, &e);
}

void QWidgetPrivate::raise_sys()
{
   Q_Q(QWidget);

   if (q->isWindow() || q->testAttribute(Qt::WA_NativeWindow)) {
      q->windowHandle()->raise();
   } else if (renderToTexture) {
      if (QWidget *p = q->parentWidget()) {
         setDirtyOpaqueRegion();
         p->d_func()->invalidateBuffer(effectiveRectFor(q->geometry()));
      }
   }
}

void QWidget::lower()
{
   Q_D(QWidget);

   if (! isWindow()) {
      QWidget *p = parentWidget();
      const int parentChildCount = p->children().size();

      if (parentChildCount < 2) {
         return;
      }

      const int from = p->children().indexOf(this);
      Q_ASSERT(from >= 0);

      // Do nothing if the widget is already in correct stacking order _and_ created.
      if (from != 0)  {
         CSInternalChildren::moveChildren(p, from, 0);
      }

      if (!testAttribute(Qt::WA_WState_Created) && p->testAttribute(Qt::WA_WState_Created)) {
         create();
      } else if (from == 0) {
         return;
      }
   }

   if (testAttribute(Qt::WA_WState_Created)) {
      d->lower_sys();
   }

   if (d->extra && d->extra->hasWindowContainer) {
      QWindowContainer::parentWasLowered(this);
   }

   QEvent e(QEvent::ZOrderChange);
   QApplication::sendEvent(this, &e);
}

void QWidgetPrivate::lower_sys()
{
   Q_Q(QWidget);

   if (q->isWindow() || q->testAttribute(Qt::WA_NativeWindow)) {
      Q_ASSERT(q->testAttribute(Qt::WA_WState_Created));
      q->windowHandle()->lower();
   } else if (QWidget *p = q->parentWidget()) {
      setDirtyOpaqueRegion();
      p->d_func()->invalidateBuffer(effectiveRectFor(q->geometry()));
   }
}
void QWidget::stackUnder(QWidget *w)
{
   Q_D(QWidget);
   QWidget *p = parentWidget();

   if (!w || isWindow() || p != w->parentWidget() || this == w) {
      return;
   }

   if (p) {
      int from = p->children().indexOf(this);
      int to   = p->children().indexOf(w);

      Q_ASSERT(from >= 0);
      Q_ASSERT(to >= 0);

      if (from < to) {
         --to;
      }

      // Do nothing if the widget is already in correct stacking order _and_ created.
      if (from != to) {
         CSInternalChildren::moveChildren(p, from, to);
      }

      if (!testAttribute(Qt::WA_WState_Created) && p->testAttribute(Qt::WA_WState_Created)) {
         create();
      } else if (from == to) {
         return;
      }
   }

   if (testAttribute(Qt::WA_WState_Created)) {
      d->stackUnder_sys(w);
   }

   QEvent e(QEvent::ZOrderChange);
   QApplication::sendEvent(this, &e);
}

void QWidgetPrivate::stackUnder_sys(QWidget *)
{
   Q_Q(QWidget);

   if (QWidget *p = q->parentWidget()) {
      setDirtyOpaqueRegion();
      p->d_func()->invalidateBuffer(effectiveRectFor(q->geometry()));
   }
}

QRect QWidgetPrivate::frameStrut() const
{
   Q_Q(const QWidget);

   if (!q->isWindow() || (q->windowType() == Qt::Desktop) || q->testAttribute(Qt::WA_DontShowOnScreen)) {
      // x2 = x1 + w - 1, so w/h = 1
      return QRect(0, 0, 1, 1);
   }

   if (m_privateData.fstrut_dirty && q->isVisible() && q->testAttribute(Qt::WA_WState_Created)) {
      const_cast<QWidgetPrivate *>(this)->updateFrameStrut();
   }

   return maybeTopData() ? maybeTopData()->frameStrut : QRect();
}

void QWidgetPrivate::updateFrameStrut()
{
   Q_Q(QWidget);

   if (q->m_widgetData->fstrut_dirty) {
      if (QTLWExtra *te = maybeTopData()) {
         if (te->window && te->window->handle()) {
            const QMargins margins = te->window->frameMargins();

            if (!margins.isNull()) {
               te->frameStrut.setCoords(margins.left(), margins.top(), margins.right(), margins.bottom());
               q->m_widgetData->fstrut_dirty = false;
            }
         }
      }
   }
}

#ifdef QT_KEYPAD_NAVIGATION
bool QWidgetPrivate::navigateToDirection(Direction direction)
{
   QWidget *targetWidget = widgetInNavigationDirection(direction);

   if (targetWidget) {
      targetWidget->setFocus();
   }

   return (targetWidget != 0);
}

QWidget *QWidgetPrivate::widgetInNavigationDirection(Direction direction)
{
   const QWidget *sourceWidget = QApplication::focusWidget();

   if (! sourceWidget) {
      return 0;
   }

   const QRect sourceRect = sourceWidget->rect().translated(sourceWidget->mapToGlobal(QPoint()));

   const int sourceX =
         (direction == DirectionNorth || direction == DirectionSouth) ?
         (sourceRect.left() + (sourceRect.right() - sourceRect.left()) / 2)
         : (direction == DirectionEast ? sourceRect.right() : sourceRect.left());

   const int sourceY =
         (direction == DirectionEast || direction == DirectionWest) ?
         (sourceRect.top() + (sourceRect.bottom() - sourceRect.top()) / 2)
         : (direction == DirectionSouth ? sourceRect.bottom() : sourceRect.top());

   const QPoint sourcePoint(sourceX, sourceY);
   const QPoint sourceCenter = sourceRect.center();
   const QWidget *sourceWindow = sourceWidget->window();

   QWidget *targetWidget = 0;
   int shortestDistance = INT_MAX;

   for (QWidget *targetCandidate : QApplication::allWidgets()) {

      const QRect targetCandidateRect = targetCandidate->rect().translated(targetCandidate->mapToGlobal(QPoint()));

      // For focus proxies the child widget handling the focus can have keypad navigation focus,
      // but the owner of the proxy can not. Additionally, empty widgets should be ignored.

      if (targetCandidate->focusProxy() || targetCandidateRect.isEmpty()) {
         continue;
      }

      // Only navigate to a target widget that...
      if (targetCandidate != sourceWidget
            // ...takes the focus,
            && targetCandidate->focusPolicy() & Qt::TabFocus
            // ...is above if DirectionNorth,
            && !(direction == DirectionNorth && targetCandidateRect.bottom() > sourceRect.top())
            // ...is on the right if DirectionEast,
            && !(direction == DirectionEast  && targetCandidateRect.left()   < sourceRect.right())
            // ...is below if DirectionSouth,
            && !(direction == DirectionSouth && targetCandidateRect.top()    < sourceRect.bottom())
            // ...is on the left if DirectionWest,
            && !(direction == DirectionWest  && targetCandidateRect.right()  > sourceRect.left())
            // ...is enabled,
            && targetCandidate->isEnabled()
            // ...is visible,
            && targetCandidate->isVisible()
            // ...is in the same window,
            && targetCandidate->window() == sourceWindow) {
         const int targetCandidateDistance = pointToRect(sourcePoint, targetCandidateRect);

         if (targetCandidateDistance < shortestDistance) {
            shortestDistance = targetCandidateDistance;
            targetWidget = targetCandidate;
         }
      }
   }

   return targetWidget;
}

bool QWidgetPrivate::canKeypadNavigate(Qt::Orientation orientation)
{
   return orientation == Qt::Horizontal ?
         (QWidgetPrivate::widgetInNavigationDirection(QWidgetPrivate::DirectionEast) ||
         QWidgetPrivate::widgetInNavigationDirection(QWidgetPrivate::DirectionWest))
         : (QWidgetPrivate::widgetInNavigationDirection(QWidgetPrivate::DirectionNorth) ||
         QWidgetPrivate::widgetInNavigationDirection(QWidgetPrivate::DirectionSouth));
}

bool QWidgetPrivate::inTabWidget(QWidget *widget)
{
   for (QWidget *tabWidget = widget; tabWidget; tabWidget = tabWidget->parentWidget()) {
      if (dynamic_cast<const QTabWidget *>(tabWidget)) {
         return true;
      }
   }

   return false;
}
#endif

void QWidget::setBackingStore(QBackingStore *store)
{
   // ### createWinId() ??

   if (! isTopLevel()) {
      return;
   }

   Q_D(QWidget);

   QTLWExtra *topData = d->topData();

   if (topData->backingStore == store) {
      return;
   }

   QBackingStore *oldStore = topData->backingStore;
   deleteBackingStore(d);
   topData->backingStore = store;

   QWidgetBackingStore *bs = d->maybeBackingStore();

   if (!bs) {
      return;
   }

   if (isTopLevel()) {
      if (bs->store != oldStore && bs->store != store) {
         delete bs->store;
      }

      bs->store = store;
   }
}

QBackingStore *QWidget::backingStore() const
{
   Q_D(const QWidget);
   QTLWExtra *extra = d->maybeTopData();

   if (extra && extra->backingStore) {
      return extra->backingStore;
   }

   QWidgetBackingStore *bs = d->maybeBackingStore();

   return bs ? bs->store : nullptr;
}

void QWidgetPrivate::getLayoutItemMargins(int *left, int *top, int *right, int *bottom) const
{
   if (left) {
      *left = (int)leftLayoutItemMargin;
   }

   if (top) {
      *top = (int)topLayoutItemMargin;
   }

   if (right) {
      *right = (int)rightLayoutItemMargin;
   }

   if (bottom) {
      *bottom = (int)bottomLayoutItemMargin;
   }
}

void QWidgetPrivate::setLayoutItemMargins(int left, int top, int right, int bottom)
{
   if (leftLayoutItemMargin == left && topLayoutItemMargin == top
         && rightLayoutItemMargin == right && bottomLayoutItemMargin == bottom) {
      return;
   }

   Q_Q(QWidget);

   leftLayoutItemMargin   = (signed char)left;
   topLayoutItemMargin    = (signed char)top;
   rightLayoutItemMargin  = (signed char)right;
   bottomLayoutItemMargin = (signed char)bottom;

   q->updateGeometry();
}

void QWidgetPrivate::setLayoutItemMargins(QStyle::SubElement element, const QStyleOption *opt)
{
   Q_Q(QWidget);
   QStyleOption myOpt;

   if (! opt) {
      myOpt.initFrom(q);
      myOpt.rect.setRect(0, 0, 32768, 32768);     // arbitrary
      opt = &myOpt;
   }

   QRect liRect = q->style()->subElementRect(element, opt, q);

   if (liRect.isValid()) {
      leftLayoutItemMargin   = (signed char)(opt->rect.left() - liRect.left());
      topLayoutItemMargin    = (signed char)(opt->rect.top() - liRect.top());
      rightLayoutItemMargin  = (signed char)(liRect.right() - opt->rect.right());
      bottomLayoutItemMargin = (signed char)(liRect.bottom() - opt->rect.bottom());
   } else {
      leftLayoutItemMargin   = 0;
      topLayoutItemMargin    = 0;
      rightLayoutItemMargin  = 0;
      bottomLayoutItemMargin = 0;
   }
}

// resets the Qt::WA_QuitOnClose attribute to the default value for transient widgets.
void QWidgetPrivate::adjustQuitOnCloseAttribute()
{
   Q_Q(QWidget);

   if (! q->parentWidget()) {
      Qt::WindowType type = q->windowType();

      if (type == Qt::Widget || type == Qt::SubWindow) {
         type = Qt::Window;
      }

      if (type != Qt::Widget && type != Qt::Window && type != Qt::Dialog) {
         q->setAttribute(Qt::WA_QuitOnClose, false);
      }
   }
}

QOpenGLContext *QWidgetPrivate::shareContext() const
{
#ifdef QT_NO_OPENGL
   return nullptr;

#else
   if (! extra || ! extra->topextra || ! extra->topextra->window) {
      qWarning("QWidget::shareContext() Requested a share context for a widget which does not have a window handle");
      return nullptr;
   }

   QWidgetPrivate *that = const_cast<QWidgetPrivate *>(this);

   if (! extra->topextra->shareContext) {
      QOpenGLContext *ctx = new QOpenGLContext;
      ctx->setShareContext(qt_gl_global_share_context());
      ctx->setFormat(extra->topextra->window->format());
      ctx->setScreen(extra->topextra->window->screen());
      ctx->create();
      that->extra->topextra->shareContext = ctx;
   }

   return that->extra->topextra->shareContext;
#endif
}

#ifndef QT_NO_OPENGL
void QWidgetPrivate::sendComposeStatus(QWidget *widget, bool end)
{
   QWidgetPrivate *widgetData = QWidgetPrivate::get(widget);

   if (! widgetData->textureChildSeen) {
      return;
   }

   if (end) {
      widgetData->endCompose();
   } else {
      widgetData->beginCompose();
   }

   for (int i = 0; i < widget->children().size(); ++i) {
      QWidget *tmp = dynamic_cast<QWidget *>(widget->children().at(i));

      if (tmp != nullptr && ! tmp->isWindow() && ! tmp->isHidden() && QWidgetPrivate::get(tmp)->textureChildSeen) {
         sendComposeStatus(tmp, end);
      }
   }
}
#endif

Q_GUI_EXPORT QWidgetData *qt_qwidget_data(QWidget *widget)
{
   return widget->m_widgetData;
}

Q_GUI_EXPORT QWidgetPrivate *qt_widget_private(QWidget *widget)
{
   return widget->d_func();
}

#ifndef QT_NO_GRAPHICSVIEW
QGraphicsProxyWidget *QWidget::graphicsProxyWidget() const
{
   Q_D(const QWidget);

   if (d->extra) {
      return d->extra->proxyWidget;
   }

   return nullptr;
}
#endif

#ifndef QT_NO_GESTURES
void QWidget::grabGesture(Qt::GestureType gesture, Qt::GestureFlags flags)
{
   Q_D(QWidget);
   d->gestureContext.insert(gesture, flags);
   (void)QGestureManager::instance(); // create a gesture manager
}

void QWidget::ungrabGesture(Qt::GestureType gesture)
{
   Q_D(QWidget);

   if (d->gestureContext.remove(gesture)) {
      if (QGestureManager *manager = QGestureManager::instance()) {
         manager->cleanupCachedGestures(this, gesture);
      }
   }
}
#endif

void QWidget::destroy(bool destroyWindow, bool destroySubWindows)
{
   Q_D(QWidget);

   d->aboutToDestroy();

   if (! isWindow() && parentWidget()) {
      parentWidget()->d_func()->invalidateBuffer(d->effectiveRectFor(geometry()));
   }

   d->deactivateWidgetCleanup();

   if ((windowType() == Qt::Popup) && qApp) {
      qApp->d_func()->closePopup(this);
   }

   if (this == QApplicationPrivate::active_window) {
      QApplication::setActiveWindow(nullptr);
   }

   if (QWidget::mouseGrabber() == this) {
      releaseMouse();
   }

   if (QWidget::keyboardGrabber() == this) {
      releaseKeyboard();
   }

   setAttribute(Qt::WA_WState_Created, false);

   if (windowType() != Qt::Desktop) {
      if (destroySubWindows) {
         QObjectList childList(children());

         for (int i = 0; i < childList.size(); i++) {
            QWidget *widget = dynamic_cast<QWidget *>(childList.at(i));

            if (widget != nullptr && widget->testAttribute(Qt::WA_NativeWindow)) {
               if (widget->windowHandle()) {
                  widget->destroy();
               }
            }
         }
      }

      if (destroyWindow) {
         d->deleteTLSysExtra();
      } else {
         if (parentWidget() && parentWidget()->testAttribute(Qt::WA_WState_Created)) {
            d->hide_sys();
         }
      }

      d->setWinId(0);
   }
}

QPaintEngine *QWidget::paintEngine() const
{
   qWarning("QWidget::paintEngine() This method should not be called directly");

#ifdef Q_OS_WIN
   const_cast<QWidgetPrivate *>(d_func())->noPaintOnScreen = 1;
#endif

   return nullptr;
}

// Do not call QWindow::mapToGlobal() until QPlatformWindow is properly showing
static inline bool canMapPosition(QWindow *window)
{
   return window->handle() && !qt_window_private(window)->resizeEventPending;
}

#ifndef QT_NO_GRAPHICSVIEW
static inline QGraphicsProxyWidget *graphicsProxyWidget(const QWidget *w)
{
   QGraphicsProxyWidget *result = nullptr;
   const QWidgetPrivate *d = qt_widget_private(const_cast<QWidget *>(w));

   if (d->extra) {
      result = d->extra->proxyWidget;
   }

   return result;
}
#endif

struct MapToGlobalTransformResult {
   QTransform transform;
   QWindow *window;
};

static MapToGlobalTransformResult mapToGlobalTransform(const QWidget *w)
{
   MapToGlobalTransformResult result;
   result.window = nullptr;

   while (w != nullptr) {

#ifndef QT_NO_GRAPHICSVIEW
      if (QGraphicsProxyWidget *qgpw = graphicsProxyWidget(w)) {
         if (const QGraphicsScene *scene = qgpw->scene()) {
            const QList <QGraphicsView *> views = scene->views();

            if (! views.isEmpty()) {
               result.transform *= qgpw->sceneTransform();
               result.transform *= views.first()->viewportTransform();
               w = views.first()->viewport();
            }
         }
      }
#endif

      QWindow *window = w->windowHandle();

      if (window != nullptr && canMapPosition(window)) {
         result.window = window;
         break;
      }

      const QPoint topLeft = w->geometry().topLeft();
      result.transform.translate(topLeft.x(), topLeft.y());

      if (w->isWindow()) {
         break;
      }

      w = w->parentWidget();
   }

   return result;
}

QPoint QWidget::mapToGlobal(const QPoint &pos) const
{
   const MapToGlobalTransformResult t = mapToGlobalTransform(this);
   const QPoint g = t.transform.map(pos);
   return t.window ? t.window->mapToGlobal(g) : g;
}

QPoint QWidget::mapFromGlobal(const QPoint &pos) const
{
   const MapToGlobalTransformResult t = mapToGlobalTransform(this);
   const QPoint windowLocal = t.window ? t.window->mapFromGlobal(pos) : pos;
   return t.transform.inverted().map(windowLocal);
}

QWidget *qt_pressGrab = nullptr;
QWidget *qt_mouseGrb  = nullptr;

static bool mouseGrabWithCursor = false;
static QWidget *keyboardGrb     = nullptr;

static inline QWindow *grabberWindow(const QWidget *w)
{
   QWindow *window = w->windowHandle();

   if (! window)
      if (const QWidget *nativeParent = w->nativeParentWidget()) {
         window = nativeParent->windowHandle();
      }

   return window;
}

#ifndef QT_NO_CURSOR
static void grabMouseForWidget(QWidget *widget, const QCursor *cursor = nullptr)
#else
static void grabMouseForWidget(QWidget *widget)
#endif

{
   if (qt_mouseGrb) {
      qt_mouseGrb->releaseMouse();
   }

   mouseGrabWithCursor = false;

   if (QWindow *window = grabberWindow(widget)) {

#ifndef QT_NO_CURSOR
      if (cursor) {
         mouseGrabWithCursor = true;
         QGuiApplication::setOverrideCursor(*cursor);
      }
#endif

      window->setMouseGrabEnabled(true);
   }

   qt_mouseGrb  = widget;
   qt_pressGrab = nullptr;
}

static void releaseMouseGrabOfWidget(QWidget *widget)
{
   if (qt_mouseGrb == widget) {

      if (QWindow *window = grabberWindow(widget)) {

#ifndef QT_NO_CURSOR
         if (mouseGrabWithCursor) {
            QGuiApplication::restoreOverrideCursor();
            mouseGrabWithCursor = false;
         }
#endif
         window->setMouseGrabEnabled(false);
      }
   }

   qt_mouseGrb = nullptr;
}

void QWidget::grabMouse()
{
   grabMouseForWidget(this);
}

#ifndef QT_NO_CURSOR
void QWidget::grabMouse(const QCursor &cursor)
{
   grabMouseForWidget(this, &cursor);
}
#endif

bool QWidgetPrivate::stealMouseGrab(bool grab)
{
   // combination of grab/releaseMouse() but with error checking,
   // has no effect on the result of mouseGrabber()

   Q_Q(QWidget);
   QWindow *window = grabberWindow(q);
   return window ? window->setMouseGrabEnabled(grab) : false;
}

void QWidget::releaseMouse()
{
   releaseMouseGrabOfWidget(this);
}

void QWidget::grabKeyboard()
{
   if (keyboardGrb) {
      keyboardGrb->releaseKeyboard();
   }

   if (QWindow *window = grabberWindow(this)) {
      window->setKeyboardGrabEnabled(true);
   }

   keyboardGrb = this;
}

bool QWidgetPrivate::stealKeyboardGrab(bool grab)
{
   // This is like a combination of grab/releaseKeyboard() but with error
   // checking and it has no effect on the result of keyboardGrabber()

   Q_Q(QWidget);

   QWindow *window = grabberWindow(q);
   return window ? window->setKeyboardGrabEnabled(grab) : false;
}

void QWidget::releaseKeyboard()
{
   if (keyboardGrb == this) {
      if (QWindow *window = grabberWindow(this)) {
         window->setKeyboardGrabEnabled(false);
      }

      keyboardGrb = nullptr;
   }
}

QWidget *QWidget::mouseGrabber()
{
   if (qt_mouseGrb) {
      return qt_mouseGrb;
   }

   return qt_pressGrab;
}

QWidget *QWidget::keyboardGrabber()
{
   return keyboardGrb;
}

void QWidget::activateWindow()
{
   QWindow *const wnd = window()->windowHandle();

   if (wnd) {
      wnd->requestActivate();
   }
}

int QWidget::metric(PaintDeviceMetric m) const
{
   Q_D(const QWidget);

   QWindow *topLevelWindow = nullptr;
   QScreen *screen = nullptr;

   if (QWidget *topLevel = window()) {
      topLevelWindow = topLevel->windowHandle();

      if (topLevelWindow) {
         screen = topLevelWindow->screen();
      }
   }

   if (!screen && QGuiApplication::primaryScreen()) {
      screen = QGuiApplication::primaryScreen();
   }

   if (! screen) {
      if (m == PdmDpiX || m == PdmDpiY) {
         return 72;
      }

      return QPaintDevice::metric(m);
   }

   int val;

   if (m == PdmWidth) {
      val = m_widgetData->crect.width();

   } else if (m == PdmWidthMM) {
      val = m_widgetData->crect.width() * screen->physicalSize().width() / screen->geometry().width();

   } else if (m == PdmHeight) {
      val = m_widgetData->crect.height();

   } else if (m == PdmHeightMM) {
      val = m_widgetData->crect.height() * screen->physicalSize().height() / screen->geometry().height();

   } else if (m == PdmDepth) {
      return screen->depth();

   } else if (m == PdmDpiX) {
      if (d->extra && d->extra->customDpiX) {
         return d->extra->customDpiX;

      } else if (parent()) {
         return static_cast<QWidget *>(parent())->metric(m);

      }

      return qRound(screen->logicalDotsPerInchX());

   } else if (m == PdmDpiY) {
      if (d->extra && d->extra->customDpiY) {
         return d->extra->customDpiY;
      } else if (parent()) {
         return static_cast<QWidget *>(parent())->metric(m);
      }

      return qRound(screen->logicalDotsPerInchY());

   } else if (m == PdmPhysicalDpiX) {
      return qRound(screen->physicalDotsPerInchX());

   } else if (m == PdmPhysicalDpiY) {
      return qRound(screen->physicalDotsPerInchY());

   } else if (m == PdmDevicePixelRatio) {
      return topLevelWindow ? topLevelWindow->devicePixelRatio() : qApp->devicePixelRatio();

   } else if (m == PdmDevicePixelRatioScaled) {
      return (QPaintDevice::devicePixelRatioFScale() *
                  (topLevelWindow ? topLevelWindow->devicePixelRatio() : qApp->devicePixelRatio()));
   } else {
      val = QPaintDevice::metric(m);// XXX
   }

   return val;
}

void QWidget::initPainter(QPainter *painter) const
{
   const QPalette &pal = palette();
   painter->d_func()->state->pen = QPen(pal.brush(foregroundRole()), 1);
   painter->d_func()->state->bgBrush = pal.brush(backgroundRole());

   QFont f(font(), const_cast<QWidget *>(this));
   painter->d_func()->state->deviceFont = f;
   painter->d_func()->state->font = f;
}

QPaintDevice *QWidget::redirected(QPoint *offset) const
{
   return d_func()->redirected(offset);
}

QPainter *QWidget::sharedPainter() const
{
   // Someone sent a paint event directly to the widget
   if (!d_func()->redirectDev) {
      return nullptr;
   }

   QPainter *sp = d_func()->sharedPainter();

   if (! sp || !sp->isActive()) {
      return nullptr;
   }

   if (sp->paintEngine()->paintDevice() != d_func()->redirectDev) {
      return nullptr;
   }

   return sp;
}

void QWidget::setMask(const QRegion &newMask)
{
   Q_D(QWidget);

   d->createExtra();

   if (newMask == d->extra->mask) {
      return;
   }

#ifndef QT_NO_BACKINGSTORE
   const QRegion oldMask(d->extra->mask);
#endif

   d->extra->mask = newMask;
   d->extra->hasMask = !newMask.isEmpty();

   if (! testAttribute(Qt::WA_WState_Created)) {
      return;
   }

   d->setMask_sys(newMask);

#ifndef QT_NO_BACKINGSTORE

   if (! isVisible()) {
      return;
   }

   if (!d->extra->hasMask) {
      // Mask was cleared; update newly exposed area.
      QRegion expose(rect());
      expose -= oldMask;

      if (!expose.isEmpty()) {
         d->setDirtyOpaqueRegion();
         update(expose);
      }

      return;
   }

   if (! isWindow()) {
      // Update newly exposed area on the parent widget.
      QRegion parentExpose(rect());
      parentExpose -= newMask;

      if (!parentExpose.isEmpty()) {
         d->setDirtyOpaqueRegion();
         parentExpose.translate(m_widgetData->crect.topLeft());
         parentWidget()->update(parentExpose);
      }

      // Update newly exposed area on this widget
      if (!oldMask.isEmpty()) {
         update(newMask - oldMask);
      }
   }

#endif
}

void QWidgetPrivate::setMask_sys(const QRegion &region)
{
   Q_Q(QWidget);

   if (QWindow *window = q->windowHandle()) {
      window->setMask(region);
   }
}

void QWidget::setMask(const QBitmap &bitmap)
{
   setMask(QRegion(bitmap));
}

void QWidget::clearMask()
{
   setMask(QRegion());
}

void QWidgetPrivate::setWidgetParentHelper(QObject *widgetAsObject, QObject *newParent)
{
   Q_ASSERT(widgetAsObject->isWidgetType());
   Q_ASSERT(!newParent || newParent->isWidgetType());
   QWidget *widget = static_cast<QWidget *>(widgetAsObject);
   widget->setParent(static_cast<QWidget *>(newParent));
}

void QWidgetPrivate::setNetWmWindowTypes(bool skipIfMissing)
{
   Q_Q(QWidget);

   if (!q->windowHandle()) {
      return;
   }

   int wmWindowType = 0;

   if (q->testAttribute(Qt::WA_X11NetWmWindowTypeDesktop)) {
      wmWindowType |= QXcbWindowFunctions::Desktop;
   }

   if (q->testAttribute(Qt::WA_X11NetWmWindowTypeDock)) {
      wmWindowType |= QXcbWindowFunctions::Dock;
   }

   if (q->testAttribute(Qt::WA_X11NetWmWindowTypeToolBar)) {
      wmWindowType |= QXcbWindowFunctions::Toolbar;
   }

   if (q->testAttribute(Qt::WA_X11NetWmWindowTypeMenu)) {
      wmWindowType |= QXcbWindowFunctions::Menu;
   }

   if (q->testAttribute(Qt::WA_X11NetWmWindowTypeUtility)) {
      wmWindowType |= QXcbWindowFunctions::Utility;
   }

   if (q->testAttribute(Qt::WA_X11NetWmWindowTypeSplash)) {
      wmWindowType |= QXcbWindowFunctions::Splash;
   }

   if (q->testAttribute(Qt::WA_X11NetWmWindowTypeDialog)) {
      wmWindowType |= QXcbWindowFunctions::Dialog;
   }

   if (q->testAttribute(Qt::WA_X11NetWmWindowTypeDropDownMenu)) {
      wmWindowType |= QXcbWindowFunctions::DropDownMenu;
   }

   if (q->testAttribute(Qt::WA_X11NetWmWindowTypePopupMenu)) {
      wmWindowType |= QXcbWindowFunctions::PopupMenu;
   }

   if (q->testAttribute(Qt::WA_X11NetWmWindowTypeToolTip)) {
      wmWindowType |= QXcbWindowFunctions::Tooltip;
   }

   if (q->testAttribute(Qt::WA_X11NetWmWindowTypeNotification)) {
      wmWindowType |= QXcbWindowFunctions::Notification;
   }

   if (q->testAttribute(Qt::WA_X11NetWmWindowTypeCombo)) {
      wmWindowType |= QXcbWindowFunctions::Combo;
   }

   if (q->testAttribute(Qt::WA_X11NetWmWindowTypeDND)) {
      wmWindowType |= QXcbWindowFunctions::Dnd;
   }

   if (wmWindowType == 0 && skipIfMissing) {
      return;
   }

   QXcbWindowFunctions::setWmWindowType(q->windowHandle(), static_cast<QXcbWindowFunctions::WmWindowType>(wmWindowType));
}

QDebug operator<<(QDebug debug, const QWidget *widget)
{
   const QDebugStateSaver saver(debug);
   debug.nospace();

   if (widget != nullptr) {
      debug << widget->metaObject()->className() << '(' ;

      if (! widget->objectName().isEmpty()) {
         debug << widget->objectName();
      }

      debug << ')';

   } else {
      debug << "QWidget(0x0)";
   }

   return debug;
}

QWidgetPrivate *QWidgetPrivate::cs_getPrivate(QWidget *object)
{
   return object->d_ptr.data();
}

QWidget *QWidgetPrivate::cs_getPublic(QWidgetPrivate *object)
{
   return object->q_ptr;
}
