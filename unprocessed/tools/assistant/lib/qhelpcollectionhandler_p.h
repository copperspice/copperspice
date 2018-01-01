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

#ifndef QHELPCOLLECTIONHANDLER_H
#define QHELPCOLLECTIONHANDLER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of the help generator tools. This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QStringList>

#include <QtSql/QSqlQuery>

QT_BEGIN_NAMESPACE

class QHelpCollectionHandler : public QObject
{
    Q_OBJECT

public:
    struct DocInfo
    {
        QString fileName;
        QString folderName;
        QString namespaceName;
    };
    typedef QList<DocInfo> DocInfoList;

    explicit QHelpCollectionHandler(const QString &collectionFile,
        QObject *parent = 0);
    ~QHelpCollectionHandler();

    QString collectionFile() const;

    bool openCollectionFile();
    bool copyCollectionFile(const QString &fileName);

    QStringList customFilters() const;
    bool removeCustomFilter(const QString &filterName);
    bool addCustomFilter(const QString &filterName,
        const QStringList &attributes);

    DocInfoList registeredDocumentations() const;
    bool registerDocumentation(const QString &fileName);
    bool unregisterDocumentation(const QString &namespaceName);

    bool removeCustomValue(const QString &key);
    QVariant customValue(const QString &key, const QVariant &defaultValue) const;
    bool setCustomValue(const QString &key, const QVariant &value);

    bool addFilterAttributes(const QStringList &attributes);
    QStringList filterAttributes() const;
    QStringList filterAttributes(const QString &filterName) const;
    
    int registerNamespace(const QString &nspace, const QString &fileName);
    bool registerVirtualFolder(const QString &folderName, int namespaceId);
    void optimizeDatabase(const QString &fileName);

signals:
    void error(const QString &msg);

private:
    bool isDBOpened();
    bool createTables(QSqlQuery *query);

    bool m_dbOpened;
    QString m_collectionFile;
    QString m_connectionName;
    mutable QSqlQuery m_query;
};

QT_END_NAMESPACE

#endif  //QHELPCOLLECTIONHANDLER_H
