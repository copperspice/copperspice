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
#include <qmainwindowlayout_p.h>

#ifdef Q_OS_DARWIN
#include <qplatform_nativeinterface.h>
#endif

#include <qtoolbar_p.h>
#include <qtoolbarseparator_p.h>
#include <qtoolbarlayout_p.h>
#include <qdebug.h>

#define POPUP_TIMER_INTERVAL 500

// qmainwindow.cpp
extern QMainWindowLayout *qt_mainwindow_layout(const QMainWindow *window);

void QToolBarPrivate::init()
{
   Q_Q(QToolBar);

   q->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
   q->setBackgroundRole(QPalette::Button);
   q->setAttribute(Qt::WA_Hover);
   q->setAttribute(Qt::WA_X11NetWmWindowTypeToolBar);
   q->setProperty("_q_platform_MacUseNSWindow", QVariant(true));

   QStyle *style = q->style();
   int e = style->pixelMetric(QStyle::PM_ToolBarIconSize, nullptr, q);
   iconSize = QSize(e, e);

   layout = new QToolBarLayout(q);
   layout->updateMarginAndSpacing();

   toggleViewAction = new QAction(q);
   toggleViewAction->setCheckable(true);
   q->setMovable(q->style()->styleHint(QStyle::SH_ToolBar_Movable, nullptr, q ));

   QObject::connect(toggleViewAction, &QAction::triggered, q, &QToolBar::_q_toggleView);
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

   if (state != nullptr) {
      return;
   }

   QMainWindow *win = qobject_cast<QMainWindow *>(q->parent());
   Q_ASSERT(win != nullptr);

   QMainWindowLayout *layout = qt_mainwindow_layout(win);
   Q_ASSERT(layout != nullptr);

   if (layout->pluggingWidget != nullptr) {
      // the main window is animating a docking operation
      return;
   }

   state             = new DragState;
   state->pressPos   = pos;
   state->dragging   = false;
   state->moving     = false;
   state->widgetItem = nullptr;

   if (q->isRightToLeft()) {
      state->pressPos = QPoint(q->width() - state->pressPos.x(), state->pressPos.y());
   }
}

void QToolBarPrivate::startDrag(bool moving)
{
   Q_Q(QToolBar);

   Q_ASSERT(state != nullptr);

   if ((moving && state->moving) || state->dragging) {
      return;
   }

   QMainWindow *win = qobject_cast<QMainWindow *>(q->parent());
   Q_ASSERT(win != nullptr);

   QMainWindowLayout *layout = qt_mainwindow_layout(win);
   Q_ASSERT(layout != nullptr);

   if (!moving) {
      state->widgetItem = layout->unplug(q);
      Q_ASSERT(state->widgetItem != nullptr);
   }

   state->dragging = !moving;
   state->moving   = moving;
}

void QToolBarPrivate::endDrag()
{
   Q_Q(QToolBar);
   Q_ASSERT(state != nullptr);

   q->releaseMouse();

   if (state->dragging) {
      QMainWindowLayout *layout = qt_mainwindow_layout(qobject_cast<QMainWindow *>(q->parentWidget()));
      Q_ASSERT(layout != nullptr);

      if (!layout->plug(state->widgetItem)) {
         if (q->isFloatable()) {
            layout->restore();

            setWindowState(true); // gets rid of the X11BypassWindowManager window flag
            // and activates the resizer

            q->activateWindow();
         } else {
            layout->revert(state->widgetItem);
         }
      }
   }

   delete state;
   state = nullptr;
}

