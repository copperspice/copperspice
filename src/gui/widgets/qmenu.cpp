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

#include <qmenu.h>

#ifndef QT_NO_MENU

#include <qdebug.h>
#include <qstyle.h>
#include <qevent.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qplatform_theme.h>

#ifdef Q_OS_DARWIN
#include <qmacnativewidget_mac.h>
#endif

#include <qapplication.h>
#include <qdesktopwidget.h>

#ifndef QT_NO_ACCESSIBILITY
#  include <qaccessible.h>
#endif

#ifndef QT_NO_EFFECTS
# include <qeffects_p.h>
#endif

#ifndef QT_NO_WHATSTHIS
# include <qwhatsthis.h>
#endif

#include <qmenu_p.h>
#include <qmenubar_p.h>
#include <qwidgetaction.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qpushbutton_p.h>
#include <qaction_p.h>

#include <qguiapplication_p.h>

QMenu *QMenuPrivate::mouseDown = 0;

/* QMenu code */
// internal class used for the torn off popup
class QTornOffMenu : public QMenu
{
   GUI_CS_OBJECT(QTornOffMenu)

   class QTornOffMenuPrivate : public QMenuPrivate
   {
      Q_DECLARE_PUBLIC(QMenu)

    public:
      QTornOffMenuPrivate(QMenu *p) : causedMenu(p) {
         tornoff = 1;
         causedPopup.widget = 0;
         causedPopup.action = ((QTornOffMenu *)p)->d_func()->causedPopup.action;
         causedStack = ((QTornOffMenu *)p)->d_func()->calcCausedStack();
      }

      QVector<QPointer<QWidget>> calcCausedStack() const override {
         return causedStack;
      }

      QPointer<QMenu> causedMenu;
      QVector<QPointer<QWidget>> causedStack;
   };

 public:
   QTornOffMenu(QMenu *p) : QMenu(*(new QTornOffMenuPrivate(p))) {
      Q_D(QTornOffMenu);

      // make the torn-off menu a sibling of p (instead of a child)
      QWidget *parentWidget = d->causedStack.isEmpty() ? p : d->causedStack.last();
      if (parentWidget->parentWidget()) {
         parentWidget = parentWidget->parentWidget();
      }

      setParent(parentWidget, Qt::Window | Qt::Tool);
      setAttribute(Qt::WA_DeleteOnClose, true);
      setAttribute(Qt::WA_X11NetWmWindowTypeMenu, true);
      setWindowTitle(p->windowTitle());
      setEnabled(p->isEnabled());

      //QObject::connect(this, SIGNAL(triggered(QAction*)), this, SLOT(onTrigger(QAction*)));
      //QObject::connect(this, SIGNAL(hovered(QAction*)), this, SLOT(onHovered(QAction*)));

      QList<QAction *> items = p->actions();
      for (int i = 0; i < items.count(); i++) {
         addAction(items.at(i));
      }
   }

   void syncWithMenu(QMenu *menu, QActionEvent *act) {
      Q_D(QTornOffMenu);
      if (menu != d->causedMenu) {
         return;
      }
      if (act->type() == QEvent::ActionAdded) {
         insertAction(act->before(), act->action());
      } else if (act->type() == QEvent::ActionRemoved) {
         removeAction(act->action());
      }
   }

   void actionEvent(QActionEvent *e)  override {
      QMenu::actionEvent(e);
      setFixedSize(sizeHint());
   }

 public:

   GUI_CS_SLOT_1(Public, void onTrigger(QAction *action))
   GUI_CS_SLOT_2(onTrigger)

   GUI_CS_SLOT_1(Public, void onHovered(QAction *action))
   GUI_CS_SLOT_2(onHovered)

 private:
   Q_DECLARE_PRIVATE(QTornOffMenu)
   friend class QMenuPrivate;
};

void QTornOffMenu::onTrigger(QAction *action)
{
   d_func()->activateAction(action, QAction::Trigger, false);
}

void QTornOffMenu::onHovered(QAction *action)
{
   d_func()->activateAction(action, QAction::Hover, false);
}

void QMenuPrivate::init()
{
   Q_Q(QMenu);

#ifndef QT_NO_WHATSTHIS
   q->setAttribute(Qt::WA_CustomWhatsThis);
#endif

   q->setAttribute(Qt::WA_X11NetWmWindowTypePopupMenu);
   defaultMenuAction = menuAction = new QAction(q);
   menuAction->d_func()->menu = q;
   q->setMouseTracking(q->style()->styleHint(QStyle::SH_Menu_MouseTracking, 0, q));

   if (q->style()->styleHint(QStyle::SH_Menu_Scrollable, 0, q)) {
      scroll = new QMenuPrivate::QMenuScroller;
      scroll->scrollFlags = QMenuPrivate::QMenuScroller::ScrollNone;
   }

   setPlatformMenu(QGuiApplicationPrivate::platformTheme()->createPlatformMenu());
   sloppyState.initialize(q);
   delayState.initialize(q);
   mousePopupDelay = q->style()->styleHint(QStyle::SH_Menu_SubMenuPopupDelay, 0, q);
}

void QMenuPrivate::setPlatformMenu(QPlatformMenu *menu)
{
   Q_Q(QMenu);

   if (! platformMenu.isNull() && ! platformMenu->parent()) {
      delete platformMenu.data();
   }

   platformMenu = menu;

   if (!platformMenu.isNull()) {
      QObject::connect(platformMenu, SIGNAL(aboutToShow()), q, SLOT(_q_platformMenuAboutToShow()));
      QObject::connect(platformMenu, SIGNAL(aboutToHide()), q, SLOT(aboutToHide()));
   }
}

// forward declare function
static void copyActionToPlatformItem(const QAction *action, QPlatformMenuItem *item, QPlatformMenu *itemsMenu);

void QMenuPrivate::syncPlatformMenu()
{
   Q_Q(QMenu);
   if (platformMenu.isNull()) {
      return;
   }

   QPlatformMenuItem *beforeItem = nullptr;
   const QList<QAction *> actions = q->actions();

   for (QList<QAction *>::const_reverse_iterator it = actions.rbegin(), end = actions.rend(); it != end; ++it) {
      QPlatformMenuItem *menuItem = platformMenu->createMenuItem();
      QAction *action = *it;

      menuItem->setTag(reinterpret_cast<quintptr>(action));

      QObject::connect(menuItem, SIGNAL(activated()), action, SLOT(trigger()), Qt::QueuedConnection);
      QObject::connect(menuItem, SIGNAL(hovered()), action,   SLOT(hovered()), Qt::QueuedConnection);

      copyActionToPlatformItem(action, menuItem, platformMenu.data());
      platformMenu->insertMenuItem(menuItem, beforeItem);
      beforeItem = menuItem;
   }
   platformMenu->syncSeparatorsCollapsible(collapsibleSeparators);
   platformMenu->setEnabled(q->isEnabled());
}

int QMenuPrivate::scrollerHeight() const
{
   Q_Q(const QMenu);
   return qMax(QApplication::globalStrut().height(), q->style()->pixelMetric(QStyle::PM_MenuScrollerHeight, 0, q));
}

//Windows and KDE allows menus to cover the taskbar, while GNOME and Mac don't
QRect QMenuPrivate::popupGeometry(const QWidget *widget) const
{
   if (QGuiApplicationPrivate::platformTheme() &&
      QGuiApplicationPrivate::platformTheme()->themeHint(QPlatformTheme::UseFullScreenForPopupMenu).toBool()) {

      return QApplication::desktop()->screenGeometry(widget);

   } else {
      return QApplication::desktop()->availableGeometry(widget);

   }
}

//Windows and KDE allows menus to cover the taskbar, while GNOME and Mac don't
QRect QMenuPrivate::popupGeometry(int screen) const
{

   if (QGuiApplicationPrivate::platformTheme() &&
      QGuiApplicationPrivate::platformTheme()->themeHint(QPlatformTheme::UseFullScreenForPopupMenu).toBool()) {
      return QApplication::desktop()->screenGeometry(screen);

   } else {
      return QApplication::desktop()->availableGeometry(screen);

   }
}

QVector<QPointer<QWidget>> QMenuPrivate::calcCausedStack() const
{
   QVector<QPointer<QWidget>> ret;
   for (QWidget *widget = causedPopup.widget; widget; ) {
      ret.append(widget);
      if (QTornOffMenu *qtmenu = qobject_cast<QTornOffMenu *>(widget)) {
         ret += qtmenu->d_func()->causedStack;
      }

      if (QMenu *qmenu = qobject_cast<QMenu *>(widget)) {
         widget = qmenu->d_func()->causedPopup.widget;
      } else {
         break;
      }
   }

   return ret;
}

void QMenuPrivate::updateActionRects() const
{
   Q_Q(const QMenu);
   updateActionRects(popupGeometry(q));
}

void QMenuPrivate::updateActionRects(const QRect &screen) const
{
   Q_Q(const QMenu);
   if (!itemsDirty) {
      return;
   }

   q->ensurePolished();

   //let's reinitialize the buffer
   actionRects.resize(actions.count());
   actionRects.fill(QRect());

   int lastVisibleAction = getLastVisibleAction();

   int max_column_width = 0,
       dh = screen.height(),
       y = 0;
   QStyle *style = q->style();
   QStyleOption opt;
   opt.init(q);
   const int hmargin = style->pixelMetric(QStyle::PM_MenuHMargin, &opt, q),
             vmargin = style->pixelMetric(QStyle::PM_MenuVMargin, &opt, q),
             icone = style->pixelMetric(QStyle::PM_SmallIconSize, &opt, q);
   const int fw = style->pixelMetric(QStyle::PM_MenuPanelWidth, &opt, q);
   const int deskFw = style->pixelMetric(QStyle::PM_MenuDesktopFrameWidth, &opt, q);
   const int tearoffHeight = tearoff ? style->pixelMetric(QStyle::PM_MenuTearoffHeight, &opt, q) : 0;

   //for compatibility now - will have to refactor this away
   tabWidth = 0;
   maxIconWidth = 0;
   hasCheckableItems = false;
   ncols = 1;

   for (int i = 0; i < actions.count(); ++i) {
      QAction *action = actions.at(i);
      if (action->isSeparator() || !action->isVisible() || widgetItems.contains(action)) {
         continue;
      }
      //..and some members
      hasCheckableItems |= action->isCheckable();
      QIcon is = action->icon();
      if (!is.isNull()) {
         maxIconWidth = qMax(maxIconWidth, icone + 4);
      }
   }

   //calculate size
   QFontMetrics qfm = q->fontMetrics();
   bool previousWasSeparator = true; // this is true to allow removing the leading separators
   for (int i = 0; i <= lastVisibleAction; i++) {
      QAction *action = actions.at(i);

      const bool isSection = action->isSeparator() && (!action->text().isEmpty() || !action->icon().isNull());
      const bool isPlainSeparator = (isSection && !q->style()->styleHint(QStyle::SH_Menu_SupportsSections))
         || (action->isSeparator() && !isSection);

      if (!action->isVisible() ||
         (collapsibleSeparators && previousWasSeparator && isPlainSeparator))  {
         continue;   // we continue, this action will get an empty QRect
      }

      previousWasSeparator = isPlainSeparator;

      //let the style modify the above size..
      QStyleOptionMenuItem opt;
      q->initStyleOption(&opt, action);
      const QFontMetrics &fm = opt.fontMetrics;

      QSize sz;
      if (QWidget *w = widgetItems.value(action)) {
         sz = w->sizeHint().expandedTo(w->minimumSize()).expandedTo(w->minimumSizeHint()).boundedTo(w->maximumSize());
      } else {
         //calc what I think the size is..
         if (action->isSeparator()) {
            sz = QSize(2, 2);
         } else {
            QString s = action->text();
            int t = s.indexOf(QLatin1Char('\t'));

            if (t != -1) {
               tabWidth = qMax(int(tabWidth), qfm.width(s.mid(t + 1)));
               s = s.left(t);

#ifndef QT_NO_SHORTCUT
            } else {
               QKeySequence seq = action->shortcut();

               if (! seq.isEmpty()) {
                  tabWidth = qMax(int(tabWidth), qfm.width(seq.toString(QKeySequence::NativeText)));
               }
#endif
            }

            sz.setWidth(fm.boundingRect(QRect(), Qt::TextSingleLine | Qt::TextShowMnemonic, s).width());
            sz.setHeight(qMax(fm.height(), qfm.height()));

            QIcon is = action->icon();
            if (!is.isNull()) {
               QSize is_sz = QSize(icone, icone);
               if (is_sz.height() > sz.height()) {
                  sz.setHeight(is_sz.height());
               }
            }
         }
         sz = style->sizeFromContents(QStyle::CT_MenuItem, &opt, sz, q);
      }


      if (!sz.isEmpty()) {
         max_column_width = qMax(max_column_width, sz.width());
         //wrapping
         if (!scroll &&
            y + sz.height() + vmargin > dh - (deskFw * 2)) {
            ncols++;
            y = vmargin;
         }
         y += sz.height();
         //update the item
         actionRects[i] = QRect(0, 0, sz.width(), sz.height());
      }
   }

   max_column_width += tabWidth; //finally add in the tab width
   const int sfcMargin = style->sizeFromContents(QStyle::CT_Menu, &opt, QApplication::globalStrut(),
         q).width() - QApplication::globalStrut().width();

   const int min_column_width = q->minimumWidth() - (sfcMargin + leftmargin + rightmargin + 2 * (fw + hmargin));
   max_column_width = qMax(min_column_width, max_column_width);

   //calculate position
   const int base_y = vmargin + fw + topmargin +
      (scroll ? scroll->scrollOffset : 0) +
      tearoffHeight;
   int x = hmargin + fw + leftmargin;
   y = base_y;

   for (int i = 0; i < actions.count(); i++) {
      QRect &rect = actionRects[i];
      if (rect.isNull()) {
         continue;
      }
      if (!scroll &&
         y + rect.height() > dh - deskFw * 2) {
         x += max_column_width + hmargin;
         y = base_y;
      }
      rect.translate(x, y);                        //move
      rect.setWidth(max_column_width); //uniform width

      //we need to update the widgets geometry
      if (QWidget *widget = widgetItems.value(actions.at(i))) {
         widget->setGeometry(rect);
         widget->setVisible(actions.at(i)->isVisible());
      }

      y += rect.height();
   }
   itemsDirty = 0;
}

