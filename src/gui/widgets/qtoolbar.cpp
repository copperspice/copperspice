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

#include <qtoolbar.h>

#ifndef QT_NO_TOOLBAR

#include <qapplication.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qlayout.h>
#include <qmainwindow.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qrubberband.h>
#include <qsignalmapper.h>
#include <qstylepainter.h>
#include <qtoolbutton.h>
#include <qwidgetaction.h>
#include <qtimer.h>
#include <qwidgetaction_p.h>

#ifdef Q_OS_MAC
#include <qt_mac_p.h>
#include <qt_cocoa_helpers_mac_p.h>
#endif

#include <qmainwindowlayout_p.h>

#include <qtoolbar_p.h>
#include <qtoolbarseparator_p.h>
#include <qtoolbarlayout_p.h>
#include <qdebug.h>

#define POPUP_TIMER_INTERVAL 500

QT_BEGIN_NAMESPACE

#ifdef Q_OS_MAC
static void qt_mac_updateToolBarButtonHint(QWidget *parentWidget)
{
   if (!(parentWidget->windowFlags() & Qt::CustomizeWindowHint)) {
      parentWidget->setWindowFlags(parentWidget->windowFlags() | Qt::MacWindowToolBarButtonHint);
   }
}
#endif

// qmainwindow.cpp
extern QMainWindowLayout *qt_mainwindow_layout(const QMainWindow *window);

/******************************************************************************
** QToolBarPrivate
*/

void QToolBarPrivate::init()
{
   Q_Q(QToolBar);
   q->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
   q->setBackgroundRole(QPalette::Button);
   q->setAttribute(Qt::WA_Hover);
   q->setAttribute(Qt::WA_X11NetWmWindowTypeToolBar);

   QStyle *style = q->style();
   int e = style->pixelMetric(QStyle::PM_ToolBarIconSize, 0, q);
   iconSize = QSize(e, e);

   layout = new QToolBarLayout(q);
   layout->updateMarginAndSpacing();

#ifdef Q_OS_MAC
   if (q->parentWidget() && q->parentWidget()->isWindow()) {
      // Make sure that the window has the "toolbar" button.
      QWidget *parentWidget = q->parentWidget();
      qt_mac_updateToolBarButtonHint(parentWidget);
      reinterpret_cast<QToolBar *>(parentWidget)->d_func()->createWinId(); // Please let me create your winId...
      extern OSWindowRef qt_mac_window_for(const QWidget *); // qwidget_mac.cpp
      macWindowToolbarShow(q->parentWidget(), true);
   }
#endif

   toggleViewAction = new QAction(q);
   toggleViewAction->setCheckable(true);
   q->setMovable(q->style()->styleHint(QStyle::SH_ToolBar_Movable, 0, q ));
   QObject::connect(toggleViewAction, SIGNAL(triggered(bool)), q, SLOT(_q_toggleView(bool)));
}

void QToolBarPrivate::_q_toggleView(bool b)
{
   Q_Q(QToolBar);
   if (b == q->isHidden()) {
      if (b) {
         q->show();
      } else {
         q->close();
      }
   }
}

void QToolBarPrivate::_q_updateIconSize(const QSize &sz)
{
   Q_Q(QToolBar);
   if (!explicitIconSize) {
      // iconSize not explicitly set
      q->setIconSize(sz);
      explicitIconSize = false;
   }
}

void QToolBarPrivate::_q_updateToolButtonStyle(Qt::ToolButtonStyle style)
{
   Q_Q(QToolBar);
   if (!explicitToolButtonStyle) {
      q->setToolButtonStyle(style);
      explicitToolButtonStyle = false;
   }
}

void QToolBarPrivate::updateWindowFlags(bool floating, bool unplug)
{
   Q_Q(QToolBar);
   Qt::WindowFlags flags = floating ? Qt::Tool : Qt::Widget;

   flags |= Qt::FramelessWindowHint;

   if (unplug) {
      flags |= Qt::X11BypassWindowManagerHint;
#ifdef Q_OS_MAC
      flags |= Qt::WindowStaysOnTopHint;
#endif
   }

   q->setWindowFlags(flags);
}

