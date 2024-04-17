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

#include <qtoolbutton.h>

#if ! defined(QT_NO_TOOLBUTTON)

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qicon.h>
#include <qmainwindow.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qpointer.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qstylepainter.h>
#include <qtoolbar.h>
#include <qtooltip.h>
#include <qvariant.h>

#include <qabstractbutton_p.h>
#include <qaction_p.h>
#include <qmenu_p.h>

class QToolButtonPrivate : public QAbstractButtonPrivate
{
   Q_DECLARE_PUBLIC(QToolButton)

 public:
   enum ButtonStatus {
      NoButtonPressed   = 0,
      MenuButtonPressed = 1,
      ToolButtonPressed = 2
   };

#ifndef QT_NO_MENU
   void _q_buttonPressed();
   void _q_buttonReleased();
   void popupTimerDone();
   void _q_updateButtonDown();
   void _q_menuTriggered(QAction *);
#endif

   void init();
   bool updateHoverControl(const QPoint &pos);
   void _q_actionTriggered();
   QStyle::SubControl newHoverControl(const QPoint &pos);

   QStyle::SubControl hoverControl;
   QRect hoverRect;
   QPointer<QAction> menuAction;       // menu set by the user (setMenu)
   QBasicTimer popupTimer;
   int delay;
   Qt::ArrowType arrowType;
   Qt::ToolButtonStyle toolButtonStyle;
   QToolButton::ToolButtonPopupMode popupMode;

   uint buttonPressed  : 2;
   uint menuButtonDown : 1;
   uint autoRaise      : 1;
   uint repeat         : 1;

   QAction *defaultAction;

#ifndef QT_NO_MENU
   bool hasMenu() const;
   QList<QAction *> actionsCopy;
#endif
};

#ifndef QT_NO_MENU
bool QToolButtonPrivate::hasMenu() const
{
   return ((defaultAction && defaultAction->menu())
         || (menuAction && menuAction->menu())
         || actions.size() > (defaultAction ? 1 : 0));
}
#endif

QToolButton::QToolButton(QWidget *parent)
   : QAbstractButton(*new QToolButtonPrivate, parent)
{
   Q_D(QToolButton);
   d->init();
}

//  Set-up code common to all the constructors
void QToolButtonPrivate::init()
{
   Q_Q(QToolButton);

   defaultAction = nullptr;

#ifndef QT_NO_TOOLBAR
   if (qobject_cast<QToolBar *>(q->parent())) {
      autoRaise = true;
   } else
#endif
      autoRaise = false;

   arrowType = Qt::NoArrow;
   menuButtonDown = false;
   popupMode = QToolButton::DelayedPopup;
   buttonPressed = QToolButtonPrivate::NoButtonPressed;

   toolButtonStyle = Qt::ToolButtonIconOnly;
   hoverControl = QStyle::SC_None;

   q->setFocusPolicy(Qt::TabFocus);
   q->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed,
         QSizePolicy::ToolButton));

#ifndef QT_NO_MENU
   QObject::connect(q, &QToolButton::pressed,  q, &QToolButton::_q_buttonPressed);
   QObject::connect(q, &QToolButton::released, q, &QToolButton::_q_buttonReleased);
#endif

   setLayoutItemMargins(QStyle::SE_ToolButtonLayoutItem);
   delay = q->style()->styleHint(QStyle::SH_ToolButton_PopupDelay, nullptr, q);
}

