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

#ifndef QSCRIPTDEBUGGERCONSOLECOMMANDGROUPDATA_P_H
#define QSCRIPTDEBUGGERCONSOLECOMMANDGROUPDATA_P_H

#include <QtCore/qobjectdefs.h>
#include <qscopedpointer_p.h>
#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

class QString;

class QScriptDebuggerConsoleCommandGroupDataPrivate;
class QScriptDebuggerConsoleCommandGroupData
{
 public:
   QScriptDebuggerConsoleCommandGroupData();
   QScriptDebuggerConsoleCommandGroupData(
      const QString &shortDescription,
      const QString &longDescription);
   QScriptDebuggerConsoleCommandGroupData(
      const QScriptDebuggerConsoleCommandGroupData &other);
   ~QScriptDebuggerConsoleCommandGroupData();

   QString shortDescription() const;
   QString longDescription() const;

   bool isValid() const;

   QScriptDebuggerConsoleCommandGroupData &operator=(
      const QScriptDebuggerConsoleCommandGroupData &other);

 private:
   QScopedSharedPointer<QScriptDebuggerConsoleCommandGroupDataPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QScriptDebuggerConsoleCommandGroupData)
};

typedef QMap<QString, QScriptDebuggerConsoleCommandGroupData> QScriptDebuggerConsoleCommandGroupMap;

QT_END_NAMESPACE

#endif
