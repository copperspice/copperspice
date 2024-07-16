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

#include <qshortcut.h>
#include <qwidget_p.h>

#ifndef QT_NO_SHORTCUT

#include <qevent.h>
#include <qwhatsthis.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qapplication.h>

#include <qapplication_p.h>
#include <qshortcutmap_p.h>

#include <qaction_p.h>
#include <qwidgetwindow_p.h>
#include <qplatform_menu.h>

#define QAPP_CHECK(functionName) \
    if (! qApp) { \
        qWarning("QShortcut()::" functionName " QApplication must be started before calling this method"); \
        return; \
    }

static bool correctWidgetContext(Qt::ShortcutContext context, QWidget *w, QWidget *active_window);

#ifndef QT_NO_GRAPHICSVIEW
static bool correctGraphicsWidgetContext(Qt::ShortcutContext context, QGraphicsWidget *w, QWidget *active_window);
#endif

#ifndef QT_NO_ACTION
static bool correctActionContext(Qt::ShortcutContext context, QAction *a, QWidget *active_window);
#endif
bool qWidgetShortcutContextMatcher(QObject *object, Qt::ShortcutContext context)
{
   Q_ASSERT_X(object, "QShortcutMap", "Shortcut has no owner, invalid map state");

   QWidget *active_window = QApplication::activeWindow();

   // popups do not become the active window,
   // so we fake it here to get the correct context
   // for the shortcut system.
   if (QApplication::activePopupWidget()) {
      active_window = QApplication::activePopupWidget();
   }

   if (!active_window) {
      QWindow *qwindow = QGuiApplication::focusWindow();
      if (qwindow && qwindow->isActive()) {
         while (qwindow) {
            QWidgetWindow *widgetWindow = qobject_cast<QWidgetWindow *>(qwindow);
            if (widgetWindow) {
               active_window = widgetWindow->widget();
               break;
            }
            qwindow = qwindow->parent();
         }
      }
   }

   if (!active_window) {
      return false;
   }

#ifndef QT_NO_ACTION
   if (QAction *a = qobject_cast<QAction *>(object)) {
      return correctActionContext(context, a, active_window);
   }
#endif

#ifndef QT_NO_GRAPHICSVIEW
   if (QGraphicsWidget *gw = qobject_cast<QGraphicsWidget *>(object)) {
      return correctGraphicsWidgetContext(context, gw, active_window);
   }
#endif

   QWidget *w = qobject_cast<QWidget *>(object);
   if (!w) {
      QShortcut *s = qobject_cast<QShortcut *>(object);
      if (s) {
         w = s->parentWidget();
      }
   }

   if (!w) {
      QWindow *qwindow = qobject_cast<QWindow *>(object);
      while (qwindow) {
         QWidgetWindow *widget_window = qobject_cast<QWidgetWindow *>(qwindow);
         if (widget_window) {
            w = widget_window->widget();
            break;
         }
         qwindow = qwindow->parent();
      }
   }

   if (!w) {
      return false;
   }

   return correctWidgetContext(context, w, active_window);
}

