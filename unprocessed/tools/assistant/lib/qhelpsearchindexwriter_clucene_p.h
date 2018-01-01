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

#ifndef QHELPSEARCHINDEXWRITERCLUCENE_H
#define QHELPSEARCHINDEXWRITERCLUCENE_H

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

#include "qhelpenginecore.h"
#include "fulltextsearch/qanalyzer_p.h"

#include <QtCore/QUrl>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QStringList>
#include <QtCore/QWaitCondition>

QT_BEGIN_NAMESPACE

class QCLuceneIndexWriter;

namespace fulltextsearch {
namespace clucene {

class QHelpSearchIndexWriter : public QThread
{
    Q_OBJECT

public:
    QHelpSearchIndexWriter();
    ~QHelpSearchIndexWriter();

    void cancelIndexing();
    void updateIndex(const QString &collectionFile,
        const QString &indexFilesFolder, bool reindex);
    void optimizeIndex();

signals:
    void indexingStarted();
    void indexingFinished();

private:
    void run();

    bool addDocuments(const QList<QUrl> docFiles, const QHelpEngineCore &engine,
        const QStringList &attributes, const QString &namespaceName,
        QCLuceneIndexWriter *writer, QCLuceneAnalyzer &analyzer);
    void removeDocuments(const QString &indexPath, const QString &namespaceName);

    bool writeIndexMap(QHelpEngineCore& engine,
        const QMap<QString, QDateTime>& indexMap);

    QList<QUrl> indexableFiles(QHelpEngineCore *helpEngine,
        const QString &namespaceName, const QStringList &attributes) const;

    void closeIndexWriter(QCLuceneIndexWriter *writer);

private:
    QMutex mutex;
    QWaitCondition waitCondition;

    bool m_cancel;
    bool m_reindex;
    QString m_collectionFile;
    QString m_indexFilesFolder;
};

}   // namespace clucene
}   // namespace fulltextsearch


QT_END_NAMESPACE

#endif  // QHELPSEARCHINDEXWRITERCLUCENE_H
