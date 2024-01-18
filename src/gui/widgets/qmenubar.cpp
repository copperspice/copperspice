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

#include <qmenubar.h>

#include <qstyle.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qdesktopwidget.h>

#ifndef QT_NO_ACCESSIBILITY
# include <qaccessible.h>
#endif

#include <qpainter.h>
#include <qstylepainter.h>
#include <qevent.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qwhatsthis.h>
#include <qplatform_theme.h>
#include <qplatform_integration.h>

#include <qguiapplication_p.h>

#ifndef QT_NO_MENUBAR

#include <qmenu_p.h>
#include <qmenubar_p.h>
#include <qdebug.h>

class QMenuBarExtension : public QToolButton
{
 public:
   explicit QMenuBarExtension(QWidget *parent);

   QSize sizeHint() const  override;
   void paintEvent(QPaintEvent *)  override;
};

QMenuBarExtension::QMenuBarExtension(QWidget *parent)
   : QToolButton(parent)
{
   setObjectName(QLatin1String("qt_menubar_ext_button"));
   setAutoRaise(true);

#ifndef QT_NO_MENU
   setPopupMode(QToolButton::InstantPopup);
#endif
   setIcon(style()->standardIcon(QStyle::SP_ToolBarHorizontalExtensionButton, nullptr, parentWidget()));
}

void QMenuBarExtension::paintEvent(QPaintEvent *)
{
   QStylePainter p(this);
   QStyleOptionToolButton opt;
   initStyleOption(&opt);

   // We do not need to draw both extension arrows
   opt.features &= ~QStyleOptionToolButton::HasMenu;
   p.drawComplexControl(QStyle::CC_ToolButton, opt);
}

QSize QMenuBarExtension::sizeHint() const
{
   int ext = style()->pixelMetric(QStyle::PM_ToolBarExtensionExtent, nullptr, parentWidget());
   return QSize(ext, ext);
}

QAction *QMenuBarPrivate::actionAt(QPoint p) const
{
   for (int i = 0; i < actions.size(); ++i) {
      if (actionRect(actions.at(i)).contains(p)) {
         return actions.at(i);
      }
   }
   return nullptr;
}

QRect QMenuBarPrivate::menuRect(bool extVisible) const
{
   Q_Q(const QMenuBar);

   int hmargin = q->style()->pixelMetric(QStyle::PM_MenuBarPanelWidth, nullptr, q);
   QRect result = q->rect();
   result.adjust(hmargin, 0, -hmargin, 0);

   if (extVisible) {
      if (q->isRightToLeft()) {
         result.setLeft(result.left() + extension->sizeHint().width());
      } else {
         result.setWidth(result.width() - extension->sizeHint().width());
      }
   }

   if (leftWidget && leftWidget->isVisible()) {
      QSize sz = leftWidget->sizeHint();
      if (q->isRightToLeft()) {
         result.setRight(result.right() - sz.width());
      } else {
         result.setLeft(result.left() + sz.width());
      }
   }

   if (rightWidget && rightWidget->isVisible()) {
      QSize sz = rightWidget->sizeHint();
      if (q->isRightToLeft()) {
         result.setLeft(result.left() + sz.width());
      } else {
         result.setRight(result.right() - sz.width());
      }
   }

   return result;
}

bool QMenuBarPrivate::isVisible(QAction *action)
{
   return !hiddenActions.contains(action);
}

void QMenuBarPrivate::updateGeometries()
{
   Q_Q(QMenuBar);
   if (! itemsDirty) {
      return;
   }

   int q_width = q->width() - (q->style()->pixelMetric(QStyle::PM_MenuBarPanelWidth, nullptr, q) * 2);
   int q_start = -1;

   if (leftWidget || rightWidget) {
      int vmargin = q->style()->pixelMetric(QStyle::PM_MenuBarVMargin, nullptr, q)
         + q->style()->pixelMetric(QStyle::PM_MenuBarPanelWidth, nullptr, q);

      int hmargin = q->style()->pixelMetric(QStyle::PM_MenuBarHMargin, nullptr, q)
         + q->style()->pixelMetric(QStyle::PM_MenuBarPanelWidth, nullptr, q);

      if (leftWidget && leftWidget->isVisible()) {
         QSize sz = leftWidget->sizeHint();
         q_width -= sz.width();
         q_start = sz.width();
         QPoint pos(hmargin, (q->height() - leftWidget->height()) / 2);
         QRect vRect = QStyle::visualRect(q->layoutDirection(), q->rect(), QRect(pos, sz));
         leftWidget->setGeometry(vRect);
      }
      if (rightWidget && rightWidget->isVisible()) {
         QSize sz = rightWidget->sizeHint();
         q_width -= sz.width();
         QPoint pos(q->width() - sz.width() - hmargin, vmargin);
         QRect vRect = QStyle::visualRect(q->layoutDirection(), q->rect(), QRect(pos, sz));
         rightWidget->setGeometry(vRect);
      }
   }

#ifdef Q_OS_DARWIN
   if (q->isNativeMenuBar()) {
      //nothing to see here folks, move along
      itemsDirty = false;
      return;
   }
#endif

   calcActionRects(q_width, q_start);
   currentAction = nullptr;

#ifndef QT_NO_SHORTCUT

   if (itemsDirty) {
      for (int j = 0; j < shortcutIndexMap.size(); ++j) {
         q->releaseShortcut(shortcutIndexMap.value(j));
      }

      shortcutIndexMap.resize(0);

      for (int i = 0; i < actions.count(); i++) {
         shortcutIndexMap.append(q->grabShortcut(QKeySequence::mnemonic(actions.at(i)->text())));
      }
   }
#endif

   itemsDirty = false;

   hiddenActions.clear();
   //this is the menu rectangle without any extension
   QRect menuRect = this->menuRect(false);

   //we try to see if the actions will fit there
   bool hasHiddenActions = false;
   for (int i = 0; i < actions.count(); ++i) {
      const QRect &rect = actionRects.at(i);
      if (rect.isValid() && !menuRect.contains(rect)) {
         hasHiddenActions = true;
         break;
      }
   }

   //...and if not, determine the ones that fit on the menu with the extension visible
   if (hasHiddenActions) {
      menuRect = this->menuRect(true);
      for (int i = 0; i < actions.count(); ++i) {
         const QRect &rect = actionRects.at(i);
         if (rect.isValid() && !menuRect.contains(rect)) {
            hiddenActions.append(actions.at(i));
         }
      }
   }

   if (hiddenActions.count() > 0) {
      QMenu *pop = extension->menu();
      if (!pop) {
         pop = new QMenu(q);
         extension->setMenu(pop);
      }
      pop->clear();
      pop->addActions(hiddenActions);

      int vmargin = q->style()->pixelMetric(QStyle::PM_MenuBarVMargin, nullptr, q);
      int x = q->isRightToLeft()
         ? menuRect.left() - extension->sizeHint().width() + 1
         : menuRect.right();
      extension->setGeometry(x, vmargin, extension->sizeHint().width(), menuRect.height() - vmargin * 2);
      extension->show();
   } else {
      extension->hide();
   }
   q->updateGeometry();
}

