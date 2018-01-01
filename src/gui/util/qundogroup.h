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

#ifndef QUNDOGROUP_H
#define QUNDOGROUP_H

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QUndoGroupPrivate;
class QUndoStack;
class QAction;

#ifndef QT_NO_UNDOGROUP

class Q_GUI_EXPORT QUndoGroup : public QObject
{
   GUI_CS_OBJECT(QUndoGroup)
   Q_DECLARE_PRIVATE(QUndoGroup)

 public:
   explicit QUndoGroup(QObject *parent = nullptr);
   ~QUndoGroup();

   void addStack(QUndoStack *stack);
   void removeStack(QUndoStack *stack);
   QList<QUndoStack *> stacks() const;
   QUndoStack *activeStack() const;

#ifndef QT_NO_ACTION
   QAction *createUndoAction(QObject *parent, const QString &prefix = QString()) const;
   QAction *createRedoAction(QObject *parent, const QString &prefix = QString()) const;
#endif

   bool canUndo() const;
   bool canRedo() const;
   QString undoText() const;
   QString redoText() const;
   bool isClean() const;

   GUI_CS_SLOT_1(Public, void undo())
   GUI_CS_SLOT_2(undo)
   GUI_CS_SLOT_1(Public, void redo())
   GUI_CS_SLOT_2(redo)
   GUI_CS_SLOT_1(Public, void setActiveStack(QUndoStack *stack))
   GUI_CS_SLOT_2(setActiveStack)

   GUI_CS_SIGNAL_1(Public, void activeStackChanged(QUndoStack *stack))
   GUI_CS_SIGNAL_2(activeStackChanged, stack)
   GUI_CS_SIGNAL_1(Public, void indexChanged(int idx))
   GUI_CS_SIGNAL_2(indexChanged, idx)
   GUI_CS_SIGNAL_1(Public, void cleanChanged(bool clean))
   GUI_CS_SIGNAL_2(cleanChanged, clean)
   GUI_CS_SIGNAL_1(Public, void canUndoChanged(bool canUndo))
   GUI_CS_SIGNAL_2(canUndoChanged, canUndo)
   GUI_CS_SIGNAL_1(Public, void canRedoChanged(bool canRedo))
   GUI_CS_SIGNAL_2(canRedoChanged, canRedo)
   GUI_CS_SIGNAL_1(Public, void undoTextChanged(const QString &undoText))
   GUI_CS_SIGNAL_2(undoTextChanged, undoText)
   GUI_CS_SIGNAL_1(Public, void redoTextChanged(const QString &redoText))
   GUI_CS_SIGNAL_2(redoTextChanged, redoText)

 private:
   Q_DISABLE_COPY(QUndoGroup)

 protected:
   QScopedPointer<QUndoGroupPrivate> d_ptr;
};

#endif // QT_NO_UNDOGROUP

QT_END_NAMESPACE

#endif // QUNDOGROUP_H
