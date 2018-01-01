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

#include "fulltextsearch/qindexreader_p.h"
#include "fulltextsearch/qqueryparser_p.h"
#include "fulltextsearch/qsearchable_p.h"
#include "qclucenefieldnames_p.h"
#include "qhelpenginecore.h"

#include "qhelpsearchindexreader_clucene_p.h"

#include <QtCore/QDir>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <QtCore/QSharedPointer>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtCore/QMutexLocker>

QT_BEGIN_NAMESPACE

namespace fulltextsearch {
namespace clucene {

QHelpSearchIndexReaderClucene::QHelpSearchIndexReaderClucene()
    : QHelpSearchIndexReader()
{
    // nothing todo
}

QHelpSearchIndexReaderClucene::~QHelpSearchIndexReaderClucene()
{
}


void QHelpSearchIndexReaderClucene::run()
{
    mutex.lock();

    if (m_cancel) {
        mutex.unlock();
        return;
    }

    const QString collectionFile(this->m_collectionFile);
    const QList<QHelpSearchQuery> &queryList = this->m_query;
    const QString indexPath(m_indexFilesFolder);

    mutex.unlock();

    QHelpEngineCore engine(collectionFile, 0);
    if (!engine.setupData())
        return;

    QFileInfo fInfo(indexPath);
    if (fInfo.exists() && !fInfo.isWritable()) {
        qWarning("Full Text Search, could not read index (missing permissions).");
        return;
    }

    if(QCLuceneIndexReader::indexExists(indexPath)) {
        mutex.lock();
        if (m_cancel) {
            mutex.unlock();
            return;
        }
        mutex.unlock();

        emit searchingStarted();

#if !defined(QT_NO_EXCEPTIONS)
        try {
#endif
            QCLuceneBooleanQuery booleanQueryTitle;
            QCLuceneBooleanQuery booleanQueryContent;
            QCLuceneStandardAnalyzer analyzer;
            const QStringList& attribList =
                engine.filterAttributes(engine.currentFilter());
            bool titleQueryIsValid = buildQuery(queryList, TitleTokenizedField,
                                       attribList, booleanQueryTitle, analyzer);
            bool contentQueryIsValid = buildQuery(queryList, ContentField,
                                     attribList, booleanQueryContent, analyzer);
            if (!titleQueryIsValid && !contentQueryIsValid) {
                emit searchingFinished(0);
                return;
            }

            QCLuceneIndexSearcher indexSearcher(indexPath);

            // QCLuceneHits object must be allocated on the heap, because
            // there is no default constructor.
            QSharedPointer<QCLuceneHits> titleHits;
            QSharedPointer<QCLuceneHits> contentHits;
            if (titleQueryIsValid) {
                titleHits = QSharedPointer<QCLuceneHits>(new QCLuceneHits(
                    indexSearcher.search(booleanQueryTitle)));
            }
            if (contentQueryIsValid) {
                contentHits = QSharedPointer<QCLuceneHits>(new QCLuceneHits(
                    indexSearcher.search(booleanQueryContent)));
            }
            bool boost = true;
            if ((titleHits.isNull() || titleHits->length() == 0)
                && (contentHits.isNull() || contentHits->length() == 0)) {
                booleanQueryTitle = QCLuceneBooleanQuery();
                booleanQueryContent = QCLuceneBooleanQuery();
                titleQueryIsValid =
                    buildTryHarderQuery(queryList, TitleTokenizedField,
                                        attribList, booleanQueryTitle, analyzer);
                contentQueryIsValid =
                    buildTryHarderQuery(queryList, ContentField, attribList,
                                        booleanQueryContent, analyzer);
                if (!titleQueryIsValid && !contentQueryIsValid) {
                    emit searchingFinished(0);
                    return;
                }
                if (titleQueryIsValid) {
                    titleHits = QSharedPointer<QCLuceneHits>(new QCLuceneHits(
                        indexSearcher.search(booleanQueryTitle)));
                }
                if (contentQueryIsValid) {
                    contentHits = QSharedPointer<QCLuceneHits>(new QCLuceneHits(
                        indexSearcher.search(booleanQueryContent)));
                }
                boost = false;
            }
            QList<QSharedPointer<QCLuceneHits> > cluceneHitsList;
            if (!titleHits.isNull())
                cluceneHitsList.append(titleHits);
            if (!contentHits.isNull())
                cluceneHitsList.append(contentHits);

            QSet<QString> pathSet;
            QCLuceneDocument document;
            const QStringList namespaceList = engine.registeredDocumentations();

            foreach (const QSharedPointer<QCLuceneHits> &hits, cluceneHitsList) {
                for (qint32 i = 0; i < hits->length(); i++) {
                    document = hits->document(i);
                    const QString path = document.get(PathField);
                    if (!pathSet.contains(path) && namespaceList.contains(
                            document.get(NamespaceField), Qt::CaseInsensitive)) {
                        pathSet.insert(path);
                        hitList.append(qMakePair(path, document.get(TitleField)));
                    }
                    document.clear();

                    mutex.lock();
                    if (m_cancel) {
                        mutex.unlock();
                        emit searchingFinished(0);
                        return;
                    }
                    mutex.unlock();
                }
            }

            indexSearcher.close();
            const int count = hitList.count();
            if ((count > 0) && boost)
                boostSearchHits(engine, hitList, queryList);
            emit searchingFinished(hitList.count());

#if !defined(QT_NO_EXCEPTIONS)
        } catch(...) {
            mutex.lock();
            hitList.clear();
            mutex.unlock();
            emit searchingFinished(0);
        }
#endif
    }
}

bool QHelpSearchIndexReaderClucene::buildQuery(
    const QList<QHelpSearchQuery> &queries, const QString &fieldName,
    const QStringList &filterAttributes, QCLuceneBooleanQuery &booleanQuery,
    QCLuceneAnalyzer &analyzer)
{
    bool queryIsValid = false;
    foreach (const QHelpSearchQuery &query, queries) {
        if (fieldName != ContentField && isNegativeQuery(query)) {
            queryIsValid = false;
            break;
        }
        switch (query.fieldName) {
            case QHelpSearchQuery::FUZZY:
                if (addFuzzyQuery(query, fieldName, booleanQuery, analyzer))
                    queryIsValid = true;
                break;
            case QHelpSearchQuery::WITHOUT:
                if (fieldName != ContentField)
                    return false;
                if (addWithoutQuery(query, fieldName, booleanQuery))
                    queryIsValid = true;
                break;
            case QHelpSearchQuery::PHRASE:
               if (addPhraseQuery(query, fieldName, booleanQuery))
                   queryIsValid = true;
               break;
            case QHelpSearchQuery::ALL:
               if (addAllQuery(query, fieldName, booleanQuery))
                   queryIsValid = true;
               break;
            case QHelpSearchQuery::DEFAULT:
               if (addDefaultQuery(query, fieldName, true, booleanQuery, analyzer))
                   queryIsValid = true;
               break;
            case QHelpSearchQuery::ATLEAST:
               if (addAtLeastQuery(query, fieldName, booleanQuery, analyzer))
                   queryIsValid = true;
               break;
            default:
               Q_ASSERT(!"Invalid field name");
        }
    }

    if (queryIsValid && !filterAttributes.isEmpty()) {
        queryIsValid =
            addAttributesQuery(filterAttributes, booleanQuery, analyzer);
    }

    return queryIsValid;
}

bool QHelpSearchIndexReaderClucene::buildTryHarderQuery(
    const QList<QHelpSearchQuery> &queries, const QString &fieldName,
    const QStringList &filterAttributes, QCLuceneBooleanQuery &booleanQuery,
    QCLuceneAnalyzer &analyzer)
{
    if (queries.isEmpty())
        return false;
    const QHelpSearchQuery &query = queries.front();
    if (query.fieldName != QHelpSearchQuery::DEFAULT)
        return false;
    if (isNegativeQuery(query))
        return false;
    if (!addDefaultQuery(query, fieldName, false, booleanQuery, analyzer))
        return false;
    if (filterAttributes.isEmpty())
        return true;
    return addAttributesQuery(filterAttributes, booleanQuery, analyzer);
}

bool QHelpSearchIndexReaderClucene::isNegativeQuery(const QHelpSearchQuery &query) const
{
    const QString &search = query.wordList.join(" ");
    return search.contains('!') || search.contains('-')
            || search.contains(QLatin1String(" NOT "));
}

bool QHelpSearchIndexReaderClucene::addFuzzyQuery(const QHelpSearchQuery &query,
    const QString &fieldName, QCLuceneBooleanQuery &booleanQuery,
    QCLuceneAnalyzer &analyzer)
{
    bool queryIsValid = false;
    const QLatin1String fuzzy("~");
    foreach (const QString &term, query.wordList) {
        if (!term.isEmpty()) {
            QCLuceneQuery *lQuery =
                    QCLuceneQueryParser::parse(term + fuzzy, fieldName, analyzer);
            if (lQuery != 0) {
                booleanQuery.add(lQuery, true, false, false);
                queryIsValid = true;
            }
        }
    }
    return queryIsValid;
}

bool QHelpSearchIndexReaderClucene::addWithoutQuery(const QHelpSearchQuery &query,
    const QString &fieldName, QCLuceneBooleanQuery &booleanQuery)
{
    bool queryIsValid = false;
    const QStringList &stopWords = QCLuceneStopAnalyzer().englishStopWords();
    foreach (const QString &term, query.wordList) {
        if (stopWords.contains(term, Qt::CaseInsensitive))
            continue;
        QCLuceneQuery *lQuery = new QCLuceneTermQuery(QCLuceneTerm(
                fieldName, term.toLower()));
        booleanQuery.add(lQuery, true, false, true);
        queryIsValid = true;
    }
    return queryIsValid;
}

bool QHelpSearchIndexReaderClucene::addPhraseQuery(const QHelpSearchQuery &query,
    const QString &fieldName, QCLuceneBooleanQuery &booleanQuery)
{
    bool queryIsValid = false;
    const QString &term = query.wordList.at(0).toLower();
    if (term.contains(QLatin1Char(' '))) {
        const QStringList termList = term.split(QLatin1String(" "));
        QCLucenePhraseQuery *q = new QCLucenePhraseQuery();
        const QStringList stopWords = QCLuceneStopAnalyzer().englishStopWords();
        foreach (const QString &term, termList) {
            if (!stopWords.contains(term, Qt::CaseInsensitive))
                q->addTerm(QCLuceneTerm(fieldName, term.toLower()));
        }
        if (!q->getTerms().isEmpty()) {
            booleanQuery.add(q, true, true, false);
            queryIsValid = true;
        }
    } else {
        QCLuceneQuery *lQuery = new QCLuceneTermQuery(QCLuceneTerm(
                fieldName, term.toLower()));
        booleanQuery.add(lQuery, true, true, false);
        queryIsValid = true;
    }
    return queryIsValid;
}

bool QHelpSearchIndexReaderClucene::addAllQuery(const QHelpSearchQuery &query,
    const QString &fieldName, QCLuceneBooleanQuery &booleanQuery)
{
    bool queryIsValid = false;
    const QStringList &stopWords = QCLuceneStopAnalyzer().englishStopWords();
    foreach (const QString &term, query.wordList) {
        if (stopWords.contains(term, Qt::CaseInsensitive))
            continue;
        QCLuceneQuery *lQuery = new QCLuceneTermQuery(QCLuceneTerm(
                fieldName, term.toLower()));
        booleanQuery.add(lQuery, true, true, false);
        queryIsValid = true;
    }
    return queryIsValid;
}

bool QHelpSearchIndexReaderClucene::addDefaultQuery(const QHelpSearchQuery &query,
    const QString &fieldName, bool allTermsRequired,
    QCLuceneBooleanQuery &booleanQuery,
    QCLuceneAnalyzer &analyzer)
{
    bool queryIsValid = false;
    foreach (const QString &term, query.wordList) {
        QCLuceneQuery *lQuery =
            QCLuceneQueryParser::parse(term.toLower(), fieldName, analyzer);
        if (lQuery) {
            booleanQuery.add(lQuery, true, allTermsRequired, false);
            queryIsValid = true;
        }
    }
    return queryIsValid;
}

bool QHelpSearchIndexReaderClucene::addAtLeastQuery(
    const QHelpSearchQuery &query, const QString &fieldName,
    QCLuceneBooleanQuery &booleanQuery, QCLuceneAnalyzer &analyzer)
{
    bool queryIsValid = false;
    foreach (const QString &term, query.wordList) {
        if (!term.isEmpty()) {
            QCLuceneQuery *lQuery =
                QCLuceneQueryParser::parse(term, fieldName, analyzer);
            if (lQuery) {
                booleanQuery.add(lQuery, true, false, false);
                queryIsValid = true;
            }
        }
    }
    return queryIsValid;
}

bool QHelpSearchIndexReaderClucene::addAttributesQuery(
    const QStringList &filterAttributes, QCLuceneBooleanQuery &booleanQuery,
    QCLuceneAnalyzer &analyzer)
{
    QCLuceneQuery* lQuery = QCLuceneQueryParser::parse(QLatin1String("+")
        + filterAttributes.join(QLatin1String(" +")), AttributeField, analyzer);
    if (!lQuery)
        return false;
    booleanQuery.add(lQuery, true, true, false);
    return true;
}

void QHelpSearchIndexReaderClucene::boostSearchHits(const QHelpEngineCore &engine,
    QList<QHelpSearchEngine::SearchHit> &hitList, const QList<QHelpSearchQuery> &queryList)
{
    foreach (const QHelpSearchQuery &query, queryList) {
        if (query.fieldName != QHelpSearchQuery::DEFAULT)
            continue;

        QString joinedQuery = query.wordList.join(QLatin1String(" "));

        QCLuceneStandardAnalyzer analyzer;
        QCLuceneQuery *parsedQuery = QCLuceneQueryParser::parse(
            joinedQuery, ContentField, analyzer);

        if (parsedQuery) {
            joinedQuery = parsedQuery->toString();
            delete parsedQuery;
        }

        const QString contentString(ContentField + QLatin1String(":"));
        int length = contentString.length();
        int index = joinedQuery.indexOf(contentString);

        QString term;
        int nextIndex = 0;
        QStringList searchTerms;
        while (index != -1) {
            nextIndex = joinedQuery.indexOf(contentString, index + 1);
            term = joinedQuery.mid(index + length, nextIndex - (length + index)).simplified();
            if (term.startsWith(QLatin1String("\""))
                && term.endsWith(QLatin1String("\""))) {
                searchTerms.append(term.remove(QLatin1String("\"")));
            } else {
                searchTerms += term.split(QLatin1Char(' '));
            }
            index = nextIndex;
        }
        searchTerms.removeDuplicates();

        int count = qMin(75, hitList.count());
        QMap<int, QHelpSearchEngine::SearchHit> hitMap;
        for (int i = 0; i < count; ++i) {
            const QHelpSearchEngine::SearchHit &hit = hitList.at(i);
            QString data = QString::fromUtf8(engine.fileData(hit.first));

            int counter = 0;
            foreach (const QString &term, searchTerms)
                counter += data.count(term, Qt::CaseInsensitive);
            hitMap.insertMulti(counter, hit);
        }

        QList<QHelpSearchEngine::SearchHit> boostedList;
        QMap<int, QHelpSearchEngine::SearchHit>::const_iterator it = hitMap.constEnd();
        do {
            --it;
            boostedList.append(it.value());
        } while (it != hitMap.constBegin());
        boostedList += hitList.mid(count, hitList.count());
        mutex.lock();
        hitList = boostedList;
        mutex.unlock();
    }
}

}   // namespace clucene
}   // namespace fulltextsearch

QT_END_NAMESPACE