QRect QMenuBarPrivate::actionRect(QAction *act) const
{
   const int index = actions.indexOf(act);

   //makes sure the geometries are up-to-date
   const_cast<QMenuBarPrivate *>(this)->updateGeometries();

   if (index < 0 || index >= actionRects.count()) {
      return QRect();   // that can happen in case of native menubar
   }

   return actionRects.at(index);
}

void QMenuBarPrivate::focusFirstAction()
{
   if (!currentAction) {
      updateGeometries();
      int index = 0;
      while (index < actions.count() && actionRects.at(index).isNull()) {
         ++index;
      }

      if (index < actions.count()) {
         setCurrentAction(actions.at(index));
      }
   }
}

void QMenuBarPrivate::setKeyboardMode(bool b)
{
   Q_Q(QMenuBar);
   if (b && !q->style()->styleHint(QStyle::SH_MenuBar_AltKeyNavigation, nullptr, q)) {
      setCurrentAction(nullptr);
      return;
   }

   keyboardState = b;
   if (b) {
      QWidget *fw = QApplication::focusWidget();
      if (fw != q) {
         keyboardFocusWidget = fw;
      }
      focusFirstAction();
      q->setFocus(Qt::MenuBarFocusReason);

   } else {
      if (!popupState) {
         setCurrentAction(nullptr);
      }
      if (keyboardFocusWidget) {
         if (QApplication::focusWidget() == q) {
            keyboardFocusWidget->setFocus(Qt::MenuBarFocusReason);
         }
         keyboardFocusWidget = nullptr;
      }
   }
   q->update();
}

void QMenuBarPrivate::popupAction(QAction *action, bool activateFirst)
{
   Q_Q(QMenuBar);
   if (!action || !action->menu() || closePopupMode) {
      return;
   }

   popupState = true;
   if (action->isEnabled() && action->menu()->isEnabled()) {
      closePopupMode = 0;
      activeMenu = action->menu();
      activeMenu->d_func()->causedPopup.widget = q;
      activeMenu->d_func()->causedPopup.action = action;

      QRect adjustedActionRect = actionRect(action);
      QPoint pos(q->mapToGlobal(QPoint(adjustedActionRect.left(), adjustedActionRect.bottom() + 1)));
      QSize popup_size = activeMenu->sizeHint();

      //we put the popup menu on the screen containing the bottom-center of the action rect
      QRect screenRect = QApplication::desktop()->screenGeometry(pos + QPoint(adjustedActionRect.width() / 2, 0));
      pos = QPoint(qMax(pos.x(), screenRect.x()), qMax(pos.y(), screenRect.y()));

      const bool fitUp = (q->mapToGlobal(adjustedActionRect.topLeft()).y() >= popup_size.height());
      const bool fitDown = (pos.y() + popup_size.height() <= screenRect.bottom());
      const bool rtl = q->isRightToLeft();
      const int actionWidth = adjustedActionRect.width();

      if (!fitUp && !fitDown) { //we should shift the menu
         bool shouldShiftToRight = !rtl;
         if (rtl && popup_size.width() > pos.x()) {
            shouldShiftToRight = true;
         } else if (actionWidth + popup_size.width() + pos.x() > screenRect.right()) {
            shouldShiftToRight = false;
         }

         if (shouldShiftToRight) {
            pos.rx() += actionWidth + (rtl ? popup_size.width() : 0);
         } else {
            //shift to left
            if (!rtl) {
               pos.rx() -= popup_size.width();
            }
         }
      } else if (rtl) {
         pos.rx() += actionWidth;
      }

      if (!defaultPopDown || (fitUp && !fitDown)) {
         pos.setY(qMax(screenRect.y(), q->mapToGlobal(QPoint(0, adjustedActionRect.top() - popup_size.height())).y()));
      }
      activeMenu->popup(pos);
      if (activateFirst) {
         activeMenu->d_func()->setFirstActionActive();
      }
   }
   q->update(actionRect(action));
}

void QMenuBarPrivate::setCurrentAction(QAction *action, bool popup, bool activateFirst)
{
   if (currentAction == action && popup == popupState) {
      return;
   }

   autoReleaseTimer.stop();

   doChildEffects = (popup && !activeMenu);

   Q_Q(QMenuBar);
   QWidget *fw = nullptr;

   if (QMenu *menu = activeMenu) {
      activeMenu = nullptr;

      if (popup) {
         fw = q->window()->focusWidget();
         q->setFocus(Qt::NoFocusReason);
      }
      menu->hide();
   }

   if (currentAction) {
      q->update(actionRect(currentAction));
   }

   popupState = popup;

#ifndef QT_NO_STATUSTIP
   QAction *previousAction = currentAction;
#endif

   currentAction = action;
   if (action && action->isEnabled()) {
      activateAction(action, QAction::Hover);

      if (popup) {
         popupAction(action, activateFirst);
      }

      q->update(actionRect(action));

#ifndef QT_NO_STATUSTIP
   }  else if (previousAction) {
      QString empty;
      QStatusTipEvent tip(empty);
      QApplication::sendEvent(q, &tip);
#endif
   }

   if (fw) {
      fw->setFocus(Qt::NoFocusReason);
   }
}

