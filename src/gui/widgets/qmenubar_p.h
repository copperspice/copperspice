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

#ifndef QMENUBAR_P_H
#define QMENUBAR_P_H

#include <QtGui/qstyleoption.h>
#include <qmenu_p.h> // Mac needs what in this file!

#ifdef Q_WS_X11
#include <qabstractplatformmenubar_p.h>
#endif

#ifndef QT_NO_MENUBAR
class QToolBar;
class QMenuBarExtension;

class QMenuBarPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QMenuBar)

 public:
   QMenuBarPrivate() : itemsDirty(0), currentAction(0), mouseDown(0),
      closePopupMode(0), defaultPopDown(1), popupState(0), keyboardState(0), altPressed(0)
#ifndef Q_WS_X11
      , nativeMenuBar(-1)
#endif
      , doChildEffects(false)

#ifdef Q_OS_MAC
      , mac_menubar(0)
#endif

#ifdef Q_WS_X11
      , platformMenuBar(0)
#endif
   { }

   ~QMenuBarPrivate() {
#ifdef Q_WS_X11
      delete platformMenuBar;
#endif

#ifdef Q_OS_MAC
      delete mac_menubar;
#endif
   }

   void init();
   QAction *getNextAction(const int start, const int increment) const;

   //item calculations
   uint itemsDirty : 1;

   QVector<int> shortcutIndexMap;
   mutable QVector<QRect> actionRects;
   void calcActionRects(int max_width, int start) const;
   QRect actionRect(QAction *) const;
   void updateGeometries();

   //selection
   QPointer<QAction>currentAction;
   uint mouseDown : 1, closePopupMode : 1, defaultPopDown;
   QAction *actionAt(QPoint p) const;
   void setCurrentAction(QAction *, bool = false, bool = false);
   void popupAction(QAction *, bool);

   //active popup state
   uint popupState : 1;
   QPointer<QMenu> activeMenu;

   //keyboard mode for keyboard navigation
   void focusFirstAction();
   void setKeyboardMode(bool);
   uint keyboardState : 1, altPressed : 1;
   QPointer<QWidget> keyboardFocusWidget;

#ifndef Q_WS_X11
   int nativeMenuBar : 3;  // Only has values -1, 0, and 1
#endif

   //firing of events
   void activateAction(QAction *, QAction::ActionEvent);

   void _q_actionTriggered();
   void _q_actionHovered();
   void _q_internalShortcutActivated(int);
   void _q_updateLayout();

   //extra widgets in the menubar
   QPointer<QWidget> leftWidget, rightWidget;
   QMenuBarExtension *extension;
   bool isVisible(QAction *action);

   //menu fading/scrolling effects
   bool doChildEffects;

   QRect menuRect(bool) const;

   // reparenting
   void handleReparent();
   QWidget *oldParent;
   QWidget *oldWindow;

   QList<QAction *> hiddenActions;
   //default action
   QPointer<QAction> defaultAction;

   QBasicTimer autoReleaseTimer;

#ifdef Q_WS_X11
   QAbstractPlatformMenuBar *platformMenuBar;
#endif

#ifdef Q_OS_MAC
   //mac menubar binding
   struct QMacMenuBarPrivate {
      QList<QMacMenuAction *> actionItems;
      OSMenuRef menu, apple_menu;
      QMacMenuBarPrivate();
      ~QMacMenuBarPrivate();

      void addAction(QAction *, QAction * = 0);
      void addAction(QMacMenuAction *, QMacMenuAction * = 0);
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
   } *mac_menubar;
   static bool macUpdateMenuBarImmediatly();
   bool macWidgetHasNativeMenubar(QWidget *widget);
   void macCreateMenuBar(QWidget *);
   void macDestroyMenuBar();
   OSMenuRef macMenu();
#endif

#ifdef Q_WS_X11
   void updateCornerWidgetToolBar();
   QToolBar *cornerWidgetToolBar;
   QWidget *cornerWidgetContainer;
#endif

};

#endif // QT_NO_MENUBAR

#endif // QMENUBAR_P_H
