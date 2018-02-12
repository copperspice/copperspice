/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qapplication.h>
#include <qapplication_p.h>
#include <qbrush.h>
#include <qcursor.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <qhash.h>
#include <qlayout.h>
#include <qmenu.h>
#include <qmetaobject.h>
#include <qpixmap.h>
#include <qpointer.h>
#include <qstack.h>
#include <qstyle.h>
#include <qstylefactory.h>
#include <qvariant.h>
#include <qwidget.h>
#include <qstyleoption.h>

#ifndef QT_NO_ACCESSIBILITY
# include <qaccessible.h>
#endif

#if defined(Q_OS_WIN)
# include <qt_windows.h>
#endif

#if defined(Q_OS_MAC)
# include <qt_mac_p.h>
# include <qt_cocoa_helpers_mac_p.h>
# include <qmainwindow.h>
# include <qtoolbar.h>
# include <qmainwindowlayout_p.h>
#endif

#if defined(Q_WS_QWS)
# include <qwsdisplay_qws.h>
# include <qwsmanager_qws.h>
# include <qpaintengine.h>
# include <qwindowsurface_qws_p.h>
#endif

#if defined(Q_WS_QPA)
#include <qplatformwindow_qpa.h>
#endif

#include <qpainter.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qdebug.h>
#include <qstylesheetstyle_p.h>
#include <qstyle_p.h>
#include <qinputcontext_p.h>
#include <qfileinfo.h>

#if defined (Q_OS_WIN)
# include <qwininputcontext_p.h>
#endif

#if defined(Q_WS_X11)
# include <qpaintengine_x11_p.h>
# include <qx11info_x11.h>
#endif

#include <qgraphicseffect_p.h>
#include <qwindowsurface_p.h>
#include <qbackingstore_p.h>

#ifdef Q_OS_MAC
# include <qpaintengine_mac_p.h>
#endif

#include <qpaintengine_raster_p.h>

#include <qwidget_p.h>
#include <qaction_p.h>
#include <qlayout_p.h>
#include <QtGui/qgraphicsproxywidget.h>
#include <QtGui/qgraphicsscene.h>
#include <qgraphicsproxywidget_p.h>
#include <QtGui/qabstractscrollarea.h>
#include <qabstractscrollarea_p.h>
#include <qevent_p.h>
#include <qgraphicssystem_p.h>
#include <qgesturemanager_p.h>

#ifdef QT_KEYPAD_NAVIGATION
#include <qtabwidget.h>             // needed in inTabWidget()
#endif

QT_BEGIN_NAMESPACE

#if !defined(Q_WS_QWS)
static bool qt_enable_backingstore = true;
#endif

#ifdef Q_WS_X11
// for compatibility with Qt 4.0
Q_GUI_EXPORT void qt_x11_set_global_double_buffer(bool enable)
{
   qt_enable_backingstore = enable;
}
#endif

#ifdef Q_OS_MAC
bool qt_mac_clearDirtyOnWidgetInsideDrawWidget = false;
#endif

static inline bool qRectIntersects(const QRect &r1, const QRect &r2)
{
   return (qMax(r1.left(), r2.left()) <= qMin(r1.right(), r2.right()) &&
           qMax(r1.top(), r2.top()) <= qMin(r1.bottom(), r2.bottom()));
}

static inline bool hasBackingStoreSupport()
{
#ifdef Q_OS_MAC
   return QApplicationPrivate::graphicsSystem() != 0;
#else
   return true;
#endif
}

extern bool qt_sendSpontaneousEvent(QObject *, QEvent *); // qapplication.cpp
extern QDesktopWidget *qt_desktopWidget; // qapplication.cpp

// internal
QWidgetBackingStoreTracker::QWidgetBackingStoreTracker()
   :   m_ptr(0)
{
}

QWidgetBackingStoreTracker::~QWidgetBackingStoreTracker()
{
   delete m_ptr;
}

// internal
void QWidgetBackingStoreTracker::create(QWidget *widget)
{
   destroy();
   m_ptr = new QWidgetBackingStore(widget);
}

// internal
void QWidgetBackingStoreTracker::destroy()
{
   delete m_ptr;
   m_ptr = 0;
   m_widgets.clear();
}

// internal
void QWidgetBackingStoreTracker::registerWidget(QWidget *w)
{
   Q_ASSERT(m_ptr);
   Q_ASSERT(w->internalWinId());
   Q_ASSERT(qt_widget_private(w)->maybeBackingStore() == m_ptr);
   m_widgets.insert(w);
}

// internal
void QWidgetBackingStoreTracker::unregisterWidget(QWidget *w)
{
   if (m_widgets.remove(w) && m_widgets.isEmpty()) {
      delete m_ptr;
      m_ptr = 0;
   }
}

// internal
void QWidgetBackingStoreTracker::unregisterWidgetSubtree(QWidget *widget)
{
   unregisterWidget(widget);
   for (QObject * child : widget->children())  {
      if (QWidget *childWidget = qobject_cast<QWidget *>(child)) {
         unregisterWidgetSubtree(childWidget);
      }
   }
}

QWidgetPrivate::QWidgetPrivate()
   :    extra(0)
   , focus_next(0)
   , focus_prev(0)
   , focus_child(0)
   , layout(0)
   , needsFlush(0)
   , redirectDev(0)
   , widgetItem(0)
   , extraPaintEngine(0)
   , polished(0)
   , graphicsEffect(0)
#if !defined(QT_NO_IM)
   , imHints(Qt::ImhNone)
#endif
   , inheritedFontResolveMask(0)
   , inheritedPaletteResolveMask(0)
   , leftmargin(0)
   , topmargin(0)
   , rightmargin(0)
   , bottommargin(0)
   , leftLayoutItemMargin(0)
   , topLayoutItemMargin(0)
   , rightLayoutItemMargin(0)
   , bottomLayoutItemMargin(0)
   , hd(0)
   , size_policy(QSizePolicy::Preferred, QSizePolicy::Preferred)
   , fg_role(QPalette::NoRole)
   , bg_role(QPalette::NoRole)
   , dirtyOpaqueChildren(1)
   , isOpaque(0)
   , inDirtyList(0)
   , isScrolled(0)
   , isMoved(0)
   , isGLWidget(0)
   , usesDoubleBufferedGLContext(0)

#ifndef QT_NO_IM
   , inheritsInputMethodHints(0)
#endif

   , inSetParent(0)

#if defined(Q_WS_X11)
   , picture(0)

#elif defined(Q_OS_WIN)
   , noPaintOnScreen(0)

#ifndef QT_NO_GESTURES
   , nativeGesturePanEnabled(0)

#endif

#elif defined(Q_OS_MAC)
   , needWindowChange(0)
   , qd_hd(0)
#endif

{
   if (! qApp) {
      qFatal("QWidget: Must construct a QApplication before a QPaintDevice");
      return;
   }

   memset(high_attributes, 0, sizeof(high_attributes));

#ifdef Q_OS_MAC
   drawRectOriginalAdded = false;
   originalDrawMethod = true;
   changeMethods = false;
   isInUnifiedToolbar = false;
   unifiedSurface = 0;
   toolbar_ancestor = 0;
   flushRequested = false;
   touchEventsEnabled = false;
#endif

#ifdef QWIDGET_EXTRA_DEBUG
   static int count = 0;
   qDebug() << "widgets" << ++count;
#endif

}

QWidgetPrivate::~QWidgetPrivate()
{
   if (widgetItem) {
      widgetItem->wid = 0;
   }

   if (extra) {
      deleteExtra();
   }

#ifndef QT_NO_GRAPHICSEFFECT
   delete graphicsEffect;
#endif
}

class QDummyWindowSurface : public QWindowSurface
{
 public:
   QDummyWindowSurface(QWidget *window) : QWindowSurface(window) {}

   QPaintDevice *paintDevice() override {
      return window();
   }

   void flush(QWidget *, const QRegion &, const QPoint &) override {}
};

QWindowSurface *QWidgetPrivate::createDefaultWindowSurface()
{
   Q_Q(QWidget);

   QWindowSurface *surface;

#ifndef QT_NO_PROPERTIES
   if (q->property("_q_DummyWindowSurface").toBool()) {
      surface = new QDummyWindowSurface(q);
   } else
#endif
   {
      if (QApplicationPrivate::graphicsSystem()) {
         surface = QApplicationPrivate::graphicsSystem()->createWindowSurface(q);
      } else {
         surface = createDefaultWindowSurface_sys();
      }
   }

   return surface;
}

/*!
    \internal
*/
void QWidgetPrivate::scrollChildren(int dx, int dy)
{
   Q_Q(QWidget);
   if (q->children().size() > 0) {        // scroll children
      QPoint pd(dx, dy);
      QObjectList childObjects = q->children();

      for (int i = 0; i < childObjects.size(); ++i) {
         // move all children
         QWidget *w = qobject_cast<QWidget *>(childObjects.at(i));

         if (w && !w->isWindow()) {
            QPoint oldp = w->pos();
            QRect  r(w->pos() + pd, w->size());
            w->data->crect = r;

#ifndef Q_WS_QWS
            if (w->testAttribute(Qt::WA_WState_Created)) {
               w->d_func()->setWSGeometry();
            }
#endif
            w->d_func()->setDirtyOpaqueRegion();
            QMoveEvent e(r.topLeft(), oldp);
            QApplication::sendEvent(w, &e);
         }
      }
   }
}

#ifndef QT_NO_IM
QInputContext *QWidgetPrivate::assignedInputContext() const
{
   const QWidget *widget = q_func();
   while (widget) {
      if (QInputContext *qic = widget->d_func()->ic) {
         return qic;
      }
      widget = widget->parentWidget();
   }
   return 0;
}

QInputContext *QWidgetPrivate::inputContext() const
{
   if (QInputContext *qic = assignedInputContext()) {
      return qic;
   }
   return qApp->inputContext();
}

QInputContext *QWidget::inputContext()
{
   Q_D(QWidget);
   if (!testAttribute(Qt::WA_InputMethodEnabled)) {
      return 0;
   }

   return d->inputContext();
}

void QWidget::setInputContext(QInputContext *context)
{
   Q_D(QWidget);
   if (!testAttribute(Qt::WA_InputMethodEnabled)) {
      return;
   }

   if (context == d->ic) {
      return;
   }

   if (d->ic) {
      delete d->ic;
   }

   d->ic = context;

   if (d->ic) {
      d->ic->setParent(this);
   }
}
#endif // QT_NO_IM

