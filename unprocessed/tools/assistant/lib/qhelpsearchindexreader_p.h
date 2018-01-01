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

#ifndef QHELPSEARCHINDEXREADER_H
#define QHELPSEARCHINDEXREADER_H

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

#include "qhelpsearchengine.h"

#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>

QT_BEGIN_NAMESPACE

class QHelpEngineCore;

namespace fulltextsearch {

class QHelpSearchIndexReader : public QThread
{
    Q_OBJECT

public:
    QHelpSearchIndexReader();
    ~QHelpSearchIndexReader();

    void cancelSearching();
    void search(const QString &collectionFile,
        const QString &indexFilesFolder,
        const QList<QHelpSearchQuery> &queryList);
    int hitCount() const;
    QList<QHelpSearchEngine::SearchHit> hits(int start, int end) const;

signals:
    void searchingStarted();
    void searchingFinished(int hits);

protected:
    mutable QMutex mutex;
    QList<QHelpSearchEngine::SearchHit> hitList;
    bool m_cancel;
    QString m_collectionFile;
    QList<QHelpSearchQuery> m_query;
    QString m_indexFilesFolder;

private:
    virtual void run()=0;
};

}   // namespace fulltextsearch

QT_END_NAMESPACE

#endif  // QHELPSEARCHINDEXREADER_H