QSize QMenuPrivate::adjustMenuSizeForScreen(const QRect &screen)
{
   Q_Q(QMenu);
   QSize ret = screen.size();
   itemsDirty = true;
   updateActionRects(screen);
   const int fw = q->style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, q);
   ret.setWidth(actionRects.at(getLastVisibleAction()).right() + fw);
   return ret;
}

int QMenuPrivate::getLastVisibleAction() const
{
   //let's try to get the last visible action
   int lastVisibleAction = actions.count() - 1;
   for (; lastVisibleAction >= 0; --lastVisibleAction) {
      const QAction *action = actions.at(lastVisibleAction);
      if (action->isVisible()) {
         //removing trailing separators
         if (action->isSeparator() && collapsibleSeparators) {
            continue;
         }
         break;
      }
   }
   return lastVisibleAction;
}


QRect QMenuPrivate::actionRect(QAction *act) const
{
   int index = actions.indexOf(act);
   if (index == -1) {
      return QRect();
   }

   updateActionRects();

   //we found the action
   return actionRects.at(index);
}

void QMenuPrivate::hideUpToMenuBar()
{
   Q_Q(QMenu);
   bool fadeMenus = q->style()->styleHint(QStyle::SH_Menu_FadeOutOnHide);
   if (!tornoff) {
      QWidget *caused = causedPopup.widget;
      hideMenu(q); //hide after getting causedPopup
      while (caused) {
#ifndef QT_NO_MENUBAR
         if (QMenuBar *mb = qobject_cast<QMenuBar *>(caused)) {
            mb->d_func()->setCurrentAction(0);
            mb->d_func()->setKeyboardMode(false);
            caused = 0;
         } else
#endif
            if (QMenu *m = qobject_cast<QMenu *>(caused)) {
               caused = m->d_func()->causedPopup.widget;

               if (! m->d_func()->tornoff) {
                  hideMenu(m);
               }

               if (!fadeMenus) {
                  // Mac does not clear the action until after hidden
                  m->d_func()->setCurrentAction(0);
               }

            } else {
               caused = 0;
            }
      }
   }
   setCurrentAction(0);
}

void QMenuPrivate::hideMenu(QMenu *menu)
{
   if (!menu) {
      return;
   }

#if ! defined(QT_NO_EFFECTS)
   bool blockValue = menu->blockSignals(true);
   aboutToHide = true;

   // Flash item which is about to trigger (if any).
   if (menu->style()->styleHint(QStyle::SH_Menu_FlashTriggeredItem)
      && currentAction && currentAction == actionAboutToTrigger
      && menu->actions().contains(currentAction)) {
      QEventLoop eventLoop;
      QAction *activeAction = currentAction;

      menu->setActiveAction(0);
      QTimer::singleShot(60, &eventLoop, SLOT(quit()));
      eventLoop.exec();

      // Select and wait 20 ms.
      menu->setActiveAction(activeAction);
      QTimer::singleShot(20, &eventLoop, SLOT(quit()));
      eventLoop.exec();
   }

   aboutToHide = false;
   menu->blockSignals(blockValue);
#endif

   if (activeMenu == menu) {
      activeMenu = 0;
   }

   menu->d_func()->causedPopup.action = 0;
   menu->close();
   menu->d_func()->causedPopup.widget = 0;
}

void QMenuPrivate::popupAction(QAction *action, int delay, bool activateFirst)
{
   Q_Q(QMenu);

   if (action ) {

      if (action->isEnabled()) {

         if (! delay) {
            q->internalDelayedPopup();

         } else if (action->menu() && ! action->menu()->isVisible()) {
            delayState.start(delay, action);

         } else if (!action->menu()) {
            delayState.stop();
         }

         if (activateFirst && action->menu()) {
            action->menu()->d_func()->setFirstActionActive();
         }
      }

   } else if (QMenu *menu = activeMenu) {
      //hide the current item
      hideMenu(menu);
   }

}

void QMenuPrivate::setSyncAction()
{
   Q_Q(QMenu);
   QAction *current = currentAction;

   if (current && (!current->isEnabled() || current->menu() || current->isSeparator())) {
      current = 0;
   }

   for (QWidget *caused = q; caused;) {
      if (QMenu *m = qobject_cast<QMenu *>(caused)) {
         caused = m->d_func()->causedPopup.widget;

         if (m->d_func()->eventLoop) {
            m->d_func()->syncAction = current;   // synchronous operation
         }

      } else {
         break;
      }
   }
}


void QMenuPrivate::setFirstActionActive()
{
   Q_Q(QMenu);
   updateActionRects();
   for (int i = 0, saccum = 0; i < actions.count(); i++) {
      const QRect &rect = actionRects.at(i);
      if (rect.isNull()) {
         continue;
      }
      if (scroll && scroll->scrollFlags & QMenuScroller::ScrollUp) {
         saccum -= rect.height();
         if (saccum > scroll->scrollOffset - scrollerHeight()) {
            continue;
         }
      }

      QAction *act = actions.at(i);
      if (!act->isSeparator() &&
         (q->style()->styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, 0, q)
            || act->isEnabled())) {
         setCurrentAction(act);
         break;
      }
   }
}

// popup == -1 means do not popup, 0 means immediately, others mean use a timer
void QMenuPrivate::setCurrentAction(QAction *action, int popup, SelectionReason reason, bool activateFirst)
{
   Q_Q(QMenu);
   tearoffHighlighted = 0;

   if (action && (action->isSeparator()
         || (!action->isEnabled() && ! q->style()->styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, 0, q)))) {
      action = nullptr;
   }
   // Reselect the currently active action in case mouse moved over other menu items when
   // moving from sub menu action to sub menu (QTBUG-20094).
   if (reason != SelectedFromKeyboard) {
      if (QMenu *menu = qobject_cast<QMenu *>(causedPopup.widget)) {
         if (causedPopup.action && menu->d_func()->activeMenu == q)
            // Reselect parent menu action only if mouse is over a menu and parent menu action is not already selected (QTBUG-47987)
            if (hasReceievedEnter && menu->d_func()->currentAction != causedPopup.action) {
               menu->d_func()->setCurrentAction(causedPopup.action, 0, reason, false);
            }
      }
   }

   if (currentAction) {
      q->update(actionRect(currentAction));
   }

   QMenu *hideActiveMenu = activeMenu;
   QAction *previousAction = currentAction;

   currentAction = action;
   if (action) {
      if (!action->isSeparator()) {
         activateAction(action, QAction::Hover);

         if (popup != -1) {
            // if the menu is visible then activate the required action,
            // otherwise we just mark the action as currentAction
            // and activate it when the menu will be popuped.
            if (q->isVisible()) {
               popupAction(currentAction, popup, activateFirst);
            }
         }

         q->update(actionRect(action));

         if (reason == SelectedFromKeyboard) {
            QWidget *widget = widgetItems.value(action);
            if (widget) {
               if (widget->focusPolicy() != Qt::NoFocus) {
                  widget->setFocus(Qt::TabFocusReason);
               }
            } else {
               //when the action has no QWidget, the QMenu itself should
               // get the focus
               // Since the menu is a pop-up, it uses the popup reason.
               if (!q->hasFocus()) {
                  q->setFocus(Qt::PopupFocusReason);
               }
            }
         }
      }

#ifndef QT_NO_STATUSTIP
   }  else if (previousAction) {
      previousAction->d_func()->showStatusText(topCausedWidget(), QString());
#endif
   }

   if (hideActiveMenu && previousAction != currentAction) {
      if (popup == -1) {

#ifndef QT_NO_EFFECTS
         // kill any running effect
         qFadeEffect(0);
         qScrollEffect(0);
#endif
         hideMenu(hideActiveMenu);
      } else if (!currentAction || !currentAction->menu()) {
         sloppyState.startTimerIfNotRunning();
      }
   }
}

void QMenuSloppyState::reset()
{
   m_enabled = false;
   m_first_mouse = true;
   m_init_guard = false;
   m_uni_dir_discarded_count = 0;
   m_time.stop();
   m_reset_action = nullptr;
   m_origin_action = nullptr;
   m_action_rect = QRect();
   m_previous_point = QPointF();
   if (m_sub_menu) {
      QMenuPrivate::get(m_sub_menu)->sloppyState.m_parent = nullptr;
      m_sub_menu = nullptr;
   }
}

void QMenuSloppyState::enter()
{
   QMenuPrivate *menuPriv = QMenuPrivate::get(m_menu);

   if (m_discard_state_when_entering_parent && m_sub_menu == menuPriv->activeMenu) {
      menuPriv->hideMenu(m_sub_menu);
      reset();
   }
   if (m_parent) {
      m_parent->childEnter();
   }
}
void QMenuSloppyState::childEnter()
{
   stopTimer();
   if (m_parent) {
      m_parent->childEnter();
   }
}

void QMenuSloppyState::leave()
{
   if (!m_dont_start_time_on_leave) {
      if (m_parent) {
         m_parent->childLeave();
      }
      startTimerIfNotRunning();
   }
}
void QMenuSloppyState::childLeave()
{
   if (m_enabled && !QMenuPrivate::get(m_menu)->hasReceievedEnter) {
      startTimerIfNotRunning();
      if (m_parent) {
         m_parent->childLeave();
      }
   }
}

void QMenuSloppyState::setSubMenuPopup(const QRect &actionRect, QAction *resetAction, QMenu *subMenu)
{
   m_enabled = true;
   m_init_guard = true;
   m_time.stop();
   m_action_rect = actionRect;
   m_sub_menu = subMenu;
   QMenuPrivate::get(subMenu)->sloppyState.m_parent = this;
   m_reset_action = resetAction;
   m_origin_action = resetAction;
}
bool QMenuSloppyState::hasParentActiveDelayTimer() const
{
   return m_parent && m_parent->m_menu && QMenuPrivate::get(m_parent->m_menu)->delayState.timer.isActive();
}

class ResetOnDestroy
{
 public:
   ResetOnDestroy(QMenuSloppyState *sloppyState, bool *guard)
      : toReset(sloppyState)
      , guard(guard) {
      *guard = false;
   }

   ~ResetOnDestroy() {
      if (!*guard) {
         toReset->reset();
      }
   }

   QMenuSloppyState *toReset;
   bool *guard;
};
void QMenuSloppyState::timeout()
{
   QMenuPrivate *menu_priv = QMenuPrivate::get(m_menu);

   bool reallyHasMouse = menu_priv->hasReceievedEnter;
   if (!reallyHasMouse) {
      // Check whether the menu really has a mouse, because only active popup
      // menu gets the enter/leave events. Currently Cocoa is an exception.
      const QPoint lastCursorPos = QGuiApplicationPrivate::lastCursorPosition.toPoint();
      reallyHasMouse = m_menu->frameGeometry().contains(lastCursorPos);
   }

   if (menu_priv->currentAction == m_reset_action
      && reallyHasMouse
      && (menu_priv->currentAction
         && menu_priv->currentAction->menu() == menu_priv->activeMenu)) {
      return;
   }

   ResetOnDestroy resetState(this, &m_init_guard);

   if (hasParentActiveDelayTimer() || !m_menu->isVisible()) {
      return;
   }

   if (m_sub_menu) {
      menu_priv->hideMenu(m_sub_menu);
   }

   if (reallyHasMouse) {
      menu_priv->setCurrentAction(m_reset_action, 0);
   } else {
      menu_priv->setCurrentAction(nullptr, 0);
   }
}
//return the top causedPopup.widget that is not a QMenu
QWidget *QMenuPrivate::topCausedWidget() const
{
   QWidget *top = causedPopup.widget;
   while (QMenu *m = qobject_cast<QMenu *>(top)) {
      top = m->d_func()->causedPopup.widget;
   }
   return top;
}