void QToolBarPrivate::setWindowState(bool floating, bool unplug, const QRect &rect)
{
   Q_Q(QToolBar);
   bool visible = !q->isHidden();
   bool wasFloating = q->isFloating(); // ...is also currently using popup menus

   q->hide();

   updateWindowFlags(floating, unplug);

   if (floating != wasFloating) {
      layout->checkUsePopupMenu();
   }

   if (!rect.isNull()) {
      q->setGeometry(rect);
   }

   if (visible) {
      q->show();
   }

   if (floating != wasFloating) {
      emit q->topLevelChanged(floating);
   }
}

void QToolBarPrivate::initDrag(const QPoint &pos)
{
   Q_Q(QToolBar);

   if (state != 0) {
      return;
   }

   QMainWindow *win = qobject_cast<QMainWindow *>(q->parent());
   Q_ASSERT(win != 0);

   QMainWindowLayout *layout = qt_mainwindow_layout(win);
   Q_ASSERT(layout != 0);

   if (layout->pluggingWidget != 0) { // the main window is animating a docking operation
      return;
   }

   state = new DragState;
   state->pressPos = pos;
   state->dragging = false;
   state->moving = false;
   state->widgetItem = 0;

   if (q->isRightToLeft()) {
      state->pressPos = QPoint(q->width() - state->pressPos.x(), state->pressPos.y());
   }
}

void QToolBarPrivate::startDrag(bool moving)
{
   Q_Q(QToolBar);

   Q_ASSERT(state != 0);

   if ((moving && state->moving) || state->dragging) {
      return;
   }

   QMainWindow *win = qobject_cast<QMainWindow *>(q->parent());
   Q_ASSERT(win != 0);

   QMainWindowLayout *layout = qt_mainwindow_layout(win);
   Q_ASSERT(layout != 0);

   if (!moving) {
      state->widgetItem = layout->unplug(q);
      Q_ASSERT(state->widgetItem != 0);
   }

   state->dragging = !moving;
   state->moving = moving;
}

void QToolBarPrivate::endDrag()
{
   Q_Q(QToolBar);
   Q_ASSERT(state != 0);

   q->releaseMouse();

   if (state->dragging) {
      QMainWindowLayout *layout = qt_mainwindow_layout(qobject_cast<QMainWindow *>(q->parentWidget()));
      Q_ASSERT(layout != 0);

      if (!layout->plug(state->widgetItem)) {
         if (q->isFloatable()) {
            layout->restore();

#if defined(Q_WS_X11) || defined(Q_OS_MAC)
            setWindowState(true); // gets rid of the X11BypassWindowManager window flag
            // and activates the resizer
#endif
            q->activateWindow();
         } else {
            layout->revert(state->widgetItem);
         }
      }
   }

   delete state;
   state = 0;
}

bool QToolBarPrivate::mousePressEvent(QMouseEvent *event)
{
   Q_Q(QToolBar);

   QStyleOptionToolBar opt;
   q->initStyleOption(&opt);

   if (q->style()->subElementRect(QStyle::SE_ToolBarHandle, &opt, q).contains(event->pos()) == false) {
#ifdef Q_OS_MAC
      // When using the unified toolbar on Mac OS X the user can can click and
      // drag between toolbar contents to move the window. Make this work by
      // implementing the standard mouse-dragging code and then call
      // window->move() in mouseMoveEvent below.
      if (QMainWindow *mainWindow = qobject_cast<QMainWindow *>(q->parent())) {
         if (mainWindow->toolBarArea(q) == Qt::TopToolBarArea
               && mainWindow->unifiedTitleAndToolBarOnMac()
               && q->childAt(event->pos()) == 0) {
            macWindowDragging = true;
            macWindowDragPressPosition = event->pos();
            return true;
         }
      }
#endif
      return false;
   }

   if (event->button() != Qt::LeftButton) {
      return true;
   }

   if (!layout->movable()) {
      return true;
   }

   initDrag(event->pos());
   return true;
}

