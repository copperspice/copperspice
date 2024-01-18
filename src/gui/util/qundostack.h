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

#ifndef QUNDOSTACK_H
#define QUNDOSTACK_H

#include <qobject.h>
#include <qscopedpointer.h>
#include <qstring.h>

class QAction;
class QUndoCommandPrivate;
class QUndoStackPrivate;

#ifndef QT_NO_UNDOCOMMAND

class Q_GUI_EXPORT QUndoCommand
{
   QUndoCommandPrivate *d;

 public:
   explicit QUndoCommand(QUndoCommand *parent = nullptr);
   explicit QUndoCommand(const QString &text, QUndoCommand *parent = nullptr);

   QUndoCommand(const QUndoCommand &) = delete;
   QUndoCommand &operator=(const QUndoCommand &) = delete;

   virtual ~QUndoCommand();

   virtual void undo();
   virtual void redo();

   QString text() const;
   QString actionText() const;
   void setText(const QString &text);

   virtual int id() const;
   virtual bool mergeWith(const QUndoCommand *other);

   int childCount() const;
   const QUndoCommand *child(int index) const;

 private:
   friend class QUndoStack;
};

#endif // QT_NO_UNDOCOMMAND

#ifndef QT_NO_UNDOSTACK

class Q_GUI_EXPORT QUndoStack : public QObject
{
   GUI_CS_OBJECT(QUndoStack)

   GUI_CS_PROPERTY_READ(active, isActive)
   GUI_CS_PROPERTY_WRITE(active, setActive)
   GUI_CS_PROPERTY_READ(undoLimit, undoLimit)
   GUI_CS_PROPERTY_WRITE(undoLimit, setUndoLimit)

 public:
   explicit QUndoStack(QObject *parent = nullptr);

   QUndoStack(const QUndoStack &) = delete;
   QUndoStack &operator=(const QUndoStack &) = delete;

   ~QUndoStack();

   void clear();

   void push(QUndoCommand *cmd);

   bool canUndo() const;
   bool canRedo() const;
   QString undoText() const;
   QString redoText() const;

   int count() const;
   int index() const;
   QString text(int idx) const;

#ifndef QT_NO_ACTION
   QAction *createUndoAction(QObject *parent, const QString &prefix = QString()) const;
   QAction *createRedoAction(QObject *parent, const QString &prefix = QString()) const;
#endif

   bool isActive() const;
   bool isClean() const;
   int cleanIndex() const;

   void beginMacro(const QString &text);
   void endMacro();

   void setUndoLimit(int limit);
   int undoLimit() const;

   const QUndoCommand *command(int index) const;

   GUI_CS_SLOT_1(Public, void setClean())
   GUI_CS_SLOT_2(setClean)

   GUI_CS_SLOT_1(Public, void setIndex(int idx))
   GUI_CS_SLOT_2(setIndex)

   GUI_CS_SLOT_1(Public, void undo())
   GUI_CS_SLOT_2(undo)

   GUI_CS_SLOT_1(Public, void redo())
   GUI_CS_SLOT_2(redo)

   GUI_CS_SLOT_1(Public, void setActive(bool active = true))
   GUI_CS_SLOT_2(setActive)

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

 protected:
   QScopedPointer<QUndoStackPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QUndoStack)
   friend class QUndoGroup;
};

#endif // QT_NO_UNDOSTACK

#endif // QUNDOSTACK_H