QAction *QMenuPrivate::actionAt(QPoint p) const
{
   if (!q_func()->rect().contains(p)) {   //sanity check
      return 0;
   }

   for (int i = 0; i < actionRects.count(); i++) {
      if (actionRects.at(i).contains(p)) {
         return actions.at(i);
      }
   }
   return 0;
}

void QMenuPrivate::setOverrideMenuAction(QAction *a)
{
   Q_Q(QMenu);
   QObject::disconnect(menuAction, SIGNAL(destroyed()), q, SLOT(_q_overrideMenuActionDestroyed()));
   if (a) {
      menuAction = a;
      QObject::connect(a, SIGNAL(destroyed()), q, SLOT(_q_overrideMenuActionDestroyed()));
   } else { //we revert back to the default action created by the QMenu itself
      menuAction = defaultMenuAction;
   }
}

void QMenuPrivate::_q_overrideMenuActionDestroyed()
{
   menuAction = defaultMenuAction;
}

void QMenuPrivate::updateLayoutDirection()
{
   Q_Q(QMenu);
   //we need to mimic the cause of the popup's layout direction
   //to allow setting it on a mainwindow for example
   //we call setLayoutDirection_helper to not overwrite a user-defined value

   if (! q->testAttribute(Qt::WA_SetLayoutDirection)) {
      if (QWidget *w = causedPopup.widget) {
         setLayoutDirection_helper(w->layoutDirection());

      } else if (QWidget *w = q->parentWidget()) {
         setLayoutDirection_helper(w->layoutDirection());

      } else {
         setLayoutDirection_helper(QApplication::layoutDirection());

      }
   }
}


/*!
    Returns the action associated with this menu.
*/
QAction *QMenu::menuAction() const
{
   return d_func()->menuAction;
}

QString QMenu::title() const
{
   return d_func()->menuAction->text();
}

void QMenu::setTitle(const QString &text)
{
   d_func()->menuAction->setText(text);
}


QIcon QMenu::icon() const
{
   return d_func()->menuAction->icon();
}

void QMenu::setIcon(const QIcon &icon)
{
   d_func()->menuAction->setIcon(icon);
}


//actually performs the scrolling
void QMenuPrivate::scrollMenu(QAction *action, QMenuScroller::ScrollLocation location, bool active)
{
   Q_Q(QMenu);
   if (! scroll || !scroll->scrollFlags) {
      return;
   }
   updateActionRects();
   int newOffset = 0;
   const int topScroll = (scroll->scrollFlags & QMenuScroller::ScrollUp)   ? scrollerHeight() : 0;
   const int botScroll = (scroll->scrollFlags & QMenuScroller::ScrollDown) ? scrollerHeight() : 0;
   const int vmargin = q->style()->pixelMetric(QStyle::PM_MenuVMargin, 0, q);
   const int fw = q->style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, q);

   if (location == QMenuScroller::ScrollTop) {
      for (int i = 0, saccum = 0; i < actions.count(); i++) {
         if (actions.at(i) == action) {
            newOffset = topScroll - saccum;
            break;
         }
         saccum += actionRects.at(i).height();
      }
   } else {
      for (int i = 0, saccum = 0; i < actions.count(); i++) {
         saccum += actionRects.at(i).height();
         if (actions.at(i) == action) {
            if (location == QMenuScroller::ScrollCenter) {
               newOffset = ((q->height() / 2) - botScroll) - (saccum - topScroll);
            } else {
               newOffset = (q->height() - botScroll) - saccum;
            }
            break;
         }
      }
      if (newOffset) {
         newOffset -= fw * 2;
      }
   }

   //figure out which scroll flags
   uint newScrollFlags = QMenuScroller::ScrollNone;
   if (newOffset < 0) { //easy and cheap one
      newScrollFlags |= QMenuScroller::ScrollUp;
   }

   int saccum = newOffset;
   for (int i = 0; i < actionRects.count(); i++) {
      saccum += actionRects.at(i).height();
      if (saccum > q->height()) {
         newScrollFlags |= QMenuScroller::ScrollDown;
         break;
      }
   }

   if (!(newScrollFlags & QMenuScroller::ScrollDown) && (scroll->scrollFlags & QMenuScroller::ScrollDown)) {
      newOffset = q->height() - (saccum - newOffset) - fw * 2 - vmargin;  //last item at bottom
   }

   if (!(newScrollFlags & QMenuScroller::ScrollUp) && (scroll->scrollFlags & QMenuScroller::ScrollUp)) {
      newOffset = 0;  //first item at top
   }

   if (newScrollFlags & QMenuScroller::ScrollUp) {
      newOffset -= vmargin;
   }

   QRect screen = popupGeometry(q);
   const int desktopFrame = q->style()->pixelMetric(QStyle::PM_MenuDesktopFrameWidth, 0, q);
   if (q->height() < screen.height() - (desktopFrame * 2) - 1) {
      QRect geom = q->geometry();
      if (newOffset > scroll->scrollOffset && (scroll->scrollFlags & newScrollFlags & QMenuScroller::ScrollUp)) { //scroll up
         const int newHeight = geom.height() - (newOffset - scroll->scrollOffset);
         if (newHeight > geom.height()) {
            geom.setHeight(newHeight);
         }
      } else if (scroll->scrollFlags & newScrollFlags & QMenuScroller::ScrollDown) {
         int newTop = geom.top() + (newOffset - scroll->scrollOffset);
         if (newTop < desktopFrame + screen.top()) {
            newTop = desktopFrame + screen.top();
         }
         if (newTop < geom.top()) {
            geom.setTop(newTop);
            newOffset = 0;
            newScrollFlags &= ~QMenuScroller::ScrollUp;
         }
      }
      if (geom.bottom() > screen.bottom() - desktopFrame) {
         geom.setBottom(screen.bottom() - desktopFrame);
      }

      if (geom.top() < desktopFrame + screen.top()) {
         geom.setTop(desktopFrame + screen.top());
      }

      if (geom != q->geometry()) {
         q->setGeometry(geom);
      }
   }

   //actually update flags
   const int delta = qMin(0, newOffset) - scroll->scrollOffset; //make sure the new offset is always negative
   if (!itemsDirty && delta) {
      //we've scrolled so we need to update the action rects
      for (int i = 0; i < actionRects.count(); ++i) {
         QRect &current = actionRects[i];
         current.moveTop(current.top() + delta);

         //we need to update the widgets geometry
         if (QWidget *w = widgetItems.value(actions.at(i))) {
            w->setGeometry(current);
         }
      }
   }
   scroll->scrollOffset += delta;
   scroll->scrollFlags = newScrollFlags;
   if (active) {
      setCurrentAction(action);
   }

   q->update();     //issue an update so we see all the new state..
}

void QMenuPrivate::scrollMenu(QMenuScroller::ScrollLocation location, bool active)
{
   Q_Q(QMenu);
   updateActionRects();
   if (location == QMenuScroller::ScrollBottom) {
      for (int i = actions.size() - 1; i >= 0; --i) {
         QAction *act = actions.at(i);
         if (actionRects.at(i).isNull()) {
            continue;
         }
         if (!act->isSeparator() &&
            (q->style()->styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, 0, q)
               || act->isEnabled())) {
            if (scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollDown) {
               scrollMenu(act, QMenuPrivate::QMenuScroller::ScrollBottom, active);
            } else if (active) {
               setCurrentAction(act, /*popup*/ -1, QMenuPrivate::SelectedFromKeyboard);
            }
            break;
         }
      }
   } else if (location == QMenuScroller::ScrollTop) {
      for (int i = 0; i < actions.size(); ++i) {
         QAction *act = actions.at(i);
         if (actionRects.at(i).isNull()) {
            continue;
         }
         if (!act->isSeparator() &&
            (q->style()->styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, 0, q)
               || act->isEnabled())) {
            if (scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp) {
               scrollMenu(act, QMenuPrivate::QMenuScroller::ScrollTop, active);
            } else if (active) {
               setCurrentAction(act, /*popup*/ -1, QMenuPrivate::SelectedFromKeyboard);
            }
            break;
         }
      }
   }
}

//only directional
void QMenuPrivate::scrollMenu(QMenuScroller::ScrollDirection direction, bool page, bool active)
{
   Q_Q(QMenu);

   if (!scroll || !(scroll->scrollFlags & direction)) {
      //not really possible...
      return;
   }

   updateActionRects();
   const int topScroll = (scroll->scrollFlags & QMenuScroller::ScrollUp)   ? scrollerHeight() : 0;
   const int botScroll = (scroll->scrollFlags & QMenuScroller::ScrollDown) ? scrollerHeight() : 0;
   const int vmargin = q->style()->pixelMetric(QStyle::PM_MenuVMargin, 0, q);
   const int fw = q->style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, q);
   const int offset = topScroll ? topScroll - vmargin : 0;

   if (direction == QMenuScroller::ScrollUp) {
      for (int i = 0, saccum = 0; i < actions.count(); i++) {
         saccum -= actionRects.at(i).height();
         if (saccum <= scroll->scrollOffset - offset) {
            scrollMenu(actions.at(i), page ? QMenuScroller::ScrollBottom : QMenuScroller::ScrollTop, active);
            break;
         }
      }
   } else if (direction == QMenuScroller::ScrollDown) {
      bool scrolled = false;
      for (int i = 0, saccum = 0; i < actions.count(); i++) {
         const int iHeight = actionRects.at(i).height();
         saccum -= iHeight;
         if (saccum <= scroll->scrollOffset - offset) {
            const int scrollerArea = q->height() - botScroll - fw * 2;
            int visible = (scroll->scrollOffset - offset) - saccum;
            for (i++ ; i < actions.count(); i++) {
               visible += actionRects.at(i).height();
               if (visible > scrollerArea - topScroll) {
                  scrolled = true;
                  scrollMenu(actions.at(i), page ? QMenuScroller::ScrollTop : QMenuScroller::ScrollBottom, active);
                  break;
               }
            }
            break;
         }
      }
      if (!scrolled) {
         scroll->scrollFlags &= ~QMenuScroller::ScrollDown;
         q->update();
      }
   }
}

/* This is poor-mans eventfilters. This avoids the use of
   eventFilter (which can be nasty for users of QMenuBar's). */
bool QMenuPrivate::mouseEventTaken(QMouseEvent *e)
{
   Q_Q(QMenu);
   QPoint pos = q->mapFromGlobal(e->globalPos());
   if (scroll && !activeMenu) { //let the scroller "steal" the event
      bool isScroll = false;
      if (pos.x() >= 0 && pos.x() < q->width()) {
         for (int dir = QMenuScroller::ScrollUp; dir <= QMenuScroller::ScrollDown; dir = dir << 1) {
            if (scroll->scrollFlags & dir) {
               if (dir == QMenuScroller::ScrollUp) {
                  isScroll = (pos.y() <= scrollerHeight());
               } else if (dir == QMenuScroller::ScrollDown) {
                  isScroll = (pos.y() >= q->height() - scrollerHeight());
               }
               if (isScroll) {
                  scroll->scrollDirection = dir;
                  break;
               }
            }
         }
      }
      if (isScroll) {
         scroll->scrollTimer.start(50, q);
         return true;
      } else {
         scroll->scrollTimer.stop();
      }
   }

   if (tearoff) { //let the tear off thingie "steal" the event..
      QRect tearRect(0, 0, q->width(), q->style()->pixelMetric(QStyle::PM_MenuTearoffHeight, 0, q));
      if (scroll && scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp) {
         tearRect.translate(0, scrollerHeight());
      }
      q->update(tearRect);
      if (tearRect.contains(pos) && hasMouseMoved(e->globalPos())) {
         setCurrentAction(0);
         tearoffHighlighted = 1;
         if (e->type() == QEvent::MouseButtonRelease) {
            if (!tornPopup) {
               tornPopup = new QTornOffMenu(q);
            }
            tornPopup->setGeometry(q->geometry());
            tornPopup->show();
            hideUpToMenuBar();
         }
         return true;
      }
      tearoffHighlighted = 0;
   }

   if (q->frameGeometry().contains(e->globalPos())) {
      //otherwise if the event is in our rect we want it..
      return false;
   }

   for (QWidget *caused = causedPopup.widget; caused;) {
      bool passOnEvent = false;
      QWidget *next_widget = 0;
      QPoint cpos = caused->mapFromGlobal(e->globalPos());

#ifndef QT_NO_MENUBAR
      if (QMenuBar *mb = qobject_cast<QMenuBar *>(caused)) {
         passOnEvent = mb->rect().contains(cpos);
      } else
#endif

         if (QMenu *m = qobject_cast<QMenu *>(caused)) {
            passOnEvent = m->rect().contains(cpos);
            next_widget = m->d_func()->causedPopup.widget;
         }
      if (passOnEvent) {
         if (e->type() != QEvent::MouseButtonRelease || mouseDown == caused) {
            QMouseEvent new_e(e->type(), cpos, caused->mapTo(caused->topLevelWidget(), cpos), e->screenPos(),
               e->button(), e->buttons(), e->modifiers(), e->source());
            QApplication::sendEvent(caused, &new_e);
            return true;
         }
      }

      caused = next_widget;
      if (!caused) {
         sloppyState.leave();   // Start timers
      }
   }
   return false;
}

