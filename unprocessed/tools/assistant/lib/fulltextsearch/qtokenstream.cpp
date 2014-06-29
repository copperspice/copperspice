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

#include "qtokenstream_p.h"

#include <CLucene.h>
#include <CLucene/analysis/AnalysisHeader.h>

QT_BEGIN_NAMESPACE

QCLuceneTokenStreamPrivate::QCLuceneTokenStreamPrivate()
    : QSharedData()
{
    tokenStream = 0;
    deleteCLuceneTokenStream = true;
}

QCLuceneTokenStreamPrivate::QCLuceneTokenStreamPrivate(const QCLuceneTokenStreamPrivate &other)
    : QSharedData()
{
    tokenStream = _CL_POINTER(other.tokenStream);
    deleteCLuceneTokenStream = other.deleteCLuceneTokenStream;
}

QCLuceneTokenStreamPrivate::~QCLuceneTokenStreamPrivate()
{
    if (deleteCLuceneTokenStream)
        _CLDECDELETE(tokenStream);
}


QCLuceneTokenStream::QCLuceneTokenStream()
    : d(new QCLuceneTokenStreamPrivate())
{
    // nothing todo
}

QCLuceneTokenStream::~QCLuceneTokenStream()
{
    // nothing todo
}

void QCLuceneTokenStream::close()
{
    d->tokenStream->close();
}

bool QCLuceneTokenStream::next(QCLuceneToken &token)
{
    return d->tokenStream->next(token.d->token);   
}

QT_END_NAMESPACE
