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

#include "qworkspace_container.h"
#include "qmdiarea_container.h"

#include <QtGui/QWorkspace>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

QWorkspaceContainer::QWorkspaceContainer(QWorkspace *widget, QObject *parent)
    : QObject(parent),
      m_workspace(widget)
{
}

int QWorkspaceContainer::count() const
{
    return m_workspace->windowList(QWorkspace::CreationOrder).count();
}

QWidget *QWorkspaceContainer::widget(int index) const
{
    if (index < 0)
        return 0;
    return m_workspace->windowList(QWorkspace::CreationOrder).at(index);
}

int QWorkspaceContainer::currentIndex() const
{
    if (QWidget *aw = m_workspace->activeWindow())
        return m_workspace->windowList(QWorkspace::CreationOrder).indexOf(aw);
    return -1;
}

void QWorkspaceContainer::setCurrentIndex(int index)
{
    m_workspace->setActiveWindow(m_workspace->windowList(QWorkspace::CreationOrder).at(index));
}

void QWorkspaceContainer::addWidget(QWidget *widget)
{
    QWidget *frame = m_workspace->addWindow(widget, Qt::Window);
    frame->show();
    m_workspace->cascade();
    QMdiAreaContainer::positionNewMdiChild(m_workspace, frame);
}

void QWorkspaceContainer::insertWidget(int, QWidget *widget)
{
    addWidget(widget);
}

void QWorkspaceContainer::remove(int /* index */)
{
    // nothing to do here, reparenting to formwindow is apparently sufficient
}
}

QT_END_NAMESPACE
