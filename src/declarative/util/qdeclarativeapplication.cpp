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

#include "qdeclarativeapplication_p.h"
#include <QtGui/QApplication>

QT_BEGIN_NAMESPACE

class QDeclarativeApplicationPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeApplication)

 public:
   QDeclarativeApplicationPrivate() : active(QApplication::activeWindow() != 0),
      layoutDirection(QApplication::layoutDirection()) {}
   bool active;
   Qt::LayoutDirection layoutDirection;
};

/*
    This object and its properties are documented as part of the Qt object,
    in qdeclarativengine.cpp
*/

QDeclarativeApplication::QDeclarativeApplication(QObject *parent) : QObject(*new QDeclarativeApplicationPrivate(),
         parent)
{
   if (qApp) {
      qApp->installEventFilter(this);
   }
}

QDeclarativeApplication::~QDeclarativeApplication()
{
}

bool QDeclarativeApplication::active() const
{
   Q_D(const QDeclarativeApplication);
   return d->active;
}

Qt::LayoutDirection QDeclarativeApplication::layoutDirection() const
{
   Q_D(const QDeclarativeApplication);
   return d->layoutDirection;
}

bool QDeclarativeApplication::eventFilter(QObject *obj, QEvent *event)
{
   Q_UNUSED(obj)
   Q_D(QDeclarativeApplication);
   if (event->type() == QEvent::ApplicationActivate
         || event->type() == QEvent::ApplicationDeactivate) {
      bool active = d->active;
      if (event->type() == QEvent::ApplicationActivate) {
         active  = true;
      } else if (event->type() == QEvent::ApplicationDeactivate) {
         active  = false;
      }

      if (d->active != active) {
         d->active = active;
         emit activeChanged();
      }
   }
   if (event->type() == QEvent::LayoutDirectionChange) {
      Qt::LayoutDirection direction = QApplication::layoutDirection();
      if (d->layoutDirection != direction) {
         d->layoutDirection = direction;
         emit layoutDirectionChanged();
      }
   }
   return false;
}

QT_END_NAMESPACE