static bool correctWidgetContext(Qt::ShortcutContext context, QWidget *w, QWidget *active_window)
{
   bool visible = w->isVisible();
#ifdef Q_OS_DARWIN
   if (!qApp->testAttribute(Qt::AA_DontUseNativeMenuBar) && qobject_cast<QMenuBar *>(w)) {
      visible = true;
   }
#endif

   if (!visible || !w->isEnabled()) {
      return false;
   }

   if (context == Qt::ApplicationShortcut) {
      return QApplicationPrivate::tryModalHelper(w, nullptr);   // true, unless w is shadowed by a modal dialog
   }

   if (context == Qt::WidgetShortcut) {
      return w == QApplication::focusWidget();
   }

   if (context == Qt::WidgetWithChildrenShortcut) {
      const QWidget *tw = QApplication::focusWidget();
      while (tw && tw != w && (tw->windowType() == Qt::Widget || tw->windowType() == Qt::Popup || tw->windowType() == Qt::SubWindow)) {
         tw = tw->parentWidget();
      }
      return tw == w;
   }

   // Below is Qt::WindowShortcut context
   QWidget *tlw = w->window();

#ifndef QT_NO_GRAPHICSVIEW
   if (QWExtra *topData = static_cast<QWidgetPrivate *>(QWidgetPrivate::get(tlw))->extra) {
      if (topData->proxyWidget) {
         bool res = correctGraphicsWidgetContext(context, (QGraphicsWidget *)topData->proxyWidget, active_window);
         return res;
      }
   }
#endif

   /* if a floating tool window is active, keep shortcuts on the
    * parent working */
   if (active_window != tlw && active_window && active_window->windowType() == Qt::Tool && active_window->parentWidget()) {
      active_window = active_window->parentWidget()->window();
   }

   if (active_window  != tlw) {
      return false;
   }

   /* if we live in a MDI subwindow, ignore the event if we are
      not the active document window */
   const QWidget *sw = w;
   while (sw && !(sw->windowType() == Qt::SubWindow) && !sw->isWindow()) {
      sw = sw->parentWidget();
   }
   if (sw && (sw->windowType() == Qt::SubWindow)) {
      QWidget *focus_widget = QApplication::focusWidget();
      while (focus_widget && focus_widget != sw) {
         focus_widget = focus_widget->parentWidget();
      }
      return sw == focus_widget;
   }

   return true;
}

#ifndef QT_NO_GRAPHICSVIEW
static bool correctGraphicsWidgetContext(Qt::ShortcutContext context, QGraphicsWidget *w, QWidget *active_window)
{
   bool visible = w->isVisible();
#ifdef Q_OS_DARWIN
   if (!qApp->testAttribute(Qt::AA_DontUseNativeMenuBar) && qobject_cast<QMenuBar *>(w)) {
      visible = true;
   }
#endif

   if (! visible || !w->isEnabled() || !w->scene()) {
      return false;
   }

   if (context == Qt::ApplicationShortcut) {
      // Applicationwide shortcuts are always reachable unless their owner
      // is shadowed by modality. In QGV there's no modality concept, but we
      // must still check if all views are shadowed.
      QList<QGraphicsView *> views = w->scene()->views();
      for (int i = 0; i < views.size(); ++i) {
         if (QApplicationPrivate::tryModalHelper(views.at(i), nullptr)) {
            return true;
         }
      }
      return false;
   }

   if (context == Qt::WidgetShortcut) {
      return static_cast<QGraphicsItem *>(w) == w->scene()->focusItem();
   }

   if (context == Qt::WidgetWithChildrenShortcut) {
      const QGraphicsItem *ti = w->scene()->focusItem();
      if (ti && ti->isWidget()) {
         const QGraphicsWidget *tw = static_cast<const QGraphicsWidget *>(ti);
         while (tw && tw != w && (tw->windowType() == Qt::Widget || tw->windowType() == Qt::Popup)) {
            tw = tw->parentWidget();
         }
         return tw == w;
      }
      return false;
   }

   // Below is Qt::WindowShortcut context

   // Find the active view (if any).
   QList<QGraphicsView *> views = w->scene()->views();
   QGraphicsView *activeView = nullptr;

   for (int i = 0; i < views.size(); ++i) {
      QGraphicsView *view = views.at(i);
      if (view->window() == active_window) {
         activeView = view;
         break;
      }
   }
   if (! activeView) {
      return false;
   }

   // The shortcut is reachable if owned by a windowless widget, or if the
   // widget's window is the same as the focus item's window.
   QGraphicsWidget *a = w->scene()->activeWindow();
   return !w->window() || a == w->window();
}
#endif

