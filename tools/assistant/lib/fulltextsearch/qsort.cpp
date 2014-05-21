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

#include "qsort_p.h"
#include "qclucene_global_p.h"

#include <CLucene.h>
#include <CLucene/search/Sort.h>

QT_BEGIN_NAMESPACE

QCLuceneSortPrivate::QCLuceneSortPrivate()
    : QSharedData()
{
    sort = 0;
    deleteCLuceneSort = true;
}

QCLuceneSortPrivate::QCLuceneSortPrivate (const QCLuceneSortPrivate &other)
    : QSharedData()
{
    sort = _CL_POINTER(other.sort);
    deleteCLuceneSort = other.deleteCLuceneSort;
}

QCLuceneSortPrivate::~QCLuceneSortPrivate()
{
    if (deleteCLuceneSort)
        _CLDECDELETE(sort);
}


QCLuceneSort::QCLuceneSort()
    : d(new QCLuceneSortPrivate())
{
    d->sort = new lucene::search::Sort();
}

QCLuceneSort::QCLuceneSort(const QStringList &fieldNames)
    : d(new QCLuceneSortPrivate())
{
    d->sort = new lucene::search::Sort();
    setSort(fieldNames);
}

QCLuceneSort::QCLuceneSort(const QString &field, bool reverse)
    : d(new QCLuceneSortPrivate())
{
    d->sort = new lucene::search::Sort();
    setSort(field, reverse);
}

QCLuceneSort::~QCLuceneSort()
{
    // nothing todo
}

QString QCLuceneSort::toString() const
{
    return TCharToQString(d->sort->toString());
}

void QCLuceneSort::setSort(const QStringList &fieldNames)
{
    TCHAR **nameArray = new TCHAR*[fieldNames.count()];
    for (int i = 0; i < fieldNames.count(); ++i)
        nameArray[i] = QStringToTChar(fieldNames.at(i));

    d->sort->setSort((const TCHAR**)nameArray);

    for (int i = 0; i < fieldNames.count(); ++i)
        delete [] nameArray[i];
    delete [] nameArray;
}

void QCLuceneSort::setSort(const QString &field, bool reverse)
{
    TCHAR *name = QStringToTChar(field);
    d->sort->setSort(name, reverse);
    delete [] name;
}

QT_END_NAMESPACE