bool QToolBarPrivate::mouseReleaseEvent(QMouseEvent *)
{
   if (state != 0) {
      endDrag();
      return true;
   } else {
#ifdef Q_OS_MAC
      if (!macWindowDragging) {
         return false;
      }
      macWindowDragging = false;
      macWindowDragPressPosition = QPoint();
      return true;
#endif
      return false;
   }
}

bool QToolBarPrivate::mouseMoveEvent(QMouseEvent *event)
{
   Q_Q(QToolBar);

   if (!state) {

#ifdef Q_OS_MAC
      if (!macWindowDragging) {
         return false;
      }
      QWidget *w = q->window();
      const QPoint delta = event->pos() - macWindowDragPressPosition;
      w->move(w->pos() + delta);
      return true;
#endif

      return false;
   }

   QMainWindow *win = qobject_cast<QMainWindow *>(q->parent());
   if (win == 0) {
      return true;
   }

   QMainWindowLayout *layout = qt_mainwindow_layout(win);
   Q_ASSERT(layout != 0);

   if (layout->pluggingWidget == 0
         && (event->pos() - state->pressPos).manhattanLength() > QApplication::startDragDistance()) {
      const bool wasDragging = state->dragging;
      const bool moving = !q->isWindow() && (orientation == Qt::Vertical ?
                                             event->x() >= 0 && event->x() < q->width() :
                                             event->y() >= 0 && event->y() < q->height());

      startDrag(moving);
      if (!moving && !wasDragging) {
#ifdef Q_OS_WIN
         grabMouseWhileInWindow();
#else
         q->grabMouse();
#endif
      }
   }

   if (state->dragging) {
      QPoint pos = event->globalPos();
      // if we are right-to-left, we move so as to keep the right edge the same distance
      // from the mouse
      if (q->isLeftToRight()) {
         pos -= state->pressPos;
      } else {
         pos += QPoint(state->pressPos.x() - q->width(), -state->pressPos.y());
      }

      q->move(pos);
      layout->hover(state->widgetItem, event->globalPos());
   } else if (state->moving) {

      const QPoint rtl(q->width() - state->pressPos.x(), state->pressPos.y()); //for RTL
      const QPoint globalPressPos = q->mapToGlobal(q->isRightToLeft() ? rtl : state->pressPos);
      int pos = 0;

      QPoint delta = event->globalPos() - globalPressPos;
      if (orientation == Qt::Vertical) {
         pos = q->y() + delta.y();
      } else {
         if (q->isRightToLeft()) {
            pos = win->width() - q->width() - q->x()  - delta.x();
         } else {
            pos = q->x() + delta.x();
         }
      }

      layout->moveToolBar(q, pos);
   }
   return true;
}

void QToolBarPrivate::unplug(const QRect &_r)
{
   Q_Q(QToolBar);
   QRect r = _r;
   r.moveTopLeft(q->mapToGlobal(QPoint(0, 0)));
   setWindowState(true, true, r);
   layout->setExpanded(false);
}

void QToolBarPrivate::plug(const QRect &r)
{
   setWindowState(false, false, r);
}

QToolBar::QToolBar(QWidget *parent)
   : QWidget(*new QToolBarPrivate, parent, 0)
{
   Q_D(QToolBar);
   d->init();
}

QToolBar::QToolBar(const QString &title, QWidget *parent)
   : QWidget(*new QToolBarPrivate, parent, 0)
{
   Q_D(QToolBar);
   d->init();
   setWindowTitle(title);
}

QToolBar::~QToolBar()
{
   // Remove the toolbar button if there is nothing left.
   QMainWindow *mainwindow = qobject_cast<QMainWindow *>(parentWidget());

   if (mainwindow) {
#ifdef Q_OS_MAC
      QMainWindowLayout *mainwin_layout = qt_mainwindow_layout(mainwindow);

      if (mainwin_layout && mainwin_layout->layoutState.toolBarAreaLayout.isEmpty()
            && mainwindow->testAttribute(Qt::WA_WState_Created)) {
         macWindowToolbarShow(mainwindow, false);
      }
#endif
   }
}