void QToolButton::initStyleOption(QStyleOptionToolButton *option) const
{
   if (!option) {
      return;
   }

   Q_D(const QToolButton);
   option->initFrom(this);
   bool forceNoText = false;
   option->iconSize = iconSize(); //default value

#ifndef QT_NO_TOOLBAR
   if (parentWidget()) {
      if (QToolBar *toolBar = qobject_cast<QToolBar *>(parentWidget())) {
         option->iconSize = toolBar->iconSize();
      }
   }
#endif // QT_NO_TOOLBAR

   if (!forceNoText) {
      option->text = d->text;
   }
   option->icon = d->icon;
   option->arrowType = d->arrowType;
   if (d->down) {
      option->state |= QStyle::State_Sunken;
   }
   if (d->checked) {
      option->state |= QStyle::State_On;
   }
   if (d->autoRaise) {
      option->state |= QStyle::State_AutoRaise;
   }
   if (!d->checked && !d->down) {
      option->state |= QStyle::State_Raised;
   }

   option->subControls = QStyle::SC_ToolButton;
   option->activeSubControls = QStyle::SC_None;

   option->features = QStyleOptionToolButton::None;
   if (d->popupMode == QToolButton::MenuButtonPopup) {
      option->subControls |= QStyle::SC_ToolButtonMenu;
      option->features |= QStyleOptionToolButton::MenuButtonPopup;
   }
   if (option->state & QStyle::State_MouseOver) {
      option->activeSubControls = d->hoverControl;
   }
   if (d->menuButtonDown) {
      option->state |= QStyle::State_Sunken;
      option->activeSubControls |= QStyle::SC_ToolButtonMenu;
   }
   if (d->down) {
      option->state |= QStyle::State_Sunken;
      option->activeSubControls |= QStyle::SC_ToolButton;
   }


   if (d->arrowType != Qt::NoArrow) {
      option->features |= QStyleOptionToolButton::Arrow;
   }
   if (d->popupMode == QToolButton::DelayedPopup) {
      option->features |= QStyleOptionToolButton::PopupDelay;
   }
#ifndef QT_NO_MENU
   if (d->hasMenu()) {
      option->features |= QStyleOptionToolButton::HasMenu;
   }
#endif
   if (d->toolButtonStyle == Qt::ToolButtonFollowStyle) {
      option->toolButtonStyle = Qt::ToolButtonStyle(style()->styleHint(QStyle::SH_ToolButtonStyle, option, this));
   } else {
      option->toolButtonStyle = d->toolButtonStyle;
   }

   if (option->toolButtonStyle == Qt::ToolButtonTextBesideIcon) {
      // If the action is not prioritized, remove the text label to save space
      if (d->defaultAction && d->defaultAction->priority() < QAction::NormalPriority) {
         option->toolButtonStyle = Qt::ToolButtonIconOnly;
      }
   }

   if (d->icon.isNull() && d->arrowType == Qt::NoArrow && !forceNoText) {
      if (!d->text.isEmpty()) {
         option->toolButtonStyle = Qt::ToolButtonTextOnly;
      } else if (option->toolButtonStyle != Qt::ToolButtonTextOnly) {
         option->toolButtonStyle = Qt::ToolButtonIconOnly;
      }
   }

   option->pos = pos();
   option->font = font();
}

QToolButton::~QToolButton()
{
}

QSize QToolButton::sizeHint() const
{
   Q_D(const QToolButton);

   if (d->sizeHint.isValid()) {
      return d->sizeHint;
   }
   ensurePolished();

   int w = 0, h = 0;
   QStyleOptionToolButton opt;
   initStyleOption(&opt);

   QFontMetrics fm = fontMetrics();
   if (opt.toolButtonStyle != Qt::ToolButtonTextOnly) {
      QSize icon = opt.iconSize;
      w = icon.width();
      h = icon.height();
   }

   if (opt.toolButtonStyle != Qt::ToolButtonIconOnly) {
      QSize textSize = fm.size(Qt::TextShowMnemonic, text());
      textSize.setWidth(textSize.width() + fm.width(QLatin1Char(' ')) * 2);
      if (opt.toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
         h += 4 + textSize.height();
         if (textSize.width() > w) {
            w = textSize.width();
         }
      } else if (opt.toolButtonStyle == Qt::ToolButtonTextBesideIcon) {
         w += 4 + textSize.width();
         if (textSize.height() > h) {
            h = textSize.height();
         }
      } else { // TextOnly
         w = textSize.width();
         h = textSize.height();
      }
   }

   opt.rect.setSize(QSize(w, h)); // PM_MenuButtonIndicator depends on the height
   if (d->popupMode == MenuButtonPopup) {
      w += style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &opt, this);
   }

   d->sizeHint = style()->sizeFromContents(QStyle::CT_ToolButton, &opt, QSize(w, h), this).
      expandedTo(QApplication::globalStrut());
   return d->sizeHint;
}