void QMenuPrivate::activateCausedStack(const QVector<QPointer<QWidget>> &causedStack, QAction *action, QAction::ActionEvent action_e,
   bool self)
{
   bool oldValue = activationRecursionGuard;
   activationRecursionGuard = true;

   if (self) {
      action->activate(action_e);
   }

   for (int i = 0; i < causedStack.size(); ++i) {
      QPointer<QWidget> widget = causedStack.at(i);

      if (!widget) {
         continue;
      }

      //fire
      if (QMenu *qmenu = qobject_cast<QMenu *>(widget)) {
         widget = qmenu->d_func()->causedPopup.widget;

         if (action_e == QAction::Trigger) {
            emit qmenu->triggered(action);
         } else if (action_e == QAction::Hover) {
            emit qmenu->hovered(action);
         }

#ifndef QT_NO_MENUBAR
      } else if (QMenuBar *qmenubar = qobject_cast<QMenuBar *>(widget)) {
         if (action_e == QAction::Trigger) {
            emit qmenubar->triggered(action);

         } else if (action_e == QAction::Hover) {
            emit qmenubar->hovered(action);

         }

         break;
#endif
      }
   }

   activationRecursionGuard = oldValue;
}

void QMenuPrivate::activateAction(QAction *action, QAction::ActionEvent action_e, bool self)
{
   Q_Q(QMenu);

#ifndef QT_NO_WHATSTHIS
   bool inWhatsThisMode = QWhatsThis::inWhatsThisMode();
#endif

   if (!action || !q->isEnabled()
      || (action_e == QAction::Trigger
#ifndef QT_NO_WHATSTHIS
         && !inWhatsThisMode
#endif
         && (action->isSeparator() || !action->isEnabled()))) {
      return;
   }

   /* I have to save the caused stack here because it will be undone after popup execution (ie in the hide).
      Then I iterate over the list to actually send the events. --Sam
   */
   const QVector<QPointer<QWidget>> causedStack = calcCausedStack();
   if (action_e == QAction::Trigger) {
#ifndef QT_NO_WHATSTHIS
      if (!inWhatsThisMode) {
         actionAboutToTrigger = action;
      }
#endif

      if (q->testAttribute(Qt::WA_DontShowOnScreen)) {
         hideUpToMenuBar();
      } else {
         for (QWidget *widget = QApplication::activePopupWidget(); widget; ) {
            if (QMenu *qmenu = qobject_cast<QMenu *>(widget)) {
               if (qmenu == q) {
                  hideUpToMenuBar();
               }
               widget = qmenu->d_func()->causedPopup.widget;
            } else {
               break;
            }
         }
      }

#ifndef QT_NO_WHATSTHIS
      if (inWhatsThisMode) {
         QString s = action->whatsThis();
         if (s.isEmpty()) {
            s = whatsThis;
         }
         QWhatsThis::showText(q->mapToGlobal(actionRect(action).center()), s, q);
         return;
      }
#endif
   }


   activateCausedStack(causedStack, action, action_e, self);


   if (action_e == QAction::Hover) {
#ifndef QT_NO_ACCESSIBILITY
      if (QAccessible::isActive()) {
         int actionIndex = indexOf(action);
         QAccessibleEvent focusEvent(q, QAccessible::Focus);
         focusEvent.setChild(actionIndex);
         QAccessible::updateAccessibility(&focusEvent);
      }
#endif
      action->showStatusText(topCausedWidget());
   } else {
      actionAboutToTrigger = 0;
   }
}

void QMenuPrivate::_q_actionTriggered()
{
   Q_Q(QMenu);

   if (QAction *action = qobject_cast<QAction *>(q->sender())) {
      QPointer<QAction> actionGuard = action;

      if (platformMenu && widgetItems.value(action)) {
         platformMenu->dismiss();
      }
      emit q->triggered(action);

      if (!activationRecursionGuard && actionGuard) {
         //in case the action has not been activated by the mouse
         //we check the parent hierarchy
         QVector< QPointer<QWidget>> list;

         for (QWidget *widget = q->parentWidget(); widget; ) {
            if (qobject_cast<QMenu *>(widget)
#ifndef QT_NO_MENUBAR
               || qobject_cast<QMenuBar *>(widget)
#endif
            ) {
               list.append(widget);
               widget = widget->parentWidget();
            } else {
               break;
            }
         }
         activateCausedStack(list, action, QAction::Trigger, false);
      }
   }
}

void QMenuPrivate::_q_actionHovered()
{
   Q_Q(QMenu);
   if (QAction *action = qobject_cast<QAction *>(q->sender())) {
      emit q->hovered(action);
   }
}

void QMenuPrivate::_q_platformMenuAboutToShow()
{
   Q_Q(QMenu);

#ifdef Q_OS_DARWIN
   if (platformMenu)

      for (QAction *action : q->actions()) {

         if (QWidget *widget = widgetItems.value(action)) {
            if (widget->parent() == q) {
               QPlatformMenuItem *menuItem = platformMenu->menuItemForTag(reinterpret_cast<quintptr>(action));
               moveWidgetToPlatformItem(widget, menuItem);
               platformMenu->syncMenuItem(menuItem);
            }
         }
      }
#endif

   emit q->aboutToShow();
}
bool QMenuPrivate::hasMouseMoved(const QPoint &globalPos)
{
   //determines if the mouse has moved (ie its initial position has
   //changed by more than QApplication::startDragDistance()
   //or if there were at least 6 mouse motions)
   return motions > 6 ||
      QApplication::startDragDistance() < (mousePopupPos - globalPos).manhattanLength();
}


/*!
    Initialize \a option with the values from this menu and information from \a action. This method
    is useful for subclasses when they need a QStyleOptionMenuItem, but don't want
    to fill in all the information themselves.

    \sa QStyleOption::initFrom() QMenuBar::initStyleOption()
*/
void QMenu::initStyleOption(QStyleOptionMenuItem *option, const QAction *action) const
{
   if (!option || !action) {
      return;
   }

   Q_D(const QMenu);
   option->initFrom(this);
   option->palette = palette();
   option->state = QStyle::State_None;

   if (window()->isActiveWindow()) {
      option->state |= QStyle::State_Active;
   }
   if (isEnabled() && action->isEnabled()
      && (!action->menu() || action->menu()->isEnabled())) {
      option->state |= QStyle::State_Enabled;
   } else {
      option->palette.setCurrentColorGroup(QPalette::Disabled);
   }

   option->font = action->font().resolve(font());
   option->fontMetrics = QFontMetrics(option->font);

   if (d->currentAction && d->currentAction == action && !d->currentAction->isSeparator()) {
      option->state |= QStyle::State_Selected
         | (d->mouseDown ? QStyle::State_Sunken : QStyle::State_None);
   }

   option->menuHasCheckableItems = d->hasCheckableItems;
   if (!action->isCheckable()) {
      option->checkType = QStyleOptionMenuItem::NotCheckable;
   } else {
      option->checkType = (action->actionGroup() && action->actionGroup()->isExclusive())
         ? QStyleOptionMenuItem::Exclusive : QStyleOptionMenuItem::NonExclusive;
      option->checked = action->isChecked();
   }
   if (action->menu()) {
      option->menuItemType = QStyleOptionMenuItem::SubMenu;
   } else if (action->isSeparator()) {
      option->menuItemType = QStyleOptionMenuItem::Separator;
   } else if (d->defaultAction == action) {
      option->menuItemType = QStyleOptionMenuItem::DefaultItem;
   } else {
      option->menuItemType = QStyleOptionMenuItem::Normal;
   }
   if (action->isIconVisibleInMenu()) {
      option->icon = action->icon();
   }

   QString textAndAccel = action->text();

#ifndef QT_NO_SHORTCUT
   if (textAndAccel.indexOf(QLatin1Char('\t')) == -1) {
      QKeySequence seq = action->shortcut();

      if (!seq.isEmpty()) {
         textAndAccel += QLatin1Char('\t') + seq.toString(QKeySequence::NativeText);
      }
   }

#endif
   option->text = textAndAccel;
   option->tabWidth = d->tabWidth;
   option->maxIconWidth = d->maxIconWidth;
   option->menuRect = rect();
}

QMenu::QMenu(QWidget *parent)
   : QWidget(*new QMenuPrivate, parent, Qt::Popup)
{
   Q_D(QMenu);
   d->init();
}

QMenu::QMenu(const QString &title, QWidget *parent)
   : QWidget(*new QMenuPrivate, parent, Qt::Popup)
{
   Q_D(QMenu);
   d->init();
   d->menuAction->setText(title);
}

/*! \internal
 */
QMenu::QMenu(QMenuPrivate &dd, QWidget *parent)
   : QWidget(dd, parent, Qt::Popup)
{
   Q_D(QMenu);
   d->init();
}

/*!
    Destroys the menu.
*/
QMenu::~QMenu()
{
   Q_D(QMenu);
   if (!d->widgetItems.isEmpty()) {  // avoid detach on shared null hash
      QHash<QAction *, QWidget *>::iterator it = d->widgetItems.begin();
      for (; it != d->widgetItems.end(); ++it) {
         if (QWidget *widget = it.value()) {
            QWidgetAction *action = static_cast<QWidgetAction *>(it.key());
            action->releaseWidget(widget);
            *it = 0;
         }
      }
   }

   if (d->eventLoop) {
      d->eventLoop->exit();
   }
   hideTearOffMenu();
}


QAction *QMenu::addAction(const QString &text)
{
   QAction *ret = new QAction(text, this);
   addAction(ret);
   return ret;
}


QAction *QMenu::addAction(const QIcon &icon, const QString &text)
{
   QAction *ret = new QAction(icon, text, this);
   addAction(ret);
   return ret;
}


QAction *QMenu::addAction(const QString &text, const QObject *receiver, const QString &member, const QKeySequence &shortcut)
{
   QAction *action = new QAction(text, this);

#ifdef QT_NO_SHORTCUT
   Q_UNUSED(shortcut);
#else
   action->setShortcut(shortcut);
#endif

   QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
   addAction(action);
   return action;
}

QAction *QMenu::addAction(const QIcon &icon, const QString &text, const QObject *receiver,
   const QString &member, const QKeySequence &shortcut)
{
   QAction *action = new QAction(icon, text, this);

#ifdef QT_NO_SHORTCUT
   Q_UNUSED(shortcut);
#else
   action->setShortcut(shortcut);
#endif

   QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
   addAction(action);
   return action;
}

QAction *QMenu::addMenu(QMenu *menu)
{
   QAction *action = menu->menuAction();
   addAction(action);
   return action;
}

QMenu *QMenu::addMenu(const QString &title)
{
   QMenu *menu = new QMenu(title, this);
   addAction(menu->menuAction());
   return menu;
}

QMenu *QMenu::addMenu(const QIcon &icon, const QString &title)
{
   QMenu *menu = new QMenu(title, this);
   menu->setIcon(icon);
   addAction(menu->menuAction());
   return menu;
}

QAction *QMenu::addSeparator()
{
   QAction *action = new QAction(this);
   action->setSeparator(true);
   addAction(action);
   return action;
}
QAction *QMenu::addSection(const QString &text)
{
   QAction *action = new QAction(text, this);
   action->setSeparator(true);
   addAction(action);
   return action;
}
QAction *QMenu::addSection(const QIcon &icon, const QString &text)
{
   QAction *action = new QAction(icon, text, this);
   action->setSeparator(true);
   addAction(action);
   return action;
}

/*!
    This convenience function inserts \a menu before action \a before
    and returns the menus menuAction().

    \sa QWidget::insertAction(), addMenu()
*/
QAction *QMenu::insertMenu(QAction *before, QMenu *menu)
{
   QAction *action = menu->menuAction();
   insertAction(before, action);
   return action;
}

/*!
    This convenience function creates a new separator action, i.e. an
    action with QAction::isSeparator() returning true. The function inserts
    the newly created action into this menu's list of actions before
    action \a before and returns it.

    \sa QWidget::insertAction(), addSeparator()
*/
QAction *QMenu::insertSeparator(QAction *before)
{
   QAction *action = new QAction(this);
   action->setSeparator(true);
   insertAction(before, action);
   return action;
}
QAction *QMenu::insertSection(QAction *before, const QString &text)
{
   QAction *action = new QAction(text, this);
   action->setSeparator(true);
   insertAction(before, action);
   return action;
}
QAction *QMenu::insertSection(QAction *before, const QIcon &icon, const QString &text)
{
   QAction *action = new QAction(icon, text, this);
   action->setSeparator(true);
   insertAction(before, action);
   return action;
}

