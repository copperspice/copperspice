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

#ifndef QHELPDBREADER_H
#define QHELPDBREADER_H

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

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtCore/QByteArray>
#include <QtCore/QSet>

QT_BEGIN_NAMESPACE

class QSqlQuery;

class QHelpDBReader : public QObject
{
    Q_OBJECT

public:
    QHelpDBReader(const QString &dbName); 
    QHelpDBReader(const QString &dbName, const QString &uniqueId,
        QObject *parent);
    ~QHelpDBReader();

    bool init();

    QString errorMessage() const;

    QString databaseName() const;
    QString namespaceName() const;
    QString virtualFolder() const;
    QList<QStringList> filterAttributeSets() const;
    QStringList files(const QStringList &filterAttributes,
        const QString &extensionFilter = QString()) const;
    bool fileExists(const QString &virtualFolder, const QString &filePath,
        const QStringList &filterAttributes = QStringList()) const;
    QByteArray fileData(const QString &virtualFolder,
        const QString &filePath) const;

    QStringList customFilters() const;
    QStringList filterAttributes(const QString &filterName = QString()) const;
    QStringList indicesForFilter(const QStringList &filterAttributes) const;
    void linksForKeyword(const QString &keyword, const QStringList &filterAttributes,
        QMap<QString, QUrl> &linkMap) const;

    void linksForIdentifier(const QString &id, const QStringList &filterAttributes,
        QMap<QString, QUrl> &linkMap) const;

    QList<QByteArray> contentsForFilter(const QStringList &filterAttributes) const;
    QUrl urlOfPath(const QString &relativePath) const;

    QSet<int> indexIds(const QStringList &attributes) const;
    bool createAttributesCache(const QStringList &attributes,
        const QSet<int> &indexIds);
    QVariant metaData(const QString &name) const;

private:
    void initObject(const QString &dbName, const QString &uniqueId);
    QUrl buildQUrl(const QString &ns, const QString &folder,
        const QString &relFileName, const QString &anchor) const;
    QString mergeList(const QStringList &list) const;
    QString quote(const QString &string) const;

    bool m_initDone;
    QString m_dbName;
    QString m_uniqueId;
    QString m_error;
    QSqlQuery *m_query;
    mutable QString m_namespace;
    QSet<QString> m_viewAttributes;
    bool m_useAttributesCache;
    QSet<int> m_indicesCache;
};

QT_END_NAMESPACE

#endif
