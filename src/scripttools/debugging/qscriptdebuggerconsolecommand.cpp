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

#include "qscriptdebuggerconsolecommand_p.h"
#include "qscriptdebuggerconsolecommand_p_p.h"

#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

/*!
  \since 4.5
  \class QScriptDebuggerConsoleCommand
  \internal

  \brief The QScriptDebuggerConsoleCommand class is the base class of console commands.

  \sa QScriptDebuggerConsoleCommandManager
*/

QScriptDebuggerConsoleCommandPrivate::QScriptDebuggerConsoleCommandPrivate()
{
}

QScriptDebuggerConsoleCommandPrivate::~QScriptDebuggerConsoleCommandPrivate()
{
}

QScriptDebuggerConsoleCommand::QScriptDebuggerConsoleCommand()
   : d_ptr(new QScriptDebuggerConsoleCommandPrivate)
{
   d_ptr->q_ptr = this;
}

QScriptDebuggerConsoleCommand::~QScriptDebuggerConsoleCommand()
{
}

QScriptDebuggerConsoleCommand::QScriptDebuggerConsoleCommand(QScriptDebuggerConsoleCommandPrivate &dd)
   : d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

/*!
  \fn QString QScriptDebuggerConsoleCommand::name() const

  Returns the name of this console command.
*/

/*!
  \fn QString QScriptDebuggerConsoleCommand::group() const

  Returns the group that this console command belongs to.
*/

/*!
  \fn QString QScriptDebuggerConsoleCommand::shortDescription() const

  Returns a short (one line) description of the command.
*/

/*!
  \fn QString QScriptDebuggerConsoleCommand::longDescription() const

  Returns a detailed description of how to use the command.
*/

/*!
  \fn QScriptDebuggerConsoleCommandJob *QScriptDebuggerConsoleCommand::createJob(
        const QStringList &arguments,
        QScriptDebuggerConsole *console,
        QScriptMessageHandlerInterface *messageHandler,
        QScriptDebuggerCommandSchedulerInterface *scheduler) = 0

  Creates a job that will execute this command with the given \a
  arguments. If the command cannot be executed (e.g. because one or
  more arguments are invalid), a suitable error message should be
  output to the \a messageHandler, and 0 should be returned.
*/

/*!
  Returns a list of names of commands that may also be of interest to
  users of this command.
*/
QStringList QScriptDebuggerConsoleCommand::seeAlso() const
{
   return QStringList();
}

/*!
  Returns a list of aliases for this command.
*/
QStringList QScriptDebuggerConsoleCommand::aliases() const
{
   return QStringList();
}

QStringList QScriptDebuggerConsoleCommand::argumentTypes() const
{
   return QStringList();
}

QStringList QScriptDebuggerConsoleCommand::subCommands() const
{
   return QStringList();
}

QT_END_NAMESPACE
