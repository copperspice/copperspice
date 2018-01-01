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

#ifndef QSQLQUERYMODEL_P_H
#define QSQLQUERYMODEL_P_H

#include <qabstractitemmodel_p.h>
#include <QtSql/qsqlerror.h>
#include <QtSql/qsqlquery.h>
#include <QtSql/qsqlrecord.h>
#include <QtCore/qhash.h>
#include <QtCore/qvarlengtharray.h>
#include <QtCore/qvector.h>

QT_BEGIN_NAMESPACE

class QSqlQueryModelPrivate: public QAbstractItemModelPrivate
{
   Q_DECLARE_PUBLIC(QSqlQueryModel)

 public:
   QSqlQueryModelPrivate() : atEnd(false) {}
   ~QSqlQueryModelPrivate();

   void prefetch(int);
   void initColOffsets(int size);

   mutable QSqlQuery query;
   mutable QSqlError error;
   QModelIndex bottom;
   QSqlRecord rec;
   uint atEnd : 1;
   QVector<QHash<int, QVariant> > headers;
   QVarLengthArray<int, 56> colOffsets; // used to calculate indexInQuery of columns
};

QT_END_NAMESPACE

#endif // QSQLQUERYMODEL_P_H
