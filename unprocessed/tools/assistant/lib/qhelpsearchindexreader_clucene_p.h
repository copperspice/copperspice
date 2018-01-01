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

#ifndef QHELPSEARCHINDEXREADERCLUCENE_H
#define QHELPSEARCHINDEXREADERCLUCENE_H

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
#include <QtCore/QStringList>

#include "fulltextsearch/qanalyzer_p.h"
#include "fulltextsearch/qquery_p.h"
#include "qhelpsearchindexreader_p.h"

QT_BEGIN_NAMESPACE

namespace fulltextsearch {
namespace clucene {

class QHelpSearchIndexReaderClucene : public QHelpSearchIndexReader
{
    Q_OBJECT

public:
    QHelpSearchIndexReaderClucene();
    ~QHelpSearchIndexReaderClucene();

private:
    void run();
    void boostSearchHits(const QHelpEngineCore &engine, QList<QHelpSearchEngine::SearchHit> &hitList,
        const QList<QHelpSearchQuery> &queryList);
    bool buildQuery(const QList<QHelpSearchQuery> &queries,
                    const QString &fieldName,
                    const QStringList &filterAttributes,
                    QCLuceneBooleanQuery &booleanQuery,
                    QCLuceneAnalyzer &analyzer);
    bool buildTryHarderQuery(const QList<QHelpSearchQuery> &queries,
                             const QString &fieldName,
                             const QStringList &filterAttributes,
                             QCLuceneBooleanQuery &booleanQuery,
                             QCLuceneAnalyzer &analyzer);
    bool addFuzzyQuery(const QHelpSearchQuery &query, const QString &fieldName,
                       QCLuceneBooleanQuery &booleanQuery, QCLuceneAnalyzer &analyzer);
    bool addWithoutQuery(const QHelpSearchQuery &query, const QString &fieldName,
                         QCLuceneBooleanQuery &booleanQuery);
    bool addPhraseQuery(const QHelpSearchQuery &query, const QString &fieldName,
                        QCLuceneBooleanQuery &booleanQuery);
    bool addAllQuery(const QHelpSearchQuery &query, const QString &fieldName,
                     QCLuceneBooleanQuery &booleanQuery);
    bool addDefaultQuery(const QHelpSearchQuery &query, const QString &fieldName,
                         bool allTermsRequired, QCLuceneBooleanQuery &booleanQuery,
                         QCLuceneAnalyzer &analyzer);
    bool addAtLeastQuery(const QHelpSearchQuery &query, const QString &fieldName,
                         QCLuceneBooleanQuery &booleanQuery, QCLuceneAnalyzer &analyzer);
    bool addAttributesQuery(const QStringList &filterAttributes,
               QCLuceneBooleanQuery &booleanQuery, QCLuceneAnalyzer &analyzer);
    bool isNegativeQuery(const QHelpSearchQuery &query) const;
};

}   // namespace clucene
}   // namespace fulltextsearch

QT_END_NAMESPACE

#endif  // QHELPSEARCHINDEXREADERCLUCENE_H
