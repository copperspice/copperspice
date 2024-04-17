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

#ifndef QWIDGETACTION_P_H
#define QWIDGETACTION_P_H

#include <qaction_p.h>

class QWidgetActionPrivate : public QActionPrivate
{
   Q_DECLARE_PUBLIC(QWidgetAction)

 public:
   QWidgetActionPrivate()
      : defaultWidgetInUse(false), autoCreated(false)
   { }

   QPointer<QWidget> defaultWidget;
   QList<QWidget *> createdWidgets;

   uint defaultWidgetInUse : 1;
   uint autoCreated        : 1; // created by QToolBar::addWidget and the like

   void _q_widgetDestroyed(QObject *o) {
      createdWidgets.removeAll(static_cast<QWidget *>(o));
   }
};

#endif
