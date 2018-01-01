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

#ifndef QHELPSEARCHINDEXWRITERDEFAULT_H
#define QHELPSEARCHINDEXWRITERDEFAULT_H

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

#include "qhelpsearchindex_default_p.h"

#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QStringList>
#include <QtCore/QWaitCondition>

QT_BEGIN_NAMESPACE

namespace fulltextsearch {
namespace std {

class Writer
{
public:
    Writer(const QString &path);
    ~Writer();

    void reset();
    bool writeIndex() const;
    void removeIndex() const;
    void setIndexFile(const QString &namespaceName, const QString &attributes);
    void insertInIndex(const QString &string, int docNum);
    void insertInDocumentList(const QString &title, const QString &url);

private:
    QString indexPath;
    QString indexFile;
    QString documentFile;

    QHash<QString, Entry*> index;
    QList<QStringList> documentList;
};


class QHelpSearchIndexWriter : public QThread
{
    Q_OBJECT

public:
    QHelpSearchIndexWriter();
    ~QHelpSearchIndexWriter();

    void cancelIndexing();
    void updateIndex(const QString &collectionFile,
        const QString &indexFilesFolder, bool reindex);

signals:
    void indexingStarted();
    void indexingFinished();

private:
    void run();
    QString addNamespace(const QString namespaces, const QString &namespaceName);
    QString removeNamespace(const QString namespaces, const QString &namespaceName);

private:
    QMutex mutex;
    QWaitCondition waitCondition;

    bool m_cancel;
    bool m_reindex;
    QString m_collectionFile;
    QString m_indexFilesFolder;
};

}   // namespace std
}   // namespace fulltextsearch

QT_END_NAMESPACE

#endif  // QHELPSEARCHINDEXWRITERDEFAULT_H
