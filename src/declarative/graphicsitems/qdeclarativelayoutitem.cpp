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

#include "private/qdeclarativelayoutitem_p.h"

#include <QDebug>

#include <limits.h>

QT_BEGIN_NAMESPACE

/*!
    \qmlclass LayoutItem QDeclarativeLayoutItem
    \ingroup qml-utility-elements
    \since 4.7
    \brief The LayoutItem element allows declarative UI elements to be placed inside Qt's Graphics View layouts.

    LayoutItem is a variant of \l Item with additional size hint properties. These properties provide the size hints
    necessary for items to work in conjunction with Qt \l{Graphics View Framework}{Graphics View} layout classes
    such as QGraphicsLinearLayout and QGraphicsGridLayout. The Qt layout mechanisms will resize the LayoutItem as appropriate,
    taking its size hints into account, and you can propagate this to the other elements in your UI via anchors and bindings.

    This is a QGraphicsLayoutItem subclass, and its properties merely expose the QGraphicsLayoutItem functionality to QML.

    The \l{declarative/cppextensions/qgraphicslayouts/layoutitem}{LayoutItem example}
    demonstrates how a LayoutItem can be used within a \l{Graphics View Framework}{Graphics View}
    scene.
*/

/*!
    \qmlproperty QSizeF LayoutItem::maximumSize

    The maximumSize property can be set to specify the maximum desired size of this LayoutItem
*/

/*!
    \qmlproperty QSizeF LayoutItem::minimumSize

    The minimumSize property can be set to specify the minimum desired size of this LayoutItem
*/

/*!
    \qmlproperty QSizeF LayoutItem::preferredSize

    The preferredSize property can be set to specify the preferred size of this LayoutItem
*/

QDeclarativeLayoutItem::QDeclarativeLayoutItem(QDeclarativeItem *parent)
   : QDeclarativeItem(parent), m_maximumSize(INT_MAX, INT_MAX), m_minimumSize(0, 0), m_preferredSize(0, 0)
{
   setGraphicsItem(this);
}

void QDeclarativeLayoutItem::setGeometry(const QRectF &rect)
{
   setX(rect.x());
   setY(rect.y());
   setWidth(rect.width());
   setHeight(rect.height());
}

QSizeF QDeclarativeLayoutItem::sizeHint(Qt::SizeHint w, const QSizeF &constraint) const
{
   Q_UNUSED(constraint);
   if (w == Qt::MinimumSize) {
      return m_minimumSize;
   } else if (w == Qt::MaximumSize) {
      return m_maximumSize;
   } else {
      return m_preferredSize;
   }
}

QT_END_NAMESPACE
