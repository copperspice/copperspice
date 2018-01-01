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

#ifndef TOOLBARCOLORBOX_H
#define TOOLBARCOLORBOX_H

#include <QtGui/QLabel>
#include <QtGui/QColor>
#include <QtCore/QPoint>

QT_FORWARD_DECLARE_CLASS(QContextMenuEvent)
QT_FORWARD_DECLARE_CLASS(QAction)

namespace QmlJSDebugger {

class ToolBarColorBox : public QLabel
{
    Q_OBJECT

public:
    explicit ToolBarColorBox(QWidget *parent = nullptr);
    void setColor(const QColor &color);

protected:
    void contextMenuEvent(QContextMenuEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
private slots:
    void copyColorToClipboard();

private:
    QPixmap createDragPixmap(int size = 24) const;

private:
    bool m_dragStarted;
    QPoint m_dragBeginPoint;
    QAction *m_copyHexColor;
    QColor m_color;
};

} // namespace QmlJSDebugger

#endif // TOOLBARCOLORBOX_H