bool QToolBarPrivate::mousePressEvent(QMouseEvent *event)
{
   Q_Q(QToolBar);

   QStyleOptionToolBar opt;
   q->initStyleOption(&opt);

   if (q->style()->subElementRect(QStyle::SE_ToolBarHandle, &opt, q).contains(event->pos()) == false) {
#ifdef Q_OS_DARWIN
      // When using the unified toolbar on Mac OS X the user can can click and
      // drag between toolbar contents to move the window. Make this work by
      // implementing the standard mouse-dragging code and then call
      // window->move() in mouseMoveEvent below.
      if (QMainWindow *mainWindow = qobject_cast<QMainWindow *>(q->parent())) {
         if (mainWindow->toolBarArea(q) == Qt::TopToolBarArea
            && mainWindow->unifiedTitleAndToolBarOnMac()
            && q->childAt(event->pos()) == nullptr) {
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
   if (state != nullptr) {
      endDrag();
      return true;

   } else {

#ifdef Q_OS_DARWIN
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

#ifdef Q_OS_DARWIN
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
   if (win == nullptr) {
      return true;
   }

   QMainWindowLayout *layout = qt_mainwindow_layout(win);
   Q_ASSERT(layout != nullptr);

   if (layout->pluggingWidget == nullptr
               && (event->pos() - state->pressPos).manhattanLength() > QApplication::startDragDistance()) {

      const bool wasDragging = state->dragging;
      const bool moving = !q->isWindow() && (orientation == Qt::Vertical ?
            event->x() >= 0 && event->x() < q->width() : event->y() >= 0 && event->y() < q->height());

      startDrag(moving);
      if (!moving && !wasDragging) {

         q->grabMouse();
      }
   }

   if (state->dragging) {
      QPoint pos = event->globalPos();
      // if we are right-to-left, we move so as to keep the right edge the same distance from the mouse
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
   : QWidget(*new QToolBarPrivate, parent, Qt::EmptyFlag)
{
   Q_D(QToolBar);
   d->init();
}

QToolBar::QToolBar(const QString &title, QWidget *parent)
   : QWidget(*new QToolBarPrivate, parent, Qt::EmptyFlag)
{
   Q_D(QToolBar);
   d->init();
   setWindowTitle(title);
}

QToolBar::~QToolBar()
{
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
         QLayoutItem *item = nullptr;
         do {
            item = layout->itemAt(i++);
            if (item && (item->widget() == this)) {
               sz = mw->iconSize();
            }
         } while (!sz.isValid() && item != nullptr);
      }
   }
   if (!sz.isValid()) {
      const int metric = style()->pixelMetric(QStyle::PM_ToolBarIconSize, nullptr, this);
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

QAction *QToolBar::addAction(const QString &text, const QObject *receiver, const QString &member)
{
   QAction *action = new QAction(text, this);
   QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
   addAction(action);

   return action;
}

QAction *QToolBar::addAction(const QIcon &icon, const QString &text,
   const QObject *receiver, const QString &member)
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
      return nullptr;
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
         Q_ASSERT_X(widgetAction == nullptr || d->layout->indexOf(widgetAction) == -1,
            "QToolBar", "Widgets can not be inserted multiple times");

         // reparent the action to this toolbar if it has been created
         // using the addAction(text) etc. convenience functions to preserve backwards compatibility
         // The widget is already reparented to us due to the createWidget call inside createItem()

         if (widgetAction != nullptr && widgetAction->d_func()->autoCreated) {
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
   if (popup == nullptr || popup->isHidden()) {
      return false;
   }

   QWidget *w = popup;
   while (w != nullptr) {
      if (w == tb) {
         return true;
      }
      w = w->parentWidget();
   }

   QMenu *menu = qobject_cast<QMenu *>(popup);
   if (menu == nullptr) {
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

#ifdef Q_OS_DARWIN
static void enableMacToolBar(QToolBar *toolbar, bool enable)
{
   QPlatformNativeInterface *nativeInterface = QApplication::platformNativeInterface();
   QPlatformNativeInterface::FP_Integration function =
      nativeInterface->nativeResourceFunctionForIntegration("setContentBorderAreaEnabled");

   if (! function) {
      return;   // Not Cocoa platform plugin.
   }

   typedef void (*SetContentBorderAreaEnabledFunction)(QWindow * window, void *identifier, bool enabled);
   (reinterpret_cast<SetContentBorderAreaEnabledFunction>(function))(toolbar->window()->windowHandle(), toolbar, enable);
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
         if (! isHidden()) {
            break;
         }
         [[fallthrough]];

      case QEvent::Show:
         d->toggleViewAction->setChecked(event->type() == QEvent::Show);
#ifdef Q_OS_DARWIN
         enableMacToolBar(this, event->type() == QEvent::Show);
#endif
         emit visibilityChanged(event->type() == QEvent::Show);
         break;

      case QEvent::ParentChange:
         d->layout->checkUsePopupMenu();
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
         // there is nothing special to do here and we do not want to update the whole widget
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
         if (d->state != nullptr && d->state->dragging) {

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
      return nullptr;
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

   option->lineWidth = style()->pixelMetric(QStyle::PM_ToolBarFrameWidth, nullptr, this);
   option->features = d->layout->movable()
      ? QStyleOptionToolBar::Movable
      : QStyleOptionToolBar::None;

   // if the tool bar is not in a QMainWindow, this will make the painting right
   option->toolBarArea = Qt::NoToolBarArea;

   // Add more styleoptions if the toolbar has been added to a mainwindow.
   QMainWindow *mainWindow = qobject_cast<QMainWindow *>(parentWidget());

   if (! mainWindow) {
      return;
   }

   QMainWindowLayout *layout = qt_mainwindow_layout(mainWindow);
   Q_ASSERT_X(layout != nullptr, "QToolBar::initStyleOption()",
               "QMainWindow->layout() != QMainWindowLayout");

   layout->getStyleOptionInfo(option, const_cast<QToolBar *>(this));
}

void QToolBar::_q_toggleView(bool isToggleView)
{
   Q_D(QToolBar);
   d->_q_toggleView(isToggleView);
}

void QToolBar::_q_updateIconSize(const QSize &size)
{
   Q_D(QToolBar);
   d->_q_updateIconSize(size);
}

void QToolBar::_q_updateToolButtonStyle(Qt::ToolButtonStyle style)
{
   Q_D(QToolBar);
   d->_q_updateToolButtonStyle(style);
}

bool QToolBar::cs_isMainWindow() const
{
   return (qobject_cast<QMainWindow *>(parentWidget()) != nullptr);
}

#endif  // QT_NO_TOOLBAR
