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

#include <qundostack.h>

#include <qalgorithms.h>
#include <qdebug.h>
#include <qundogroup.h>

#include <qundostack_p.h>

#ifndef QT_NO_UNDOCOMMAND

QUndoCommand::QUndoCommand(const QString &text, QUndoCommand *parent)
{
   d = new QUndoCommandPrivate;

   if (parent != nullptr) {
      parent->d->child_list.append(this);
   }

   setText(text);
}

QUndoCommand::QUndoCommand(QUndoCommand *parent)
{
   d = new QUndoCommandPrivate;

   if (parent != nullptr) {
      parent->d->child_list.append(this);
   }
}

QUndoCommand::~QUndoCommand()
{
   qDeleteAll(d->child_list);
   delete d;
}

int QUndoCommand::id() const
{
   return -1;
}

bool QUndoCommand::mergeWith(const QUndoCommand *command)
{
   (void) command;
   return false;
}

void QUndoCommand::redo()
{
   for (int i = 0; i < d->child_list.size(); ++i) {
      d->child_list.at(i)->redo();
   }
}

void QUndoCommand::undo()
{
   for (int i = d->child_list.size() - 1; i >= 0; --i) {
      d->child_list.at(i)->undo();
   }
}

QString QUndoCommand::text() const
{
   return d->text;
}

QString QUndoCommand::actionText() const
{
   return d->actionText;
}

void QUndoCommand::setText(const QString &text)
{
   int cdpos = text.indexOf('\n');

   if (cdpos > 0) {
      d->text = text.left(cdpos);
      d->actionText = text.mid(cdpos + 1);
   } else {
      d->text = text;
      d->actionText = text;
   }
}

int QUndoCommand::childCount() const
{
   return d->child_list.count();
}

const QUndoCommand *QUndoCommand::child(int index) const
{
   if (index < 0 || index >= d->child_list.count()) {
      return nullptr;
   }

   return d->child_list.at(index);
}

#endif // QT_NO_UNDOCOMMAND

#ifndef QT_NO_UNDOSTACK

#ifndef QT_NO_ACTION

QUndoAction::QUndoAction(const QString &prefix, QObject *parent)
   : QAction(parent)
{
   m_prefix = prefix;
}

void QUndoAction::setPrefixedText(const QString &text)
{
   if (m_defaultText.isEmpty()) {
      QString s = m_prefix;

      if (! m_prefix.isEmpty() && ! text.isEmpty()) {
         s.append(' ');
      }

      s.append(text);
      setText(s);

   } else {
      if (text.isEmpty()) {
         setText(m_defaultText);
      } else {
         setText(m_prefix.formatArg(text));
      }
   }
}

void QUndoAction::setTextFormat(const QString &textFormat, const QString &defaultText)
{
   m_prefix = textFormat;
   m_defaultText = defaultText;
}

#endif // QT_NO_ACTION

void QUndoStackPrivate::setIndex(int idx, bool clean)
{
   Q_Q(QUndoStack);

   bool was_clean = index == clean_index;

   if (idx != index) {
      index = idx;
      emit q->indexChanged(index);
      emit q->canUndoChanged(q->canUndo());
      emit q->undoTextChanged(q->undoText());
      emit q->canRedoChanged(q->canRedo());
      emit q->redoTextChanged(q->redoText());
   }

   if (clean) {
      clean_index = index;
   }

   bool is_clean = index == clean_index;

   if (is_clean != was_clean) {
      emit q->cleanChanged(is_clean);
   }
}

bool QUndoStackPrivate::checkUndoLimit()
{
   if (undo_limit <= 0 || !macro_stack.isEmpty() || undo_limit >= command_list.count()) {
      return false;
   }

   int del_count = command_list.count() - undo_limit;

   for (int i = 0; i < del_count; ++i) {
      delete command_list.takeFirst();
   }

   index -= del_count;

   if (clean_index != -1) {
      if (clean_index < del_count) {
         clean_index = -1;   // we've deleted the clean command
      } else {
         clean_index -= del_count;
      }
   }

   return true;
}

QUndoStack::QUndoStack(QObject *parent)
   : QObject(parent), d_ptr(new QUndoStackPrivate)
{
   d_ptr->q_ptr = this;

#ifndef QT_NO_UNDOGROUP

   if (QUndoGroup *group = qobject_cast<QUndoGroup *>(parent)) {
      group->addStack(this);
   }

#endif
}

QUndoStack::~QUndoStack()
{
#ifndef QT_NO_UNDOGROUP
   Q_D(QUndoStack);

   if (d->group != nullptr) {
      d->group->removeStack(this);
   }

#endif

   clear();
}