void QToolBar::setMovable(bool movable)
{
   Q_D(QToolBar);

   if (!movable == !d->movable) {
      return;
   }

   d->movable = movable;
   d->layout->invalidate();
   emit movableChanged(d->movable);
}

bool QToolBar::isMovable() const
{
   Q_D(const QToolBar);
   return d->movable;
}

bool QToolBar::isFloatable() const
{
   Q_D(const QToolBar);
   return d->floatable;
}

void QToolBar::setFloatable(bool floatable)
{
   Q_D(QToolBar);
   d->floatable = floatable;
}

bool QToolBar::isFloating() const
{
   return isWindow();
}

void QToolBar::setAllowedAreas(Qt::ToolBarAreas areas)
{
   Q_D(QToolBar);
   areas &= Qt::ToolBarArea_Mask;
   if (areas == d->allowedAreas) {
      return;
   }
   d->allowedAreas = areas;
   emit allowedAreasChanged(d->allowedAreas);
}

Qt::ToolBarAreas QToolBar::allowedAreas() const
{
   Q_D(const QToolBar);

#ifdef Q_OS_MAC
   if (QMainWindow *window = qobject_cast<QMainWindow *>(parentWidget())) {
      if (window->unifiedTitleAndToolBarOnMac()) { // Don't allow drags to the top (for now).
         return (d->allowedAreas & ~Qt::TopToolBarArea);
      }
   }
#endif

   return d->allowedAreas;
}

void QToolBar::setOrientation(Qt::Orientation orientation)
{
   Q_D(QToolBar);
   if (orientation == d->orientation) {
      return;
   }

   d->orientation = orientation;

   if (orientation == Qt::Vertical) {
      setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
   } else {
      setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
   }

   d->layout->invalidate();
   d->layout->activate();

   emit orientationChanged(d->orientation);
}

Qt::Orientation QToolBar::orientation() const
{
   Q_D(const QToolBar);
   return d->orientation;
}

QSize QToolBar::iconSize() const
{
   Q_D(const QToolBar);
   return d->iconSize;
}

void QToolBar::setIconSize(const QSize &iconSize)
{
   Q_D(QToolBar);
   QSize sz = iconSize;
   if (!sz.isValid()) {
      QMainWindow *mw = qobject_cast<QMainWindow *>(parentWidget());
      if (mw && mw->layout()) {
         QLayout *layout = mw->layout();
         int i = 0;
         QLayoutItem *item = 0;
         do {
            item = layout->itemAt(i++);
            if (item && (item->widget() == this)) {
               sz = mw->iconSize();
            }
         } while (!sz.isValid() && item != 0);
      }
   }
   if (!sz.isValid()) {
      const int metric = style()->pixelMetric(QStyle::PM_ToolBarIconSize, 0, this);
      sz = QSize(metric, metric);
   }
   if (d->iconSize != sz) {
      d->iconSize = sz;
      setMinimumSize(0, 0);
      emit iconSizeChanged(d->iconSize);
   }
   d->explicitIconSize = iconSize.isValid();

   d->layout->invalidate();
}

Qt::ToolButtonStyle QToolBar::toolButtonStyle() const
{
   Q_D(const QToolBar);
   return d->toolButtonStyle;
}

void QToolBar::setToolButtonStyle(Qt::ToolButtonStyle toolButtonStyle)
{
   Q_D(QToolBar);
   d->explicitToolButtonStyle = true;

   if (d->toolButtonStyle == toolButtonStyle) {
      return;
   }

   d->toolButtonStyle = toolButtonStyle;
   setMinimumSize(0, 0);
   emit toolButtonStyleChanged(d->toolButtonStyle);
}

void QToolBar::clear()
{
   QList<QAction *> actions = this->actions();

   for (int i = 0; i < actions.size(); i++) {
      removeAction(actions.at(i));
   }
}

QAction *QToolBar::addAction(const QString &text)
{
   QAction *action = new QAction(text, this);
   addAction(action);
   return action;
}

QAction *QToolBar::addAction(const QIcon &icon, const QString &text)
{
   QAction *action = new QAction(icon, text, this);
   addAction(action);
   return action;
}

