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

#include <qsqlindex.h>

#include <qsqlfield.h>
#include <qstringlist.h>

QSqlIndex::QSqlIndex(const QString &cursorname, const QString &name)
   : cursor(cursorname), nm(name)
{
}

QSqlIndex::QSqlIndex(const QSqlIndex &other)
   : QSqlRecord(other), cursor(other.cursor), nm(other.nm), sorts(other.sorts)
{
}

QSqlIndex &QSqlIndex::operator=(const QSqlIndex &other)
{
   cursor = other.cursor;
   nm = other.nm;
   sorts = other.sorts;
   QSqlRecord::operator=(other);
   return *this;
}


QSqlIndex::~QSqlIndex()
{

}

void QSqlIndex::setName(const QString &name)
{
   nm = name;
}

void QSqlIndex::append(const QSqlField &field)
{
   append(field, false);
}

void QSqlIndex::append(const QSqlField &field, bool desc)
{
   sorts.append(desc);
   QSqlRecord::append(field);
}

bool QSqlIndex::isDescending(int i) const
{
   if (i >= 0 && i < sorts.size()) {
      return sorts[i];
   }
   return false;
}

void QSqlIndex::setDescending(int i, bool desc)
{
   if (i >= 0 && i < sorts.size()) {
      sorts[i] = desc;
   }
}


/*! \internal

  Creates a string representing the field number \a i using prefix \a
  prefix. If \a verbose is true, ASC or DESC is included in the field
  description if the field is sorted in ASCending or DESCending order.
*/

QString QSqlIndex::createField(int i, const QString &prefix, bool verbose) const
{
   QString f;
   if (!prefix.isEmpty()) {
      f += prefix + QLatin1Char('.');
   }
   f += field(i).name();
   if (verbose)
      f += QLatin1Char(' ') + QString((isDescending(i)
               ? QLatin1String("DESC") : QLatin1String("ASC")));
   return f;
}

void QSqlIndex::setCursorName(const QString &cursorName)
{
   cursor = cursorName;
}