void QUndoStack::clear()
{
   Q_D(QUndoStack);

   if (d->command_list.isEmpty()) {
      return;
   }

   bool was_clean = isClean();

   d->macro_stack.clear();
   qDeleteAll(d->command_list);
   d->command_list.clear();

   d->index = 0;
   d->clean_index = 0;

   emit indexChanged(0);
   emit canUndoChanged(false);
   emit undoTextChanged(QString());
   emit canRedoChanged(false);
   emit redoTextChanged(QString());

   if (! was_clean) {
      emit cleanChanged(true);
   }
}

void QUndoStack::push(QUndoCommand *cmd)
{
   Q_D(QUndoStack);

   cmd->redo();

   bool macro = ! d->macro_stack.isEmpty();
   QUndoCommand *cur = nullptr;

   if (macro) {
      QUndoCommand *macro_cmd = d->macro_stack.last();

      if (! macro_cmd->d->child_list.isEmpty()) {
         cur = macro_cmd->d->child_list.last();
      }

   } else {
      if (d->index > 0) {
         cur = d->command_list.at(d->index - 1);
      }

      while (d->index < d->command_list.size()) {
         delete d->command_list.takeLast();
      }

      if (d->clean_index > d->index) {
         d->clean_index = -1;   // we've deleted the clean state
      }
   }

   bool try_merge = cur != nullptr && cur->id() != -1
         && cur->id() == cmd->id() && (macro || d->index != d->clean_index);

   if (try_merge && cur->mergeWith(cmd)) {
      delete cmd;

      if (! macro) {
         emit indexChanged(d->index);
         emit canUndoChanged(canUndo());
         emit undoTextChanged(undoText());
         emit canRedoChanged(canRedo());
         emit redoTextChanged(redoText());
      }

   } else {
      if (macro) {
         d->macro_stack.last()->d->child_list.append(cmd);
      } else {
         d->command_list.append(cmd);
         d->checkUndoLimit();
         d->setIndex(d->index + 1, false);
      }
   }
}

void QUndoStack::setClean()
{
   Q_D(QUndoStack);

   if (! d->macro_stack.isEmpty()) {
      qWarning("QUndoStack::setClean() Unable to clean the stack while executing a macro");
      return;
   }

   d->setIndex(d->index, true);
}

bool QUndoStack::isClean() const
{
   Q_D(const QUndoStack);

   if (! d->macro_stack.isEmpty()) {
      return false;
   }

   return d->clean_index == d->index;
}

int QUndoStack::cleanIndex() const
{
   Q_D(const QUndoStack);
   return d->clean_index;
}

void QUndoStack::undo()
{
   Q_D(QUndoStack);

   if (d->index == 0) {
      return;
   }

   if (! d->macro_stack.isEmpty()) {
      qWarning("QUndoStack::undo() Unable to undo the stack while executing a macro");
      return;
   }

   int idx = d->index - 1;
   d->command_list.at(idx)->undo();
   d->setIndex(idx, false);
}

void QUndoStack::redo()
{
   Q_D(QUndoStack);

   if (d->index == d->command_list.size()) {
      return;
   }

   if (! d->macro_stack.isEmpty()) {
      qWarning("QUndoStack::redo() Unable to redo the stack while executing a macro");
      return;
   }

   d->command_list.at(d->index)->redo();
   d->setIndex(d->index + 1, false);
}

int QUndoStack::count() const
{
   Q_D(const QUndoStack);
   return d->command_list.size();
}

int QUndoStack::index() const
{
   Q_D(const QUndoStack);
   return d->index;
}

void QUndoStack::setIndex(int idx)
{
   Q_D(QUndoStack);

   if (! d->macro_stack.isEmpty()) {
      qWarning("QUndoStack::setIndex() Unable to set the index on the stack while executing a macro");
      return;
   }

   if (idx < 0) {
      idx = 0;
   } else if (idx > d->command_list.size()) {
      idx = d->command_list.size();
   }

   int i = d->index;

   while (i < idx) {
      d->command_list.at(i++)->redo();
   }

   while (i > idx) {
      d->command_list.at(--i)->undo();
   }

   d->setIndex(idx, false);
}

bool QUndoStack::canUndo() const
{
   Q_D(const QUndoStack);

   if (! d->macro_stack.isEmpty()) {
      return false;
   }

   return d->index > 0;
}

bool QUndoStack::canRedo() const
{
   Q_D(const QUndoStack);

   if (!d->macro_stack.isEmpty()) {
      return false;
   }

   return d->index < d->command_list.size();
}

