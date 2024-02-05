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

#ifndef QMENUBAR_P_H
#define QMENUBAR_P_H

#include <qstyleoption.h>
#include <qmenu_p.h>          // Mac needs what in this file
#include <qplatform_menu.h>

#ifndef QT_NO_MENUBAR
class QToolBar;
class QMenuBarExtension;

class QMenuBarPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QMenuBar)

 public:
   QMenuBarPrivate()
      : itemsDirty(0), currentAction(nullptr), mouseDown(0),
        closePopupMode(0), defaultPopDown(1), popupState(0), keyboardState(0), altPressed(0),
        doChildEffects(false), platformMenuBar(nullptr)
   {
   }

   ~QMenuBarPrivate()
   {
      delete platformMenuBar;
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
   QVector<QPointer<QWidget>> oldParents;

   QList<QAction *> hiddenActions;

   //default action
   QPointer<QAction> defaultAction;

   QBasicTimer autoReleaseTimer;

   QPlatformMenuBar *platformMenuBar;
   int indexOf(QAction *act) const {
      return q_func()->actions().indexOf(act);
   }
};

#endif // QT_NO_MENUBAR

#endif // QMENUBAR_P_H