QAction *QToolBar::addAction(const QString &text, const QObject *receiver, const char *member)
{
   QAction *action = new QAction(text, this);
   QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
   addAction(action);
   return action;
}

QAction *QToolBar::addAction(const QIcon &icon, const QString &text,
                             const QObject *receiver, const char *member)
{
   QAction *action = new QAction(icon, text, this);
   QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
   addAction(action);
   return action;
}

QAction *QToolBar::addSeparator()
{
   QAction *action = new QAction(this);
   action->setSeparator(true);
   addAction(action);
   return action;
}

QAction *QToolBar::insertSeparator(QAction *before)
{
   QAction *action = new QAction(this);
   action->setSeparator(true);
   insertAction(before, action);
   return action;
}

QAction *QToolBar::addWidget(QWidget *widget)
{
   QWidgetAction *action = new QWidgetAction(this);
   action->setDefaultWidget(widget);
   action->d_func()->autoCreated = true;
   addAction(action);
   return action;
}

QAction *QToolBar::insertWidget(QAction *before, QWidget *widget)
{
   QWidgetAction *action = new QWidgetAction(this);
   action->setDefaultWidget(widget);
   action->d_func()->autoCreated = true;
   insertAction(before, action);
   return action;
}

QRect QToolBar::actionGeometry(QAction *action) const
{
   Q_D(const QToolBar);

   int index = d->layout->indexOf(action);
   if (index == -1) {
      return QRect();
   }
   return d->layout->itemAt(index)->widget()->geometry();
}

QAction *QToolBar::actionAt(const QPoint &p) const
{
   Q_D(const QToolBar);
   QWidget *widget = childAt(p);
   int index = d->layout->indexOf(widget);
   if (index == -1) {
      return 0;
   }
   QLayoutItem *item = d->layout->itemAt(index);
   return static_cast<QToolBarItem *>(item)->action;
}

void QToolBar::actionEvent(QActionEvent *event)
{
   Q_D(QToolBar);
   QAction *action = event->action();
   QWidgetAction *widgetAction = qobject_cast<QWidgetAction *>(action);

   switch (event->type()) {
      case QEvent::ActionAdded: {
         Q_ASSERT_X(widgetAction == 0 || d->layout->indexOf(widgetAction) == -1,
                    "QToolBar", "Widgets can not be inserted multiple times");

         // reparent the action to this toolbar if it has been created
         // using the addAction(text) etc. convenience functions, to
         // preserve Qt 4.1.x behavior. The widget is already
         // reparented to us due to the createWidget call inside
         // createItem()
         if (widgetAction != 0 && widgetAction->d_func()->autoCreated) {
            widgetAction->setParent(this);
         }

         int index = d->layout->count();
         if (event->before()) {
            index = d->layout->indexOf(event->before());
            Q_ASSERT_X(index != -1, "QToolBar::insertAction", "internal error");
         }
         d->layout->insertAction(index, action);
         break;
      }

      case QEvent::ActionChanged:
         d->layout->invalidate();
         break;

      case QEvent::ActionRemoved: {
         int index = d->layout->indexOf(action);
         if (index != -1) {
            delete d->layout->takeAt(index);
         }
         break;
      }

      default:
         Q_ASSERT_X(false, "QToolBar::actionEvent", "internal error");
   }
}

void QToolBar::changeEvent(QEvent *event)
{
   Q_D(QToolBar);
   switch (event->type()) {
      case QEvent::WindowTitleChange:
         d->toggleViewAction->setText(windowTitle());
         break;
      case QEvent::StyleChange:
         d->layout->invalidate();
         if (!d->explicitIconSize) {
            setIconSize(QSize());
         }
         d->layout->updateMarginAndSpacing();
         break;
      case QEvent::LayoutDirectionChange:
         d->layout->invalidate();
         break;
      default:
         break;
   }
   QWidget::changeEvent(event);
}