// obsolete
void QWidget::resetInputContext()
{
   if (!hasFocus()) {
      return;
   }
#ifndef QT_NO_IM
   QInputContext *qic = this->inputContext();
   if (qic) {
      qic->reset();
   }
#endif // QT_NO_IM
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

   if (on && !f->hasFocus()) {
      f->setFocus();
   }

   if ((!on && !QWidgetPrivate::editingWidget)
         || (on && QWidgetPrivate::editingWidget == f)) {
      return;
   }

   if (!on && QWidgetPrivate::editingWidget == f) {
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
   if (!d->extra) {
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

QWidgetMapper *QWidgetPrivate::mapper = 0;          // widget with wid
QWidgetSet *QWidgetPrivate::allWidgets = 0;         // widgets with no wid


/*****************************************************************************
  QWidget utility functions
*****************************************************************************/

QRegion qt_dirtyRegion(QWidget *widget)
{
   if (!widget) {
      return QRegion();
   }

   QWidgetBackingStore *bs = qt_widget_private(widget)->maybeBackingStore();
   if (!bs) {
      return QRegion();
   }

   return bs->dirtyRegion(widget);
}

/*****************************************************************************
  QWidget member functions
*****************************************************************************/

struct QWidgetExceptionCleaner {
   /* this cleans up when the constructor throws an exception */
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

QWidget::QWidget(QWidget *parent, Qt::WindowFlags f)
   : QObject(0), QPaintDevice(), d_ptr(new QWidgetPrivate)
{
   d_ptr->q_ptr = this;
   Q_D(QWidget);

   QT_TRY {
      d->init(parent, f);

   } QT_CATCH(...) {
      QWidgetExceptionCleaner::cleanup(this, d_func());
      QT_RETHROW;
   }
}

// internal
QWidget::QWidget(QWidgetPrivate &dd, QWidget *parent, Qt::WindowFlags f)
   : QObject(0), QPaintDevice(), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
   Q_D(QWidget);

   QT_TRY {
      d->init(parent, f);

   } QT_CATCH(...) {
      QWidgetExceptionCleaner::cleanup(this, d_func());
      QT_RETHROW;
   }
}

void QWidget::_q_showIfNotHidden()
{
   Q_D(QWidget);
   d->_q_showIfNotHidden();
}

// internal
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
#ifndef Q_OS_MAC
      if (flags & (Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint)) {
         flags |= Qt::WindowSystemMenuHint;
#else
      if (flags & (Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint
                   | Qt::WindowSystemMenuHint)) {
#endif
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

   if (customize)
      ; // don't modify window flags if the user explicitly set them

   else if (type == Qt::Dialog || type == Qt::Sheet) {
      flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowContextHelpButtonHint | Qt::WindowCloseButtonHint;
   }

   else if (type == Qt::Tool) {
      flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint;
   }

   else
      flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint
               | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint;


}

void QWidgetPrivate::init(QWidget *parentWidget, Qt::WindowFlags f)
{
   Q_Q(QWidget);

   if (QApplication::type() == QApplication::Tty) {
      qFatal("QWidget: Cannot create a QWidget when no GUI is being used");
   }

   Q_ASSERT(allWidgets);
   if (allWidgets) {
      allWidgets->insert(q);
   }

   QWidget *desktopWidget = 0;
   if (parentWidget && parentWidget->windowType() == Qt::Desktop) {
      desktopWidget = parentWidget;
      parentWidget = 0;
   }

   q->data = &data;

   if (! q->parent() ) {
      Q_ASSERT_X(q->thread() == qApp->thread(), "QWidget", "Widgets must be created in the GUI thread.");
   }

#if defined(Q_WS_X11)
   if (desktopWidget) {
      // make sure the widget is created on the same screen as the
      // programmer specified desktop widget
      xinfo = desktopWidget->d_func()->xinfo;
   }

#elif defined(Q_WS_QPA)
   if (desktopWidget) {
      int screen = desktopWidget->d_func()->topData()->screenIndex;
      topData()->screenIndex = screen;
      QPlatformIntegration *platform = QApplicationPrivate::platformIntegration();
      platform->moveToScreen(q, screen);
   }
#else
   Q_UNUSED(desktopWidget);
#endif

   data.fstrut_dirty = true;

   data.winid = 0;
   data.widget_attributes = 0;
   data.window_flags = f;
   data.window_state = 0;
   data.focus_policy = 0;
   data.context_menu_policy = Qt::DefaultContextMenu;
   data.window_modality = Qt::NonModal;

   data.sizehint_forced = 0;
   data.is_closing = 0;
   data.in_show = 0;
   data.in_set_window_state = 0;
   data.in_destructor = false;

   // Widgets with Qt::MSWindowsOwnDC (typically QGLWidget) must have a window handle.
   if (f & Qt::MSWindowsOwnDC)  {
      q->setAttribute(Qt::WA_NativeWindow);
   }

   q->setAttribute(Qt::WA_QuitOnClose); // might be cleared in adjustQuitOnCloseAttribute()
   adjustQuitOnCloseAttribute();

   q->setAttribute(Qt::WA_WState_Hidden);

   // give potential windows a bigger "pre-initial" size; create_sys() will give them a new size later
   data.crect = parentWidget ? QRect(0, 0, 100, 30) : QRect(0, 0, 640, 480);

   focus_next = focus_prev = q;

   if ((f & Qt::WindowType_Mask) == Qt::Desktop) {
      q->create();
   }

   else if (parentWidget) {
      q->setParent(parentWidget, data.window_flags);
   }

   else {
      adjustFlags(data.window_flags, q);
      resolveLayoutDirection();

      // opaque system background?
      const QBrush &background = q->palette().brush(QPalette::Window);
      setOpaque(q->isWindow() && background.style() != Qt::NoBrush && background.isOpaque());
   }

   data.fnt = QFont(data.fnt, q);

#if defined(Q_WS_X11)
   data.fnt.x11SetScreen(xinfo.screen());
#endif

   q->setAttribute(Qt::WA_PendingMoveEvent);
   q->setAttribute(Qt::WA_PendingResizeEvent);

   if (++QWidgetPrivate::instanceCounter > QWidgetPrivate::maxInstances) {
      QWidgetPrivate::maxInstances = QWidgetPrivate::instanceCounter;
   }

   QEvent e(QEvent::Create);
   QApplication::sendEvent(q, &e);
   QApplication::postEvent(q, new QEvent(QEvent::PolishRequest));

   extraPaintEngine = 0;
}

void QWidgetPrivate::createRecursively()
{
   Q_Q(QWidget);
   q->create(0, true, true);

   for (int i = 0; i < q->children().size(); ++i) {
      QWidget *child = qobject_cast<QWidget *>(q->children().at(i));

      if (child && !child->isHidden() && !child->isWindow() && !child->testAttribute(Qt::WA_WState_Created)) {
         child->d_func()->createRecursively();
      }
   }
}

void QWidget::create(WId window, bool initializeWindow, bool destroyOldWindow)
{
   Q_D(QWidget);

   if (testAttribute(Qt::WA_WState_Created) && window == 0 && internalWinId()) {
      return;
   }

   if (d->data.in_destructor) {
      return;
   }

   Qt::WindowType type = windowType();
   Qt::WindowFlags &flags = data->window_flags;

   if ((type == Qt::Widget || type == Qt::SubWindow) && ! parentWidget()) {
      type = Qt::Window;
      flags |= Qt::Window;
   }

#ifndef Q_WS_QPA
   if (QWidget *parent = parentWidget()) {

      if (type & Qt::Window) {
         if (! parent->testAttribute(Qt::WA_WState_Created)) {
            parent->createWinId();
         }

      } else if (testAttribute(Qt::WA_NativeWindow) && !parent->internalWinId()
                 && !testAttribute(Qt::WA_DontCreateNativeAncestors)) {
         // We're about to create a native child widget that doesn't have a native parent;
         // enforce a native handle for the parent unless the Qt::WA_DontCreateNativeAncestors
         // attribute is set.
         d->createWinId(window);

         // Nothing more to do.
         Q_ASSERT(testAttribute(Qt::WA_WState_Created));
         Q_ASSERT(internalWinId());
         return;
      }
   }
#endif

   static int paintOnScreenEnv = -1;
   if (paintOnScreenEnv == -1) {
      paintOnScreenEnv = qgetenv("QT_ONSCREEN_PAINT").toInt() > 0 ? 1 : 0;
   }
   if (paintOnScreenEnv == 1) {
      setAttribute(Qt::WA_PaintOnScreen);
   }

   if (QApplicationPrivate::testAttribute(Qt::AA_NativeWindows)) {
      setAttribute(Qt::WA_NativeWindow);
   }

#ifdef ALIEN_DEBUG
   qDebug() << "QWidget::create:" << this << "parent:" << parentWidget()
            << "Alien?" << !testAttribute(Qt::WA_NativeWindow);
#endif

#if defined (Q_OS_WIN) && !defined(QT_NO_DRAGANDDROP)
   // Unregister the dropsite (if already registered) before we
   // re-create the widget with a native window.
   if (testAttribute(Qt::WA_WState_Created) && !internalWinId() && testAttribute(Qt::WA_NativeWindow)
         && d->extra && d->extra->dropTarget) {
      d->registerDropSite(false);
   }
#endif

   d->updateIsOpaque();

   setAttribute(Qt::WA_WState_Created);                        // set created flag
   d->create_sys(window, initializeWindow, destroyOldWindow);

   // a real toplevel window needs a backing store
   if (isWindow() && windowType() != Qt::Desktop) {
      d->topData()->backingStore.destroy();
      if (hasBackingStoreSupport()) {
         d->topData()->backingStore.create(this);
      }
   }

   d->setModal_sys();

   if (!isWindow() && parentWidget() && parentWidget()->testAttribute(Qt::WA_DropSiteRegistered)) {
      setAttribute(Qt::WA_DropSiteRegistered, true);
   }

#ifdef QT_EVAL
   extern void qt_eval_init_widget(QWidget * w);
   qt_eval_init_widget(this);
#endif

   // need to force the resting of the icon after changing parents
   if (testAttribute(Qt::WA_SetWindowIcon)) {
      d->setWindowIcon_sys(true);
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
}

QWidget::~QWidget()
{
   Q_D(QWidget);

   d->data.in_destructor = true;

#if defined (QT_CHECK_STATE)
   if (paintingActive()) {
      qWarning("QWidget: %s (%s) deleted while being painted", className(), name());
   }
#endif

#ifndef QT_NO_GESTURES
   for (Qt::GestureType type : d->gestureContext.keys())
   ungrabGesture(type);
#endif

   // force acceptDrops false before winId is destroyed
   d->registerDropSite(false);

#ifndef QT_NO_ACTION
   // remove all actions from this widget
   for (int i = 0; i < d->actions.size(); ++i) {
      QActionPrivate *apriv = d->actions.at(i)->d_func();
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

   // CopperSpice - ensure parent is notified while 'this' is still a widget
   QObject::cs_forceRemoveChild();

   // delete layout while we still are a valid widget
   delete d->layout;
   d->layout = 0;

   // Remove myself from focus list
   Q_ASSERT(d->focus_next->d_func()->focus_prev == this);
   Q_ASSERT(d->focus_prev->d_func()->focus_next == this);

   if (d->focus_next != this) {
      d->focus_next->d_func()->focus_prev = d->focus_prev;
      d->focus_prev->d_func()->focus_next = d->focus_next;
      d->focus_next = d->focus_prev = 0;
   }

   QT_TRY {
      clearFocus();

   } QT_CATCH(...) {
      // ignore this problem because we are in a destructor
   }

   d->setDirtyOpaqueRegion();

   if (isWindow() && isVisible() && internalWinId()) {
      QT_TRY {
         d->close_helper(QWidgetPrivate::CloseNoEvent);

      } QT_CATCH(...) {
         // if we are out of memory, at least hide the window
         QT_TRY {
            hide();
         } QT_CATCH(...) {
            // and if that also does not work, then give up
         }
      }
   }

#if defined(Q_OS_WIN) || defined(Q_WS_X11)|| defined(Q_OS_MAC)
   else if (!internalWinId() && isVisible()) {
      qApp->d_func()->sendSyntheticEnterLeave(this);
   }

#elif defined(Q_WS_QWS) || defined(Q_WS_QPA)
   else if (isVisible()) {
      qApp->d_func()->sendSyntheticEnterLeave(this);
   }
#endif

   if (QWidgetBackingStore *bs = d->maybeBackingStore()) {
      bs->removeDirtyWidget(this);
      if (testAttribute(Qt::WA_StaticContents)) {
         bs->removeStaticWidget(this);
      }
   }

   delete d->needsFlush;
   d->needsFlush = 0;

   // used by declarative library
   CSAbstractDeclarativeData *tmp_Data = CSInternalDeclarativeData::get_m_declarativeData(this);

   if (tmp_Data) {
      CSAbstractDeclarativeData::destroyed(tmp_Data, this);
      CSInternalDeclarativeData::set_m_declarativeData(this, 0);
   }

#ifdef Q_OS_MAC
   // QCocoaView holds a pointer back to this widget. Clear it now
   // to make sure it's not followed later on. The lifetime of the
   // QCocoaView might exceed the lifetime of this widget in cases
   // where Cocoa itself holds references to it.
   extern void qt_mac_clearCocoaViewQWidgetPointers(QWidget *);
   qt_mac_clearCocoaViewQWidgetPointers(this);
#endif

   CSInternalChildren::deleteChildren(this);

#ifndef QT_NO_ACCESSIBILITY
   QAccessible::updateAccessibility(this, 0, QAccessible::ObjectDestroyed);
#endif

   QApplication::removePostedEvents(this);

   QT_TRY {
      destroy();                                        // platform-dependent cleanup
   } QT_CATCH(...) {
      // if this fails we can not do anything
   }
   --QWidgetPrivate::instanceCounter;

   // might have been deleted by ~QApplication
   if (QWidgetPrivate::allWidgets) {
      QWidgetPrivate::allWidgets->remove(this);
   }

   QT_TRY {
      QEvent e(QEvent::Destroy);
      QCoreApplication::sendEvent(this, &e);

   } QT_CATCH(const std::exception &) {
      // if this fails we ca not do anything
   }
}

bool QWidget::cs_isWidgetType() const
{
   return true;
}

int QWidgetPrivate::instanceCounter = 0;  // Current number of widget instances
int QWidgetPrivate::maxInstances = 0;     // Maximum number of widget instances

void QWidgetPrivate::setWinId(WId id)                // set widget identifier
{
   Q_Q(QWidget);
   // the user might create a widget with Qt::Desktop window
   // attribute (or create another QDesktopWidget instance), which
   // will have the same windowid (the root window id) as the
   // qt_desktopWidget. We should not add the second desktop widget
   // to the mapper.
   bool userDesktopWidget = qt_desktopWidget != 0 && qt_desktopWidget != q && q->windowType() == Qt::Desktop;
   if (mapper && data.winid && !userDesktopWidget) {
      mapper->remove(data.winid);
   }

   const WId oldWinId = data.winid;

   data.winid = id;

#if defined(Q_WS_X11)
   hd = id;
#endif

   if (mapper && id && !userDesktopWidget) {
      mapper->insert(data.winid, q);
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
      x->icon = 0;
      x->iconPixmap = 0;
      x->windowSurface = 0;
      x->sharedPainter = 0;
      x->incw = x->inch = 0;
      x->basew = x->baseh = 0;
      x->frameStrut.setCoords(0, 0, 0, 0);
      x->normalGeometry = QRect(0, 0, -1, -1);
      x->savedFlags = 0;
      x->opacity = 255;
      x->posFromMove = false;
      x->sizeAdjusted = false;
      x->inTopLevelResize = false;
      x->inRepaint = false;
      x->embedded = 0;

#if defined(Q_OS_MAC)
      x->wasMaximized = false;
#endif

      createTLSysExtra();

#ifdef QWIDGET_EXTRA_DEBUG
      static int count = 0;
      qDebug() << "tlextra" << ++count;
#endif

#if defined(Q_WS_QPA)
      x->platformWindow = 0;
      x->platformWindowFormat = QPlatformWindowFormat::defaultFormat();
      x->screenIndex = 0;
#endif
   }
}

// internal
void QWidgetPrivate::createExtra()
{
   if (!extra) {                                // if not exists
      extra = new QWExtra;
      extra->glContext = 0;
      extra->topextra = 0;

#ifndef QT_NO_GRAPHICSVIEW
      extra->proxyWidget = 0;
#endif

#ifndef QT_NO_CURSOR
      extra->curs = 0;
#endif

      extra->minw = 0;
      extra->minh = 0;
      extra->maxw = QWIDGETSIZE_MAX;
      extra->maxh = QWIDGETSIZE_MAX;
      extra->customDpiX = 0;
      extra->customDpiY = 0;
      extra->explicitMinSize = 0;
      extra->explicitMaxSize = 0;
      extra->autoFillBackground = 0;
      extra->nativeChildrenForced = 0;
      extra->inRenderWithPainter = 0;
      extra->hasMask = 0;
      createSysExtra();

#ifdef QWIDGET_EXTRA_DEBUG
      static int count = 0;
      qDebug() << "extra" << ++count;
#endif

   }
}

// internal
void QWidgetPrivate::deleteExtra()
{
   if (extra) {                                // if exists

#ifndef QT_NO_CURSOR
      delete extra->curs;
#endif

      deleteSysExtra();

#ifndef QT_NO_STYLE_STYLESHEET
      // dereference the stylesheet style
      if (QStyleSheetStyle *proxy = qobject_cast<QStyleSheetStyle *>(extra->style)) {
         proxy->deref();
      }
#endif

      if (extra->topextra) {
         deleteTLSysExtra();
         extra->topextra->backingStore.destroy();
         delete extra->topextra->icon;
         delete extra->topextra->iconPixmap;

#if defined(Q_WS_QWS) && !defined(QT_NO_QWS_MANAGER)
         delete extra->topextra->qwsManager;
#endif

         delete extra->topextra->windowSurface;
         delete extra->topextra;
      }

      delete extra;
      // extra->xic destroyed in QWidget::destroy()
      extra = 0;
   }
}

bool QWidgetPrivate::isOverlapped(const QRect &rect) const
{
   Q_Q(const QWidget);

   const QWidget *w = q;
   QRect r = rect;

   while (w) {
      if (w->isWindow()) {
         return false;
      }

      QWidget *parent = w->parentWidget();

      QWidgetPrivate *pd = parent->d_func();
      bool above = false;

      for (int i = 0; i < parent->children().size(); ++i) {
         QWidget *sibling = qobject_cast<QWidget *>(parent->children().at(i));

         if (!sibling || !sibling->isVisible() || sibling->isWindow()) {
            continue;
         }

         if (!above) {
            above = (sibling == w);
            continue;
         }

         if (qRectIntersects(sibling->d_func()->effectiveRectFor(sibling->data->crect), r)) {
            const QWExtra *siblingExtra = sibling->d_func()->extra;

            if (siblingExtra && siblingExtra->hasMask && !sibling->d_func()->graphicsEffect
                  && !siblingExtra->mask.translated(sibling->data->crect.topLeft()).intersects(r)) {
               continue;
            }
            return true;
         }
      }
      w = w->parentWidget();
      r.translate(pd->data.crect.topLeft());
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

   if (enable && !q->isWindow() && q->parentWidget() && !q->parentWidget()->updatesEnabled()) {
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

   for (int i = 0; i < q->children().size(); ++i) {
      QWidget *w = qobject_cast<QWidget *>(q->children().at(i));

      if (w && !w->isWindow() && !w->testAttribute(attribute)) {
         w->d_func()->setUpdatesEnabled_helper(enable);
      }
   }
}

// internal
void QWidgetPrivate::propagatePaletteChange()
{
   Q_Q(QWidget);

   // Propagate a new inherited mask to all children.

#ifndef QT_NO_GRAPHICSVIEW
   if (!q->parentWidget() && extra && extra->proxyWidget) {
      QGraphicsProxyWidget *p = extra->proxyWidget;
      inheritedPaletteResolveMask = p->d_func()->inheritedPaletteResolveMask | p->palette().resolve();
   } else
#endif

      if (q->isWindow() && !q->testAttribute(Qt::WA_WindowPropagation)) {
         inheritedPaletteResolveMask = 0;
      }

   int mask = data.pal.resolve() | inheritedPaletteResolveMask;

   QEvent pc(QEvent::PaletteChange);
   QApplication::sendEvent(q, &pc);

   for (int i = 0; i < q->children().size(); ++i) {
      QWidget *w = qobject_cast<QWidget *>(q->children().at(i));

      if (w && !w->testAttribute(Qt::WA_StyleSheet)
            && (!w->isWindow() || w->testAttribute(Qt::WA_WindowPropagation))) {
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

   if (!q->isVisible()) {
      return QRegion();
   }

   QRegion r(q->rect());
   const QWidget *w = q;
   const QWidget *ignoreUpTo;
   int ox = 0;
   int oy = 0;

   while (w && w->isVisible() && !w->isWindow() && w->parentWidget()) {
      ox -= w->x();
      oy -= w->y();
      ignoreUpTo = w;
      w = w->parentWidget();
      r &= QRegion(ox, oy, w->width(), w->height());

      int i = 0;
      while (w->children().at(i++) != static_cast<const QObject *>(ignoreUpTo)) {
      }

      while ( i < w->children().size()) {
         if (QWidget *sibling = qobject_cast<QWidget *>(w->children().at(i))) {
            if (sibling->isVisible() && !sibling->isWindow()) {

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

#ifndef QT_NO_GRAPHICSEFFECT
void QWidgetPrivate::invalidateGraphicsEffectsRecursively()
{
   Q_Q(QWidget);
   QWidget *w = q;
   do {
      if (w->graphicsEffect()) {
         QWidgetEffectSourcePrivate *sourced =
            static_cast<QWidgetEffectSourcePrivate *>(w->graphicsEffect()->source()->d_func());
         if (!sourced->updateDueToGraphicsEffect) {
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
   if (!parent) {
      return;
   }

   // TODO: instead of setting dirtyflag, manipulate the dirtyregion directly?
   QWidgetPrivate *pd = parent->d_func();
   if (!pd->dirtyOpaqueChildren) {
      pd->setDirtyOpaqueRegion();
   }
}

const QRegion &QWidgetPrivate::getOpaqueChildren() const
{
   Q_Q(const QWidget);

   if (!dirtyOpaqueChildren) {
      return opaqueChildren;
   }

   QWidgetPrivate *that = const_cast<QWidgetPrivate *>(this);
   that->opaqueChildren = QRegion();

   for (int i = 0; i < q->children().size(); ++i) {
      QWidget *child = qobject_cast<QWidget *>(q->children().at(i));
      if (!child || !child->isVisible() || child->isWindow()) {
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
   if (!r.isEmpty()) {
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

#ifdef Q_OS_MAC
   if (q->d_func()->isInUnifiedToolbar) {
      return;
   }
#endif

   QRect clipBoundingRect;
   bool dirtyClipBoundingRect = true;

   QRegion parentClip;
   bool dirtyParentClip = true;

   QPoint parentOffset = data.crect.topLeft();

   const QWidget *w = q;

   while (w) {
      if (w->isWindow()) {
         break;
      }

      QWidget *parent = w->parentWidget();
      QWidgetPrivate *pd = parent->d_func();

      const int myIndex = parent->children().indexOf(const_cast<QWidget *>(w));
      const QRect widgetGeometry = w->d_func()->effectiveRectFor(w->data->crect);

      for (int i = myIndex + 1; i < parent->children().size(); ++i) {
         QWidget *sibling = qobject_cast<QWidget *>(parent->children().at(i));

         if (!sibling || !sibling->isVisible() || sibling->isWindow()) {
            continue;
         }

         const QRect siblingGeometry = sibling->d_func()->effectiveRectFor(sibling->data->crect);
         if (!qRectIntersects(siblingGeometry, widgetGeometry)) {
            continue;
         }

         if (dirtyClipBoundingRect) {
            clipBoundingRect = sourceRegion.boundingRect();
            dirtyClipBoundingRect = false;
         }

         if (!qRectIntersects(siblingGeometry, clipBoundingRect.translated(parentOffset))) {
            continue;
         }

         if (dirtyParentClip) {
            parentClip = sourceRegion.translated(parentOffset);
            dirtyParentClip = false;
         }

         const QPoint siblingPos(sibling->data->crect.topLeft());
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
      parentOffset += pd->data.crect.topLeft();
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
      offset -= data.crect.topLeft();
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
      offset -= wd->data.crect.topLeft();
      w = w->parentWidget();
   }
}

bool QWidgetPrivate::paintOnScreen() const
{

#if defined(Q_WS_QWS)
   return false;
#elif  defined(QT_NO_BACKINGSTORE)
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
   // hw: todo: only needed if opacity actually changed
   setDirtyOpaqueRegion();

#ifndef QT_NO_GRAPHICSEFFECT
   if (graphicsEffect) {
      // ### We should probably add QGraphicsEffect::isOpaque at some point.
      setOpaque(false);
      return;
   }
#endif

   Q_Q(QWidget);

#ifdef Q_WS_X11
   if (q->testAttribute(Qt::WA_X11OpenGLOverlay)) {
      setOpaque(false);
      return;
   }
#endif

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
   if (isOpaque == opaque) {
      return;
   }
   isOpaque = opaque;

#ifdef Q_OS_MAC
   macUpdateIsOpaque();
#endif

#ifdef Q_WS_X11
   x11UpdateIsOpaque();
#endif

#ifdef Q_OS_WIN
   winUpdateIsOpaque();
#endif

}

void QWidgetPrivate::updateIsTranslucent()
{

#ifdef Q_OS_MAC
   macUpdateIsOpaque();
#endif

#ifdef Q_WS_X11
   x11UpdateIsOpaque();
#endif

#ifdef Q_OS_WIN
   winUpdateIsOpaque();
#endif

}

void QPixmap::fill( const QWidget *widget, const QPoint &off )
{
   QPainter p(this);
   p.translate(-off);
   widget->d_func()->paintBackground(&p, QRect(off, size()));
}

static inline void fillRegion(QPainter *painter, const QRegion &rgn, const QBrush &brush)
{
   Q_ASSERT(painter);

   if (brush.style() == Qt::TexturePattern) {

#ifdef Q_OS_MAC
      // Optimize pattern filling on mac by using HITheme directly
      // when filling with the standard widget background.
      // Defined in qmacstyle_mac.cpp
      extern void qt_mac_fill_background(QPainter * painter, const QRegion & rgn, const QBrush & brush);
      qt_mac_fill_background(painter, rgn, brush);
#else
      {
         const QRect rect(rgn.boundingRect());
         painter->setClipRegion(rgn);
         painter->drawTiledPixmap(rect, brush.texture(), rect.topLeft());
      }
#endif

   } else if (brush.gradient()
              && brush.gradient()->coordinateMode() == QGradient::ObjectBoundingMode) {
      painter->save();
      painter->setClipRegion(rgn);
      painter->fillRect(0, 0, painter->device()->width(), painter->device()->height(), brush);
      painter->restore();

   } else {
      const QVector<QRect> &rects = rgn.rects();
      for (int i = 0; i < rects.size(); ++i) {
         painter->fillRect(rects.at(i), brush);
      }
   }
}

void QWidgetPrivate::paintBackground(QPainter *painter, const QRegion &rgn, int flags) const
{
   Q_Q(const QWidget);

#ifndef QT_NO_SCROLLAREA
   bool resetBrushOrigin = false;
   QPointF oldBrushOrigin;

   //If we are painting the viewport of a scrollarea, we must apply an offset to the brush in case we are drawing a texture
   QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea *>(q->parent());

   if (scrollArea && scrollArea->viewport() == q) {
      QWidgetPrivate *scrollPrivate = static_cast<QWidget *>(scrollArea)->d_ptr.data();
      QAbstractScrollAreaPrivate *priv = static_cast<QAbstractScrollAreaPrivate *>(scrollPrivate);

      oldBrushOrigin = painter->brushOrigin();
      resetBrushOrigin = true;
      painter->setBrushOrigin(-priv->contentsOffset());

   }
#endif

   const QBrush autoFillBrush = q->palette().brush(q->backgroundRole());

   if ((flags & DrawAsRoot) && !(q->autoFillBackground() && autoFillBrush.isOpaque())) {
      const QBrush bg = q->palette().brush(QPalette::Window);

#if defined(Q_WS_QWS) || defined(Q_WS_QPA)
      if (!(flags & DontSetCompositionMode)) {

         //copy alpha straight in
         QPainter::CompositionMode oldMode = painter->compositionMode();
         painter->setCompositionMode(QPainter::CompositionMode_Source);
         fillRegion(painter, rgn, bg);
         painter->setCompositionMode(oldMode);
      } else {
         fillRegion(painter, rgn, bg);
      }
#else
      fillRegion(painter, rgn, bg);
#endif

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

// internal
extern QWidget *qt_button_down;

void QWidgetPrivate::deactivateWidgetCleanup()
{
   Q_Q(QWidget);

   // If this was the active application window, reset it
   if (QApplication::activeWindow() == q) {
      QApplication::setActiveWindow(0);
   }

   // If the is the active mouse press widget, reset it
   if (q == qt_button_down) {
      qt_button_down = 0;
   }
}

QWidget *QWidget::find(WId id)
{
   return QWidgetPrivate::mapper ? QWidgetPrivate::mapper->value(id, 0) : 0;
}

WId QWidget::winId() const
{
   if (!testAttribute(Qt::WA_WState_Created) || !internalWinId()) {

#ifdef ALIEN_DEBUG
      qDebug() << "QWidget::winId: creating native window for" << this;
#endif

      QWidget *that = const_cast<QWidget *>(this);

#ifndef Q_WS_QPA
      that->setAttribute(Qt::WA_NativeWindow);
#endif

      that->d_func()->createWinId();
      return that->data->winid;
   }

   return data->winid;
}


void QWidgetPrivate::createWinId(WId winid)
{
   Q_Q(QWidget);

#ifdef ALIEN_DEBUG
   qDebug() << "QWidgetPrivate::createWinId for" << q << winid;
#endif

   const bool forceNativeWindow = q->testAttribute(Qt::WA_NativeWindow);
   if (!q->testAttribute(Qt::WA_WState_Created) || (forceNativeWindow && !q->internalWinId())) {

#ifndef Q_WS_QPA
      if (!q->isWindow()) {

         QWidget *parent = q->parentWidget();
         QWidgetPrivate *pd = parent->d_func();

         if (forceNativeWindow && !q->testAttribute(Qt::WA_DontCreateNativeAncestors)) {
            parent->setAttribute(Qt::WA_NativeWindow);
         }
         if (!parent->internalWinId()) {
            pd->createWinId();
         }

         for (int i = 0; i < parent->children().size(); ++i) {
            QWidget *w = qobject_cast<QWidget *>(parent->children().at(i));

            if (w && !w->isWindow() && (!w->testAttribute(Qt::WA_WState_Created)
                                        || (!w->internalWinId() && w->testAttribute(Qt::WA_NativeWindow)))) {
               if (w != q) {
                  w->create();

               } else {
                  w->create(winid);
                  // if the window has already been created, we
                  // need to raise it to its proper stacking position
                  if (winid) {
                     w->raise();
                  }
               }
            }
         }
      } else {
         q->create();
      }
#else
      Q_UNUSED(winid);
      q->create();
#endif

   }
}

// internal
void QWidget::createWinId()
{
   Q_D(QWidget);

#ifdef ALIEN_DEBUG
   qDebug()  << "QWidget::createWinId" << this;
#endif

   d->createWinId();
}

WId QWidget::effectiveWinId() const
{
   WId id = internalWinId();

   if (id || !testAttribute(Qt::WA_WState_Created)) {
      return id;
   }

   QWidget *realParent = nativeParentWidget();

   if (! realParent && d_func()->inSetParent) {

      // In transitional state. This is really just a workaround. The real problem
      // is that QWidgetPrivate::setParent_sys (platform specific code) first sets
      // the window id to 0 (setWinId(0)) before it sets the Qt::WA_WState_Created
      // attribute to false. The correct way is to do it the other way around, and
      // in that case the Qt::WA_WState_Created logic above will kick in and
      // return 0 whenever the widget is in a transitional state. However, changing
      // the original logic for all platforms is far more intrusive and might
      // break existing applications.
      // Note: The widget can only be in a transitional state when changing its
      // parent -- everything else is an internal error -- hence explicitly checking
      // against 'inSetParent' rather than doing an unconditional return whenever
      // 'realParent' is 0 (which may cause strange artifacts and headache later).

      return 0;
   }

   // This widget *must* have a native parent widget.
   Q_ASSERT(realParent);
   Q_ASSERT(realParent->internalWinId());

   return realParent->internalWinId();
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
   d->createExtra();

   QStyleSheetStyle *proxy = qobject_cast<QStyleSheetStyle *>(d->extra->style);
   d->extra->styleSheet = styleSheet;
   if (styleSheet.isEmpty()) { // stylesheet removed
      if (!proxy) {
         return;
      }

      d->inheritStyle();
      return;
   }

   if (proxy) { // style sheet update
      proxy->repolish(this);
      return;
   }

   if (testAttribute(Qt::WA_SetStyle)) {
      d->setStyle_helper(new QStyleSheetStyle(d->extra->style), true);
   } else {
      d->setStyle_helper(new QStyleSheetStyle(0), true);
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

   setAttribute(Qt::WA_SetStyle, style != 0);
   d->createExtra();

#ifndef QT_NO_STYLE_STYLESHEET
   if (QStyleSheetStyle *proxy = qobject_cast<QStyleSheetStyle *>(style)) {

      //if for some reason someone try to set a QStyleSheetStyle, ref it
      //(this may happen for exemple in QButtonDialogBox which propagates its style)

      proxy->ref();
      d->setStyle_helper(style, false);

   } else if (qobject_cast<QStyleSheetStyle *>(d->extra->style) || !qApp->styleSheet().isEmpty()) {
      // if we have an application stylesheet or have a proxy already, propagate
      d->setStyle_helper(new QStyleSheetStyle(style), true);

   } else
#endif

      d->setStyle_helper(style, false);
}

void QWidgetPrivate::setStyle_helper(QStyle *newStyle, bool propagate, bool

#ifdef Q_OS_MAC
                                     metalHack
#endif
                                    )
{
   Q_Q(QWidget);
   QStyle *oldStyle  = q->style();

#ifndef QT_NO_STYLE_STYLESHEET
   QWeakPointer<QStyle> origStyle;
#endif

#ifdef Q_OS_MAC
   // the metalhack boolean allows Qt/Mac to do a proper re-polish depending
   // on how the Qt::WA_MacBrushedMetal attribute is set. It is only ever
   // set when changing that attribute and passes the widget's CURRENT style.
   // therefore no need to do a reassignment.
   if (!metalHack)
#endif

   {
      createExtra();

#ifndef QT_NO_STYLE_STYLESHEET
      origStyle = extra->style.data();
#endif
      extra->style = newStyle;
   }

   // repolish
   if (q->windowType() != Qt::Desktop) {
      if (polished) {
         oldStyle->unpolish(q);
#ifdef Q_OS_MAC
         if (metalHack) {
            macUpdateMetalAttribute();
         }
#endif
         q->style()->polish(q);

#ifdef Q_OS_MAC
      } else if (metalHack) {
         macUpdateMetalAttribute();
#endif
      }
   }

   if (propagate) {
      for (int i = 0; i < q->children().size(); ++i) {
         QWidget *c = qobject_cast<QWidget *>(q->children().at(i));

         if (c) {
            c->d_func()->inheritStyle();
         }
      }
   }

#ifndef QT_NO_STYLE_STYLESHEET
   if (!qobject_cast<QStyleSheetStyle *>(newStyle)) {
      if (const QStyleSheetStyle *cssStyle = qobject_cast<QStyleSheetStyle *>(origStyle.data())) {
         cssStyle->clearWidgetFont(q);
      }
   }
#endif

   QEvent e(QEvent::StyleChange);
   QApplication::sendEvent(q, &e);

#ifndef QT_NO_STYLE_STYLESHEET
   // dereference the old stylesheet style
   if (QStyleSheetStyle *proxy = qobject_cast<QStyleSheetStyle *>(origStyle.data())) {
      proxy->deref();
   }
#endif
}

// Inherits style from the current parent and propagates it as necessary
void QWidgetPrivate::inheritStyle()
{
#ifndef QT_NO_STYLE_STYLESHEET
   Q_Q(QWidget);

   QStyleSheetStyle *proxy = extra ? qobject_cast<QStyleSheetStyle *>(extra->style) : 0;

   if (!q->styleSheet().isEmpty()) {
      Q_ASSERT(proxy);
      proxy->repolish(q);
      return;
   }

   QStyle *origStyle = proxy ? proxy->base : (extra ? (QStyle *)extra->style : 0);
   QWidget *parent = q->parentWidget();
   QStyle *parentStyle = (parent && parent->d_func()->extra) ? (QStyle *)parent->d_func()->extra->style : 0;

   // If we have stylesheet on app or parent has stylesheet style, we need
   // to be running a proxy
   if (!qApp->styleSheet().isEmpty() || qobject_cast<QStyleSheetStyle *>(parentStyle)) {
      QStyle *newStyle = parentStyle;
      if (q->testAttribute(Qt::WA_SetStyle)) {
         newStyle = new QStyleSheetStyle(origStyle);
      } else if (QStyleSheetStyle *newProxy = qobject_cast<QStyleSheetStyle *>(parentStyle)) {
         newProxy->ref();
      }

      setStyle_helper(newStyle, true);
      return;
   }

   // So, we have no stylesheet on parent/app and we have an empty stylesheet
   // we just need our original style back
   if (origStyle == (extra ? (QStyle *)extra->style : 0)) { // is it any different?
      return;
   }

   // We could have inherited the proxy from our parent (which has a custom style)
   // In such a case we need to start following the application style (i.e revert
   // the propagation behavior of QStyleSheetStyle)
   if (!q->testAttribute(Qt::WA_SetStyle)) {
      origStyle = 0;
   }

   setStyle_helper(origStyle, true);
#endif // QT_NO_STYLE_STYLESHEET
}

Qt::WindowModality QWidget::windowModality() const
{
   return static_cast<Qt::WindowModality>(data->window_modality);
}

void QWidget::setWindowModality(Qt::WindowModality windowModality)
{
   data->window_modality = windowModality;
   // setModal_sys() will be called by setAttribute()
   setAttribute(Qt::WA_ShowModal, (data->window_modality != Qt::NonModal));
   setAttribute(Qt::WA_SetWindowModality, true);
}

bool QWidget::isMinimized() const
{
   return data->window_state & Qt::WindowMinimized;
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
   show();
}

bool QWidget::isMaximized() const
{
   return data->window_state & Qt::WindowMaximized;
}


Qt::WindowStates QWidget::windowState() const
{
   return Qt::WindowStates(data->window_state);
}

// internal
void QWidget::overrideWindowState(Qt::WindowStates newstate)
{
   QWindowStateChangeEvent e(Qt::WindowStates(data->window_state), true);
   data->window_state  = newstate;
   QApplication::sendEvent(this, &e);
}

bool QWidget::isFullScreen() const
{
   return data->window_state & Qt::WindowFullScreen;
}

void QWidget::showFullScreen()
{

#ifdef Q_OS_MAC
   // If the unified toolbar is enabled, we have to disable it before going fullscreen.
   QMainWindow *mainWindow = qobject_cast<QMainWindow *>(this);
   if (mainWindow && mainWindow->unifiedTitleAndToolBarOnMac()) {
      mainWindow->setUnifiedTitleAndToolBarOnMac(false);
      QMainWindowLayout *mainLayout = qobject_cast<QMainWindowLayout *>(mainWindow->layout());
      mainLayout->activateUnifiedToolbarAfterFullScreen = true;
   }
#endif
   ensurePolished();

   setWindowState((windowState() & ~(Qt::WindowMinimized | Qt::WindowMaximized))
                  | Qt::WindowFullScreen);
   show();
   activateWindow();
}

void QWidget::showMaximized()
{
   ensurePolished();

   setWindowState((windowState() & ~(Qt::WindowMinimized | Qt::WindowFullScreen))
                  | Qt::WindowMaximized);
#ifdef Q_OS_MAC
   // If the unified toolbar was enabled before going fullscreen, we have to enable it back.
   QMainWindow *mainWindow = qobject_cast<QMainWindow *>(this);
   if (mainWindow) {
      QMainWindowLayout *mainLayout = qobject_cast<QMainWindowLayout *>(mainWindow->layout());
      if (mainLayout->activateUnifiedToolbarAfterFullScreen) {
         mainWindow->setUnifiedTitleAndToolBarOnMac(true);
         mainLayout->activateUnifiedToolbarAfterFullScreen = false;
      }
   }
#endif
   show();
}

void QWidget::showNormal()
{
   ensurePolished();
   setWindowState(windowState() & ~(Qt::WindowMinimized | Qt::WindowMaximized | Qt::WindowFullScreen));

#ifdef Q_OS_MAC
   // If the unified toolbar was enabled before going fullscreen, we have to enable it back.
   QMainWindow *mainWindow = qobject_cast<QMainWindow *>(this);
   if (mainWindow) {
      QMainWindowLayout *mainLayout = qobject_cast<QMainWindowLayout *>(mainWindow->layout());
      if (mainLayout->activateUnifiedToolbarAfterFullScreen) {
         mainWindow->setUnifiedTitleAndToolBarOnMac(true);
         mainLayout->activateUnifiedToolbarAfterFullScreen = false;
      }
   }
#endif
   show();
}

bool QWidget::isEnabledTo(QWidget *ancestor) const
{
   const QWidget *w = this;
   while (!w->testAttribute(Qt::WA_ForceDisabled)
          && !w->isWindow()
          && w->parentWidget()
          && w->parentWidget() != ancestor) {
      w = w->parentWidget();
   }
   return !w->testAttribute(Qt::WA_ForceDisabled);
}

#ifndef QT_NO_ACTION

void QWidget::addAction(QAction *action)
{
   insertAction(0, action);
}

void QWidget::addActions(QList<QAction *> actions)
{
   for (int i = 0; i < actions.count(); i++) {
      insertAction(0, actions.at(i));
   }
}

void QWidget::insertAction(QAction *before, QAction *action)
{
   if (!action) {
      qWarning("QWidget::insertAction: Attempt to insert null action");
      return;
   }

   Q_D(QWidget);
   if (d->actions.contains(action)) {
      removeAction(action);
   }

   int pos = d->actions.indexOf(before);
   if (pos < 0) {
      before = 0;
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
   for (int i = 0; i < actions.count(); ++i) {
      insertAction(before, actions.at(i));
   }
}

void QWidget::removeAction(QAction *action)
{
   if (!action) {
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

   if (enable && !q->isWindow() && q->parentWidget() && !q->parentWidget()->isEnabled()) {
      return;   // nothing we can do
   }

   if (enable != q->testAttribute(Qt::WA_Disabled)) {
      return;   // nothing to do
   }

   q->setAttribute(Qt::WA_Disabled, !enable);
   updateSystemBackground();

   if (!enable && q->window()->focusWidget() == q) {
      bool parentIsEnabled = (!q->parentWidget() || q->parentWidget()->isEnabled());
      if (!parentIsEnabled || !q->focusNextChild()) {
         q->clearFocus();
      }
   }

   Qt::WidgetAttribute attribute = enable ? Qt::WA_ForceDisabled : Qt::WA_Disabled;
   for (int i = 0; i < q->children().size(); ++i) {
      QWidget *w = qobject_cast<QWidget *>(q->children().at(i));

      if (w && !w->testAttribute(attribute)) {
         w->d_func()->setEnabled_helper(enable);
      }
   }

#if defined(Q_WS_X11)
   if (q->testAttribute(Qt::WA_SetCursor) || q->isWindow()) {
      // enforce the windows behavior of clearing the cursor on
      // disabled widgets
      qt_x11_enforce_cursor(q);
   }
#endif

#if defined(Q_OS_MAC)
   setEnabled_helper_sys(enable);
#endif

#ifndef QT_NO_IM
   if (q->testAttribute(Qt::WA_InputMethodEnabled) && q->hasFocus()) {
      QWidget *focusWidget = effectiveFocusWidget();
      QInputContext *qic = focusWidget->d_func()->inputContext();
      if (enable) {
         if (focusWidget->testAttribute(Qt::WA_InputMethodEnabled)) {
            qic->setFocusWidget(focusWidget);
         }
      } else {
         qic->reset();
         qic->setFocusWidget(0);
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

void QWidget::setDisabled(bool disable)
{
   setEnabled(!disable);
}

QRect QWidget::frameGeometry() const
{
   Q_D(const QWidget);

   if (isWindow() && ! (windowType() == Qt::Popup)) {
      QRect fs = d->frameStrut();

      return QRect(data->crect.x() - fs.left(),
                   data->crect.y() - fs.top(),
                   data->crect.width() + fs.left() + fs.right(),
                   data->crect.height() + fs.top() + fs.bottom());
   }

   return data->crect;
}

int QWidget::x() const
{
   Q_D(const QWidget);
   if (isWindow() && ! (windowType() == Qt::Popup)) {
      return data->crect.x() - d->frameStrut().left();
   }
   return data->crect.x();
}

int QWidget::y() const
{
   Q_D(const QWidget);
   if (isWindow() && ! (windowType() == Qt::Popup)) {
      return data->crect.y() - d->frameStrut().top();
   }
   return data->crect.y();
}

QPoint QWidget::pos() const
{
   Q_D(const QWidget);
   if (isWindow() && ! (windowType() == Qt::Popup)) {
      QRect fs = d->frameStrut();
      return QPoint(data->crect.x() - fs.left(), data->crect.y() - fs.top());
   }
   return data->crect.topLeft();
}


QRect QWidget::normalGeometry() const
{
   Q_D(const QWidget);
   if (!d->extra || !d->extra->topextra) {
      return QRect();
   }

   if (!isMaximized() && !isFullScreen()) {
      return geometry();
   }

   return d->topData()->normalGeometry;
}

QRect QWidget::childrenRect() const
{
   QRect r(0, 0, 0, 0);

   for (int i = 0; i < children().size(); ++i) {
      QWidget *w = qobject_cast<QWidget *>(children().at(i));

      if (w && ! w->isWindow() && !w->isHidden()) {
         r |= w->geometry();
      }
   }
   return r;
}

QRegion QWidget::childrenRegion() const
{
   QRegion r;

   for (int i = 0; i < children().size(); ++i) {
      QWidget *w = qobject_cast<QWidget *>(children().at(i));

      if (w && !w->isWindow() && ! w->isHidden()) {
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
          ? QSize(d->extra->topextra->incw, d->extra->topextra->inch)
          : QSize(0, 0);
}

QSize QWidget::baseSize() const
{
   Q_D(const QWidget);
   return (d->extra != 0 && d->extra->topextra != 0)
          ? QSize(d->extra->topextra->basew, d->extra->topextra->baseh)
          : QSize(0, 0);
}

bool QWidgetPrivate::setMinimumSize_helper(int &minw, int &minh)
{
   Q_Q(QWidget);

#ifdef Q_WS_QWS
   if (q->isWindow()) {
      const QRect maxWindowRect = QApplication::desktop()->availableGeometry(QApplication::desktop()->screenNumber(q));
      if (!maxWindowRect.isEmpty()) {
         // ### This is really just a work-around. Layout shouldn't be
         // asking for minimum sizes bigger than the screen.
         if (minw > maxWindowRect.width()) {
            minw = maxWindowRect.width();
         }
         if (minh > maxWindowRect.height()) {
            minh = maxWindowRect.height();
         }
      }
   }
#endif

   int mw = minw, mh = minh;
   if (mw == QWIDGETSIZE_MAX) {
      mw = 0;
   }
   if (mh == QWIDGETSIZE_MAX) {
      mh = 0;
   }

   if (minw > QWIDGETSIZE_MAX || minh > QWIDGETSIZE_MAX) {
      qWarning("QWidget::setMinimumSize: (%s/%s) The largest allowed size is (%d,%d)",
               q->objectName().toLocal8Bit().data(), q->metaObject()->className(), QWIDGETSIZE_MAX,
               QWIDGETSIZE_MAX);

      minw = mw = qMin(minw, QWIDGETSIZE_MAX);
      minh = mh = qMin(minh, QWIDGETSIZE_MAX);
   }

   if (minw < 0 || minh < 0) {
      qWarning("QWidget::setMinimumSize: (%s/%s) Negative sizes (%d,%d) are not possible",
               q->objectName().toLocal8Bit().data(), q->metaObject()->className(), minw, minh);

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

void QWidget::setMinimumSize(int minw, int minh)
{
   Q_D(QWidget);
   if (!d->setMinimumSize_helper(minw, minh)) {
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
         data->window_state = data->window_state | Qt::WindowMaximized;
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
      qWarning("QWidget::setMaximumSize: (%s/%s) The largest allowed size is (%d,%d)",
               q->objectName().toLocal8Bit().data(), q->metaObject()->className(), QWIDGETSIZE_MAX,
               QWIDGETSIZE_MAX);

      maxw = qMin(maxw, QWIDGETSIZE_MAX);
      maxh = qMin(maxh, QWIDGETSIZE_MAX);
   }

   if (maxw < 0 || maxh < 0) {
      qWarning("QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) are not possible",
               q->objectName().toLocal8Bit().data(), q->metaObject()->className(), maxw, maxh);

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
   if (!d->setMaximumSize_helper(maxw, maxh)) {
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

#ifdef Q_WS_QWS
   // temporary fix for 4.3.x.
   // Should move the embedded spesific contraints in setMinimumSize_helper into QLayout
   int tmpW = w;
   int tmpH = h;
   bool minSizeSet = d->setMinimumSize_helper(tmpW, tmpH);
#else
   bool minSizeSet = d->setMinimumSize_helper(w, h);
#endif

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

QPoint QWidget::mapTo(QWidget *parent, const QPoint &pos) const
{
   QPoint p = pos;
   if (parent) {
      const QWidget *w = this;
      while (w != parent) {
         Q_ASSERT_X(w, "QWidget::mapTo(QWidget *parent, const QPoint &pos)",
                    "parent must be in parent hierarchy");
         p = w->mapToParent(p);
         w = w->parentWidget();
      }
   }
   return p;
}

QPoint QWidget::mapFrom(QWidget *parent, const QPoint &pos) const
{
   QPoint p(pos);
   if (parent) {
      const QWidget *w = this;
      while (w != parent) {
         Q_ASSERT_X(w, "QWidget::mapFrom(QWidget *parent, const QPoint &pos)",
                    "parent must be in parent hierarchy");

         p = w->mapFromParent(p);
         w = w->parentWidget();
      }
   }
   return p;
}

QPoint QWidget::mapToParent(const QPoint &pos) const
{
   return pos + data->crect.topLeft();
}

QPoint QWidget::mapFromParent(const QPoint &pos) const
{
   return pos - data->crect.topLeft();
}

QWidget *QWidget::window() const
{
   QWidget *w = (QWidget *)this;
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
   while (parent && !parent->internalWinId()) {
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
   if (!isEnabled()) {
      data->pal.setCurrentColorGroup(QPalette::Disabled);

   } else if ((!isVisible() || isActiveWindow())

#if defined(Q_OS_WIN)
              && !QApplicationPrivate::isBlockedByModal(const_cast<QWidget *>(this))
#endif
             ) {
      data->pal.setCurrentColorGroup(QPalette::Active);

   } else {
#ifdef Q_OS_MAC
      extern bool qt_mac_can_clickThrough(const QWidget *); //qwidget_mac.cpp
      if (qt_mac_can_clickThrough(this)) {
         data->pal.setCurrentColorGroup(QPalette::Active);
      } else
#endif
         data->pal.setCurrentColorGroup(QPalette::Inactive);
   }
   return data->pal;
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

// internal
QPalette QWidgetPrivate::naturalWidgetPalette(uint inheritedMask) const
{
   Q_Q(const QWidget);
   QPalette naturalPalette = QApplication::palette(q);

   if (!q->testAttribute(Qt::WA_StyleSheet)
         && (!q->isWindow() || q->testAttribute(Qt::WA_WindowPropagation)

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

// internal
void QWidgetPrivate::resolvePalette()
{
   QPalette naturalPalette = naturalWidgetPalette(inheritedPaletteResolveMask);
   QPalette resolvedPalette = data.pal.resolve(naturalPalette);
   setPalette_helper(resolvedPalette);
}

void QWidgetPrivate::setPalette_helper(const QPalette &palette)
{
   Q_Q(QWidget);

   if (data.pal == palette && data.pal.resolve() == palette.resolve()) {
      return;
   }

   data.pal = palette;
   updateSystemBackground();
   propagatePaletteChange();
   updateIsOpaque();

   q->update();
   updateIsOpaque();
}

void QWidget::setFont(const QFont &font)
{
   Q_D(QWidget);

#ifndef QT_NO_STYLE_STYLESHEET
   const QStyleSheetStyle *style;
   if (d->extra && (style = qobject_cast<const QStyleSheetStyle *>(d->extra->style))) {
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

// internal
QFont QWidgetPrivate::naturalWidgetFont(uint inheritedMask) const
{
   Q_Q(const QWidget);
   QFont naturalFont = QApplication::font(q);
   if (!q->testAttribute(Qt::WA_StyleSheet)
         && (!q->isWindow() || q->testAttribute(Qt::WA_WindowPropagation)

#ifndef QT_NO_GRAPHICSVIEW
             || (extra && extra->proxyWidget)
#endif
            )) {
      if (QWidget *p = q->parentWidget()) {
         if (!p->testAttribute(Qt::WA_StyleSheet)) {
            if (!naturalFont.isCopyOf(QApplication::font())) {
               QFont inheritedFont = p->font();
               inheritedFont.resolve(inheritedMask);
               naturalFont = inheritedFont.resolve(naturalFont);
            } else {
               naturalFont = p->font();
            }
         }
      }
#ifndef QT_NO_GRAPHICSVIEW
      else if (extra && extra->proxyWidget) {
         QFont inheritedFont = extra->proxyWidget->font();
         inheritedFont.resolve(inheritedMask);
         naturalFont = inheritedFont.resolve(naturalFont);
      }
#endif
   }

   naturalFont.resolve(0);
   return naturalFont;
}

// internal
void QWidgetPrivate::resolveFont()
{
   QFont naturalFont = naturalWidgetFont(inheritedFontResolveMask);
   QFont resolvedFont = data.fnt.resolve(naturalFont);
   setFont_helper(resolvedFont);
}

// internal
void QWidgetPrivate::updateFont(const QFont &font)
{
   Q_Q(QWidget);

#ifndef QT_NO_STYLE_STYLESHEET
   const QStyleSheetStyle *cssStyle;
   cssStyle = extra ? qobject_cast<const QStyleSheetStyle *>(extra->style) : 0;
#endif

   data.fnt = QFont(font, q);

#if defined(Q_WS_X11)
   // make sure the font set on this widget is associated with the correct screen
   data.fnt.x11SetScreen(xinfo.screen());
#endif

   // Combine new mask with natural mask and propagate to children.
#ifndef QT_NO_GRAPHICSVIEW
   if (!q->parentWidget() && extra && extra->proxyWidget) {
      QGraphicsProxyWidget *p = extra->proxyWidget;
      inheritedFontResolveMask = p->d_func()->inheritedFontResolveMask | p->font().resolve();
   } else
#endif
      if (q->isWindow() && !q->testAttribute(Qt::WA_WindowPropagation)) {
         inheritedFontResolveMask = 0;
      }
   uint newMask = data.fnt.resolve() | inheritedFontResolveMask;

   for (int i = 0; i < q->children().size(); ++i) {
      QWidget *w = qobject_cast<QWidget *>(q->children().at(i));

      if (w) {
         if (0) {
#ifndef QT_NO_STYLE_STYLESHEET
         } else if (w->testAttribute(Qt::WA_StyleSheet)) {
            // Style sheets follow a different font propagation scheme.
            if (cssStyle) {
               cssStyle->updateStyleSheetFont(w);
            }
#endif

         } else if ((!w->isWindow() || w->testAttribute(Qt::WA_WindowPropagation))) {
            // Propagate font changes.
            QWidgetPrivate *wd = w->d_func();
            wd->inheritedFontResolveMask = newMask;
            wd->resolveFont();
         }
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

   if ( (direction == Qt::RightToLeft) == q->testAttribute(Qt::WA_RightToLeft)) {
      return;
   }

   q->setAttribute(Qt::WA_RightToLeft, (direction == Qt::RightToLeft));

   if (! q->children().isEmpty()) {

      for (int i = 0; i < q->children().size(); ++i) {
         QWidget *w = qobject_cast<QWidget *>(q->children().at(i));
         if (w && !w->isWindow() && !w->testAttribute(Qt::WA_SetLayoutDirection)) {
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
   if (!q->testAttribute(Qt::WA_SetLayoutDirection)) {
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
   if (testAttribute(Qt::WA_SetCursor))
      return (d->extra && d->extra->curs)
             ? *d->extra->curs
             : QCursor(Qt::ArrowCursor);
   if (isWindow() || !parentWidget()) {
      return QCursor(Qt::ArrowCursor);
   }
   return parentWidget()->cursor();
}

void QWidget::setCursor(const QCursor &cursor)
{
   Q_D(QWidget);

   // On Mac we must set the cursor even if it is the ArrowCursor
#if !defined(Q_OS_MAC) && !defined(Q_WS_QWS)
   if (cursor.shape() != Qt::ArrowCursor
         || (d->extra && d->extra->curs))
#endif
   {
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

void QWidget::unsetCursor()
{
   Q_D(QWidget);
   if (d->extra) {
      delete d->extra->curs;
      d->extra->curs = 0;
   }
   if (!isWindow()) {
      setAttribute(Qt::WA_SetCursor, false);
   }
   d->unsetCursor_sys();

   QEvent event(QEvent::CursorChange);
   QApplication::sendEvent(this, &event);
}

#endif

void QWidget::render(QPaintDevice *target, const QPoint &targetOffset,
                     const QRegion &sourceRegion, RenderFlags renderFlags)
{
   d_func()->render(target, targetOffset, sourceRegion, renderFlags, false);
}

void QWidget::render(QPainter *painter, const QPoint &targetOffset,
                     const QRegion &sourceRegion, RenderFlags renderFlags)
{
   if (! painter) {
      qWarning("QWidget::render: Null pointer to painter");
      return;
   }

   if (! painter->isActive()) {
      qWarning("QWidget::render: Can not render with an inactive painter");
      return;
   }

   const qreal opacity = painter->opacity();
   if (qFuzzyIsNull(opacity)) {
      return;   // Fully transparent.
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

#ifdef Q_OS_MAC
   d->render_helper(painter, targetOffset, toBePainted, renderFlags);
#else
   QPaintEngine *engine = painter->paintEngine();
   Q_ASSERT(engine);

   QPaintEnginePrivate *enginePriv = engine->d_func();
   Q_ASSERT(enginePriv);

   QPaintDevice *target = engine->paintDevice();
   Q_ASSERT(target);

   // Render via a pixmap when dealing with non-opaque painters or printers.
   if (!inRenderWithPainter && (opacity < 1.0 || (target->devType() == QInternal::Printer))) {
      d->render_helper(painter, targetOffset, toBePainted, renderFlags);
      d->extra->inRenderWithPainter = false;
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

   render(target, targetOffset, toBePainted, renderFlags);

   // Restore system clip, viewport and transform.
   enginePriv->systemClip = oldSystemClip;
   enginePriv->setSystemViewport(oldSystemViewport);
   enginePriv->setSystemTransform(oldTransform);

   // Restore shared painter.
   d->setSharedPainter(oldPainter);
#endif

   d->extra->inRenderWithPainter = false;
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
      d->graphicsEffect = 0;
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
   if (data.in_show) {
      return true;
   }

   Q_Q(const QWidget);
   if (q->isHidden()) {
      return false;
   }

   // The widget will be shown if any of its ancestors are about to show.
   QWidget *parent = q->parentWidget();
   return parent ? parent->d_func()->isAboutToShow() : false;
}

QRegion QWidgetPrivate::prepareToRender(const QRegion &region, QWidget::RenderFlags renderFlags)
{
   Q_Q(QWidget);
   const bool isVisible = q->isVisible();

   // Make sure the widget is laid out correctly.
   if (!isVisible && !isAboutToShow()) {
      QWidget *topLevel = q->window();
      (void)topLevel->d_func()->topData(); // Make sure we at least have top-data.
      topLevel->ensurePolished();

      // Invalidate the layout of hidden ancestors (incl. myself) and pretend
      // they're not explicitly hidden.
      QWidget *widget = q;
      QWidgetList hiddenWidgets;
      while (widget) {
         if (widget->isHidden()) {
            widget->setAttribute(Qt::WA_WState_Hidden, false);
            hiddenWidgets.append(widget);
            if (!widget->isWindow() && widget->parentWidget()->d_func()->layout) {
               widget->d_func()->updateGeometry_helper(true);
            }
         }
         widget = widget->parentWidget();
      }

      // Activate top-level layout.
      if (topLevel->d_func()->layout) {
         topLevel->d_func()->layout->activate();
      }

      // Adjust size if necessary.
      QTLWExtra *topLevelExtra = topLevel->d_func()->maybeTopData();
      if (topLevelExtra && !topLevelExtra->sizeAdjusted
            && !topLevel->testAttribute(Qt::WA_Resized)) {
         topLevel->adjustSize();
         topLevel->setAttribute(Qt::WA_Resized, false);
      }

      // Activate child layouts.
      topLevel->d_func()->activateChildLayoutsRecursively();

      // We're not cheating with WA_WState_Hidden anymore.
      for (int i = 0; i < hiddenWidgets.size(); ++i) {
         QWidget *widget = hiddenWidgets.at(i);
         widget->setAttribute(Qt::WA_WState_Hidden);
         if (!widget->isWindow() && widget->parentWidget()->d_func()->layout) {
            widget->parentWidget()->d_func()->layout->invalidate();
         }
      }
   } else if (isVisible) {
      q->window()->d_func()->sendPendingMoveAndResizeEvents(true, true);
   }

   // Calculate the region to be painted.
   QRegion toBePainted = !region.isEmpty() ? region : QRegion(q->rect());

   if (!(renderFlags & QWidget::IgnoreMask) && extra && extra->hasMask) {
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

#ifndef Q_OS_MAC
   const QTransform originalTransform = painter->worldTransform();
   const bool useDeviceCoordinates = originalTransform.isScaling();

   if ( !useDeviceCoordinates) {
#endif
      // Render via a pixmap.
      const QRect rect = toBePainted.boundingRect();
      const QSize size = rect.size();
      if (size.isNull()) {
         return;
      }

      QPixmap pixmap(size);
      if (!(renderFlags & QWidget::DrawWindowBackground) || !isOpaque) {
         pixmap.fill(Qt::transparent);
      }
      q->render(&pixmap, QPoint(), toBePainted, renderFlags);

      const bool restore = !(painter->renderHints() & QPainter::SmoothPixmapTransform);
      painter->setRenderHints(QPainter::SmoothPixmapTransform, true);

      painter->drawPixmap(targetOffset, pixmap);

      if (restore) {
         painter->setRenderHints(QPainter::SmoothPixmapTransform, false);
      }

#ifndef Q_OS_MAC
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
#endif
}

void QWidgetPrivate::drawWidget(QPaintDevice *pdev, const QRegion &rgn, const QPoint &offset, int flags,
                                QPainter *sharedPainter, QWidgetBackingStore *backingStore)
{
   if (rgn.isEmpty()) {
      return;
   }

#if defined(Q_OS_MAC)
   if (qt_mac_clearDirtyOnWidgetInsideDrawWidget) {
      dirtyOnWidget = QRegion();
   }

   // We disable the rendering of QToolBar in the backingStore if
   // it's supposed to be in the unified toolbar on Mac OS X.
   if (backingStore && isInUnifiedToolbar) {
      return;
   }
#endif

   Q_Q(QWidget);

#if !defined(QT_NO_GRAPHICSEFFECT) && !defined(Q_OS_MAC)
   if (graphicsEffect && graphicsEffect->isEnabled()) {
      QGraphicsEffectSource *source = graphicsEffect->d_func()->source;
      QWidgetEffectSourcePrivate *sourced = static_cast<QWidgetEffectSourcePrivate *>
                                            (source->d_func());

      if (!sourced->context) {
         QWidgetPaintContext context(pdev, rgn, offset, flags, sharedPainter, backingStore);
         sourced->context = &context;

         if (!sharedPainter) {
            QPaintEngine *paintEngine = pdev->paintEngine();
            paintEngine->d_func()->systemClip = rgn.translated(offset);
            QPainter p(pdev);
            p.translate(offset);
            context.painter = &p;
            graphicsEffect->draw(&p);
            paintEngine->d_func()->systemClip = QRegion();

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
         sourced->context = 0;
         return;
      }
   }
#endif //QT_NO_GRAFFICSEFFECT

   const bool asRoot = flags & DrawAsRoot;
   const bool alsoOnScreen = flags & DrawPaintOnScreen;
   const bool recursive = flags & DrawRecursive;
   const bool alsoInvisible = flags & DrawInvisible;

   Q_ASSERT(sharedPainter ? sharedPainter->isActive() : true);

   QRegion toBePainted(rgn);
   if (asRoot && !alsoInvisible) {
      toBePainted &= clipRect();   //(rgn & visibleRegion());
   }

   if (!(flags & DontSubtractOpaqueChildren)) {
      subtractOpaqueChildren(toBePainted, q->rect());
   }

   if (!toBePainted.isEmpty()) {
      bool onScreen = paintOnScreen();

      if (!onScreen || alsoOnScreen) {
         //update the "in paint event" flag
         if (q->testAttribute(Qt::WA_WState_InPaintEvent)) {
            qWarning("QWidget::repaint() Recursive repaint detected");
         }

         q->setAttribute(Qt::WA_WState_InPaintEvent);

         //clip away the new area
         QPaintEngine *paintEngine = pdev->paintEngine();
         if (paintEngine) {
            setRedirected(pdev, -offset);

#ifdef Q_OS_MAC
            // (Alien support) Special case for Mac when redirecting: If the paint device
            // is of the Widget type we need to set WA_WState_InPaintEvent since painting
            // outside the paint event is not supported on QWidgets. The attributeis
            // restored further down.
            if (pdev->devType() == QInternal::Widget) {
               static_cast<QWidget *>(pdev)->setAttribute(Qt::WA_WState_InPaintEvent);
            }
#endif
            if (sharedPainter) {
               paintEngine->d_func()->systemClip = toBePainted;
            } else {
               paintEngine->d_func()->systemRect = q->data->crect;
            }

            // paint the background
            if ((asRoot || q->autoFillBackground() || onScreen || q->testAttribute(Qt::WA_StyledBackground))
                  && !q->testAttribute(Qt::WA_OpaquePaintEvent) && !q->testAttribute(Qt::WA_NoSystemBackground)) {
               QPainter p(q);
               paintBackground(&p, toBePainted, (asRoot || onScreen) ? flags | DrawAsRoot : 0);
            }

            if (!sharedPainter) {
               paintEngine->d_func()->systemClip = toBePainted.translated(offset);
            }

            if (!onScreen && !asRoot && !isOpaque && q->testAttribute(Qt::WA_TintedBackground)) {
               QPainter p(q);
               QColor tint = q->palette().window().color();
               tint.setAlphaF(qreal(.6));
               p.fillRect(toBePainted.boundingRect(), tint);
            }
         }

         // actually send the paint event
         QPaintEvent e(toBePainted);
         QCoreApplication::sendSpontaneousEvent(q, &e);

#if !defined(Q_WS_QWS) && !defined(Q_WS_QPA)
         if (backingStore && !onScreen && !asRoot && (q->internalWinId() || !q->nativeParentWidget()->isWindow())) {
            backingStore->markDirtyOnScreen(toBePainted, q, offset);
         }
#endif

         //restore
         if (paintEngine) {
#ifdef Q_OS_MAC
            if (pdev->devType() == QInternal::Widget) {
               static_cast<QWidget *>(pdev)->setAttribute(Qt::WA_WState_InPaintEvent, false);
            }
#endif
            restoreRedirected();
            if (!sharedPainter) {
               paintEngine->d_func()->systemRect = QRect();
            } else {
               paintEngine->d_func()->currentClipWidget = 0;
            }
            paintEngine->d_func()->systemClip = QRegion();
         }
         q->setAttribute(Qt::WA_WState_InPaintEvent, false);
         if (q->paintingActive() && !q->testAttribute(Qt::WA_PaintOutsidePaintEvent)) {
            qWarning("QWidget::repaint: It is dangerous to leave painters active on a widget outside of the PaintEvent");
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
      paintSiblingsRecursive(pdev, q->children(), q->children().size() - 1, rgn, offset, flags & ~DrawAsRoot
#ifdef Q_BACKINGSTORE_SUBSURFACES
                             , q->windowSurface()
#endif
                             , sharedPainter, backingStore);
   }
}

void QWidgetPrivate::render(QPaintDevice *target, const QPoint &targetOffset,
                            const QRegion &sourceRegion, QWidget::RenderFlags renderFlags,
                            bool readyToRender)
{
   if (!target) {
      qWarning("QWidget::render() Null pointer to paint device");
      return;
   }

   const bool inRenderWithPainter = extra && extra->inRenderWithPainter;

   QRegion paintRegion;

   if (! inRenderWithPainter && ! readyToRender)  {
      paintRegion = prepareToRender(sourceRegion, renderFlags);
   } else {
      paintRegion = sourceRegion;
   }

   if (paintRegion.isEmpty()) {
      return;
   }

#ifndef Q_OS_MAC
   QPainter *oldSharedPainter = inRenderWithPainter ? sharedPainter() : 0;

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
#endif

   // Use the target's redirected device if set and adjust offset and paint
   // region accordingly. This is typically the case when people call render
   // from the paintEvent.
   QPoint offset = targetOffset;
   offset -= paintRegion.boundingRect().topLeft();
   QPoint redirectionOffset;
   QPaintDevice *redirected = 0;

   if (target->devType() == QInternal::Widget) {
      redirected = static_cast<QWidget *>(target)->d_func()->redirected(&redirectionOffset);
   }
   if (!redirected) {
      redirected = QPainter::redirected(target, &redirectionOffset);
   }

   if (redirected) {
      target = redirected;
      offset -= redirectionOffset;
   }

   if (! inRenderWithPainter) { // Clip handled by shared painter (in qpainter.cpp).

      if (QPaintEngine *targetEngine = target->paintEngine()) {
         const QRegion targetSystemClip = targetEngine->systemClip();

         if (!targetSystemClip.isEmpty()) {
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

#if defined(Q_WS_QWS) || defined(Q_WS_QPA)
   flags |= DontSetCompositionMode;
#endif

   if (target->devType() == QInternal::Printer) {
      QPainter p(target);
      render_helper(&p, targetOffset, paintRegion, renderFlags);
      return;
   }

#ifndef Q_OS_MAC
   // Render via backingstore.
   drawWidget(target, paintRegion, offset, flags, sharedPainter());

   // Restore shared painter.
   if (oldSharedPainter) {
      setSharedPainter(oldSharedPainter);
   }
#else
   // Render via backingstore (no shared painter).
   drawWidget(target, paintRegion, offset, flags, 0);
#endif
}

void QWidgetPrivate::paintSiblingsRecursive(QPaintDevice *pdev, const QObjectList &siblings, int index,
      const QRegion &rgn, const QPoint &offset, int flags

#ifdef Q_BACKINGSTORE_SUBSURFACES
      , const QWindowSurface *currentSurface
#endif
      , QPainter *sharedPainter, QWidgetBackingStore *backingStore)
{
   QWidget *w = 0;
   QRect boundingRect;
   bool dirtyBoundingRect = true;

   const bool exludeOpaqueChildren = (flags & DontDrawOpaqueChildren);
   const bool excludeNativeChildren = (flags & DontDrawNativeChildren);

   do {
      QWidget *x =  qobject_cast<QWidget *>(siblings.at(index));

      if (x && !(exludeOpaqueChildren && x->d_func()->isOpaque) && !x->isHidden() && !x->isWindow()
            && !(excludeNativeChildren && x->internalWinId())) {

         if (dirtyBoundingRect) {
            boundingRect = rgn.boundingRect();
            dirtyBoundingRect = false;
         }

         if (qRectIntersects(boundingRect, x->d_func()->effectiveRectFor(x->data->crect))) {

#ifdef Q_BACKINGSTORE_SUBSURFACES
            if (x->windowSurface() == currentSurface)
#endif
            {
               w = x;
               break;
            }
         }
      }
      --index;

   } while (index >= 0);

   if (! w) {
      return;
   }

   QWidgetPrivate *wd = w->d_func();
   const QPoint widgetPos(w->data->crect.topLeft());
   const bool hasMask = wd->extra && wd->extra->hasMask && !wd->graphicsEffect;

   if (index > 0) {
      QRegion wr(rgn);

      if (wd->isOpaque) {
         wr -= hasMask ? wd->extra->mask.translated(widgetPos) : w->data->crect;
      }

#ifdef Q_BACKINGSTORE_SUBSURFACES
      paintSiblingsRecursive(pdev, siblings, --index, wr, offset, flags, currentSurface, sharedPainter, backingStore);
#else
      paintSiblingsRecursive(pdev, siblings, --index, wr, offset, flags, sharedPainter, backingStore);
#endif

   }

#ifdef QT_NO_GRAPHICSVIEW
   if (w->updatesEnabled()) {
#else
   if (w->updatesEnabled() && (!w->d_func()->extra || ! w->d_func()->extra->proxyWidget)) {
#endif

      QRegion wRegion(rgn);
      wRegion &= wd->effectiveRectFor(w->data->crect);

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

   if (!context) {
      // Device coordinates without context not yet supported.
      qWarning("QGraphicsEffectSource::boundingRect: Not yet implemented, lacking device context");
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
   if (!context && deviceCoordinates) {
      // Device coordinates without context not yet supported.
      qWarning("QGraphicsEffectSource::pixmap: Not yet implemented, lacking device context");
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

   QPixmap pixmap(effectRect.size());
   pixmap.fill(Qt::transparent);
   m_widget->render(&pixmap, pixmapOffset, QRegion(), QWidget::DrawChildren);
   return pixmap;
}
#endif //QT_NO_GRAPHICSEFFECT

#ifndef QT_NO_GRAPHICSVIEW

// internal
QGraphicsProxyWidget *QWidgetPrivate::nearestGraphicsProxyWidget(const QWidget *origin)
{
   if (origin) {
      QWExtra *extra = origin->d_func()->extra;
      if (extra && extra->proxyWidget) {
         return extra->proxyWidget;
      }
      return nearestGraphicsProxyWidget(origin->parentWidget());
   }
   return 0;
}
#endif

void QWidgetPrivate::setLocale_helper(const QLocale &loc, bool forceUpdate)
{
   Q_Q(QWidget);
   if (locale == loc && !forceUpdate) {
      return;
   }

   locale = loc;

   if (! q->children().isEmpty()) {
      for (int i = 0; i < q->children().size(); ++i) {
         QWidget *w = qobject_cast<QWidget *>(q->children().at(i));

         if (!w) {
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

static QString constructWindowTitleFromFilePath(const QString &filePath)
{
   QFileInfo fi(filePath);
   QString windowTitle = fi.fileName() + QLatin1String("[*]");

#ifndef Q_OS_MAC
   QString appName = QApplication::applicationName();
   if (!appName.isEmpty()) {
      windowTitle += QLatin1Char(' ') + QChar(0x2014) + QLatin1Char(' ') + appName;
   }
#endif

   return windowTitle;
}

QString QWidget::windowTitle() const
{
   Q_D(const QWidget);
   if (d->extra && d->extra->topextra) {
      if (!d->extra->topextra->caption.isEmpty()) {
         return d->extra->topextra->caption;
      }
      if (!d->extra->topextra->filePath.isEmpty()) {
         return constructWindowTitleFromFilePath(d->extra->topextra->filePath);
      }
   }
   return QString();
}

QString qt_setWindowTitle_helperHelper(const QString &title, const QWidget *widget)
{
   Q_ASSERT(widget);

#ifdef QT_EVAL
   extern QString qt_eval_adapt_window_title(const QString & title);
   QString cap = qt_eval_adapt_window_title(title);
#else
   QString cap = title;
#endif

   if (cap.isEmpty()) {
      return cap;
   }

   QLatin1String placeHolder("[*]");
   int placeHolderLength = 3; // QLatin1String doesn't have length()

   int index = cap.indexOf(placeHolder);

   // here the magic begins
   while (index != -1) {
      index += placeHolderLength;
      int count = 1;
      while (cap.indexOf(placeHolder, index) == index) {
         ++count;
         index += placeHolderLength;
      }

      if (count % 2) { // odd number of [*] -> replace last one
         int lastIndex = cap.lastIndexOf(placeHolder, index - 1);
         if (widget->isWindowModified()
               && widget->style()->styleHint(QStyle::SH_TitleBar_ModifyNotification, 0, widget)) {
            cap.replace(lastIndex, 3, QWidget::tr("*"));
         } else {
            cap.remove(lastIndex, 3);
         }
      }

      index = cap.indexOf(placeHolder, index);
   }

   cap.replace(QLatin1String("[*][*]"), placeHolder);

   return cap;
}

void QWidgetPrivate::setWindowTitle_helper(const QString &title)
{
   Q_Q(QWidget);
   if (q->testAttribute(Qt::WA_WState_Created)) {
      setWindowTitle_sys(qt_setWindowTitle_helperHelper(title, q));
   }
}

void QWidgetPrivate::setWindowIconText_helper(const QString &title)
{
   Q_Q(QWidget);
   if (q->testAttribute(Qt::WA_WState_Created)) {
      setWindowIconText_sys(qt_setWindowTitle_helperHelper(title, q));
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
}

void QWidget::setWindowTitle(const QString &title)
{
   if (QWidget::windowTitle() == title && !title.isEmpty() && !title.isNull()) {
      return;
   }

   Q_D(QWidget);
   d->topData()->caption = title;
   d->setWindowTitle_helper(title);

   QEvent e(QEvent::WindowTitleChange);
   QApplication::sendEvent(this, &e);
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
   QApplication::sendEvent(q_func(), &e);

   for (int i = 0; i < q->children().size(); ++i) {
      QWidget *w = qobject_cast<QWidget *>(q->children().at(i));

      if (w && !w->isWindow()) {
         QApplication::sendEvent(w, &e);
      }
   }
}

void QWidget::setWindowIcon(const QIcon &icon)
{
   Q_D(QWidget);

   setAttribute(Qt::WA_SetWindowIcon, !icon.isNull());
   d->createTLExtra();

   if (!d->extra->topextra->icon) {
      d->extra->topextra->icon = new QIcon();
   }
   *d->extra->topextra->icon = icon;

   delete d->extra->topextra->iconPixmap;
   d->extra->topextra->iconPixmap = 0;

   d->setWindowIcon_sys();
   d->setWindowIcon_helper();
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
#ifdef Q_OS_MAC
      setWindowTitle_helper(QFileInfo(filePath).fileName());
#else
      Q_Q(QWidget);
      Q_UNUSED(filePath);
      setWindowTitle_helper(q->windowTitle());
#endif
   }
#ifdef Q_OS_MAC
   setWindowFilePath_sys(filePath);
#endif
}

QString QWidget::windowRole() const
{
   Q_D(const QWidget);
   return (d->extra && d->extra->topextra) ? d->extra->topextra->role : QString();
}

void QWidget::setWindowRole(const QString &role)
{
#if defined(Q_WS_X11)
   Q_D(QWidget);
   d->topData()->role = role;
   d->setWindowRole();
#else
   Q_UNUSED(role)
#endif
}

void QWidget::setFocusProxy(QWidget *w)
{
   Q_D(QWidget);
   if (!w && !d->extra) {
      return;
   }

   for (QWidget *fp  = w; fp; fp = fp->focusProxy()) {
      if (fp == this) {
         qWarning("QWidget: %s (%s) already in focus proxy chain", metaObject()->className(),
                  objectName().toLocal8Bit().constData());
         return;
      }
   }

   d->createExtra();
   d->extra->focus_proxy = w;
}

QWidget *QWidget::focusProxy() const
{
   Q_D(const QWidget);
   return d->extra ? (QWidget *)d->extra->focus_proxy : 0;
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
   if (!isEnabled()) {
      return;
   }

   QWidget *f = this;
   while (f->d_func()->extra && f->d_func()->extra->focus_proxy) {
      f = f->d_func()->extra->focus_proxy;
   }

   if (QApplication::focusWidget() == f

#if defined(Q_OS_WIN)
         && GetFocus() == f->internalWinId()
#endif
      ) {
      return;
   }

#ifndef QT_NO_GRAPHICSVIEW
   QWidget *previousProxyFocus = 0;

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

   QWidget *w = f;
   if (isHidden()) {
      while (w && w->isHidden()) {
         w->d_func()->focus_child = f;
         w = w->isWindow() ? 0 : w->parentWidget();
      }
   } else {
      while (w) {
         w->d_func()->focus_child = f;
         w = w->isWindow() ? 0 : w->parentWidget();
      }
   }

#ifndef QT_NO_GRAPHICSVIEW
   // Update proxy state
   if (QWExtra *topData = window()->d_func()->extra) {
      if (topData->proxyWidget && !topData->proxyWidget->hasFocus()) {
         topData->proxyWidget->d_func()->focusFromWidgetToProxy = 1;
         topData->proxyWidget->setFocus(reason);
         topData->proxyWidget->d_func()->focusFromWidgetToProxy = 0;
      }
   }
#endif

   if (f->isActiveWindow()) {
      QApplicationPrivate::setFocusWidget(f, reason);

#ifndef QT_NO_ACCESSIBILITY
# ifdef Q_OS_WIN
      // The negation of the condition in setFocus_sys
      if (!(testAttribute(Qt::WA_WState_Created) && window()->windowType() != Qt::Popup && internalWinId()))
         //setFocusWidget will already post a focus event for us (that the AT client receives) on Windows
# endif
# ifdef  Q_OS_UNIX
         // menus update the focus manually and this would create bogus events
         if (!(f->inherits("QMenuBar") || f->inherits("QMenu") || f->inherits("QMenuItem")))
# endif
            QAccessible::updateAccessibility(f, 0, QAccessible::Focus);
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

            if (!isHidden()) {
#ifndef QT_NO_GRAPHICSVIEW
               // Update proxy state
               if (QWExtra *topData = window()->d_func()->extra)
                  if (topData->proxyWidget && topData->proxyWidget->hasFocus()) {
                     topData->proxyWidget->d_func()->updateProxyInputMethodAcceptanceFromWidget();
                  }
#endif
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
   }
}

void QWidget::clearFocus()
{
   QWidget *w = this;

   while (w) {
      if (w->d_func()->focus_child == this) {
         w->d_func()->focus_child = 0;
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
      QApplicationPrivate::setFocusWidget(0, Qt::OtherFocusReason);

#if defined(Q_OS_WIN)
      if (!(windowType() == Qt::Popup) && GetFocus() == internalWinId()) {
         SetFocus(0);
      } else
#endif

      {
#ifndef QT_NO_ACCESSIBILITY
         QAccessible::updateAccessibility(this, 0, QAccessible::Focus);
#endif

      }
   }
}

bool QWidget::focusNextPrevChild(bool next)
{
   Q_D(QWidget);
   QWidget *p = parentWidget();
   bool isSubWindow = (windowType() == Qt::SubWindow);
   if (!isWindow() && !isSubWindow && p) {
      return p->focusNextPrevChild(next);
   }
#ifndef QT_NO_GRAPHICSVIEW
   if (d->extra && d->extra->proxyWidget) {
      return d->extra->proxyWidget->focusNextPrevChild(next);
   }
#endif
   QWidget *w = QApplicationPrivate::focusNextPrevChild_helper(this, next);
   if (!w) {
      return false;
   }

   w->setFocus(next ? Qt::TabFocusReason : Qt::BacktabFocusReason);
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

#ifdef Q_OS_MAC
   extern bool qt_mac_is_macdrawer(const QWidget *); //qwidget_mac.cpp

   if (qt_mac_is_macdrawer(tlw) && tlw->parentWidget() && tlw->parentWidget()->isActiveWindow()) {
      return true;
   }

   extern bool qt_mac_insideKeyWindow(const QWidget *); //qwidget_mac.cpp
   if (QApplication::testAttribute(Qt::AA_MacPluginApplication) && qt_mac_insideKeyWindow(tlw)) {
      return true;
   }
#endif

   if (style()->styleHint(QStyle::SH_Widget_ShareActivation, 0, this)) {
      if (tlw->windowType() == Qt::Tool &&
            !tlw->isModal() &&
            (!tlw->parentWidget() || tlw->parentWidget()->isActiveWindow())) {
         return true;
      }
      QWidget *w = QApplication::activeWindow();
      while (w && tlw->windowType() == Qt::Tool &&
             !w->isModal() && w->parentWidget()) {
         w = w->parentWidget()->window();
         if (w == tlw) {
            return true;
         }
      }
   }

#if defined(Q_OS_WIN32)
   HWND active = GetActiveWindow();

   if (!tlw->testAttribute(Qt::WA_WState_Created)) {
      return false;
   }
   return active == tlw->internalWinId() || ::IsChild(active, tlw->internalWinId());

#else
   return false;

#endif

}

void QWidget::setTabOrder(QWidget *first, QWidget *second)
{
   if (!first || !second || first->focusPolicy() == Qt::NoFocus || second->focusPolicy() == Qt::NoFocus) {
      return;
   }

   if (first->window() != second->window()) {
      qWarning("QWidget::setTabOrder: 'first' and 'second' must be in the same window");
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

// internal
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
   QWidget *firstOld = 0;

   //QWidget *firstNew = q; //invariant
   QWidget *o = 0; // last in the old list
   QWidget *n = q; // last in the new list

   bool prevWasNew = true;
   QWidget *w = focus_next;

   //Note: for efficiency, we do not maintain the list invariant inside the loop
   //we append items to the relevant list, and we optimize by not changing pointers
   //when subsequent items are going into the same list.
   while (w  != q) {
      bool currentIsNew =  q->isAncestorOf(w);
      if (currentIsNew) {
         if (!prevWasNew) {
            //prev was old -- append to new list
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

   //repair the old list:
   if (firstOld) {
      o->d_func()->focus_next = firstOld;
      firstOld->d_func()->focus_prev = o;
   }

   if (!q->isWindow()) {
      QWidget *topLevel = q->window();
      //insert new chain into toplevel's chain

      QWidget *prev = topLevel->d_func()->focus_prev;

      topLevel->d_func()->focus_prev = n;
      prev->d_func()->focus_next = q;

      focus_prev = prev;
      n->d_func()->focus_next = topLevel;
   } else {
      //repair the new list
      n->d_func()->focus_next = q;
      focus_prev = n;
   }

}

// internal
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

      return QSize(data->crect.width() + fs.left() + fs.right(),
                   data->crect.height() + fs.top() + fs.bottom());
   }

   return data->crect.size();
}

void QWidget::move(const QPoint &p)
{
   Q_D(QWidget);
   setAttribute(Qt::WA_Moved);

   if (isWindow()) {
      d->topData()->posFromMove = true;
   }

   if (testAttribute(Qt::WA_WState_Created)) {
      d->setGeometry_sys(p.x() + geometry().x() - QWidget::x(),
                         p.y() + geometry().y() - QWidget::y(),
                         width(), height(), true);
      d->setDirtyOpaqueRegion();

   } else {
      data->crect.moveTopLeft(p); // no frame yet
      setAttribute(Qt::WA_PendingMoveEvent);
   }
}

void QWidget::resize(const QSize &s)
{
   Q_D(QWidget);
   setAttribute(Qt::WA_Resized);

   if (testAttribute(Qt::WA_WState_Created)) {
      d->setGeometry_sys(geometry().x(), geometry().y(), s.width(), s.height(), false);
      d->setDirtyOpaqueRegion();

   } else {
      data->crect.setSize(s.boundedTo(maximumSize()).expandedTo(minimumSize()));
      setAttribute(Qt::WA_PendingResizeEvent);

   }
}

void QWidget::setGeometry(const QRect &r)
{
   Q_D(QWidget);
   setAttribute(Qt::WA_Resized);
   setAttribute(Qt::WA_Moved);

   if (isWindow()) {
      d->topData()->posFromMove = false;
   }

   if (testAttribute(Qt::WA_WState_Created)) {
      d->setGeometry_sys(r.x(), r.y(), r.width(), r.height(), true);
      d->setDirtyOpaqueRegion();

   } else {

      data->crect.setTopLeft(r.topLeft());
      data->crect.setSize(r.size().boundedTo(maximumSize()).expandedTo(minimumSize()));

      setAttribute(Qt::WA_PendingMoveEvent);
      setAttribute(Qt::WA_PendingResizeEvent);
   }
}


QByteArray QWidget::saveGeometry() const
{

#ifdef Q_OS_MAC
   // We check if the window was maximized during this invocation. If so, we need to record the
   // starting position as 0,0.
   Q_D(const QWidget);
   QRect newFramePosition = frameGeometry();
   QRect newNormalPosition = normalGeometry();

   if (d->topData()->wasMaximized && !(windowState() & Qt::WindowMaximized)) {
      // Change the starting position
      newFramePosition.moveTo(0, 0);
      newNormalPosition.moveTo(0, 0);
   }
#endif

   QByteArray array;
   QDataStream stream(&array, QIODevice::WriteOnly);
   stream.setVersion(QDataStream::Qt_4_0);
   const quint32 magicNumber = 0x1D9D0CB;
   quint16 majorVersion = 1;
   quint16 minorVersion = 0;
   stream << magicNumber
          << majorVersion
          << minorVersion

#ifdef Q_OS_MAC
          << newFramePosition
          << newNormalPosition
#else
          << frameGeometry()
          << normalGeometry()
#endif

          << qint32(QApplication::desktop()->screenNumber(this))
          << quint8(windowState() & Qt::WindowMaximized)
          << quint8(windowState() & Qt::WindowFullScreen);
   return array;
}

bool QWidget::restoreGeometry(const QByteArray &geometry)
{
   if (geometry.size() < 4) {
      return false;
   }
   QDataStream stream(geometry);
   stream.setVersion(QDataStream::Qt_4_0);

   const quint32 magicNumber = 0x1D9D0CB;
   quint32 storedMagicNumber;
   stream >> storedMagicNumber;
   if (storedMagicNumber != magicNumber) {
      return false;
   }

   const quint16 currentMajorVersion = 1;
   quint16 majorVersion = 0;
   quint16 minorVersion = 0;

   stream >> majorVersion >> minorVersion;

   if (majorVersion != currentMajorVersion) {
      return false;
   }
   // (Allow all minor versions.)

   QRect restoredFrameGeometry;
   QRect restoredNormalGeometry;
   qint32 restoredScreenNumber;
   quint8 maximized;
   quint8 fullScreen;

   stream >> restoredFrameGeometry
          >> restoredNormalGeometry
          >> restoredScreenNumber
          >> maximized
          >> fullScreen;

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
                                     .size()
                                     .expandedTo(d_func()->adjustedSize()));
   }

   const QDesktopWidget *const desktop = QApplication::desktop();
   if (restoredScreenNumber >= desktop->numScreens()) {
      restoredScreenNumber = desktop->primaryScreen();
   }

   const QRect availableGeometry = desktop->availableGeometry(restoredScreenNumber);

   // Modify the restored geometry if we are about to restore to coordinates
   // that would make the window "lost". This happens if:
   // - The restored geometry is completely oustside the available geometry
   // - The title bar is outside the available geometry.
   // - (Mac only) The window is higher than the available geometry. It must
   //   be possible to bring the size grip on screen by moving the window.

#ifdef Q_OS_MAC
   restoredFrameGeometry.setHeight(qMin(restoredFrameGeometry.height(), availableGeometry.height()));
   restoredNormalGeometry.setHeight(qMin(restoredNormalGeometry.height(), availableGeometry.height() - frameHeight));
#endif

   if (!restoredFrameGeometry.intersects(availableGeometry)) {
      restoredFrameGeometry.moveBottom(qMin(restoredFrameGeometry.bottom(), availableGeometry.bottom()));
      restoredFrameGeometry.moveLeft(qMax(restoredFrameGeometry.left(), availableGeometry.left()));
      restoredFrameGeometry.moveRight(qMin(restoredFrameGeometry.right(), availableGeometry.right()));
   }
   restoredFrameGeometry.moveTop(qMax(restoredFrameGeometry.top(), availableGeometry.top()));

   if (!restoredNormalGeometry.intersects(availableGeometry)) {
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

#ifdef Q_WS_X11
      if (isFullScreen()) {
         offset = d_func()->topData()->fullScreenOffset;
      }
#endif

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

   d->leftmargin = left;
   d->topmargin = top;
   d->rightmargin = right;
   d->bottommargin = bottom;

   if (QLayout *l = d->layout) {
      l->update();   //force activate; will do updateGeometry
   } else {
      updateGeometry();
   }

   // ### Qt5: compat, remove
   if (isVisible()) {
      update();
      QResizeEvent e(data->crect.size(), data->crect.size());
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
                QPoint(data->crect.width() - 1 - d->rightmargin,
                data->crect.height() - 1 - d->bottommargin));

}

Qt::ContextMenuPolicy QWidget::contextMenuPolicy() const
{
   return (Qt::ContextMenuPolicy)data->context_menu_policy;
}

void QWidget::setContextMenuPolicy(Qt::ContextMenuPolicy policy)
{
   data->context_menu_policy = (uint) policy;
}

Qt::FocusPolicy QWidget::focusPolicy() const
{
   return (Qt::FocusPolicy)data->focus_policy;
}

void QWidget::setFocusPolicy(Qt::FocusPolicy policy)
{
   data->focus_policy = (uint) policy;
   Q_D(QWidget);
   if (d->extra && d->extra->focus_proxy) {
      d->extra->focus_proxy->setFocusPolicy(policy);
   }
}

void QWidget::setUpdatesEnabled(bool enable)
{
   Q_D(QWidget);
   setAttribute(Qt::WA_ForceUpdatesDisabled, !enable);
   d->setUpdatesEnabled_helper(enable);
}

void QWidgetPrivate::show_recursive()
{
   Q_Q(QWidget);
   // polish if necessary

   if (!q->testAttribute(Qt::WA_WState_Created)) {
      createRecursively();
   }
   q->ensurePolished();

   if (!q->isWindow() && q->parentWidget()->d_func()->layout && !q->parentWidget()->data->in_show) {
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
      QMoveEvent e(data.crect.topLeft(), data.crect.topLeft());
      QApplication::sendEvent(q, &e);
      q->setAttribute(Qt::WA_PendingMoveEvent, false);
   }

   if (q->testAttribute(Qt::WA_PendingResizeEvent)) {
      QResizeEvent e(data.crect.size(), QSize());
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
      if (QWidget *child = qobject_cast<QWidget *>(q->children().at(i))) {
         child->d_func()->sendPendingMoveAndResizeEvents(recursive, disableUpdates);
      }
   }
}

void QWidgetPrivate::activateChildLayoutsRecursively()
{
   Q_Q(QWidget);

   sendPendingMoveAndResizeEvents(false, true);

   for (int i = 0; i < q->children().size(); ++i) {
      QWidget *child = qobject_cast<QWidget *>(q->children().at(i));

      if (!child || child->isHidden() || child->isWindow()) {
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
      if (!wasVisible) {
         child->setAttribute(Qt::WA_WState_Visible, false);
      }
   }
}

void QWidgetPrivate::show_helper()
{
   Q_Q(QWidget);
   data.in_show = true; // qws optimization
   // make sure we receive pending move and resize events
   sendPendingMoveAndResizeEvents();

   // become visible before showing all children
   q->setAttribute(Qt::WA_WState_Visible);

   // finally show all children recursively
   showChildren(false);

   // popup handling: new popups and tools need to be raised, and
   // existing popups must be closed. Also propagate the current
   // windows's KeyboardFocusChange status.

   if (q->isWindow()) {
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
   bool isEmbedded = false;
#ifndef QT_NO_GRAPHICSVIEW
   if (q->isWindow()) {
      isEmbedded = q->graphicsProxyWidget() ? true : false;
      if (!isEmbedded && !bypassGraphicsProxyWidget(q)) {
         QGraphicsProxyWidget *ancestorProxy = nearestGraphicsProxyWidget(q->parentWidget());
         if (ancestorProxy) {
            isEmbedded = true;
            ancestorProxy->d_func()->embedSubWindow(q);
         }
      }
   }
#else
   Q_UNUSED(isEmbedded);
#endif

   // On Windows, show the popup now so that our own focus handling
   // stores the correct old focus widget even if it's stolen in the showevent

#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
   if (!isEmbedded && q->windowType() == Qt::Popup) {
      qApp->d_func()->openPopup(q);
   }
#endif

   // send the show event before showing the window
   QShowEvent showEvent;
   QApplication::sendEvent(q, &showEvent);

   if (!isEmbedded && q->isModal() && q->isWindow())
      // QApplicationPrivate::enterModal *before* show, otherwise the initial
      // stacking might be wrong
   {
      QApplicationPrivate::enterModal(q);
   }


   show_sys();

#if !defined(Q_OS_WIN) && ! defined(Q_OS_MAC)
   if (!isEmbedded && q->windowType() == Qt::Popup) {
      qApp->d_func()->openPopup(q);
   }
#endif

#ifndef QT_NO_ACCESSIBILITY
   if (q->windowType() != Qt::ToolTip) {   // Tooltips are read aloud twice in MS narrator.
      QAccessible::updateAccessibility(q, 0, QAccessible::ObjectShow);
   }
#endif

   if (QApplicationPrivate::hidden_focus_widget == q) {
      QApplicationPrivate::hidden_focus_widget = 0;
      q->setFocus(Qt::OtherFocusReason);
   }

   // Process events when showing a Qt::SplashScreen widget before the event loop
   // is spinnning; otherwise it might not show up on particular platforms.
   // This makes QSplashScreen behave the same on all platforms.
   if (!qApp->d_func()->in_exec && q->windowType() == Qt::SplashScreen) {
      QApplication::processEvents();
   }

   data.in_show = false;  // reset qws optimization
}

// internal
void QWidgetPrivate::hide_helper()
{
   Q_Q(QWidget);

   bool isEmbedded = false;

#if !defined QT_NO_GRAPHICSVIEW
   isEmbedded = q->isWindow() && !bypassGraphicsProxyWidget(q) && nearestGraphicsProxyWidget(q->parentWidget()) != 0;
#else
   Q_UNUSED(isEmbedded);
#endif

   if (!isEmbedded && (q->windowType() == Qt::Popup)) {
      qApp->d_func()->closePopup(q);
   }

   // Move test modal here.  Otherwise, a modal dialog could get
   // destroyed and we lose all access to its parent because we haven't
   // left modality.  (Eg. modal Progress Dialog)
   if (!isEmbedded && q->isModal() && q->isWindow()) {
      QApplicationPrivate::leaveModal(q);
   }

#if defined(Q_OS_WIN)
   if (q->isWindow() && !(q->windowType() == Qt::Popup) && q->parentWidget()
         && !q->parentWidget()->isHidden() && q->isActiveWindow()) {
      q->parentWidget()->activateWindow();   // Activate parent
   }
#endif

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

#if defined(Q_OS_WIN) || defined(Q_WS_X11) || defined (Q_WS_QWS) || defined(Q_OS_MAC) || defined(Q_WS_QPA)
      qApp->d_func()->sendSyntheticEnterLeave(q);
#endif

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
      QAccessible::updateAccessibility(q, 0, QAccessible::ObjectHide);
   }
#endif
}

void QWidget::setVisible(bool visible)
{
   if (visible) {
      // show

      if (testAttribute(Qt::WA_WState_ExplicitShowHide) && !testAttribute(Qt::WA_WState_Hidden)) {
         return;
      }

      Q_D(QWidget);

      // Designer uses a trick to make grabWidget work without showing
      if (!isWindow() && parentWidget() && parentWidget()->isVisible()
            && !parentWidget()->testAttribute(Qt::WA_WState_Created)) {
         parentWidget()->window()->d_func()->createRecursively();
      }

      //we have to at least create toplevels before applyX11SpecificCommandLineArguments
      //but not children of non-visible parents
      QWidget *pw = parentWidget();
      if (!testAttribute(Qt::WA_WState_Created)
            && (isWindow() || pw->testAttribute(Qt::WA_WState_Created))) {
         create();
      }

#if defined(Q_WS_X11)
      if (windowType() == Qt::Window) {
         QApplicationPrivate::applyX11SpecificCommandLineArguments(this);
      }

#elif defined(Q_WS_QWS)
      if (windowType() == Qt::Window) {
         QApplicationPrivate::applyQWSSpecificCommandLineArguments(this);
      }
#endif

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

      if (!isWindow()) {
         QWidget *parent = parentWidget();
         while (parent && parent->isVisible() && parent->d_func()->layout  && !parent->data->in_show) {
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
      if (!wasResized
            && (isWindow() || !parentWidget()->d_func()->layout))  {
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
         // remove posted quit events when showing a new window
         QCoreApplication::removePostedEvents(qApp, QEvent::Quit);

         d->show_helper();

#if defined(Q_oS_WIN) || defined(Q_WS_X11) || defined (Q_WS_QWS) || defined(Q_OS_MAC) || defined(Q_WS_QPA)
         qApp->d_func()->sendSyntheticEnterLeave(this);
#endif
      }

      QEvent showToParentEvent(QEvent::ShowToParent);
      QApplication::sendEvent(this, &showToParentEvent);

   } else {
      // hide

      if (testAttribute(Qt::WA_WState_ExplicitShowHide) && testAttribute(Qt::WA_WState_Hidden)) {
         return;
      }

#if defined(Q_OS_WIN)
      // reset WS_DISABLED style in a Blocked window

      if (isWindow() && testAttribute(Qt::WA_WState_Created) && QApplicationPrivate::isBlockedByModal(this)) {
         LONG dwStyle = GetWindowLong(winId(), GWL_STYLE);
         dwStyle &= ~WS_DISABLED;
         SetWindowLong(winId(), GWL_STYLE, dwStyle);
      }
#endif

      if (QApplicationPrivate::hidden_focus_widget == this) {
         QApplicationPrivate::hidden_focus_widget = 0;
      }

      Q_D(QWidget);

      // hw: The test on getOpaqueRegion() needs to be more intelligent
      // currently it doesn't work if the widget is hidden (the region will
      // be clipped). The real check should be testing the cached region
      // (and dirty flag) directly.
      if (!isWindow() && parentWidget()) { // && !d->getOpaqueRegion().isEmpty())
         parentWidget()->d_func()->setDirtyOpaqueRegion();
      }

      setAttribute(Qt::WA_WState_Hidden);
      setAttribute(Qt::WA_WState_ExplicitShowHide);
      if (testAttribute(Qt::WA_WState_Created)) {
         d->hide_helper();
      }

      // invalidate layout similar to updateGeometry()
      if (!isWindow() && parentWidget()) {
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

void QWidgetPrivate::_q_showIfNotHidden()
{
   Q_Q(QWidget);
   if ( !(q->isHidden() && q->testAttribute(Qt::WA_WState_ExplicitShowHide)) ) {
      q->setVisible(true);
   }
}

void QWidgetPrivate::showChildren(bool spontaneous)
{
   Q_Q(QWidget);

   QList<QObject *> childList = q->children();

   for (int i = 0; i < childList.size(); ++i) {
      QWidget *widget = qobject_cast<QWidget *>(childList.at(i));
      if (!widget
            || widget->isWindow()
            || widget->testAttribute(Qt::WA_WState_Hidden)) {
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
      QWidget *widget = qobject_cast<QWidget *>(childList.at(i));

      if (!widget || widget->isWindow() || widget->testAttribute(Qt::WA_WState_Hidden)) {
         continue;
      }

#ifdef Q_OS_MAC
      // Before doing anything we need to make sure that we don't leave anything in a non-consistent state.
      // When hiding a widget we need to make sure that no mouse_down events are active, because
      // the mouse_up event will never be received by a hidden widget or one of its descendants.
      // The solution is simple, before going through with this we check if there are any mouse_down events in
      // progress, if so we check if it is related to this widget or not. If so, we just reset the mouse_down and
      // then we continue.
      // In X11 and Windows we send a mouse_release event, however we don't do that here because we were already
      // ignoring that from before. I.e. Carbon did not send the mouse release event, so we will not send the
      // mouse release event. There are two ways to interpret this:
      // 1. If we don't send the mouse release event, the widget might get into an inconsistent state, i.e. it
      // might be waiting for a release event that will never arrive.
      // 2. If we send the mouse release event, then the widget might decide to trigger an action that is not
      // supposed to trigger because it is not visible.

      if (widget == qt_button_down) {
         qt_button_down = 0;
      }
#endif

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
         if (widget->internalWinId()
               && widget->testAttribute(Qt::WA_DontCreateNativeAncestors)) {
            // hide_sys() on an ancestor won't have any affect on this
            // widget, so it needs an explicit hide_sys() of its own
            widget->d_func()->hide_sys();
         }
      }
#if defined(Q_OS_WIN) || defined(Q_WS_X11) || defined (Q_WS_QWS) || defined(Q_OS_MAC) || defined(Q_WS_QPA)
      qApp->d_func()->sendSyntheticEnterLeave(widget);
#endif

#ifndef QT_NO_ACCESSIBILITY
      if (!spontaneous) {
         QAccessible::updateAccessibility(widget, 0, QAccessible::ObjectHide);
      }
#endif
   }
}

bool QWidgetPrivate::close_helper(CloseMode mode)
{
   if (data.is_closing) {
      return true;
   }

   Q_Q(QWidget);
   data.is_closing = 1;

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
         data.is_closing = 0;
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
         QApplicationPrivate::emitLastWindowClosed();
      }
   }

   if (!that.isNull()) {
      data.is_closing = 0;
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

bool QWidget::isVisibleTo(QWidget *ancestor) const
{
   if (!ancestor) {
      return isVisible();
   }

   const QWidget *w = this;

   while (!w->isHidden()
          && !w->isWindow()
          && w->parentWidget()
          && w->parentWidget() != ancestor) {
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

#ifdef Q_WS_QWS
   const QWSWindowSurface *surface = static_cast<const QWSWindowSurface *>(windowSurface());
   if (surface) {
      const QPoint offset = mapTo(surface->window(), QPoint());
      r &= surface->clipRegion().translated(-offset);
   }
#endif

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

#if defined(Q_WS_X11)
      QRect screen = QApplication::desktop()->screenGeometry(q->x11Info().screen());
#else // all others
      QRect screen = QApplication::desktop()->screenGeometry(q->pos());
#endif

      s.setWidth(qMin(s.width(), screen.width() * 2 / 3));
      s.setHeight(qMin(s.height(), screen.height() * 2 / 3));

      if (QTLWExtra *extra = maybeTopData()) {
         extra->sizeAdjusted = true;
      }
   }

   if (!s.isValid()) {
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

#if defined(Q_OS_WIN)
inline void setDisabledStyle(QWidget *w, bool setStyle)
{
   // set/reset WS_DISABLED style.
   if (w && w->isWindow() && w->isVisible() && w->isEnabled()) {
      LONG dwStyle = GetWindowLong(w->winId(), GWL_STYLE);
      LONG newStyle = dwStyle;
      if (setStyle) {
         newStyle |= WS_DISABLED;
      } else {
         newStyle &= ~WS_DISABLED;
      }
      if (newStyle != dwStyle) {
         SetWindowLong(w->winId(), GWL_STYLE, newStyle);
         // we might need to repaint in some situations (eg. menu)
         w->repaint();
      }
   }
}
#endif

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
         case QEvent::ContextMenu:

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
         // Don't reset input context here. Whether reset or not is
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

      case QEvent::NonClientAreaMouseButtonPress: {
         QWidget *w;
         while ((w = QApplication::activePopupWidget()) && w != this) {
            w->close();
            if (QApplication::activePopupWidget() == w) { // widget does not want to disappear
               w->hide();   // hide at least
            }
         }
         break;
      }

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
         if (!(k->modifiers() & (Qt::ControlModifier | Qt::AltModifier))) {  //### Add MetaModifier?
            if (k->key() == Qt::Key_Backtab
                  || (k->key() == Qt::Key_Tab && (k->modifiers() & Qt::ShiftModifier))) {
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
         if (!k->isAccepted() && QApplication::keypadNavigationEnabled()
               && !(k->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::ShiftModifier))) {
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
         if (!k->isAccepted()
               && k->modifiers() & Qt::ShiftModifier && k->key() == Qt::Key_F1
               && d->whatsThis.size()) {
            QWhatsThis::showText(mapToGlobal(inputMethodQuery(Qt::ImMicroFocus).toRect().center()), d->whatsThis, this);
            k->accept();
         }
#endif
      }
      break;

      case QEvent::KeyRelease:
         keyReleaseEvent((QKeyEvent *)event);
      // fall through

      case QEvent::ShortcutOverride:
         break;

      case QEvent::InputMethod:
         inputMethodEvent((QInputMethodEvent *) event);
         break;

      case QEvent::PolishRequest:
         ensurePolished();
         break;

      case QEvent::Polish: {
         style()->polish(this);
         setAttribute(Qt::WA_WState_Polished);
         if (!QApplication::font(this).isCopyOf(QApplication::font())) {
            d->resolveFont();
         }
         if (!QApplication::palette(this).isCopyOf(QApplication::palette())) {
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
         break;

      case QEvent::Resize:
         resizeEvent((QResizeEvent *)event);
         break;

      case QEvent::Close:
         closeEvent((QCloseEvent *)event);
         break;

#ifndef QT_NO_CONTEXTMENU
      case QEvent::ContextMenu:
         switch (data->context_menu_policy) {
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
                  QMenu::exec(d->actions, static_cast<QContextMenuEvent *>(event)->globalPos(),
                              0, this);
                  break;
               }
               // fall through
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
      case QEvent::WindowStateChange:
      case QEvent::LocaleChange:
      case QEvent::MacSizeChange:
      case QEvent::ContentsRectChange:
         changeEvent(event);
         break;

      case QEvent::WindowActivate:
      case QEvent::WindowDeactivate: {
         if (isVisible() && !palette().isEqual(QPalette::Active, QPalette::Inactive)) {
            update();
         }
         QList<QObject *> childList = children();
         for (int i = 0; i < childList.size(); ++i) {
            QWidget *w = qobject_cast<QWidget *>(childList.at(i));
            if (w && w->isVisible() && !w->isWindow()) {
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

      case QEvent::WindowBlocked:
      case QEvent::WindowUnblocked: {
         QList<QObject *> childList = children();

         for (int i = 0; i < childList.size(); ++i) {
            QObject *o = childList.at(i);

            if (o && o != QApplication::activeModalWidget()) {
               if (qobject_cast<QWidget *>(o) && static_cast<QWidget *>(o)->isWindow()) {
                  // do not forward the event to child windows,
                  // QApplication does this for us
                  continue;
               }
               QApplication::sendEvent(o, event);
            }
         }

#if defined(Q_OS_WIN)
         setDisabledStyle(this, (event->type() == QEvent::WindowBlocked));
#endif
      }

      break;

#ifndef QT_NO_TOOLTIP
      case QEvent::ToolTip:
         if (!d->toolTip.isEmpty()) {
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

#ifndef QT_NO_ACCESSIBILITY
      case QEvent::AccessibilityDescription:

      case QEvent::AccessibilityHelp: {
         QAccessibleEvent *ev = static_cast<QAccessibleEvent *>(event);
         if (ev->child()) {
            return false;
         }

         switch (ev->type()) {

#ifndef QT_NO_TOOLTIP
            case QEvent::AccessibilityDescription:
               ev->setValue(d->toolTip);
               break;
#endif

#ifndef QT_NO_WHATSTHIS
            case QEvent::AccessibilityHelp:
               ev->setValue(d->whatsThis);
               break;
#endif

            default:
               return false;
         }

         break;
      }
#endif

      case QEvent::EmbeddingControl:
         d->topData()->frameStrut.setCoords(0 , 0, 0, 0);
         data->fstrut_dirty = false;

#if defined(Q_OS_WIN) || defined(Q_WS_X11)
         d->topData()->embedded = 1;
#endif
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
            QWidget *w = qobject_cast<QWidget *>(childList.at(i));
            if (w && w->isVisible() && !w->isWindow()) {
               QApplication::sendEvent(w, event);
            }
         }
         break;
      }

#ifdef Q_OS_MAC
      case QEvent::MacGLWindowChange:
         d->needWindowChange = false;
         break;
#endif

      case QEvent::TouchBegin:
      case QEvent::TouchUpdate:
      case QEvent::TouchEnd: {

#ifndef Q_OS_MAC
         QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
         const QTouchEvent::TouchPoint &touchPoint = touchEvent->touchPoints().first();

         if (touchPoint.isPrimary() || touchEvent->deviceType() == QTouchEvent::TouchPad) {
            break;
         }

         // fake a mouse event!
         QEvent::Type eventType = QEvent::None;
         switch (touchEvent->type()) {
            case QEvent::TouchBegin:
               eventType = QEvent::MouseButtonPress;
               break;
            case QEvent::TouchUpdate:
               eventType = QEvent::MouseMove;
               break;
            case QEvent::TouchEnd:
               eventType = QEvent::MouseButtonRelease;
               break;
            default:
               Q_ASSERT(!true);
               break;
         }
         if (eventType == QEvent::None) {
            break;
         }

         QMouseEvent mouseEvent(eventType, touchPoint.pos().toPoint(), touchPoint.screenPos().toPoint(),
                                Qt::LeftButton, Qt::LeftButton, touchEvent->modifiers());

         (void) QApplication::sendEvent(this, &mouseEvent);

#endif
         break;
      }

#ifndef QT_NO_GESTURES
      case QEvent::Gesture:
         event->ignore();
         break;
#endif

#ifndef QT_NO_PROPERTIES
      case QEvent::DynamicPropertyChange: {
         const QByteArray &propName = static_cast<QDynamicPropertyChangeEvent *>(event)->propertyName();

         if (! propName.startsWith("_q_customDpi") && propName.length() == 13) {
            uint value = property(propName.constData()).toUInt();

            if (!d->extra) {
               d->createExtra();
            }

            const char axis = propName.at(12);

            if (axis == 'X') {
               d->extra->customDpiX = value;
            } else if (axis == 'Y') {
               d->extra->customDpiY = value;
            }
            d->updateFont(d->data.fnt);
         }
         // fall through
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
      case QEvent::EnabledChange:
         update();

#ifndef QT_NO_ACCESSIBILITY
         QAccessible::updateAccessibility(this, 0, QAccessible::StateChanged);
#endif
         break;

      case QEvent::FontChange:
      case QEvent::StyleChange: {
         Q_D(QWidget);
         update();
         updateGeometry();
         if (d->layout) {
            d->layout->invalidate();
         }

#ifdef Q_WS_QWS
         if (isWindow()) {
            d->data.fstrut_dirty = true;
         }
#endif

         break;
      }

      case QEvent::PaletteChange:
         update();
         break;

#ifdef Q_OS_MAC
      case QEvent::MacSizeChange:
         updateGeometry();
         break;
      case QEvent::ToolTipChange:
      case QEvent::MouseTrackingChange:
         qt_mac_update_mouseTracking(this);
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
         if (QApplication::activePopupWidget() == w) { // widget does not want to disappear
            w->hide();   // hide at least
         }
      }
      if (!rect().contains(event->pos())) {
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
   mousePressEvent(event);                        // try mouse press event
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
   if ((windowType() == Qt::Popup) && event->key() == Qt::Key_Escape) {
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

void QWidget::resizeEvent(QResizeEvent * /* event */)
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
      case Qt::ImMicroFocus:
         return QRect(width() / 2, 0, 1, height());
      case Qt::ImFont:
         return font();
      case Qt::ImAnchorPosition:
         // Fallback.
         return inputMethodQuery(Qt::ImCursorPosition);
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
   return 0;
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

   // Optimization to update input context only it has already been created.
   if (d->ic || qApp->d_func()->inputContext) {
      QInputContext *ic = inputContext();
      if (ic) {
         ic->update();
      }
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

#endif // QT_NO_DRAGANDDROP


void QWidget::showEvent(QShowEvent *)
{
}

void QWidget::hideEvent(QHideEvent *)
{
}


#if defined(Q_OS_MAC)
bool QWidget::macEvent(EventHandlerCallRef, EventRef)
{
   return false;
}
#endif

#if defined(Q_OS_WIN)
bool QWidget::winEvent(MSG *message, long *result)
{
   Q_UNUSED(message);
   Q_UNUSED(result);
   return false;
}
#endif

#if defined(Q_WS_X11)
bool QWidget::x11Event(XEvent *)
{
   return false;
}
#endif

#if defined(Q_WS_QWS)
bool QWidget::qwsEvent(QWSEvent *)
{
   return false;
}
#endif

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

      if (QWidget *w = qobject_cast<QWidget *>(o)) {
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
   if (!l) {
      qWarning("QWidget::setLayout: Cannot set layout to 0");
      return;
   }
   if (layout()) {
      if (layout() != l)
         qWarning("QWidget::setLayout: Attempting to set QLayout \"%s\" on %s \"%s\", which already has a"
                  " layout", l->objectName().toLocal8Bit().data(), metaObject()->className(),
                  objectName().toLocal8Bit().data());
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
         qWarning("QWidget::setLayout: Attempting to set QLayout \"%s\" on %s \"%s\", when the QLayout already has a parent",
                  l->objectName().toLocal8Bit().data(), metaObject()->className(),
                  objectName().toLocal8Bit().data());
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

   QLayout *l =  layout();

   if (!l) {
      return 0;
   }

   d->layout = 0;
   l->setParent(0);
   return l;
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
   d->size_policy = policy;

#ifndef QT_NO_GRAPHICSVIEW
   if (QWExtra *extra = d->extra) {
      if (extra->proxyWidget) {
         extra->proxyWidget->setSizePolicy(policy);
      }
   }
#endif

   updateGeometry();

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

// internal
bool QWidgetPrivate::hasHeightForWidth() const
{
   return layout ? layout->hasHeightForWidth() : size_policy.hasHeightForWidth();
}

QWidget *QWidget::childAt(const QPoint &p) const
{
   return d_func()->childAt_helper(p, false);
}

QWidget *QWidgetPrivate::childAt_helper(const QPoint &p, bool ignoreChildrenInDestructor) const
{
   Q_Q(const QWidget);

   if (q->children().isEmpty()) {
      return 0;
   }

#ifdef Q_OS_MAC
   // Unified tool bars on the Mac require special handling since they live outside
   // QMainWindow's geometry(). See commit: 35667fd45ada49269a5987c235fdedfc43e92bb8

   bool includeFrame = q->isWindow() && qobject_cast<const QMainWindow *>(q)
                       && static_cast<const QMainWindow *>(q)->unifiedTitleAndToolBarOnMac();
   if (includeFrame) {
      return childAtRecursiveHelper(p, ignoreChildrenInDestructor, includeFrame);
   }
#endif

   if (!pointInsideRectAndMask(p)) {
      return 0;
   }

   return childAtRecursiveHelper(p, ignoreChildrenInDestructor);
}

QWidget *QWidgetPrivate::childAtRecursiveHelper(const QPoint &p, bool ignoreChildrenInDestructor,
      bool includeFrame) const
{
   Q_Q(const QWidget);

#ifndef Q_OS_MAC
   Q_UNUSED(includeFrame);
#endif

   for (int i = q->children().size() - 1; i >= 0; --i) {
      QWidget *child = qobject_cast<QWidget *>(q->children().at(i));

      if (!child || child->isWindow() || child->isHidden() || child->testAttribute(Qt::WA_TransparentForMouseEvents)
            || (ignoreChildrenInDestructor && child->data->in_destructor)) {
         continue;
      }

      // Map the point 'p' from parent coordinates to child coordinates.
      QPoint childPoint = p;

#ifdef Q_OS_MAC
      // 'includeFrame' is true if the child's parent is a top-level QMainWindow with an unified tool bar.
      // An unified tool bar on the Mac lives outside QMainWindow's geometry(), so a normal
      // QWidget::mapFromParent won't do the trick.
      if (includeFrame && qobject_cast<QToolBar *>(child) && qt_widget_private(child)->isInUnifiedToolbar) {
	 childPoint = qt_mac_nativeMapFromParent(child, p);
      } else
#endif
         childPoint -= child->data->crect.topLeft();

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
   return 0;
}

void QWidgetPrivate::updateGeometry_helper(bool forceUpdate)
{
   Q_Q(QWidget);
   if (widgetItem) {
      widgetItem->invalidateSizeCache();
   }
   QWidget *parent;
   if (forceUpdate || !extra || extra->minw != extra->maxw || extra->minh != extra->maxh) {
      if (!q->isWindow() && !q->isHidden() && (parent = q->parentWidget())) {
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
   if (data->window_flags == flags) {
      return;
   }

   Q_D(QWidget);

   if ((data->window_flags | flags) & Qt::Window) {
      // the old type was a window and/or the new type is a window
      QPoint oldPos = pos();
      bool visible = isVisible();
      setParent(parentWidget(), flags);

      // if both types are windows or neither of them are, we restore
      // the old position
      if (!((data->window_flags ^ flags) & Qt::Window)
            && (visible || testAttribute(Qt::WA_Moved))) {
         move(oldPos);
      }
      // for backward-compatibility we change Qt::WA_QuitOnClose attribute value only when the window was recreated.
      d->adjustQuitOnCloseAttribute();
   } else {
      data->window_flags = flags;
   }
}

void QWidget::overrideWindowFlags(Qt::WindowFlags flags)
{
   data->window_flags = flags;
}

void QWidget::setParent(QWidget *parent)
{
   if (parent == parentWidget()) {
      return;
   }

   setParent((QWidget *)parent, windowFlags() & ~Qt::WindowType_Mask);
}

void QWidget::setParent(QWidget *parent, Qt::WindowFlags f)
{
   Q_D(QWidget);

   d->inSetParent  = true;
   bool resized    = testAttribute(Qt::WA_Resized);
   bool wasCreated = testAttribute(Qt::WA_WState_Created);

   QWidget *oldtlw = window();
   QWidget *desktopWidget = 0;

   if (parent && parent->windowType() == Qt::Desktop) {
      desktopWidget = parent;
   }

   bool newParent = (parent != parentWidget()) || ! wasCreated || desktopWidget;

#if defined(Q_WS_X11) || defined(Q_OS_WIN) || defined(Q_OS_MAC)
   if (newParent && parent && ! desktopWidget) {
      if (testAttribute(Qt::WA_NativeWindow) && !qApp->testAttribute(Qt::AA_DontCreateNativeWidgetSiblings)

#if defined(Q_OS_MAC)
            // On Mac, toolbars inside the unified title bar will never overlap with
            // siblings in the content view. So we skip enforce native siblings in that case
            && !d->isInUnifiedToolbar && parentWidget() && parentWidget()->isWindow()
#endif
         ) {
         parent->d_func()->enforceNativeChildren();
      }

      else if (parent->d_func()->nativeChildrenForced() || parent->testAttribute(Qt::WA_PaintOnScreen)) {
         setAttribute(Qt::WA_NativeWindow);
      }
   }
#endif

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
   QWidgetBackingStoreTracker *oldBsTracker = oldTopExtra ? &oldTopExtra->backingStore : 0;

   // calls QWidgetPrivate per platform
   d->setParent_sys(parent, f);

   QTLWExtra *topExtra = window()->d_func()->maybeTopData();
   QWidgetBackingStoreTracker *bsTracker = topExtra ? &topExtra->backingStore : 0;

   if (oldBsTracker && oldBsTracker != bsTracker) {
      oldBsTracker->unregisterWidgetSubtree(this);
   }

   if (desktopWidget) {
      parent = 0;
   }

#ifdef Q_BACKINGSTORE_SUBSURFACES
   QTLWExtra *extra = d->maybeTopData();
   QWindowSurface *windowSurface = (extra ? extra->windowSurface : 0);

   if (newParent && windowSurface) {
      QWidgetBackingStore *oldBs = oldtlw->d_func()->maybeBackingStore();
      if (oldBs) {
         oldBs->subSurfaces.removeAll(windowSurface);
      }

      if (parent) {
         QWidgetBackingStore *newBs = parent->d_func()->maybeBackingStore();
         if (newBs) {
            newBs->subSurfaces.append(windowSurface);
         }
      }
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

   if (!testAttribute(Qt::WA_StyleSheet) && (!parent || !parent->testAttribute(Qt::WA_StyleSheet))) {
      d->resolveFont();
      d->resolvePalette();
   }

   d->resolveLayoutDirection();
   d->resolveLocale();

   // Note: GL widgets under WGL or EGL will always need a ParentChange
   // event to handle recreation/rebinding of the GL context, hence the
   // (f & Qt::MSWindowsOwnDC) clause (which is set on QGLWidgets on all platforms)

   if (newParent

#if defined(Q_OS_WIN) || defined(QT_OPENGL_ES)
         || (f & Qt::MSWindowsOwnDC)
#endif
      ) {
      // propagate enabled updates enabled state to non-windows
      if (!isWindow()) {
         if (!testAttribute(Qt::WA_ForceDisabled)) {
            d->setEnabled_helper(parent ? parent->isEnabled() : true);
         }
         if (!testAttribute(Qt::WA_ForceUpdatesDisabled)) {
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

      //### already hidden above ---> must probably do something smart on the mac
      // #ifdef Q_OS_MAC
      //             extern bool qt_mac_is_macdrawer(const QWidget *); //qwidget_mac.cpp
      //             if(!qt_mac_is_macdrawer(q)) //special case
      //                 q->setAttribute(Qt::WA_WState_Hidden);
      // #else
      //             q->setAttribute(Qt::WA_WState_Hidden);
      // #endif

      sendChildEvents = CSInternalEvents::get_m_sendChildEvents(this);

      if (parent && sendChildEvents && d->polished) {
         QChildEvent e(QEvent::ChildPolished, this);
         QCoreApplication::sendEvent(parent, &e);
      }

      QEvent e(QEvent::ParentChange);
      QApplication::sendEvent(this, &e);
   }

   if (!wasCreated) {
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

   d->inSetParent = false;
}

void QWidget::scroll(int dx, int dy)
{
   if ((!updatesEnabled() && children().size() == 0) || !isVisible()) {
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
         for (const QRect & rect : (d->dirty.translated(dx, dy)).rects()) {
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

void QWidget::scroll(int dx, int dy, const QRect &r)
{

   if ((!updatesEnabled() && children().size() == 0) || !isVisible()) {
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
         for (const QRect & rect : (d->dirty.translated(dx, dy) & r).rects()) {
            proxy->update(rect);
         }
      }
      proxy->scroll(dx, dy, r.translated(proxy->subWidgetRect(this).topLeft().toPoint()));
      return;
   }
#endif
   d->scroll_sys(dx, dy, r);
}

void QWidget::repaint()
{
   repaint(rect());
}

void QWidget::repaint(int x, int y, int w, int h)
{
   if (x > data->crect.width() || y > data->crect.height()) {
      return;
   }

   if (w < 0) {
      w = data->crect.width()  - x;
   }
   if (h < 0) {
      h = data->crect.height() - y;
   }

   repaint(QRect(x, y, w, h));
}

// overload
void QWidget::repaint(const QRect &rect)
{
   Q_D(QWidget);

   if (testAttribute(Qt::WA_WState_ConfigPending)) {
      update(rect);
      return;
   }

   if (!isVisible() || !updatesEnabled() || rect.isEmpty()) {
      return;
   }

   if (hasBackingStoreSupport()) {

#ifdef Q_OS_MAC
      if (qt_widget_private(this)->isInUnifiedToolbar) {
         qt_widget_private(this)->unifiedSurface->renderToolbar(this, true);
         return;
      }
#endif

      QTLWExtra *tlwExtra = window()->d_func()->maybeTopData();
      if (tlwExtra && !tlwExtra->inTopLevelResize && tlwExtra->backingStore) {
         tlwExtra->inRepaint = true;
         tlwExtra->backingStore->markDirty(rect, this, true);
         tlwExtra->inRepaint = false;
      }
   } else {
      d->repaint_sys(rect);
   }
}

// overload
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

#ifdef Q_OS_MAC
      if (qt_widget_private(this)->isInUnifiedToolbar) {
         qt_widget_private(this)->unifiedSurface->renderToolbar(this, true);
         return;
      }
#endif
      QTLWExtra *tlwExtra = window()->d_func()->maybeTopData();
      if (tlwExtra && !tlwExtra->inTopLevelResize && tlwExtra->backingStore) {
         tlwExtra->inRepaint = true;
         tlwExtra->backingStore->markDirty(rgn, this, true);
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
   if (!isVisible() || !updatesEnabled()) {
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
#ifdef Q_OS_MAC
      if (qt_widget_private(this)->isInUnifiedToolbar) {
         qt_widget_private(this)->unifiedSurface->renderToolbar(this, true);
         return;
      }
#endif
      QTLWExtra *tlwExtra = window()->d_func()->maybeTopData();
      if (tlwExtra && !tlwExtra->inTopLevelResize && tlwExtra->backingStore) {
         tlwExtra->backingStore->markDirty(r, this);
      }
   } else {
      d_func()->repaint_sys(r);
   }
}

void QWidget::update(const QRegion &rgn)
{
   if (!isVisible() || !updatesEnabled() || rgn.isEmpty()) {
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
#ifdef Q_OS_MAC
      if (qt_widget_private(this)->isInUnifiedToolbar) {
         qt_widget_private(this)->unifiedSurface->renderToolbar(this, true);
         return;
      }
#endif
      QTLWExtra *tlwExtra = window()->d_func()->maybeTopData();
      if (tlwExtra && !tlwExtra->inTopLevelResize && tlwExtra->backingStore) {
         tlwExtra->backingStore->markDirty(r, this);
      }
   } else {
      d_func()->repaint_sys(r);
   }
}

// internal
static void setAttribute_internal(Qt::WidgetAttribute attribute, bool on, QWidgetData *data,
                                  QWidgetPrivate *d)
{
   if (attribute < int(8 * sizeof(uint))) {
      if (on) {
         data->widget_attributes |= (1 << attribute);
      } else {
         data->widget_attributes &= ~(1 << attribute);
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

void QWidget::setAttribute(Qt::WidgetAttribute attribute, bool on)
{
   if (testAttribute(attribute) == on) {
      return;
   }

   Q_D(QWidget);
   Q_ASSERT_X(sizeof(d->high_attributes) * 8 >= (Qt::WA_AttributeCount - sizeof(uint) * 8),
              "QWidget::setAttribute(WidgetAttribute, bool)",
              "QWidgetPrivate::high_attributes[] too small to contain all attributes in WidgetAttribute");

#ifdef Q_OS_WIN
   // ### Do not use PaintOnScreen+paintEngine() to do native painting in 5.0

   if (attribute == Qt::WA_PaintOnScreen && on && !inherits("QGLWidget")) {
      // see qwidget_win.cpp, ::paintEngine for details
      paintEngine();

      if (d->noPaintOnScreen) {
         return;
      }
   }
#endif

   setAttribute_internal(attribute, on, data, d);

   switch (attribute) {

#ifndef QT_NO_DRAGANDDROP
      case Qt::WA_AcceptDrops:  {
         if (on && !testAttribute(Qt::WA_DropSiteRegistered)) {
            setAttribute(Qt::WA_DropSiteRegistered, true);
         } else if (!on && (isWindow() || !parentWidget() || !parentWidget()->testAttribute(Qt::WA_DropSiteRegistered))) {
            setAttribute(Qt::WA_DropSiteRegistered, false);
         }
         QEvent e(QEvent::AcceptDropsChange);
         QApplication::sendEvent(this, &e);
         break;
      }
      case Qt::WA_DropSiteRegistered:  {
         d->registerDropSite(on);

         for (int i = 0; i < children().size(); ++i) {
            QWidget *w = qobject_cast<QWidget *>(children().at(i));
            if (w && !w->isWindow() && !w->testAttribute(Qt::WA_AcceptDrops) && w->testAttribute(Qt::WA_DropSiteRegistered) != on) {
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

#ifdef Q_OS_MAC
         d->setStyle_helper(style(), false, true);
      // Make sure things get unpolished/polished correctly.
      // fall through since changing the metal attribute affects the opaque size grip.

      case Qt::WA_MacOpaqueSizeGrip:
         d->macUpdateOpaqueSizeGrip();
         break;

      case Qt::WA_MacShowFocusRect:
         if (hasFocus()) {
            clearFocus();
            setFocus();
         }
         break;

      case Qt::WA_Hover:
         qt_mac_update_mouseTracking(this);
         break;
#endif

      case Qt::WA_MacAlwaysShowToolWindow:

#ifdef Q_OS_MAC
         d->macUpdateHideOnSuspend();
#endif
         break;

      case Qt::WA_MacNormalSize:
      case Qt::WA_MacSmallSize:
      case Qt::WA_MacMiniSize:

#ifdef Q_OS_MAC
      {
         // We can only have one of these set at a time
         const Qt::WidgetAttribute MacSizes[] = { Qt::WA_MacNormalSize, Qt::WA_MacSmallSize,
                                                  Qt::WA_MacMiniSize
                                                };
         for (int i = 0; i < 3; ++i) {
            if (MacSizes[i] != attribute) {
               setAttribute_internal(MacSizes[i], false, data, d);
            }
         }
         d->macUpdateSizeAttribute();
      }
#endif
      break;
      case Qt::WA_ShowModal:
         if (!on) {
            if (isVisible()) {
               QApplicationPrivate::leaveModal(this);
            }
            // reset modality type to Modeless when clearing WA_ShowModal
            data->window_modality = Qt::NonModal;
         } else if (data->window_modality == Qt::NonModal) {
            // determine the modality type if it hasn't been set prior
            // to setting WA_ShowModal. set the default to WindowModal
            // if we are the child of a group leader; otherwise use
            // ApplicationModal.
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
            data->window_modality = (w && w->testAttribute(Qt::WA_GroupLeader))
                                    ? Qt::WindowModal
                                    : Qt::ApplicationModal;
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
#ifndef QT_NO_IM
         QWidget *focusWidget = d->effectiveFocusWidget();
         QInputContext *ic = 0;
         if (on && !internalWinId() && hasFocus()
               && focusWidget->testAttribute(Qt::WA_InputMethodEnabled)) {
            ic = focusWidget->d_func()->inputContext();
            if (ic) {
               ic->reset();
               ic->setFocusWidget(0);
            }
         }
         if (!qApp->testAttribute(Qt::AA_DontCreateNativeWidgetSiblings) && parentWidget()

#ifdef Q_OS_MAC
               // On Mac, toolbars inside the unified title bar will never overlap with
               // siblings in the content view. So we skip enforce native siblings in that case
               && !d->isInUnifiedToolbar && parentWidget()->isWindow()
#endif

            ) {
            parentWidget()->d_func()->enforceNativeChildren();
         }
         if (on && !internalWinId() && testAttribute(Qt::WA_WState_Created)) {
            d->createWinId();
         }
         if (ic && isEnabled() && focusWidget->isEnabled()
               && focusWidget->testAttribute(Qt::WA_InputMethodEnabled)) {
            ic->setFocusWidget(focusWidget);
         }
#endif
         break;
      }
      case Qt::WA_PaintOnScreen:
         d->updateIsOpaque();

#if defined(Q_OS_WIN) || defined(Q_WS_X11) || defined(Q_OS_MAC)
         // Recreate the widget if it is already created as an alien widget and
         // WA_PaintOnScreen is enabled. Paint on screen widgets must have win id.
         // So must their children.

         if (on) {
            setAttribute(Qt::WA_NativeWindow);
            d->enforceNativeChildren();
         }
#endif
      // fall through
      case Qt::WA_OpaquePaintEvent:
         d->updateIsOpaque();
         break;

      case Qt::WA_NoSystemBackground:
         d->updateIsOpaque();
      // fall through...

      case Qt::WA_UpdatesDisabled:
         d->updateSystemBackground();
         break;

      case Qt::WA_TransparentForMouseEvents:

#ifdef Q_OS_MAC
         d->macUpdateIgnoreMouseEvents();
#endif
         break;

      case Qt::WA_InputMethodEnabled: {

#ifndef QT_NO_IM
         QWidget *focusWidget = d->effectiveFocusWidget();
         QInputContext *ic = focusWidget->d_func()->assignedInputContext();
         if (!ic && (!on || hasFocus())) {
            ic = focusWidget->d_func()->inputContext();
         }
         if (ic) {
            if (on && hasFocus() && ic->focusWidget() != focusWidget && isEnabled()
                  && focusWidget->testAttribute(Qt::WA_InputMethodEnabled)) {
               ic->setFocusWidget(focusWidget);
            } else if (!on && ic->focusWidget() == focusWidget) {
               ic->reset();
               ic->setFocusWidget(0);
            }
         }
#endif
         break;
      }
      case Qt::WA_WindowPropagation:
         d->resolvePalette();
         d->resolveFont();
         d->resolveLocale();
         break;

#ifdef Q_WS_X11
      case Qt::WA_NoX11EventCompression:
         if (!d->extra) {
            d->createExtra();
         }
         d->extra->compress_events = on;
         break;
      case Qt::WA_X11OpenGLOverlay:
         d->updateIsOpaque();
         break;
      case Qt::WA_X11DoNotAcceptFocus:
         if (testAttribute(Qt::WA_WState_Created)) {
            d->updateX11AcceptFocus();
         }
         break;
#endif

      case Qt::WA_DontShowOnScreen: {
         if (on && isVisible()) {
            // Make sure we keep the current state and only hide the widget
            // from the desktop. show_sys will only update platform specific
            // attributes at this point.
            d->hide_sys();

#ifdef Q_WS_QWS
            // Release the region for this window from qws if the widget has
            // been shown before the attribute was set.
            if (QWSWindowSurface *surface = static_cast<QWSWindowSurface *>(windowSurface())) {
               QWidget::qwsDisplay()->requestRegion(surface->winId(), surface->key(),
                                                    surface->permanentState(), QRegion());
            }
#endif
            d->show_sys();
         }
         break;
      }

#ifdef Q_WS_X11
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
         if (testAttribute(Qt::WA_WState_Created)) {
            d->setNetWmWindowTypes();
         }
         break;
#endif

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
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
         if (on) {
            d->registerTouchWindow();
         }
#endif
         break;

      case Qt::WA_LockPortraitOrientation:
      case Qt::WA_LockLandscapeOrientation:
      case Qt::WA_AutoOrientation: {
         const Qt::WidgetAttribute orientations[3] = {
            Qt::WA_LockPortraitOrientation,
            Qt::WA_LockLandscapeOrientation,
            Qt::WA_AutoOrientation
         };

         if (on) {
            // We can only have one of these set at a time
            for (int i = 0; i < 3; ++i) {
               if (orientations[i] != attribute) {
                  setAttribute_internal(orientations[i], false, data, d);
               }
            }
         }

         break;
      }
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
   if (!isWindow()) {
      return;
   }

   opacity = qBound(qreal(0.0), opacity, qreal(1.0));
   QTLWExtra *extra = d->topData();
   extra->opacity = uint(opacity * 255);
   setAttribute(Qt::WA_WState_WindowOpacitySet);

#ifndef Q_WS_QWS
   if (!testAttribute(Qt::WA_WState_Created)) {
      return;
   }
#endif

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

   d->setWindowOpacity_sys(opacity);
}

bool QWidget::isWindowModified() const
{
   return testAttribute(Qt::WA_WindowModified);
}

void QWidget::setWindowModified(bool mod)
{
   Q_D(QWidget);
   setAttribute(Qt::WA_WindowModified, mod);

#ifndef Q_OS_MAC
   if (!windowTitle().contains(QLatin1String("[*]")) && mod) {
      qWarning("QWidget::setWindowModified() The window title does not contain a '[*]' placeholder");
   }
#endif

   d->setWindowTitle_helper(windowTitle());
   d->setWindowIconText_helper(windowIconText());

#ifdef Q_OS_MAC
   d->setWindowModified_sys(mod);
#endif

   QEvent e(QEvent::ModifiedChange);
   QApplication::sendEvent(this, &e);
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
   QAccessible::updateAccessibility(this, 0, QAccessible::NameChanged);
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
   QAccessible::updateAccessibility(this, 0, QAccessible::DescriptionChanged);
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
   return qApp->d_func()->shortcutMap.addShortcut(this, key, context);
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
#endif // QT_NO_SHORTCUT

void QWidget::updateMicroFocus()
{
#if !defined(QT_NO_IM) && (defined(Q_WS_X11) || defined(Q_WS_QWS))
   Q_D(QWidget);
   // and optimization to update input context only it has already been created.
   if (d->assignedInputContext() || qApp->d_func()->inputContext) {
      QInputContext *ic = inputContext();
      if (ic) {
         ic->update();
      }
   }
#endif

#ifndef QT_NO_ACCESSIBILITY
   if (isVisible()) {
      // ##### is this correct
      QAccessible::updateAccessibility(this, 0, QAccessible::StateChanged);
   }
#endif
}


#if defined (Q_OS_WIN)
HDC QWidget::getDC() const
{
   Q_D(const QWidget);
   if (d->hd) {
      return (HDC) d->hd;
   }
   return GetDC(winId());
}

void QWidget::releaseDC(HDC hdc) const
{
   Q_D(const QWidget);

   // If its the widgets own dc, it will be released elsewhere. If
   // its a different HDC we release it and issue a warning if it fails.

   if (hdc != d->hd && !ReleaseDC(winId(), hdc)) {
      qErrnoWarning("QWidget::releaseDC(): failed to release HDC");
   }
}
#else

Qt::HANDLE QWidget::handle() const
{
   Q_D(const QWidget);
   if (!internalWinId() && testAttribute(Qt::WA_WState_Created)) {
      (void)winId();   // enforce native window
   }
   return d->hd;
}
#endif

void QWidget::raise()
{
   Q_D(QWidget);

   if (!isWindow()) {
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
      }

      else if (from == parentChildCount - 1) {
         return;
      }

      QRegion region(rect());
      d->subtractOpaqueSiblings(region);
      d->invalidateBuffer(region);
   }
   if (testAttribute(Qt::WA_WState_Created)) {
      d->raise_sys();
   }

   QEvent e(QEvent::ZOrderChange);
   QApplication::sendEvent(this, &e);
}

void QWidget::lower()
{
   Q_D(QWidget);
   if (!isWindow()) {
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

   QEvent e(QEvent::ZOrderChange);
   QApplication::sendEvent(this, &e);
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
      int to = p->children().indexOf(w);

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

void QWidget::styleChange(QStyle &) { }
void QWidget::enabledChange(bool) { }  // compat
void QWidget::paletteChange(const QPalette &) { }  // compat
void QWidget::fontChange(const QFont &) { }  // compat
void QWidget::windowActivationChange(bool) { }  // compat
void QWidget::languageChange() { }  // compat

QRect QWidgetPrivate::frameStrut() const
{
   Q_Q(const QWidget);

   if (!q->isWindow() || (q->windowType() == Qt::Desktop) || q->testAttribute(Qt::WA_DontShowOnScreen)) {
      // x2 = x1 + w - 1, so w/h = 1
      return QRect(0, 0, 1, 1);
   }

   if (data.fstrut_dirty

#ifndef Q_OS_WIN
         // ### Fix properly for 4.3
         && q->isVisible()
#endif
         && q->testAttribute(Qt::WA_WState_Created)) {
      const_cast<QWidgetPrivate *>(this)->updateFrameStrut();
   }

   return maybeTopData() ? maybeTopData()->frameStrut : QRect();
}

#ifdef QT_KEYPAD_NAVIGATION

// internal
bool QWidgetPrivate::navigateToDirection(Direction direction)
{
   QWidget *targetWidget = widgetInNavigationDirection(direction);
   if (targetWidget) {
      targetWidget->setFocus();
   }
   return (targetWidget != 0);
}

// internal
QWidget *QWidgetPrivate::widgetInNavigationDirection(Direction direction)
{
   const QWidget *sourceWidget = QApplication::focusWidget();
   if (!sourceWidget) {
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

   for (QWidget * targetCandidate : QApplication::allWidgets()) {

      const QRect targetCandidateRect = targetCandidate->rect().translated(targetCandidate->mapToGlobal(QPoint()));

      // For focus proxies, the child widget handling the focus can have keypad navigation focus,
      // but the owner of the proxy cannot.
      // Additionally, empty widgets should be ignored.
      if (targetCandidate->focusProxy() || targetCandidateRect.isEmpty()) {
         continue;
      }

      // Only navigate to a target widget that...
      if (       targetCandidate != sourceWidget
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

// internal
bool QWidgetPrivate::canKeypadNavigate(Qt::Orientation orientation)
{
   return orientation == Qt::Horizontal ?
          (QWidgetPrivate::widgetInNavigationDirection(QWidgetPrivate::DirectionEast)
           || QWidgetPrivate::widgetInNavigationDirection(QWidgetPrivate::DirectionWest))
          : (QWidgetPrivate::widgetInNavigationDirection(QWidgetPrivate::DirectionNorth)
             || QWidgetPrivate::widgetInNavigationDirection(QWidgetPrivate::DirectionSouth));
}

// internal
bool QWidgetPrivate::inTabWidget(QWidget *widget)
{
   for (QWidget *tabWidget = widget; tabWidget; tabWidget = tabWidget->parentWidget())
      if (qobject_cast<const QTabWidget *>(tabWidget)) {
         return true;
      }
   return false;
}
#endif

void QWidget::setWindowSurface(QWindowSurface *surface)
{
   // ### createWinId() ??

#ifndef Q_BACKINGSTORE_SUBSURFACES
   if (!isTopLevel()) {
      return;
   }
#endif

   Q_D(QWidget);

   QTLWExtra *topData = d->topData();
   if (topData->windowSurface == surface) {
      return;
   }

   QWindowSurface *oldSurface = topData->windowSurface;
   delete topData->windowSurface;
   topData->windowSurface = surface;

   QWidgetBackingStore *bs = d->maybeBackingStore();
   if (!bs) {
      return;
   }

   if (isTopLevel()) {
      if (bs->windowSurface != oldSurface && bs->windowSurface != surface) {
         delete bs->windowSurface;
      }
      bs->windowSurface = surface;
   }

#ifdef Q_BACKINGSTORE_SUBSURFACES
   else {
      bs->subSurfaces.append(surface);
   }
   bs->subSurfaces.removeOne(oldSurface);
#endif

}

QWindowSurface *QWidget::windowSurface() const
{
   Q_D(const QWidget);
   QTLWExtra *extra = d->maybeTopData();
   if (extra && extra->windowSurface) {
      return extra->windowSurface;
   }

   QWidgetBackingStore *bs = d->maybeBackingStore();

#ifdef Q_BACKINGSTORE_SUBSURFACES
   if (bs && bs->subSurfaces.isEmpty()) {
      return bs->windowSurface;
   }

   if (!isTopLevel()) {
      const QWidget *w = parentWidget();
      while (w) {
         QTLWExtra *extra = w->d_func()->maybeTopData();
         if (extra && extra->windowSurface) {
            return extra->windowSurface;
         }
         if (w->isTopLevel()) {
            break;
         }
         w = w->parentWidget();
      }
   }
#endif

   return bs ? bs->windowSurface : 0;
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
   if (leftLayoutItemMargin == left
         && topLayoutItemMargin == top
         && rightLayoutItemMargin == right
         && bottomLayoutItemMargin == bottom) {
      return;
   }

   Q_Q(QWidget);
   leftLayoutItemMargin = (signed char)left;
   topLayoutItemMargin = (signed char)top;
   rightLayoutItemMargin = (signed char)right;
   bottomLayoutItemMargin = (signed char)bottom;
   q->updateGeometry();
}

void QWidgetPrivate::setLayoutItemMargins(QStyle::SubElement element, const QStyleOption *opt)
{
   Q_Q(QWidget);
   QStyleOption myOpt;
   if (!opt) {
      myOpt.initFrom(q);
      myOpt.rect.setRect(0, 0, 32768, 32768);     // arbitrary
      opt = &myOpt;
   }

   QRect liRect = q->style()->subElementRect(element, opt, q);
   if (liRect.isValid()) {
      leftLayoutItemMargin = (signed char)(opt->rect.left() - liRect.left());
      topLayoutItemMargin = (signed char)(opt->rect.top() - liRect.top());
      rightLayoutItemMargin = (signed char)(liRect.right() - opt->rect.right());
      bottomLayoutItemMargin = (signed char)(liRect.bottom() - opt->rect.bottom());
   } else {
      leftLayoutItemMargin = 0;
      topLayoutItemMargin = 0;
      rightLayoutItemMargin = 0;
      bottomLayoutItemMargin = 0;
   }
}
// resets the Qt::WA_QuitOnClose attribute to the default value for transient widgets.
void QWidgetPrivate::adjustQuitOnCloseAttribute()
{
   Q_Q(QWidget);

   if (!q->parentWidget()) {
      Qt::WindowType type = q->windowType();
      if (type == Qt::Widget || type == Qt::SubWindow) {
         type = Qt::Window;
      }
      if (type != Qt::Widget && type != Qt::Window && type != Qt::Dialog) {
         q->setAttribute(Qt::WA_QuitOnClose, false);
      }
   }
}



Q_GUI_EXPORT QWidgetData *qt_qwidget_data(QWidget *widget)
{
   return widget->data;
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
   return 0;
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

#ifndef Q_OS_MAC
   if (! testAttribute(Qt::WA_WState_Created)) {
      return;
   }
#endif

   d->setMask_sys(newMask);

#ifndef QT_NO_BACKINGSTORE
   if (!isVisible()) {
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

   if (!isWindow()) {
      // Update newly exposed area on the parent widget.
      QRegion parentExpose(rect());
      parentExpose -= newMask;
      if (!parentExpose.isEmpty()) {
         d->setDirtyOpaqueRegion();
         parentExpose.translate(data->crect.topLeft());
         parentWidget()->update(parentExpose);
      }

      // Update newly exposed area on this widget
      if (!oldMask.isEmpty()) {
         update(newMask - oldMask);
      }
   }
#endif
}

void QWidget::hide()
{
   setVisible(false);
}

void QWidget::show()
{
   setVisible(true);
}

void QWidget::setHidden(bool hidden)
{
   setVisible(! hidden);
}

void QWidget::setMask(const QBitmap &bitmap)
{
   setMask(QRegion(bitmap));
}

void QWidget::clearMask()
{
   setMask(QRegion());
}

#ifdef Q_OS_MAC
void QWidgetPrivate::syncUnifiedMode()
{
   // Purpose of this method is to keep the unifiedToolbar in sync. Make sure we either
   // exchange the drawing methods or let the toolbar know it does not require to draw the baseline

   // This function makes sense only if this is a top level
   Q_Q(QWidget);

   if (!q->isWindow()) {
      return;
   }

   OSWindowRef window = qt_mac_window_for(q);

   if (changeMethods) {
      // Ok, we are in documentMode
      if (originalDrawMethod) {
         qt_mac_replaceDrawRect(window, this);
      }

   } else {
      if (!originalDrawMethod) {
         qt_mac_replaceDrawRectOriginal(window, this);
      }
   }
}
#endif


QWidgetPrivate *QWidgetPrivate::cs_getPrivate(QWidget *object)
{
   return object->d_ptr.data();
}

QT_END_NAMESPACE