void QMenuBarPrivate::calcActionRects(int max_width, int start) const
{
   Q_Q(const QMenuBar);

   if (!itemsDirty) {
      return;
   }

   //let's reinitialize the buffer
   actionRects.resize(actions.count());
   actionRects.fill(QRect());

   const QStyle *style = q->style();

   const int itemSpacing = style->pixelMetric(QStyle::PM_MenuBarItemSpacing, nullptr, q);
   int max_item_height = 0, separator = -1, separator_start = 0, separator_len = 0;

   //calculate size
   const QFontMetrics fm = q->fontMetrics();
   const int hmargin = style->pixelMetric(QStyle::PM_MenuBarHMargin, nullptr, q),
             vmargin = style->pixelMetric(QStyle::PM_MenuBarVMargin, nullptr, q),
             icone = style->pixelMetric(QStyle::PM_SmallIconSize, nullptr, q);

   for (int i = 0; i < actions.count(); i++) {
      QAction *action = actions.at(i);

      if (!action->isVisible()) {
         continue;
      }

      QSize sz;

      //calc what I think the size is..
      if (action->isSeparator()) {
         if (style->styleHint(QStyle::SH_DrawMenuBarSeparator, nullptr, q)) {
            separator = i;
         }
         continue; //we don't really position these!
      } else {
         const QString s = action->text();
         QIcon is = action->icon();
         // If an icon is set, only the icon is visible
         if (!is.isNull()) {
            sz = sz.expandedTo(QSize(icone, icone));
         } else if (!s.isEmpty()) {
            sz = fm.size(Qt::TextShowMnemonic, s);
         }
      }

      //let the style modify the above size..
      QStyleOptionMenuItem opt;
      q->initStyleOption(&opt, action);
      sz = q->style()->sizeFromContents(QStyle::CT_MenuBarItem, &opt, sz, q);

      if (!sz.isEmpty()) {
         {
            //update the separator state
            int iWidth = sz.width() + itemSpacing;
            if (separator == -1) {
               separator_start += iWidth;
            } else {
               separator_len += iWidth;
            }
         }
         //maximum height
         max_item_height = qMax(max_item_height, sz.height());
         //append
         actionRects[i] = QRect(0, 0, sz.width(), sz.height());
      }
   }

   //calculate position
   const int fw = q->style()->pixelMetric(QStyle::PM_MenuBarPanelWidth, nullptr, q);
   int x = fw + ((start == -1) ? hmargin : start) + itemSpacing;
   int y = fw + vmargin;
   for (int i = 0; i < actions.count(); i++) {
      QRect &rect = actionRects[i];
      if (rect.isNull()) {
         continue;
      }

      //resize
      rect.setHeight(max_item_height);

      //move
      if (separator != -1 && i >= separator) { //after the separator
         int left = (max_width - separator_len - hmargin - itemSpacing) + (x - separator_start - hmargin);
         if (left < separator_start) { //wrap
            separator_start = x = hmargin;
            y += max_item_height;
         }
         rect.moveLeft(left);
      } else {
         rect.moveLeft(x);
      }
      rect.moveTop(y);

      //keep moving along..
      x += rect.width() + itemSpacing;

      //make sure we follow the layout direction
      rect = QStyle::visualRect(q->layoutDirection(), q->rect(), rect);
   }
}

void QMenuBarPrivate::activateAction(QAction *action, QAction::ActionEvent action_e)
{
   Q_Q(QMenuBar);
   if (!action || !action->isEnabled()) {
      return;
   }
   action->activate(action_e);
   if (action_e == QAction::Hover) {
      action->showStatusText(q);
   }

   //     if(action_e == QAction::Trigger)
   //         emit q->activated(action);
   //     else if(action_e == QAction::Hover)
   //         emit q->highlighted(action);
}

void QMenuBarPrivate::_q_actionTriggered()
{
   Q_Q(QMenuBar);
   if (QAction *action = qobject_cast<QAction *>(q->sender())) {
      emit q->triggered(action);
   }
}

void QMenuBarPrivate::_q_actionHovered()
{
   Q_Q(QMenuBar);

   if (QAction *action = qobject_cast<QAction *>(q->sender())) {
      emit q->hovered(action);

#ifndef QT_NO_ACCESSIBILITY
      if (QAccessible::isActive()) {
         int actionIndex = actions.indexOf(action);
         QAccessibleEvent focusEvent(q, QAccessible::Focus);
         focusEvent.setChild(actionIndex);
         QAccessible::updateAccessibility(&focusEvent);
      }
#endif

   }
}

void QMenuBar::initStyleOption(QStyleOptionMenuItem *option, const QAction *action) const
{
   if (!option || !action) {
      return;
   }

   Q_D(const QMenuBar);
   option->palette = palette();
   option->state = QStyle::State_None;

   if (isEnabled() && action->isEnabled()) {
      option->state |= QStyle::State_Enabled;
   } else {
      option->palette.setCurrentColorGroup(QPalette::Disabled);
   }

   option->fontMetrics = fontMetrics();
   if (d->currentAction && d->currentAction == action) {
      option->state |= QStyle::State_Selected;
      if (d->popupState && !d->closePopupMode) {
         option->state |= QStyle::State_Sunken;
      }
   }
   if (hasFocus() || d->currentAction) {
      option->state |= QStyle::State_HasFocus;
   }

   option->menuRect = rect();
   option->menuItemType = QStyleOptionMenuItem::Normal;
   option->checkType = QStyleOptionMenuItem::NotCheckable;
   option->text = action->text();
   option->icon = action->icon();
}

void QMenuBarPrivate::init()
{
   Q_Q(QMenuBar);
   q->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
   q->setAttribute(Qt::WA_CustomWhatsThis);

   if (!QApplication::instance()->testAttribute(Qt::AA_DontUseNativeMenuBar)) {
      platformMenuBar = QGuiApplicationPrivate::platformTheme()->createPlatformMenuBar();
   }

   if (platformMenuBar) {
      q->hide();
   }

   q->setBackgroundRole(QPalette::Button);
   handleReparent();
   q->setMouseTracking(q->style()->styleHint(QStyle::SH_MenuBar_MouseTracking, nullptr, q));

   extension = new QMenuBarExtension(q);
   extension->setFocusPolicy(Qt::NoFocus);
   extension->hide();
}