void QToolBar::paintEvent(QPaintEvent *)
{
   Q_D(QToolBar);

   QPainter p(this);
   QStyle *style = this->style();
   QStyleOptionToolBar opt;
   initStyleOption(&opt);

   if (d->layout->expanded || d->layout->animating || isWindow()) {
      //if the toolbar is expended, we need to fill the background with the window color
      //because some styles may expects that.
      p.fillRect(opt.rect, palette().background());
      style->drawControl(QStyle::CE_ToolBar, &opt, &p, this);
      style->drawPrimitive(QStyle::PE_FrameMenu, &opt, &p, this);
   } else {
      style->drawControl(QStyle::CE_ToolBar, &opt, &p, this);
   }

   opt.rect = style->subElementRect(QStyle::SE_ToolBarHandle, &opt, this);
   if (opt.rect.isValid()) {
      style->drawPrimitive(QStyle::PE_IndicatorToolBarHandle, &opt, &p, this);
   }
}

/*
    Checks if an expanded toolbar has to wait for this popup to close before
    the toolbar collapses. This is true if
    1) the popup has the toolbar in its parent chain,
    2) the popup is a menu whose menuAction is somewhere in the toolbar.
*/
static bool waitForPopup(QToolBar *tb, QWidget *popup)
{
   if (popup == 0 || popup->isHidden()) {
      return false;
   }

   QWidget *w = popup;
   while (w != 0) {
      if (w == tb) {
         return true;
      }
      w = w->parentWidget();
   }

   QMenu *menu = qobject_cast<QMenu *>(popup);
   if (menu == 0) {
      return false;
   }

   QAction *action = menu->menuAction();
   QList<QWidget *> widgets = action->associatedWidgets();

   for (int i = 0; i < widgets.count(); ++i) {
      if (waitForPopup(tb, widgets.at(i))) {
         return true;
      }
   }

   return false;
}

#if defined(Q_OS_MAC)
static bool toolbarInUnifiedToolBar(QToolBar *toolbar)
{
   const QMainWindow *mainWindow = qobject_cast<const QMainWindow *>(toolbar->parentWidget());
   return mainWindow && mainWindow->unifiedTitleAndToolBarOnMac()
          && mainWindow->toolBarArea(toolbar) == Qt::TopToolBarArea;
}
#endif

bool QToolBar::event(QEvent *event)
{
   Q_D(QToolBar);

   switch (event->type()) {
      case QEvent::Timer:
         if (d->waitForPopupTimer.timerId() == static_cast<QTimerEvent *>(event)->timerId()) {
            QWidget *w = QApplication::activePopupWidget();
            if (!waitForPopup(this, w)) {
               d->waitForPopupTimer.stop();
               if (!this->underMouse()) {
                  d->layout->setExpanded(false);
               }
            }
         }
         break;

      case QEvent::Hide:
         if (!isHidden()) {
            break;
         }

      // fallthrough intended

      case QEvent::Show:
         d->toggleViewAction->setChecked(event->type() == QEvent::Show);
         emit visibilityChanged(event->type() == QEvent::Show);

#if defined(Q_OS_MAC)
         if (toolbarInUnifiedToolBar(this)) {
            // I can static_cast because I did the qobject_cast in the if above, therefore
            // we must have a QMainWindowLayout here.
            QMainWindowLayout *mwLayout = qt_mainwindow_layout(qobject_cast<QMainWindow *>(parentWidget()));
            mwLayout->fixSizeInUnifiedToolbar(this);
            mwLayout->syncUnifiedToolbarVisibility();
         }
#endif

         break;

      case QEvent::ParentChange:
         d->layout->checkUsePopupMenu();

#if defined(Q_OS_MAC)
         if (parentWidget() && parentWidget()->isWindow()) {
            qt_mac_updateToolBarButtonHint(parentWidget());
         }
#endif
         break;

      case QEvent::MouseButtonPress: {
         if (d->mousePressEvent(static_cast<QMouseEvent *>(event))) {
            return true;
         }
         break;
      }
      case QEvent::MouseButtonRelease:
         if (d->mouseReleaseEvent(static_cast<QMouseEvent *>(event))) {
            return true;
         }
         break;

      case QEvent::HoverEnter:
      case QEvent::HoverLeave:
         // there's nothing special to do here and we don't want to update the whole widget
         return true;

      case QEvent::HoverMove: {
#ifndef QT_NO_CURSOR
         QHoverEvent *e = static_cast<QHoverEvent *>(event);
         QStyleOptionToolBar opt;
         initStyleOption(&opt);
         if (style()->subElementRect(QStyle::SE_ToolBarHandle, &opt, this).contains(e->pos())) {
            setCursor(Qt::SizeAllCursor);
         } else {
            unsetCursor();
         }
#endif
         break;
      }
      case QEvent::MouseMove:
         if (d->mouseMoveEvent(static_cast<QMouseEvent *>(event))) {
            return true;
         }
         break;

      case QEvent::Leave:
         if (d->state != 0 && d->state->dragging) {

#ifdef Q_OS_WIN
            // This is a workaround for loosing the mouse on Vista.
            QPoint pos = QCursor::pos();
            QMouseEvent fake(QEvent::MouseMove, mapFromGlobal(pos), pos, Qt::NoButton,
                             QApplication::mouseButtons(), QApplication::keyboardModifiers());
            d->mouseMoveEvent(&fake);
#endif

         } else {
            if (!d->layout->expanded) {
               break;
            }

            QWidget *w = QApplication::activePopupWidget();
            if (waitForPopup(this, w)) {
               d->waitForPopupTimer.start(POPUP_TIMER_INTERVAL, this);
               break;
            }

            d->waitForPopupTimer.stop();
            d->layout->setExpanded(false);
            break;
         }
      default:
         break;
   }
   return QWidget::event(event);
}

