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

#ifndef QHELPSEARCHINDEXREADERDEFAULT_H
#define QHELPSEARCHINDEXREADERDEFAULT_H

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
#include "qhelpsearchindexreader_p.h"

#include <QtCore/QHash>
#include <QtCore/QPair>

QT_BEGIN_NAMESPACE

namespace fulltextsearch {
namespace std {

class Reader
{
    typedef QList<QStringList> DocumentList;
    typedef QHash<QString, Entry*> EntryTable;
    typedef QPair<EntryTable, DocumentList> Index;
    typedef QHash<QString, Index> IndexTable;

public:
    Reader();
    ~Reader();

    bool readIndex();
    bool initCheck() const;
    void setIndexPath(const QString &path);
    void filterFilesForAttributes(const QStringList &attributes);
    void setIndexFile(const QString &namespaceName, const QString &attributes);
    bool splitSearchTerm(const QString &searchTerm, QStringList *terms,
        QStringList *termSeq, QStringList *seqWords);

    void searchInIndex(const QStringList &terms);
    QVector<DocumentInfo> hits();
    bool searchForPattern(const QStringList &patterns,
        const QStringList &words, const QByteArray &data);

private:
    QVector<Document> setupDummyTerm(const QStringList &terms, const EntryTable &entryTable);
    QStringList getWildcardTerms(const QString &term, const EntryTable &entryTable);
    void buildMiniIndex(const QString &string);
    void reset();
    void cleanupIndex(EntryTable &entryTable);

private:
    uint wordNum;
    QString indexPath;
    QString indexFile;
    QString documentFile;

    IndexTable indexTable;
    QList<TermInfo> termList;
    IndexTable searchIndexTable;
    QHash<QString, PosEntry*> miniIndex;
};


class QHelpSearchIndexReaderDefault : public QHelpSearchIndexReader
{
    Q_OBJECT

public:
    QHelpSearchIndexReaderDefault();
    ~QHelpSearchIndexReaderDefault();

private:
    void run();

private:
    Reader m_reader;
};

}   // namespace std
}   // namespace fulltextsearch

QT_END_NAMESPACE

#endif  // QHELPSEARCHINDEXREADERDEFAULT_H
