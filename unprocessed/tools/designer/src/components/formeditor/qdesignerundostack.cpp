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

#include "qdesignerundostack.h"

#include <QtGui/QUndoStack>
#include <QtGui/QUndoCommand>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

QDesignerUndoStack::QDesignerUndoStack(QObject *parent) :
    QObject(parent),
    m_undoStack(new QUndoStack),
    m_fakeDirty(false)
{
    connect(m_undoStack, SIGNAL(indexChanged(int)), this, SIGNAL(changed()));
}

QDesignerUndoStack::~QDesignerUndoStack()
{ // QUndoStack is managed by the QUndoGroup
}

void QDesignerUndoStack::push(QUndoCommand * cmd)
{
    m_undoStack->push(cmd);
}

void QDesignerUndoStack::beginMacro(const QString &text)
{
    m_undoStack->beginMacro(text);
}

void QDesignerUndoStack::endMacro()
{
    m_undoStack->endMacro();
}

int  QDesignerUndoStack::index() const
{
    return m_undoStack->index();
}

const QUndoStack *QDesignerUndoStack::qundoStack() const
{
    return m_undoStack;
}
QUndoStack *QDesignerUndoStack::qundoStack()
{
    return m_undoStack;
}

bool QDesignerUndoStack::isDirty() const
{
    return m_fakeDirty || !m_undoStack->isClean();
}

void QDesignerUndoStack::setDirty(bool v)
{
    if (isDirty() == v)
        return;
    if (v) {
        m_fakeDirty = true;
        emit changed();
    } else {
        m_fakeDirty = false;
        m_undoStack->setClean();
    }
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