//Gets the next action for keyboard navigation
QAction *QMenuBarPrivate::getNextAction(const int _start, const int increment) const
{
   Q_Q(const QMenuBar);
   const_cast<QMenuBarPrivate *>(this)->updateGeometries();
   bool allowActiveAndDisabled = q->style()->styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, nullptr, q);
   const int start = (_start == -1 && increment == -1) ? actions.count() : _start;
   const int end =  increment == -1 ? 0 : actions.count() - 1;

   for (int i = start; i != end;) {
      i += increment;
      QAction *current = actions.at(i);
      if (!actionRects.at(i).isNull() && (allowActiveAndDisabled || current->isEnabled())) {
         return current;
      }
   }

   if (_start != -1) {
      // try from the beginning or the end
      return getNextAction(-1, increment);
   }

   return nullptr;
}

QMenuBar::QMenuBar(QWidget *parent) : QWidget(*new QMenuBarPrivate, parent, Qt::EmptyFlag)
{
   Q_D(QMenuBar);
   d->init();
}

QMenuBar::~QMenuBar()
{
   Q_D(QMenuBar);

   delete d->platformMenuBar;
   d->platformMenuBar = nullptr;
}

QAction *QMenuBar::addAction(const QString &text)
{
   QAction *ret = new QAction(text, this);
   addAction(ret);
   return ret;
}

QAction *QMenuBar::addAction(const QString &text, const QObject *receiver, const QString &member)
{
   QAction *ret = new QAction(text, this);
   QObject::connect(ret, SIGNAL(triggered(bool)), receiver, member);
   addAction(ret);
   return ret;
}

QMenu *QMenuBar::addMenu(const QString &title)
{
   QMenu *menu = new QMenu(title, this);
   addAction(menu->menuAction());
   return menu;
}

QMenu *QMenuBar::addMenu(const QIcon &icon, const QString &title)
{
   QMenu *menu = new QMenu(title, this);
   menu->setIcon(icon);
   addAction(menu->menuAction());
   return menu;
}

QAction *QMenuBar::addMenu(QMenu *menu)
{
   QAction *action = menu->menuAction();
   addAction(action);
   return action;
}

QAction *QMenuBar::addSeparator()
{
   QAction *ret = new QAction(this);
   ret->setSeparator(true);
   addAction(ret);
   return ret;
}

QAction *QMenuBar::insertSeparator(QAction *before)
{
   QAction *action = new QAction(this);
   action->setSeparator(true);
   insertAction(before, action);
   return action;
}

QAction *QMenuBar::insertMenu(QAction *before, QMenu *menu)
{
   QAction *action = menu->menuAction();
   insertAction(before, action);
   return action;
}

QAction *QMenuBar::activeAction() const
{
   Q_D(const QMenuBar);
   return d->currentAction;
}

void QMenuBar::setActiveAction(QAction *act)
{
   Q_D(QMenuBar);
   d->setCurrentAction(act, true, false);
}

void QMenuBar::clear()
{
   QList<QAction *> acts = actions();
   for (int i = 0; i < acts.size(); i++) {
      removeAction(acts[i]);
   }
}

void QMenuBar::setDefaultUp(bool b)
{
   Q_D(QMenuBar);
   d->defaultPopDown = !b;
}

bool QMenuBar::isDefaultUp() const
{
   Q_D(const QMenuBar);
   return !d->defaultPopDown;
}

void QMenuBar::resizeEvent(QResizeEvent *)
{
   Q_D(QMenuBar);
   d->itemsDirty = true;
   d->updateGeometries();
}

void QMenuBar::paintEvent(QPaintEvent *e)
{
   Q_D(QMenuBar);
   QPainter p(this);
   QRegion emptyArea(rect());

   //draw the items
   for (int i = 0; i < d->actions.count(); ++i) {
      QAction *action = d->actions.at(i);
      QRect adjustedActionRect = d->actionRect(action);
      if (adjustedActionRect.isEmpty() || !d->isVisible(action)) {
         continue;
      }
      if (!e->rect().intersects(adjustedActionRect)) {
         continue;
      }

      emptyArea -= adjustedActionRect;
      QStyleOptionMenuItem opt;
      initStyleOption(&opt, action);
      opt.rect = adjustedActionRect;
      p.setClipRect(adjustedActionRect);
      style()->drawControl(QStyle::CE_MenuBarItem, &opt, &p, this);
   }

   //draw border
   if (int fw = style()->pixelMetric(QStyle::PM_MenuBarPanelWidth, nullptr, this)) {
      QRegion borderReg;
      borderReg += QRect(0, 0, fw, height()); //left
      borderReg += QRect(width() - fw, 0, fw, height()); //right
      borderReg += QRect(0, 0, width(), fw); //top
      borderReg += QRect(0, height() - fw, width(), fw); //bottom
      p.setClipRegion(borderReg);
      emptyArea -= borderReg;

      QStyleOptionFrame frame;
      frame.rect = rect();
      frame.palette = palette();
      frame.state = QStyle::State_None;
      frame.lineWidth = style()->pixelMetric(QStyle::PM_MenuBarPanelWidth);
      frame.midLineWidth = 0;
      style()->drawPrimitive(QStyle::PE_PanelMenuBar, &frame, &p, this);
   }

   p.setClipRegion(emptyArea);
   QStyleOptionMenuItem menuOpt;
   menuOpt.palette = palette();
   menuOpt.state = QStyle::State_None;
   menuOpt.menuItemType = QStyleOptionMenuItem::EmptyArea;
   menuOpt.checkType = QStyleOptionMenuItem::NotCheckable;
   menuOpt.rect = rect();
   menuOpt.menuRect = rect();
   style()->drawControl(QStyle::CE_MenuBarEmptyArea, &menuOpt, &p, this);
}

void QMenuBar::setVisible(bool visible)
{
   if (isNativeMenuBar()) {
      if (!visible) {
         QWidget::setVisible(false);
      }

      return;
   }

   QWidget::setVisible(visible);
}

