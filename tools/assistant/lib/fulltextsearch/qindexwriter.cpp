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

#include "qindexwriter_p.h"
#include "qindexreader_p.h"

#include <CLucene.h>
#include <CLucene/index/IndexWriter.h>

QT_BEGIN_NAMESPACE

QCLuceneIndexWriterPrivate::QCLuceneIndexWriterPrivate()
    : QSharedData()
{
    writer = 0;
    deleteCLuceneIndexWriter = true;
}

QCLuceneIndexWriterPrivate::QCLuceneIndexWriterPrivate(const QCLuceneIndexWriterPrivate &other)
    : QSharedData()
{
    writer = _CL_POINTER(other.writer);
    deleteCLuceneIndexWriter = other.deleteCLuceneIndexWriter;
}

QCLuceneIndexWriterPrivate::~QCLuceneIndexWriterPrivate()
{
    if (deleteCLuceneIndexWriter)
        _CLDECDELETE(writer);
}


QCLuceneIndexWriter::QCLuceneIndexWriter(const QString &path,
                                         QCLuceneAnalyzer &analyzer,
                                         bool create, bool closeDir)
    : d(new QCLuceneIndexWriterPrivate())
    , analyzer(analyzer)
{
    d->writer = new lucene::index::IndexWriter(path, 
        analyzer.d->analyzer, create, closeDir);
}

QCLuceneIndexWriter::~QCLuceneIndexWriter()
{
    // nothing todo
}

void QCLuceneIndexWriter::close()
{
    d->writer->close();
}

void QCLuceneIndexWriter::optimize()
{
    d->writer->optimize();
}

qint32 QCLuceneIndexWriter::docCount()
{
    return qint32(d->writer->docCount());
}

QCLuceneAnalyzer QCLuceneIndexWriter::getAnalyzer()
{
    return analyzer;
}

void QCLuceneIndexWriter::addIndexes(const QList<QCLuceneIndexReader*> &readers)
{
    using namespace lucene::index;
    IndexReader** readerArray = new IndexReader*[readers.count()];
    
    for (int i = 0; i < readers.count(); ++i)
        readerArray[i] = (readers.at(i))->d->reader;

    d->writer->addIndexes(readerArray);
    delete [] readerArray;
}

void QCLuceneIndexWriter::addDocument(QCLuceneDocument &doc, 
                                      QCLuceneAnalyzer &analyzer)
{
    if (doc.d->document)
        d->writer->addDocument(doc.d->document, analyzer.d->analyzer);
}

qint32 QCLuceneIndexWriter::getMaxFieldLength() const
{
    return qint32(d->writer->getMaxFieldLength());
}

void QCLuceneIndexWriter::setMaxFieldLength(qint32 value)
{
    d->writer->setMaxFieldLength(int32_t(value));
}

qint32 QCLuceneIndexWriter::getMaxBufferedDocs() const
{
    return qint32(d->writer->getMaxBufferedDocs());
}

void QCLuceneIndexWriter::setMaxBufferedDocs(qint32 value)
{
    d->writer->setMaxBufferedDocs(int32_t(value));
}

qint64 QCLuceneIndexWriter::getWriteLockTimeout() const
{
    return qint64(d->writer->getWriteLockTimeout());
}

void QCLuceneIndexWriter::setWriteLockTimeout(qint64 writeLockTimeout)
{
    d->writer->setWriteLockTimeout(int64_t(writeLockTimeout));
}

qint64 QCLuceneIndexWriter::getCommitLockTimeout() const
{
    return qint64(d->writer->getCommitLockTimeout());
}

void QCLuceneIndexWriter::setCommitLockTimeout(qint64 commitLockTimeout)
{
    d->writer->setCommitLockTimeout(int64_t(commitLockTimeout));
}

qint32 QCLuceneIndexWriter::getMergeFactor() const
{
    return qint32(d->writer->getMergeFactor());
}

void QCLuceneIndexWriter::setMergeFactor(qint32 value)
{
    d->writer->setMergeFactor(int32_t(value));
}

qint32 QCLuceneIndexWriter::getTermIndexInterval() const
{
    return qint32(d->writer->getTermIndexInterval());
}

void QCLuceneIndexWriter::setTermIndexInterval(qint32 interval)
{
    d->writer->setTermIndexInterval(int32_t(interval));
}

qint32 QCLuceneIndexWriter::getMinMergeDocs() const
{
    return qint32(d->writer->getMinMergeDocs());
}

void QCLuceneIndexWriter::setMinMergeDocs(qint32 value)
{
    d->writer->setMinMergeDocs(int32_t(value));
}

qint32 QCLuceneIndexWriter::getMaxMergeDocs() const
{
    return qint32(d->writer->getMaxMergeDocs());
}

void QCLuceneIndexWriter::setMaxMergeDocs(qint32 value)
{
    d->writer->setMaxMergeDocs(int32_t(value));
}

bool QCLuceneIndexWriter::getUseCompoundFile() const
{
    return d->writer->getUseCompoundFile();
}

void QCLuceneIndexWriter::setUseCompoundFile(bool value)
{
    d->writer->setUseCompoundFile(value);
}

QT_END_NAMESPACE
