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

#include "openpagesmodel.h"

#include "helpenginewrapper.h"
#include "helpviewer.h"
#include "tracer.h"

#include <QtCore/QStringList>
#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

OpenPagesModel::OpenPagesModel(QObject *parent) : QAbstractTableModel(parent)
{
    TRACE_OBJ
}

int OpenPagesModel::rowCount(const QModelIndex &parent) const
{
    TRACE_OBJ
    return  parent.isValid() ? 0 : m_pages.count();
}

int OpenPagesModel::columnCount(const QModelIndex &/*parent*/) const
{
    TRACE_OBJ
    return 2;
}

QVariant OpenPagesModel::data(const QModelIndex &index, int role) const
{
    TRACE_OBJ
    if (!index.isValid() || index.row() >= rowCount() || index.column() > 0
        || role != Qt::DisplayRole)
        return QVariant();
    QString title = m_pages.at(index.row())->title();
    title.replace(QLatin1Char('&'), QLatin1String("&&"));
    return title.isEmpty() ? QLatin1String("(Untitled)") : title;
}

void OpenPagesModel::addPage(const QUrl &url, qreal zoom)
{
    TRACE_OBJ
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    HelpViewer *page = new HelpViewer(zoom);
    connect(page, SIGNAL(titleChanged()), this, SLOT(handleTitleChanged()));
    m_pages << page;
    endInsertRows();
    page->setSource(url);
}

void OpenPagesModel::removePage(int index)
{
    TRACE_OBJ
    Q_ASSERT(index >= 0 && index < rowCount());
    beginRemoveRows(QModelIndex(), index, index);
    HelpViewer *page = m_pages.at(index);
    m_pages.removeAt(index);
    endRemoveRows();
    page->deleteLater();
}

HelpViewer *OpenPagesModel::pageAt(int index) const
{
    TRACE_OBJ
    Q_ASSERT(index >= 0 && index < rowCount());
    return m_pages.at(index);
}

void OpenPagesModel::handleTitleChanged()
{
    TRACE_OBJ
    HelpViewer *page = static_cast<HelpViewer *>(sender());
    const int row = m_pages.indexOf(page);
    Q_ASSERT(row != -1 );
    const QModelIndex &item = index(row, 0);
    emit dataChanged(item, item);
}

QT_END_NAMESPACE
