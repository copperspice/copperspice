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

#include "qfilter_p.h"

#include <CLucene.h>
#include <CLucene/search/Filter.h>

QT_BEGIN_NAMESPACE

QCLuceneFilterPrivate::QCLuceneFilterPrivate()
    : QSharedData()
{
    filter = 0;
    deleteCLuceneFilter = true;
}

QCLuceneFilterPrivate::QCLuceneFilterPrivate(const QCLuceneFilterPrivate &other)
    : QSharedData()
{
    filter = _CL_POINTER(other.filter);
    deleteCLuceneFilter = other.deleteCLuceneFilter;
}

QCLuceneFilterPrivate::~QCLuceneFilterPrivate ()
{
    if (deleteCLuceneFilter)
        _CLDECDELETE(filter);
}


QCLuceneFilter::QCLuceneFilter()
    : d(new QCLuceneFilterPrivate())
{
    // nothing todo
}

QCLuceneFilter::~QCLuceneFilter()
{
    // nothing todo
}

QT_END_NAMESPACE
