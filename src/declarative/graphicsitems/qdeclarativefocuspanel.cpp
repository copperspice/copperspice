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

#include <qdeclarativefocuspanel_p.h>
#include <qdeclarativeitem_p.h>
#include <QtGui/qgraphicsscene.h>
#include <QEvent>

QT_BEGIN_NAMESPACE

QDeclarativeFocusPanel::QDeclarativeFocusPanel(QDeclarativeItem *parent) : QDeclarativeItem(parent)
{
   Q_D(QDeclarativeItem);
   d->flags |= QGraphicsItem::ItemIsPanel;
}

QDeclarativeFocusPanel::~QDeclarativeFocusPanel()
{
}

/*!
    \qmlproperty bool FocusPanel::active

    Sets whether the item is the active focus panel.
*/

bool QDeclarativeFocusPanel::sceneEvent(QEvent *event)
{
   if (event->type() == QEvent::WindowActivate ||
         event->type() == QEvent::WindowDeactivate) {
      emit activeChanged();
   }
   return QDeclarativeItem::sceneEvent(event);
}

QT_END_NAMESPACE