QString QUndoStack::undoText() const
{
   Q_D(const QUndoStack);

   if (!d->macro_stack.isEmpty()) {
      return QString();
   }

   if (d->index > 0) {
      return d->command_list.at(d->index - 1)->actionText();
   }

   return QString();
}

QString QUndoStack::redoText() const
{
   Q_D(const QUndoStack);

   if (!d->macro_stack.isEmpty()) {
      return QString();
   }

   if (d->index < d->command_list.size()) {
      return d->command_list.at(d->index)->actionText();
   }

   return QString();
}

#ifndef QT_NO_ACTION
QAction *QUndoStack::createUndoAction(QObject *parent, const QString &prefix) const
{
   QUndoAction *result = new QUndoAction(prefix, parent);

   if (prefix.isEmpty()) {
      result->setTextFormat(tr("Undo %1"), tr("Undo", "Default text for undo action"));
   }

   result->setEnabled(canUndo());
   result->setPrefixedText(undoText());

   connect(this,   &QUndoStack::canUndoChanged,  result, &QUndoAction::setEnabled);
   connect(this,   &QUndoStack::undoTextChanged, result, &QUndoAction::setPrefixedText);
   connect(result, &QUndoAction::triggered,      this,   &QUndoStack::undo);

   return result;
}

QAction *QUndoStack::createRedoAction(QObject *parent, const QString &prefix) const
{
   QUndoAction *result = new QUndoAction(prefix, parent);

   if (prefix.isEmpty()) {
      result->setTextFormat(tr("Redo %1"), tr("Redo", "Default text for redo action"));
   }

   result->setEnabled(canRedo());
   result->setPrefixedText(redoText());

   connect(this,   &QUndoStack::canRedoChanged,  result, &QUndoAction::setEnabled);
   connect(this,   &QUndoStack::redoTextChanged, result, &QUndoAction::setPrefixedText);
   connect(result, &QUndoAction::triggered,      this,   &QUndoStack::redo);

   return result;
}
#endif

void QUndoStack::beginMacro(const QString &text)
{
   Q_D(QUndoStack);
   QUndoCommand *cmd = new QUndoCommand();
   cmd->setText(text);

   if (d->macro_stack.isEmpty()) {
      while (d->index < d->command_list.size()) {
         delete d->command_list.takeLast();
      }

      if (d->clean_index > d->index) {
         d->clean_index = -1;   // we have deleted the clean state
      }

      d->command_list.append(cmd);

   } else {
      d->macro_stack.last()->d->child_list.append(cmd);
   }

   d->macro_stack.append(cmd);

   if (d->macro_stack.count() == 1) {
      emit canUndoChanged(false);
      emit undoTextChanged(QString());
      emit canRedoChanged(false);
      emit redoTextChanged(QString());
   }
}

void QUndoStack::endMacro()
{
   Q_D(QUndoStack);

   if (d->macro_stack.isEmpty()) {
      qWarning("QUndoStack::endMacro() No matching beginMacro()");
      return;
   }

   d->macro_stack.removeLast();

   if (d->macro_stack.isEmpty()) {
      d->checkUndoLimit();
      d->setIndex(d->index + 1, false);
   }
}

const QUndoCommand *QUndoStack::command(int index) const
{
   Q_D(const QUndoStack);

   if (index < 0 || index >= d->command_list.count()) {
      return nullptr;
   }

   return d->command_list.at(index);
}

QString QUndoStack::text(int idx) const
{
   Q_D(const QUndoStack);

   if (idx < 0 || idx >= d->command_list.size()) {
      return QString();
   }

   return d->command_list.at(idx)->text();
}

void QUndoStack::setUndoLimit(int limit)
{
   Q_D(QUndoStack);

   if (! d->command_list.isEmpty()) {
      qWarning("QUndoStack::setUndoLimit() Stack undo limit can only be set when the stack is empty");
      return;
   }

   if (limit == d->undo_limit) {
      return;
   }

   d->undo_limit = limit;
   d->checkUndoLimit();
}

int QUndoStack::undoLimit() const
{
   Q_D(const QUndoStack);

   return d->undo_limit;
}

void QUndoStack::setActive(bool active)
{
#ifdef QT_NO_UNDOGROUP
   (void) active;
#else
   Q_D(QUndoStack);

   if (d->group != nullptr) {
      if (active) {
         d->group->setActiveStack(this);
      } else if (d->group->activeStack() == this) {
         d->group->setActiveStack(nullptr);
      }
   }

#endif
}

bool QUndoStack::isActive() const
{
#ifdef QT_NO_UNDOGROUP
   return true;
#else
   Q_D(const QUndoStack);
   return d->group == nullptr || d->group->activeStack() == this;
#endif
}

#endif