#ifndef QT_NO_ACTION
static bool correctActionContext(Qt::ShortcutContext context, QAction *a, QWidget *active_window)
{
   const QList<QWidget *> &widgets = static_cast<QActionPrivate *>(QActionPrivate::get(a))->widgets;

#if defined(CS_SHOW_DEBUG_GUI)
   if (widgets.isEmpty()) {
      qDebug() << "QShortCut() " << a << "is not connected to any widget, will not trigger";
   }
#endif

   for (int i = 0; i < widgets.size(); ++i) {
      QWidget *w = widgets.at(i);

#ifndef QT_NO_MENU
      if (QMenu *menu = qobject_cast<QMenu *>(w)) {

#ifdef Q_OS_DARWIN
         // On Mac, menu item shortcuts are processed before reaching any window.
         // That means that if a menu action shortcut has not been already processed
         // (and reaches this point), then the menu item itself has been disabled.
         // This occurs at the QPA level on Mac, where we disable all the Cocoa menus
         // when showing a modal window. (Notice that only the QPA menu is disabled,
         // not the QMenu.) Since we can also reach this code by climbing the menu
         // hierarchy (see below), or when the shortcut is not a key-equivalent, we
         // need to check whether the QPA menu is actually disabled.

         QPlatformMenu *pm = menu->platformMenu();
         if (pm == nullptr || ! pm->isEnabled()) {
            continue;
         }
#endif

         QAction *a = menu->menuAction();
         if (correctActionContext(context, a, active_window)) {
            return true;
         }

      } else
#endif
         if (correctWidgetContext(context, w, active_window)) {
            return true;
         }
   }

#ifndef QT_NO_GRAPHICSVIEW
   const QList<QGraphicsWidget *> &graphicsWidgets = static_cast<QActionPrivate *>(QActionPrivate::get(a))->graphicsWidgets;

#if defined(CS_SHOW_DEBUG_GUI)
   if (graphicsWidgets.isEmpty()) {
      qDebug() << "QShortCut() " << a << " is not connected to any widget, will not trigger";
   }
#endif

   for (int i = 0; i < graphicsWidgets.size(); ++i) {
      QGraphicsWidget *w = graphicsWidgets.at(i);

      if (correctGraphicsWidgetContext(context, w, active_window)) {
         return true;
      }
   }
#endif

   return false;
}
#endif // QT_NO_ACTION

class QShortcutPrivate
{
   Q_DECLARE_PUBLIC(QShortcut)

 public:
   QShortcutPrivate() : sc_context(Qt::WindowShortcut), sc_enabled(true), sc_autorepeat(true), sc_id(0) {}
   virtual ~QShortcutPrivate() {}

   QKeySequence sc_sequence;
   Qt::ShortcutContext sc_context;
   bool sc_enabled;
   bool sc_autorepeat;
   int sc_id;
   QString sc_whatsthis;
   void redoGrab(QShortcutMap &map);

 protected:
   QShortcut *q_ptr;

};

void QShortcutPrivate::redoGrab(QShortcutMap &map)
{
   Q_Q(QShortcut);

   if (! q->parent()) {
      qWarning("QShortcut::redoGrab() Parent window was not defined");
      return;
   }

   if (sc_id) {
      map.removeShortcut(sc_id, q);
   }

   if (sc_sequence.isEmpty()) {
      return;
   }

   sc_id = map.addShortcut(q, sc_sequence, sc_context, qWidgetShortcutContextMatcher);

   if (!sc_enabled) {
      map.setShortcutEnabled(false, sc_id, q);
   }

   if (!sc_autorepeat) {
      map.setShortcutAutoRepeat(false, sc_id, q);
   }
}

/*!
    Constructs a QShortcut object for the \a parent widget. Since no
    shortcut key sequence is specified, the shortcut will not emit any
    signals.

    \sa setKey()
*/
QShortcut::QShortcut(QWidget *parent)
   : QObject(parent), d_ptr(new QShortcutPrivate)
{
   d_ptr->q_ptr = this;
   Q_ASSERT(parent != nullptr);
}

