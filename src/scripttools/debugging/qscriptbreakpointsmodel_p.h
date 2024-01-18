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

#ifndef QSCRIPTBREAKPOINTSMODEL_P_H
#define QSCRIPTBREAKPOINTSMODEL_P_H

#include <QtCore/qabstractitemmodel.h>
#include "qscriptbreakpointdata_p.h"

QT_BEGIN_NAMESPACE

class QScriptDebuggerJobSchedulerInterface;
class QScriptDebuggerCommandSchedulerInterface;

class QScriptBreakpointsModelPrivate;
class QScriptBreakpointsModel
   : public QAbstractItemModel
{
   SCRIPT_T_CS_OBJECT(QScriptBreakpointsModel)
 public:
   QScriptBreakpointsModel(QScriptDebuggerJobSchedulerInterface *jobScheduler,
                           QScriptDebuggerCommandSchedulerInterface *commandScheduler,
                           QObject *parent = nullptr);
   ~QScriptBreakpointsModel();

   void setBreakpoint(const QScriptBreakpointData &data);
   void setBreakpointData(int id, const QScriptBreakpointData &data);
   void deleteBreakpoint(int id);

   void addBreakpoint(int id, const QScriptBreakpointData &data);
   void modifyBreakpoint(int id, const QScriptBreakpointData &data);
   void removeBreakpoint(int id);

   int breakpointIdAt(int row) const;
   QScriptBreakpointData breakpointDataAt(int row) const;
   QScriptBreakpointData breakpointData(int id) const;

   int resolveBreakpoint(qint64 scriptId, int lineNumber) const;
   int resolveBreakpoint(const QString &fileName, int lineNumber) const;

   QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
   QModelIndex parent(const QModelIndex &child) const;
   int columnCount(const QModelIndex &parent = QModelIndex()) const;
   int rowCount(const QModelIndex &parent = QModelIndex()) const;
   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
   bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
   QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const;
   Qt::ItemFlags flags(const QModelIndex &index) const;

 private:
   Q_DECLARE_PRIVATE(QScriptBreakpointsModel)
   Q_DISABLE_COPY(QScriptBreakpointsModel)
};

QT_END_NAMESPACE

#endif