void QMenuBar::mousePressEvent(QMouseEvent *e)
{
   Q_D(QMenuBar);

   if (e->button() != Qt::LeftButton) {
      return;
   }

   d->mouseDown = true;

   QAction *action = d->actionAt(e->pos());
   if (!action || !d->isVisible(action) || !action->isEnabled()) {
      d->setCurrentAction(nullptr);

#ifndef QT_NO_WHATSTHIS
      if (QWhatsThis::inWhatsThisMode()) {
         QWhatsThis::showText(e->globalPos(), d->whatsThis, this);
      }
#endif
      return;
   }

   if (d->currentAction == action && d->popupState) {
      if (QMenu *menu = d->activeMenu) {
         d->activeMenu = nullptr;
         menu->setAttribute(Qt::WA_NoMouseReplay);
         menu->hide();
      }

   } else {
      d->setCurrentAction(action, true);
   }
}

void QMenuBar::mouseReleaseEvent(QMouseEvent *e)
{
   Q_D(QMenuBar);

   if (e->button() != Qt::LeftButton || !d->mouseDown) {
      return;
   }

   d->mouseDown = false;
   QAction *action = d->actionAt(e->pos());
   if ((d->closePopupMode && action == d->currentAction) || !action || !action->menu()) {
      //we set the current action before activating
      //so that we let the leave event set the current back to 0
      d->setCurrentAction(action, false);
      if (action) {
         d->activateAction(action, QAction::Trigger);
      }
   }
   d->closePopupMode = 0;
}

void QMenuBar::keyPressEvent(QKeyEvent *e)
{
   Q_D(QMenuBar);

   d->updateGeometries();
   int key = e->key();

   if (isRightToLeft()) { // in reverse mode open/close key for submenues are reversed
      if (key == Qt::Key_Left) {
         key = Qt::Key_Right;
      } else if (key == Qt::Key_Right) {
         key = Qt::Key_Left;
      }
   }
   if (key == Qt::Key_Tab) { //means right
      key = Qt::Key_Right;
   } else if (key == Qt::Key_Backtab) { //means left
      key = Qt::Key_Left;
   }

   bool key_consumed = false;
   switch (key) {
      case Qt::Key_Up:
      case Qt::Key_Down:
      case Qt::Key_Enter:
      case Qt::Key_Space:
      case Qt::Key_Return: {
         if (!style()->styleHint(QStyle::SH_MenuBar_AltKeyNavigation, nullptr, this) || !d->currentAction) {
            break;
         }
         if (d->currentAction->menu()) {
            d->popupAction(d->currentAction, true);
         } else if (key == Qt::Key_Enter || key == Qt::Key_Return || key == Qt::Key_Space) {
            d->activateAction(d->currentAction, QAction::Trigger);
            d->setCurrentAction(d->currentAction, false);
            d->setKeyboardMode(false);
         }
         key_consumed = true;
         break;
      }

      case Qt::Key_Right:
      case Qt::Key_Left: {
         if (d->currentAction) {
            int index = d->actions.indexOf(d->currentAction);
            if (QAction *nextAction = d->getNextAction(index, key == Qt::Key_Left ? -1 : +1)) {
               d->setCurrentAction(nextAction, d->popupState, true);
               key_consumed = true;
            }
         }
         break;
      }

      default:
         key_consumed = false;
   }

   if (!key_consumed && e->matches(QKeySequence::Cancel)) {
      d->setCurrentAction(nullptr);
      d->setKeyboardMode(false);
      key_consumed = true;
   }

   if (!key_consumed && ( !e->modifiers() ||
         (e->modifiers() & (Qt::MetaModifier | Qt::AltModifier))) && e->text().length() == 1 && ! d ->popupState) {

      int clashCount = 0;
      QAction *first = nullptr;

      QAction *currentSelected   = nullptr;
      QAction *firstAfterCurrent = nullptr;

      {
         QString char1 = e->text()[0].toUpper();

         for (int i = 0; i < d->actions.size(); ++i) {
            if (d->actionRects.at(i).isNull()) {
               continue;
            }

            QAction *act = d->actions.at(i);
            QString str  = act->text();

            if (! str.isEmpty()) {
               int ampersand = str.indexOf('&');

               if (ampersand >= 0) {
                  if (str[ampersand + 1].toUpper() == char1) {
                     ++clashCount;

                     if (! first) {
                        first = act;
                     }

                     if (act == d->currentAction) {
                        currentSelected = act;

                     } else if (! firstAfterCurrent && currentSelected) {
                        firstAfterCurrent = act;
                     }
                  }
               }
            }
         }
      }

      QAction *next_action = nullptr;

      if (clashCount >= 1) {
         if (clashCount == 1 || !d->currentAction || (currentSelected && !firstAfterCurrent)) {
            next_action = first;
         } else {
            next_action = firstAfterCurrent;
         }
      }

      if (next_action) {
         key_consumed = true;
         d->setCurrentAction(next_action, true, true);
      }
   }

   if (key_consumed) {
      e->accept();
   } else {
      e->ignore();
   }
}

void QMenuBar::mouseMoveEvent(QMouseEvent *e)
{
   Q_D(QMenuBar);

   if (!(e->buttons() & Qt::LeftButton)) {
      d->mouseDown = false;
   }

   bool popupState = d->popupState || d->mouseDown;
   QAction *action = d->actionAt(e->pos());

   if ((action && d->isVisible(action)) || !popupState) {
      d->setCurrentAction(action, popupState);
   }
}

void QMenuBar::leaveEvent(QEvent *)
{
   Q_D(QMenuBar);

   if ((!hasFocus() && !d->popupState) ||
      (d->currentAction && d->currentAction->menu() == nullptr)) {
      d->setCurrentAction(nullptr);
   }
}

QPlatformMenu *getPlatformMenu(QAction *action)
{
   if (!action || !action->menu()) {
      return nullptr;
   }

   return action->menu()->platformMenu();
}

