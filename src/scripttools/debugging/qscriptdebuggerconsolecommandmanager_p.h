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

#ifndef QSCRIPTDEBUGGERCONSOLECOMMANDMANAGER_P_H
#define QSCRIPTDEBUGGERCONSOLECOMMANDMANAGER_P_H

#include <QtCore/qobjectdefs.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qmap.h>
#include <QtCore/qlist.h>
#include <qscriptdebuggerconsolecommandgroupdata_p.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerConsoleCommand;
class QStringList;
class QScriptDebuggerConsoleCommandManagerPrivate;

class QScriptDebuggerConsoleCommandManager
{
 public:
   QScriptDebuggerConsoleCommandManager();
   ~QScriptDebuggerConsoleCommandManager();

   void addCommand(QScriptDebuggerConsoleCommand *command);
   void addCommandGroup(const QString &name,
                        const QScriptDebuggerConsoleCommandGroupData &data);

   QScriptDebuggerConsoleCommand *findCommand(const QString &name) const;
   QMap<QString, QList<QScriptDebuggerConsoleCommand *> > commands() const;
   QList<QScriptDebuggerConsoleCommand *> commandsInGroup(const QString &name) const;

   QScriptDebuggerConsoleCommandGroupData commandGroupData(const QString &name) const;
   QScriptDebuggerConsoleCommandGroupMap commandGroups() const;

   QStringList completions(const QString &prefix) const;

 private:
   QScopedPointer<QScriptDebuggerConsoleCommandManagerPrivate> d_ptr;

   Q_DECLARE_PRIVATE(QScriptDebuggerConsoleCommandManager)
};

QT_END_NAMESPACE

#endif
