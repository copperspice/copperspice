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

#include <qundogroup.h>
#include <qundostack.h>
#include <qundostack_p.h>

#ifndef QT_NO_UNDOGROUP

QT_BEGIN_NAMESPACE

class QUndoGroupPrivate
{
   Q_DECLARE_PUBLIC(QUndoGroup)

 public:
   QUndoGroupPrivate() : active(0) {}
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

/*!
    Destroys the QUndoGroup.
*/
QUndoGroup::~QUndoGroup()
{
   // Ensure all QUndoStacks no longer refer to this group.
   Q_D(QUndoGroup);
   QList<QUndoStack *>::iterator it = d->stack_list.begin();
   QList<QUndoStack *>::iterator end = d->stack_list.end();
   while (it != end) {
      (*it)->d_func()->group = 0;
      ++it;
   }
}

/*!
    Adds \a stack to this group. The group does not take ownership of the stack. Another
    way of adding a stack to a group is by specifying the group as the stack's parent
    QObject in QUndoStack::QUndoStack(). In this case, the stack is deleted when the
    group is deleted, in the usual manner of QObjects.

    \sa removeStack() stacks() QUndoStack::QUndoStack()
*/

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

/*!
    Removes \a stack from this group. If the stack was the active stack in the group,
    the active stack becomes 0.

    \sa addStack() stacks() QUndoStack::~QUndoStack()
*/

void QUndoGroup::removeStack(QUndoStack *stack)
{
   Q_D(QUndoGroup);

   if (d->stack_list.removeAll(stack) == 0) {
      return;
   }
   if (stack == d->active) {
      setActiveStack(0);
   }
   stack->d_func()->group = 0;
}

/*!
    Returns a list of stacks in this group.

    \sa addStack() removeStack()
*/

QList<QUndoStack *> QUndoGroup::stacks() const
{
   Q_D(const QUndoGroup);
   return d->stack_list;
}

/*!
    Sets the active stack of this group to \a stack.

    If the stack is not a member of this group, this function does nothing.

    Synonymous with calling QUndoStack::setActive() on \a stack.

    The actions returned by createUndoAction() and createRedoAction() will now behave
    in the same way as those returned by \a stack's QUndoStack::createUndoAction()
    and QUndoStack::createRedoAction().

    \sa QUndoStack::setActive() activeStack()
*/

void QUndoGroup::setActiveStack(QUndoStack *stack)
{
   Q_D(QUndoGroup);
   if (d->active == stack) {
      return;
   }

   if (d->active != 0) {
      disconnect(d->active, SIGNAL(canUndoChanged(bool)),             this, SLOT(canUndoChanged(bool)));
      disconnect(d->active, SIGNAL(undoTextChanged(const QString &)), this, SLOT(undoTextChanged(const QString &)));
      disconnect(d->active, SIGNAL(canRedoChanged(bool)),             this, SLOT(canRedoChanged(bool)));
      disconnect(d->active, SIGNAL(redoTextChanged(const QString &)), this, SLOT(redoTextChanged(const QString &)));
      disconnect(d->active, SIGNAL(indexChanged(int)),                this, SLOT(indexChanged(int)));
      disconnect(d->active, SIGNAL(cleanChanged(bool)),               this, SLOT(cleanChanged(bool)));
   }

   d->active = stack;

   if (d->active == 0) {
      emit canUndoChanged(false);
      emit undoTextChanged(QString());
      emit canRedoChanged(false);
      emit redoTextChanged(QString());
      emit cleanChanged(true);
      emit indexChanged(0);

   } else {
      connect(d->active, SIGNAL(canUndoChanged(bool)),              this, SLOT(canUndoChanged(bool)));
      connect(d->active, SIGNAL(undoTextChanged(const QString &)),  this, SLOT(undoTextChanged(const QString &)));
      connect(d->active, SIGNAL(canRedoChanged(bool)),              this, SLOT(canRedoChanged(bool)));
      connect(d->active, SIGNAL(redoTextChanged(const QString &)),  this, SLOT(redoTextChanged(const QString &)));
      connect(d->active, SIGNAL(indexChanged(int)),                 this, SLOT(indexChanged(int)));
      connect(d->active, SIGNAL(cleanChanged(bool)),                this, SLOT(cleanChanged(bool)));

      emit canUndoChanged(d->active->canUndo());
      emit undoTextChanged(d->active->undoText());
      emit canRedoChanged(d->active->canRedo());
      emit redoTextChanged(d->active->redoText());
      emit cleanChanged(d->active->isClean());
      emit indexChanged(d->active->index());
   }

   emit activeStackChanged(d->active);
}

/*!
    Returns the active stack of this group.

    If none of the stacks are active, or if the group is empty, this function
    returns 0.

    \sa setActiveStack() QUndoStack::setActive()
*/

QUndoStack *QUndoGroup::activeStack() const
{
   Q_D(const QUndoGroup);
   return d->active;
}

/*!
    Calls QUndoStack::undo() on the active stack.

    If none of the stacks are active, or if the group is empty, this function
    does nothing.

    \sa redo() canUndo() setActiveStack()
*/

void QUndoGroup::undo()
{
   Q_D(QUndoGroup);
   if (d->active != 0) {
      d->active->undo();
   }
}

/*!
    Calls QUndoStack::redo() on the active stack.

    If none of the stacks are active, or if the group is empty, this function
    does nothing.

    \sa undo() canRedo() setActiveStack()
*/


void QUndoGroup::redo()
{
   Q_D(QUndoGroup);
   if (d->active != 0) {
      d->active->redo();
   }
}

/*!
    Returns the value of the active stack's QUndoStack::canUndo().

    If none of the stacks are active, or if the group is empty, this function
    returns false.

    \sa canRedo() setActiveStack()
*/

bool QUndoGroup::canUndo() const
{
   Q_D(const QUndoGroup);
   return d->active != 0 && d->active->canUndo();
}

/*!
    Returns the value of the active stack's QUndoStack::canRedo().

    If none of the stacks are active, or if the group is empty, this function
    returns false.

    \sa canUndo() setActiveStack()
*/

bool QUndoGroup::canRedo() const
{
   Q_D(const QUndoGroup);
   return d->active != 0 && d->active->canRedo();
}

/*!
    Returns the value of the active stack's QUndoStack::undoText().

    If none of the stacks are active, or if the group is empty, this function
    returns an empty string.

    \sa redoText() setActiveStack()
*/

QString QUndoGroup::undoText() const
{
   Q_D(const QUndoGroup);
   return d->active == 0 ? QString() : d->active->undoText();
}

/*!
    Returns the value of the active stack's QUndoStack::redoText().

    If none of the stacks are active, or if the group is empty, this function
    returns an empty string.

    \sa undoText() setActiveStack()
*/

QString QUndoGroup::redoText() const
{
   Q_D(const QUndoGroup);
   return d->active == 0 ? QString() : d->active->redoText();
}

/*!
    Returns the value of the active stack's QUndoStack::isClean().

    If none of the stacks are active, or if the group is empty, this function
    returns true.

    \sa setActiveStack()
*/

bool QUndoGroup::isClean() const
{
   Q_D(const QUndoGroup);
   return d->active == 0 || d->active->isClean();
}

#ifndef QT_NO_ACTION

/*!
    Creates an undo QAction object with parent \a parent.

    Triggering this action will cause a call to QUndoStack::undo() on the active stack.
    The text of this action will always be the text of the command which will be undone
    in the next call to undo(), prefixed by \a prefix. If there is no command available
    for undo, if the group is empty or if none of the stacks are active, this action will
    be disabled.

    If \a prefix is empty, the default template "Undo %1" is used instead of prefix.
    Before Qt 4.8, the prefix "Undo" was used by default.

    \sa createRedoAction() canUndo() QUndoCommand::text()
*/

QAction *QUndoGroup::createUndoAction(QObject *parent, const QString &prefix) const
{
   QUndoAction *result = new QUndoAction(prefix, parent);
   if (prefix.isEmpty()) {
      result->setTextFormat(tr("Undo %1"), tr("Undo", "Default text for undo action"));
   }

   result->setEnabled(canUndo());
   result->setPrefixedText(undoText());

   connect(this, SIGNAL(canUndoChanged(bool)), result, SLOT(setEnabled(bool)));
   connect(this, SIGNAL(undoTextChanged(const QString &)), result, SLOT(setPrefixedText(const QString &)));
   connect(result, SIGNAL(triggered()), this, SLOT(undo()));

   return result;
}

/*!
    Creates an redo QAction object with parent \a parent.

    Triggering this action will cause a call to QUndoStack::redo() on the active stack.
    The text of this action will always be the text of the command which will be redone
    in the next call to redo(), prefixed by \a prefix. If there is no command available
    for redo, if the group is empty or if none of the stacks are active, this action will
    be disabled.

    If \a prefix is empty, the default template "Redo %1" is used instead of prefix.
    Before Qt 4.8, the prefix "Redo" was used by default.

    \sa createUndoAction() canRedo() QUndoCommand::text()
*/

QAction *QUndoGroup::createRedoAction(QObject *parent, const QString &prefix) const
{
   QUndoAction *result = new QUndoAction(prefix, parent);
   if (prefix.isEmpty()) {
      result->setTextFormat(tr("Redo %1"), tr("Redo", "Default text for redo action"));
   }

   result->setEnabled(canRedo());
   result->setPrefixedText(redoText());

   connect(this, SIGNAL(canRedoChanged(bool)),             result, SLOT(setEnabled(bool)));
   connect(this, SIGNAL(redoTextChanged(const QString &)), result, SLOT(setPrefixedText(const QString &)));
   connect(result, SIGNAL(triggered()), this, SLOT(redo()));

   return result;
}

#endif // QT_NO_ACTION

/*! \fn void QUndoGroup::activeStackChanged(QUndoStack *stack)

    This signal is emitted whenever the active stack of the group changes. This can happen
    when setActiveStack() or QUndoStack::setActive() is called, or when the active stack
    is removed form the group. \a stack is the new active stack. If no stack is active,
    \a stack is 0.

    \sa setActiveStack() QUndoStack::setActive()
*/

/*! \fn void QUndoGroup::indexChanged(int idx)

    This signal is emitted whenever the active stack emits QUndoStack::indexChanged()
    or the active stack changes.

    \a idx is the new current index, or 0 if the active stack is 0.

    \sa QUndoStack::indexChanged() setActiveStack()
*/

/*! \fn void QUndoGroup::cleanChanged(bool clean)

    This signal is emitted whenever the active stack emits QUndoStack::cleanChanged()
    or the active stack changes.

    \a clean is the new state, or true if the active stack is 0.

    \sa QUndoStack::cleanChanged() setActiveStack()
*/

/*! \fn void QUndoGroup::canUndoChanged(bool canUndo)

    This signal is emitted whenever the active stack emits QUndoStack::canUndoChanged()
    or the active stack changes.

    \a canUndo is the new state, or false if the active stack is 0.

    \sa QUndoStack::canUndoChanged() setActiveStack()
*/

/*! \fn void QUndoGroup::canRedoChanged(bool canRedo)

    This signal is emitted whenever the active stack emits QUndoStack::canRedoChanged()
    or the active stack changes.

    \a canRedo is the new state, or false if the active stack is 0.

    \sa QUndoStack::canRedoChanged() setActiveStack()
*/

/*! \fn void QUndoGroup::undoTextChanged(const QString &undoText)

    This signal is emitted whenever the active stack emits QUndoStack::undoTextChanged()
    or the active stack changes.

    \a undoText is the new state, or an empty string if the active stack is 0.

    \sa QUndoStack::undoTextChanged() setActiveStack()
*/

/*! \fn void QUndoGroup::redoTextChanged(const QString &redoText)

    This signal is emitted whenever the active stack emits QUndoStack::redoTextChanged()
    or the active stack changes.

    \a redoText is the new state, or an empty string if the active stack is 0.

    \sa QUndoStack::redoTextChanged() setActiveStack()
*/

QT_END_NAMESPACE

#endif // QT_NO_UNDOGROUP
