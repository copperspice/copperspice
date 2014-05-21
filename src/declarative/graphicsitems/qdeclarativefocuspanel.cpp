/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "private/qdeclarativefocuspanel_p.h"

#include "private/qdeclarativeitem_p.h"

#include <QtGui/qgraphicsscene.h>
#include <QEvent>

QT_BEGIN_NAMESPACE

/*!
   \qmlclass FocusPanel QDeclarativeFocusPanel
    \since 4.7
    \ingroup qml-basic-interaction-elements

   \brief The FocusPanel item explicitly creates a focus panel.
   \inherits Item

    Focus panels assist in keyboard focus handling when building QML
    applications.  All the details are covered in the 
    \l {qmlfocus}{keyboard focus documentation}.
*/

QDeclarativeFocusPanel::QDeclarativeFocusPanel(QDeclarativeItem *parent) :
    QDeclarativeItem(parent)
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
        event->type() == QEvent::WindowDeactivate)
        emit activeChanged();
    return QDeclarativeItem::sceneEvent(event);
}

QT_END_NAMESPACE
