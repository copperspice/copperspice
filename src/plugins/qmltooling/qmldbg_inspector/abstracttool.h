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

#ifndef ABSTRACTTOOL_H
#define ABSTRACTTOOL_H

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE
class QMouseEvent;
class QKeyEvent;
class QWheelEvent;
QT_END_NAMESPACE

namespace QmlJSDebugger {

class AbstractViewInspector;

class AbstractTool : public QObject
{
    Q_OBJECT

public:
    explicit AbstractTool(AbstractViewInspector *inspector);

    AbstractViewInspector *inspector() const { return m_inspector; }

    virtual void leaveEvent(QEvent *event) = 0;

    virtual void mousePressEvent(QMouseEvent *event) = 0;
    virtual void mouseMoveEvent(QMouseEvent *event) = 0;
    virtual void mouseReleaseEvent(QMouseEvent *event) = 0;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) = 0;

    virtual void hoverMoveEvent(QMouseEvent *event) = 0;
    virtual void wheelEvent(QWheelEvent *event) = 0;

    virtual void keyPressEvent(QKeyEvent *event) = 0;
    virtual void keyReleaseEvent(QKeyEvent *keyEvent) = 0;

private:
    AbstractViewInspector *m_inspector;
};

} // namespace QmlJSDebugger

#endif // ABSTRACTTOOL_H