/*!
  This sets the default action to \a act. The default action may have
  a visual cue, depending on the current QStyle. A default action
  usually indicates what will happen by default when a drop occurs.

  \sa defaultAction()
*/
void QMenu::setDefaultAction(QAction *act)
{
   d_func()->defaultAction = act;
}

QAction *QMenu::defaultAction() const
{
   return d_func()->defaultAction;
}

void QMenu::setTearOffEnabled(bool b)
{
   Q_D(QMenu);
   if (d->tearoff == b) {
      return;
   }
   if (!b) {
      hideTearOffMenu();
   }
   d->tearoff = b;

   d->itemsDirty = true;
   if (isVisible()) {
      resize(sizeHint());
   }
}

bool QMenu::isTearOffEnabled() const
{
   return d_func()->tearoff;
}

bool QMenu::isTearOffMenuVisible() const
{
   if (d_func()->tornPopup) {
      return d_func()->tornPopup->isVisible();
   }
   return false;
}

/*!
   This function will forcibly hide the torn off menu making it
   disappear from the users desktop.

   \sa isTearOffMenuVisible() isTearOffEnabled()
*/
void QMenu::hideTearOffMenu()
{
   if (QWidget *w = d_func()->tornPopup) {
      w->close();
   }
}


/*!
  Sets the currently highlighted action to \a act.
*/
void QMenu::setActiveAction(QAction *act)
{
   Q_D(QMenu);
   d->setCurrentAction(act, 0);
   if (d->scroll) {
      d->scrollMenu(act, QMenuPrivate::QMenuScroller::ScrollCenter);
   }
}

QAction *QMenu::activeAction() const
{
   return d_func()->currentAction;
}


bool QMenu::isEmpty() const
{
   bool ret = true;
   for (int i = 0; ret && i < actions().count(); ++i) {
      const QAction *action = actions().at(i);
      if (!action->isSeparator() && action->isVisible()) {
         ret = false;
      }
   }
   return ret;
}

void QMenu::clear()
{
   QList<QAction *> acts = actions();

   for (int i = 0; i < acts.size(); i++) {
      removeAction(acts[i]);
      if (acts[i]->parent() == this && acts[i]->d_func()->widgets.isEmpty()) {
         delete acts[i];
      }
   }
}

/*!
  If a menu does not fit on the screen it lays itself out so that it
  does fit. It is style dependent what layout means (for example, on
  Windows it will use multiple columns).

  This functions returns the number of columns necessary.
*/
int QMenu::columnCount() const
{
   return d_func()->ncols;
}

/*!
  Returns the item at \a pt; returns 0 if there is no item there.
*/
QAction *QMenu::actionAt(const QPoint &pt) const
{
   if (QAction *ret = d_func()->actionAt(pt)) {
      return ret;
   }
   return 0;
}

/*!
  Returns the geometry of action \a act.
*/
QRect QMenu::actionGeometry(QAction *act) const
{
   return d_func()->actionRect(act);
}

/*!
    \reimp
*/
QSize QMenu::sizeHint() const
{
   Q_D(const QMenu);
   d->updateActionRects();

   QSize s;
   for (int i = 0; i < d->actionRects.count(); ++i) {
      const QRect &rect = d->actionRects.at(i);
      if (rect.isNull()) {
         continue;
      }
      if (rect.bottom() >= s.height()) {
         s.setHeight(rect.y() + rect.height());
      }
      if (rect.right() >= s.width()) {
         s.setWidth(rect.x() + rect.width());
      }
   }
   // Note that the action rects calculated above already include
   // the top and left margins, so we only need to add margins for
   // the bottom and right.
   QStyleOption opt(0);
   opt.init(this);
   const int fw = style()->pixelMetric(QStyle::PM_MenuPanelWidth, &opt, this);
   s.rwidth() += style()->pixelMetric(QStyle::PM_MenuHMargin, &opt, this) + fw + d->rightmargin;
   s.rheight() += style()->pixelMetric(QStyle::PM_MenuVMargin, &opt, this) + fw + d->bottommargin;

   return style()->sizeFromContents(QStyle::CT_Menu, &opt,
         s.expandedTo(QApplication::globalStrut()), this);
}

/*!
    Displays the menu so that the action \a atAction will be at the
    specified \e global position \a p. To translate a widget's local
    coordinates into global coordinates, use QWidget::mapToGlobal().

    When positioning a menu with exec() or popup(), bear in mind that
    you cannot rely on the menu's current size(). For performance
    reasons, the menu adapts its size only when necessary, so in many
    cases, the size before and after the show is different. Instead,
    use sizeHint() which calculates the proper size depending on the
    menu's current contents.

    \sa QWidget::mapToGlobal(), exec()
*/
void QMenu::popup(const QPoint &p, QAction *atAction)
{
   Q_D(QMenu);

   if (d->scroll) { // reset scroll state from last popup
      if (d->scroll->scrollOffset) {
         d->itemsDirty = 1;   // sizeHint will be incorrect if there is previous scroll
      }
      d->scroll->scrollOffset = 0;
      d->scroll->scrollFlags = QMenuPrivate::QMenuScroller::ScrollNone;
   }

   d->tearoffHighlighted = 0;
   d->motions = 0;
   d->doChildEffects = true;
   d->updateLayoutDirection();

#ifndef QT_NO_MENUBAR
   // if this menu is part of a chain attached to a QMenuBar, set the
   // _NET_WM_WINDOW_TYPE_DROPDOWN_MENU X11 window type
   setAttribute(Qt::WA_X11NetWmWindowTypeDropDownMenu, qobject_cast<QMenuBar *>(d->topCausedWidget()) != 0);
#endif

   ensurePolished(); // Get the right font
   emit aboutToShow();
   const bool actionListChanged = d->itemsDirty;
   d->updateActionRects();
   QPoint pos;
   QPushButton *causedButton = qobject_cast<QPushButton *>(d->causedPopup.widget);
   if (actionListChanged && causedButton) {
      pos = QPushButtonPrivate::get(causedButton)->adjustedMenuPosition();
   } else {
      pos = p;
   }

   QSize size = sizeHint();
   QRect screen;

#ifndef QT_NO_GRAPHICSVIEW
   bool isEmbedded = !bypassGraphicsProxyWidget(this) && d->nearestGraphicsProxyWidget(this);

   if (isEmbedded) {
      screen = d->popupGeometry(this);
   } else
#endif
      screen = d->popupGeometry(QApplication::desktop()->screenNumber(p));

   const int desktopFrame = style()->pixelMetric(QStyle::PM_MenuDesktopFrameWidth, 0, this);
   bool adjustToDesktop = !window()->testAttribute(Qt::WA_DontShowOnScreen);

   // if the screens have very different geometries and the menu is too big, we have to recalculate
   if (size.height() > screen.height() || size.width() > screen.width()) {
      size = d->adjustMenuSizeForScreen(screen);
      adjustToDesktop = true;
   }

   // Layout is not right, we might be able to save horizontal space
   if (d->ncols > 1 && size.height() < screen.height()) {
      size = d->adjustMenuSizeForScreen(screen);
      adjustToDesktop = true;
   }


#ifdef QT_KEYPAD_NAVIGATION
   if (!atAction && QApplication::keypadNavigationEnabled()) {
      // Try to have one item activated
      if (d->defaultAction && d->defaultAction->isEnabled()) {
         atAction = d->defaultAction;
         // TODO: This works for first level menus, not yet sub menus
      } else {
         for (QAction *action : d->actions) {
            if (action->isEnabled()) {
               atAction = action;
               break;
            }
         }
      }
      d->currentAction = atAction;
   }
#endif
   if (d->ncols > 1) {
      pos.setY(screen.top() + desktopFrame);
   } else if (atAction) {
      for (int i = 0, above_height = 0; i < d->actions.count(); i++) {
         QAction *action = d->actions.at(i);
         if (action == atAction) {
            int newY = pos.y() - above_height;
            if (d->scroll && newY < desktopFrame) {
               d->scroll->scrollFlags = d->scroll->scrollFlags
                  | QMenuPrivate::QMenuScroller::ScrollUp;
               d->scroll->scrollOffset = newY;
               newY = desktopFrame;
            }
            pos.setY(newY);

            if (d->scroll && d->scroll->scrollFlags != QMenuPrivate::QMenuScroller::ScrollNone
               && !style()->styleHint(QStyle::SH_Menu_FillScreenWithScroll, 0, this)) {
               int below_height = above_height + d->scroll->scrollOffset;

               for (int i2 = i; i2 < d->actionRects.count(); i2++) {
                  below_height += d->actionRects.at(i2).height();
               }

               size.setHeight(below_height);
            }
            break;
         } else {
            above_height += d->actionRects.at(i).height();
         }
      }
   }

   QPoint mouse = QCursor::pos();
   d->mousePopupPos = mouse;
   const bool snapToMouse = !d->causedPopup.widget && (QRect(p.x() - 3, p.y() - 3, 6, 6).contains(mouse));

   const QSize menuSize(sizeHint());
   if (adjustToDesktop) {
      // handle popup falling "off screen"
      if (isRightToLeft()) {
         if (snapToMouse) { // position flowing left from the mouse
            pos.setX(mouse.x() - size.width());
         }

#ifndef QT_NO_MENUBAR
         // if in a menubar, it should be right-aligned
         if (qobject_cast<QMenuBar *>(d->causedPopup.widget) || qobject_cast<QMenu *>(d->causedPopup.widget)) {
            pos.rx() -= size.width();
         }
#endif

         if (pos.x() < screen.left() + desktopFrame) {
            pos.setX(qMax(p.x(), screen.left() + desktopFrame));
         }
         if (pos.x() + size.width() - 1 > screen.right() - desktopFrame) {
            pos.setX(qMax(p.x() - size.width(), screen.right() - desktopFrame - size.width() + 1));
         }
      } else {
         if (pos.x() + size.width() - 1 > screen.right() - desktopFrame) {
            pos.setX(screen.right() - desktopFrame - size.width() + 1);
         }
         if (pos.x() < screen.left() + desktopFrame) {
            pos.setX(screen.left() + desktopFrame);
         }
      }
      if (pos.y() + size.height() - 1 > screen.bottom() - desktopFrame) {
         if (snapToMouse) {
            pos.setY(qMin(mouse.y() - (size.height() + desktopFrame), screen.bottom() - desktopFrame - size.height() + 1));
         } else {
            pos.setY(qMax(p.y() - (size.height() + desktopFrame), screen.bottom() - desktopFrame - size.height() + 1));
         }
      } else if (pos.y() < screen.top() + desktopFrame) {
         pos.setY(screen.top() + desktopFrame);
      }

      if (pos.y() < screen.top() + desktopFrame) {
         pos.setY(screen.top() + desktopFrame);
      }
      if (pos.y() + menuSize.height() - 1 > screen.bottom() - desktopFrame) {
         if (d->scroll) {
            d->scroll->scrollFlags |= uint(QMenuPrivate::QMenuScroller::ScrollDown);
            int y = qMax(screen.y(), pos.y());
            size.setHeight(screen.bottom() - (desktopFrame * 2) - y);
         } else {
            // Too big for screen, bias to see bottom of menu (for some reason)
            pos.setY(screen.bottom() - size.height() + 1);
         }
      }
   }
   const int subMenuOffset = style()->pixelMetric(QStyle::PM_SubMenuOverlap, 0, this);
   QMenu *caused = qobject_cast<QMenu *>(d_func()->causedPopup.widget);

   if (caused && caused->geometry().width() + menuSize.width() + subMenuOffset < screen.width()) {
      QRect parentActionRect(caused->d_func()->actionRect(caused->d_func()->currentAction));
      const QPoint actionTopLeft = caused->mapToGlobal(parentActionRect.topLeft());
      parentActionRect.moveTopLeft(actionTopLeft);
      if (isRightToLeft()) {
         if ((pos.x() + menuSize.width() > parentActionRect.left() - subMenuOffset)
            && (pos.x() < parentActionRect.right())) {
            pos.rx() = parentActionRect.left() - menuSize.width();
            if (pos.x() < screen.x()) {
               pos.rx() = parentActionRect.right();
            }
            if (pos.x() + menuSize.width() > screen.x() + screen.width()) {
               pos.rx() = screen.x();
            }
         }

      } else {
         if ((pos.x() < parentActionRect.right() + subMenuOffset)
            && (pos.x() + menuSize.width() > parentActionRect.left())) {
            pos.rx() = parentActionRect.right();
            if (pos.x() + menuSize.width() > screen.x() + screen.width()) {
               pos.rx() = parentActionRect.left() - menuSize.width();
            }

            if (pos.x() < screen.x()) {
               pos.rx() = screen.x() + screen.width() - menuSize.width();
            }
         }
      }
   }
   setGeometry(QRect(pos, size));
#ifndef QT_NO_EFFECTS

   int hGuess = isRightToLeft() ? QEffects::LeftScroll : QEffects::RightScroll;
   int vGuess = QEffects::DownScroll;

   if (isRightToLeft()) {
      if ((snapToMouse && (pos.x() + size.width() / 2 > mouse.x())) ||
         (qobject_cast<QMenu *>(d->causedPopup.widget) && pos.x() + size.width() / 2 > d->causedPopup.widget->x())) {
         hGuess = QEffects::RightScroll;
      }
   } else {
      if ((snapToMouse && (pos.x() + size.width() / 2 < mouse.x())) ||
         (qobject_cast<QMenu *>(d->causedPopup.widget) && pos.x() + size.width() / 2 < d->causedPopup.widget->x())) {
         hGuess = QEffects::LeftScroll;
      }
   }

#ifndef QT_NO_MENUBAR
   if ((snapToMouse && (pos.y() + size.height() / 2 < mouse.y())) ||
      (qobject_cast<QMenuBar *>(d->causedPopup.widget) &&
         pos.y() + size.width() / 2 < d->causedPopup.widget->mapToGlobal(d->causedPopup.widget->pos()).y())) {
      vGuess = QEffects::UpScroll;
   }
#endif

   if (QApplication::isEffectEnabled(Qt::UI_AnimateMenu)) {
      bool doChildEffects = true;
#ifndef QT_NO_MENUBAR
      if (QMenuBar *mb = qobject_cast<QMenuBar *>(d->causedPopup.widget)) {
         doChildEffects = mb->d_func()->doChildEffects;
         mb->d_func()->doChildEffects = false;
      } else
#endif
         if (QMenu *m = qobject_cast<QMenu *>(d->causedPopup.widget)) {
            doChildEffects = m->d_func()->doChildEffects;
            m->d_func()->doChildEffects = false;
         }

      if (doChildEffects) {
         if (QApplication::isEffectEnabled(Qt::UI_FadeMenu)) {
            qFadeEffect(this);
         } else if (d->causedPopup.widget) {
            qScrollEffect(this, qobject_cast<QMenu *>(d->causedPopup.widget) ? hGuess : vGuess);
         } else {
            qScrollEffect(this, hGuess | vGuess);
         }
      } else {
         // kill any running effect
         qFadeEffect(0);
         qScrollEffect(0);

         show();
      }
   } else
#endif
   {
      show();
   }

#ifndef QT_NO_ACCESSIBILITY
   QAccessibleEvent event(this, QAccessible::PopupMenuStart);
   QAccessible::updateAccessibility(&event);
#endif
}

