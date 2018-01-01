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

#ifndef QMENU_P_H
#define QMENU_P_H

#include <QtGui/qmenubar.h>
#include <QtGui/qstyleoption.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qmap.h>
#include <QtCore/qhash.h>
#include <QtCore/qbasictimer.h>
#include <qwidget_p.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_MENU

class QTornOffMenu;
class QEventLoop;

#ifdef Q_OS_MAC

#  ifdef __OBJC__
QT_END_NAMESPACE
@class NSMenuItem;
QT_BEGIN_NAMESPACE
#  else
typedef void NSMenuItem;
#  endif

struct QMacMenuAction {
   QMacMenuAction()
      : menuItem(0), ignore_accel(0), merged(0), menu(0) {
   }
   ~QMacMenuAction();

   NSMenuItem *menuItem;
   uchar ignore_accel : 1;
   uchar merged : 1;
   QPointer<QAction> action;
   OSMenuRef menu;
};

struct QMenuMergeItem {
   inline QMenuMergeItem(NSMenuItem *c, QMacMenuAction *a) : menuItem(c), action(a) { }
   NSMenuItem *menuItem;
   QMacMenuAction *action;
};
typedef QList<QMenuMergeItem> QMenuMergeList;
#endif


class QMenuPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QMenu)

 public:
   QMenuPrivate() : itemsDirty(0), maxIconWidth(0), tabWidth(0), ncols(0),
      collapsibleSeparators(true), toolTipsVisible(false),
      activationRecursionGuard(false), hasHadMouse(0), aboutToHide(0), motions(0),
      currentAction(0),
#ifdef QT_KEYPAD_NAVIGATION
      selectAction(0),
      cancelAction(0),
#endif
      scroll(0), eventLoop(0), tearoff(0), tornoff(0), tearoffHighlighted(0),
      hasCheckableItems(0), sloppyDelayTimer(0), sloppyAction(0), doChildEffects(false)
#ifdef Q_OS_MAC
      , mac_menu(0)
#endif

   { }
   ~QMenuPrivate() {
      delete scroll;
#ifdef Q_OS_MAC
      delete mac_menu;
#endif
   }
   void init();

   static QMenuPrivate *get(QMenu *m) {
      return m->d_func();
   }
   int scrollerHeight() const;

   //item calculations
   mutable uint itemsDirty : 1;
   mutable uint maxIconWidth, tabWidth;
   QRect actionRect(QAction *) const;

   mutable QVector<QRect> actionRects;
   mutable QHash<QAction *, QWidget *> widgetItems;
   void updateActionRects() const;
   void updateActionRects(const QRect &screen) const;
   QRect popupGeometry(const QWidget *widget) const;
   QRect popupGeometry(int screen = -1) const;
   mutable uint ncols : 4; //4 bits is probably plenty
   uint collapsibleSeparators : 1;
   uint toolTipsVisible : 1;
   QSize adjustMenuSizeForScreen(const QRect &screen);
   int getLastVisibleAction() const;

   bool activationRecursionGuard;

   //selection
   static QMenu *mouseDown;
   QPoint mousePopupPos;
   uint hasHadMouse : 1;
   uint aboutToHide : 1;
   int motions;
   QAction *currentAction;

#ifdef QT_KEYPAD_NAVIGATION
   QAction *selectAction;
   QAction *cancelAction;
