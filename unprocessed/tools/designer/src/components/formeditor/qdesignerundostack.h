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

#ifndef QDESIGNERUNDOSTACK_H
#define QDESIGNERUNDOSTACK_H

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE
class QUndoStack;
class QUndoCommand;

namespace qdesigner_internal {

/* QDesignerUndoStack: A QUndoStack extended by a way of setting it to
 * "dirty" indepently of commands (by modifications without commands
 * such as resizing). Accomplished via bool m_fakeDirty flag. The
 * lifecycle of the QUndoStack is managed by the QUndoGroup. */
class QDesignerUndoStack : public QObject
{
    Q_DISABLE_COPY(QDesignerUndoStack)
    Q_OBJECT
public:
    explicit QDesignerUndoStack(QObject *parent = 0);
    virtual ~QDesignerUndoStack();

    void push(QUndoCommand * cmd);
    void beginMacro(const QString &text);
    void endMacro();
    int  index() const;

    const QUndoStack *qundoStack() const;
    QUndoStack *qundoStack();

    bool isDirty() const;

signals:
    void changed();

public slots:
    void setDirty(bool);

private:
    QUndoStack *m_undoStack;
    bool m_fakeDirty;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QDESIGNERUNDOSTACK_H
