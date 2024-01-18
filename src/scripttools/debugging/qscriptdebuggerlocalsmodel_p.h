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

#ifndef QSCRIPTDEBUGGERLOCALSMODEL_P_H
#define QSCRIPTDEBUGGERLOCALSMODEL_P_H

#include <QtCore/qabstractitemmodel.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerValue;
class QScriptDebuggerJobSchedulerInterface;
class QScriptDebuggerCommandSchedulerInterface;
class QScriptDebuggerLocalsModelPrivate;

class QScriptDebuggerLocalsModel : public QAbstractItemModel
{
   SCRIPT_T_CS_OBJECT(QScriptDebuggerLocalsModel)

 public:
   QScriptDebuggerLocalsModel(QScriptDebuggerJobSchedulerInterface *jobScheduler,
                              QScriptDebuggerCommandSchedulerInterface *commandScheduler,
                              QObject *parent = nullptr);

   ~QScriptDebuggerLocalsModel();

   void init(int frameIndex);
   void sync(int frameIndex);

   int frameIndex() const;

   QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
   QModelIndex parent(const QModelIndex &child) const;
   int columnCount(const QModelIndex &parent) const;
   int rowCount(const QModelIndex &parent) const;
   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
   bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
   QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const;
   Qt::ItemFlags flags(const QModelIndex &index) const;
   bool hasChildren(const QModelIndex &parent) const;
   bool canFetchMore(const QModelIndex &parent) const;
   void fetchMore(const QModelIndex &parent);

 public:
   CS_SIGNAL_1(Public, void scopeObjectAvailable(const QModelIndex &index))
   CS_SIGNAL_2(scopeObjectAvailable, index)

 private:
   Q_DECLARE_PRIVATE(QScriptDebuggerLocalsModel)
   Q_DISABLE_COPY(QScriptDebuggerLocalsModel)
};

QT_END_NAMESPACE

#endif