void QMenuBar::actionEvent(QActionEvent *e)
{
   Q_D(QMenuBar);
   d->itemsDirty = true;

   if (d->platformMenuBar) {

      QPlatformMenuBar *nativeMenuBar = d->platformMenuBar;

      if (!nativeMenuBar) {
         return;
      }

      if (e->type() == QEvent::ActionAdded) {
         QPlatformMenu *menu = getPlatformMenu(e->action());

         if (menu) {
            QPlatformMenu *beforeMenu = nullptr;

            for (int beforeIndex = d->indexOf(e->action()) + 1;
               !beforeMenu && (beforeIndex < actions().size());
               ++beforeIndex) {
               beforeMenu = getPlatformMenu(actions().at(beforeIndex));
            }

            menu->setTag(reinterpret_cast<quintptr>(e->action()));
            menu->setText(e->action()->text());
            d->platformMenuBar->insertMenu(menu, beforeMenu);
         }

      } else if (e->type() == QEvent::ActionRemoved) {
         QPlatformMenu *menu = getPlatformMenu(e->action());
         if (menu) {
            d->platformMenuBar->removeMenu(menu);
         }

      } else if (e->type() == QEvent::ActionChanged) {
         QPlatformMenu *cur  = d->platformMenuBar->menuForTag(reinterpret_cast<quintptr>(e->action()));
         QPlatformMenu *menu = getPlatformMenu(e->action());

         // the menu associated with the action can change, need to
         // remove and/or insert the new platform menu

         if (menu != cur) {
            if (cur) {
               d->platformMenuBar->removeMenu(cur);
            }

            if (menu) {
               menu->setTag(reinterpret_cast<quintptr>(e->action()));

               QPlatformMenu *beforeMenu = nullptr;

               for (int beforeIndex = d->indexOf(e->action()) + 1; ! beforeMenu && (beforeIndex < actions().size()); ++beforeIndex) {
                  beforeMenu = getPlatformMenu(actions().at(beforeIndex));
               }

               d->platformMenuBar->insertMenu(menu, beforeMenu);
            }

         } else if (menu) {
            menu->setText(e->action()->text());
            menu->setVisible(e->action()->isVisible());
            menu->setEnabled(e->action()->isEnabled());
            d->platformMenuBar->syncMenu(menu);
         }
      }
   }

   if (e->type() == QEvent::ActionAdded) {
      connect(e->action(), &QAction::triggered, this, &QMenuBar::_q_actionTriggered);
      connect(e->action(), &QAction::hovered,   this, &QMenuBar::_q_actionHovered);

   } else if (e->type() == QEvent::ActionRemoved) {
      e->action()->disconnect(this);
   }

   if (isVisible()) {
      d->updateGeometries();
      update();
   }
}

void QMenuBar::focusInEvent(QFocusEvent *)
{
   Q_D(QMenuBar);
   if (d->keyboardState) {
      d->focusFirstAction();
   }
}

void QMenuBar::focusOutEvent(QFocusEvent *)
{
   Q_D(QMenuBar);
   if (!d->popupState) {
      d->setCurrentAction(nullptr);
      d->setKeyboardMode(false);
   }
}

void QMenuBar::timerEvent (QTimerEvent *e)
{
   Q_D(QMenuBar);
   if (e->timerId() == d->autoReleaseTimer.timerId()) {
      d->autoReleaseTimer.stop();
      d->setCurrentAction(nullptr);
   }
   QWidget::timerEvent(e);
}

void QMenuBarPrivate::handleReparent()
{
   Q_Q(QMenuBar);
   QWidget *newParent = q->parentWidget();
   //Note: if parent is reparented, then window may change even if parent doesn't.
   // We need to install an avent filter on each parent up to the parent that is
   // also a window (for shortcuts)
   QWidget *newWindow = newParent ? newParent->window() : nullptr;

   QVector<QPointer<QWidget>> newParents;

   // Remove event filters on ex-parents, keep them on still-parents
   // The parents are always ordered in the vector

   for (const QPointer<QWidget> &w : oldParents) {
      if (w) {
         if (newParent == w) {
            newParents.append(w);
            if (newParent != newWindow) { //stop at the window
               newParent = newParent->parentWidget();
            }
         } else {
            w->removeEventFilter(q);
         }
      }
   }

   // At this point, newParent is the next one to be added to newParents
   while (newParent && newParent != newWindow) {
      //install event filters all the way up to (excluding) the window
      newParents.append(newParent);
      newParent->installEventFilter(q);
      newParent = newParent->parentWidget();
   }

   if (newParent && newWindow) {
      // Install the event filter on the window
      newParents.append(newParent);
      newParent->installEventFilter(q);
   }
   oldParents = newParents;
   if (platformMenuBar) {
      if (newWindow) {
         // force the underlying platform window to be created, since
         // the platform menubar needs it (and we have no other way to
         // discover when the platform window is created)
         newWindow->createWinId();
         platformMenuBar->handleReparent(newWindow->windowHandle());
      } else {
         platformMenuBar->handleReparent(nullptr);
      }
   }
}

void QMenuBar::changeEvent(QEvent *e)
{
   Q_D(QMenuBar);

   if (e->type() == QEvent::StyleChange) {
      d->itemsDirty = true;
      setMouseTracking(style()->styleHint(QStyle::SH_MenuBar_MouseTracking, nullptr, this));

      if (parentWidget()) {
         resize(parentWidget()->width(), heightForWidth(parentWidget()->width()));
      }

      d->updateGeometries();

   } else if (e->type() == QEvent::ParentChange) {
      d->handleReparent();

   } else if (e->type() == QEvent::FontChange
      || e->type() == QEvent::ApplicationFontChange) {
      d->itemsDirty = true;
      d->updateGeometries();
   }

   QWidget::changeEvent(e);
}

bool QMenuBar::event(QEvent *e)
{
   Q_D(QMenuBar);
   switch (e->type()) {
      case QEvent::KeyPress: {
         QKeyEvent *ke = (QKeyEvent *)e;
         if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab) {
            keyPressEvent(ke);
            return true;
         }

      }
      break;

#ifndef QT_NO_SHORTCUT
      case QEvent::Shortcut: {
         QShortcutEvent *se = static_cast<QShortcutEvent *>(e);
         int shortcutId = se->shortcutId();
         for (int j = 0; j < d->shortcutIndexMap.size(); ++j) {
            if (shortcutId == d->shortcutIndexMap.value(j)) {
               d->_q_internalShortcutActivated(j);
            }
         }
      }
      break;
#endif

      case QEvent::Show:
         d->_q_updateLayout();
         break;

      case QEvent::ShortcutOverride: {
         QKeyEvent *kev = static_cast<QKeyEvent *>(e);
         //we only filter out escape if there is a current action
         if (kev->matches(QKeySequence::Cancel) && d->currentAction) {
            e->accept();
            return true;
         }
      }
      break;

#ifndef QT_NO_WHATSTHIS
      case QEvent::QueryWhatsThis:
         e->setAccepted(d->whatsThis.size());
         if (QAction *action = d->actionAt(static_cast<QHelpEvent *>(e)->pos())) {
            if (action->whatsThis().size() || action->menu()) {
               e->accept();
            }
         }
         return true;
#endif

      case QEvent::LayoutDirectionChange:
         d->_q_updateLayout();
         break;
      default:
         break;
   }
   return QWidget::event(e);
}