QSize QToolButton::minimumSizeHint() const
{
   return sizeHint();
}

Qt::ToolButtonStyle QToolButton::toolButtonStyle() const
{
   Q_D(const QToolButton);
   return d->toolButtonStyle;
}

Qt::ArrowType QToolButton::arrowType() const
{
   Q_D(const QToolButton);
   return d->arrowType;
}

void QToolButton::setToolButtonStyle(Qt::ToolButtonStyle style)
{
   Q_D(QToolButton);
   if (d->toolButtonStyle == style) {
      return;
   }

   d->toolButtonStyle = style;
   d->sizeHint = QSize();
   updateGeometry();

   if (isVisible()) {
      update();
   }
}

void QToolButton::setArrowType(Qt::ArrowType type)
{
   Q_D(QToolButton);
   if (d->arrowType == type) {
      return;
   }

   d->arrowType = type;
   d->sizeHint = QSize();
   updateGeometry();
   if (isVisible()) {
      update();
   }
}

void QToolButton::paintEvent(QPaintEvent *)
{
   QStylePainter p(this);
   QStyleOptionToolButton opt;
   initStyleOption(&opt);
   p.drawComplexControl(QStyle::CC_ToolButton, opt);
}

void QToolButton::actionEvent(QActionEvent *event)
{
   Q_D(QToolButton);
   QAction *action = event->action();

   switch (event->type()) {
      case QEvent::ActionChanged:
         if (action == d->defaultAction) {
            setDefaultAction(action);   // update button state
         }
         break;

      case QEvent::ActionAdded:
         connect(action, &QAction::triggered, this, &QToolButton::_q_actionTriggered);
         break;

      case QEvent::ActionRemoved:
         if (d->defaultAction == action) {
            d->defaultAction = nullptr;
         }

#ifndef QT_NO_MENU
         if (action == d->menuAction) {
            d->menuAction = nullptr;
         }
#endif
         action->disconnect(this);
         break;
      default:
         ;
   }
   QAbstractButton::actionEvent(event);
}

QStyle::SubControl QToolButtonPrivate::newHoverControl(const QPoint &pos)
{
   Q_Q(QToolButton);
   QStyleOptionToolButton opt;
   q->initStyleOption(&opt);
   opt.subControls = QStyle::SC_All;
   hoverControl = q->style()->hitTestComplexControl(QStyle::CC_ToolButton, &opt, pos, q);
   if (hoverControl == QStyle::SC_None) {
      hoverRect = QRect();
   } else {
      hoverRect = q->style()->subControlRect(QStyle::CC_ToolButton, &opt, hoverControl, q);
   }
   return hoverControl;
}

bool QToolButtonPrivate::updateHoverControl(const QPoint &pos)
{
   Q_Q(QToolButton);

   QRect lastHoverRect = hoverRect;
   QStyle::SubControl lastHoverControl = hoverControl;

   bool doesHover = q->testAttribute(Qt::WA_Hover);

   if (lastHoverControl != newHoverControl(pos) && doesHover) {
      q->update(lastHoverRect);
      q->update(hoverRect);
      return true;
   }

   return !doesHover;
}

void QToolButtonPrivate::_q_actionTriggered()
{
   Q_Q(QToolButton);
   if (QAction *action = qobject_cast<QAction *>(q->sender())) {
      emit q->triggered(action);
   }
}

void QToolButton::enterEvent(QEvent *e)
{
   Q_D(QToolButton);

   if (d->autoRaise) {
      update();
   }

   if (d->defaultAction) {
      d->defaultAction->hover();
   }

   QAbstractButton::enterEvent(e);
}

void QToolButton::leaveEvent(QEvent *e)
{
   Q_D(QToolButton);
   if (d->autoRaise) {
      update();
   }

   QAbstractButton::leaveEvent(e);
}

