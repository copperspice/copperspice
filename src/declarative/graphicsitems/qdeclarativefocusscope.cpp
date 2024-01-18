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

#include "private/qdeclarativefocusscope_p.h"

#include "private/qdeclarativeitem_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmlclass FocusScope QDeclarativeFocusScope
    \since 4.7
    \ingroup qml-basic-interaction-elements

    \brief The FocusScope object explicitly creates a focus scope.
    \inherits Item

    Focus scopes assist in keyboard focus handling when building reusable QML
    components.  All the details are covered in the
    \l {qmlfocus}{keyboard focus documentation}.

    \sa {declarative/keyinteraction/focus}{Keyboard focus example}
*/

QDeclarativeFocusScope::QDeclarativeFocusScope(QDeclarativeItem *parent) :
   QDeclarativeItem(parent)
{
   Q_D(QDeclarativeItem);
   d->flags |= QGraphicsItem::ItemIsFocusScope;
}

QDeclarativeFocusScope::~QDeclarativeFocusScope()
{
}
QT_END_NAMESPACE
