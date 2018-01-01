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

#ifndef QABSTRACTBUTTON_P_H
#define QABSTRACTBUTTON_P_H

#include <QtCore/qbasictimer.h>
#include <qwidget_p.h>

QT_BEGIN_NAMESPACE

class QAbstractButtonPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QAbstractButton)

 public:
   QAbstractButtonPrivate(QSizePolicy::ControlType type = QSizePolicy::DefaultType);

   QString text;
   QIcon icon;
   QSize iconSize;

#ifndef QT_NO_SHORTCUT
   QKeySequence shortcut;
   int shortcutId;
#endif

   uint checkable : 1;
   uint checked : 1;
   uint autoRepeat : 1;
   uint autoExclusive : 1;
   uint down : 1;
   uint blockRefresh : 1;
   uint pressed : 1;

#ifndef QT_NO_BUTTONGROUP
   QButtonGroup *group;
#endif

   QBasicTimer repeatTimer;
   QBasicTimer animateTimer;

   int autoRepeatDelay, autoRepeatInterval;

   QSizePolicy::ControlType controlType;
   mutable QSize sizeHint;

   void init();
   void click();
   void refresh();

   QList<QAbstractButton *>queryButtonList() const;
   QAbstractButton *queryCheckedButton() const;
   void notifyChecked();
   void moveFocus(int key);
   void fixFocusPolicy();

   void emitPressed();
   void emitReleased();
   void emitClicked();
};

QT_END_NAMESPACE

#endif // QABSTRACTBUTTON_P_H