void QToolButton::timerEvent(QTimerEvent *e)
{
#ifndef QT_NO_MENU
   Q_D(QToolButton);
   if (e->timerId() == d->popupTimer.timerId()) {
      d->popupTimerDone();
      return;
   }
#endif
   QAbstractButton::timerEvent(e);
}

void QToolButton::changeEvent(QEvent *e)
{
#ifndef QT_NO_TOOLBAR
   Q_D(QToolButton);
   if (e->type() == QEvent::ParentChange) {
      if (qobject_cast<QToolBar *>(parentWidget())) {
         d->autoRaise = true;
      }

   } else if (e->type() == QEvent::StyleChange

#ifdef Q_OS_DARWIN
      || e->type() == QEvent::MacSizeChange
#endif

   ) {
      d->delay = style()->styleHint(QStyle::SH_ToolButton_PopupDelay, nullptr, this);
      d->setLayoutItemMargins(QStyle::SE_ToolButtonLayoutItem);
   }
#endif

   QAbstractButton::changeEvent(e);
}

void QToolButton::mousePressEvent(QMouseEvent *e)
{
   Q_D(QToolButton);

#ifndef QT_NO_MENU
   QStyleOptionToolButton opt;
   initStyleOption(&opt);

   if (e->button() == Qt::LeftButton && (d->popupMode == MenuButtonPopup)) {
      QRect popupr = style()->subControlRect(QStyle::CC_ToolButton, &opt, QStyle::SC_ToolButtonMenu, this);

      if (popupr.isValid() && popupr.contains(e->pos())) {
         d->buttonPressed = QToolButtonPrivate::MenuButtonPressed;
         showMenu();
         return;
      }
   }
#endif

   d->buttonPressed = QToolButtonPrivate::ToolButtonPressed;
   QAbstractButton::mousePressEvent(e);
}

void QToolButton::mouseReleaseEvent(QMouseEvent *e)
{
   Q_D(QToolButton);
   QAbstractButton::mouseReleaseEvent(e);
   d->buttonPressed = QToolButtonPrivate::NoButtonPressed;
}

bool QToolButton::hitButton(const QPoint &pos) const
{
   Q_D(const QToolButton);

   if (QAbstractButton::hitButton(pos)) {
      return (d->buttonPressed != QToolButtonPrivate::MenuButtonPressed);
   }

   return false;
}

#if ! defined(QT_NO_MENU)

void QToolButton::setMenu(QMenu *menu)
{
   Q_D(QToolButton);

   if (d->menuAction == (menu ? menu->menuAction() : nullptr)) {
      return;
   }

   if (d->menuAction) {
      removeAction(d->menuAction);
   }

   if (menu) {
      d->menuAction = menu->menuAction();
      addAction(d->menuAction);
   } else {
      d->menuAction = nullptr;
   }

   d->sizeHint = QSize();
   updateGeometry();
   update();
}

QMenu *QToolButton::menu() const
{
   Q_D(const QToolButton);

   if (d->menuAction) {
      return d->menuAction->menu();
   }

   return nullptr;
}

void QToolButton::showMenu()
{
   Q_D(QToolButton);

   if (!d->hasMenu()) {
      d->menuButtonDown = false;
      return; // no menu to show
   }

   // prevent recursions spinning another event loop
   if (d->menuButtonDown) {
      return;
   }

   d->menuButtonDown = true;
   repaint();
   d->popupTimer.stop();
   d->popupTimerDone();
}

void QToolButtonPrivate::_q_buttonPressed()
{
   Q_Q(QToolButton);

   if (!hasMenu()) {
      return;   // no menu to show
   }

   if (popupMode == QToolButton::MenuButtonPopup) {
      return;

   } else if (delay > 0 && popupMode == QToolButton::DelayedPopup) {
      popupTimer.start(delay, q);

   } else if (delay == 0 || popupMode == QToolButton::InstantPopup) {
      q->showMenu();
   }
}