bool QMenuBar::eventFilter(QObject *object, QEvent *event)
{
   Q_D(QMenuBar);

   if (event->type() == QEvent::ParentChange) {
      //GrandparentChange
      d->handleReparent();
   }

   if (object == d->leftWidget || object == d->rightWidget) {
      switch (event->type()) {
         case QEvent::ShowToParent:
         case QEvent::HideToParent:
            d->_q_updateLayout();
            break;
         default:
            break;
      }
   }

   if (style()->styleHint(QStyle::SH_MenuBar_AltKeyNavigation, nullptr, this)) {
      if (d->altPressed) {
         switch (event->type()) {
            case QEvent::KeyPress:
            case QEvent::KeyRelease: {
               QKeyEvent *kev = static_cast<QKeyEvent *>(event);
               if (kev->key() == Qt::Key_Alt || kev->key() == Qt::Key_Meta) {
                  if (event->type() == QEvent::KeyPress) {
                     // Alt-press does not interest us, we have the shortcut-override event
                     break;
                  }

                  d->setKeyboardMode(!d->keyboardState);
               }
            }
            [[fallthrough]];

            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
            case QEvent::MouseMove:
            case QEvent::FocusIn:
            case QEvent::FocusOut:
            case QEvent::ActivationChange:
               d->altPressed = false;
               qApp->removeEventFilter(this);
               break;

            default:
               break;
         }

      } else if (isVisible()) {
         if (event->type() == QEvent::ShortcutOverride) {
            QKeyEvent *kev = static_cast<QKeyEvent *>(event);

            if ((kev->key() == Qt::Key_Alt || kev->key() == Qt::Key_Meta)
                  && kev->modifiers() == Qt::AltModifier) {
               d->altPressed = true;
               qApp->installEventFilter(this);
            }
         }
      }
   }

   return false;
}

QAction *QMenuBar::actionAt(const QPoint &pt) const
{
   Q_D(const QMenuBar);
   return d->actionAt(pt);
}

QRect QMenuBar::actionGeometry(QAction *act) const
{
   Q_D(const QMenuBar);
   return d->actionRect(act);
}

QSize QMenuBar::minimumSizeHint() const
{
   Q_D(const QMenuBar);

   const bool as_gui_menubar = !isNativeMenuBar();

   ensurePolished();
   QSize ret(0, 0);

   const_cast<QMenuBarPrivate *>(d)->updateGeometries();
   const int hmargin = style()->pixelMetric(QStyle::PM_MenuBarHMargin, nullptr, this);
   const int vmargin = style()->pixelMetric(QStyle::PM_MenuBarVMargin, nullptr, this);
   int fw = style()->pixelMetric(QStyle::PM_MenuBarPanelWidth, nullptr, this);
   int spaceBelowMenuBar = style()->styleHint(QStyle::SH_MainWindow_SpaceBelowMenuBar, nullptr, this);

   if (as_gui_menubar) {
      int w = parentWidget() ? parentWidget()->width() : QApplication::desktop()->width();
      d->calcActionRects(w - (2 * fw), 0);
      for (int i = 0; ret.isNull() && i < d->actions.count(); ++i) {
         ret = d->actionRects.at(i).size();
      }
      if (!d->extension->isHidden()) {
         ret += QSize(d->extension->sizeHint().width(), 0);
      }
      ret += QSize(2 * fw + hmargin, 2 * fw + vmargin);
   }

   int margin = 2 * vmargin + 2 * fw + spaceBelowMenuBar;

   if (d->leftWidget) {
      QSize sz = d->leftWidget->minimumSizeHint();
      ret.setWidth(ret.width() + sz.width());
      if (sz.height() + margin > ret.height()) {
         ret.setHeight(sz.height() + margin);
      }
   }

   if (d->rightWidget) {
      QSize sz = d->rightWidget->minimumSizeHint();
      ret.setWidth(ret.width() + sz.width());
      if (sz.height() + margin > ret.height()) {
         ret.setHeight(sz.height() + margin);
      }
   }

   if (as_gui_menubar) {
      QStyleOptionMenuItem opt;
      opt.rect = rect();
      opt.menuRect = rect();
      opt.state = QStyle::State_None;
      opt.menuItemType = QStyleOptionMenuItem::Normal;
      opt.checkType = QStyleOptionMenuItem::NotCheckable;
      opt.palette = palette();
      return (style()->sizeFromContents(QStyle::CT_MenuBar, &opt,
               ret.expandedTo(QApplication::globalStrut()),
               this));
   }
   return ret;
}

QSize QMenuBar::sizeHint() const
{
   Q_D(const QMenuBar);

   const bool as_gui_menubar = !isNativeMenuBar();

   ensurePolished();
   QSize ret(0, 0);

   const_cast<QMenuBarPrivate *>(d)->updateGeometries();
   const int hmargin = style()->pixelMetric(QStyle::PM_MenuBarHMargin, nullptr, this);
   const int vmargin = style()->pixelMetric(QStyle::PM_MenuBarVMargin, nullptr, this);

   int fw = style()->pixelMetric(QStyle::PM_MenuBarPanelWidth, nullptr, this);
   int spaceBelowMenuBar = style()->styleHint(QStyle::SH_MainWindow_SpaceBelowMenuBar, nullptr, this);

   if (as_gui_menubar) {
      const int w = parentWidget() ? parentWidget()->width() : QApplication::desktop()->width();
      d->calcActionRects(w - (2 * fw), 0);
      for (int i = 0; i < d->actionRects.count(); ++i) {
         const QRect &actionRect = d->actionRects.at(i);
         ret = ret.expandedTo(QSize(actionRect.x() + actionRect.width(), actionRect.y() + actionRect.height()));
      }
      //the action geometries already contain the top and left
      //margins. So we only need to add those from right and bottom.
      ret += QSize(fw + hmargin, fw + vmargin);
   }

   int margin = 2 * vmargin + 2 * fw + spaceBelowMenuBar;

   if (d->leftWidget) {
      QSize sz = d->leftWidget->sizeHint();
      sz.rheight() += margin;
      ret = ret.expandedTo(sz);
   }

   if (d->rightWidget) {
      QSize sz = d->rightWidget->sizeHint();
      ret.setWidth(ret.width() + sz.width());

      if (sz.height() + margin > ret.height()) {
         ret.setHeight(sz.height() + margin);
      }
   }

   if (as_gui_menubar) {
      QStyleOptionMenuItem opt;
      opt.rect = rect();
      opt.menuRect = rect();
      opt.state = QStyle::State_None;
      opt.menuItemType = QStyleOptionMenuItem::Normal;
      opt.checkType = QStyleOptionMenuItem::NotCheckable;
      opt.palette = palette();

      return (style()->sizeFromContents(QStyle::CT_MenuBar, &opt,
               ret.expandedTo(QApplication::globalStrut()), this));
   }
   return ret;
}

