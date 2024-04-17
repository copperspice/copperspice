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

#include <qundogroup.h>

#include <qundostack.h>

#include <qundostack_p.h>

#ifndef QT_NO_UNDOGROUP

class QUndoGroupPrivate
{
   Q_DECLARE_PUBLIC(QUndoGroup)

 public:
   QUndoGroupPrivate() : active(nullptr) {}
   virtual ~QUndoGroupPrivate() {}

   QUndoStack *active;
   QList<QUndoStack *> stack_list;

 protected:
   QUndoGroup *q_ptr;
};

QUndoGroup::QUndoGroup(QObject *parent)
   : QObject(parent), d_ptr(new QUndoGroupPrivate)
{
   d_ptr->q_ptr = this;
}

QUndoGroup::~QUndoGroup()
{
   // Ensure all QUndoStacks no longer refer to this group.
   Q_D(QUndoGroup);

   QList<QUndoStack *>::iterator it = d->stack_list.begin();
   QList<QUndoStack *>::iterator end = d->stack_list.end();

   while (it != end) {
      (*it)->d_func()->group = nullptr;
      ++it;
   }
}

void QUndoGroup::addStack(QUndoStack *stack)
{
   Q_D(QUndoGroup);

   if (d->stack_list.contains(stack)) {
      return;
   }

   d->stack_list.append(stack);

   if (QUndoGroup *other = stack->d_func()->group) {
      other->removeStack(stack);
   }

   stack->d_func()->group = this;
}

void QUndoGroup::removeStack(QUndoStack *stack)
{
   Q_D(QUndoGroup);

   if (d->stack_list.removeAll(stack) == 0) {
      return;
   }

   if (stack == d->active) {
      setActiveStack(nullptr);
   }

   stack->d_func()->group = nullptr;
}

QList<QUndoStack *> QUndoGroup::stacks() const
{
   Q_D(const QUndoGroup);
   return d->stack_list;
}

void QUndoGroup::setActiveStack(QUndoStack *stack)
{
   Q_D(QUndoGroup);

   if (d->active == stack) {
      return;
   }

   if (d->active != nullptr) {
      disconnect(d->active, &QUndoStack::canUndoChanged,  this, &QUndoGroup::canUndoChanged);
      disconnect(d->active, &QUndoStack::undoTextChanged, this, &QUndoGroup::undoTextChanged);
      disconnect(d->active, &QUndoStack::canRedoChanged,  this, &QUndoGroup::canRedoChanged);
      disconnect(d->active, &QUndoStack::redoTextChanged, this, &QUndoGroup::redoTextChanged);
      disconnect(d->active, &QUndoStack::indexChanged,    this, &QUndoGroup::indexChanged);
      disconnect(d->active, &QUndoStack::cleanChanged,    this, &QUndoGroup::cleanChanged);
   }

   d->active = stack;

   if (d->active == nullptr) {
      emit canUndoChanged(false);
      emit undoTextChanged(QString());
      emit canRedoChanged(false);
      emit redoTextChanged(QString());
      emit cleanChanged(true);
      emit indexChanged(0);

   } else {
      connect(d->active, &QUndoStack::canUndoChanged,  this, &QUndoGroup::canUndoChanged);
      connect(d->active, &QUndoStack::undoTextChanged, this, &QUndoGroup::undoTextChanged);
      connect(d->active, &QUndoStack::canRedoChanged,  this, &QUndoGroup::canRedoChanged);
      connect(d->active, &QUndoStack::redoTextChanged, this, &QUndoGroup::redoTextChanged);
      connect(d->active, &QUndoStack::indexChanged,    this, &QUndoGroup::indexChanged);
      connect(d->active, &QUndoStack::cleanChanged,    this, &QUndoGroup::cleanChanged);

      emit canUndoChanged(d->active->canUndo());
      emit undoTextChanged(d->active->undoText());
      emit canRedoChanged(d->active->canRedo());
      emit redoTextChanged(d->active->redoText());
      emit cleanChanged(d->active->isClean());
      emit indexChanged(d->active->index());
   }

   emit activeStackChanged(d->active);
}

QUndoStack *QUndoGroup::activeStack() const
{
   Q_D(const QUndoGroup);
   return d->active;
}

void QUndoGroup::undo()
{
   Q_D(QUndoGroup);

   if (d->active != nullptr) {
      d->active->undo();
   }
}

void QUndoGroup::redo()
{
   Q_D(QUndoGroup);

   if (d->active != nullptr) {
      d->active->redo();
   }
}

bool QUndoGroup::canUndo() const
{
   Q_D(const QUndoGroup);
   return d->active != nullptr && d->active->canUndo();
}

bool QUndoGroup::canRedo() const
{
   Q_D(const QUndoGroup);
   return d->active != nullptr && d->active->canRedo();
}

QString QUndoGroup::undoText() const
{
   Q_D(const QUndoGroup);
   return d->active == nullptr ? QString() : d->active->undoText();
}

QString QUndoGroup::redoText() const
{
   Q_D(const QUndoGroup);
   return d->active == nullptr ? QString() : d->active->redoText();
}

bool QUndoGroup::isClean() const
{
   Q_D(const QUndoGroup);
   return d->active == nullptr || d->active->isClean();
}

#ifndef QT_NO_ACTION

QAction *QUndoGroup::createUndoAction(QObject *parent, const QString &prefix) const
{
   QUndoAction *result = new QUndoAction(prefix, parent);

   if (prefix.isEmpty()) {
      result->setTextFormat(tr("Undo %1"), tr("Undo", "Default text for undo action"));
   }

   result->setEnabled(canUndo());
   result->setPrefixedText(undoText());

   connect(this,   &QUndoGroup::canUndoChanged,  result, &QUndoAction::setEnabled);
   connect(this,   &QUndoGroup::undoTextChanged, result, &QUndoAction::setPrefixedText);
   connect(result, &QUndoAction::triggered,      this,   &QUndoGroup::undo);

   return result;
}

QAction *QUndoGroup::createRedoAction(QObject *parent, const QString &prefix) const
{
   QUndoAction *result = new QUndoAction(prefix, parent);

   if (prefix.isEmpty()) {
      result->setTextFormat(tr("Redo %1"), tr("Redo", "Default text for redo action"));
   }

   result->setEnabled(canRedo());
   result->setPrefixedText(redoText());

   connect(this,   &QUndoGroup::canRedoChanged,  result, &QUndoAction::setEnabled);
   connect(this,   &QUndoGroup::redoTextChanged, result, &QUndoAction::setPrefixedText);
   connect(result, &QUndoAction::triggered,      this,   &QUndoGroup::redo);

   return result;
}

#endif // QT_NO_ACTION

#endif // QT_NO_UNDOGROUP