void QToolButtonPrivate::_q_buttonReleased()
{
   popupTimer.stop();
}

void QToolButtonPrivate::popupTimerDone()
{
   Q_Q(QToolButton);

   popupTimer.stop();
   if (!menuButtonDown && !down) {
      return;
   }

   menuButtonDown = true;
   QPointer<QMenu> actualMenu;
   bool mustDeleteActualMenu = false;

   if (menuAction) {
      actualMenu = menuAction->menu();

   } else if (defaultAction && defaultAction->menu()) {
      actualMenu = defaultAction->menu();

   } else {
      actualMenu = new QMenu(q);
      mustDeleteActualMenu = true;

      for (int i = 0; i < actions.size(); i++) {
         actualMenu->addAction(actions.at(i));
      }
   }

   repeat = q->autoRepeat();
   q->setAutoRepeat(false);
   bool horizontal = true;

#if ! defined(QT_NO_TOOLBAR)
   QToolBar *tb = qobject_cast<QToolBar *>(q->parent());
   if (tb && tb->orientation() == Qt::Vertical) {
      horizontal = false;
   }
#endif

   QPoint p;
   const QRect rect = q->rect(); // Find screen via point in case of QGraphicsProxyWidget.

   QRect screen = QApplication::desktop()->availableGeometry(q->mapToGlobal(rect.center()));
   QSize sh = ((QToolButton *)(QMenu *)actualMenu)->receivers(SLOT(aboutToShow())) ? QSize() : actualMenu->sizeHint();

   if (horizontal) {
      if (q->isRightToLeft()) {
         if (q->mapToGlobal(QPoint(0, rect.bottom())).y() + sh.height() <= screen.height()) {
            p = q->mapToGlobal(rect.bottomRight());
         } else {
            p = q->mapToGlobal(rect.topRight() - QPoint(0, sh.height()));
         }
         p.rx() -= sh.width();

      } else {
         if (q->mapToGlobal(QPoint(0, rect.bottom())).y() + sh.height() <= screen.height()) {
            p = q->mapToGlobal(rect.bottomLeft());
         } else {
            p = q->mapToGlobal(rect.topLeft() - QPoint(0, sh.height()));
         }
      }

   } else {
      if (q->isRightToLeft()) {
         if (q->mapToGlobal(QPoint(rect.left(), 0)).x() - sh.width() <= screen.x()) {
            p = q->mapToGlobal(rect.topRight());
         } else {
            p = q->mapToGlobal(rect.topLeft());
            p.rx() -= sh.width();
         }
      } else {
         if (q->mapToGlobal(QPoint(rect.right(), 0)).x() + sh.width() <= screen.right()) {
            p = q->mapToGlobal(rect.topRight());
         } else {
            p = q->mapToGlobal(rect.topLeft() - QPoint(sh.width(), 0));
         }
      }
   }
   p.rx() = qMax(screen.left(), qMin(p.x(), screen.right() - sh.width()));
   p.ry() += 1;

   QPointer<QToolButton> that = q;
   actualMenu->setNoReplayFor(q);

   if (! mustDeleteActualMenu) {
      // only if action are not in this widget
      QObject::connect(actualMenu.data(), &QMenu::triggered, q, &QToolButton::_q_menuTriggered);
   }

   QObject::connect(actualMenu.data(), &QMenu::aboutToHide, q, &QToolButton::_q_updateButtonDown);

   actualMenu->d_func()->causedPopup.widget = q;
   actualMenu->d_func()->causedPopup.action = defaultAction;
   actionsCopy = q->actions();  // the list of action may be modified in slots
   actualMenu->exec(p);

   if (! that) {
      return;
   }

   QObject::disconnect(actualMenu.data(), &QMenu::aboutToHide, q, &QToolButton::_q_updateButtonDown);

   if (mustDeleteActualMenu) {
      delete actualMenu;
   } else {
      QObject::disconnect(actualMenu.data(), &QMenu::triggered, q, &QToolButton::_q_menuTriggered);
   }

   actionsCopy.clear();

   if (repeat) {
      q->setAutoRepeat(true);
   }
}