int QMenuBar::heightForWidth(int) const
{
   Q_D(const QMenuBar);

   const bool as_gui_menubar = !isNativeMenuBar();

   const_cast<QMenuBarPrivate *>(d)->updateGeometries();
   int height = 0;
   const int vmargin = style()->pixelMetric(QStyle::PM_MenuBarVMargin, nullptr, this);

   int fw = style()->pixelMetric(QStyle::PM_MenuBarPanelWidth, nullptr, this);
   int spaceBelowMenuBar = style()->styleHint(QStyle::SH_MainWindow_SpaceBelowMenuBar, nullptr, this);

   if (as_gui_menubar) {
      for (int i = 0; i < d->actionRects.count(); ++i) {
         height = qMax(height, d->actionRects.at(i).height());
      }

      if (height) {
         //there is at least one non-null item
         height += spaceBelowMenuBar;
      }
      height += 2 * fw;
      height += 2 * vmargin;
   }
   int margin = 2 * vmargin + 2 * fw + spaceBelowMenuBar;

   if (d->leftWidget) {
      height = qMax(d->leftWidget->sizeHint().height() + margin, height);
   }

   if (d->rightWidget) {
      height = qMax(d->rightWidget->sizeHint().height() + margin, height);
   }

   if (as_gui_menubar) {
      QStyleOptionMenuItem opt;
      opt.initFrom(this);
      opt.menuRect = rect();
      opt.state = QStyle::State_None;
      opt.menuItemType = QStyleOptionMenuItem::Normal;
      opt.checkType = QStyleOptionMenuItem::NotCheckable;
      return style()->sizeFromContents(QStyle::CT_MenuBar, &opt, QSize(0, height), this).height(); //not pretty..
   }
   return height;
}

void QMenuBarPrivate::_q_internalShortcutActivated(int id)
{
   Q_Q(QMenuBar);
   QAction *act = actions.at(id);

   setCurrentAction(act, true, true);

   if (act && !act->menu()) {
      activateAction(act, QAction::Trigger);
      //100 is the same as the default value in QPushButton::animateClick
      autoReleaseTimer.start(100, q);
   } else if (act && q->style()->styleHint(QStyle::SH_MenuBar_AltKeyNavigation, nullptr, q)) {
      // When we open a menu using a shortcut, we should end up in keyboard state
      setKeyboardMode(true);
   }
}

void QMenuBarPrivate::_q_updateLayout()
{
   Q_Q(QMenuBar);
   itemsDirty = true;
   if (q->isVisible()) {
      updateGeometries();
      q->update();
   }
}

void QMenuBar::setCornerWidget(QWidget *w, Qt::Corner corner)
{
   Q_D(QMenuBar);
   switch (corner) {
      case Qt::TopLeftCorner:
         if (d->leftWidget) {
            d->leftWidget->removeEventFilter(this);
         }
         d->leftWidget = w;
         break;
      case Qt::TopRightCorner:
         if (d->rightWidget) {
            d->rightWidget->removeEventFilter(this);
         }
         d->rightWidget = w;
         break;
      default:
         qWarning("QMenuBar::setCornerWidget: Only TopLeftCorner and TopRightCorner are supported");
         return;
   }

   if (w) {
      w->setParent(this);
      w->installEventFilter(this);
   }


   d->_q_updateLayout();
}

QWidget *QMenuBar::cornerWidget(Qt::Corner corner) const
{
   Q_D(const QMenuBar);

   QWidget *w = nullptr;

   switch (corner) {
      case Qt::TopLeftCorner:
         w = d->leftWidget;
         break;

      case Qt::TopRightCorner:
         w = d->rightWidget;
         break;

      default:
         qWarning("QMenuBar::cornerWidget: Only TopLeftCorner and TopRightCorner are supported");
         break;
   }

   return w;
}

void QMenuBar::setNativeMenuBar(bool nativeMenuBar)
{
   Q_D(QMenuBar);

   if (nativeMenuBar != bool(d->platformMenuBar)) {
      if (!nativeMenuBar) {
         delete d->platformMenuBar;
         d->platformMenuBar = nullptr;

      } else {
         if (!d->platformMenuBar) {
            d->platformMenuBar = QGuiApplicationPrivate::platformTheme()->createPlatformMenuBar();
         }
      }

      updateGeometry();
      if (!nativeMenuBar && parentWidget()) {
         setVisible(true);
      }
   }
}

bool QMenuBar::isNativeMenuBar() const
{
   Q_D(const QMenuBar);
   return bool(d->platformMenuBar);
}

QPlatformMenuBar *QMenuBar::platformMenuBar()
{
   Q_D(const QMenuBar);
   return d->platformMenuBar;
}

void QMenuBar::_q_actionTriggered()
{
   Q_D(QMenuBar);
   d->_q_actionTriggered();
}

void QMenuBar::_q_actionHovered()
{
   Q_D(QMenuBar);
   d->_q_actionHovered();
}

void QMenuBar::_q_internalShortcutActivated(int id)
{
   Q_D(QMenuBar);
   d->_q_internalShortcutActivated(id);
}

void QMenuBar::_q_updateLayout()
{
   Q_D(QMenuBar);
   d->_q_updateLayout();
}

#endif // QT_NO_MENUBAR
