/****************************************************************************
**
** Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team.
** All rights reserved.
**
** Portion Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
****************************************************************************/

#include "qsearchable_p.h"

#include <CLucene.h>
#include <CLucene/search/SearchHeader.h>

QT_BEGIN_NAMESPACE

QCLuceneSearchablePrivate::QCLuceneSearchablePrivate()
    : QSharedData()
{
    searchable = 0;
    deleteCLuceneSearchable = true;
}

QCLuceneSearchablePrivate::QCLuceneSearchablePrivate(const QCLuceneSearchablePrivate &other)
    : QSharedData()
{
    searchable = _CL_POINTER(other.searchable);
    deleteCLuceneSearchable = other.deleteCLuceneSearchable;
}

QCLuceneSearchablePrivate::~QCLuceneSearchablePrivate()
{
    if (deleteCLuceneSearchable)
        _CLDECDELETE(searchable);
}


QCLuceneSearchable::QCLuceneSearchable()
    : d(new QCLuceneSearchablePrivate())
{
    // nothing todo
}

QCLuceneSearchable::~QCLuceneSearchable()
{
    // nothing todo
}


QCLuceneSearcher::QCLuceneSearcher()
    : QCLuceneSearchable()
{
    // nothing todo
}

QCLuceneSearcher::~QCLuceneSearcher()
{
    // nothing todo;
}

QCLuceneHits QCLuceneSearcher::search(const QCLuceneQuery &query)
{
    return search(query, QCLuceneFilter());
}

QCLuceneHits QCLuceneSearcher::search(const QCLuceneQuery &query,
                                      const QCLuceneFilter &filter)
{
    return QCLuceneHits(*this, query, filter);
}

QCLuceneHits QCLuceneSearcher::search(const QCLuceneQuery &query,
                                      const QCLuceneSort &sort)
{
    return QCLuceneHits(*this, query, QCLuceneFilter(), sort);
}

QCLuceneHits QCLuceneSearcher::search(const QCLuceneQuery &query,
                                      const QCLuceneFilter &filter,
                                      const QCLuceneSort &sort)
{
    return QCLuceneHits(*this, query, filter, sort);
}


QCLuceneIndexSearcher::QCLuceneIndexSearcher(const QString &path)
    : QCLuceneSearcher()
{
    lucene::search::IndexSearcher *searcher =
        new lucene::search::IndexSearcher(path);

    reader.d->reader = searcher->getReader();
    reader.d->deleteCLuceneIndexReader = false;

    d->searchable = searcher;
}

QCLuceneIndexSearcher::QCLuceneIndexSearcher(const QCLuceneIndexReader &reader)
    : QCLuceneSearcher()
    , reader(reader)
{
    d->searchable = new lucene::search::IndexSearcher(reader.d->reader);
}

QCLuceneIndexSearcher::~QCLuceneIndexSearcher()
{
    // nothing todo
}

void QCLuceneIndexSearcher::close()
{
    d->searchable->close();
}

qint32 QCLuceneIndexSearcher::maxDoc() const
{
    return qint32(d->searchable->maxDoc());
}

QCLuceneIndexReader QCLuceneIndexSearcher::getReader()
{
    return reader;
}

bool QCLuceneIndexSearcher::doc(qint32 i, QCLuceneDocument &document)
{
    return d->searchable->doc(int32_t(i), document.d->document);
}


QCLuceneMultiSearcher::QCLuceneMultiSearcher(const QList<QCLuceneSearchable> searchables)
: QCLuceneSearcher()
{
    lucene::search::Searchable** list=
        _CL_NEWARRAY(lucene::search::Searchable*, searchables.count());

    d->searchable = new lucene::search::MultiSearcher(list);

    _CLDELETE_ARRAY(list);
}

QCLuceneMultiSearcher::~QCLuceneMultiSearcher()
{
    // nothing todo
}

void QCLuceneMultiSearcher::close()
{
    d->searchable->close();
}

qint32 QCLuceneMultiSearcher::maxDoc() const
{
    return qint32(d->searchable->maxDoc());
}

qint32 QCLuceneMultiSearcher::subDoc(qint32 index) const
{
    lucene::search::MultiSearcher *searcher =
        static_cast<lucene::search::MultiSearcher*> (d->searchable);

    if (searcher == 0)
        return 0;

    return qint32(searcher->subDoc(int32_t(index)));
}

qint32 QCLuceneMultiSearcher::subSearcher(qint32 index) const
{
    lucene::search::MultiSearcher *searcher =
        static_cast<lucene::search::MultiSearcher*> (d->searchable);

    if (searcher == 0)
        return 0;

    return qint32(searcher->subSearcher(int32_t(index)));
}

qint32 QCLuceneMultiSearcher::searcherIndex(qint32 index) const
{
    lucene::search::MultiSearcher *searcher =
        static_cast<lucene::search::MultiSearcher*> (d->searchable);

    if (searcher == 0)
        return 0;

    return qint32(searcher->searcherIndex(int32_t(index)));
}

bool QCLuceneMultiSearcher::doc(qint32 i, QCLuceneDocument &document)
{
    return d->searchable->doc(int32_t(i), document.d->document);
}

QT_END_NAMESPACE
