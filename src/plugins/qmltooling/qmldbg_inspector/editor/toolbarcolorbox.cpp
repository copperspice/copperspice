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

#include "toolbarcolorbox.h"

#include "../qmlinspectorconstants.h"

#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QClipboard>
#include <QtGui/QApplication>
#include <QtGui/QColorDialog>
#include <QtGui/QDrag>

#include <QtCore/QMimeData>
#include <QtCore/QDebug>

namespace QmlJSDebugger {

ToolBarColorBox::ToolBarColorBox(QWidget *parent) :
    QLabel(parent)
{
    m_copyHexColor = new QAction(QIcon(QLatin1String(":/qml/images/color-picker-hicontrast.png")),
                                 tr("Copy Color"), this);
    connect(m_copyHexColor, SIGNAL(triggered()), SLOT(copyColorToClipboard()));
    setScaledContents(false);
}

void ToolBarColorBox::setColor(const QColor &color)
{
    m_color = color;

    QPixmap pix = createDragPixmap(width());
    setPixmap(pix);
    update();
}

void ToolBarColorBox::mousePressEvent(QMouseEvent *event)
{
    m_dragBeginPoint = event->pos();
    m_dragStarted = false;
}

void ToolBarColorBox::mouseMoveEvent(QMouseEvent *event)
{

    if (event->buttons() & Qt::LeftButton
            && (QPoint(event->pos() - m_dragBeginPoint).manhattanLength()
                > Constants::DragStartDistance)
            && !m_dragStarted)
    {
        m_dragStarted = true;
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setText(m_color.name());
        drag->setMimeData(mimeData);
        drag->setPixmap(createDragPixmap());

        drag->exec();
    }
}

QPixmap ToolBarColorBox::createDragPixmap(int size) const
{
    QPixmap pix(size, size);
    QPainter p(&pix);

    QColor borderColor1 = QColor(143, 143 ,143);
    QColor borderColor2 = QColor(43, 43, 43);

    p.setBrush(QBrush(m_color));
    p.setPen(QPen(QBrush(borderColor2),1));

    p.fillRect(0, 0, size, size, borderColor1);
    p.drawRect(1,1, size - 3, size - 3);
    return pix;
}

void ToolBarColorBox::contextMenuEvent(QContextMenuEvent *ev)
{
    QMenu contextMenu;
    contextMenu.addAction(m_copyHexColor);
    contextMenu.exec(ev->globalPos());
}

void ToolBarColorBox::copyColorToClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(m_color.name());
}

} // namespace QmlJSDebugger
