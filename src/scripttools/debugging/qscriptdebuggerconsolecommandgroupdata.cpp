/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qscriptdebuggerconsolecommandgroupdata_p.h"

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

/*!
  \since 4.5
  \class QScriptDebuggerConsoleCommandGroupData
  \internal

  \brief The QScriptDebuggerConsoleCommandGroupData class holds data associated with a console command group.
*/

class QScriptDebuggerConsoleCommandGroupDataPrivate
{
 public:
   QScriptDebuggerConsoleCommandGroupDataPrivate();
   ~QScriptDebuggerConsoleCommandGroupDataPrivate();

   QString shortDescription;
   QString longDescription;

   QBasicAtomicInt ref;
};

QScriptDebuggerConsoleCommandGroupDataPrivate::QScriptDebuggerConsoleCommandGroupDataPrivate()
{
   ref.store(0);
}

QScriptDebuggerConsoleCommandGroupDataPrivate::~QScriptDebuggerConsoleCommandGroupDataPrivate()
{
}

QScriptDebuggerConsoleCommandGroupData::QScriptDebuggerConsoleCommandGroupData()
   : d_ptr(0)
{
}

QScriptDebuggerConsoleCommandGroupData::QScriptDebuggerConsoleCommandGroupData(
   const QString &shortDescription, const QString &longDescription)
   : d_ptr(new QScriptDebuggerConsoleCommandGroupDataPrivate)
{
   d_ptr->shortDescription = shortDescription;
   d_ptr->longDescription = longDescription;
   d_ptr->ref.ref();
}

QScriptDebuggerConsoleCommandGroupData::QScriptDebuggerConsoleCommandGroupData(
   const QScriptDebuggerConsoleCommandGroupData &other)
   : d_ptr(other.d_ptr.data())
{
   if (d_ptr) {
      d_ptr->ref.ref();
   }
}

QScriptDebuggerConsoleCommandGroupData::~QScriptDebuggerConsoleCommandGroupData()
{
}

QScriptDebuggerConsoleCommandGroupData &QScriptDebuggerConsoleCommandGroupData::operator=(
   const QScriptDebuggerConsoleCommandGroupData &other)
{
   d_ptr.assign(other.d_ptr.data());
   return *this;
}

QString QScriptDebuggerConsoleCommandGroupData::shortDescription() const
{
   Q_D(const QScriptDebuggerConsoleCommandGroupData);
   return d->shortDescription;
}

QString QScriptDebuggerConsoleCommandGroupData::longDescription() const
{
   Q_D(const QScriptDebuggerConsoleCommandGroupData);
   return d->longDescription;
}

bool QScriptDebuggerConsoleCommandGroupData::isValid() const
{
   Q_D(const QScriptDebuggerConsoleCommandGroupData);
   return (d != 0);
}

QT_END_NAMESPACE