QAction *QMenu::exec()
{
   return exec(pos());
}


QAction *QMenu::exec(const QPoint &p, QAction *action)
{
   Q_D(QMenu);
   ensurePolished();
   createWinId();

   QEventLoop eventLoop;
   d->eventLoop = &eventLoop;
   popup(p, action);

   QPointer<QObject> guard = this;
   (void) eventLoop.exec();

   if (guard.isNull()) {
      return 0;
   }

   action = d->syncAction;
   d->syncAction = 0;
   d->eventLoop = 0;
   return action;
}

QAction *QMenu::exec(const QList<QAction *> &actions, const QPoint &pos, QAction *at, QWidget *parent)
{
   QMenu menu(parent);
   menu.addActions(actions);
   return menu.exec(pos, at);
}

/*!
  \reimp
*/
void QMenu::hideEvent(QHideEvent *)
{
   Q_D(QMenu);
   emit aboutToHide();
   if (d->eventLoop) {
      d->eventLoop->exit();
   }

   d->setCurrentAction(0);

#ifndef QT_NO_ACCESSIBILITY
   QAccessibleEvent event(this, QAccessible::PopupMenuEnd);
   QAccessible::updateAccessibility(&event);
#endif

#ifndef QT_NO_MENUBAR
   if (QMenuBar *mb = qobject_cast<QMenuBar *>(d->causedPopup.widget)) {
      mb->d_func()->setCurrentAction(0);
   }
#endif
   d->mouseDown = 0;
   d->hasHadMouse = false;

   if (d->activeMenu) {
      d->hideMenu(d->activeMenu);
   }

   d->causedPopup.widget = 0;
   d->causedPopup.action = 0;
   if (d->scroll) {
      d->scroll->scrollTimer.stop();   //make sure the timer stops
   }
}

/*!
  \reimp
*/
void QMenu::paintEvent(QPaintEvent *e)
{
   Q_D(QMenu);
   d->updateActionRects();
   QPainter p(this);
   QRegion emptyArea = QRegion(rect());

   QStyleOptionMenuItem menuOpt;
   menuOpt.initFrom(this);
   menuOpt.state = QStyle::State_None;
   menuOpt.checkType = QStyleOptionMenuItem::NotCheckable;
   menuOpt.maxIconWidth = 0;
   menuOpt.tabWidth = 0;
   style()->drawPrimitive(QStyle::PE_PanelMenu, &menuOpt, &p, this);

   //draw the items that need updating..
   for (int i = 0; i < d->actions.count(); ++i) {
      QAction *action = d->actions.at(i);
      QRect adjustedActionRect = d->actionRects.at(i);
      if (!e->rect().intersects(adjustedActionRect)
         || d->widgetItems.value(action)) {
         continue;
      }
      //set the clip region to be extra safe (and adjust for the scrollers)
      QRegion adjustedActionReg(adjustedActionRect);
      emptyArea -= adjustedActionReg;
      p.setClipRegion(adjustedActionReg);

      QStyleOptionMenuItem opt;
      initStyleOption(&opt, action);
      opt.rect = adjustedActionRect;
      style()->drawControl(QStyle::CE_MenuItem, &opt, &p, this);
   }

   const int fw = style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this);
   //draw the scroller regions..
   if (d->scroll) {
      menuOpt.menuItemType = QStyleOptionMenuItem::Scroller;
      menuOpt.state |= QStyle::State_Enabled;
      if (d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp) {
         menuOpt.rect.setRect(fw, fw, width() - (fw * 2), d->scrollerHeight());
         emptyArea -= QRegion(menuOpt.rect);
         p.setClipRect(menuOpt.rect);
         style()->drawControl(QStyle::CE_MenuScroller, &menuOpt, &p, this);
      }
      if (d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollDown) {
         menuOpt.rect.setRect(fw, height() - d->scrollerHeight() - fw, width() - (fw * 2),
            d->scrollerHeight());
         emptyArea -= QRegion(menuOpt.rect);
         menuOpt.state |= QStyle::State_DownArrow;
         p.setClipRect(menuOpt.rect);
         style()->drawControl(QStyle::CE_MenuScroller, &menuOpt, &p, this);
      }
   }
   //paint the tear off..
   if (d->tearoff) {
      menuOpt.menuItemType = QStyleOptionMenuItem::TearOff;
      menuOpt.rect.setRect(fw, fw, width() - (fw * 2),
         style()->pixelMetric(QStyle::PM_MenuTearoffHeight, 0, this));
      if (d->scroll && d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp) {
         menuOpt.rect.translate(0, d->scrollerHeight());
      }

      emptyArea -= QRegion(menuOpt.rect);
      p.setClipRect(menuOpt.rect);
      menuOpt.state = QStyle::State_None;
      if (d->tearoffHighlighted) {
         menuOpt.state |= QStyle::State_Selected;
      }

      style()->drawControl(QStyle::CE_MenuTearoff, &menuOpt, &p, this);
   }

   //draw border
   if (fw) {
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
      frame.lineWidth = style()->pixelMetric(QStyle::PM_MenuPanelWidth);
      frame.midLineWidth = 0;
      style()->drawPrimitive(QStyle::PE_FrameMenu, &frame, &p, this);
   }

   //finally the rest of the space
   p.setClipRegion(emptyArea);
   menuOpt.state = QStyle::State_None;
   menuOpt.menuItemType = QStyleOptionMenuItem::EmptyArea;
   menuOpt.checkType = QStyleOptionMenuItem::NotCheckable;
   menuOpt.rect = rect();
   menuOpt.menuRect = rect();
   style()->drawControl(QStyle::CE_MenuEmptyArea, &menuOpt, &p, this);
}

#ifndef QT_NO_WHEELEVENT
/*!
  \reimp
*/
void QMenu::wheelEvent(QWheelEvent *e)
{
   Q_D(QMenu);
   if (d->scroll && rect().contains(e->pos()))
      d->scrollMenu(e->delta() > 0 ?
         QMenuPrivate::QMenuScroller::ScrollUp : QMenuPrivate::QMenuScroller::ScrollDown);
}
#endif

/*!
  \reimp
*/
void QMenu::mousePressEvent(QMouseEvent *e)
{
   Q_D(QMenu);

   if (d->aboutToHide || d->mouseEventTaken(e)) {
      return;
   }

   // Workaround for XCB on multiple screens which doesn't have offset. If the menu is open on one screen
   // and mouse clicks on second screen, e->pos() is QPoint(0,0) and the menu doesn't hide. This trick makes
   // possible to hide the menu when mouse clicks on another screen (e->screenPos() returns correct value).
   // Only when mouse clicks in QPoint(0,0) on second screen, the menu doesn't hide.
   if ((e->pos().isNull() && !e->screenPos().isNull()) || !rect().contains(e->pos())) {
      if (d->noReplayFor && QRect(d->noReplayFor->mapToGlobal(QPoint()), d->noReplayFor->size()).contains(e->globalPos())) {
         setAttribute(Qt::WA_NoMouseReplay);
      }

      if (d->eventLoop) { // synchronous operation
         d->syncAction = 0;
      }

      d->hideUpToMenuBar();
      return;
   }

   d->mouseDown = this;

   QAction *action = d->actionAt(e->pos());
   d->setCurrentAction(action, 20);
   update();
}

/*!
  \reimp
*/
void QMenu::mouseReleaseEvent(QMouseEvent *e)
{
   Q_D(QMenu);
   if (d->aboutToHide || d->mouseEventTaken(e)) {
      return;
   }
   if (d->mouseDown != this) {
      d->mouseDown = 0;
      return;
   }

   d->mouseDown = 0;
   d->setSyncAction();
   QAction *action = d->actionAt(e->pos());

   if (action && action == d->currentAction) {

      if (!action->menu()) {

#if defined(Q_OS_WIN)
         // On Windows only context menus can be activated with the right button
         if (e->button() == Qt::LeftButton || d->topCausedWidget() == 0)
#endif
            d->activateAction(action, QAction::Trigger);
      }
   } else if ((!action || action->isEnabled()) && d->hasMouseMoved(e->globalPos())) {
      d->hideUpToMenuBar();
   }
}

/*!
  \reimp
*/
void QMenu::changeEvent(QEvent *e)
{
   Q_D(QMenu);
   if (e->type() == QEvent::StyleChange || e->type() == QEvent::FontChange ||
      e->type() == QEvent::LayoutDirectionChange) {
      d->itemsDirty = 1;
      setMouseTracking(style()->styleHint(QStyle::SH_Menu_MouseTracking, 0, this));
      if (isVisible()) {
         resize(sizeHint());
      }
      if (!style()->styleHint(QStyle::SH_Menu_Scrollable, 0, this)) {
         delete d->scroll;
         d->scroll = 0;
      } else if (!d->scroll) {
         d->scroll = new QMenuPrivate::QMenuScroller;
         d->scroll->scrollFlags = QMenuPrivate::QMenuScroller::ScrollNone;
      }
   } else if (e->type() == QEvent::EnabledChange) {
      if (d->tornPopup) { // torn-off menu
         d->tornPopup->setEnabled(isEnabled());
      }

      d->menuAction->setEnabled(isEnabled());
      if (!d->platformMenu.isNull()) {
         d->platformMenu->setEnabled(isEnabled());
      }
   }

   QWidget::changeEvent(e);
}