/*!
    Returns a checkable action that can be used to show or hide this
    toolbar.

    The action's text is set to the toolbar's window title.

    \sa QAction::text QWidget::windowTitle
*/
QAction *QToolBar::toggleViewAction() const
{
   Q_D(const QToolBar);
   return d->toggleViewAction;
}

QWidget *QToolBar::widgetForAction(QAction *action) const
{
   Q_D(const QToolBar);

   int index = d->layout->indexOf(action);
   if (index == -1) {
      return 0;
   }

   return d->layout->itemAt(index)->widget();
}

extern QMainWindowLayout *qt_mainwindow_layout(const QMainWindow *window);

/*!
    \internal
*/
void QToolBar::initStyleOption(QStyleOptionToolBar *option) const
{
   Q_D(const QToolBar);

   if (!option) {
      return;
   }

   option->initFrom(this);

   if (orientation() == Qt::Horizontal) {
      option->state |= QStyle::State_Horizontal;
   }

   option->lineWidth = style()->pixelMetric(QStyle::PM_ToolBarFrameWidth, 0, this);
   option->features = d->layout->movable()
                      ? QStyleOptionToolBar::Movable
                      : QStyleOptionToolBar::None;

   // if the tool bar is not in a QMainWindow, this will make the painting right
   option->toolBarArea = Qt::NoToolBarArea;

   // Add more styleoptions if the toolbar has been added to a mainwindow.
   QMainWindow *mainWindow = qobject_cast<QMainWindow *>(parentWidget());

   if (!mainWindow) {
      return;
   }

   QMainWindowLayout *layout = qt_mainwindow_layout(mainWindow);
   Q_ASSERT_X(layout != 0, "QToolBar::initStyleOption()",
              "QMainWindow->layout() != QMainWindowLayout");

   layout->getStyleOptionInfo(option, const_cast<QToolBar *>(this));
}

void QToolBar::_q_toggleView(bool un_named_arg1)
{
   Q_D(QToolBar);
   d->_q_toggleView(un_named_arg1);
}

void QToolBar::_q_updateIconSize(const QSize &un_named_arg1)
{
   Q_D(QToolBar);
   d->_q_updateIconSize(un_named_arg1);
}

void QToolBar::_q_updateToolButtonStyle(Qt::ToolButtonStyle un_named_arg1)
{
   Q_D(QToolBar);
   d->_q_updateToolButtonStyle(un_named_arg1);
}

bool QToolBar::cs_isMainWindow() const
{
   return (qobject_cast<QMainWindow *>(parentWidget()) != 0);
}

QT_END_NAMESPACE

#endif // QT_NO_TOOLBAR
