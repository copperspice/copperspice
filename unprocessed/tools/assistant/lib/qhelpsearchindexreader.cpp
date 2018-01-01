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

#include "qhelpsearchindexreader_p.h"

QT_BEGIN_NAMESPACE

namespace fulltextsearch {

QHelpSearchIndexReader::QHelpSearchIndexReader()
    : QThread()
    , m_cancel(false)
{
    // nothing todo
}

QHelpSearchIndexReader::~QHelpSearchIndexReader()
{
    mutex.lock();
    this->m_cancel = true;
    mutex.unlock();

    wait();
}

void QHelpSearchIndexReader::cancelSearching()
{
    mutex.lock();
    this->m_cancel = true;
    mutex.unlock();
}

void QHelpSearchIndexReader::search(const QString &collectionFile, const QString &indexFilesFolder,
    const QList<QHelpSearchQuery> &queryList)
{
    wait();

    this->hitList.clear();
    this->m_cancel = false;
    this->m_query = queryList;
    this->m_collectionFile = collectionFile;
    this->m_indexFilesFolder = indexFilesFolder;

    start(QThread::NormalPriority);
}

int QHelpSearchIndexReader::hitCount() const
{
    QMutexLocker lock(&mutex);
    return hitList.count();
}

QList<QHelpSearchEngine::SearchHit> QHelpSearchIndexReader::hits(int start,
                                                                 int end) const
{
    QList<QHelpSearchEngine::SearchHit> hits;
    QMutexLocker lock(&mutex);
    for (int i = start; i < end && i < hitList.count(); ++i)
        hits.append(hitList.at(i));
    return hits;
}


}   // namespace fulltextsearch

QT_END_NAMESPACE
