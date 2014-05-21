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

#ifndef QSCRIPTDEBUGGERSTACKMODEL_P_H
#define QSCRIPTDEBUGGERSTACKMODEL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qabstractitemmodel.h>

#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class QScriptContextInfo;

class QScriptDebuggerStackModelPrivate;
class QScriptDebuggerStackModel
    : public QAbstractTableModel
{
public:
    QScriptDebuggerStackModel(QObject *parent = 0);
    ~QScriptDebuggerStackModel();

    QList<QScriptContextInfo> contextInfos() const;
    void setContextInfos(const QList<QScriptContextInfo> &infos);

    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation, int role) const;

private:
    Q_DECLARE_PRIVATE(QScriptDebuggerStackModel)
    Q_DISABLE_COPY(QScriptDebuggerStackModel)
};

QT_END_NAMESPACE

#endif