QShortcut::QShortcut(const QKeySequence &key, QWidget *parent, const QString &member,
   const QString &ambiguousMember, Qt::ShortcutContext context)
   : QObject(parent), d_ptr(new QShortcutPrivate)
{
   d_ptr->q_ptr = this;
   QAPP_CHECK("QShortcut");

   Q_D(QShortcut);
   Q_ASSERT(parent != nullptr);

   d->sc_context  = context;
   d->sc_sequence = key;
   d->redoGrab(qApp->d_func()->shortcutMap);

   if (! member.isEmpty()) {
      connect(this, SIGNAL(activated()), parent, member);
   }

   if (! ambiguousMember.isEmpty()) {
      connect(this, SIGNAL(activatedAmbiguously()), parent, ambiguousMember);
   }
}

QShortcut::~QShortcut()
{
   Q_D(QShortcut);
   if (qApp) {
      qApp->d_func()->shortcutMap.removeShortcut(d->sc_id, this);
   }
}

void QShortcut::setKey(const QKeySequence &key)
{
   Q_D(QShortcut);

   if (d->sc_sequence == key) {
      return;
   }

   QAPP_CHECK("setKey");
   d->sc_sequence = key;
   d->redoGrab(qApp->d_func()->shortcutMap);
}

QKeySequence QShortcut::key() const
{
   Q_D(const QShortcut);
   return d->sc_sequence;
}

void QShortcut::setEnabled(bool enable)
{
   Q_D(QShortcut);
   if (d->sc_enabled == enable) {
      return;
   }
   QAPP_CHECK("setEnabled");
   d->sc_enabled = enable;
   qApp->d_func()->shortcutMap.setShortcutEnabled(enable, d->sc_id, this);
}

bool QShortcut::isEnabled() const
{
   Q_D(const QShortcut);
   return d->sc_enabled;
}

void QShortcut::setContext(Qt::ShortcutContext context)
{
   Q_D(QShortcut);
   if (d->sc_context == context) {
      return;
   }
   QAPP_CHECK("setContext");
   d->sc_context = context;
   d->redoGrab(qApp->d_func()->shortcutMap);
}

Qt::ShortcutContext QShortcut::context() const
{
   Q_D(const QShortcut);
   return d->sc_context;
}

void QShortcut::setWhatsThis(const QString &text)
{
   Q_D(QShortcut);
   d->sc_whatsthis = text;
}

QString QShortcut::whatsThis() const
{
   Q_D(const QShortcut);
   return d->sc_whatsthis;
}


void QShortcut::setAutoRepeat(bool on)
{
   Q_D(QShortcut);
   if (d->sc_autorepeat == on) {
      return;
   }
   QAPP_CHECK("setAutoRepeat");
   d->sc_autorepeat = on;
   qApp->d_func()->shortcutMap.setShortcutAutoRepeat(on, d->sc_id, this);
}

bool QShortcut::autoRepeat() const
{
   Q_D(const QShortcut);
   return d->sc_autorepeat;
}


int QShortcut::id() const
{
   Q_D(const QShortcut);
   return d->sc_id;
}

/*!
    \internal
*/
bool QShortcut::event(QEvent *e)
{
   Q_D(QShortcut);
   bool handled = false;

   if (d->sc_enabled && e->type() == QEvent::Shortcut) {
      QShortcutEvent *se = static_cast<QShortcutEvent *>(e);
      if (se->shortcutId() == d->sc_id && se->key() == d->sc_sequence) {

#ifndef QT_NO_WHATSTHIS
         if (QWhatsThis::inWhatsThisMode()) {
            QWhatsThis::showText(QCursor::pos(), d->sc_whatsthis);
            handled = true;
         } else
#endif
            if (se->isAmbiguous()) {
               emit activatedAmbiguously();
            } else {
               emit activated();
            }
         handled = true;
      }
   }
   return handled;
}
#endif // QT_NO_SHORTCUT

