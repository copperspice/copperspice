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

#ifndef QHELPINDEXWIDGET_H
#define QHELPINDEXWIDGET_H

#include <QtHelp/qhelp_global.h>

#include <QtCore/QUrl>
#include <QtGui/QStringListModel>
#include <QtGui/QListView>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Help)

class QHelpEnginePrivate;
class QHelpIndexModelPrivate;

class QHELP_EXPORT QHelpIndexModel : public QStringListModel
{
    Q_OBJECT

public:
    void createIndex(const QString &customFilterName);
    QModelIndex filter(const QString &filter,
        const QString &wildcard = QString());

    QMap<QString, QUrl> linksForKeyword(const QString &keyword) const;
    bool isCreatingIndex() const;

Q_SIGNALS:
    void indexCreationStarted();
    void indexCreated();

private Q_SLOTS:
    void insertIndices();
    void invalidateIndex(bool onShutDown = false);

private:
    QHelpIndexModel(QHelpEnginePrivate *helpEngine);
    ~QHelpIndexModel();

    QHelpIndexModelPrivate *d;
    friend class QHelpEnginePrivate;
};

class QHELP_EXPORT QHelpIndexWidget : public QListView
{
    Q_OBJECT

Q_SIGNALS:
    void linkActivated(const QUrl &link, const QString &keyword);
    void linksActivated(const QMap<QString, QUrl> &links,
        const QString &keyword);

public Q_SLOTS:
    void filterIndices(const QString &filter,
        const QString &wildcard = QString());
    void activateCurrentItem();

private Q_SLOTS:
    void showLink(const QModelIndex &index);

private:
    QHelpIndexWidget();
    friend class QHelpEngine;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
