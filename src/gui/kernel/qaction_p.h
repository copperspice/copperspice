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

#ifndef QACTION_P_H
#define QACTION_P_H

#include <qaction.h>
#include <qmenu.h>
#include <qgraphicswidget_p.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_ACTION
class QShortcutMap;

class QActionPrivate
{
   Q_DECLARE_PUBLIC(QAction)

 public:
   QActionPrivate();
   virtual ~QActionPrivate();

   static QActionPrivate *get(QAction *q) {
      return q->d_func();
   }

   bool showStatusText(QWidget *w, const QString &str);

   QPointer<QActionGroup> group;
   QString text;
   QString iconText;
   QIcon icon;
   QString tooltip;
   QString statustip;
   QString whatsthis;

   QVariant userData;

#ifndef QT_NO_SHORTCUT
   QKeySequence shortcut;
   QList<QKeySequence> alternateShortcuts;

   int shortcutId;
   QList<int> alternateShortcutIds;
   Qt::ShortcutContext shortcutContext;
   uint autorepeat : 1;
#endif

   QFont font;
   QPointer<QMenu> menu;
   uint enabled : 1, forceDisabled : 1;
   uint visible : 1, forceInvisible : 1;
   uint checkable : 1;
   uint checked : 1;
   uint separator : 1;
   uint fontSet : 1;

   int iconVisibleInMenu : 3;  // Only has values -1, 0, and 1

   QAction::MenuRole menuRole;
   QAction::Priority priority;

   QList<QWidget *> widgets;

#ifndef QT_NO_GRAPHICSVIEW
   QList<QGraphicsWidget *> graphicsWidgets;
#endif

#ifndef QT_NO_SHORTCUT
   void redoGrab(QShortcutMap &map);
   void redoGrabAlternate(QShortcutMap &map);
   void setShortcutEnabled(bool enable, QShortcutMap &map);

   static QShortcutMap *globalMap;
#endif

   void sendDataChanged();

 protected:
   QAction *q_ptr;

};

#endif // QT_NO_ACTION

QT_END_NAMESPACE

#endif // QACTION_P_H
