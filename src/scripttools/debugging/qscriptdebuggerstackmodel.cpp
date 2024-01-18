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

#include "qscriptdebuggerstackmodel_p.h"
#include "qabstractitemmodel_p.h"
#include <QtScript/qscriptcontextinfo.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qcoreapplication.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerStackModelPrivate
   : public QAbstractItemModelPrivate
{
   Q_DECLARE_PUBLIC(QScriptDebuggerStackModel)
 public:
   QScriptDebuggerStackModelPrivate();
   ~QScriptDebuggerStackModelPrivate();

   QList<QScriptContextInfo> contextInfos;
};

QScriptDebuggerStackModelPrivate::QScriptDebuggerStackModelPrivate()
{
}

QScriptDebuggerStackModelPrivate::~QScriptDebuggerStackModelPrivate()
{
}

QScriptDebuggerStackModel::QScriptDebuggerStackModel(QObject *parent)
   : QAbstractTableModel(*new QScriptDebuggerStackModelPrivate, parent)
{
}

QScriptDebuggerStackModel::~QScriptDebuggerStackModel()
{
}

QList<QScriptContextInfo> QScriptDebuggerStackModel::contextInfos() const
{
   Q_D(const QScriptDebuggerStackModel);
   return d->contextInfos;
}

void QScriptDebuggerStackModel::setContextInfos(const QList<QScriptContextInfo> &infos)
{
   Q_D(QScriptDebuggerStackModel);
   layoutAboutToBeChanged();
   d->contextInfos = infos;
   layoutChanged();
}

/*!
  \reimp
*/
int QScriptDebuggerStackModel::columnCount(const QModelIndex &parent) const
{
   if (!parent.isValid()) {
      return 3;
   }
   return 0;
}

/*!
  \reimp
*/
int QScriptDebuggerStackModel::rowCount(const QModelIndex &parent) const
{
   Q_D(const QScriptDebuggerStackModel);
   if (!parent.isValid()) {
      return d->contextInfos.count();
   }
   return 0;
}

/*!
  \reimp
*/
QVariant QScriptDebuggerStackModel::data(const QModelIndex &index, int role) const
{
   Q_D(const QScriptDebuggerStackModel);
   if (!index.isValid()) {
      return QVariant();
   }
   if (index.row() >= d->contextInfos.count()) {
      return QVariant();
   }
   const QScriptContextInfo &info = d->contextInfos.at(index.row());
   if (role == Qt::DisplayRole) {
      if (index.column() == 0) {
         return index.row();
      } else if (index.column() == 1) {
         QString name = info.functionName();
         if (name.isEmpty()) {
            name = QString::fromLatin1("<anonymous>");
         }
         return name;
      } else if (index.column() == 2) {
         QString fn = QFileInfo(info.fileName()).fileName();
         if (fn.isEmpty()) {
            if (info.functionType() == QScriptContextInfo::ScriptFunction) {
               fn = QString::fromLatin1("<anonymous script, id=%0>").arg(info.scriptId());
            } else {
               fn = QString::fromLatin1("<native>");
            }

         }
         return QString::fromLatin1("%0:%1").arg(fn).arg(info.lineNumber());
      }
   } else if (role == Qt::ToolTipRole) {
      if (QFileInfo(info.fileName()).fileName() != info.fileName()) {
         return info.fileName();
      }
   }
   return QVariant();
}

/*!
  \reimp
*/
QVariant QScriptDebuggerStackModel::headerData(int section, Qt::Orientation orient, int role) const
{
   if (orient != Qt::Horizontal) {
      return QVariant();
   }
   if (role == Qt::DisplayRole) {
      if (section == 0) {
         return QCoreApplication::translate("QScriptDebuggerStackModel", "Level");
      } else if (section == 1) {
         return QCoreApplication::translate("QScriptDebuggerStackModel", "Name");
      } else if (section == 2) {
         return QCoreApplication::translate("QScriptDebuggerStackModel", "Location");
      }
   }
   return QVariant();
}

QT_END_NAMESPACE