/*!
  \reimp
*/
bool QMenu::event(QEvent *e)
{
   Q_D(QMenu);
   switch (e->type()) {
      case QEvent::Polish:
         d->updateLayoutDirection();
         break;
      case QEvent::ShortcutOverride: {
         QKeyEvent *kev = static_cast<QKeyEvent *>(e);
         if (kev->key() == Qt::Key_Up || kev->key() == Qt::Key_Down
            || kev->key() == Qt::Key_Left || kev->key() == Qt::Key_Right
            || kev->key() == Qt::Key_Enter || kev->key() == Qt::Key_Return
            || kev->matches(QKeySequence::Cancel)) {

            e->accept();
            return true;
         }
      }
      break;
      case QEvent::KeyPress: {
         QKeyEvent *ke = (QKeyEvent *)e;
         if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab) {
            keyPressEvent(ke);
            return true;
         }
      }
      break;

      case QEvent::MouseButtonPress:
      case QEvent::ContextMenu: {
         bool canPopup = true;
         if (e->type() == QEvent::MouseButtonPress) {
            canPopup = (static_cast<QMouseEvent *>(e)->button() == Qt::LeftButton);
         }
         if (canPopup && d->delayState.timer.isActive()) {
            d->delayState.stop();
            internalDelayedPopup();
         }
      }
      break;

      case QEvent::Resize: {
         QStyleHintReturnMask menuMask;
         QStyleOption option;
         option.initFrom(this);
         if (style()->styleHint(QStyle::SH_Menu_Mask, &option, this, &menuMask)) {
            setMask(menuMask.region);
         }
         d->itemsDirty = 1;
         d->updateActionRects();
         break;
      }

      case QEvent::Show:
         d->mouseDown = 0;
         d->updateActionRects();
         d->sloppyState.reset();
         if (d->currentAction) {
            d->popupAction(d->currentAction, 0, false);
         }
         break;

#ifndef QT_NO_TOOLTIP
      case QEvent::ToolTip:
         if (d->toolTipsVisible) {
            const QHelpEvent *ev = static_cast<const QHelpEvent *>(e);
            if (const QAction *action = actionAt(ev->pos())) {
               const QString toolTip = action->d_func()->tooltip;
               if (!toolTip.isEmpty()) {
                  QToolTip::showText(ev->globalPos(), toolTip, this);
               }
               return true;
            }
         }
         break;
#endif

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

      default:
         break;
   }
   return QWidget::event(e);
}

/*!
    \reimp
*/
bool QMenu::focusNextPrevChild(bool next)
{
   setFocus();
   QKeyEvent ev(QEvent::KeyPress, next ? Qt::Key_Tab : Qt::Key_Backtab, Qt::NoModifier);
   keyPressEvent(&ev);
   return true;
}

/*!
  \reimp
*/
void QMenu::keyPressEvent(QKeyEvent *e)
{
   Q_D(QMenu);
   d->updateActionRects();
   int key = e->key();
   if (isRightToLeft()) {  // in reverse mode open/close key for submenues are reversed
      if (key == Qt::Key_Left) {
         key = Qt::Key_Right;
      } else if (key == Qt::Key_Right) {
         key = Qt::Key_Left;
      }
   }
#ifndef Q_OS_DARWIN
   if (key == Qt::Key_Tab) { //means down
      key = Qt::Key_Down;
   }
   if (key == Qt::Key_Backtab) { //means up
      key = Qt::Key_Up;
   }
#endif

   bool key_consumed = false;
   switch (key) {
      case Qt::Key_Home:
         key_consumed = true;
         if (d->scroll) {
            d->scrollMenu(QMenuPrivate::QMenuScroller::ScrollTop, true);
         }
         break;
      case Qt::Key_End:
         key_consumed = true;
         if (d->scroll) {
            d->scrollMenu(QMenuPrivate::QMenuScroller::ScrollBottom, true);
         }
         break;
      case Qt::Key_PageUp:
         key_consumed = true;
         if (d->currentAction && d->scroll) {
            if (d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp) {
               d->scrollMenu(QMenuPrivate::QMenuScroller::ScrollUp, true, true);
            } else {
               d->scrollMenu(QMenuPrivate::QMenuScroller::ScrollTop, true);
            }
         }
         break;

      case Qt::Key_PageDown:
         key_consumed = true;
         if (d->currentAction && d->scroll) {
            if (d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollDown) {
               d->scrollMenu(QMenuPrivate::QMenuScroller::ScrollDown, true, true);
            } else {
               d->scrollMenu(QMenuPrivate::QMenuScroller::ScrollBottom, true);
            }
         }
         break;
      case Qt::Key_Up:
      case Qt::Key_Down: {
         key_consumed = true;
         QAction *nextAction = 0;
         QMenuPrivate::QMenuScroller::ScrollLocation scroll_loc = QMenuPrivate::QMenuScroller::ScrollStay;
         if (!d->currentAction) {
            if (key == Qt::Key_Down) {
               for (int i = 0; i < d->actions.count(); ++i) {
                  QAction *act = d->actions.at(i);
                  if (d->actionRects.at(i).isNull()) {
                     continue;
                  }
                  if (!act->isSeparator() &&
                     (style()->styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, 0, this)
                        || act->isEnabled())) {
                     nextAction = act;
                     break;
                  }
               }
            } else {
               for (int i = d->actions.count() - 1; i >= 0; --i) {
                  QAction *act = d->actions.at(i);
                  if (d->actionRects.at(i).isNull()) {
                     continue;
                  }
                  if (!act->isSeparator() &&
                     (style()->styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, 0, this)
                        || act->isEnabled())) {
                     nextAction = act;
                     break;
                  }
               }
            }

         } else {
            for (int i = 0, y = 0; !nextAction && i < d->actions.count(); i++) {
               QAction *act = d->actions.at(i);
               if (act == d->currentAction) {
                  if (key == Qt::Key_Up) {
                     for (int next_i = i - 1; true; next_i--) {
                        if (next_i == -1) {
                           if (!style()->styleHint(QStyle::SH_Menu_SelectionWrap, 0, this)) {
                              break;
                           }
                           if (d->scroll) {
                              scroll_loc = QMenuPrivate::QMenuScroller::ScrollBottom;
                           }
                           next_i = d->actionRects.count() - 1;
                        }
                        QAction *next = d->actions.at(next_i);
                        if (next == d->currentAction) {
                           break;
                        }
                        if (d->actionRects.at(next_i).isNull()) {
                           continue;
                        }
                        if (next->isSeparator() ||
                           (!next->isEnabled() &&
                              !style()->styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, 0, this))) {
                           continue;
                        }
                        nextAction = next;
                        if (d->scroll && (d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp)) {
                           int topVisible = d->scrollerHeight();
                           if (d->tearoff) {
                              topVisible += style()->pixelMetric(QStyle::PM_MenuTearoffHeight, 0, this);
                           }
                           if (((y + d->scroll->scrollOffset) - topVisible) <= d->actionRects.at(next_i).height()) {
                              scroll_loc = QMenuPrivate::QMenuScroller::ScrollTop;
                           }
                        }
                        break;
                     }
                     if (!nextAction && d->tearoff) {
                        d->tearoffHighlighted = 1;
                     }
                  } else {
                     y += d->actionRects.at(i).height();
                     for (int next_i = i + 1; true; next_i++) {
                        if (next_i == d->actionRects.count()) {
                           if (!style()->styleHint(QStyle::SH_Menu_SelectionWrap, 0, this)) {
                              break;
                           }
                           if (d->scroll) {
                              scroll_loc = QMenuPrivate::QMenuScroller::ScrollTop;
                           }
                           next_i = 0;
                        }
                        QAction *next = d->actions.at(next_i);
                        if (next == d->currentAction) {
                           break;
                        }
                        if (d->actionRects.at(next_i).isNull()) {
                           continue;
                        }
                        if (next->isSeparator() ||
                           (!next->isEnabled() &&
                              !style()->styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, 0, this))) {
                           continue;
                        }
                        nextAction = next;
                        if (d->scroll && (d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollDown)) {
                           int bottomVisible = height() - d->scrollerHeight();
                           if (d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp) {
                              bottomVisible -= d->scrollerHeight();
                           }
                           if (d->tearoff) {
                              bottomVisible -= style()->pixelMetric(QStyle::PM_MenuTearoffHeight, 0, this);
                           }
                           if ((y + d->scroll->scrollOffset + d->actionRects.at(next_i).height()) > bottomVisible) {
                              scroll_loc = QMenuPrivate::QMenuScroller::ScrollBottom;
                           }
                        }
                        break;
                     }
                  }
                  break;
               }
               y += d->actionRects.at(i).height();
            }
         }
         if (nextAction) {
            if (d->scroll && scroll_loc != QMenuPrivate::QMenuScroller::ScrollStay) {
               d->scroll->scrollTimer.stop();
               d->scrollMenu(nextAction, scroll_loc);
            }
            d->setCurrentAction(nextAction, /*popup*/ -1, QMenuPrivate::SelectedFromKeyboard);
         }
         break;
      }

      case Qt::Key_Right:
         if (d->currentAction && d->currentAction->isEnabled() && d->currentAction->menu()) {
            d->popupAction(d->currentAction, 0, true);
            key_consumed = true;
            break;
         }
      //FALL THROUGH
      case Qt::Key_Left: {
         if (d->currentAction && !d->scroll) {
            QAction *nextAction = 0;
            if (key == Qt::Key_Left) {
               QRect actionR = d->actionRect(d->currentAction);
               for (int x = actionR.left() - 1; !nextAction && x >= 0; x--) {
                  nextAction = d->actionAt(QPoint(x, actionR.center().y()));
               }
            } else {
               QRect actionR = d->actionRect(d->currentAction);
               for (int x = actionR.right() + 1; !nextAction && x < width(); x++) {
                  nextAction = d->actionAt(QPoint(x, actionR.center().y()));
               }
            }
            if (nextAction) {
               d->setCurrentAction(nextAction, /*popup*/ -1, QMenuPrivate::SelectedFromKeyboard);
               key_consumed = true;
            }
         }
         if (!key_consumed && key == Qt::Key_Left && qobject_cast<QMenu *>(d->causedPopup.widget)) {
            QPointer<QWidget> caused = d->causedPopup.widget;
            d->hideMenu(this);
            if (caused) {
               caused->setFocus();
            }
            key_consumed = true;
         }
         break;
      }

      case Qt::Key_Alt:
         if (d->tornoff) {
            break;
         }

         key_consumed = true;
         if (style()->styleHint(QStyle::SH_MenuBar_AltKeyNavigation, 0, this)) {
            d->hideMenu(this);
#ifndef QT_NO_MENUBAR
            if (QMenuBar *mb = qobject_cast<QMenuBar *>(QApplication::focusWidget())) {
               mb->d_func()->setKeyboardMode(false);
            }
#endif
         }
         break;

      case Qt::Key_Space:
         if (!style()->styleHint(QStyle::SH_Menu_SpaceActivatesItem, 0, this)) {
            break;
         }
         // for motif, fall through

#ifdef QT_KEYPAD_NAVIGATION
      case Qt::Key_Select:
#endif

      case Qt::Key_Return:
      case Qt::Key_Enter: {
         if (!d->currentAction) {
            d->setFirstActionActive();
            key_consumed = true;
            break;
         }

         d->setSyncAction();

         if (d->currentAction->menu()) {
            d->popupAction(d->currentAction, 0, true);
         } else {
            d->activateAction(d->currentAction, QAction::Trigger);
         }
         key_consumed = true;
         break;
      }
#ifndef QT_NO_WHATSTHIS
      case Qt::Key_F1:
         if (!d->currentAction || d->currentAction->whatsThis().isEmpty()) {
            break;
         }
         QWhatsThis::enterWhatsThisMode();
         d->activateAction(d->currentAction, QAction::Trigger);
         return;