void QToolButtonPrivate::_q_updateButtonDown()
{
   Q_Q(QToolButton);

   menuButtonDown = false;

   if (q->isDown()) {
      q->setDown(false);
   } else {
      q->repaint();
   }
}

void QToolButtonPrivate::_q_menuTriggered(QAction *action)
{
   Q_Q(QToolButton);
   if (action && !actionsCopy.contains(action)) {
      emit q->triggered(action);
   }
}

void QToolButton::setPopupMode(ToolButtonPopupMode mode)
{
   Q_D(QToolButton);
   d->popupMode = mode;
}

QToolButton::ToolButtonPopupMode QToolButton::popupMode() const
{
   Q_D(const QToolButton);
   return d->popupMode;
}
#endif

void QToolButton::setAutoRaise(bool enable)
{
   Q_D(QToolButton);
   d->autoRaise = enable;

   update();
}

bool QToolButton::autoRaise() const
{
   Q_D(const QToolButton);
   return d->autoRaise;
}

void QToolButton::setDefaultAction(QAction *action)
{
   Q_D(QToolButton);

#if ! defined(QT_NO_MENU)
   bool hadMenu = false;
   hadMenu = d->hasMenu();
#endif

   d->defaultAction = action;
   if (!action) {
      return;
   }

   if (!actions().contains(action)) {
      addAction(action);
   }

   QString buttonText = action->iconText();

   // If iconText() is generated from text(), we need to escape any '&'s so they
   // don't turn into shortcuts
   if (QActionPrivate::get(action)->iconText.isEmpty()) {
      buttonText.replace(QLatin1String("&"), QLatin1String("&&"));
   }

   setText(buttonText);
   setIcon(action->icon());

#ifndef QT_NO_TOOLTIP
   setToolTip(action->toolTip());
#endif

#ifndef QT_NO_STATUSTIP
   setStatusTip(action->statusTip());
#endif

#ifndef QT_NO_WHATSTHIS
   setWhatsThis(action->whatsThis());
#endif

#if ! defined(QT_NO_MENU)
   if (action->menu() && !hadMenu) {
      // 'default' popup mode defined introduced by tool bar.
      // should have changed QToolButton's default instead.
      setPopupMode(QToolButton::MenuButtonPopup);
   }
#endif

   setCheckable(action->isCheckable());
   setChecked(action->isChecked());
   setEnabled(action->isEnabled());

   if (action->d_func()->fontSet) {
      setFont(action->font());
   }
}

QAction *QToolButton::defaultAction() const
{
   Q_D(const QToolButton);
   return d->defaultAction;
}

void QToolButton::nextCheckState()
{
   Q_D(QToolButton);

   if (!d->defaultAction) {
      QAbstractButton::nextCheckState();
   } else {
      d->defaultAction->trigger();
   }
}

bool QToolButton::event(QEvent *event)
{
   switch (event->type()) {
      case QEvent::HoverEnter:
      case QEvent::HoverLeave:
      case QEvent::HoverMove:
         if (const QHoverEvent *he = static_cast<const QHoverEvent *>(event)) {
            d_func()->updateHoverControl(he->pos());
         }
         break;

      default:
         break;
   }

   return QAbstractButton::event(event);
}

#if ! defined(QT_NO_MENU)
void QToolButton::_q_buttonPressed()
{
   Q_D(QToolButton);
   d->_q_buttonPressed();
}

void QToolButton::_q_buttonReleased()
{
   Q_D(QToolButton);
   d->_q_buttonReleased();
}

void QToolButton::_q_updateButtonDown()
{
   Q_D(QToolButton);
   d->_q_updateButtonDown();
}

void QToolButton::_q_menuTriggered(QAction *action)
{
   Q_D(QToolButton);
   d->_q_menuTriggered(action);
}
#endif

void QToolButton::_q_actionTriggered()
{
   Q_D(QToolButton);
   d->_q_actionTriggered();
}

#endif
