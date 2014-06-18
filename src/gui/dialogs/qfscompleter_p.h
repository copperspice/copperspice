/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

#ifndef QFSCOMPLETER_P_H
#define QFSCOMPLETER_P_H

#include "qcompleter.h"
#include <QtGui/qfilesystemmodel.h>
QT_BEGIN_NAMESPACE
#ifndef QT_NO_FSCOMPLETER

/*!
    QCompleter that can deal with QFileSystemModel
  */
class QFSCompleter :  public QCompleter
{
 public:
   QFSCompleter(QFileSystemModel *model, QObject *parent = 0)
      : QCompleter(model, parent), proxyModel(0), sourceModel(model) {
#if defined(Q_OS_WIN)
      setCaseSensitivity(Qt::CaseInsensitive);
#endif
   }
   QString pathFromIndex(const QModelIndex &index) const;
   QStringList splitPath(const QString &path) const;

   QAbstractProxyModel *proxyModel;
   QFileSystemModel *sourceModel;
};
#endif // QT_NO_FSCOMPLETER
QT_END_NAMESPACE
#endif // QFSCOMPLETOR_P_H