#endif
      default:
         key_consumed = false;
   }

   if (!key_consumed && (e->matches(QKeySequence::Cancel)
#ifdef QT_KEYPAD_NAVIGATION
         || e->key() == Qt::Key_Back
#endif
      )) {
      key_consumed = true;
      if (d->tornoff) {
         close();
         return;
      }
      {
         QPointer<QWidget> caused = d->causedPopup.widget;
         d->hideMenu(this); // hide after getting causedPopup
#ifndef QT_NO_MENUBAR
         if (QMenuBar *mb = qobject_cast<QMenuBar *>(caused)) {
            mb->d_func()->setCurrentAction(d->menuAction);
            mb->d_func()->setKeyboardMode(true);
         }
#endif
      }
   }

   if (! key_consumed) {                                // send to menu bar
      if ((!e->modifiers() || e->modifiers() == Qt::AltModifier || e->modifiers() == Qt::ShiftModifier) &&
         e->text().length() == 1) {
         bool activateAction = false;
         QAction *nextAction = 0;
         if (style()->styleHint(QStyle::SH_Menu_KeyboardSearch, 0, this) && !e->modifiers()) {
            int best_match_count = 0;
            d->searchBufferTimer.start(2000, this);
            d->searchBuffer += e->text();

            for (int i = 0; i < d->actions.size(); ++i) {
               int match_count = 0;

               if (d->actionRects.at(i).isNull()) {
                  continue;
               }

               QAction *act = d->actions.at(i);
               const QString act_text = act->text();

               for (int c = 0; c < d->searchBuffer.size(); ++c) {
                  if (act_text.indexOf(d->searchBuffer.at(c), 0, Qt::CaseInsensitive) != -1) {
                     ++match_count;
                  }
               }
               if (match_count > best_match_count) {
                  best_match_count = match_count;
                  nextAction = act;
               }
            }
         }

#ifndef QT_NO_SHORTCUT
         else {
            int clashCount = 0;

            QAction *first = 0, *currentSelected = 0, *firstAfterCurrent = 0;
            QChar c = e->text().at(0).toUpper()[0];

            for (int i = 0; i < d->actions.size(); ++i) {
               if (d->actionRects.at(i).isNull()) {
                  continue;
               }

               QAction *act = d->actions.at(i);
               QKeySequence sequence = QKeySequence::mnemonic(act->text());

               int key = sequence[0] & 0xffff;

               if (key == c.unicode()) {
                  clashCount++;
                  if (!first) {
                     first = act;
                  }
                  if (act == d->currentAction) {
                     currentSelected = act;
                  } else if (!firstAfterCurrent && currentSelected) {
                     firstAfterCurrent = act;
                  }
               }
            }
            if (clashCount == 1) {
               activateAction = true;
            }
            if (clashCount >= 1) {
               if (clashCount == 1 || !currentSelected || !firstAfterCurrent) {
                  nextAction = first;
               } else {
                  nextAction = firstAfterCurrent;
               }
            }
         }
#endif
         if (nextAction) {
            key_consumed = true;
            if (d->scroll) {
               d->scrollMenu(nextAction, QMenuPrivate::QMenuScroller::ScrollCenter, false);
            }
            d->setCurrentAction(nextAction, 0, QMenuPrivate::SelectedFromElsewhere, true);
            if (!nextAction->menu() && activateAction) {
               d->setSyncAction();
               d->activateAction(nextAction, QAction::Trigger);
            }
         }
      }
      if (!key_consumed) {
#ifndef QT_NO_MENUBAR
         if (QMenuBar *mb = qobject_cast<QMenuBar *>(d->topCausedWidget())) {
            QAction *oldAct = mb->d_func()->currentAction;
            QApplication::sendEvent(mb, e);
            if (mb->d_func()->currentAction != oldAct) {
               key_consumed = true;
            }
         }
#endif
      }

#ifdef Q_OS_WIN32
      if (key_consumed && (e->key() == Qt::Key_Control || e->key() == Qt::Key_Shift || e->key() == Qt::Key_Meta)) {
         QApplication::beep();
      }
#endif // Q_OS_WIN32
   }
   if (key_consumed) {
      e->accept();
   } else {
      e->ignore();
   }
}

/*!
  \reimp
*/
void QMenu::mouseMoveEvent(QMouseEvent *e)
{
   Q_D(QMenu);
   if (!isVisible() || d->aboutToHide || d->mouseEventTaken(e)) {
      return;
   }

   d->motions++;

   if (d->motions == 0) {
      return;
   }

   d->hasHadMouse = d->hasHadMouse || rect().contains(e->pos());

   QAction *action = d->actionAt(e->pos());

   if ((!action || action->isSeparator()) && !d->sloppyState.enabled()) {
      if (d->hasHadMouse
         || (!d->currentAction || !d->currentAction->menu() || !d->currentAction->menu()->isVisible())) {
         d->setCurrentAction(action);
      }

      return;
   }
   if (e->buttons()) {
      d->mouseDown = this;
   }

   if (d->activeMenu) {
      d->activeMenu->d_func()->setCurrentAction(0);
   }

   QMenuSloppyState::MouseEventResult sloppyEventResult = d->sloppyState.processMouseEvent(e->localPos(), action, d->currentAction);
   if (sloppyEventResult == QMenuSloppyState::EventShouldBePropagated) {
      d->setCurrentAction(action, d->mousePopupDelay);
   } else if (sloppyEventResult == QMenuSloppyState::EventDiscardsSloppyState) {
      d->sloppyState.reset();
      d->hideMenu(d->activeMenu);
   }
}

/*!
  \reimp
*/
void QMenu::enterEvent(QEvent *)
{
   Q_D(QMenu);
   d->hasReceievedEnter = true;
   d->sloppyState.enter();
   d->motions = -1; // force us to ignore the generate mouse move in mouseMoveEvent()
}

/*!
  \reimp
*/
void QMenu::leaveEvent(QEvent *)
{
   Q_D(QMenu);
   d->hasReceievedEnter = false;

   if (!d->activeMenu && d->currentAction) {
      setActiveAction(0);
   }
}

/*!
  \reimp
*/
void QMenu::timerEvent(QTimerEvent *e)
{
   Q_D(QMenu);

   if (d->scroll && d->scroll->scrollTimer.timerId() == e->timerId()) {
      d->scrollMenu((QMenuPrivate::QMenuScroller::ScrollDirection)d->scroll->scrollDirection);

      if (d->scroll->scrollFlags == QMenuPrivate::QMenuScroller::ScrollNone) {
         d->scroll->scrollTimer.stop();
      }

   } else if (d->delayState.timer.timerId() == e->timerId()) {
      if (d->currentAction && !d->currentAction->menu()) {
         return;
      }
      d->delayState.stop();
      d->sloppyState.stopTimer();
      internalDelayedPopup();
   } else if (d->sloppyState.isTimerId(e->timerId())) {
      d->sloppyState.timeout();
   } else if (d->searchBufferTimer.timerId() == e->timerId()) {
      d->searchBuffer.clear();
   }
}

static void copyActionToPlatformItem(const QAction *action, QPlatformMenuItem *item, QPlatformMenu *itemsMenu)
{
   item->setText(action->text());
   item->setIsSeparator(action->isSeparator());
   if (action->isIconVisibleInMenu()) {
      item->setIcon(action->icon());
      if (QWidget *w = action->parentWidget()) {
         QStyleOption opt;
         opt.init(w);
         item->setIconSize(w->style()->pixelMetric(QStyle::PM_SmallIconSize, &opt, w));
      } else {
         QStyleOption opt;
         item->setIconSize(qApp->style()->pixelMetric(QStyle::PM_SmallIconSize, &opt, 0));
      }
   } else {
      item->setIcon(QIcon());
   }

   item->setVisible(action->isVisible());
   item->setShortcut(action->shortcut());
   item->setCheckable(action->isCheckable());
   item->setChecked(action->isChecked());
   item->setFont(action->font());
   item->setRole((QPlatformMenuItem::MenuRole) action->menuRole());
   item->setEnabled(action->isEnabled());

   if (action->menu()) {
      if (!action->menu()->platformMenu()) {
         action->menu()->setPlatformMenu(itemsMenu->createSubMenu());
      }
      item->setMenu(action->menu()->platformMenu());
   } else {
      item->setMenu(0);
   }
}

/*!
  \reimp
*/
void QMenu::actionEvent(QActionEvent *e)
{
   Q_D(QMenu);
   d->itemsDirty = 1;
   setAttribute(Qt::WA_Resized, false);
   if (d->tornPopup) {
      d->tornPopup->syncWithMenu(this, e);
   }
   if (e->type() == QEvent::ActionAdded) {
      if (!d->tornoff) {
         connect(e->action(), SIGNAL(triggered()), this, SLOT(_q_actionTriggered()));
         connect(e->action(), SIGNAL(hovered()), this, SLOT(_q_actionHovered()));
      }
      if (QWidgetAction *wa = qobject_cast<QWidgetAction *>(e->action())) {
         QWidget *widget = wa->requestWidget(this);
         if (widget) {
            d->widgetItems.insert(wa, widget);
         }
      }
   } else if (e->type() == QEvent::ActionRemoved) {
      e->action()->disconnect(this);
      if (e->action() == d->currentAction) {
         d->currentAction = 0;
      }
      if (QWidgetAction *wa = qobject_cast<QWidgetAction *>(e->action())) {
         if (QWidget *widget = d->widgetItems.value(wa)) {

#ifdef Q_OS_DARWIN
            QWidget *p = widget->parentWidget();
            if (p != this && qobject_cast<QMacNativeWidget *>(p)) {
               // This widget was reparented into a native Mac view
               // (see QMenuPrivate::moveWidgetToPlatformItem).
               // Reset the parent and delete the native widget.
               widget->setParent(this);
               p->deleteLater();
            }
#endif

            wa->releaseWidget(widget);
         }
      }
      d->widgetItems.remove(e->action());
   }

   if (!d->platformMenu.isNull()) {
      if (e->type() == QEvent::ActionAdded) {
         QPlatformMenuItem *menuItem = d->platformMenu->createMenuItem();
         menuItem->setTag(reinterpret_cast<quintptr>(e->action()));

         QObject::connect(menuItem, SIGNAL(activated()), e->action(), SLOT(trigger()));
         QObject::connect(menuItem, SIGNAL(hovered()), e->action(),   SLOT(hovered()));

         copyActionToPlatformItem(e->action(), menuItem, d->platformMenu);
         QPlatformMenuItem *beforeItem = d->platformMenu->menuItemForTag(reinterpret_cast<quintptr>(e->before()));
         d->platformMenu->insertMenuItem(menuItem, beforeItem);

      } else if (e->type() == QEvent::ActionRemoved) {
         QPlatformMenuItem *menuItem = d->platformMenu->menuItemForTag(reinterpret_cast<quintptr>(e->action()));
         d->platformMenu->removeMenuItem(menuItem);
         delete menuItem;

      } else if (e->type() == QEvent::ActionChanged) {
         QPlatformMenuItem *menuItem = d->platformMenu->menuItemForTag(reinterpret_cast<quintptr>(e->action()));
         if (menuItem) {
            copyActionToPlatformItem(e->action(), menuItem, d->platformMenu);
            d->platformMenu->syncMenuItem(menuItem);
         }
      }

      d->platformMenu->syncSeparatorsCollapsible(d->collapsibleSeparators);
   }


   if (isVisible()) {
      d->updateActionRects();
      resize(sizeHint());
      update();
   }
}

void QMenu::internalDelayedPopup()
{
   Q_D(QMenu);

   //hide the current item
   if (QMenu *menu = d->activeMenu) {
      if (d->activeMenu->menuAction() != d->currentAction) {
         d->hideMenu(menu);
      }
   }

   if (!d->currentAction || !d->currentAction->isEnabled() || !d->currentAction->menu() ||
      !d->currentAction->menu()->isEnabled() || d->currentAction->menu()->isVisible()) {
      return;
   }

   //setup
   d->activeMenu = d->currentAction->menu();
   d->activeMenu->d_func()->causedPopup.widget = this;
   d->activeMenu->d_func()->causedPopup.action = d->currentAction;

   int subMenuOffset = style()->pixelMetric(QStyle::PM_SubMenuOverlap, 0, this);
   const QRect actionRect(d->actionRect(d->currentAction));
   const QPoint rightPos(mapToGlobal(QPoint(actionRect.right() + subMenuOffset + 1, actionRect.top())));

   d->activeMenu->popup(rightPos);
   d->sloppyState.setSubMenuPopup(actionRect, d->currentAction, d->activeMenu);


#if !defined(Q_OS_DARWIN)
   // Send the leave event to the current menu - only active popup menu gets
   // mouse enter/leave events. Currently Cocoa is an exception, so disable
   // it there to avoid event duplication.
   if (underMouse()) {
      QEvent leaveEvent(QEvent::Leave);
      QCoreApplication::sendEvent(this, &leaveEvent);
   }
#endif
}
void QMenu::setNoReplayFor(QWidget *noReplayFor)
{
   d_func()->noReplayFor = noReplayFor;
}

QPlatformMenu *QMenu::platformMenu()
{

   return d_func()->platformMenu;
}
void QMenu::setPlatformMenu(QPlatformMenu *platformMenu)
{
   d_func()->setPlatformMenu(platformMenu);
   d_func()->syncPlatformMenu();
}

bool QMenu::separatorsCollapsible() const
{
   Q_D(const QMenu);
   return d->collapsibleSeparators;
}

void QMenu::setSeparatorsCollapsible(bool collapse)
{
   Q_D(QMenu);

   if (d->collapsibleSeparators == collapse) {
      return;
   }

   d->collapsibleSeparators = collapse;
   d->itemsDirty = 1;

   if (isVisible()) {
      d->updateActionRects();
      update();
   }

   if (!d->platformMenu.isNull()) {
      d->platformMenu->syncSeparatorsCollapsible(collapse);
   }
}


bool QMenu::toolTipsVisible() const
{
   Q_D(const QMenu);
   return d->toolTipsVisible;
}

void QMenu::setToolTipsVisible(bool visible)
{
   Q_D(QMenu);
   if (d->toolTipsVisible == visible) {
      return;
   }

   d->toolTipsVisible = visible;
}

void QMenu::_q_actionTriggered()
{
   Q_D(QMenu);
   d->_q_actionTriggered();
}

void QMenu::_q_actionHovered()
{
   Q_D(QMenu);
   d->_q_actionHovered();
}

void QMenu::_q_overrideMenuActionDestroyed()
{
   Q_D(QMenu);
   d->_q_overrideMenuActionDestroyed();
}

void QMenu::_q_platformMenuAboutToShow()
{
   Q_D(QMenu);
   d->_q_platformMenuAboutToShow();
}

#endif // QT_NO_MENU