#endif

   QBasicTimer menuDelayTimer;
   enum SelectionReason {
      SelectedFromKeyboard,
      SelectedFromElsewhere
   };

   QWidget *topCausedWidget() const;
   QAction *actionAt(QPoint p) const;
   void setFirstActionActive();
   void setCurrentAction(QAction *, int popup = -1, SelectionReason reason = SelectedFromElsewhere,
                         bool activateFirst = false);
   void popupAction(QAction *, int, bool);
   void setSyncAction();

   //scrolling support
   struct QMenuScroller {
      enum ScrollLocation { ScrollStay, ScrollBottom, ScrollTop, ScrollCenter };
      enum ScrollDirection { ScrollNone = 0, ScrollUp = 0x01, ScrollDown = 0x02 };
      uint scrollFlags : 2, scrollDirection : 2;
      int scrollOffset;
      QBasicTimer scrollTimer;

      QMenuScroller() : scrollFlags(ScrollNone), scrollDirection(ScrollNone), scrollOffset(0) { }
      ~QMenuScroller() { }
   } *scroll;

   void scrollMenu(QMenuScroller::ScrollLocation location, bool active = false);
   void scrollMenu(QMenuScroller::ScrollDirection direction, bool page = false, bool active = false);
   void scrollMenu(QAction *action, QMenuScroller::ScrollLocation location, bool active = false);

   //synchronous operation (ie exec())
   QEventLoop *eventLoop;
   QPointer<QAction> syncAction;

   //search buffer
   QString searchBuffer;
   QBasicTimer searchBufferTimer;

   //passing of mouse events up the parent hierarchy
   QPointer<QMenu> activeMenu;
   bool mouseEventTaken(QMouseEvent *);

   //used to walk up the popup list
   struct QMenuCaused {
      QPointer<QWidget> widget;
      QPointer<QAction> action;
   };

   virtual QList<QPointer<QWidget> > calcCausedStack() const;
   QMenuCaused causedPopup;
   void hideUpToMenuBar();
   void hideMenu(QMenu *menu, bool justRegister = false);

   //index mappings
   inline QAction *actionAt(int i) const {
      return q_func()->actions().at(i);
   }
   inline int indexOf(QAction *act) const {
      return q_func()->actions().indexOf(act);
   }

   //tear off support
   uint tearoff : 1, tornoff : 1, tearoffHighlighted : 1;
   QPointer<QTornOffMenu> tornPopup;

   mutable bool hasCheckableItems;

   //sloppy selection
   int sloppyDelayTimer;
   mutable QAction *sloppyAction;
   QRegion sloppyRegion;

   //default action
   QPointer<QAction> defaultAction;

   QAction *menuAction;
   QAction *defaultMenuAction;

   void setOverrideMenuAction(QAction *);
   void _q_overrideMenuActionDestroyed();

   //firing of events
   void activateAction(QAction *, QAction::ActionEvent, bool self = true);
   void activateCausedStack(const QList<QPointer<QWidget> > &, QAction *, QAction::ActionEvent, bool);

   void _q_actionTriggered();
   void _q_actionHovered();

   bool hasMouseMoved(const QPoint &globalPos);

   void updateLayoutDirection();

   //menu fading/scrolling effects
   bool doChildEffects;

#ifdef Q_OS_MAC
   //mac menu binding
   struct QMacMenuPrivate {
      QList<QMacMenuAction *> actionItems;
      OSMenuRef menu;
      QMacMenuPrivate();
      ~QMacMenuPrivate();

      bool merged(const QAction *action) const;
      void addAction(QAction *, QMacMenuAction * = 0, QMenuPrivate *qmenu = 0);
      void addAction(QMacMenuAction *, QMacMenuAction * = 0, QMenuPrivate *qmenu = 0);
      void syncAction(QMacMenuAction *);
      inline void syncAction(QAction *a) {
         syncAction(findAction(a));
      }
      void removeAction(QMacMenuAction *);
      inline void removeAction(QAction *a) {
         removeAction(findAction(a));
      }
      inline QMacMenuAction *findAction(QAction *a) {
         for (int i = 0; i < actionItems.size(); i++) {
            QMacMenuAction *act = actionItems[i];
            if (a == act->action) {
               return act;
            }
         }
         return 0;
      }
   } *mac_menu;

   OSMenuRef macMenu(OSMenuRef merge);

   void syncSeparatorsCollapsible(bool collapsible);
   static QHash<OSMenuRef, OSMenuRef> mergeMenuHash;
   static QHash<OSMenuRef, QMenuMergeList *> mergeMenuItemsHash;
#endif

   QPointer<QAction> actionAboutToTrigger;
   QPointer<QWidget> noReplayFor;
};

#endif // QT_NO_MENU

QT_END_NAMESPACE

#endif // QMENU_P_H
